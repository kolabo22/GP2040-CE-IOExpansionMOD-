#include "addons/jingle_player.h"
#include "storagemanager.h"
#include "enums.pb.h" // BootModeの定義が含まれる場所

// 外部で定義されている起動モード変数を参照
extern ConfigMode currentConfigMode;

void JinglePlayerAddon::setup() {
    const JinglePlayerOptions& options = Storage::getInstance().getAddonOptions().jinglePlayerOptions;
    this->enabled = options.enabled;
    this->volume = (uint8_t)options.volume;

    uart_init(JQ8900_UART, JQ8900_BAUD);
    gpio_set_function(JQ8900_TX_PIN, GPIO_FUNC_UART);
    gpio_set_function(JQ8900_RX_PIN, GPIO_FUNC_UART);
    uart_set_format(JQ8900_UART, 8, 1, UART_PARITY_NONE);

    if (!this->enabled) return;

    setVolume(this->volume);
    sleep_ms(100);

    // Configモード（S2起動）かどうかの判定
    if (currentConfigMode != ConfigMode::CONFIG_MODE_NONE) {
        play(21); // Config用 (0021.mp3)
    } else {
        uint16_t idToPlay = (options.selectedId > 0) ? (uint16_t)options.selectedId : 1;
        play(idToPlay); // 通常起動用
    }
}

void JinglePlayerAddon::preprocess() {}
void JinglePlayerAddon::process() {}

void JinglePlayerAddon::sendCommand(uint8_t type, uint8_t* data, uint8_t len) {
    uint8_t buf[10];
    buf[0] = 0xAA;
    buf[1] = type;
    buf[2] = len;
    uint16_t sum = buf[0] + buf[1] + buf[2];
    for (int i = 0; i < len; i++) {
        buf[3 + i] = data[i];
        sum += data[i];
    }
    buf[3 + len] = (uint8_t)(sum & 0xFF);
    uart_write_blocking(JQ8900_UART, buf, len + 4);
}

void JinglePlayerAddon::setVolume(uint8_t volume) {
    if (volume > 30) volume = 30;
    uint8_t d[1] = {volume};
    sendCommand(0x13, d, 1);
}

void JinglePlayerAddon::play(uint16_t trackId) {
    if (!this->enabled) return;
    uint8_t d[2] = { (uint8_t)(trackId >> 8), (uint8_t)(trackId & 0xFF) };
    sendCommand(0x07, d, 2);
}

void JinglePlayerAddon::stop() {
    sendCommand(0x04, nullptr, 0);
}
