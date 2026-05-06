#include "addons/jingle_player.h"
#include "drivermanager.h"
#include "storagemanager.h"

void JinglePlayerAddon::setup() {
    const auto& options = Storage::getInstance().getAddonOptions().jinglePlayerOptions;
    if (!options.enabled) return;

    this->volume = (uint8_t)options.volume;
    this->_hasPlayedOnBoot = false;

    // JQ8900 UART通信初期化 (GP20:TX / GP21:RX)
    uart_init(uart1, 9600);
    gpio_set_function(20, GPIO_FUNC_UART);
    gpio_set_function(21, GPIO_FUNC_UART);

    // 起動直後の判定を安定させるため微小待機してからConfigModeか判定し保持
    sleep_ms(10);
    this->_wasConfigMode = DriverManager::getInstance().isConfigMode();
}

void JinglePlayerAddon::process() {
    const auto& options = Storage::getInstance().getAddonOptions().jinglePlayerOptions;
    if (!options.enabled || this->_hasPlayedOnBoot) return;

    // 起動からの経過時間を取得
    uint32_t elapsed = to_ms_since_boot(get_absolute_time());

    // 設定モードなら1.5秒、通常なら0.8秒待機してシステム安定化を待つ
    uint32_t waitThreshold = this->_wasConfigMode ? 1500 : 800;

    if (elapsed < waitThreshold) return;

    // --- 再生実行（一度だけ通過） ---
    setVolume(this->volume);
    sleep_ms(50); // パケット間の安定用ウェイト

    if (this->_wasConfigMode) {
        // 設定モード（S2起動）：0021.mp3 を再生
        play(21);
    } else {
        // 通常起動：現在の確定済みモードをリアルタイム取得して再生
        playSelectedModeJingle();
    }
    
    this->_hasPlayedOnBoot = true; 
}

void JinglePlayerAddon::playSelectedModeJingle() {
    // DriverManagerから現在動作中の入力を取得（ミニメニューの変更を反映）
    InputMode mode = DriverManager::getInstance().getInputMode();
    uint16_t track = 1;

    switch (mode) {
        case INPUT_MODE_XINPUT:       track = 1;  break;
        case INPUT_MODE_SWITCH:       track = 2;  break;
        case INPUT_MODE_PS3:          track = 3;  break;
        case INPUT_MODE_PS4:          track = 4;  break;
        case INPUT_MODE_KEYBOARD:     track = 5;  break;
        case INPUT_MODE_XBONE:        track = 6;  break;
        case INPUT_MODE_PS5:          track = 7;  break;
        case INPUT_MODE_MDMINI:       track = 8;  break;
        case INPUT_MODE_NEOGEO:       track = 10; break;
        case INPUT_MODE_PCEMINI:      track = 11; break;
        case INPUT_MODE_SWITCH_PRO:   track = 13; break; // 0013.mp3
        case INPUT_MODE_ASTRO:        track = 15; break;
        case INPUT_MODE_PSCLASSIC:    track = 16; break;
        case INPUT_MODE_XBOXORIGINAL: track = 17; break;
        case INPUT_MODE_EGRET:        track = 18; break;
        case INPUT_MODE_GENERIC:      track = 19; break;
        case INPUT_MODE_P5GENERAL:   track = 20; break; // 0020.mp3
        default:                      track = 1;  break;
    }
    play(track);
}

// JQ8900専用：音量設定（0x13）
void JinglePlayerAddon::setVolume(uint8_t volume) {
    if (volume > 30) volume = 30; // 最大値制限
    
    uint8_t cmd = 0x13;
    uint8_t len = 0x01;
    uint8_t sm = (uint8_t)(0xAA + cmd + len + volume);
    
    uint8_t packet[] = { 0xAA, cmd, len, volume, sm };
    for (int i = 0; i < 5; i++) uart_putc_raw(uart1, packet[i]);
}

// JQ8900専用：内蔵Flash(0x02)指定の曲再生（0x16）
void JinglePlayerAddon::play(uint16_t index) {
    uint8_t h = (uint8_t)((index >> 8) & 0xFF);
    uint8_t l = (uint8_t)(index & 0xFF);
    uint8_t cmd = 0x16;
    uint8_t len = 0x03;
    uint8_t device = 0x02; // 内蔵Flashメモリ

    uint8_t sm = (uint8_t)(0xAA + cmd + len + device + h + l);
    
    uint8_t packet[] = { 0xAA, cmd, len, device, h, l, sm };
    for (int i = 0; i < 7; i++) uart_putc_raw(uart1, packet[i]);
}

void JinglePlayerAddon::postprocess(bool reportSent) {}
void JinglePlayerAddon::reinit() {}
