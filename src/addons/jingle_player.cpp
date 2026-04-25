#include "addons/jingle_player.h"
#include "storagemanager.h"
#include "drivermanager.h" // 判定のために追加

void JinglePlayerAddon::setup() {
    // 【重要】WebUIの保存エラー対策：メモリ上の値を強制的に有効化
    auto& options = Storage::getInstance().getAddonOptions().jinglePlayerOptions;
    options.enabled = true; 

    this->enabled = options.enabled;
    this->volume = (uint8_t)options.volume;
    if (this->volume == 0) this->volume = 15; 

    // UART1 (GP20/21) の初期化
    uart_init(JQ8900_UART, JQ8900_BAUD);
    gpio_set_function(JQ8900_TX_PIN, GPIO_FUNC_UART);
    gpio_set_function(JQ8900_RX_PIN, GPIO_FUNC_UART);
    uart_set_format(JQ8900_UART, 8, 1, UART_PARITY_NONE);

    // JQ8900の起動準備待ち
    sleep_ms(1000); 

    // 音量設定
    setVolume(this->volume);
    sleep_ms(200);

    // 起動時の判定（DriverManagerを使用するように修正）
    if (DriverManager::getInstance().isConfigMode()) { 
        // 1. 設定モードで起動
        play(21); // 0021.mp3 を再生
        _wasConfigMode = true;
    } else {
        // 2. 通常起動
        playSelectedModeJingle();
        _wasConfigMode = false;
    }
}

void JinglePlayerAddon::process() {
    // 設定モード（WebUI等）から脱出した瞬間の判定（DriverManagerを使用するように修正）
    bool currentConfigMode = DriverManager::getInstance().isConfigMode();

    if (_wasConfigMode && !currentConfigMode) {
        // 設定画面を閉じてゲームに戻った瞬間に機種別ジングルを鳴らす
        playSelectedModeJingle();
    }
    _wasConfigMode = currentConfigMode;
}

// 機種（InputMode）ごとのID計算と再生
void JinglePlayerAddon::playSelectedModeJingle() {
    InputMode mode = Storage::getInstance().getConfig().gamepadOptions.inputMode;

    // リスト準拠：内部ID + 1 が MP3ファイル番号
    uint16_t trackId = (uint16_t)mode + 1;

    // 範囲外チェック
    if (trackId > 20) trackId = 1;

    play(trackId);
}

void JinglePlayerAddon::preprocess() {}
void JinglePlayerAddon::postprocess(bool) {}
void JinglePlayerAddon::reinit() {}

bool JinglePlayerAddon::available() {
    return true; 
}

std::string JinglePlayerAddon::name() {
    return "jinglePlayerOptions";
}

// UARTプロトコル送信
void JinglePlayerAddon::sendCommand(uint8_t type, uint8_t* data, uint8_t len) {
    uint8_t buf[10]; // サイズを明示的に確保
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
    uint8_t v = (volume > 30) ? 30 : volume;
    uint8_t d[1] = {v};
    sendCommand(0x13, d, 1);
}

void JinglePlayerAddon::play(uint16_t trackId) {
    uint8_t d[2] = { (uint8_t)(trackId >> 8), (uint8_t)(trackId & 0xFF) };
    sendCommand(0x07, d, 2);
}

void JinglePlayerAddon::stop() {
    sendCommand(0x04, nullptr, 0);
}
