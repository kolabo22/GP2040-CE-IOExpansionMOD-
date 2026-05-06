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
	
    // 提示されたSDカード配置リストに準拠
switch (mode) {
        case INPUT_MODE_XINPUT:      track = 1;  break;
        case INPUT_MODE_SWITCH:      track = 2;  break;
        case INPUT_MODE_PS3:         track = 3;  break; // DirectInput/PS3
        case INPUT_MODE_PS4:         track = 4;  break;
        case INPUT_MODE_KEYBOARD:    track = 5;  break;
        case INPUT_MODE_XBONE:       track = 6;  break;
        case INPUT_MODE_PS5:         track = 7;  break;
        
        // エラー修正箇所：定義名をプロジェクトの基準に合わせる
        case INPUT_MODE_GENESIS:     track = 8;  break; // MDからGENESISへ
        case INPUT_MODE_NEOGEO:      track = 10; break;
        case INPUT_MODE_PCEMINI:     track = 11; break; // PCEからPCEMINIへ
        
        // もし以下のモードがまだエラーになる場合は、一旦コメントアウトするか default に逃がします
        case INPUT_MODE_ASTRO:    track = 15; break; 
        case INPUT_MODE_PSCLASSIC:   track = 16; break; // CLASSICから修正
        case INPUT_MODE_XBOXORIGINAL: track = 17; break;
        case INPUT_MODE_EGRET:       track = 18; break;
        
        default:                     track = 1;  break;
    }
    play(track);
}

// JQ8900専用：音量設定 (0x0C)
void JinglePlayerAddon::setVolume(uint8_t volume) {
    if (volume > 30) volume = 30; // JQ8900の最大値は30
    // パケット: [7E] [長さ03] [コマンド0C] [パラメータ] [チェックサム] [EF]
    uint8_t checksum = 0x7E ^ 0x03 ^ 0x0C ^ volume;
    uint8_t packet[] = { 0x7E, 0x03, 0x0C, volume, checksum, 0xEF };
    for (int i = 0; i < 6; i++) uart_putc_raw(uart1, packet[i]);
}

// JQ8900専用：指定曲再生 (0x07)
void JinglePlayerAddon::play(uint16_t index) {
    uint8_t h = (uint8_t)((index >> 8) & 0xFF);
    uint8_t l = (uint8_t)(index & 0xFF);
    // パケット: [7E] [長さ04] [コマンド07] [索引H] [索引L] [チェックサム] [EF]
    uint8_t checksum = 0x7E ^ 0x04 ^ 0x07 ^ h ^ l;
    uint8_t packet[] = { 0x7E, 0x04, 0x07, h, l, checksum, 0xEF };
    for (int i = 0; i < 7; i++) uart_putc_raw(uart1, packet[i]);
}

void JinglePlayerAddon::postprocess(bool reportSent) {}
void JinglePlayerAddon::reinit() {}
