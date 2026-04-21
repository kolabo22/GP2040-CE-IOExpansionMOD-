#include "jingle_player.h"

void JinglePlayer::setup() {
    uart_init(JQ8900_UART, JQ8900_BAUD);
    gpio_set_function(JQ8900_TX_PIN, GPIO_FUNC_UART);
    gpio_set_function(JQ8900_RX_PIN, GPIO_FUNC_UART);
    uart_set_format(JQ8900_UART, 8, 1, UART_PARITY_NONE);
}

void JinglePlayer::sendCommand(uint8_t type, uint8_t* data, uint8_t len) {
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

void JinglePlayer::setVolume(uint8_t volume) {
    if (volume > 30) volume = 30;
    uint8_t d = {volume};
    sendCommand(0x13, d, 1);
}

void JinglePlayer::play(uint16_t trackId) {
    uint8_t d = {(uint8_t)(trackId >> 8), (uint8_t)(trackId & 0xFF)};
    sendCommand(0x07, d, 2);
}

void JinglePlayer::stop() {
    sendCommand(0x04, nullptr, 0);
}
