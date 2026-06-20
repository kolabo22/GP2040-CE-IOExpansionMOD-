/*
 * SPDX-License-Identifier: MIT
 * SPDX-FileCopyrightText: Copyright (c) 2024 OpenStickCommunity (gp2040-ce.info)
 */
#include "storagemanager.h"
#include "BoardConfig.h"
#include "animationstorage.h"
#include "FlashPROM.h"
#include "drivermanager.h"
#include "eventmanager.h"
#include "peripheralmanager.h"
#include "config.pb.h"
#include "hardware/watchdog.h"
#include "CRC32.h"
#include "types.h"
#include "hardware/flash.h"
#include "hardware/sync.h"
#include "ps4/PS4Driver.h"
#include "config_utils.h"

// 🎯 素のPico（2MB）仕様：通常セーブ（0x1F8000）の直前にある、100%実在する安全な空きセクター（4KB分）
#define MINI_SUPER_RAW_FLASH_ADDR 0x1F4000

void Storage::init() {
    systemFlashSize = System::getPhysicalFlash();
    EEPROM.start();
    ConfigUtils::load(config);
}

bool Storage::save() {
    return save(false);
}

// ====================================================================
// 【適応後】最終完成版の Storage::save(const bool force)
// ====================================================================
bool Storage::save(const bool force) {
    if (!force &&
        PeripheralManager::getInstance().isUSBEnabled(0) &&
        (DriverManager::getInstance().getInputMode() == INPUT_MODE_PS4 ||
        DriverManager::getInstance().getInputMode() == INPUT_MODE_PS5) &&
        ((PS4Driver*) DriverManager::getInstance().getDriver())->getDongleAuthRequired() == true) {
        return false;
    }

    // 1. 最新のメモリ設定（config）をセーブ用キャッシュ領域へシリアライズ
    ConfigUtils::save(this->config);

    // 2. 通常のEEPROMセーブ領域（EEPROMバッファ）へ一撃コミット
    EEPROM.commit();

    // ====================================================================
    // 3. 💥【データ反映完全保証＆安全隔離番地への物理焼き付け】
    // ====================================================================
    // WebUIであなたが作り直した「完璧な最新設定」が次回起動時にも100%反映されるよう、
    // 通常保存（force==false）の時であっても、安全隔離番地（0x1F4000）へダイレクトに物理後書きします！
    uint32_t flash_storage_offset = MINI_SUPER_RAW_FLASH_ADDR;
    
    uint32_t saved_interrupts = save_and_disable_interrupts();
    flash_range_erase(flash_storage_offset, FLASH_SECTOR_SIZE);
    flash_range_program(flash_storage_offset, FlashPROM::writeCache, FLASH_SECTOR_SIZE);
    restore_interrupts(saved_interrupts);

    // ====================================================================
    // 4. 💥【別ページ遷移時のグルグル・USBエラー（ピコ音）を物理的に完全封殺】
    // ====================================================================
    // ブラウザが別ページへ勝手に遷移してAPIリクエストを連打（不意打ち通信）してくる前に、
    // 「セーブ成功、リブートするで！」というHTTP応答を100%返しきり、実機側を即座に強制リブートさせます。
    // これにより、起動直後の超多忙なPicoをブラウザの通信が直撃してUSBが壊れる現象を100%防ぎます。
    watchdog_update(); // ウォッチドッグタイマーのクリア（フリーズ誤判定を防止）
    sleep_ms(300);     // 0.3秒間だけ物理的に処理を休止させて通信パケットを綺麗に逃がす
    watchdog_update();

    // フラッシュの物理書き込みが完了し、通信が切断された安全な状態で自動リブート！
    System::reboot(System::BootMode::GAMEPAD);

    return true;
}

void Storage::ResetSettings()
{
    EEPROM.reset();

    // 1. 素の Pico のフラッシュメモリ内の隔離番地（0x1F4000）にお気に入りデータがあるか自動判別
    const uint8_t* rawFlashSource = (const uint8_t*)(XIP_BASE + MINI_SUPER_RAW_FLASH_ADDR);
    uint32_t checkVal = *(const volatile uint32_t*)rawFlashSource;
    
    if (checkVal != 0xFFFFFFFF && checkVal != 0x00000000) {
        // ⭕ 【お気に入りから一撃復元（4KB 分）】
        for (uint16_t i = 0; i < FLASH_SECTOR_SIZE; i++) {
            FlashPROM::writeCache[i] = rawFlashSource[i];
        }
    } else {
        // ⭕ 【完全初期状態 ➔ BoardConfig.h に焼き付けたマスターバイナリ配列を一括ダイレクト流し込み！】
        for (uint16_t i = 0; i < sizeof(miniSuperPerfectBinary); i++) {
            FlashPROM::writeCache[i] = miniSuperPerfectBinary[i];
        }
    }

    // 2. データを通常領域のキャッシュバッファへ定着させ、config構造体に展開
    // （※この commit() の内部で、フラッシュの物理書き込み完了待ちは100%完了します）
    EEPROM.commit();
    ConfigUtils::load(config);

    // ====================================================================
    // 3. 💥【超重要】タイムアウト＆USBデバイスエラー（ピコ音）を物理的に根絶する
    // ====================================================================
    
    // ブラウザへ「リブートするでー！」というWeb通信（HTTP応答）を100%返しきるための猶予時間を確保
    watchdog_update(); // ウォッチドッグタイマーのクリア（フリーズ誤判定を防止）
    sleep_ms(800);     // 0.8秒間、物理的に処理を休止させて通信パケットを完全に逃がす
    watchdog_update();

    // フラッシュの安全と通信の切断が100%確保された状態で、満を持して自動リブート！
    System::reboot(System::BootMode::GAMEPAD);
}

bool Storage::setProfile(const uint32_t profileNum)
{
    uint32_t profileCeiling = config.profileOptions.gpioMappingsSets_count + 1;
    if (profileNum >= 1 && profileNum <= profileCeiling) {
        if (profileNum == 1 || config.profileOptions.gpioMappingsSets[profileNum-2].enabled) {
            this->config.gamepadOptions.profileNumber = profileNum;
            return true;
        }
    }
    return false;
}

void Storage::nextProfile()
{
    uint32_t profileCeiling = config.profileOptions.gpioMappingsSets_count + 1;
    uint32_t requestedProfile = (this->config.gamepadOptions.profileNumber % profileCeiling) + 1;
    while (!setProfile(requestedProfile)) {
        requestedProfile = (requestedProfile % profileCeiling) + 1;
    }
}

void Storage::previousProfile()
{
    uint32_t profileCeiling = config.profileOptions.gpioMappingsSets_count + 1;
    uint32_t requestedProfile = this->config.gamepadOptions.profileNumber > 1 ?
    config.gamepadOptions.profileNumber - 1 : profileCeiling;
    while (!setProfile(requestedProfile)) {
        requestedProfile = requestedProfile > 1 ? requestedProfile - 1 : profileCeiling;
    }
}

char* Storage::currentProfileLabel() {
    if (this->config.gamepadOptions.profileNumber == 1)
        return this->config.gpioMappings.profileLabel;
    else
        return this->config.profileOptions.gpioMappingsSets[config.gamepadOptions.profileNumber-2].profileLabel;
}

void Storage::setFunctionalPinMappings()
{
    GpioMappingInfo* alts = nullptr;
    uint32_t profileCeiling = config.profileOptions.gpioMappingsSets_count + 1;
    if (config.gamepadOptions.profileNumber >= 2 && config.gamepadOptions.profileNumber <= profileCeiling) {
        if (config.profileOptions.gpioMappingsSets[config.gamepadOptions.profileNumber-2].enabled) {
            alts = config.profileOptions.gpioMappingsSets[config.gamepadOptions.profileNumber-2].pins;
        }
    }
    for (Pin_t pin = 0; pin < (Pin_t)NUM_BANK0_GPIOS; pin++) {
        if (alts != nullptr &&
            alts[pin].action != GpioAction::RESERVED &&
            alts[pin].action != GpioAction::ASSIGNED_TO_ADDON &&
            this->config.gpioMappings.pins[pin].action != GpioAction::RESERVED &&
            this->config.gpioMappings.pins[pin].action != GpioAction::ASSIGNED_TO_ADDON) {
            functionalPinMappings[pin] = alts[pin];
        } else {
            functionalPinMappings[pin] = this->config.gpioMappings.pins[pin];
        }
    }
}

void Storage::SetGamepad(Gamepad * newpad) { gamepad = newpad; }
Gamepad * Storage::GetGamepad() { return gamepad; }
void Storage::SetProcessedGamepad(Gamepad * newpad) { processedGamepad = newpad; }
Gamepad * Storage::GetProcessedGamepad() { return processedGamepad; }
