#include "addons/jingle_player.h"
#include "storagemanager.h"
#include "drivermanager.h"

void JinglePlayerAddon::setup() {
    // 1. メモリ上の値を読み込み（セーブが成功していれば反映される）
    auto& options = Storage::getInstance().getAddonOptions().jinglePlayerOptions;
    this->enabled = options.enabled;
    this->volume = (uint8_t)options.volume;
    
    // セーブエラー対策：options.enabledがfalseでも、コード上で強制的に動かす
    this->enabled = true; 
    if (this->volume == 0) this->volume = 15; // 初期値0なら15に設定

    // 2. UART初期化（sleepなしの安全版）
    uart_init(JQ8900_UART, JQ8900_BAUD);
    gpio_set_function(JQ8900_TX_PIN, GPIO_FUNC_UART);
    gpio_set_function(JQ8900_RX_PIN, GPIO_FUNC_UART);

    // 3. 音量設定
    setVolume(this->volume);

    // 4. 【最優先】設定モード判定
    // 判定順序を入れ替え、ConfigModeなら即座に21番を鳴らす
    if (DriverManager::getInstance().isConfigMode()) {
        play(21); // 設定モード用 (0021.mp3)
        _wasConfigMode = true;
    } else {
        // 通常起動なら機種別を再生
        playSelectedModeJingle();
        _wasConfigMode = false;
    }
}

void JinglePlayerAddon::process() {
    bool currentConfigMode = DriverManager::getInstance().isConfigMode();

    // 設定画面（WebUI）を閉じてゲーム画面に戻った瞬間の判定
    if (_wasConfigMode && !currentConfigMode) {
        playSelectedModeJingle();
    }
    _wasConfigMode = currentConfigMode;
}

void JinglePlayerAddon::playSelectedModeJingle() {
    InputMode mode = Storage::getInstance().getConfig().gamepadOptions.inputMode;
    uint16_t trackId = (uint16_t)mode + 1;

    // リスト外チェック
    if (trackId > 20) trackId = 1;

    play(trackId);
}

// --- 通信プロトコル（安全な固定配列版） ---
void JinglePlayerAddon::sendCommand(uint8_t type, uint8_t* data, uint8_t len) {
    uint8_t buf[10] = {0};
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
