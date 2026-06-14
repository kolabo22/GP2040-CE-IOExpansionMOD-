#include "addons/board_led.h"
#include "drivermanager.h"
#include "drivers/ps4/PS4Driver.h"
#include "usbdriver.h"
#include "helper.h"
#include "config.pb.h"

bool BoardLedAddon::available() {
    // 1. 参照元を非constにして取得（WebConfigデータ構造体）
    OnBoardLedOptions& options = Storage::getInstance().getAddonOptions().onBoardLedOptions;

    // 2. WebConfigに保存データがない（未設定リセット）場合のみ初期値をダイレクト上書き
    if (!options.isConfigured) {
        options.enabled = true;
        // 初期モードを「モードインジケーター（USB接続/設定モード識別点滅）」に設定
        options.mode = OnBoardLedMode::ON_BOARD_LED_MODE_MODE_INDICATOR; 
    }

    return options.enabled && options.mode != OnBoardLedMode::ON_BOARD_LED_MODE_OFF; // Available only when it's not set to off
}

void BoardLedAddon::setup() {
    // setupでも未設定時の値を完全に保証する
    OnBoardLedOptions& options = Storage::getInstance().getAddonOptions().onBoardLedOptions;
    if (!options.isConfigured) {
        options.enabled = true;
        options.mode = OnBoardLedMode::ON_BOARD_LED_MODE_MODE_INDICATOR;
    }

    onBoardLedMode = options.mode;
    isConfigMode = DriverManager::getInstance().isConfigMode();
    timeSinceBlink = getMillis();
    prevState = -1;

    gpio_init(BOARD_LED_PIN);
    gpio_set_dir(BOARD_LED_PIN, GPIO_OUT);
}

void BoardLedAddon::process() {
    bool state = 0;
    Gamepad * processedGamepad;
    uint16_t joystickMid = GAMEPAD_JOYSTICK_MID;
    if ( DriverManager::getInstance().getDriver() != nullptr ) {
        joystickMid = DriverManager::getInstance().getDriver()->GetJoystickMidValue();
    }
    switch (onBoardLedMode) {
        case OnBoardLedMode::ON_BOARD_LED_MODE_INPUT_TEST: // Blinks on input
            processedGamepad = Storage::getInstance().GetProcessedGamepad();
            state =    (processedGamepad->state.buttons != 0)
                    || (processedGamepad->state.dpad    != 0)
                    || (processedGamepad->state.lx      != joystickMid)
                    || (processedGamepad->state.rx      != joystickMid)
                    || (processedGamepad->state.ly      != joystickMid)
                    || (processedGamepad->state.ry      != joystickMid)
                    || (processedGamepad->state.lt      != 0)
                    || (processedGamepad->state.rt      != 0)
                    || (processedGamepad->state.aux     != 0);
            if (prevState != state) {
                gpio_put(BOARD_LED_PIN, state ? 1 : 0);
            }
            prevState = state;
            break;
        case OnBoardLedMode::ON_BOARD_LED_MODE_MODE_INDICATOR: // Blinks based on USB state and config mode
            if (!get_usb_mounted()) { // USB not mounted
                uint32_t millis = getMillis();
                if ((millis - timeSinceBlink) > BLINK_INTERVAL_USB_UNMOUNTED) {
                    gpio_put(BOARD_LED_PIN, prevState ? 1 : 0);
                    timeSinceBlink = millis;
                    prevState = !prevState;
                }
            } else {
                if (isConfigMode) { // Current mode is config
                    uint32_t millis = getMillis();
                    if ((millis - timeSinceBlink) > BLINK_INTERVAL_CONFIG_MODE) {
                        gpio_put(BOARD_LED_PIN, prevState ? 1 : 0);
                        timeSinceBlink = millis;
                        prevState = !prevState;
                    }
                } else { // Regular mode and functional
                    if (prevState != 1) {
                        gpio_put(BOARD_LED_PIN, 1);
                        prevState = 1;
                    }
                }
            }
            break;
        case OnBoardLedMode::ON_BOARD_LED_MODE_PS_AUTH:
            processedGamepad = Storage::getInstance().GetProcessedGamepad();
            if(processedGamepad->getOptions().inputMode == INPUT_MODE_PS4 ||
                processedGamepad->getOptions().inputMode == INPUT_MODE_PS5) {
                state = ((PS4Driver*)DriverManager::getInstance().getDriver())->getAuthSent() == true;
            }
            if (prevState != state) {
                gpio_put(BOARD_LED_PIN, state ? 1 : 0);
            }
            prevState = state;
            break;
        case OnBoardLedMode::ON_BOARD_LED_MODE_OFF:
        default:
            break;
    }
}
