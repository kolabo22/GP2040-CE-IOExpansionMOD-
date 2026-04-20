#include "addons/jingle_player.h"
#include "storagemanager.h"
#include "hardware/gpio.h"
#include "hardware/sync.h"
#include "pico/time.h"

// ピン定義
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
    
    // 起動時の初期モードを保持
    lastInputMode = (uint8_t)Storage::getInstance().getGamepadOptions().inputMode;
    bootPlayed = false;

    // S2ボタン(WebConfigモード)での起動を検知
    if (Storage::getInstance().getConfigMode()) {
        lastInputMode = 20; // 内部的に20(Config)として扱う
    }
}

void JinglePlayerAddon::preprocess() {}
void JinglePlayerAddon::postprocess(bool reportSent) {}
void JinglePlayerAddon::reinit() {}

void JinglePlayerAddon::process() {
    // 1. 起動直後の再生
    if (!bootPlayed) {
        static uint32_t bootDelay = 0;
        if (bootDelay < 500) { 
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
    static uint32_t changeDebounce = 0;
    if (currentMode != lastInputMode) {
        if (changeDebounce < 2000) { // ループ回数でデバウンス
            changeDebounce++;
        } else {
            lastInputMode = currentMode;
            playJingleByMode();
            changeDebounce = 0;
        }
    } else {
        changeDebounce = 2000; 
    }
}

void JinglePlayerAddon::setVolume(uint8_t volume) {
    if (volume > 30) volume = 30;
    sendOneLineCommand(0x0A); // クリア
    sleep_ms(10);
    if (volume >= 10) {
        sendOneLineCommand(volume / 10);
        sleep_ms(10);
    }
    sendOneLineCommand(volume % 10);
    sleep_ms(10);
    sendOneLineCommand(0x0C); // 音量設定
}

void JinglePlayerAddon::playJingleByMode() {
    if (isPlaying()) {
        sendOneLineCommand(0x13); // Stop
        sleep_ms(100);
    }
    
    sendOneLineCommand(0x0A); // クリア
    sleep_ms(20);
    
    uint8_t songNumber = 0;
    if (lastInputMode == 20) {
        songNumber = 20; // Config音
    } else if (lastInputMode <= 18) {
        songNumber = lastInputMode + 1; // モード曲
    }

    if (songNumber > 0) {
        sendOneLineCommand(songNumber); 
        sleep_ms(20);
        sendOneLineCommand(0x0B); // 再生
    }
}

bool JinglePlayerAddon::isPlaying() {
    return gpio_get(JINGLE_PLAYER_BUSY_PIN) == 1;
}

void JinglePlayerAddon::sendOneLineCommand(uint8_t addr) {
    gpio_put(JINGLE_PLAYER_VPP_PIN, 0);
    sleep_us(4000);
    for (int i = 0; i < 8; i++) {
        gpio_put(JINGLE_PLAYER_VPP_PIN, 1);
        if (addr & 0x01) {
            sleep_us(600);
            gpio_put(JINGLE_PLAYER_VPP_PIN, 0);
            sleep_us(200);
        } else {
            sleep_us(200);
            gpio_put(JINGLE_PLAYER_VPP_PIN, 0);
            sleep_us(600);
        }
        addr >>= 1;
    }
    gpio_put(JINGLE_PLAYER_VPP_PIN, 1);
    sleep_us(500); 
}
