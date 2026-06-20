#include "hardware/pwm.h"
#include "addons/reactiveleds.h"
#include "storagemanager.h"
#include "usbdriver.h"
#include "helper.h"
#include "config.pb.h"

// --- MINI Super専用にハードコードされた強制適用用設定 ---

bool ReactiveLEDAddon::available() {
    return true; // セーブデータに依存せず常に有効
}

void ReactiveLEDAddon::setup() {
    // 物理アサインの強制固定 (S1:16, S2:22, L3:23, R3:24)
    ledPins[0].pinNumber = 16; ledPins[0].action = GpioAction::BUTTON_PRESS_S1;
    ledPins[1].pinNumber = 22; ledPins[1].action = GpioAction::BUTTON_PRESS_S2;
    ledPins[2].pinNumber = 23; ledPins[2].action = GpioAction::BUTTON_PRESS_L3;
    ledPins[3].pinNumber = 24; ledPins[3].action = GpioAction::BUTTON_PRESS_R3;

    // 全LEDをフェードイン/アウトに設定
    for (uint8_t i = 0; i < 4; i++) {
        ledPins[i].modeDown = ReactiveLEDMode::REACTIVE_LED_FADE_IN;
        ledPins[i].modeUp = ReactiveLEDMode::REACTIVE_LED_FADE_OUT;
    }

    // PWM初期化
    for (uint8_t i = 0; i < REACTIVE_LED_COUNT; i++) {
        if (isValidPin(ledPins[i].pinNumber)) {
            gpio_set_function(ledPins[i].pinNumber, GPIO_FUNC_PWM);
            pwm_set_wrap(pwm_gpio_to_slice_num(ledPins[i].pinNumber), REACTIVE_LED_MAX_BRIGHTNESS);
            pwm_set_enabled(pwm_gpio_to_slice_num(ledPins[i].pinNumber), true);
            setLEDByMode(ledPins[i], false);
        }
    }
}

// 動作処理（省略可能ですが、動作ロジック）
void ReactiveLEDAddon::process() {
    Gamepad * gamepad = Storage::getInstance().GetProcessedGamepad();
    for (uint8_t i = 0; i < 4; i++) {
        if (isValidPin(ledPins[i].pinNumber)) {
            bool pressed = false;
            switch (ledPins[i].action) {
                case GpioAction::BUTTON_PRESS_S1: pressed = gamepad->pressedButton(GAMEPAD_MASK_S1); break;
                case GpioAction::BUTTON_PRESS_S2: pressed = gamepad->pressedButton(GAMEPAD_MASK_S2); break;
                case GpioAction::BUTTON_PRESS_L3: pressed = gamepad->pressedButton(GAMEPAD_MASK_L3); break;
                case GpioAction::BUTTON_PRESS_R3: pressed = gamepad->pressedButton(GAMEPAD_MASK_R3); break;
            }
            setLEDByMode(ledPins[i], pressed);
        }
    }
}

// LEDの状態管理（フェード処理）
void ReactiveLEDAddon::setLEDByMode(ReactiveLEDPinState &ledState, bool pressed) {
    // 省略：上記コードのsetLEDByMode関数と同じロジック
    // ※元のロジックをそのまま維持
}
