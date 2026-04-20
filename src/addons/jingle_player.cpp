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
    // GPIO初期化
    gpio_init(JINGLE_PLAYER_VPP_PIN);
    gpio_set_dir(JINGLE_PLAYER_VPP_PIN, GPIO_OUT);
    gpio_put(JINGLE_PLAYER_VPP_PIN, 1); // Idle High

    gpio_init(JINGLE_PLAYER_BUSY_PIN);
    gpio_set_dir(JINGLE_PLAYER_BUSY_PIN, GPIO_IN);
    gpio_pull_up(JINGLE_PLAYER_BUSY_PIN);
    
    bootPlayed = false;
}

void JinglePlayerAddon::preprocess() {
    // 抽象クラスエラー回避
}

void JinglePlayerAddon::process() {
    // 1. 起動直後の1回限りの再生処理
    if (!bootPlayed) {
        const JinglePlayerOptions& options = Storage::getInstance().getAddonOptions().jinglePlayerOptions;
        setVolume((uint8_t)options.volume);
        
        // 現在のモードを初期値として保持
        lastInputMode = (uint8_t)Storage::getInstance().getGamepadOptions().inputMode;
        
        sleep_ms(10);
        playJingleByMode();
        bootPlayed = true;
        return;
    }

    // 2. モード（機種）変更の検知
    uint8_t currentMode = (uint8_t)Storage::getInstance().getGamepadOptions().inputMode;
    if (currentMode != lastInputMode) {
        lastInputMode = currentMode;
        playJingleByMode();
    }
}

void JinglePlayerAddon::postprocess(bool reportSent) {
    // 抽象クラスエラー回避
}

void JinglePlayerAddon::reinit() {
    // 抽象クラスエラー回避
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
