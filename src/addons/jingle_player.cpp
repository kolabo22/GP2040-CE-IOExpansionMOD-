#include "addons/jingle_player.h"
#include "storagemanager.h"
#include "drivermanager.h"

void JinglePlayerAddon::setup() {
    const auto& options = Storage::getInstance().getAddonOptions().jinglePlayerOptions;
    if (!options.enabled) return;

    this->volume = (uint8_t)options.volume;
    _hasPlayedOnBoot = false;
    _wasConfigMode = false;

    // UART1 (GP20:TX / GP21:RX)
    uart_init(uart1, 9600);
    gpio_set_function(20, GPIO_FUNC_UART);
    gpio_set_function(21, GPIO_FUNC_UART);
}

void JinglePlayerAddon::process() {
    const auto& options = Storage::getInstance().getAddonOptions().jinglePlayerOptions;
    if (!options.enabled) return;

    static uint32_t bootDelay = 0;
    if (!_hasPlayedOnBoot) {
        // JQ8900の起動完了を待つ (約1.5秒〜2秒)
        if (bootDelay < 200000) { 
            bootDelay++;
            return;
        }

        bool isConfig = DriverManager::getInstance().isConfigMode();
        setVolume(this->volume);
        sleep_ms(20);

        if (isConfig) {
            play(21); // 0021.mp3: WebConfigモード
        } else {
            playSelectedModeJingle();
        }
        
        _hasPlayedOnBoot = true;
        _wasConfigMode = isConfig;
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

// 最もシンプルな再生コマンド（開始-長さ-コマンド-データH-データL-終了）
void JinglePlayerAddon::play(uint16_t index) {
    uint8_t h = (uint8_t)((index >> 8) & 0xFF);
    uint8_t l = (uint8_t)(index & 0xFF);
    
    // チェックサムを計算せず、固定のパケットを送る（JQ8900のサブセット仕様）
    uint8_t packet[] = { 0x7E, 0x04, 0x03, h, l, 0xEF }; 
    for (int i = 0; i < 6; i++) uart_putc_raw(uart1, packet[i]);
}

// 音量設定も同様にシンプル化
void JinglePlayerAddon::setVolume(uint8_t volume) {
    if (volume > 30) volume = 30;
    uint8_t packet[] = { 0x7E, 0x03, 0x06, volume, 0xEF }; 
    for (int i = 0; i < 5; i++) uart_putc_raw(uart1, packet[i]);
}

void JinglePlayerAddon::postprocess(bool reportSent) {}
void JinglePlayerAddon::reinit() {}
