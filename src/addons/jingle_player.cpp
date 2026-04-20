#include "addons/jingle_player.h"
#include "storagemanager.h"
#include "hardware/gpio.h"
#include "hardware/sync.h"
#include "pico/time.h"

// ピン定義 (固定)
#define JINGLE_PLAYER_VPP_PIN 21
#define JINGLE_PLAYER_BUSY_PIN 20

/**
 * アドオンが有効かどうかを判定
 */
bool JinglePlayerAddon::available() {
    return Storage::getInstance().getAddonOptions().jinglePlayerOptions.enabled;
}

/**
 * 初期設定：起動時に一度だけ実行
 */
void JinglePlayerAddon::setup() {
    gpio_init(JINGLE_PLAYER_VPP_PIN);
    gpio_set_dir(JINGLE_PLAYER_VPP_PIN, GPIO_OUT);
    gpio_put(JINGLE_PLAYER_VPP_PIN, 1); // Idle High

    gpio_init(JINGLE_PLAYER_BUSY_PIN);
    gpio_set_dir(JINGLE_PLAYER_BUSY_PIN, GPIO_IN);
    gpio_pull_up(JINGLE_PLAYER_BUSY_PIN);
    
    // 現在のモードを取得して初期値とする
    lastInputMode = (uint8_t)Storage::getInstance().getGamepadOptions().inputMode;
    bootPlayed = false;

    // S2ボタン(WebConfigモード)での起動を検知
    // v0.7.12 では Storage 内部の config 構造体にある configMode を参照します
    if (Storage::getInstance().getConfig().configMode) {
        lastInputMode = 20; // 内部ID 20 (Config) として扱う
    }

}

/**
 * 抽象クラスエラー回避用の空実装
 */
void JinglePlayerAddon::preprocess() {}
void JinglePlayerAddon::postprocess(bool reportSent) {}
void JinglePlayerAddon::reinit() {}

/**
 * メインループ：モード変更の監視と再生指示
 */
void JinglePlayerAddon::process() {
    // 1. 起動直後の再生
    if (!bootPlayed) {
        static uint32_t bootDelay = 0;
        if (bootDelay < 1000) { // 起動時はシステムが安定するまで待機
            bootDelay++;
            return;
        }

        const JinglePlayerOptions& options = Storage::getInstance().getAddonOptions().jinglePlayerOptions;
        setVolume((uint8_t)options.volume);
        
        sleep_ms(50);
        playJingleByMode();
        bootPlayed = true;
        return;
    }

    // 2. モード（機種）変更の検知
    uint8_t currentMode = (uint8_t)Storage::getInstance().getGamepadOptions().inputMode;
    
    // 二度鳴り防止用デバウンス
    // メニュー終了時などに値が不安定になるのを防ぐ
    static uint32_t changeDebounce = 0;
    if (currentMode != lastInputMode) {
        if (changeDebounce < 5000) { // ループ回数でカウント
            changeDebounce++;
        } else {
            lastInputMode = currentMode;
            playJingleByMode();
            changeDebounce = 0;
        }
    } else {
        changeDebounce = 0; 
    }
}

/**
 * 音量設定コマンドの送信 (0-30)
 */
void JinglePlayerAddon::setVolume(uint8_t volume) {
    if (volume > 30) volume = 30;
    
    sendOneLineCommand(0x0A); // 数値クリア
    sleep_ms(20);
    
    if (volume >= 10) {
        sendOneLineCommand(volume / 10);
        sleep_ms(20);
    }
    sendOneLineCommand(volume % 10);
    sleep_ms(20);
    
    sendOneLineCommand(0x0C); // 音量設定確定
}

/**
 * 現在のモードIDに基づいた選曲と再生
 */
void JinglePlayerAddon::playJingleByMode() {
    // 再生中の場合は停止
    if (isPlaying()) {
        sendOneLineCommand(0x13); // Stop
        sleep_ms(100);
    }
    
    sendOneLineCommand(0x0A); // 数値クリア
    sleep_ms(20);
    
    uint8_t songNumber = 0;
    // 内部IDに基づく曲番号計算 (ID順)
    if (lastInputMode == 20) {
        songNumber = 21; // Config(ID:20) は 0021.mp3
    } else {
        songNumber = lastInputMode + 1; // ID:0(XInput) は 0001.mp3
    }

    if (songNumber > 0) {
        sendOneLineCommand(songNumber); 
        sleep_ms(20);
        sendOneLineCommand(0x0B); // 再生確定
    }
}

/**
 * JQ8900が再生中(Busy)か確認
 */
bool JinglePlayerAddon::isPlaying() {
    return gpio_get(JINGLE_PLAYER_BUSY_PIN) == 1;
}

/**
 * JQ8900 One-lineシリアル送信プロトコル
 */
void JinglePlayerAddon::sendOneLineCommand(uint8_t addr) {
    // 信号開始 (Guide Code): 4ms Low
    gpio_put(JINGLE_PLAYER_VPP_PIN, 0);
    sleep_us(4000);

    // 8ビットデータ送信 (LSB First)
    for (int i = 0; i < 8; i++) {
        gpio_put(JINGLE_PLAYER_VPP_PIN, 1);
        if (addr & 0x01) {
            // Bit 1 - High:Low = 3:1 (600us:200us)
            sleep_us(600);
            gpio_put(JINGLE_PLAYER_VPP_PIN, 0);
            sleep_us(200);
        } else {
            // Bit 0 - High:Low = 1:3 (200us:600us)
            sleep_us(200);
            gpio_put(JINGLE_PLAYER_VPP_PIN, 0);
            sleep_us(600);
        }
        addr >>= 1;
    }
    // 送信終了後の Idle High
    gpio_put(JINGLE_PLAYER_VPP_PIN, 1);
    sleep_us(500); 
}
