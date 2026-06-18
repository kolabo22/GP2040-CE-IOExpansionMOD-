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

// 💡 webconfig.cpp 側で現在処理中の通信宛先（URL文字列）を直接外部参照
extern std::string http_post_uri;

// 16MB 💡 日常の通常セーブ（2MB）から完全に隔離された「お気に入りマスター設定専用」の永久隔離聖域
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
// 💾 🎯 ① 通常セーブとお気に入り隔離セーブの「データ中身の完全分離」
// ==============================================================================
bool Storage::save(const bool force) {
    if (!force &&
        PeripheralManager::getInstance().isUSBEnabled(0) &&
        (DriverManager::getInstance().getInputMode() == INPUT_MODE_PS4 ||
        DriverManager::getInstance().getInputMode() == INPUT_MODE_PS5) &&
        ((PS4Driver*)DriverManager::getInstance().getDriver())->getDongleAuthRequired() == true ) {
        return false;
    }

    // 💡 【最重要：日常セーブとお気に入り保存のデータ完全分離ロジック】
    if (http_post_uri == "/api/backup") {
        // ⭕ 【お気に入り隔離セーブ（Backupボタン）が押された瞬間の挙動】
        // 日常の通常セーブによってバッファが上書きされる「一歩手前」で、
        // 「その時点でのすべての設定項目」を綺麗なProtobuf形式バイナリにシリアライズして writeCache に格納します。
        ConfigUtils::save(this->config);

        // その「その時点の全設定データ」を、4MB目の特設隔離聖域へRAW直撃転送して永久ロック保存します！
        uint32_t ints = save_and_disable_interrupts();
        flash_range_erase(MINI_SUPER_RAW_FLASH_ADDR, FLASH_SECTOR_SIZE * 4);
        flash_range_program(MINI_SUPER_RAW_FLASH_ADDR, FlashPROM::writeCache, FLASH_SECTOR_SIZE * 4);
        restore_interrupts(ints);
        
        // 💡 4MB目の隔離部屋への保存が終わったら、日常の2MB通常セーブ（EEPROM.commit）は【一切呼び出さずにreturn】します。
        // これにより、通常の部屋は一切汚さず、バックアップボタンを押した時点の全設定のみが聖域に100%独立してキープされます！
        return true;
    }

    // ⭕ 【通常各項目の保存（Saveボタン）の挙動】
    // 普段、色々と設定をガチャガチャ変更して試す日常の変更セーブ（forceがfalse、かつ通常URL時）です。
    // バックアップ（Backup）ボタンが押されていない時は、上記の4MB目処理を完全にスルーし、
    // 従来通り通常領域（2MB内）の仮想EEPROM（FlashPROM::writeCache）への保存予約を行います。
    // これにより、普段のSaveによって4MB目の隔離聖域データが都度変化して汚れることは100%絶対にありません！
    bool result = ConfigUtils::save(config);
    EEPROM.commit();
    return result;
}

// ==============================================================================
// 💾 🎯 ② 初期化 / ロード（隔離聖域からのコピペ復元 ＆ バニラ仕分け自動リブート版）
// ==============================================================================
void Storage::ResetSettings()
{
    // 4MB 目の隔離聖域にデータがあるかを自動判別
    const uint8_t* rawFlashSource = (const uint8_t*)(XIP_BASE + MINI_SUPER_RAW_FLASH_ADDR);
    uint32_t checkVal = *(const volatile uint32_t*)rawFlashSource;

    // 💡 【ファイルから復元（Restoreボタン）が押された瞬間の挙動】
    // 日常の通常セーブ（最終状態）は完全に無視し、4MB目の隔離聖域に厳重保管されている
    // 「バックアップした時点のすべての設定項目」の16KBを writeCache へ一撃で「逆コピペ」して通常部屋へ復元します！
    if (checkVal != 0xFFFFFFFF && checkVal != 0x00000000) {
        for (uint16_t i = 0; i < (FLASH_SECTOR_SIZE * 4); i++) {
            FlashPROM::writeCache[i] = rawFlashSource[i];
        }
    } else {
        // 【完全初期状態の場合（工場出荷時）】公式安全デフォルトを展開
        memset(&this->config, 0, sizeof(Config));
        this->config.displayOptions.enabled = true;
        this->config.addonOptions.onBoardLedOptions.enabled = true;
        this->config.addonOptions.onBoardLedOptions.mode = static_cast<OnBoardLedMode>(1);
    }

    ConfigUtils::save(this->config);
    EEPROM.commit();
    ConfigUtils::load(config);

    // 💡 【初期化(Reset)はバニラ通り自動リブート ➔ ロード(Restore)は画面をキープして保持】
    if (http_post_uri == "/api/resetSettings") {
        System::reboot(System::BootMode::GAMEPAD);
    }
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
