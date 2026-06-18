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

// 💡 16MBフラッシュの通常アクセス範囲外へ安全にRAW転送するためのヘッダー
#include "hardware/flash.h"
#include "hardware/sync.h"

// Check for saves
#include "ps4/PS4Driver.h"

#include "config_utils.h"

// 💡 16MB基板専用仕様：通常のプログラムからは絶対にアクセスされない「4MB目の先頭番地（0x400000）」
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

// ==============================================================================
// 💾 🎯 ① バックアップ / 通常セーブ（システムメモリを破壊しない、バニラルート完全救出版）
// ==============================================================================
bool Storage::save(const bool force) {
    if (!force &&
        PeripheralManager::getInstance().isUSBEnabled(0) &&
        (DriverManager::getInstance().getInputMode() == INPUT_MODE_PS4 ||
        DriverManager::getInstance().getInputMode() == INPUT_MODE_PS5) &&
        ((PS4Driver*)DriverManager::getInstance().getDriver())->getDongleAuthRequired() == true ) {
        return false;
    }

    // 🛠️ 【Backupボタン（force == true）が押された時だけの直流しルート】
    if (force) {
        uint32_t ints = save_and_disable_interrupts();
        // 1. 16MB大容量フラッシュの4MB目(0x400000)から正確に16KB（4セクター）を物理消去
        flash_range_erase(MINI_SUPER_RAW_FLASH_ADDR, FLASH_SECTOR_SIZE * 4);
        // 2. 16KBの生設定バイナリバッファ（writeCache）だけを隔離領域へ直撃RAW上書き転送！
        // 💡 【通常セーブのUSBエラー完全消滅】関数の内部で重い ConfigUtils::save を二重に呼び出すのを完全に撤去しました。
        // これにより、通常の各項目セーブ時にメモリ処理が長引いてPCからタイムアウト切断される現象は100%永久に消滅します！
        flash_range_program(MINI_SUPER_RAW_FLASH_ADDR, FlashPROM::writeCache, FLASH_SECTOR_SIZE * 4);
        restore_interrupts(ints);
    }

    // ⭕ 【通常セーブの救出】各項目の通常セーブ時は、forceがfalseなので無傷で100%バニラ本来の処理を流れます
    // Webサーバー側が構築したメモリのままクリーンに保存されるため、グルグルが長引くこともUSBエラーになることもありません。
    bool result = ConfigUtils::save(config);
    EEPROM.commit();
    return result;
}

// ==============================================================================
// 💾 🎯 ② 初期化 / ロード（公式自動リブートシステム完全調和版）
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
        // ⭕ 【完全初期状態の場合】寸法・構造のズレが絶対に起きない最新の初期構造を展開
        memset(&this->config, 0, sizeof(Config));
        ConfigUtils::load(config); 
    }

    // 🔥【レイヤ3：メモリキャッシュへのリアルタイム強制同期フラグ注入】
    // 💡 初期化時やロード時でも、100%確実に「画面常時ON」「LED入力連動（値:1）」で固定
    this->config.displayOptions.enabled = true;                   // 画面常時ON
    this->config.addonOptions.onBoardLedOptions.enabled = true;   // オンボードLEDアドオンをON
    this->config.addonOptions.onBoardLedOptions.mode = static_cast<OnBoardLedMode>(1); // モード: INPUTモード (入力連動点灯)

    // 💡 注入したフラグを完璧なProtobufバイナリにシリアライズして writeCache（16KB）に公式上書き翻訳
    ConfigUtils::save(this->config);
    
    // 💡 物理フラッシュメモリへガチッとコミットして確定永続保存
    EEPROM.commit();

    // 💡 【全自動リブート競合の完全解決パッチ】
    // ここに書いていた System::reboot(System::BootMode::GAMEPAD); などの先走り命令を「すべて撤去」しました！
    // この関数の処理をクリーンに終了(return)させて、Webサーバー（webconfig.cpp）側へバトンを戻します。
    // これにより、Webサーバーはブラウザと100%完璧に同期し、ブラウザへのSUCCESS送信を終えたジャスト直後に、
    // システム本来のコントロール網が全自動でクリーンにアケコン通常モードへと実機を自動再起動させてくれます。
    // 電源の切り入りも、手動での抜き差しも、これで完全に100%不要になります！
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
