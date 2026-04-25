#include "addons/jingle_player.h"
#include "storagemanager.h"
#include "drivermanager.h"

void JinglePlayerAddon::setup() {
    // UART初期化
    uart_init(JQ8900_UART, JQ8900_BAUD);
    gpio_set_function(JQ8900_TX_PIN, GPIO_FUNC_UART);
    gpio_set_function(JQ8900_RX_PIN, GPIO_FUNC_UART);

    this->enabled = true;
    this->volume = 15; // WebUI保存ができるまでは15で固定
    
    _hasPlayedOnBoot = false; 
    _wasConfigMode = false;
}

void JinglePlayerAddon::process() {
    bool currentConfigMode = DriverManager::getInstance().isConfigMode();

    // 1. 起動直後の1回だけ実行（setupより後に実行されるため判定が確実）
    if (!_hasPlayedOnBoot) {
        setVolume(this->volume);
        if (currentConfigMode) {
            play(21); // 設定モードなら21番 (0021.mp3)
        } else {
            playSelectedModeJingle(); // 通常なら機種別
        }
        _hasPlayedOnBoot = true;
        _wasConfigMode = currentConfigMode;
        return;
    }

    // 2. 設定モード（WebUI等）から脱出した瞬間の判定
    if (_wasConfigMode && !currentConfigMode) {
        playSelectedModeJingle();
    }
    _wasConfigMode = currentConfigMode;
}

void JinglePlayerAddon::playSelectedModeJingle() {
    InputMode mode = Storage::getInstance().getConfig().gamepadOptions.inputMode;
    uint16_t trackId = 1;

    // リストに完全準拠させた機種別ID（内部ID+1ではない例外もカバー）
    switch (mode) {
        case INPUT_MODE_XINPUT:       trackId = 1;  break; // 0001.mp3
        case INPUT_MODE_SWITCH:       trackId = 2;  break; // 0002.mp3
        case INPUT_MODE_PS3:          trackId = 3;  break; // 0003.mp3
        case INPUT_MODE_PS4:          trackId = 4;  break; // 0004.mp3
        case INPUT_MODE_KEYBOARD:     trackId = 5;  break; // 0005.mp3
        case INPUT_MODE_XBONE:        trackId = 6;  break; // 0006.mp3
        case INPUT_MODE_PS5:          trackId = 7;  break; // 0007.mp3
        case INPUT_MODE_MDMINI:       trackId = 8;  break; // 0008.mp3
        case INPUT_MODE_NEOGEO:       trackId = 10; break; // 0010.mp3
        case INPUT_MODE_PCEMINI:      trackId = 11; break; // 0011.mp3
        case INPUT_MODE_ASTRO:        trackId = 15; break; // 0015.mp3
        case INPUT_MODE_PSCLASSIC:    trackId = 16; break; // 0016.mp3
        case INPUT_MODE_XBOXORIGINAL: trackId = 17; break; // 0017.mp3
        case INPUT_MODE_EGRET:        trackId = 18; break; // 0018.mp3
        case INPUT_MODE_GENERIC:      trackId = 19; break; // 0019.mp3
        default:                      trackId = 1;  break;
    }
    play(trackId);
}

void JinglePlayerAddon::preprocess() {}
void JinglePlayerAddon::postprocess(bool) {}
void JinglePlayerAddon::reinit() {}
bool JinglePlayerAddon::available() { return true; }
std::string JinglePlayerAddon::name() { return "jinglePlayerOptions"; }

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

void JinglePlayerAddon::stop() {
    sendCommand(0x04, nullptr, 0);
}
