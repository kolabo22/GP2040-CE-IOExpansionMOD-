#include "addons/jingle_player.h"
#include "drivermanager.h"

void JinglePlayerAddon::setup() {
    const auto& options = Storage::getInstance().getAddonOptions().jinglePlayerOptions;
    this->volume = (uint8_t)options.volume;

    // UART初期化
    uart_init(uart1, 9600);
    gpio_set_function(20, GPIO_FUNC_UART);
    gpio_set_function(21, GPIO_FUNC_UART);

    this->_isConfigAtBoot = DriverManager::getInstance().isConfigMode();

    if (this->_isConfigAtBoot) {
        // 【Configモード専用】setup内で完結させる（processが回らないため）
        // WebUI起動時はOLEDの干渉が少ないため、ここだけsleep_msを許容
        sleep_ms(1500); 
        setVolume(this->volume);
        sleep_ms(50);
        play(21);
        this->_state = PlayState::FINISHED;
    } else {
        // 【通常モード専用】非同期でprocessに任せる（OLEDフリーズ防止）
        this->_state = PlayState::WAIT_FOR_BOOT;
        this->_stateTimer = to_ms_since_boot(get_absolute_time());
    }
}

void JinglePlayerAddon::process() {
    if (this->_state == PlayState::FINISHED || this->_isConfigAtBoot) return;

    uint32_t now = to_ms_since_boot(get_absolute_time());

    switch (_state) {
        case PlayState::WAIT_FOR_BOOT:
            if (now - _stateTimer >= 800) {
                _state = PlayState::SET_VOLUME;
            }
            break;

        case PlayState::SET_VOLUME:
            setVolume(this->volume);
            _stateTimer = now;
            _state = PlayState::WAIT_FOR_VOLUME;
            break;

        case PlayState::WAIT_FOR_VOLUME:
            if (now - _stateTimer >= 50) {
                _state = PlayState::PLAY_JINGLE;
            }
            break;

        case PlayState::PLAY_JINGLE:
            playSelectedModeJingle();
            _state = PlayState::FINISHED;
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
