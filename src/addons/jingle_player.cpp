#include "addons/jingle_player.h"
#include "storagemanager.h"

void JinglePlayer::setup() {
    // 1. WebConfig（Storage）から保存されている設定を取得
    const JingleOptions& options = Storage::getInstance().getAddonOptions().jingleOptions;
    this->enabled = options.enabled;
    this->volume = options.volume;

    // 2. UARTの初期化 (GP20/21)
    uart_init(JQ8900_UART, JQ8900_BAUD);
    gpio_set_function(JQ8900_TX_PIN, GPIO_FUNC_UART);
    gpio_set_function(JQ8900_RX_PIN, GPIO_FUNC_UART);
    uart_set_format(JQ8900_UART, 8, 1, UART_PARITY_NONE);

    // 3. 有効設定なら初期音量を送信し、起動音(0001.mp3)を鳴らす
    if (this->enabled) {
        setVolume(this->volume);
        sleep_ms(100); // JQ8900の起動・通信安定待ち
        play(1);      // テスト再生: 0001.mp3
    }
}

// アドオンとして必須の関数（空でOK）
void JinglePlayer::preprocess() {
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
    // 有効でない場合は送信しない
    if (!this->enabled) return;
    
    uint8_t d = {(uint8_t)(trackId >> 8), (uint8_t)(trackId & 0xFF)};
    sendCommand(0x07, d, 2);
}

void JinglePlayer::stop() {
    sendCommand(0x04, nullptr, 0);
}
