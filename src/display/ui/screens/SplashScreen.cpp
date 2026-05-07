#include "SplashScreen.h"
#include "pico/stdlib.h"
#include "drivermanager.h"
#include "logos.h" // 作成したロゴデータを読み込み

void SplashScreen::init() {
    getRenderer()->clearScreen();
    splashStartTime = getMillis();
    configMode = DriverManager::getInstance().isConfigMode();
}

void SplashScreen::shutdown() {
    clearElements();
}

void SplashScreen::drawScreen() {
    uint32_t elapsed = getMillis() - splashStartTime;

    // --- 1. 起動から1.0秒間は「溜め」（何も表示しない） ---
    // JQ8900が機種判定を待っている時間と同期させます
    if (elapsed < 1000) {
        getRenderer()->clearScreen();
        return;
    }

    // --- 2. 1.0秒経過後：音が鳴るタイミングで機種別ロゴを表示 ---
    InputMode mode = DriverManager::getInstance().getInputMode();
    const uint8_t* selectedLogo = nullptr;

    // InputMode（機種）に応じて表示するロゴを切り替え
    switch (mode) {
        case INPUT_MODE_XINPUT:      selectedLogo = L_XINPUT; break;         // ID 0001
        case INPUT_MODE_SWITCH:      selectedLogo = L_SWITCH; break;         // ID 0002
        case INPUT_MODE_HID:         selectedLogo = L_PS3; break;            // ID 0003 (D-Input)
        case INPUT_MODE_PS4:         selectedLogo = L_PS4; break;            // ID 0004
        case INPUT_MODE_XBOX360:     selectedLogo = L_XBOX360; break;        // ID 0005
        case INPUT_MODE_XBUI:        selectedLogo = L_XBOXONE; break;        // ID 0006
        case INPUT_MODE_KEYBOARD:    selectedLogo = L_KEYBOARD; break;       // ID 0007
        case INPUT_MODE_NEOGEO:      selectedLogo = L_NEOGEO; break;         // ID 0008
        case INPUT_MODE_SFC:         selectedLogo = L_CLASSIC_MINI; break;   // ID 0009
        case INPUT_MODE_PCEngine:    selectedLogo = L_PCENGINE; break;       // ID 0010
        case INPUT_MODE_MD:          selectedLogo = L_MEGADRIVE; break;      // ID 0011
        case INPUT_MODE_XBOXORIGINAL: selectedLogo = L_XBOX_OG; break;       // ID 0012
        case INPUT_MODE_SWITCH_PRO:  selectedLogo = L_SWITCH_PRO; break;     // ID 0013
        case INPUT_MODE_SATURN:      selectedLogo = L_SATURN; break;         // ID 0014
        case INPUT_MODE_DREAMCAST:   selectedLogo = L_DREAMCAST; break;      // ID 0015
        case INPUT_MODE_PSCLASSIC:   selectedLogo = L_PS_CLASSIC; break;      // ID 0016
        case INPUT_MODE_ASTRO:       selectedLogo = L_ASTRO_CITY; break;     // ID 0017
        case INPUT_MODE_EGRET:       selectedLogo = L_EGRET2; break;         // ID 0018
        case INPUT_MODE_PS5:         selectedLogo = L_PS5; break;            // ID 0020
        
        default:
            // 該当がない場合はWebConfigで設定されたデフォルト画像を表示
            selectedLogo = (uint8_t*) getDisplayOptions().splashImage.bytes;
            break;
    }

    if (selectedLogo != nullptr) {
        // 全画面(128x64)でロゴを描画
        // 引数: データ, 幅, 高さ, 1ラインあたりのバイト数(128/8=16), X, Y, カラー
        getRenderer()->drawSprite((uint8_t*)selectedLogo, 128, 64, 16, 0, 0, 1);
    }
}

int8_t SplashScreen::update() {
    uint32_t elapsedDuration = getMillis() - splashStartTime;
    
    // ロゴを最低でも1.5秒間は見せるため、トータルの表示時間を2.5秒以上に強制する
    uint32_t splashDuration = getDisplayOptions().splashDuration;
    if (splashDuration < 2500) splashDuration = 2500; 

    if (!configMode) {
        // 通常起動時：設定時間が来たら対戦画面（BUTTONS）へ
        if (elapsedDuration >= splashDuration) {
            return DisplayMode::BUTTONS;
        }
    } else {
        // WebConfig（S2）起動時：B2ボタンで設定指示画面へ
        uint16_t buttonState = getGamepad()->state.buttons;
        if (prevButtonState && !buttonState) {
            if (prevButtonState == GAMEPAD_MASK_B2) {
                prevButtonState = 0;
                return DisplayMode::CONFIG_INSTRUCTION;
            }
        }
        prevButtonState = buttonState;
    }
    return -1; // 状態維持
}
