#include "addons/jingle_player.h"
#include "storagemanager.h"

void JinglePlayerAddon::setup() {
    // 【重要】セーブデータ肥大化対策：メモリ上の値を強制的に書き換える
    auto& options = Storage::getInstance().getAddonOptions().jinglePlayerOptions;
    options.enabled = true; // 強制有効化

    this->enabled = options.enabled;
    this->volume = (uint8_t)options.volume;
    if (this->volume == 0) this->volume = 15; // 0なら適度な音量に

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

    // --- 再生ロジックの判定 ---
    if (Storage::getInstance().getConfigMode()) { 
        // 1. 設定モードで起動した場合
        play(90); // 設定モード専用 (例: 090.mp3)
        _wasConfigMode = true;
    } else {
        // 2. 通常起動時：保存されている機種設定（InputMode）を見て鳴らし分け
        playSelectedModeJingle();
        _wasConfigMode = false;
    }
}

void JinglePlayerAddon::process() {
    // 3. ミニメニュー（WebUI等）から脱出した瞬間の判定
    bool currentConfigMode = Storage::getInstance().getConfigMode();

    if (_wasConfigMode && !currentConfigMode) {
        // 設定モードから抜けてゲームに戻った瞬間に機種別ジングルを鳴らす
        playSelectedModeJingle();
    }
    _wasConfigMode = currentConfigMode;
}

// 機種（InputMode）ごとの判定ロジック
void JinglePlayerAddon::playSelectedModeJingle() {
    InputMode mode = Storage::getInstance().getConfig().gamepadOptions.inputMode;
    switch (mode) {
        case INPUT_MODE_XINPUT:  play(10); break; // PC (010.mp3)
        case INPUT_MODE_SWITCH:  play(20); break; // Switch (020.mp3)
        case INPUT_MODE_PS4:     play(30); break; // PS4 (030.mp3)
        case INPUT_MODE_PS3:     play(40); break; // PS3 (040.mp3)
        case INPUT_MODE_KEYBOARD:play(50); break; // Keyboard (050.mp3)
        default:                 play(1);  break; // デフォルト (001.mp3)
    }
}

void JinglePlayerAddon::preprocess() {}
void JinglePlayerAddon::postprocess(bool) {}
void JinglePlayerAddon::reinit() {}

bool JinglePlayerAddon::available() {
    // ここも強制的にtrueを返せば、WebUIの設定に関わらずアドオンが動きます
    return true; 
}

std::string JinglePlayerAddon::name() {
    return "jinglePlayerOptions";
}

// --- 通信プロトコル部分は変更なし ---
void JinglePlayerAddon::sendCommand(uint8_t type, uint8_t* data, uint8_t len) {
    uint8_t buf[10];
    buf[0] = 0xAA; buf[1] = type; buf[2] = len;
    uint16_t sum = buf[0] + buf[1] + buf[2];
    for (int i = 0; i < len; i++) { buf[3 + i] = data[i]; sum += data[i]; }
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
