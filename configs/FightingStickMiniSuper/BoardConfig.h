#ifndef BOARD_CONFIG_H
#define BOARD_CONFIG_H

#include <stdint.h>
#include "enums.pb.h"
#include "class/hid/hid.h"

#define BOARD_CONFIG_LABEL "MINI Super"

// ====================================================================
// 1. 物理ピン（GPIO）の完全固定マッピング
// ====================================================================
#define GPIO_PIN_02 GpioAction::BUTTON_PRESS_UP
#define GPIO_PIN_03 GpioAction::BUTTON_PRESS_DOWN
#define GPIO_PIN_04 GpioAction::BUTTON_PRESS_RIGHT
#define GPIO_PIN_05 GpioAction::BUTTON_PRESS_LEFT

#define GPIO_PIN_06 GpioAction::BUTTON_PRESS_B1
#define GPIO_PIN_07 GpioAction::BUTTON_PRESS_B2
#define GPIO_PIN_08 GpioAction::BUTTON_PRESS_R2
#define GPIO_PIN_09 GpioAction::BUTTON_PRESS_L2
#define GPIO_PIN_10 GpioAction::BUTTON_PRESS_B3
#define GPIO_PIN_11 GpioAction::BUTTON_PRESS_B4
#define GPIO_PIN_12 GpioAction::BUTTON_PRESS_R1
#define GPIO_PIN_13 GpioAction::BUTTON_PRESS_L1

// 機能ボタン (S2/STARTは通常のゲーム中に100%通常ボタンとして機能させます)
#define GPIO_PIN_14 GpioAction::BUTTON_PRESS_TURBO
#define GPIO_PIN_17 GpioAction::BUTTON_PRESS_S2

// 各種アドオン/周辺機器ピンをADDON管轄へリリース
#define GPIO_PIN_15 GpioAction::ASSIGNED_TO_ADDON // Turbo_LED
#define GPIO_PIN_16 GpioAction::ASSIGNED_TO_ADDON // Player LED 1
#define GPIO_PIN_22 GpioAction::ASSIGNED_TO_ADDON // Player LED 2
#define GPIO_PIN_23 GpioAction::ASSIGNED_TO_ADDON // Player LED 3
#define GPIO_PIN_24 GpioAction::ASSIGNED_TO_ADDON // Player LED 4
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
// 2. 周辺機器・アドオンの完全固定マクロ
// ====================================================================
#define DEFAULT_INPUT_MODE INPUT_MODE_GENERIC
#define DEFAULT_SOCD_MODE SOCD_MODE_NEUTRAL
#define DEFAULT_DPAD_MODE DPAD_MODE_DIGITAL
#define DEBOUNCE_DELAY_IN_MS 5

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
// 3. LED構成・ディスプレイ・IOエクスパンダー標準定義
// ====================================================================
#define BOARD_LEDS_ENABLED 1
#define RGB_LED_NUM 47
#define LED_BRIGHTNESS_MAXIMUM 80
#define LED_BRIGHTNESS_STEPS 10
#define LED_FORMAT LED_FORMAT_GRB
#define LED_LAYOUT BUTTON_LAYOUT_STICK

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

#define PCF8575_ENABLED 1
#define PCF8575_I2C_BLOCK i2c1

// PCF8575の全16ピンの役割を、C++側が直接同期ロードできる正規マクロ名として完全ロック
#define PCF8575_PIN_00_ACTION GpioAction::BUTTON_PRESS_A3
#define PCF8575_PIN_01_ACTION GpioAction::BUTTON_PRESS_A2
#define PCF8575_PIN_02_ACTION GpioAction::BUTTON_PRESS_E1
#define PCF8575_PIN_03_ACTION GpioAction::BUTTON_PRESS_E2
#define PCF8575_PIN_04_ACTION GpioAction::BUTTON_PRESS_E3
#define PCF8575_PIN_05_ACTION GpioAction::BUTTON_PRESS_E4
#define PCF8575_PIN_06_ACTION GpioAction::BUTTON_PRESS_E5
#define PCF8575_PIN_07_ACTION GpioAction::BUTTON_PRESS_E6
#define PCF8575_PIN_10_ACTION GpioAction::BUTTON_PRESS_A4
#define PCF8575_PIN_11_ACTION GpioAction::BUTTON_PRESS_L3
#define PCF8575_PIN_12_ACTION GpioAction::BUTTON_PRESS_R3
#define PCF8575_PIN_13_ACTION GpioAction::BUTTON_PRESS_S1
#define PCF8575_PIN_14_ACTION GpioAction::BUTTON_PRESS_A1
#define PCF8575_PIN_15_ACTION GpioAction::NONE
#define PCF8575_PIN_16_ACTION GpioAction::BUTTON_PRESS_E7
#define PCF8575_PIN_17_ACTION GpioAction::BUTTON_PRESS_E8

#endif /* BOARD_CONFIG_H */
