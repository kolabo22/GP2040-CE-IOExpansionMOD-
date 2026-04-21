#include "addons/jingle_player.h"
#include "storagemanager.h"

void JinglePlayerAddon::setup() {
    const JinglePlayerOptions& options = Storage::getInstance().getAddonOptions().jinglePlayerOptions;
    this->enabled = options.enabled;
    this->volume = (uint8_t)options.volume;

    // --- UART1の初期化をより確実に ---
    uart_init(JQ8900_UART, JQ8900_BAUD);
    
    // GPIO20(TX), GPIO21(RX)にUART機能を割り当て
    gpio_set_function(JQ8900_TX_PIN, GPIO_FUNC_UART);
    gpio_set_function(JQ8900_RX_PIN, GPIO_FUNC_UART);

    // 有効でない場合は終了
    if (!this->enabled) return;

    // モジュールの起動待ち（長めに設定）
    sleep_ms(1000); 

    // 音量設定
    setVolume(this->volume);
    sleep_ms(200);

    // 起動テスト再生 (selectedIdが正しく読めていない可能性を考慮し、強制的に1番を再生)
    play(1); 
}

void JinglePlayerAddon::preprocess() {}
void JinglePlayerAddon::process() {}
void JinglePlayerAddon::postprocess(bool) {}
void JinglePlayerAddon::reinit() {}

bool JinglePlayerAddon::available() {
    const JinglePlayerOptions& options = Storage::getInstance().getAddonOptions().jinglePlayerOptions;
    return options.enabled;
}

std::string JinglePlayerAddon::name() {
    return "JinglePlayer";
}

void JinglePlayerAddon::sendCommand(uint8_t type, uint8_t* data, uint8_t len) {
    uint8_t buf[10]; // 配列サイズを固定
    buf[0] = 0xAA;
    buf[1] = type;
    buf[2] = len;
    uint16_t sum = buf[0] + buf[1] + buf[2];
    for (int i = 0; i < len; i++) {
        buf[3 + i] = data[i];
        sum += data[i];
    }
    buf[3 + len] = (uint8_t)(sum & 0xFF);
    
    // 送信
    uart_write_blocking(JQ8900_UART, buf, len + 4);
}

void JinglePlayerAddon::setVolume(uint8_t volume) {
    if (volume > 30) volume = 30;
    uint8_t d[1] = {volume}; // 配列として宣言
    sendCommand(0x13, d, 1);
}

void JinglePlayerAddon::play(uint16_t trackId) {
    // enabledチェックを外し、テストのために必ず送信するようにします
    uint8_t d[2] = { (uint8_t)(trackId >> 8), (uint8_t)(trackId & 0xFF) };
    sendCommand(0x07, d, 2);
}

void JinglePlayerAddon::stop() {
    sendCommand(0x04, nullptr, 0);
}
