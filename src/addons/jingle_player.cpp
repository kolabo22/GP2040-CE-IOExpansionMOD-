#include "addons/jingle_player.h"
#include "storagemanager.h"

void JinglePlayerAddon::setup() {
    // 確実に存在する型名を参照
    const JinglePlayerOptions& options = Storage::getInstance().getAddonOptions().jinglePlayerOptions;
    this->enabled = options.enabled;
    this->volume = (uint8_t)options.volume;

    // UART初期化
    uart_init(JQ8900_UART, JQ8900_BAUD);
    gpio_set_function(JQ8900_TX_PIN, GPIO_FUNC_UART);
    gpio_set_function(JQ8900_RX_PIN, GPIO_FUNC_UART);
    uart_set_format(JQ8900_UART, 8, 1, UART_PARITY_NONE);

    if (!this->enabled) return;

    setVolume(this->volume);
    sleep_ms(100);

    // まずは動作確認のため、設定されたIDを再生（Config判定はビルド成功後に再構築）
    uint16_t idToPlay = (options.selectedId > 0) ? (uint16_t)options.selectedId : 1;
    play(idToPlay);
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
