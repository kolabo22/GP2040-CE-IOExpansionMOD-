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

// 💡 16MBフラッシュのアクセス範囲外（RAW領域）へ直接上書き転送するためのPico SDKヘッダー
#include "hardware/flash.h"
#include "hardware/sync.h"

// Check for saves
#include "ps4/PS4Driver.h"

#include "config_utils.h"

// 💡 【MINI Super 専用】100%真っ新な「Wii公式完全準拠＆物理ピン初期デフォルト」マスターバイナリ配列
// 「Reset Settings（初期化）」を実行した時は、いつでもこの純粋な初期状態へ直撃上書き復元されます。
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
	// 「Backup To File」ボタンを押した瞬間にのみ（force = true）、このハックが発動します。
	if (force) {
		// writeCache に書き出された全設定バイナリデータを、
		// 通常プログラムのアクセス範囲外である16MB高位RAW領域「0x200000」番地へ、直接上書き転送して固定化します。
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
	// 🛠️ 【② Restore From File および Reset Settings（初期化）の完全自動判定上書き】
	// メモリ上の「config.ledOptions.dataPin」が現在 27（本物のダミーJSONまたは実稼働中のデータ）になっているか、
	// それとも完全初期化コマンドによって 0 または空っぽにクリアされているかで、ロード領域への直流し込みソースを自動判別します。
	
	EEPROM.reset();

	if (config.ledOptions.dataPin == 27 || config.ledOptions.has_dataPin) {
		// ⭕ 【Restore From File（ダミーロード復元）が実行された時】
		// 物理アドレス 0x200000 番地に保存されているお気に入りバイナリデータはそのまま完全に保持したまま、
		// 現在のロード先メモリバッファ（writeCache）に対して直接丸ごと「上書き直流し込み」を実行します！
		const uint8_t* rawFlashSource = (const uint8_t*)(XIP_BASE + MINI_SUPER_RAW_FLASH_ADDR);
		for (uint16_t i = 0; i < (FLASH_SECTOR_SIZE * 4); i++) {
			FlashPROM::writeCache[i] = rawFlashSource[i];
		}
	} else {
		// ⭕ 【Reset Settings（初期化）が実行された時】
		// ソースコード内に焼き付けられている「100%真っ新なWii公式完全準拠マスターバイナリ（miniSuperPerfectBinary）」を
		// ロード先メモリバッファ（writeCache）に対して直接丸ごと「上書き直流し込み」を実行します！
		for (uint16_t i = 0; i < sizeof(miniSuperPerfectBinary); i++) {
			FlashPROM::writeCache[i] = miniSuperPerfectBinary[i];
		}
	}

	// 2. 直流し込みしたデータを一度ロードしてConfig構造体へ展開
	ConfigUtils::load(config);

	// 3. ✨【確実な追加仕様パッチ】
	// ロードした全設定に対して、「画面常時ON（画像表示）」と「オンボードLEDの入力連動（値:1）」のフラグを確実に上書き固定します。
	config.displayOptions.enabled = true;
	config.displayOptions.splashMode = static_cast<SplashMode>(1);        // 1 = STATIC (画像表示)
	config.displayOptions.has_enabled = true;
	config.displayOptions.has_splashMode = true;

	config.addonOptions.onBoardLedOptions.enabled = true;
	config.addonOptions.onBoardLedOptions.mode = static_cast<OnBoardLedMode>(1); // 1 = INPUT (入力連動)
	config.addonOptions.onBoardLedOptions.has_enabled = true;
	config.addonOptions.onBoardLedOptions.has_mode = true;

	// 4. 全設定データをフラッシュメモリへ確定コミット
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
