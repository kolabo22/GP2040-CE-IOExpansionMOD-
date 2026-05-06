#include "addons/jingle_player.h"
#include "drivermanager.h"
#include "storagemanager.h"

void JinglePlayerAddon::setup() {
    const auto& options = Storage::getInstance().getAddonOptions().jinglePlayerOptions;
    if (!options.enabled) return;

    this->volume = (uint8_t)options.volume;
    this->_hasPlayedOnBoot = false;

    // UART初期化
    uart_init(uart1, 9600);
    gpio_set_function(20, GPIO_FUNC_UART);
    gpio_set_function(21, GPIO_FUNC_UART);

    // S2起動（ConfigMode）の判定
    bool isConfig = DriverManager::getInstance().isConfigMode();

    if (isConfig) {
        // 設定モード：システム安定化を待って21番を再生
        sleep_ms(1500); 
        setVolume(this->volume);
        sleep_ms(50);
        play(21);
    } else {
        // 通常起動：0.8秒待機して機種別音を再生
        sleep_ms(800);
        setVolume(this->volume);
        sleep_ms(50);
        playSelectedModeJingle();
    }

    this->_hasPlayedOnBoot = true;
}

void JinglePlayerAddon::process() {
    // 再生はsetupで完結しているため空でOK
}

void JinglePlayerAddon::playSelectedModeJingle() {
    InputMode mode = DriverManager::getInstance().getInputMode();
    uint16_t track = 1;

    switch (mode) {
        case INPUT_MODE_XINPUT:       track = 1;  break;
        case INPUT_MODE_SWITCH:       track = 2;  break;
        case INPUT_MODE_PS3:          track = 3;  break;
        case INPUT_MODE_PS4:          track = 4;  break;
        case INPUT_MODE_KEYBOARD:     track = 5;  break;
        case INPUT_MODE_XBONE:        track = 6;  break;
        case INPUT_MODE_PS5:          track = 7;  break;
        case INPUT_MODE_MDMINI:       track = 8;  break;
        case INPUT_MODE_NEOGEO:       track = 10; break;
        case INPUT_MODE_PCEMINI:      track = 11; break;
        case INPUT_MODE_SWITCH_PRO:   track = 13; break; 
        case INPUT_MODE_ASTRO:        track = 15; break;
        case INPUT_MODE_PSCLASSIC:    track = 16; break;
        case INPUT_MODE_XBOXORIGINAL: track = 17; break;
        case INPUT_MODE_EGRET:        track = 18; break;
        case INPUT_MODE_GENERIC:      track = 19; break;
        case INPUT_MODE_P5GENERAL:   track = 20; break;
        default:                      track = 1;  break;
    }
    play(track);
}

void JinglePlayerAddon::setVolume(uint8_t volume) {
    if (volume > 30) volume = 30;
    uint8_t cmd = 0x13;
    uint8_t len = 0x01;
    uint8_t sm = (uint8_t)(0xAA + cmd + len + volume);
    uint8_t packet[] = { 0xAA, cmd, len, volume, sm };
    for (int i = 0; i < 5; i++) uart_putc_raw(uart1, packet[i]);
}

void JinglePlayerAddon::play(uint16_t index) {
    uint8_t h = (uint8_t)((index >> 8) & 0xFF);
    uint8_t l = (uint8_t)(index & 0xFF);
    uint8_t cmd = 0x16;
    uint8_t len = 0x03;
    uint8_t device = 0x02; 
    uint8_t sm = (uint8_t)(0xAA + cmd + len + device + h + l);
    uint8_t packet[] = { 0xAA, cmd, len, device, h, l, sm };
    for (int i = 0; i < 7; i++) uart_putc_raw(uart1, packet[i]);
}

void JinglePlayerAddon::reinit() {
    this->_hasPlayedOnBoot = false;
    setup();
}

void JinglePlayerAddon::postprocess(bool reportSent) {}
