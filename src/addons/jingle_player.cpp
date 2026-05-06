#include "addons/jingle_player.h"
#include "storagemanager.h"
#include "drivermanager.h"

void JinglePlayerAddon::setup() {
    const auto& options = Storage::getInstance().getAddonOptions().jinglePlayerOptions;
    if (!options.enabled) return;

    this->volume = (uint8_t)options.volume;
    _hasPlayedOnBoot = false;

    // JQ8900仕様: 9600bps, 8bit, 1 Stop, No Parity (データシート11P)
    uart_init(uart1, 9600);
    gpio_set_function(20, GPIO_FUNC_UART); // TX
    gpio_set_function(21, GPIO_FUNC_UART); // RX
}

void JinglePlayerAddon::process() {
    const auto& options = Storage::getInstance().getAddonOptions().jinglePlayerOptions;
    if (!options.enabled) return;

    static uint32_t bootDelay = 0;
    if (!_hasPlayedOnBoot) {
        // JQ8900の起動完了待ち (約2秒)
        if (bootDelay < 250000) { 
            bootDelay++;
            return;
        }

        bool isConfig = DriverManager::getInstance().isConfigMode();
        setVolume(this->volume);
        sleep_ms(50); // コマンド間の安定待ち

        if (isConfig) {
            play(21); // WebConfigモード：0021.mp3
        } else {
            playSelectedModeJingle();
        }
        
        _hasPlayedOnBoot = true;
    }
}

void JinglePlayerAddon::playSelectedModeJingle() {
    InputMode mode = DriverManager::getInstance().getInputMode();
    uint16_t track = 1; // デフォルト 0001.mp3

    // enums.proto の定義名に完全に一致させました
    switch (mode) {
        case INPUT_MODE_XINPUT:      track = 1;  break;
        case INPUT_MODE_SWITCH:      track = 2;  break;
        case INPUT_MODE_PS3:         track = 3;  break;
        case INPUT_MODE_KEYBOARD:    track = 5;  break; // リストに基づき5番
        case INPUT_MODE_PS4:         track = 4;  break;
        case INPUT_MODE_XBONE:       track = 6;  break;
        case INPUT_MODE_MDMINI:      track = 8;  break; // 修正：MDMINI
        case INPUT_MODE_NEOGEO:      track = 10; break;
        case INPUT_MODE_PCEMINI:     track = 11; break; // 修正：PCEMINI
        case INPUT_MODE_EGRET:       track = 18; break; // 修正：EGRET
        case INPUT_MODE_ASTRO:       track = 15; break; // 修正：ASTRO
        case INPUT_MODE_PSCLASSIC:   track = 16; break; // 修正：PSCLASSIC
        case INPUT_MODE_XBOXORIGINAL: track = 17; break; // 修正：XBOXORIGINAL
        case INPUT_MODE_PS5:         track = 7;  break; // リストに基づき7番
        case INPUT_MODE_GENERIC:     track = 19; break; // 修正：GENERIC (19番)
        
        default:                     track = 1;  break;
    }
    play(track);
}

// JQ8900専用：音量設定 (データシート15P 0x13)
void JinglePlayerAddon::setVolume(uint8_t volume) {
    if (volume > 30) volume = 30;
    
    uint8_t cmd = 0x13;
    uint8_t len = 0x01;
    // チェックサム(SM): 全バイトの合計の下位8ビット
    uint8_t sm = (uint8_t)(0xAA + cmd + len + volume);
    
    uint8_t packet[] = { 0xAA, cmd, len, volume, sm };
    for (int i = 0; i < 5; i++) uart_putc_raw(uart1, packet[i]);
}

// JQ8900専用：内蔵Flash(0x02)の指定曲再生 (データシート15P 0x16準拠)
void JinglePlayerAddon::play(uint16_t index) {
    uint8_t h = (uint8_t)((index >> 8) & 0xFF);
    uint8_t l = (uint8_t)(index & 0xFF);
    uint8_t cmd = 0x16;    // 指定デバイス・指定曲再生コマンド
    uint8_t len = 0x03;    // データ長 (device + h + l)
    uint8_t device = 0x02; // ★重要：0x02 = 内蔵Flashメモリ

    // チェックサム(SM): AA + 16 + 03 + 02 + High + Low
    uint8_t sm = (uint8_t)(0xAA + cmd + len + device + h + l);
    
    uint8_t packet[] = { 0xAA, cmd, len, device, h, l, sm };
    for (int i = 0; i < 7; i++) uart_putc_raw(uart1, packet[i]);
}

void JinglePlayerAddon::postprocess(bool reportSent) {}
void JinglePlayerAddon::reinit() {}
