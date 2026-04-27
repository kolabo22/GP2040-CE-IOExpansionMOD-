#include "addons/jingle_player.h"
#include "storagemanager.h"
#include "drivermanager.h"

void JinglePlayerAddon::setup() {
    this->volume = 15; 
    _hasPlayedOnBoot = false;
    _wasConfigMode = false;

    // UART初期化（ブロッキングを防ぐため設定のみ確実に）
    uart_init(uart0, 9600);
    gpio_set_function(0, GPIO_FUNC_UART); // TX: GP0
    gpio_set_function(1, GPIO_FUNC_UART); // RX: GP1
}

void JinglePlayerAddon::process() {
    // 起動時の待機をカウントアップのみで制御（sleep_msは絶対に使わない）
    static uint32_t bootDelayCounter = 0;

    if (!_hasPlayedOnBoot) {
        bootDelayCounter++;
        // 50000回ほどループを回してから実行（フリーズ回避のための時間稼ぎ）
        if (bootDelayCounter >= 50000) {
            bool isConfig = DriverManager::getInstance().isConfigMode();
            
            // 直接コマンド送信（sleepなし）
            setVolume(this->volume);
            
            if (isConfig) {
                play(21); 
            } else {
                playSelectedModeJingle();
            }
            
            _hasPlayedOnBoot = true;
            _wasConfigMode = isConfig;
        }
        return; 
    }

    // 2. モード移行監視（非常に軽く実行）
    static uint32_t checkCounter = 0;
    if (checkCounter++ % 10000 == 0) { // 頻度を下げる
        bool currentConfig = DriverManager::getInstance().isConfigMode();
        if (_wasConfigMode && !currentConfig) {
            playSelectedModeJingle();
        }
        _wasConfigMode = currentConfig;
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
        // uart_putc の代わりに uart_is_writable を確認してブロッキングを回避
        if (uart_is_writable(uart0)) {
            uart_putc_raw(uart0, buf[i]);
        }
    }
}
