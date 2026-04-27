#include "addons/jingle_player.h"
#include "storagemanager.h"
#include "drivermanager.h"

// 静的バッファでメモリを安定化
static uint8_t g_uart_buf[10] = {0x7E, 0xFF, 0x06, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0xEF};

void JinglePlayerAddon::setup() {
    this->volume = 15; 
    _hasPlayedOnBoot = false;
    _wasConfigMode = false;

    // GP20/21は「UART1」です。ここを間違えるとフリーズします。
    uart_init(uart1, 9600);
    gpio_set_function(20, GPIO_FUNC_UART); // TX: GP20
    gpio_set_function(21, GPIO_FUNC_UART); // RX: GP21
}

void JinglePlayerAddon::process() {
    static uint32_t bootDelay = 0;

    if (!_hasPlayedOnBoot) {
        if (bootDelay < 50000) { 
            bootDelay++;
            return;
        }

        bool isConfig = DriverManager::getInstance().isConfigMode();
        setVolume(this->volume);

        if (isConfig) {
            play(21); // 設定モード
        } else {
            playSelectedModeJingle(); // 通常モード
        }
        
        _hasPlayedOnBoot = true;
        _wasConfigMode = isConfig;
    }
}

void JinglePlayerAddon::playSelectedModeJingle() {
    InputMode mode = DriverManager::getInstance().getInputMode();
    uint16_t track = 1;
    if (mode == INPUT_MODE_XINPUT) track = 1;
    else if (mode == INPUT_MODE_SWITCH) track = 2;
    else if (mode == INPUT_MODE_PS3) track = 3;
    else if (mode == INPUT_MODE_PS4) track = 4;
    else if (mode == INPUT_MODE_PS5) track = 5;
    else if (mode == INPUT_MODE_XBONE) track = 6;
    else if (mode == INPUT_MODE_KEYBOARD) track = 7;
    play(track);
}

void JinglePlayerAddon::setVolume(uint8_t volume) {
    g_uart_buf[3] = 0x06;
    g_uart_buf[5] = 0x00;
    g_uart_buf[6] = volume;
    sendCommand(g_uart_buf);
}

void JinglePlayerAddon::play(uint16_t index) {
    g_uart_buf[3] = 0x03;
    g_uart_buf[5] = (uint8_t)((index >> 8) & 0xFF);
    g_uart_buf[6] = (uint8_t)(index & 0xFF);
    sendCommand(g_uart_buf);
}

void JinglePlayerAddon::sendCommand(uint8_t* buf) {
    for (int i = 0; i < 10; i++) {
        uart_putc_raw(uart1, buf[i]); // UART1を使用
    }
}

void JinglePlayerAddon::postprocess(bool reportSent) {}
void JinglePlayerAddon::reinit() {}
