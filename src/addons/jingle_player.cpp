#include "addons/jingle_player.h"
#include "storagemanager.h"
#include "drivermanager.h"

void JinglePlayerAddon::setup() {
    const JingleOptions& options = Storage::getInstance().getAddonSettings().jingleOptions;
    this->volume = options.volume;
    _hasPlayedOnBoot = false;
    _wasConfigMode = false;

    // UART初期化 (UART0, 9600bps)
    uart_init(uart0, 9600);
    gpio_set_function(0, GPIO_FUNC_UART); // TX: GP0
    gpio_set_function(1, GPIO_FUNC_UART); // RX: GP1
}

void JinglePlayerAddon::process() {
    static uint32_t bootDelay = 0;

    if (!_hasPlayedOnBoot) {
        if (bootDelay < 1000) { // 起動直後の安定待ち
            bootDelay++;
            return;
        }

        bool isConfig = DriverManager::getInstance().isConfigMode();
        setVolume(this->volume);

        if (isConfig) {
            play(21); // 設定モード(S2押しながら)なら21番
        } else {
            playSelectedModeJingle(); // 通常は機種別
        }

        _hasPlayedOnBoot = true;
        _wasConfigMode = isConfig;
    }

    // WebUIでのセーブ反映用
    bool currentConfig = DriverManager::getInstance().isConfigMode();
    if (_wasConfigMode && !currentConfig) {
        playSelectedModeJingle();
    }
    _wasConfigMode = currentConfig;
}

void JinglePlayerAddon::playSelectedModeJingle() {
    InputMode mode = DriverManager::getInstance().getInputMode();
    switch (mode) {
        case INPUT_MODE_XINPUT:   play(1); break;
        case INPUT_MODE_SWITCH:   play(2); break;
        case INPUT_MODE_PS3:      play(3); break;
        case INPUT_MODE_PS4:      play(4); break;
        case INPUT_MODE_PS5:      play(5); break;
        case INPUT_MODE_XBONE:    play(6); break;
        case INPUT_MODE_KEYBOARD: play(7); break;
        default: play(1); break;
    }
}

void JinglePlayerAddon::setVolume(uint8_t volume) {
    uint8_t buf[10] = {0x7E, 0xFF, 0x06, 0x06, 0x00, 0x00, volume, 0x00, 0x00, 0xEF};
    sendCommand(buf);
}

void JinglePlayerAddon::play(uint16_t index) {
    uint8_t high = (index >> 8) & 0xFF;
    uint8_t low = index & 0xFF;
    uint8_t buf[10] = {0x7E, 0xFF, 0x06, 0x03, 0x00, high, low, 0x00, 0x00, 0xEF};
    sendCommand(buf);
}

void JinglePlayerAddon::sendCommand(uint8_t buf[10]) {
    for (int i = 0; i < 10; i++) {
        uart_putc(uart0, buf[i]);
    }
}
