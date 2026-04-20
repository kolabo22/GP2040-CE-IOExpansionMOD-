#include "addons/jingle_player.h"
#include "storagemanager.h"
#include "hardware/gpio.h"
#include "hardware/sync.h"
#include "pico/time.h"

// GP2040-CE v0.7.12 用のピン定義と設定取得
#define JINGLE_PLAYER_VPP_PIN 21
#define JINGLE_PLAYER_BUSY_PIN 20

bool JinglePlayerAddon::available() {
    // Storage から Web コンフィグの設定を取得
    return Storage::getInstance().getAddonOptions().jinglePlayerOptions.enabled;
}

void JinglePlayerAddon::process() {
    // 1. 起動直後の1回限りの再生処理
    if (!bootPlayed) {
        const JinglePlayerOptions& options = Storage::getInstance().getAddonOptions().jinglePlayerOptions;
        setVolume((uint8_t)options.volume);
        sleep_ms(10);
        
        // 現在のモードを取得して初期保持
        lastInputMode = (uint8_t)Storage::getInstance().getGamepadOptions().inputMode;
        
        playJingleByMode();
        bootPlayed = true;
        return;
    }

    // 2. モード（機種）変更の検知
    uint8_t currentMode = (uint8_t)Storage::getInstance().getGamepadOptions().inputMode;
    if (currentMode != lastInputMode) {
        lastInputMode = currentMode; // 先に更新
        playJingleByMode();
    }
}

void JinglePlayerAddon::preprocess() {
    // 抽象クラスエラー回避のために実装
}

void JinglePlayerAddon::process() {
    uint8_t currentMode = (uint8_t)Storage::getInstance().getGamepadOptions().inputMode;
    
    // モードが変更された瞬間を検知
    if (currentMode != lastInputMode) {
        playJingleByMode();
        lastInputMode = currentMode;
    }
}

void JinglePlayerAddon::postprocess(bool reportSent) {
    // 抽象クラスエラー回避のために実装
}

void JinglePlayerAddon::setVolume(uint8_t volume) {
    if (volume > 30) volume = 30;
    
    sendOneLineCommand(0x0A); // 数値クリア
    sleep_ms(5);
    
    if (volume >= 10) {
        sendOneLineCommand(volume / 10);
        sleep_ms(5);
    }
    sendOneLineCommand(volume % 10);
    sleep_ms(5);
    
    sendOneLineCommand(0x0C); // 音量設定コマンド
}

void JinglePlayerAddon::playJingleByMode() {
    if (isPlaying()) {
        sendOneLineCommand(0x13); // Stop
        sleep_ms(10);
    }
    sendOneLineCommand(0x0A); // 数値クリア
    sleep_ms(5);
    sendOneLineCommand(lastInputMode + 1); // モード0->曲1
    sleep_ms(5);
    sendOneLineCommand(0x0B); // 選曲確定・再生
}

bool JinglePlayerAddon::isPlaying() {
    return gpio_get(JINGLE_PLAYER_BUSY_PIN) == 1;
}

void JinglePlayerAddon::sendOneLineCommand(uint8_t addr) {
    // 割り込み禁止（save_and_disable_interrupts）を一旦削除、または極力短くします
    gpio_put(JINGLE_PLAYER_VPP_PIN, 0);
    sleep_us(4000); // Guide: 4ms

    for (int i = 0; i < 8; i++) {
        gpio_put(JINGLE_PLAYER_VPP_PIN, 1);
        if (addr & 0x01) {
            sleep_us(600); // 3:1 ratio
            gpio_put(JINGLE_PLAYER_VPP_PIN, 0);
            sleep_us(200);
        } else {
            sleep_us(200); // 1:3 ratio
            gpio_put(JINGLE_PLAYER_VPP_PIN, 0);
            sleep_us(600);
        }
        addr >>= 1;
    }
    gpio_put(JINGLE_PLAYER_VPP_PIN, 1);
    // 最後に短い猶予を入れる
    sleep_us(500); 
}

void JinglePlayerAddon::reinit() {
    // 空でOK
}
