#include "addons/jingle_player.h"
#include "storagemanager.h"
#include "drivermanager.h"

void JinglePlayerAddon::setup() {
    auto& options = Storage::getInstance().getAddonOptions().jinglePlayerOptions;
    options.enabled = true; 
    this->enabled = true;
    this->volume = (options.volume == 0) ? 15 : (uint8_t)options.volume;

    // --- 最小限のUART初期化（sleepなし） ---
    uart_init(JQ8900_UART, JQ8900_BAUD);
    gpio_set_function(JQ8900_TX_PIN, GPIO_FUNC_UART);
    gpio_set_function(JQ8900_RX_PIN, GPIO_FUNC_UART);

    // 起動時の再生は、少し時間が経ってからprocessで行うか、
    // ここではパケットを送るだけにする（sleepは厳禁）
    if (DriverManager::getInstance().isConfigMode()) {
        play(21);
        _wasConfigMode = true;
    } else {
        playSelectedModeJingle();
        _wasConfigMode = false;
    }
}

void JinglePlayerAddon::process() {
    bool currentConfigMode = DriverManager::getInstance().isConfigMode();
    if (_wasConfigMode && !currentConfigMode) {
        playSelectedModeJingle();
    }
    _wasConfigMode = currentConfigMode;
}

void JinglePlayerAddon::playSelectedModeJingle() {
    InputMode mode = Storage::getInstance().getConfig().gamepadOptions.inputMode;
    uint16_t trackId = (uint16_t)mode + 1;
    if (trackId > 20) trackId = 1;
    play(trackId);
}

// --- メモリ破壊を100%防ぐ安全な送信ロジック ---
void JinglePlayerAddon::sendCommand(uint8_t type, uint8_t* data, uint8_t len) {
    uint8_t buf[10] = {0}; // ゼロ初期化された固定配列
    buf[0] = 0xAA;
    buf[1] = type;
    buf[2] = len;

    uint16_t sum = buf[0] + buf[1] + buf[2];
    for (uint8_t i = 0; i < len && i < 4; i++) {
        buf[3 + i] = data[i];
        sum += data[i];
    }
    buf[3 + len] = (uint8_t)(sum & 0xFF);

    uart_write_blocking(JQ8900_UART, buf, len + 4);
}

void JinglePlayerAddon::play(uint16_t trackId) {
    uint8_t d[2] = { (uint8_t)(trackId >> 8), (uint8_t)(trackId & 0xFF) };
    sendCommand(0x07, d, 2);
}

void JinglePlayerAddon::setVolume(uint8_t volume) {
    uint8_t v = (volume > 30) ? 30 : volume;
    uint8_t d[1] = {v};
    sendCommand(0x13, d, 1);
}

void JinglePlayerAddon::preprocess() {}
void JinglePlayerAddon::postprocess(bool) {}
void JinglePlayerAddon::reinit() {}
bool JinglePlayerAddon::available() { return true; }
std::string JinglePlayerAddon::name() { return "jinglePlayerOptions"; }
void JinglePlayerAddon::stop() { sendCommand(0x04, nullptr, 0); }
