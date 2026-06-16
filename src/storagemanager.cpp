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

// 💡 【MINI Super 専用】100%真っ新な「Wii公式完全準拠＆物理ピン初期デフォルト」マスターバイナリ配列
// ユーザー様のご要望通り、「初期化（Reset Settings）」を実行した時はいつでもこの純粋な初期状態へ上書き復元されます。
static const uint8_t miniSuperPerfectBinary[] = {
	0x0A, 0x06, 0x08, 0x00, 0x10, 0x00, 0x18, 0x05, 0x12, 0x3E, 0x08, 0x01, 0x10, 0x02, 0x18, 0x04, 
	0x20, 0x03, 0x28, 0x05, 0x30, 0x06, 0x38, 0x0C, 0x40, 0x01, 0x0B, 0x48, 0x01, 0x07, 0x50, 0x01, 
	0x08, 0x58, 0x01, 0x0A, 0x60, 0x01, 0x09, 0x68, 0x01, 0x20, 0x70, 0x01, 0x0E, 0x78, 0x1E, 0x1A, 
	0x46, 0x08, 0x1B, 0x10, 0x00, 0x18, 0x00, 0x20, 0x01, 0x28, 0x50, 0x30, 0x0A, 0x38, 0x01, 0x40, 
	0x0E, 0x48, 0x22, 0x50, 0x00, 0x58, 0x01, 0x60, 0x02, 0x68, 0x03, 0x70, 0x04, 0x78, 0x05, 0x80, 
	0x01, 0x06, 0x88, 0x01, 0x0C, 0x90, 0x01, 0x0B, 0x98, 0x01, 0x07, 0xA0, 0x01, 0x08, 0xA8, 0x01, 
	0x0A, 0xB0, 0x01, 0x09, 0x22, 0x03, 0x08, 0x01, 0x10, 0x01, 0x2A, 0x04, 0x08, 0x01, 0x10, 0x01, 
	0x3A, 0x24, 0x08, 0x01, 0x10, 0x01, 0x12, 0x1C, 0x08, 0x0F, 0x08, 0x04, 0x08, 0x15, 0x08, 0x16, 
	0x08, 0x17, 0x08, 0x18, 0x08, 0x19, 0x08, 0x1A, 0x08, 0x10, 0x08, 0x0B, 0x08, 0x0C, 0x08, 0x09, 0x08, 
	0x0D, 0x08, 0x00, 0x08, 0x1B, 0x08, 0x1C, 0x18, 0x10, 0x4A, 0x06, 0x08, 0x01, 0x10, 0x02, 0x18, 0x01, 
	0x52, 0x02, 0x08, 0x01
};

// 💡 ユーザー様のお気に入り固定化データを記録しておく、物理フラッシュメモリ上の「秘密のバックアップ領域」
#define MASTER_BACKUP_OFFSET 0x4000

void Storage::init() {
	systemFlashSize = System::getPhysicalFlash();
	EEPROM.start();
	ConfigUtils::load(config);

	// 【初回起動救済】工場出荷状態で真っ新な場合は、100%真っ新なMINI Super初期マスターデータを流し込んで起動します
	if (!config.ledOptions.has_dataPin || config.ledOptions.dataPin == -1) {
		EEPROM.reset();
		for (uint16_t i = 0; i < sizeof(miniSuperPerfectBinary); i++) {
			FlashPROM::writeCache[i] = miniSuperPerfectBinary[i];
		}
		// 後半の「お気に入り固定化エリア」にも初期マスターとして同時に焼き付けます
		for (uint16_t i = 0; i < sizeof(miniSuperPerfectBinary); i++) {
			FlashPROM::writeCache[MASTER_BACKUP_OFFSET + i] = miniSuperPerfectBinary[i];
		}
		EEPROM.commit();
		ConfigUtils::load(config);
	}
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

	// 🛠️ 【ハックA：Backup To File / お気に入り全設定の固定化上書き】
	// ブラウザのバグ通信データを完全無視し、現在の全設定（this->config）を直接シリアライズしてバッファへ直流し込み
	ConfigUtils::save(this->config);

	// シリアライズされてwriteCacheの先頭（0番地以降）に書き込まれたばかりの「最新の全設定バイナリ」を、
	// そのまま同じフラッシュ内の『お気に入りマスター領域（MASTER_BACKUP_OFFSET）』へ丸ごと直接上書き（永久保存）します。
	for (uint16_t i = 0; i < MASTER_BACKUP_OFFSET; i++) {
		FlashPROM::writeCache[MASTER_BACKUP_OFFSET + i] = FlashPROM::writeCache[i];
	}

	return ConfigUtils::save(config), EEPROM.commit(), true;
}

void Storage::ResetSettings()
{
	// 🛠️ 【ハックB：Reset Settings（初期化）専用トリガー】
	// お気に入りデータではなく、ユーザー様のご指示通り、ソースコードに埋め込まれている
	// 「100%真っ新なWii公式完全準拠マスターバイナリ（miniSuperPerfectBinary）」を、
	// 現在のロード領域（0番地以降）に対して直接丸ごと「上書き直流し込み」を実行します！
	EEPROM.reset();
	for (uint16_t i = 0; i < sizeof(miniSuperPerfectBinary); i++) {
		FlashPROM::writeCache[i] = miniSuperPerfectBinary[i];
	}
	EEPROM.commit();

	// 完全な初期デフォルト状態で安全に自動再起動
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
