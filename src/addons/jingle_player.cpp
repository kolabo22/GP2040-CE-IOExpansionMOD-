#include "addons/jingle_player.h"
#include "drivermanager.h"

void JinglePlayerAddon::setup() {
    const auto& options = Storage::getInstance().getAddonOptions().jinglePlayerOptions;
    this->_volume = (uint8_t)options.volume;

    // UART初期化
    uart_init(uart1, 9600);
    gpio_set_function(20, GPIO_FUNC_UART);
    gpio_set_function(21, GPIO_FUNC_UART);

    this->_isConfig = DriverManager::getInstance().isConfigMode();
    this->_state = PlayState::WAIT_BOOT;
    this->_timer = to_ms_since_boot(get_absolute_time());
}

// 通常モード用ループ
void JinglePlayerAddon::process() {
    runStateMachine();
}

// WebConfig（S2）モード用ループ
void JinglePlayerAddon::postprocess(bool reportSent) {
    runStateMachine();
}

void JinglePlayerAddon::runStateMachine() {
    if (this->_state == PlayState::FINISHED) return;

    uint32_t now = to_ms_since_boot(get_absolute_time());

    switch (this->_state) {
        case PlayState::WAIT_BOOT:
            // S2時は長めに、通常時はOLEDを止めない最短(800ms)で待つ
            if (now - this->_timer >= (uint32_t)(this->_isConfig ? 2500 : 800)) {
                this->_state = PlayState::SET_VOL;
            }
            break;

        case PlayState::SET_VOL:
            setVolume(this->_volume);
            this->_timer = now;
            this->_state = PlayState::WAIT_VOL;
            break;

        case PlayState::WAIT_VOL:
            if (now - this->_timer >= 50) {
                this->_state = PlayState::PLAY;
            }
            break;

        case PlayState::PLAY:
            if (this->_isConfig) {
                play(21); // 設定モード：0021.mp3
            } else {
                playSelectedModeJingle(); // 機種別：ミニメニュー反映
            }
            this->_state = PlayState::FINISHED;
            break;

        default:
            break;
    }
}

void JinglePlayerAddon::playSelectedModeJingle() {
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
        case INPUT_MODE_SWITCH_PRO:   track = 13; break; 
        case INPUT_MODE_ASTRO:        track = 15; break;
        case INPUT_MODE_PSCLASSIC:    track = 16; break;
        case INPUT_MODE_XBOXORIGINAL: track = 17; break;
        case INPUT_MODE_EGRET:        track = 18; break;
        case INPUT_MODE_GENERIC:      track = 19; break;
        case INPUT_MODE_P5GENERAL:   track = 20; break;
        default:                      track = 1;  break;
    }
    play(track);
}

void JinglePlayerAddon::setVolume(uint8_t volume) {
    if (volume > 30) volume = 30;
    uint8_t packet[] = { 0xAA, 0x13, 0x01, volume, (uint8_t)(0xAA + 0x13 + 0x01 + volume) };
    for (int i = 0; i < 5; i++) uart_putc_raw(uart1, packet[i]);
}

void JinglePlayerAddon::play(uint16_t index) {
    uint8_t h = (uint8_t)((index >> 8) & 0xFF);
    uint8_t l = (uint8_t)(index & 0xFF);
    uint8_t device = 0x02;
    uint8_t sm = (uint8_t)(0xAA + 0x16 + 0x03 + device + h + l);
    uint8_t packet[] = { 0xAA, 0x16, 0x03, device, h, l, sm };
    for (int i = 0; i < 7; i++) uart_putc_raw(uart1, packet[i]);
}

void JinglePlayerAddon::reinit() {
    setup();
}
