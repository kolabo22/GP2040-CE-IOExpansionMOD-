#include "addons/jingle_player.h"
#include "storagemanager.h"
#include "drivermanager.h"

// JQ8900用：0x7E [サイズ] [コマンド] [パラメータ1] [パラメータ2] [チェックサム] 0xEF
// 実際にはもっとシンプルな形式（0x7E 0x02 0x02 0xEF など）も使えますが、標準的な5バイト形式で構築します
static uint8_t g_uart_buf[8]; 

void JinglePlayerAddon::setup() {
    const auto& options = Storage::getInstance().getAddonOptions().jinglePlayerOptions;
    
    // ★追加：EnabledがOFFなら何もしない
    if (!options.enabled) return;

    this->volume = (uint8_t)options.volume;
    _hasPlayedOnBoot = false;
    _wasConfigMode = false;

    // UART1 (GP20/TX, GP21/RX) の初期化
    uart_init(uart1, 9600);
    gpio_set_function(20, GPIO_FUNC_UART);
    gpio_set_function(21, GPIO_FUNC_UART);
}

void JinglePlayerAddon::process() {
    const auto& options = Storage::getInstance().getAddonOptions().jinglePlayerOptions;
    // ★追加：スイッチがOFFなら処理をスキップ
    if (!options.enabled) return;

    static uint32_t bootDelay = 0;
    if (!_hasPlayedOnBoot) {
        if (bootDelay < 150000) { // JQ8900の起動は遅いので少し長めに待機
            bootDelay++;
            return;
        }

        bool isConfig = DriverManager::getInstance().isConfigMode();
        setVolume(this->volume);
        sleep_ms(10); // コマンド間に少し猶予を置く

        if (isConfig) {
            play(21); // 0021.mp3
        } else {
            playSelectedModeJingle();
        }
        
        _hasPlayedOnBoot = true;
        _wasConfigMode = isConfig;
    }
}

void JinglePlayerAddon::playSelectedModeJingle() {
    InputMode mode = DriverManager::getInstance().getInputMode();
    uint16_t track = 1;
    switch (mode) {
        case INPUT_MODE_XINPUT:   track = 1; break;
        case INPUT_MODE_SWITCH:   track = 2; break;
        case INPUT_MODE_PS3:      track = 3; break;
        case INPUT_MODE_PS4:      track = 4; break;
        case INPUT_MODE_PS5:      track = 5; break;
        case INPUT_MODE_XBONE:    track = 6; break;
        case INPUT_MODE_KEYBOARD: track = 7; break;
        default: track = 1; break;
    }
    play(track);
}

void JinglePlayerAddon::setVolume(uint8_t volume) {
    // コマンド: 0x0C (音量設定)
    sendSimpleCommand(0x0C, volume);
}

void JinglePlayerAddon::play(uint16_t index) {
    // コマンド: 0x07 (指定曲再生)
    // indexをハイバイト・ローバイトに分ける
    uint8_t h = (uint8_t)((index >> 8) & 0xFF);
    uint8_t l = (uint8_t)(index & 0xFF);
    
    uint8_t cmd[] = { 0x7E, 0x04, 0x07, h, l, (uint8_t)(0x7E ^ 0x04 ^ 0x07 ^ h ^ l), 0xEF };
    for(int i=0; i<7; i++) uart_putc_raw(uart1, cmd[i]);
}

// JQ8900用の簡易送信用（4バイト形式：開始、長さ、コマンド、パラメータ、チェックサム、終了）
void JinglePlayerAddon::sendSimpleCommand(uint8_t cmd, uint8_t param) {
    uint8_t checksum = 0x7E ^ 0x03 ^ cmd ^ param;
    uint8_t packet[] = { 0x7E, 0x03, cmd, param, checksum, 0xEF };
    for (int i = 0; i < 6; i++) {
        uart_putc_raw(uart1, packet[i]);
    }
}

void JinglePlayerAddon::sendCommand(uint8_t* buf) {
    // 既存の10バイト固定送信はJQ8900の仕様と異なる可能性があるため、
    // 上記の sendSimpleCommand 形式に移行を推奨
}

void JinglePlayerAddon::postprocess(bool reportSent) {}
void JinglePlayerAddon::reinit() {}
        }
        
        _hasPlayedOnBoot = true;
        _wasConfigMode = isConfig;
    }

    // セーブ後の再起動用
    static uint32_t checkCounter = 0;
    if (checkCounter++ % 10000 == 0) {
        bool currentConfig = DriverManager::getInstance().isConfigMode();
        if (_wasConfigMode && !currentConfig) {
            playSelectedModeJingle();
        }
        _wasConfigMode = currentConfig;
    }
}

void JinglePlayerAddon::playSelectedModeJingle() {
    InputMode mode = DriverManager::getInstance().getInputMode();
    uint16_t track = 1;
    switch (mode) {
        case INPUT_MODE_XINPUT:   track = 1; break;
        case INPUT_MODE_SWITCH:   track = 2; break;
        case INPUT_MODE_PS3:      track = 3; break;
        case INPUT_MODE_PS4:      track = 4; break;
        case INPUT_MODE_PS5:      track = 5; break;
        case INPUT_MODE_XBONE:    track = 6; break;
        case INPUT_MODE_KEYBOARD: track = 7; break;
        default: track = 1; break;
    }
    play(track);
}

void JinglePlayerAddon::setVolume(uint8_t volume) {
    g_uart_buf[3] = 0x06;
    g_uart_buf[6] = volume;
    sendCommand(g_uart_buf);
}

void JinglePlayerAddon::play(uint16_t index) {
    g_uart_buf[3] = 0x03;
    g_uart_buf[5] = (uint8_t)((index >> 8) & 0xFF);
    g_uart_buf[6] = (uint8_t)(index & 0xFF);
    sendCommand(g_uart_buf);
}

void JinglePlayerAddon::sendCommand(uint8_t* buf) {
    for (int i = 0; i < 10; i++) {
        uart_putc_raw(uart1, buf[i]); // UART1を使用
    }
}

void JinglePlayerAddon::postprocess(bool reportSent) {}
void JinglePlayerAddon::reinit() {}
