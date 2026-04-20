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
    
    // setupでは初期化のみ行い、再生判定はprocess()へ移譲
    bootPlayed = false;
}

void JinglePlayerAddon::preprocess() {}
void JinglePlayerAddon::postprocess(bool reportSent) {}
void JinglePlayerAddon::reinit() {}

void JinglePlayerAddon::process() {
    // 1. 起動直後の再生処理
    if (!bootPlayed) {
        static uint32_t bootDelay = 0;
        // JQ8900が安定するまで、かつS2起動の判定が確定するまでしっかり待つ
        if (bootDelay < 2500) { 
            bootDelay++;
            return;
        }

        // 起動時の正確なIDを取得 (S2起動時はここが20になる)
        lastInputMode = (uint8_t)Storage::getInstance().getGamepadOptions().inputMode;

        const JinglePlayerOptions& options = Storage::getInstance().getAddonOptions().jinglePlayerOptions;
        setVolume((uint8_t)options.volume);
        
        sleep_ms(100);
        playJingleByMode(); // 1回目の再生
        bootPlayed = true;
        return;
    }

    // 2. モード（ID）変更の検知
    uint8_t currentMode = (uint8_t)Storage::getInstance().getGamepadOptions().inputMode;
    
    static uint32_t changeDebounce = 0;
    if (currentMode != lastInputMode) {
        if (changeDebounce < 15000) { // デバウンスを十分に取る
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

void JinglePlayerAddon::setVolume(uint8_t volume) {
    if (volume > 30) volume = 30;
    sendOneLineCommand(0x0A); // クリア
    sleep_ms(20);
    if (volume >= 10) {
        sendOneLineCommand(volume / 10);
        sleep_ms(20);
    }
    sendOneLineCommand(volume % 10);
    sleep_ms(20);
    sendOneLineCommand(0x0C); // 音量確定
}

void JinglePlayerAddon::playJingleByMode() {
    // 再生中の場合は停止
    if (isPlaying()) {
        sendOneLineCommand(0x13); // Stop
        sleep_ms(100);
    }
    
    sendOneLineCommand(0x0A); // 数値クリア
    sleep_ms(30); 
    
    // ロジック：ID + 1 をそのまま曲番号にする（例外なし）
    // ID 0 (XInput) -> 1曲目 / ID 20 (WebConfig) -> 21曲目
    uint8_t songNumber = lastInputMode + 1;

    if (songNumber > 0 && songNumber <= 25) { // 安全のため25番までに制限
        sendOneLineCommand(songNumber); 
        sleep_ms(30);
        sendOneLineCommand(0x0B); // 再生確定
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
