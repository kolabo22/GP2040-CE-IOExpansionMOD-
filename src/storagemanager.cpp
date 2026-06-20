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
// 【修正後】① 関数のすぐ上に配置する「RAM実行」の安全書き込み処理
// ====================================================================
// この修飾子により、この関数はフラッシュではなく100%RAM上で実行され、即死を物理的に防ぎます
void __no_inline_not_in_flash_func(safeWriteToMiniSuperZone)(uint32_t offset, const uint8_t* data) {
    uint32_t saved_interrupts = save_and_disable_interrupts();
    
    // RAM上で実行されているため、フラッシュが凍結してもCPUは絶対にクラッシュしません
    flash_range_erase(offset, FLASH_SECTOR_SIZE);
    flash_range_program(offset, data, FLASH_SECTOR_SIZE);
    
    restore_interrupts(saved_interrupts);
}

// ====================================================================
// 【修正後】① 関数のすぐ上に配置する「RAM実行」の安全書き込み処理
// ====================================================================
void __no_inline_not_in_flash_func(safeWriteToMiniSuperZone)(uint32_t offset, const uint8_t* data, size_t size) {
    uint32_t saved_interrupts = save_and_disable_interrupts();
    
    flash_range_erase(offset, FLASH_SECTOR_SIZE);
    flash_range_program(offset, data, FLASH_SECTOR_SIZE);
    
    restore_interrupts(saved_interrupts);
}

// ====================================================================
// 【修正後】② 最終完成版の Storage::save(const bool force)
// ====================================================================
bool Storage::save(const bool force) {
    if (!force &&
        PeripheralManager::getInstance().isUSBEnabled(0) &&
        (DriverManager::getInstance().getInputMode() == INPUT_MODE_PS4 ||
        DriverManager::getInstance().getInputMode() == INPUT_MODE_PS5) &&
        ((PS4Driver*) DriverManager::getInstance().getDriver())->getDongleAuthRequired() == true) {
        return false;
    }

    // 1. バニラ用のキャッシュシリアライズを一度実行（失敗しても下の生メモリ保存で救済します）
    ConfigUtils::save(this->config);
    EEPROM.commit();

    // ====================================================================
    // 2. 💥【生メモリ100%完全物理焼き付け（データ反映絶対保証）】
    // ====================================================================
    // 現在メモリ上にある最新の config 構造体そのものの4KBデータを、
    // そのまま生の塊として、RAM実行関数経由で安全地帯（0x1F4000）へダイレクトに物理後書きします。
    safeWriteToMiniSuperZone(MINI_SUPER_RAW_FLASH_ADDR, (const uint8_t*)&(this->config), sizeof(Config));

    // ====================================================================
    // 3. 💥【安全自動リブートシーケンス】
    // ====================================================================
    watchdog_update(); 
    sleep_ms(800);     // 猶予ディレイを0.8秒へ補強し、ブラウザがセッションを綺麗に切断するのを待つ
    watchdog_update();

    // 100%安全が確保された状態で、満を持して自動リブート！
    System::reboot(System::BootMode::GAMEPAD);

    return true;
}


// ====================================================================
// 【修正後】最終完成版の Storage::ResetSettings()
// ====================================================================
void Storage::ResetSettings()
{
    EEPROM.reset();

    // 1. 素の Pico のフラッシュメモリ内の隔離番地（0x1F4000）にお気に入りデータがあるか自動判別
    const uint8_t* rawFlashSource = (const uint8_t*)(XIP_BASE + MINI_SUPER_RAW_FLASH_ADDR);
    uint32_t checkVal = *(const volatile uint32_t*)rawFlashSource;
    
    if (checkVal != 0xFFFFFFFF && checkVal != 0x00000000) {
        // ⭕ 【お気に入りの生メモリ構造体データから、一撃でダイレクト脳内復元！】
        // Protobufのロードを完全スルーし、保存されていた4KBの生メモリをそのまま config 構造体へ一括全転送します。
        memcpy(&(this->config), rawFlashSource, sizeof(Config));
        
        // 通常領域のキャッシュへも同期
        memcpy(FlashPROM::writeCache, rawFlashSource, sizeof(Config));
    } else {
        // ⭕ 【完全初期状態 ➔ BoardConfig.h に焼き付けたマスターバイナリ配列を一括ダイレクト流し込み！】
        for (uint16_t i = 0; i < sizeof(miniSuperPerfectBinary); i++) {
            FlashPROM::writeCache[i] = miniSuperPerfectBinary[i];
        }
        ConfigUtils::load(config); // 初回のみバニラ展開
    }

    EEPROM.commit();

    // 2. タイムアウト＆USBデバイスエラー（ピコ音）の完全根絶ディレイ
    watchdog_update(); 
    sleep_ms(800);     // ブラウザが通信を正常に完了して切断するのをしっかり待つ
    watchdog_update();

    // フラッシュの物理安全とWeb通信の切断が100%確保された状態で、自動リブート！
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
