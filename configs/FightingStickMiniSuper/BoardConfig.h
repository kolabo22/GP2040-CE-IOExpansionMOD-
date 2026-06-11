#ifndef BOARD_CONFIG_H
#define BOARD_CONFIG_H

#include <stdint.h>
#include "enums.pb.h"
#include "class/hid/hid.h"

#define BOARD_CONFIG_LABEL "MINI Super"

// ====================================================================
// 1. 物理ピン（GPIO）の完全固定マッピング
// ====================================================================

// レバー4方向
#define GPIO_PIN_02 GpioAction::BUTTON_PRESS_UP
#define GPIO_PIN_03 GpioAction::BUTTON_PRESS_DOWN
#define GPIO_PIN_04 GpioAction::BUTTON_PRESS_RIGHT
#define GPIO_PIN_05 GpioAction::BUTTON_PRESS_LEFT

// メイン30φボタン (8ボタン)
#define GPIO_PIN_06 GpioAction::BUTTON_PRESS_B1
#define GPIO_PIN_07 GpioAction::BUTTON_PRESS_B2
#define GPIO_PIN_08 GpioAction::BUTTON_PRESS_R2
#define GPIO_PIN_09 GpioAction::BUTTON_PRESS_L2
#define GPIO_PIN_10 GpioAction::BUTTON_PRESS_B3
#define GPIO_PIN_11 GpioAction::BUTTON_PRESS_B4
#define GPIO_PIN_12 GpioAction::BUTTON_PRESS_R1
#define GPIO_PIN_13 GpioAction::BUTTON_PRESS_L1

// 機能ボタン (TURBO, S2は完全にゲーム中に通常使用する独立ボタンとして固定)
#define GPIO_PIN_14 GpioAction::BUTTON_PRESS_TURBO
#define GPIO_PIN_17 GpioAction::BUTTON_PRESS_S2

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
// 2. 周辺機器・アドオンの完全固定マクロ (基本デフォルト値)
// ====================================================================
#define DEFAULT_INPUT_MODE INPUT_MODE_GENERIC
#define DEFAULT_SOCD_MODE SOCD_MODE_NEUTRAL
#define DEFAULT_DPAD_MODE DPAD_MODE_DIGITAL
#define DEBOUNCE_DELAY_IN_MS 5

// I2C周辺機器強制ON
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
#define USB_PIN_DP 28
#define USB_PIN_VBUS_ENABLE -1

// ====================================================================
// 3. アドオン機能のデフォルト初期値定義（再設定作業をゼロにするマクロ）
// ====================================================================
// Wii拡張アドオン
#define WII_EXTENSION_ENABLED 1
#define WII_EXTENSION_I2C_BLOCK i2c0

// 連射アドオン（ボタン14、LED15、ツマミ26無段階速度制御）
#define TURBO_ENABLED 1
#define TURBO_PIN 14
#define TURBO_LED_PIN 15
#define TURBO_SHOT_PIN 26

// リアクティブLED（Player LED：押すと消え、離すと光るフェードアウトモード）
#define PLAYER_LEDS_ENABLED 1
#define PLAYER_LED_PIN_P1 16
#define PLAYER_LED_PIN_P2 22
#define PLAYER_LED_PIN_P3 23
#define PLAYER_LED_PIN_P4 24
#define PLAYER_LED_REACTIVE_MODE 2 

// ====================================================================
// 4. LED構成・点灯順序・変則ケースLED
// ====================================================================
#define BOARD_LEDS_ENABLED 1
#define RGB_LED_NUM 47
#define LED_BRIGHTNESS_MAXIMUM 80
#define LED_BRIGHTNESS_STEPS 10
#define LED_FORMAT LED_FORMAT_GRB
#define LED_LAYOUT BUTTON_LAYOUT_STICK

// インデックス14から34個分をケースLED発光エリアとして厳密固定
#define LED_CASE_START_INDEX 14
#define LED_CASE_COUNT 34

// ボタン1つにつきLED1個の1対1直列配線順
#define LED_PINS_MAPPING { 0, 1, 2, 3, 4, 5, 6, 7 }

// ====================================================================
// 5. ディスプレイ構成（OLED ＆ 雪モード固定）
// ====================================================================
#define HAS_DISPLAY 1
#define DISPLAY_I2C_BLOCK i2c0 
#define DISPLAY_FLIP 0
#define DISPLAY_INVERT 0
#define BUTTON_LAYOUT_LEFT BUTTON_LAYOUT_STICK
#define BUTTON_LAYOUT_RIGHT BUTTON_LAYOUT_VEWLIX 

#define SPLASH_MODE SplashMode::SPLASH_MODE_STATIC
#define DISPLAY_SAVER_TIMEOUT 600000
#define SCREEN_SAVER_MODE 2 
#define DISPLAY_MENU_ENABLED 1 

// ====================================================================
// 6. PCF8575 IO エクスパンダー 16ピン入力キーマッピング完全同期
// ====================================================================
#define PCF8575_ENABLED 1
#define PCF8575_I2C_BLOCK i2c1

#define PCF8575_PIN_00_ACTION 15
#define PCF8575_PIN_01_ACTION 14
#define PCF8575_PIN_02_ACTION 21
#define PCF8575_PIN_03_ACTION 22
#define PCF8575_PIN_04_ACTION 23
#define PCF8575_PIN_05_ACTION 24
#define PCF8575_PIN_06_ACTION 25
#define PCF8575_PIN_07_ACTION 26
#define PCF8575_PIN_10_ACTION 16
#define PCF8575_PIN_11_ACTION 11
#define PCF8575_PIN_12_ACTION 12
#define PCF8575_PIN_13_ACTION 9
#define PCF8575_PIN_14_ACTION 13
#define PCF8575_PIN_15_ACTION 0  
#define PCF8575_PIN_16_ACTION 27
#define PCF8575_PIN_17_ACTION 28

#endif /* BOARD_CONFIG_H */
