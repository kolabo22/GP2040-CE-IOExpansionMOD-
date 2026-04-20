#include "addons/jingle_player.h"
#include "storagemanager.h"
#include "hardware/gpio.h"
#include "hardware/sync.h"
#include "pico/time.h"

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
    
    lastInputMode = (uint8_t)Storage::getInstance().getGamepadOptions().inputMode;
    bootPlayed = false;
}

void JinglePlayerAddon::preprocess() {}
void JinglePlayerAddon::postprocess(bool reportSent) {}
void JinglePlayerAddon::reinit() {}

void JinglePlayerAddon::process() {
    if (!bootPlayed) {
        static uint32_t bootDelay = 0;
        if (bootDelay < 200) {
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

    uint8_t currentMode = (uint8_t)Storage::getInstance().getGamepadOptions().inputMode;
    if (currentMode != lastInputMode) {
        lastInputMode = currentMode;
        sleep_ms(50); 
        playJingleByMode();
    }
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
    sendOneLineCommand(0x0C); // 音量設定
}

void JinglePlayerAddon::playJingleByMode() {
    if (isPlaying()) {
        sendOneLineCommand(0x13); // Stop
        sleep_ms(50);
    }
    
    sendOneLineCommand(0x0A); // クリア
    sleep_ms(10);
    
    // IDをミニメニューの並び順（曲番号）に変換
    uint8_t songNumber = 0;
    if (lastInputMode == 20) {
        songNumber = 20; // コンフィグモードは20曲目
    } else if (lastInputMode <= 18) {
        songNumber = lastInputMode + 1; // それ以外は ID + 1
    }

    if (songNumber > 0) {
        sendOneLineCommand(songNumber); 
        sleep_ms(10);
        sendOneLineCommand(0x0B); // 再生
    }
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
