/*
 * Fighting Stick MINI Super - Custom Board Configuration
 * Generated for Command Stick PS Project (Hardware Fixed Version)
 * 
 * This file hardcodes all pin assignments, forces addons, LED, Display, Hotkey, 
 * Wii Nunchuk, I2C Speed, and Core Controller configurations, bypassing WebConfig resets.
 */

#ifndef PICO_BOARD_CONFIG_H_
#define PICO_BOARD_CONFIG_H_

#include "enums.pb.h"
#include "class/hid/hid.h"
#include "GamepadEnums.h"

// ==========================================
// 1. 基本情報定義 (システム要求準拠)
// ==========================================
#define BOARD_CONFIG_LABEL "Fighting Stick MINI Super"

// ==========================================
// 2. コントローラー根幹動作設定 (USBHIDモード固定)
// ==========================================
#define DEFAULT_INPUT_MODE         INPUT_MODE_HID        // 初期入力モード: USBHID (DInput) に固定
#define DEFAULT_SOCD_MODE          SOCD_MODE_NEUTRAL     // SOCD: ニュートラル
#define DEFAULT_DPAD_MODE          DPAD_MODE_DIGITAL     // 十字キーモード: デジタル

#define DEFAULT_4WAY_MODE          1                     // 4方向ジョイスティックモード: 有効
#define DEFAULT_FORCED_SETUP_MODE  FORCED_SETUP_MODE_OFF // 強制セットアップモード: 無効
#define DEFAULT_PROFILE_NAME       "MINI Super"          // プロファイル名: MINI Super
#define DEFAULT_DEBOUNCE_DELAY     5                     // チャタリング除去ディレイ: 5ms
#define DISPLAY_MENU_GAMEPAD_INPUT_ENABLED 1             // ディスプレイメニューにゲームパッド入力利用: 有効

// ==========================================
// 3. 物理ピン（GPIO）マッピング (ソースコード仕様準拠)
// ==========================================

// 特殊周辺機器用ピン（アドオン割当宣言）
#define GPIO_PIN_00 GpioAction::ASSIGNED_TO_ADDON        // I2C0 SDA (Wii)
#define GPIO_PIN_01 GpioAction::ASSIGNED_TO_ADDON        // I2C0 SCL (Wii)
#define GPIO_PIN_18 GpioAction::ASSIGNED_TO_ADDON        // I2C1 SDA (PCF8575)
#define GPIO_PIN_19 GpioAction::ASSIGNED_TO_ADDON        // I2C1 SCL (PCF8575)
#define GPIO_PIN_20 GpioAction::ASSIGNED_TO_ADDON        // UART1 TX (JQ8900)
#define GPIO_PIN_21 GpioAction::ASSIGNED_TO_ADDON        // UART1 RX (JQ8900)
#define GPIO_PIN_27 GpioAction::ASSIGNED_TO_ADDON        // RGB LED データ端子
#define GPIO_PIN_28 GpioAction::ASSIGNED_TO_ADDON        // USB0+ (予約)
#define GPIO_PIN_29 GpioAction::ASSIGNED_TO_ADDON        // USB0- (予約)

// レバー4方向
#define GPIO_PIN_02 GpioAction::BUTTON_PRESS_UP          // レバー上
#define GPIO_PIN_03 GpioAction::BUTTON_PRESS_DOWN        // レバー下
#define GPIO_PIN_04 GpioAction::BUTTON_PRESS_RIGHT       // レバー右
#define GPIO_PIN_05 GpioAction::BUTTON_PRESS_LEFT        // レバー左

// 三和30φメインボタン（8個）
#define GPIO_PIN_06 GpioAction::BUTTON_PRESS_B1          // 弱P / Square
#define GPIO_PIN_07 GpioAction::BUTTON_PRESS_B2          // 中P / Triangle
#define GPIO_PIN_08 GpioAction::BUTTON_PRESS_R2          // 強K / R2
#define GPIO_PIN_09 GpioAction::BUTTON_PRESS_L2          // L1 / L2
#define GPIO_PIN_10 GpioAction::BUTTON_PRESS_B3          // 強P / R1
#define GPIO_PIN_11 GpioAction::BUTTON_PRESS_B4          // 弱K / Cross
#define GPIO_PIN_12 GpioAction::BUTTON_PRESS_R1          // 中K / Circle / R1
#define GPIO_PIN_13 GpioAction::BUTTON_PRESS_L1          // L2 / L1

// 機能・マクロボタン
#define GPIO_PIN_14 GpioAction::BUTTON_PRESS_TURBO       // Turboボタン
#define GPIO_PIN_17 GpioAction::BUTTON_PRESS_S2          // マクロ2 (S2) ※WebConfig起動ピン用

// アナログ入力（可変抵抗）
#define GPIO_PIN_26 GpioAction::ASSIGNED_TO_ADDON        // Turbo VR (ADC0)

// ==========================================
// 4. キーボードマッピング設定
// ==========================================
#define KEY_DPAD_UP          HID_KEY_ARROW_UP
#define KEY_DPAD_DOWN        HID_KEY_ARROW_DOWN
#define KEY_DPAD_RIGHT       HID_KEY_ARROW_RIGHT
#define KEY_DPAD_LEFT        HID_KEY_ARROW_LEFT
#define KEY_BUTTON_B1        HID_KEY_SHIFT_LEFT
#define KEY_BUTTON_B2        HID_KEY_Z
#define KEY_BUTTON_R2        HID_KEY_X
#define KEY_BUTTON_L2        HID_KEY_V
#define KEY_BUTTON_B3        HID_KEY_CONTROL_LEFT
#define KEY_BUTTON_B4        HID_KEY_ALT_LEFT
#define KEY_BUTTON_R1        HID_KEY_SPACE
#define KEY_BUTTON_L1        HID_KEY_C
#define KEY_BUTTON_S1        HID_KEY_5
#define KEY_BUTTON_S2        HID_KEY_1
#define KEY_BUTTON_L3        HID_KEY_EQUAL
#define KEY_BUTTON_R3        HID_KEY_MINUS
#define KEY_BUTTON_A1        HID_KEY_9
#define KEY_BUTTON_A2        HID_KEY_F2
#define KEY_BUTTON_FN        -1

// ==========================================
// 5. 周辺機器・アドオンのピンおよび通信速度固定マクロ
// ==========================================

// 最優先：UART1（JQ8900音声モジュール用）
#define UART1_TX_PIN   20    // JQ8900 RXへ接続
#define UART1_RX_PIN   21    // JQ8900 TXへ接続
#define UART_ENABLED   1     // UART機能を強制有効化

// I2C0：Wii拡張コントローラ用（高速400kHz固定）
#define I2C0_ENABLED   1     
#define I2CO_PIN_SDA   0     
#define I2CO_PIN_SCL   1     
#define I2C0_SPEED     400000 

// I2C1：PCF8575 IOエクスパンダー用（高速400kHz固定）
#define I2C1_ENABLED   1     
#define I2C1_PIN_SDA   18    
#define I2C1_PIN_SCL   19    
#define I2C1_SPEED     400000 

// LED関連ピン
#define PIN_BOARD_LED         25 // Raspberry Pi Pico オンボードLED
#define BOARD_LEDS_PIN        27 // RGB LEDデータ端子

#define TURBO_LED_PIN         15 
#define PIN_REACTIVE_LED_0    16 
#define PIN_REACTIVE_LED_1    22 
#define PIN_REACTIVE_LED_2    23 
#define PIN_REACTIVE_LED_3    24 

// ==========================================
// 6. アドオンの「強制有効化」マクロ
// ==========================================
#define DISPLAY_ENABLED            1  
#define WII_EXTENSION_ENABLED      1  
#define LEDS_ENABLED               1  
#define TURBO_ENABLED              1  
#define BOARD_IO_EXTENSION_ENABLED 1  

// ==========================================
// 7. オンボードLEDの動作モード固定
// ==========================================
#define BOARD_LED_MODE        BOARD_LED_MODE_INPUT_TEST 

// ==========================================
// 8. Wii拡張コントローラ（ヌンチャク仕様完全ハードコード）
// ==========================================
#define WII_EXTENSION_TYPE          WII_EXTENSION_NUNCHUK
#define WII_NUNCHUK_BUTTON_C        BUTTON_MASK_B1      
#define WII_NUNCHUK_BUTTON_Z        BUTTON_MASK_B2      
#define WII_NUNCHUK_STICK_MODE      STICK_MODE_LEFT     

// ==========================================
// 9. LED構成・動作プロファイル（変則省電力ケースLED対応）
// ==========================================
#define LED_COUNT            47       
#define LEDS_BASE_DATA_PIN   BOARD_LEDS_PIN 
#define LED_FORMAT           LED_FORMAT_GRB 
#define LED_LAYOUT           BUTTON_LAYOUT_STICK 
#define LEDS_PER_PIXEL       1        

#define LED_BRIGHTNESS_MAXIMUM 80 
#define LED_BRIGHTNESS_STEPS   10 
#define LEDS_TURN_OFF_ON_SUSPEND 1    

#define LED_CASE_START_INDEX 13  
#define LED_CASE_COUNT       34  

// 1つのボタンにつきLED1個アサイン
#define LEDS_BUTTON_B1  0
#define LEDS_BUTTON_B2  1
#define LEDS_BUTTON_R2  2
#define LEDS_BUTTON_L2  3
#define LEDS_BUTTON_L1  4
#define LEDS_BUTTON_R1  5
#define LEDS_BUTTON_B4  6
#define LEDS_BUTTON_B3  7

// ==========================================
// 10. ディスプレイ構成（表示レイアウト・各種モード完全固定）
// ==========================================
#define BUTTON_LAYOUT        BUTTON_LAYOUT_STICK     
#define BUTTON_LAYOUT_RIGHT  BUTTON_LAYOUT_VEWLIX    
#define BUTTON_LAYOUT_CUSTOM BUTTON_LAYOUT_DEFAULT   

#define DISPLAY_STATUS_BAR_INPUT_MODE 1 
#define DISPLAY_STATUS_BAR_TURBO      1 
#define DISPLAY_STATUS_BAR_DPAD_MODE  1 
#define DISPLAY_STATUS_BAR_SOCD_MODE  1 
#define DISPLAY_STATUS_BAR_MACRO     1 
#define DISPLAY_STATUS_BAR_PROFILE   1 

#define DISPLAY_INPUT_HISTORY_ENABLED 1  
#define DISPLAY_INPUT_HISTORY_LENGTH  21 
#define DISPLAY_INPUT_HISTORY_COL     0  
#define DISPLAY_INPUT_HISTORY_ROW     7  

#define DISPLAY_SPLASH_MODE          SPLASH_MODE_CUSTOM 
#define DISPLAY_SPLASH_DURATION      7                  
#define DISPLAY_SCREENSAVER_MODE     SCREENSAVER_MODE_SNOW 
#define DISPLAY_SCREENSAVER_TIMEOUT  10                 

// ==========================================
// 11. ホットキー初期設定の強制上書き
// ==========================================
#define HOTKEY_01_ACTION             HOTKEY_ACTION_DISPLAY_MENU
#define HOTKEY_01_BUTTON_01          GAMEPAD_MASK_S2
#define HOTKEY_01_BUTTON_02          GAMEPAD_MASK_A2
#define HOTKEY_01_BUTTON_03          0 

// ==========================================
// 12. PCF8575 IO エクスパンダー設定（すべて入力固定）
// ==========================================
#define PCF8575_DIRECTION_MASK 0xFFFF 

#define PCF8575_P00_ASSIGN GAMEPAD_MASK_A3      // GP0 : A3
#define PCF8575_P01_ASSIGN GAMEPAD_MASK_A2      // GP1 : A2
#define PCF8575_P02_ASSIGN GAMEPAD_MASK_E1      // GP2 : Extra 1
#define PCF8575_P03_ASSIGN GAMEPAD_MASK_E2      // GP3 : Extra 2
#define PCF8575_P04_ASSIGN GAMEPAD_MASK_E3      // GP4 : Extra 3
#define PCF8575_P05_ASSIGN GAMEPAD_MASK_E4      // GP5 : Extra 4
#define PCF8575_P06_ASSIGN GAMEPAD_MASK_E5      // GP6 : Extra 5
#define PCF8575_P07_ASSIGN GAMEPAD_MASK_E6      // GP7 : Extra 6

#define PCF8575_P10_ASSIGN GAMEPAD_MASK_A4      // GP8 : A4
#define PCF8575_P11_ASSIGN GAMEPAD_MASK_L3      // GP9 : L3
#define PCF8575_P12_ASSIGN GAMEPAD_MASK_R3      // GP10: R3
#define PCF8575_P13_ASSIGN GAMEPAD_MASK_S1      // GP11: S1
#define PCF8575_P14_ASSIGN GAMEPAD_MASK_A1      // GP12: A1
#define PCF8575_P16_ASSIGN GAMEPAD_MASK_E7      // GP14: Extra 7
#define PCF8575_P17_ASSIGN GAMEPAD_MASK_E8      // GP15: Extra 8

#endif // PICO_BOARD_CONFIG_H_
