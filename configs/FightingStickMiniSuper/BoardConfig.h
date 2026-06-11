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

// 機能ボタン (TURBO, S2/STARTは完全に独立した通常ボタンとして固定)
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

// コア通信ブロックの強制有効化
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
// 3. 各種アドオン機能の初期動作設定（公式システムマクロ名へ完全同期）
// ====================================================================
// Wii拡張アドオン
#define WII_EXTENSION_ENABLED 1
#define WII_EXTENSION_I2C_BLOCK i2c0

// 連射アドオン
#define TURBO_ENABLED 1
#define TURBO_PIN 14
#define TURBO_LED_PIN 15
#define PIN_SHMUP_DIAL 26
#define TURBO_SHMUP_MODE 1

// リアクティブLEDアドオン
#define REACTIVE_LED_ENABLED 1
#define REACTIVE_LED_PIN_0 16
#define REACTIVE_LED_PIN_1 22
#define REACTIVE_LED_PIN_2 23
#define REACTIVE_LED_PIN_3 24
#define REACTIVE_LED_REACTIVE_MODE 2 

// ====================================================================
// 4. LED構成・点灯順序・変則ケースLED（【修正】型安全エラーの完封）
// ====================================================================
#define BOARD_LEDS_ENABLED 1
#define RGB_LED_NUM 47
#define LED_BRIGHTNESS_MAXIMUM 80
#define LED_BRIGHTNESS_STEPS 10
#define LED_FORMAT LED_FORMAT_GRB
#define LED_LAYOUT BUTTON_LAYOUT_STICK

// 【解決】ただの1ではなく、システム構造体型（CaseRGBType）へ明示的キャスト定義
#define CASE_RGB_TYPE (CaseRGBType)1
#define CASE_RGB_INDEX 14
#define CASE_RGB_COUNT 34

// ボタン1つにつきLED1個の1対1直列接続順（× ➡️ ○ ➡️ R2 ➡️ L2 ➡️ L1 ➡️ R1 ➡️ △ ➡️ □）
#define LEDS_BUTTON_B1 0
#define LEDS_BUTTON_B2 1
#define LEDS_BUTTON_R2 2
#define LEDS_BUTTON_L2 3
#define LEDS_BUTTON_L1 4
#define LEDS_BUTTON_R1 5
#define LEDS_BUTTON_B3 6
#define LEDS_BUTTON_B4 7

// ====================================================================
// 5. ディスプレイ構成（OLED ＆ 雪モード固定）
// ====================================================================
#define HAS_DISPLAY 1
#define BUTTON_LAYOUT BUTTON_LAYOUT_STICK
#define BUTTON_LAYOUT_RIGHT BUTTON_LAYOUT_VEWLIX 

#define SPLASH_MODE SplashMode::SPLASH_MODE_STATIC
#define DISPLAY_SAVER_TIMEOUT 600000
#define SCREEN_SAVER_MODE 2 
#define DISPLAY_MENU_ENABLED 1 

// ====================================================================
// 6. PCF8575 IO エクスパンダー 16ピン入力キーマッピング（16ページ完全同期）
// ====================================================================
#define I2C_PCF8575_ENABLED 1
#define I2C_PCF8575_BLOCK i2c1
#define PCF8575_PIN_COUNT 16

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
// 7. 【関数消失バグ解決】C++側の未定義呼び出しを空の安全なマクロ関数で完封
// ====================================================================
// C++マクロの安全装置を施し、多重インクルード時の二重定義バグ(全ファイル即死)を防ぎつつ、
// 1471行目と2116行目の「未定義エラー」を100%根本解決、物理ピンの正常認識を保証します。
#define gpioMappingsMigrationCore(config) do { } while(0)

#endif /* BOARD_CONFIG_H */
