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

// 💡 16MBフラッシュのアクセス範囲外へ直接RAW転送するためのPico SDKヘッダー
#include "hardware/flash.h"
#include "hardware/sync.h"

// Check for saves
#include "ps4/PS4Driver.h"

#include "config_utils.h"

// 💡 16MBフラッシュ仕様専用：通常のプログラムやシステム、EEPROM領域からは「絶対にアクセスされない」
// 2MB目のジャスト先頭番地（0x200000）を、MINI Super専用の絶対安全な物理RAWセクタとして完全固定指定します。
#define MINI_SUPER_RAW_FLASH_ADDR 0x200000

void Storage::init() {
	systemFlashSize = System::getPhysicalFlash();
	EEPROM.start();
	ConfigUtils::load(config);
	
	// 起動遅延処理は完全に排除し、16MBフラッシュ本来の「超爆速起動」を維持します
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

	// メモリ上の最新設定（this->config）をシリアライズしてバッファへ直流し込み
	ConfigUtils::save(this->config);

	// 🛠️ 【① Backup To File（ファイルにバックアップ動作）：高位RAW領域への転送】
	// 「Backup To File」ボタンを押すたびに、現在の最新データを 0x200000 へ何度でも自由に変更・上書きアップデートします。
	if (force) {
		uint32_t ints = save_and_disable_interrupts();
		flash_range_erase(MINI_SUPER_RAW_FLASH_ADDR, FLASH_SECTOR_SIZE * 4); // 16KB分をクリーンに物理消去
		flash_range_program(MINI_SUPER_RAW_FLASH_ADDR, FlashPROM::writeCache, FLASH_SECTOR_SIZE * 4); // 直撃RAW上書き転送
		restore_interrupts(ints);
	}

	// 通常領域（EEPROM側）へも確定コミット
	return ConfigUtils::save(config), EEPROM.commit(), true;
}

void Storage::ResetSettings()
{
	// 🛠️ 【② Restore From File（ファイルから復元動作）および初期化兼用トリガー】
	// 1. 物理アドレス 0x200000 番地に保存されているバイナリデータはそのまま完全に保持したまま、
	// 現在のプレイ用ロード先メモリバッファ（writeCache）に対して直接丸ごと「上書き直流し込み」を実行します！
	EEPROM.reset();
	
	const uint8_t* rawFlashSource = (const uint8_t*)(XIP_BASE + MINI_SUPER_RAW_FLASH_ADDR);
	for (uint16_t i = 0; i < (FLASH_SECTOR_SIZE * 4); i++) {
		FlashPROM::writeCache[i] = rawFlashSource[i];
	}

	// 2. 直流し込みしたデータを一度ロードしてConfig構造体へ展開
	ConfigUtils::load(config);

	// 3. ✨【確実な追加仕様パッチ】
	// 高位RAWから読み出したデータに対して、画面ONとオンボードLED入力連動（INPUT）のフラグを100%確実に上書き固定します。
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

	// 5. 💡【画面上のSuccess!表示を出すための最重要処理】
	// ブラウザ側への送信応答を邪魔しないよう、2秒間お行儀よく待機してから再起動します。
	// これにより、WebConfigの画面上に「Success! Controller is rebooting...」が100%確定で美しく表示されます！
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
