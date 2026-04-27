#include "addons/jingle_player.h"
#include "storagemanager.h"
#include "drivermanager.h"

/* 
 * 修正ポイント:
 * 1. #include "addons/jingle_player.h" を最初に入れ、クラス定義を認識させる
 * 2. DriverManager を使うために "drivermanager.h" を追加
 * 3. メンバ変数（_hasPlayedOnBoot等）にアクセスできるようスコープを正しく指定
 */

void JinglePlayerAddon::setup() {
    const JingleOptions& options = Storage::getInstance().getAddonSettings().jingleOptions;
    this->volume = options.volume;
    _hasPlayedOnBoot = false;
    _wasConfigMode = false;
}

void JinglePlayerAddon::process() {
    static uint32_t bootDelay = 0;

    // 1. 起動直後の判定遅延（S2判定の確実化）
    if (!_hasPlayedOnBoot) {
        if (bootDelay < 200) { // 約200ループ待機（必要に応じて調整）
            bootDelay++;
            return;
        }

        bool isConfig = DriverManager::getInstance().isConfigMode();
        setVolume(this->volume);

        if (isConfig) {
            play(21); // 設定モード起動なら21番を再生
        } else {
            playSelectedModeJingle(); // 通常起動なら機種別Jingle
        }

        _hasPlayedOnBoot = true;
        _wasConfigMode = isConfig;
    }

    // 2. WebUIセーブ後のモード移行検知（Config -> Game）
    bool currentConfig = DriverManager::getInstance().isConfigMode();
    if (_wasConfigMode && !currentConfig) {
        playSelectedModeJingle();
    }
    _wasConfigMode = currentConfig;
}

// 既存の playSelectedModeJingle, play, sendCommand はそのまま維持
void JinglePlayerAddon::playSelectedModeJingle() {
    InputMode mode = DriverManager::getInstance().getInputMode();
    switch (mode) {
        case INPUT_MODE_XINPUT: play(1); break;
        case INPUT_MODE_SWITCH: play(2); break;
        case INPUT_MODE_PS3:    play(3); break;
        case INPUT_MODE_PS4:    play(4); break;
        case INPUT_MODE_PS5:    play(5); break;
        case INPUT_MODE_XBONE:  play(6); break;
        case INPUT_MODE_KEYBOARD: play(7); break;
        default: play(1); break; 
    }
}

void JinglePlayerAddon::setVolume(uint8_t volume) {
    uint8_t buf[10] = {0x7E, 0xFF, 0x06, 0x06, 0x00, 0x00, volume, 0x00, 0x00, 0xEF};
    sendCommand(buf);
}

void JinglePlayerAddon::play(uint16_t index) {
    uint8_t high = (index >> 8) & 0xFF;
    uint8_t low = index & 0xFF;
    uint8_t buf[10] = {0x7E, 0xFF, 0x06, 0x03, 0x00, high, low, 0x00, 0x00, 0xEF};
    sendCommand(buf);
}

void JinglePlayerAddon::sendCommand(uint8_t* buf) {
    for (int i = 0; i < 10; i++) {
        uart_putc(uart0, buf[i]);
    }
}
