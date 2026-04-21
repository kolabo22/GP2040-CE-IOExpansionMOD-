#include "addons/jingle_player.h"
#include "storagemanager.h"
#include "system.h"

void JinglePlayer::setup() {
    // 1. StorageからJingle設定（有効/無効、音量、選択ID）を読み込む
    const JingleOptions& options = Storage::getInstance().getAddonOptions().jingleOptions;
    this->enabled = options.enabled;
    this->volume = options.volume;

    // 2. UART1 (GP20/21) の初期化 (9600bps, 8N1)
    uart_init(JQ8900_UART, JQ8900_BAUD);
    gpio_set_function(JQ8900_TX_PIN, GPIO_FUNC_UART);
    gpio_set_function(JQ8900_RX_PIN, GPIO_FUNC_UART);
    uart_set_format(JQ8900_UART, 8, 1, UART_PARITY_NONE);

    // 有効設定でない場合は何もしない
    if (!this->enabled) return;

    // 3. JQ8900の音量設定と安定待ち
    setVolume(this->volume);
    sleep_ms(100);

    // --- 起動時の再生ロジック判定 ---

    // ② S2ボタンを押しながら電源投入（Configモード）の場合
    if (System::getBootMode() == BootMode::CONFIG) {
        // 曲番21 (0021.mp3) を再生
        play(21);
    } 
    // ① 通常の電源投入時の場合
    else {
        // 現在設定されている選択IDを再生 (1なら0001.mp3)
        // IDが0（未設定）の場合はデフォルトで1を再生
        uint16_t idToPlay = (options.selectedId > 0) ? options.selectedId : 1;
        play(idToPlay);
    }
}

/**
 * ③ ミニメニューやWebConfigでの「保存・脱出時」の即時再生用
 * AddonManagerやConfigServerから呼び出されることを想定
 */
void JinglePlayer::preprocess() {
    // 実行中に動的な再生が必要な場合はここに記述
}

// JQ8900 独自の 0xAA パケット送信 (チェックサム計算付き)
void JinglePlayer::sendCommand(uint8_t type, uint8_t* data, uint8_t len) {
    uint8_t buf[10]; // コマンドバッファ
    buf[0] = 0xAA;   // 開始バイト
    buf[1] = type;   // 命令
    buf[2] = len;    // データ長
    
    uint16_t sum = buf[0] + buf[1] + buf[2];
    for (int i = 0; i < len; i++) {
        buf[3 + i] = data[i];
        sum += data[i];
    }
    buf[3 + len] = (uint8_t)(sum & 0xFF); // チェックサム(下位8bit)
    
    // UART送信 (ブロッキング)
    uart_write_blocking(JQ8900_UART, buf, len + 4);
}

// 音量設定コマンド (0x13)
void JinglePlayer::setVolume(uint8_t volume) {
    if (volume > 30) volume = 30; // JQ8900の仕様上限
    uint8_t d = {volume};
    sendCommand(0x13, d, 1);
}

// 指定曲再生コマンド (0x07)
void JinglePlayer::play(uint16_t trackId) {
    if (!this->enabled) return;

    // trackIdを高位・低位バイトに分割
    uint8_t d = {
        (uint8_t)((trackId >> 8) & 0xFF), 
        (uint8_t)(trackId & 0xFF)
    };
    sendCommand(0x07, d, 2);
}

// 再生停止コマンド (0x04)
void JinglePlayer::stop() {
    sendCommand(0x04, nullptr, 0);
}

