#ifndef BOARD_CONFIG_H
#define BOARD_CONFIG_H

#include <stdint.h>
#include "enums.pb.h"
#include "class/hid/hid.h"

#define BOARD_CONFIG_LABEL "MINI Super"

// ====================================================================
// 1. 物理ピン（GPIO）の完全固定マッピング
// ====================================================================

// レバー4方向（初期設定のままネイティブロード）
#define GPIO_PIN_02 GpioAction::BUTTON_PRESS_UP
#define GPIO_PIN_03 GpioAction::BUTTON_PRESS_DOWN
#define GPIO_PIN_04 GpioAction::BUTTON_PRESS_RIGHT
#define GPIO_PIN_05 GpioAction::BUTTON_PRESS_LEFT

// メイン30φボタン (8ボタン ＆ 初期設定のままネイティブロード)
#define GPIO_PIN_06 GpioAction::BUTTON_PRESS_B1
#define GPIO_PIN_07 GpioAction::BUTTON_PRESS_B2
#define GPIO_PIN_08 GpioAction::BUTTON_PRESS_R2
#define GPIO_PIN_09 GpioAction::BUTTON_PRESS_L2
#define GPIO_PIN_10 GpioAction::BUTTON_PRESS_B3
#define GPIO_PIN_11 GpioAction::BUTTON_PRESS_B4
#define GPIO_PIN_12 GpioAction::BUTTON_PRESS_R1
#define GPIO_PIN_13 GpioAction::BUTTON_PRESS_L1

// 機能ボタン (S2/STARTは完全に独立した通常ボタンとして固定。工場出荷状態で動きます)
#define GPIO_PIN_14 GpioAction::BUTTON_PRESS_TURBO
#define GPIO_PIN_17 GpioAction::BUTTON_PRESS_S2

// Keyboard Mapping Configuration
//                                            // GP2040 | Xinput | Switch  | PS3/4/5  | Dinput | Arcade |
#define KEY_DPAD_UP     HID_KEY_ARROW_UP      // UP     | UP     | UP      | UP       | UP     | UP     |
#define KEY_DPAD_DOWN   HID_KEY_ARROW_DOWN    // DOWN   | DOWN   | DOWN    | DOWN     | DOWN   | DOWN   |
#define KEY_DPAD_RIGHT  HID_KEY_ARROW_RIGHT   // RIGHT  | RIGHT  | RIGHT   | RIGHT    | RIGHT  | RIGHT  |
#define KEY_DPAD_LEFT   HID_KEY_ARROW_LEFT    // LEFT   | LEFT   | LEFT    | LEFT     | LEFT   | LEFT   |
#define KEY_BUTTON_B1   HID_KEY_SHIFT_LEFT    // B1     | A      | B       | Cross    | 2      | K1     |
#define KEY_BUTTON_B2   HID_KEY_Z             // B2     | B      | A       | Circle   | 3      | K2     |
#define KEY_BUTTON_R2   HID_KEY_X             // R2     | RT     | ZR      | R2       | 8      | K3     |
#define KEY_BUTTON_L2   HID_KEY_V             // L2     | LT     | ZL      | L2       | 7      | K4     |
#define KEY_BUTTON_B3   HID_KEY_CONTROL_LEFT  // B3     | X      | Y       | Square   | 1      | P1     |
#define KEY_BUTTON_B4   HID_KEY_ALT_LEFT      // B4     | Y      | X       | Triangle | 4      | P2     |
#define KEY_BUTTON_R1   HID_KEY_SPACE         // R1     | RB     | R       | R1       | 6      | P3     |
#define KEY_BUTTON_L1   HID_KEY_C             // L1     | LB     | L       | L1       | 5      | P4     |
#define KEY_BUTTON_S1   HID_KEY_5             // S1     | Back   | Minus   | Select   | 9      | Coin   |
#define KEY_BUTTON_S2   HID_KEY_1             // S2     | Start  | Plus    | Start    | 10     | Start  |
#define KEY_BUTTON_L3   HID_KEY_EQUAL         // L3     | LS     | LS      | L3       | 11     | LS     |
#define KEY_BUTTON_R3   HID_KEY_MINUS         // R3     | RS     | RS      | R3       | 12     | RS     |
#define KEY_BUTTON_A1   HID_KEY_9             // A1     | Guide  | Home    | PS       | 13     | ~      |
#define KEY_BUTTON_A2   HID_KEY_F2            // A2     | ~      | Capture | ~        | 14     | ~      |
#define KEY_BUTTON_FN   -1                    // Hotkey Function                                        |

// 各種アドオン/周辺機器ピンをADDON管轄へ明示的リリース
#define GPIO_PIN_15 GpioAction::ASSIGNED_TO_ADDON // Turbo_LED (GP15)
#define GPIO_PIN_16 GpioAction::ASSIGNED_TO_ADDON // Player LED 1 (GP16)
#define GPIO_PIN_22 GpioAction::ASSIGNED_TO_ADDON // Player LED 2 (GP22)
#define GPIO_PIN_23 GpioAction::ASSIGNED_TO_ADDON // Player LED 3 (GP23)
#define GPIO_PIN_24 GpioAction::ASSIGNED_TO_ADDON // Player LED 4 (GP24)
#define GPIO_PIN_25 GpioAction::ASSIGNED_TO_ADDON // オンボードLED

#define GPIO_PIN_00 GpioAction::ASSIGNED_TO_ADDON // I2C0 SDA (Wii)
#define GPIO_PIN_01 GpioAction::ASSIGNED_TO_ADDON // I2C0 SCL (Wii)
#define GPIO_PIN_18 GpioAction::ASSIGNED_TO_ADDON // I2C1 SDA (PCF8575)
#define GPIO_PIN_19 GpioAction::ASSIGNED_TO_ADDON // I2C1 SCL (PCF8575)
#define GPIO_PIN_20 GpioAction::ASSIGNED_TO_ADDON // UART1 TX (JQ8900)
#define GPIO_PIN_21 GpioAction::ASSIGNED_TO_ADDON // UART1 RX (JQ8900)
#define GPIO_PIN_26 GpioAction::ASSIGNED_TO_ADDON // Analog (Turbo VR)
#define GPIO_PIN_27 GpioAction::ASSIGNED_TO_ADDON // RGB LED
#define GPIO_PIN_28 GpioAction::ASSIGNED_TO_ADDON // USB0 D+
#define GPIO_PIN_29 GpioAction::ASSIGNED_TO_ADDON // USB0 D-

// ====================================================================
// 2. 周辺機器通信プロファイルの有効化（初期設定）
// ====================================================================
#define DEFAULT_INPUT_MODE INPUT_MODE_GENERIC
#define DEFAULT_SOCD_MODE SOCD_MODE_NEUTRAL
#define DEFAULT_DPAD_MODE DPAD_MODE_DIGITAL
#define DEBOUNCE_DELAY_IN_MS 5

// 周辺機器通信マニュアル有効化
#define I2C0_ENABLED 1
#define I2C0_PIN_SDA 0
#define I2C0_PIN_SCL 1
#define I2C0_SPEED 400000

#define I2C1_ENABLED 1
#define I2C1_PIN_SDA 18
#define I2C1_PIN_SCL 19
#define I2C1_SPEED 400000

#define UART1_ENABLED 1
#define UART1_PIN_TX 20
#define UART1_PIN_RX 21
#define UART1_BAUDRATE 9600

#define USB_PERIPHERAL_ENABLED 1
#define USB_PIN_DP 27
#define USB_PIN_VBUS_ENABLE -1

// 各アドオン機能の初期ONマクロ
#define WII_EXTENSION_ENABLED 1
#define TURBO_ENABLED 1
#define REACTIVE_LED_ENABLED 1
#define TURBO_ENABLED 1
#define GPIO_PIN_14 GpioAction::BUTTON_PRESS_TURBO
#define TURBO_LED_PIN 15

// ====================================================================
// 3. 【16ページ完全同期】PCF8575 IOエクスパンダー 初期アサイン定数
// ====================================================================
#define I2C_PCF8575_ENABLED 1
#define I2C_PCF8575_BLOCK i2c1

#define PCF8575_PIN00_ACTION GpioAction::BUTTON_PRESS_A3
#define PCF8575_PIN01_ACTION GpioAction::BUTTON_PRESS_A2
#define PCF8575_PIN02_ACTION GpioAction::BUTTON_PRESS_E1
#define PCF8575_PIN03_ACTION GpioAction::BUTTON_PRESS_E2
#define PCF8575_PIN04_ACTION GpioAction::BUTTON_PRESS_E3
#define PCF8575_PIN05_ACTION GpioAction::BUTTON_PRESS_E4
#define PCF8575_PIN06_ACTION GpioAction::BUTTON_PRESS_E5
#define PCF8575_PIN07_ACTION GpioAction::BUTTON_PRESS_E6
#define PCF8575_PIN08_ACTION GpioAction::BUTTON_PRESS_A4
#define PCF8575_PIN09_ACTION GpioAction::BUTTON_PRESS_L3
#define PCF8575_PIN10_ACTION GpioAction::BUTTON_PRESS_R3
#define PCF8575_PIN11_ACTION GpioAction::BUTTON_PRESS_S1
#define PCF8575_PIN12_ACTION GpioAction::BUTTON_PRESS_A1
#define PCF8575_PIN13_ACTION GpioAction::NONE
#define PCF8575_PIN14_ACTION GpioAction::BUTTON_PRESS_E7
#define PCF8575_PIN15_ACTION GpioAction::BUTTON_PRESS_E8

#define PCF8575_PIN00_DIRECTION GpioDirection::GPIO_DIRECTION_INPUT
#define PCF8575_PIN01_DIRECTION GpioDirection::GPIO_DIRECTION_INPUT
#define PCF8575_PIN02_DIRECTION GpioDirection::GPIO_DIRECTION_INPUT
#define PCF8575_PIN03_DIRECTION GpioDirection::GPIO_DIRECTION_INPUT
#define PCF8575_PIN04_DIRECTION GpioDirection::GPIO_DIRECTION_INPUT
#define PCF8575_PIN05_DIRECTION GpioDirection::GPIO_DIRECTION_INPUT
#define PCF8575_PIN06_DIRECTION GpioDirection::GPIO_DIRECTION_INPUT
#define PCF8575_PIN07_DIRECTION GpioDirection::GPIO_DIRECTION_INPUT
#define PCF8575_PIN08_DIRECTION GpioDirection::GPIO_DIRECTION_INPUT
#define PCF8575_PIN09_DIRECTION GpioDirection::GPIO_DIRECTION_INPUT
#define PCF8575_PIN10_DIRECTION GpioDirection::GPIO_DIRECTION_INPUT
#define PCF8575_PIN11_DIRECTION GpioDirection::GPIO_DIRECTION_INPUT
#define PCF8575_PIN12_DIRECTION GpioDirection::GPIO_DIRECTION_INPUT
#define PCF8575_PIN13_DIRECTION GpioDirection::GPIO_DIRECTION_INPUT
#define PCF8575_PIN14_DIRECTION GpioDirection::GPIO_DIRECTION_INPUT
#define PCF8575_PIN15_DIRECTION GpioDirection::GPIO_DIRECTION_INPUT

// ====================================================================
// 4. LED構成 ＆ ディスプレイ基本プロファイル
// ====================================================================
#define BOARD_LEDS_ENABLED 1
#define RGB_LED_NUM 47
#define LED_BRIGHTNESS_MAXIMUM 80
#define LED_BRIGHTNESS_STEPS 10
#define LED_FORMAT LED_FORMAT_GRB
#define LED_LAYOUT BUTTON_LAYOUT_STICK

// 10ページのバニラ名に完全同期
#define CASE_RGB_TYPE CaseRGBType::CASE_RGB_TYPE_STATIC
#define CASE_RGB_INDEX 14
#define CASE_RGB_COUNT 34

#define HAS_DISPLAY 1
#define DISPLAY_I2C_BLOCK i2c0 
#define DISPLAY_FLIP 0
#define DISPLAY_INVERT 0
#define BUTTON_LAYOUT BUTTON_LAYOUT_STICK
#define BUTTON_LAYOUT_RIGHT BUTTON_LAYOUT_VEWLIX 

#define SPLASH_MODE static_cast<SplashMode>(0)
#define SPLASH_CHOICE static_cast<SplashChoice>(0)
#define SPLASH_DURATION 7000
#define DISPLAY_SAVER_TIMEOUT 600000
#define DISPLAY_SAVER_MODE static_cast<DisplaySaverMode>(2) 

#endif /* BOARD_CONFIG_H */
