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
// 16MB 💡 フラッシュの通常アクセス範囲外へ安全に RAW 転送するためのヘッダー
#include "hardware/flash.h"
#include "hardware/sync.h"
// Check for saves
#include "ps4/PS4Driver.h"
#include "config_utils.h"

// 💡 【MINI Super 専用】画面 ON＆オンボード LED 入力連動（値:1）が組み込まれた真のマスターバイナリ配列
static const uint8_t miniSuperPerfectBinary[] = {
0x0A, 0x06, 0x08, 0x00, 0x10, 0x00, 0x18, 0x05, 0x12, 0x3E, 0x08, 0x01, 
0x10, 0x02, 0x18, 0x04, 
0x20, 0x03, 0x28, 0x05, 0x30, 0x06, 0x38, 0x0C, 0x40, 0x01, 0x0B, 0x48, 
0x01, 0x07, 0x50, 0x01, 
0x08, 0x58, 0x01, 0x0A, 0x60, 0x01, 0x09, 0x68, 0x01, 0x20, 0x70, 0x01, 
0x0E, 0x78, 0x1E, 0x1A, 
0x46, 0x08, 0x1B, 0x10, 0x00, 0x18, 0x00, 0x20, 0x01, 0x28, 0x50, 0x30, 
0x0A, 0x38, 0x01, 0x40, 
0x0E, 0x48, 0x22, 0x50, 0x00, 0x58, 0x01, 0x60, 0x02, 0x68, 0x03, 0x70, 
0x04, 0x78, 0x05, 0x80, 
0x01, 0x06, 0x88, 0x01, 0x0C, 0x90, 0x01, 0x0B, 0x98, 0x01, 0x07, 0xA0, 
0x01, 0x08, 0xA8, 0x01, 
0x0A, 0xB0, 0x01, 0x09, 0x22, 0x03, 0x08, 0x01, 0x10, 0x01, 0x2A, 0x04, 
0x08, 0x01, 0x10, 0x01, 0x3A, 
0x24, 0x08, 0x01, 0x10, 0x01, 0x12, 0x1C, 0x08, 0x0F, 0x08, 0x04, 0x08, 
0x15, 0x08, 0x16, 0x08, 0x17, 
0x08, 0x18, 0x08, 0x19, 0x08, 0x1A, 0x08, 0x10, 0x08, 0x0B, 0x08, 0x0C, 
0x08, 0x09, 0x08, 0x0D, 0x08, 
0x00, 0x08, 0x1B, 0x08, 0x1C, 0x18, 0x10, 0x4A, 0x06, 0x08, 0x01, 0x10, 
0x02, 0x18, 0x01, 0x52, 0x02, 
0x08, 0x01, 0x32, 0x06, 0x08, 0x01, 0x10, 0x01, 0x42, 0x06, 0x08, 0x01, 
0x10, 0x01
};

// 16MB 💡 基板専用仕様：通常のプログラムからは絶対にアクセスされない「4MB 目の先頭番地（0x400000）」
#define MINI_SUPER_RAW_FLASH_ADDR 0x400000

void Storage::init() {
    systemFlashSize = System::getPhysicalFlash();
    EEPROM.start();
    ConfigUtils::load(config);
}

bool Storage::save()
{
    return save(false);
}

bool Storage::save(const bool force) {
    if (!force &&
        PeripheralManager::getInstance().isUSBEnabled(0) &&
        (DriverManager::getInstance().getInputMode() == INPUT_MODE_PS4 ||
        DriverManager::getInstance().getInputMode() == INPUT_MODE_PS5) &&
        ((PS4Driver*)DriverManager::getInstance().getDriver())->getDongleAuthRequired() == true ) {
        return false;
    }

    ConfigUtils::save(this->config);

    // 🛠️ 【① NO ファイル仕様：ファイル保存を完全に無くした実機内完結セーブ】
    if (force) {
        uint32_t ints = save_and_disable_interrupts();
        flash_range_erase(MINI_SUPER_RAW_FLASH_ADDR, FLASH_SECTOR_SIZE * 4); // 4MB 目の空き地を物理消去
        flash_range_program(MINI_SUPER_RAW_FLASH_ADDR, FlashPROM::writeCache, FLASH_SECTOR_SIZE * 4); // メモリ設定を直撃 RAW 上書き転送
        restore_interrupts(ints);
    }

    return ConfigUtils::save(config), EEPROM.commit(), true;
}

void Storage::ResetSettings()
{
    // 🛠️ 【② NO ファイル仕様：ダイアログを完全撤廃したワンタップ一撃復元】
    EEPROM.reset();

    // 4MB 目の RAW 領域にデータが保存されているか（空っぽの 0xFF ではないか）を自動判別
    const uint8_t* rawFlashSource = (const uint8_t*)(XIP_BASE + MINI_SUPER_RAW_FLASH_ADDR);
    uint32_t checkVal = *(const volatile uint32_t*)rawFlashSource;

    // 💡【検閲の完全バイパス】ダミーファイルの送信要求を完全に無視し、実機内完結で読み込みます
    if (checkVal != 0xFFFFFFFF && checkVal != 0x00000000) {
        // ⭕ 【一度でも Backup を押したことがある場合 ➔ ファイルを一切開かずにお気に入りから一撃復元】
        for (uint16_t i = 0; i < (FLASH_SECTOR_SIZE * 4); i++) {
            FlashPROM::writeCache[i] = rawFlashSource[i];
        }
    } else {
        // ⭕ 【完全初期状態の場合 ➔ 真っ新デフォルト復元】
        for (uint16_t i = 0; i < sizeof(miniSuperPerfectBinary); i++) {
            FlashPROM::writeCache[i] = miniSuperPerfectBinary[i];
        }
    }

    // 物理フラッシュメモリへ確定コミット
    EEPROM.commit();
    ConfigUtils::load(config);

    // 💡【USB 未認識エラー＆画面フリーズの完全解決パッチ】
    // マイコン全体をフリーズさせて通信を切断させていた危険な watchdog_reboot を完全撤去。
    // WebサーバーのTCPセッションを安全にクローズしてブラウザへ100%正常に応答を返しきってから、
    // 自動的にリブート（ピホォ音）を鳴らして通常ゲームパッドとして1発起動させます！
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
