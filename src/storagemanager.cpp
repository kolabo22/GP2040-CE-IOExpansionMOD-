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
#include "tusb.h"
#include "pico/time.h"

// 16MB 💡 【真の聖域】HTMLデータやシステム領域から完全に隔離された「15MB目のジャスト先頭番地」
#define MINI_SUPER_RAW_FLASH_ADDR 0xF00000

void Storage::init() {
    systemFlashSize = System::getPhysicalFlash();
    EEPROM.start();
    
    // 💡 起動時に15MB目の隔離領域に有効な設定データが入っていれば最優先展開
    const uint8_t* rawFlashSource = (const uint8_t*)(XIP_BASE + MINI_SUPER_RAW_FLASH_ADDR);
    uint32_t checkVal = *(const volatile uint32_t*)rawFlashSource;
    if (checkVal != 0xFFFFFFFF && checkVal != 0x00000000) {
        for (uint16_t i = 0; i < (FLASH_SECTOR_SIZE * 4); i++) {
            FlashPROM::writeCache[i] = rawFlashSource[i];
        }
    }
    ConfigUtils::load(config);
}

bool Storage::save() {
    return save(false);
}

// ==============================================================================
// 💾 🎯 ① バックアップ / 通常セーブ（HTMLデータ衝突を完全隔離・16KB安全直流し版）
// ==============================================================================
bool Storage::save(const bool force) {
    if (!force &&
        PeripheralManager::getInstance().isUSBEnabled(0) &&
        (DriverManager::getInstance().getInputMode() == INPUT_MODE_PS4 ||
        DriverManager::getInstance().getInputMode() == INPUT_MODE_PS5) &&
        ((PS4Driver*)DriverManager::getInstance().getDriver())->getDongleAuthRequired() == true ) {
        return false;
    }

    bool result = ConfigUtils::save(config);

    // 🛠️ 【Backupボタン（force == true）が押された時だけの直流しルート】
    // 💡 4MB目のHTML領域との衝突を完全に脱出し、15MB目の真の安全空き地へ16KBを直撃RAW転送！
    if (force && result) {
        uint32_t ints = save_and_disable_interrupts();
        flash_range_erase(MINI_SUPER_RAW_FLASH_ADDR, FLASH_SECTOR_SIZE * 4);
        flash_range_program(MINI_SUPER_RAW_FLASH_ADDR, FlashPROM::writeCache, FLASH_SECTOR_SIZE * 4);
        restore_interrupts(ints);
    }

    return result;
}

// ==============================================================================
// 💾 🎯 ② 初期化 / ロード（マイグレーション暴走・HTMLクラッシュ完全封殺版）
// ==============================================================================
void Storage::ResetSettings()
{
    // 15MB 目の隔離領域にデータがあるかを自動判別
    const uint8_t* rawFlashSource = (const uint8_t*)(XIP_BASE + MINI_SUPER_RAW_FLASH_ADDR);
    uint32_t checkVal = *(const volatile uint32_t*)rawFlashSource;

    if (checkVal != 0xFFFFFFFF && checkVal != 0x00000000) {
        // ⭕ 【一度でもBackupを押したことがある場合】15MB目の隔離領域から「正確に16KB」を再展開
        for (uint16_t i = 0; i < (FLASH_SECTOR_SIZE * 4); i++) {
            FlashPROM::writeCache[i] = rawFlashSource[i];
        }
    } else {
        // ⭕ 【完全初期状態の場合】
        // 💡 config_utils.cpp の無限ループを完璧に回避する、合法的な最新デフォルト構造を展開！
        memset(&this->config, 0, sizeof(Config));
        ConfigUtils::load(config); // 公式の安全な初期構造が config に入ります
        
        // 🔥 【追加カスタム仕様をC++純正コードで100%確実に注入】
        this->config.displayOptions.enabled = true;                   // 1. 画面常時ON
        this->config.addonOptions.onBoardLedOptions.enabled = true;   // 2. オンボードLEDアドオンをON
        this->config.addonOptions.onBoardLedOptions.mode = static_cast<OnBoardLedMode>(1); // 3. LEDモード: 入力テスト
        
        ConfigUtils::save(this->config);
    }

    // 💡 15MB目の真の安全番地へのRAW直流しのみで設定を確定永続化させます。
    uint32_t ints = save_and_disable_interrupts();
    flash_range_erase(MINI_SUPER_RAW_FLASH_ADDR, FLASH_SECTOR_SIZE * 4);
    flash_range_program(MINI_SUPER_RAW_FLASH_ADDR, FlashPROM::writeCache, FLASH_SECTOR_SIZE * 4);
    restore_interrupts(ints);

    // 最新の設定を周辺ハードウェアクラスへ完全同期（一発点灯パッチ）
    ConfigUtils::load(config);
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
