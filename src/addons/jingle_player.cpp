#include "addons/jingle_player.h"
#include "storagemanager.h"
#include "drivermanager.h"

void JinglePlayerAddon::setup() {
    this->volume = 20; 
    _hasPlayedOnBoot = false;
    _wasConfigMode = false;

    // UART初期化 (UART0, 9600bps)
    uart_init(uart0, 9600);
    gpio_set_function(0, GPIO_FUNC_UART); // TX: GP0
    gpio_set_function(1, GPIO_FUNC_UART); // RX: GP1
}

void JinglePlayerAddon::process() {
    static uint32_t bootCounter = 0;

    // 1. 起動時の再生（ここが一番大事）
    if (!_hasPlayedOnBoot) {
        bootCounter++;
        // 起動直後、少し余裕を持ってから1回だけ実行
        if (bootCounter == 20000) { 
            bool isConfig = DriverManager::getInstance().isConfigMode();
            setVolume(this->volume);
            sleep_ms(10); // JQ8900の準備待ち

            if (isConfig) {
                play(21); // 設定モード
            } else {
                playSelectedModeJingle(); // 機種別
            }
            _hasPlayedOnBoot = true;
            _wasConfigMode = isConfig;
        }
        return; 
    }

    // 2. モード切り替え検知（毎秒10回程度に抑制して負荷軽減）
    static uint32_t lastCheck = 0;
    uint32_t now = to_ms_since_boot(get_absolute_time());
    if (now - lastCheck > 100) {
        bool currentConfig = DriverManager::getInstance().isConfigMode();
        if (_wasConfigMode && !currentConfig) {
            // WebUIでSaveを押して再起動がかかった直後の鳴らし直し
            playSelectedModeJingle();
        }
        _wasConfigMode = currentConfig;
        lastCheck = now;
    }
}

void JinglePlayerAddon::playSelectedModeJingle() {
    InputMode mode = DriverManager::getInstance().getInputMode();
    uint16_t track = 1;
    switch (mode) {
        case INPUT_MODE_XINPUT:   track = 1; break;
        case INPUT_MODE_SWITCH:   track = 2; break;
        case INPUT_MODE_PS3:      track = 3; break;
        case INPUT_MODE_PS4:      track = 4; break;
        case INPUT_MODE_PS5:      track = 5; break;
        case INPUT_MODE_XBONE:    track = 6; break;
        case INPUT_MODE_KEYBOARD: track = 7; break;
        default: track = 1; break;
    }
    play(track);
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

void JinglePlayerAddon::sendCommand(uint8_t* buf) {
    for (int i = 0; i < 10; i++) {
        uart_putc(uart0, buf[i]);
    }
    // 送信後に一瞬だけ待つ（連続送信時の衝突防止）
    sleep_ms(2);
}
