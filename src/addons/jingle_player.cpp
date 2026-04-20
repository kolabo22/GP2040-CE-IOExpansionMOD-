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

void JinglePlayerAddon::setup() {
    gpio_init(JINGLE_PLAYER_VPP_PIN);
    gpio_set_dir(JINGLE_PLAYER_VPP_PIN, GPIO_OUT);
    gpio_put(JINGLE_PLAYER_VPP_PIN, 1);

    gpio_init(JINGLE_PLAYER_BUSY_PIN);
    gpio_set_dir(JINGLE_PLAYER_BUSY_PIN, GPIO_IN);
    gpio_pull_up(JINGLE_PLAYER_BUSY_PIN);

    // 設定値の取得
    const JinglePlayerOptions& options = Storage::getInstance().getAddonOptions().jinglePlayerOptions;
    uint8_t volume = (uint8_t)options.volume;
    
    // 初期モード保持 (ConfigManager ではなく Storage を使用)
    lastInputMode = (uint8_t)Storage::getInstance().getGamepadOptions().inputMode;

    // 起動時の初期化シーケンス
    //setVolume(volume);
    //playJingleByMode();
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
    uint32_t status = save_and_disable_interrupts();
    
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
    restore_interrupts(status);
}

void JinglePlayerAddon::reinit() {
    // 空でOK
}
