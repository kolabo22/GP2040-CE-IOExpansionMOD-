#include "addons/jingle_player.h"
#include "storagemanager.h"
#include "hardware/gpio.h"
#include "hardware/sync.h"
#include "pico/time.h"

// GP2040-CE v0.7.12 用のピン定義
#define JINGLE_PLAYER_VPP_PIN 21
#define JINGLE_PLAYER_BUSY_PIN 20

bool JinglePlayerAddon::available() {
    return Storage::getInstance().getAddonOptions().jinglePlayerOptions.enabled;
}

void JinglePlayerAddon::setup() {
    gpio_init(JINGLE_PLAYER_VPP_PIN);
    gpio_set_dir(JINGLE_PLAYER_VPP_PIN, GPIO_OUT);
    gpio_put(JINGLE_PLAYER_VPP_PIN, 1);

    gpio_init(JINGLE_PLAYER_BUSY_PIN);
    gpio_set_dir(JINGLE_PLAYER_BUSY_PIN, GPIO_IN);
    gpio_pull_up(JINGLE_PLAYER_BUSY_PIN);
    
    // 重要：起動時のモードをここで正確に掴んでおく
    lastInputMode = (uint8_t)Storage::getInstance().getGamepadOptions().inputMode;
    bootPlayed = false;
}

void JinglePlayerAddon::process() {
    // 1. 起動直後の再生
    if (!bootPlayed) {
        // システム起動直後のノイズや不安定さを避けるため、少し長めに待つ
        static uint32_t bootDelay = 0;
        if (bootDelay < 200) { // 約200~500ms程度待機（ループ回数で調整）
            bootDelay++;
            return;
        }

        const JinglePlayerOptions& options = Storage::getInstance().getAddonOptions().jinglePlayerOptions;
        setVolume((uint8_t)options.volume);
        
        sleep_ms(50); // JQ8900が音量設定を受け付ける時間を稼ぐ
        playJingleByMode();
        bootPlayed = true;
        return;
    }

    // 2. モード（機種）変更の検知
    uint8_t currentMode = (uint8_t)Storage::getInstance().getGamepadOptions().inputMode;
    if (currentMode != lastInputMode) {
        lastInputMode = currentMode;
        // 連続送信を避けるため、少し待ってから再生
        sleep_ms(50); 
        playJingleByMode();
    }
}

void JinglePlayerAddon::playJingleByMode() {
    // 既存の再生がまだ続いていれば止める
    if (isPlaying()) {
        sendOneLineCommand(0x13); // Stop
        sleep_ms(50); // 停止が完了するまで待つ
    }
    
    sendOneLineCommand(0x0A); // 数値クリア
    sleep_ms(10);
    
    // lastInputMode (0:XInput, 1:Switch...) に 1 を足して 00001.mp3... を再生
    sendOneLineCommand(lastInputMode + 1); 
    sleep_ms(10);
    sendOneLineCommand(0x0B); // 選曲確定・再生
}


bool JinglePlayerAddon::isPlaying() {
    // BUSYピンがHigh(1)なら再生中
    return gpio_get(JINGLE_PLAYER_BUSY_PIN) == 1;
}

void JinglePlayerAddon::sendOneLineCommand(uint8_t addr) {
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
    sleep_us(500); 
}
