#include "addons/jingle_player.h"
#include "storagemanager.h"
#include "drivermanager.h"

void JinglePlayerAddon::setup() {
    const auto& options = Storage::getInstance().getAddonOptions().jinglePlayerOptions;
    if (!options.enabled) return;

    this->volume = (uint8_t)options.volume;
    _hasPlayedOnBoot = false;

    // JQ8900 UART通信初期化 (GP20:TX / GP21:RX)
    uart_init(uart1, 9600);
    gpio_set_function(20, GPIO_FUNC_UART);
    gpio_set_function(21, GPIO_FUNC_UART);

    // 設定モード判定
    bool isConfig = DriverManager::getInstance().isConfigMode();

    if (isConfig) {
        // 設定モード（S2起動）：システム安定化のため少し長めに待機（1.5秒）
        sleep_ms(1500); 
        setVolume(this->volume);
        sleep_ms(50);
        play(21); // 0021.mp3 を再生
        _hasPlayedOnBoot = true; // process側での重複再生を防止
    } else {
        // 通常起動：素早く再生するために待機時間を短縮（0.8秒）
        sleep_ms(800); 
    }
}

void JinglePlayerAddon::process() {
    const auto& options = Storage::getInstance().getAddonOptions().jinglePlayerOptions;
    
    // アドオン無効、または既に再生済み（設定モードなど）の場合は何もしない
    if (!options.enabled || _hasPlayedOnBoot) return;

    // 通常モード時の再生処理
    setVolume(this->volume);
    sleep_ms(50);
    playSelectedModeJingle();
    
    _hasPlayedOnBoot = true; // 起動時の一回のみ再生
}

void JinglePlayerAddon::playSelectedModeJingle() {
    InputMode mode = DriverManager::getInstance().getInputMode();
    uint16_t track = 1;

    // enums.protoの定義に基づいたモード別鳴らし分け
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
        case INPUT_MODE_ASTRO:        track = 15; break;
        case INPUT_MODE_PSCLASSIC:    track = 16; break;
        case INPUT_MODE_XBOXORIGINAL: track = 17; break;
        case INPUT_MODE_EGRET:        track = 18; break;
        case INPUT_MODE_GENERIC:      track = 19; break;
        default:                      track = 1;  break;
    }
    play(track);
}

// JQ8900専用：音量設定（0x13）
// 合計値チェックサム方式（AA + CMD + LEN + DATA）
void JinglePlayerAddon::setVolume(uint8_t volume) {
    if (volume > 30) volume = 30; // JQ8900の最大値は30
    
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
    uint8_t cmd = 0x16;    // 指定デバイス・指定曲再生
    uint8_t len = 0x03;    // データ長（device + h + l）
    uint8_t device = 0x02; // 内蔵Flashメモリ指定

    uint8_t sm = (uint8_t)(0xAA + cmd + len + device + h + l);
    
    uint8_t packet[] = { 0xAA, cmd, len, device, h, l, sm };
    for (int i = 0; i < 7; i++) uart_putc_raw(uart1, packet[i]);
}

void JinglePlayerAddon::postprocess(bool reportSent) {}
void JinglePlayerAddon::reinit() {}
