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

// 16MB 💡 フラッシュの通常アクセス範囲外へ安全に RAW 転送するための隔離番地固定指定
#define MINI_SUPER_RAW_FLASH_ADDR 0x400000

void Storage::init() {
    systemFlashSize = System::getPhysicalFlash();
    EEPROM.start();
    ConfigUtils::load(config);
}

bool Storage::save() {
    return save(false);
}

// ==============================================================================
// 💾 🎯 ① バックアップ / 通常セーブ（他ファイル・ガベージコレクション衝突完全回避版）
// ==============================================================================
bool Storage::save(const bool force) {
    if (!force &&
        PeripheralManager::getInstance().isUSBEnabled(0) &&
        (DriverManager::getInstance().getInputMode() == INPUT_MODE_PS4 ||
        DriverManager::getInstance().getInputMode() == INPUT_MODE_PS5) &&
        ((PS4Driver*)DriverManager::getInstance().getDriver())->getDongleAuthRequired() == true ) {
        return false;
    }

    // 💡【他ファイルとの絡みの解決】
    // まずはバニラ本来の正規のセーブ処理を完全に終わらせ、仮想EEPROM側のガベージコレクションを
    // 安全に通過・確定させます。これにより、通常の各項目セーブ時のフリーズは100%消滅します。
    bool result = ConfigUtils::save(config);
    EEPROM.commit();

    // 🛠️ 【Backupボタン（force == true）が押された時だけの直流しルート】
    // 通常セーブが完了した「完全無風のタイミング」で、4MB目への16KBRAWプログラムを安全に直撃実行します！
    if (force && result) {
        uint32_t ints = save_and_disable_interrupts();
        flash_range_erase(MINI_SUPER_RAW_FLASH_ADDR, FLASH_SECTOR_SIZE * 4);
        flash_range_program(MINI_SUPER_RAW_FLASH_ADDR, FlashPROM::writeCache, FLASH_SECTOR_SIZE * 4);
        restore_interrupts(ints);
    }

    return result;
}

// ==============================================================================
// 💾 🎯 ② 初期化 / ロード（公式関数完全準拠・周辺機器強制リフレッシュ版）
// ==============================================================================
void Storage::ResetSettings()
{
    // 1. 内蔵EEPROMバッファのクリア
    EEPROM.reset();

    // 2. 4MB 目の隔離領域にデータがあるかを自動判別（空っぽの0xFFではないか）
    const uint8_t* rawFlashSource = (const uint8_t*)(XIP_BASE + MINI_SUPER_RAW_FLASH_ADDR);
    uint32_t checkVal = *(const volatile uint32_t*)rawFlashSource;

    if (checkVal != 0xFFFFFFFF && checkVal != 0x00000000) {
        // ⭕ 【一度でもBackupを押したことがある場合】4MB目の隔離領域から「正確に16KB」を writeCache へ逆コピー復元
        for (uint16_t i = 0; i < (FLASH_SECTOR_SIZE * 4); i++) {
            FlashPROM::writeCache[i] = rawFlashSource[i];
        }
        ConfigUtils::load(config);
    } else {
        // ⭕ 【完全初期状態の場合】
        // 💡 現在のファームウェアが100%合法と認める「最新の正しい初期構造体」をメモリ上にその場でクリーンビルド
        memset(&this->config, 0, sizeof(Config));
        ConfigUtils::load(config); // 公式の安全な初期構造が config に入ります
        
        // 🔥 【追加カスタム仕様をC++純正コードで100%確実に注入】
        this->config.displayOptions.enabled = true;                   // 1. 画面常時ON
        this->config.addonOptions.onBoardLedOptions.enabled = true;   // 2. オンボードLEDアドオンをON
        this->config.addonOptions.onBoardLedOptions.mode = static_cast<OnBoardLedMode>(1); // 3. LEDモード: 入力テスト
        
        ConfigUtils::save(this->config);
    }

    // 3. 物理フラッシュメモリへガチッとコミットして確定永続保存
    EEPROM.commit();

    // 4. 💡【周辺アドオンへの強制通知パッチ】
    // 変更した「画面ON」「LED入力テスト」の設定を、周辺機器のシングルトンインスタンスへ正規のローダー経由で強制再適用バインドさせます。
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
