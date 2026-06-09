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

// 機能ボタン (TURBOは維持、S2はOLED表示の蛇足排除のためADDONへ逃がす)
#define GPIO_PIN_14 GpioAction::BUTTON_PRESS_TURBO
#define GPIO_PIN_17 GpioAction::ASSIGNED_TO_ADDON

// 連射(Turbo)LED・物理Player LEDアサインの完全固定
#define GPIO_PIN_15 GpioAction::ASSIGNED_TO_ADDON // Turbo_LED (GP15)
#define GPIO_PIN_16 GpioAction::ASSIGNED_TO_ADDON // Player LED 1 (GP16)
#define GPIO_PIN_22 GpioAction::ASSIGNED_TO_ADDON // Player LED 2 (GP22)
#define GPIO_PIN_23 GpioAction::ASSIGNED_TO_ADDON // Player LED 3 (GP23)
#define GPIO_PIN_24 GpioAction::ASSIGNED_TO_ADDON // Player LED 4 (GP24)

// オンボードLED (入力テストモード固定)
#define BOARD_LED_TYPE ON_BOARD_LED_MODE_INPUT_TEST 
#define GPIO_PIN_25 GpioAction::ASSIGNED_TO_ADDON

// 周辺機器・アドオン割当用ピン (GPIOピン定義からアクションを除外)
#define GPIO_PIN_00 GpioAction::ASSIGNED_TO_ADDON // I2C0 SDA (Wii)
#define GPIO_PIN_01 GpioAction::ASSIGNED_TO_ADDON // I2C0 SCL (Wii)
#define GPIO_PIN_18 GpioAction::ASSIGNED_TO_ADDON // I2C1 SDA (PCF8575)
#define GPIO_PIN_19 GpioAction::ASSIGNED_TO_ADDON // I2C1 SCL (PCF8575)
#define GPIO_PIN_20 GpioAction::ASSIGNED_TO_ADDON // UART1 TX (JQ8900)
#define GPIO_PIN_21 GpioAction::ASSIGNED_TO_ADDON // UART1 RX (JQ8900)
#define GPIO_PIN_26 GpioAction::ASSIGNED_TO_ADDON // Analog (Turbo VR)
#define GPIO_PIN_27 GpioAction::ASSIGNED_TO_ADDON // RGB LED

// USBホスト拡張用ピンのシステム側完全ロック (D+ = GP28, D- = GP29)
#define GPIO_PIN_28 GpioAction::ASSIGNED_TO_ADDON // USB0 D+
#define GPIO_PIN_29 GpioAction::ASSIGNED_TO_ADDON // USB0 D-

#define FORCED_WEB_CONFIG_BOARD_BUTTON_PIN 17 // GP17でWebConfig起動

// ====================================================================
// 2. 周辺機器・アドオンの完全固定マクロ (基本設定)
// ====================================================================
#define DEFAULT_INPUT_MODE INPUT_MODE_GENERIC
#define DEFAULT_SOCD_MODE SOCD_MODE_NEUTRAL
#define DEFAULT_DPAD_MODE DPAD_MODE_DIGITAL
#define DEBOUNCE_DELAY_IN_MS 5

// I2C 0 (Wii拡張) 強制有効化
#define I2C0_ENABLED 1
#define I2C0_PIN_SDA 0
#define I2C0_PIN_SCL 1
#define I2C0_SPEED 400000

// I2C 1 (PCF8575) 強制有効化
#define I2C1_ENABLED 1
#define I2C1_PIN_SDA 18
#define I2C1_PIN_SCL 19
#define I2C1_SPEED 400000

// UART 1 (JQ8900) 強制有効化
#define UART1_ENABLED 1
#define UART1_PIN_TX 20
#define UART1_PIN_RX 21
#define UART1_BAUDRATE 9600

// 周辺機器USBホスト機能のピン完全定義（常時5V給電）
#define USB_PERIPHERAL_ENABLED 1
#define USB_PIN_DP 28
#define USB_PIN_VBUS_ENABLE -1

// Wii 拡張コントローラー（ヌンチャク詳細アサイン完全連動）
#define WII_EXTENSION_ENABLED 1
#define WII_EXTENSION_I2C_BLOCK i2c0
#define WII_NUNCHUK_BUTTON_C GpioAction::BUTTON_PRESS_B1
#define WII_NUNCHUK_BUTTON_Z GpioAction::BUTTON_PRESS_B2
#define WII_NUNCHUK_STICK_MODE 1 // Left Analog固定

// 連射（Turbo）詳細アサイン完全連動（GP14ボタン、GP15LED、GP26アナログVR速度制御）
#define TURBO_ENABLED 1
#define TURBO_PIN 14
#define TURBO_LED_PIN 15
#define TURBO_SHOT_PIN 26 // 可変抵抗(Turbo VR)による速度制御をマクロ固定

// 物理Player LED（リアクティブLED）アサイン ＆ 動作モード完全固定
#define PLAYER_LEDS_ENABLED 1
#define PLAYER_LED_PIN_P1 16
#define PLAYER_LED_PIN_P2 22
#define PLAYER_LED_PIN_P3 23
#define PLAYER_LED_PIN_P4 24
#define PLAYER_LED_REACTIVE_MODE 2 // 2 = PLAYER_LED_REACTIVE_FADEOUT（押すとフェードアウト、離すとフェードイン）

// ====================================================================
// 3. LED構成・動作プロファイル（変則省電力ケースLED対応）
// ====================================================================
#define BOARD_LEDS_ENABLED 1
#define RGB_LED_NUM 47
#define LED_BRIGHTNESS_MAXIMUM 80
#define LED_BRIGHTNESS_STEPS 10
#define LED_FORMAT LED_FORMAT_GRB
#define LED_LAYOUT BUTTON_LAYOUT_STICK

// 【確定】インデックス14（物理15個目）から始まる34個分をケースLEDとして厳密固定
#define LED_CASE_START_INDEX 14
#define LED_CASE_COUNT 34

// ボタン1つにつきLED1個の1対1直列配線順（×→○→R2→L2→L1→R1→△→□）
// GP2040-CEの標準配列インデックス（0~7）に1対1で完全同期
#define LED_PINS_MAPPING { 0, 1, 2, 3, 4, 5, 6, 7 }

// ====================================================================
// 4. ディスプレイ構成（OLED）
// ====================================================================
#define HAS_DISPLAY 1
#define DISPLAY_I2C_BLOCK i2c0 
#define DISPLAY_FLIP 0
#define DISPLAY_INVERT 0

#define BUTTON_LAYOUT_LEFT BUTTON_LAYOUT_STICK
#define BUTTON_LAYOUT_RIGHT BUTTON_LAYOUT_VEWLIX // 右側をアーケード・ビューリックスに固定

#define CUSTOM_SPLASH_IMAGE 1
#define CUSTOM_SPLASH_TIME 7000
#define SCREEN_SAVER_TIMEOUT 600000
#define SCREEN_SAVER_MODE 2 // 「雪」モード固定

#define DISPLAY_MENU_ENABLED 1 // ゲームパッド入力でのメニュー操作有効

// ====================================================================
// 5. PCF8575 IO エクスパンダー入力マッピング (I2C1)
// ====================================================================
#define PCF8575_ENABLED 1
#define PCF8575_I2C_BLOCK i2c1

// PCF8575の全16ピンの初期割当識別コード数値をマクロレベルで完全バインド
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
#define PCF8575_PIN_15_ACTION 0  // 0 = GpioAction::NONE (無し/スキップ)
#define PCF8575_PIN_16_ACTION 27
#define PCF8575_PIN_17_ACTION 28

#endif /* BOARD_CONFIG_H */
