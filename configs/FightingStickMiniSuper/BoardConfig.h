/*
 * Fighting Stick MINI Super - Custom Board Configuration
 * Generated for Command Stick PS Project (Hardware Fixed Version)
 * 
 * This file hardcodes all pin assignments, forces addons, LED, Display, Hotkey, 
 * Wii Nunchuk, I2C Speed, and Core Controller configurations, bypassing WebConfig resets.
 */

#ifndef BOARD_CONFIG_H_
#define BOARD_CONFIG_H_

#include <stdint.h>
#include "GamepadEnums.h" // ボタン・入力定数・SOCD定数を使用するためにインクルード

// ==========================================
// 1. 基本情報定義
// ==========================================
#define BOARD_NAME "Fighting Stick MINI Super"

// ==========================================
// 2. コントローラー根幹動作設定
// ==========================================
#define DEFAULT_INPUT_MODE         INPUT_MODE_PS4        // 初期入力モード: PS4モード
#define DEFAULT_SOCD_MODE          SOCD_MODE_NEUTRAL     // SOCD: ニュートラル（上下同時＝N / 左右同時＝N）
#define DEFAULT_DPAD_MODE          DPAD_MODE_DIGITAL     // 十字キーモード: デジタル

// ご提示の追加仕様を完全ハードコード
#define DEFAULT_4WAY_MODE          1                     // 4方向ジョイスティックモード: 有効
#define DEFAULT_FORCED_SETUP_MODE  FORCED_SETUP_MODE_OFF // 強制セットアップモード: 無効
#define DEFAULT_PROFILE_NAME       "MINI Super"         // プロファイル名: MINI Super
#define DEFAULT_DEBOUNCE_DELAY     5                     // チャタリング除去ディレイ: 5ms
#define DISPLAY_MENU_GAMEPAD_INPUT_ENABLED 1             // ディスプレイメニューの入力にゲームパッド入力を利用: 有効

// ==========================================
// 3. 物理ピン（GPIO）の完全固定マクロ
// ==========================================

// レバー4方向
#define PIN_DPAD_UP    2     // レバー上
#define PIN_DPAD_DOWN  3     // レバー下
#define PIN_DPAD_RIGHT 4     // レバー右
#define PIN_DPAD_LEFT  5     // レバー左

// 三和30φメインボタン（8個）
#define PIN_BUTTON_B1  6     // 弱P / Square
#define PIN_BUTTON_B2  7     // 中P / Triangle
#define PIN_BUTTON_R2  8     // 強K / R2
#define PIN_BUTTON_L2  9     // L1 / L2
#define PIN_BUTTON_B3  10    // 強P / R1
#define PIN_BUTTON_B4  11    // 弱K / Cross
#define PIN_BUTTON_R1  12    // 中K / Circle / R1
#define PIN_BUTTON_L1  13    // L2 / L1

// 機能・マクロボタン
#define PIN_BUTTON_TURBO 14  // Turboボタン
#define PIN_BUTTON_S2    17  // マクロ2 (S2) ※WebConfig起動ピン用

// アナログ入力（ボリューム等）
#define PIN_ANALOG_TURBO_VR 26 // Turbo速度調整用可変抵抗 (ADC0)

// ==========================================
// 4. 周辺機器・アドオンのピンおよび通信速度固定マクロ
// ==========================================

// 最優先：UART1（JQ8900音声モジュール用）
#define UART1_TX_PIN   20    // JQ8900 RXへ接続
#define UART1_RX_PIN   21    // JQ8900 TXへ接続
#define UART_ENABLED   1     // UART機能を強制有効化

// I2C0：Wii拡張コントローラ用（高速400kHz固定）
#define I2C0_ENABLED   1     // I2C0を有効化
#define I2C0_SDA_PIN   0     // Wii SDA
#define I2C0_SCL_PIN   1     // Wii SCL
#define I2C0_SPEED     400000 // 通信速度を400kHzに強制固定

// I2C1：PCF8575 IOエクスパンダー用（高速400kHz固定）
#define I2C1_ENABLED   1     // I2C1を有効化
#define I2C1_SDA_PIN   18    // PCF8575 SDA
#define I2C1_SCL_PIN   19    // PCF8575 SCL
#define I2C1_SPEED     400000 // 通信速度を400kHzに強制固定

// LED関連ピン
#define PIN_BOARD_LED         25 // Raspberry Pi Pico オンボードLED (GP25)
#define BOARD_LEDS_PIN        27 // RGB LEDデータ端子 (GP27)

#define PIN_TURBO_LED         15 // Turbo状態インジケータLED
#define PIN_REACTIVE_LED_0    16 // リアクティブLED 0
#define PIN_REACTIVE_LED_1    22 // リアクティブLED 1
#define PIN_REACTIVE_LED_2    23 // リアクティブLED 2
#define PIN_REACTIVE_LED_3    24 // リアクティブLED 3

// ==========================================
// 5. アドオンの「強制有効化」マクロ
// ==========================================
#define DISPLAY_ENABLED            1  // ディスプレイ（OLED）アドオン強制有効
#define WII_EXTENSION_ENABLED      1  // Wii拡張コントローラアドオン強制有効
#define LEDS_ENABLED               1  // WS2812B LEDアドオン強制有効
#define TURBO_ENABLED              1  // Turbo機能強制有効
#define BOARD_IO_EXTENSION_ENABLED 1  // PCF8575等の拡張IOアドオン強制有効

// ==========================================
// 6. オンボードLEDの動作モード固定
// ==========================================
#define BOARD_LED_MODE        BOARD_LED_MODE_INPUT_TEST // 入力テストモードに固定

// ==========================================
// 7. Wii拡張コントローラ（ヌンチャク仕様完全ハードコード）
// ==========================================
#define WII_EXTENSION_TYPE          WII_EXTENSION_NUNCHUK
#define WII_NUNCHUK_BUTTON_C        BUTTON_MASK_B1      // Cボタン ＝ B1
#define WII_NUNCHUK_BUTTON_Z        BUTTON_MASK_B2      // Zボタン ＝ B2
#define WII_NUNCHUK_STICK_MODE      STICK_MODE_LEFT     // スティック ＝ Left Analog

// ==========================================
// 8. LED構成・動作プロファイル（変則省電力ケースLED対応）
// ==========================================
#define LED_COUNT            47       
#define LEDS_BASE_DATA_PIN   BOARD_LEDS_PIN 
#define LED_FORMAT           LED_FORMAT_GRB 
#define LED_LAYOUT           BUTTON_LAYOUT_STICK 
#define LEDS_PER_PIXEL       1        

#define LED_MAX_BRIGHTNESS   80       
#define LED_BRIGHTNESS_STEPS 10       
#define LEDS_TURN_OFF_ON_SUSPEND 1    

#define LED_CASE_START_INDEX 13  
#define LED_CASE_COUNT       34  

#define LED_PIN_00 BUTTON_MASK_B1  
#define LED_PIN_01 BUTTON_MASK_B2  
#define LED_PIN_02 BUTTON_MASK_R2  
#define LED_PIN_03 BUTTON_MASK_L2  
#define LED_PIN_04 BUTTON_MASK_L1  
#define LED_PIN_05 BUTTON_MASK_R1  
#define LED_PIN_06 BUTTON_MASK_B4  
#define LED_PIN_07 BUTTON_MASK_B3  

// ==========================================
// 9. ディスプレイ構成（表示レイアウト・各種モード完全固定）
// ==========================================
#define BUTTON_LAYOUT_LEFT   BUTTON_LAYOUT_STICK     
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
// 10. ホットキー初期設定の強制上書き
// ==========================================
#define HOTKEY_01_ACTION             HOTKEY_ACTION_DISPLAY_MENU
#define HOTKEY_01_BUTTON_01          GAMEPAD_MASK_S2
#define HOTKEY_01_BUTTON_02          GAMEPAD_MASK_A2
#define HOTKEY_01_BUTTON_03          0 

// ==========================================
// 11. PCF8575 IO エクスパンダー設定（すべて入力固定）
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

#endif // BOARD_CONFIG_H_
