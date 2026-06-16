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

// Check for saves
#include "ps4/PS4Driver.h"

#include "config_utils.h"

// 💡 16MBフラッシュ対応：32KBのエミュレート領域バッファ（writeCache）の完全なる大末尾
#define MASTER_BACKUP_OFFSET 0x7F00

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

	if (force) {
		for (uint16_t i = 0; i < (EEPROM_SIZE_BYTES - MASTER_BACKUP_OFFSET); i++) {
			FlashPROM::writeCache[MASTER_BACKUP_OFFSET + i] = FlashPROM::writeCache[i];
		}
	}

	return ConfigUtils::save(config), EEPROM.commit(), true;
}

void Storage::ResetSettings()
{
	// 🛠️ 【ハックB：Reset Settings（初期化）専用・全設定完全直流し込みトリガー】
	// 1. ベースとなるWii・GPIOピンの確定バイナリ配列をバッファへ直流し込み上書き
	EEPROM.reset();
	
	// ユーザー様からいただいた本物の確定バイナリを初期配置します
	static const uint8_t miniSuperPerfectBinary[] = {
		0x0A, 0x0C, 0x08, 0x01, 0x10, 0x00, 0x18, 0x01, 0x20, 0x00, 0x5A, 0x00, 0x12, 0x22, 0x08, 0x1B, 
		0x10, 0x00, 0x18, 0x00, 0x20, 0x01, 0x28, 0x50, 0x30, 0x0A, 0x38, 0x01, 0x40, 0x0E, 0x48, 0x22, 
		0x50, 0x00, 0x58, 0x01, 0x60, 0x02, 0x68, 0x03, 0x70, 0x04, 0x78, 0x05, 0x80, 0x01, 0x06, 0x88, 
		0x01, 0x0C, 0x90, 0x01, 0x0B, 0x98, 0x01, 0x07, 0xA0, 0x01, 0x08, 0xA8, 0x01, 0x0A, 0xB0, 0x01, 
		0x09, 0x1A, 0x44, 0x08, 0x01, 0x10, 0x02, 0x18, 0x04, 0x20, 0x03, 0x28, 0x05, 0x30, 0x06, 0x38, 
		0x0C, 0x40, 0x01, 0x0B, 0x48, 0x01, 0x07, 0x50, 0x01, 0x08, 0x58, 0x01, 0x0A, 0x60, 0x01, 0x09, 
		0x68, 0x01, 0x20, 0x70, 0x01, 0x0E, 0x78, 0x1E, 0x80, 0x01, 0x01, 0x22, 0x16, 0x0A, 0x03, 0x08, 
		0x00, 0x10, 0x01, 0x12, 0x0F, 0x08, 0x12, 0x10, 0x13, 0x18, 0x80, 0x96, 0x18, 0x22, 0x03, 0x08, 
		0x01, 0x10, 0x01, 0x2A, 0x04, 0x08, 0x01, 0x10, 0x01, 0x3A, 0x24, 0x08, 0x01, 0x10, 0x01, 0x12, 
		0x1C, 0x08, 0x0F, 0x08, 0x04, 0x08, 0x15, 0x08, 0x16, 0x08, 0x17, 0x08, 0x18, 0x08, 0x19, 0x08, 
		0x1A, 0x08, 0x10, 0x08, 0x0B, 0x08, 0x0C, 0x08, 0x09, 0x08, 0x0D, 0x08, 0x00, 0x08, 0x1B, 0x08, 
		0x1C, 0x18, 0x10, 0x4A, 0x06, 0x08, 0x01, 0x10, 0x02, 0x18, 0x01, 0x52, 0x02, 0x08, 0x01
	};
	for (uint16_t i = 0; i < sizeof(miniSuperPerfectBinary); i++) {
		FlashPROM::writeCache[i] = miniSuperPerfectBinary[i];
	}

	// 2. 直流し込みしたデータを一度ロードしてConfig構造体へ展開
	ConfigUtils::load(config);

	// 3. 🛠️【システムチェック強制パス仕様】
	// 列挙型の型違いエラーを完全に防ぐため、内部の「生の数字（enum値）」でダイレクトに上書きロックします
	config.displayOptions.enabled = true;
	config.displayOptions.splashMode = static_cast<SplashMode>(1);        // 1 = STATIC (画像表示)
	config.displayOptions.has_enabled = true;
	config.displayOptions.has_splashMode = true;

	config.addonOptions.onBoardLedOptions.enabled = true;
	config.addonOptions.onBoardLedOptions.mode = static_cast<OnBoardLedMode>(1); // 1 = INPUT (入力連動)
	config.addonOptions.onBoardLedOptions.has_enabled = true;
	config.addonOptions.onBoardLedOptions.has_mode = true;

	// 4. システム標準のシリアライザを通じて、完璧になった全設定データをフラッシュメモリへ確定コミット
	ConfigUtils::save(config);
	EEPROM.commit();

	// 2000ms（2秒）の間、WebUIに綺麗な再起動画面を表示させてから安全にコールドリセット
	watchdog_reboot(0, SRAM_END, 2000);
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

	if (config.gamepadOptions.profileNumber >= 2 &&
			config.gamepadOptions.profileNumber <= profileCeiling) {
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

