#include "addons/jingle_player.h"
#include "storagemanager.h"
#include "drivermanager.h"

// スタック破壊を防ぐための静的バッファ
static uint8_t g_uart_buf[] = {0x7E, 0xFF, 0x06, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0xEF};

void JinglePlayerAddon::setup() {
    // 構造体名 jinglePlayerOptions に合わせて設定を取得
    const auto& options = Storage::getInstance().getAddonOptions().jinglePlayerOptions;
    this->volume = (uint8_t)options.volume;
    _hasPlayedOnBoot = false;
    _wasConfigMode = false;

    // JQ8900は GP20(TX) / GP21(RX) なので UART1 を使用
    uart_init(uart1, 9600);
    gpio_set_function(20, GPIO_FUNC_UART); // TX
    gpio_set_function(21, GPIO_FUNC_UART); // RX
}

void JinglePlayerAddon::process() {
    static uint32_t bootDelay = 0;

    if (!_hasPlayedOnBoot) {
        if (bootDelay < 80000) { 
            bootDelay++;
            return;
        }

        bool isConfig = DriverManager::getInstance().isConfigMode();
        setVolume(this->volume);

        if (isConfig) {
            play(21); // 設定モード用：0021.mp3
        } else {
            playSelectedModeJingle(); // 通常機種用
        }
        
        _hasPlayedOnBoot = true;
        _wasConfigMode = isConfig;
    }

    // セーブ後の再起動用
    static uint32_t checkCounter = 0;
    if (checkCounter++ % 10000 == 0) {
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
    g_uart_buf[3] = 0x06;
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
