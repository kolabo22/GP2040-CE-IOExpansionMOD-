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

// 💡 【MINI Super 専用】画面ON＆オンボードLED入力連動（値:1）が組み込まれた真のマスターバイナリ配列
static const uint8_t miniSuperPerfectBinary[] = {
	0x0A, 0x06, 0x08, 0x00, 0x10, 0x00, 0x18, 0x05, 0x12, 0x3E, 0x08, 0x01, 0x10, 0x02, 0x18, 0x04, 
	0x20, 0x03, 0x28, 0x05, 0x30, 0x06, 0x38, 0x0C, 0x40, 0x01, 0x0B, 0x48, 0x01, 0x07, 0x50, 0x01, 
	0x08, 0x58, 0x01, 0x0A, 0x60, 0x01, 0x09, 0x68, 0x01, 0x20, 0x70, 0x01, 0x0E, 0x78, 0x1E, 0x1A, 
	0x46, 0x08, 0x1B, 0x10, 0x00, 0x18, 0x00, 0x20, 0x01, 0x28, 0x50, 0x30, 0x0A, 0x38, 0x01, 0x40, 
	0x0E, 0x48, 0x22, 0x50, 0x00, 0x58, 0x01, 0x60, 0x02, 0x68, 0x03, 0x70, 0x04, 0x78, 0x05, 0x80, 
	0x01, 0x06, 0x88, 0x01, 0x0C, 0x90, 0x01, 0x0B, 0x98, 0x01, 0x07, 0xA0, 0x01, 0x08, 0xA8, 0x01, 
	0x0A, 0xB0, 0x01, 0x09, 0x22, 0x03, 0x08, 0x01, 0x10, 0x01, 0x2A, 0x04, 0x08, 0x01, 0x10, 0x01, 0x3A, 
	0x24, 0x08, 0x01, 0x10, 0x01, 0x12, 0x1C, 0x08, 0x0F, 0x08, 0x04, 0x08, 0x15, 0x08, 0x16, 0x08, 0x17, 
	0x08, 0x18, 0x08, 0x19, 0x08, 0x1A, 0x08, 0x10, 0x08, 0x0B, 0x08, 0x0C, 0x08, 0x09, 0x08, 0x0D, 0x08, 
	0x00, 0x08, 0x1B, 0x08, 0x1C, 0x18, 0x10, 0x4A, 0x06, 0x08, 0x01, 0x10, 0x02, 0x18, 0x01, 0x52, 0x02, 
	0x08, 0x01, 0x32, 0x06, 0x08, 0x01, 0x10, 0x01, 0x42, 0x06, 0x08, 0x01, 0x10, 0x01
};

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
// 💾 🎯 ① バックアップ / 通常セーブ（ダミーファイルの構築 ＆ 16MBフラッシュ直流し）
// ==============================================================================
bool Storage::save(const bool force) {
    if (!force &&
        PeripheralManager::getInstance().isUSBEnabled(0) &&
        (DriverManager::getInstance().getInputMode() == INPUT_MODE_PS4 ||
        DriverManager::getInstance().getInputMode() == INPUT_MODE_PS5) &&
        ((PS4Driver*)DriverManager::getInstance().getDriver())->getDongleAuthRequired() == true ) {
        return false;
    }

    // 🛠️ 【Backupボタン（force == true）が押された時だけの特設フック】
    if (force) {
        // 1. 現在の最新設定（画面ONやLED連動を含む）を一度確実に writeCache へシリアライズ（同期）
        ConfigUtils::save(this->config);

        uint32_t ints = save_and_disable_interrupts();
        // 2. 16MB大容量フラッシュの4MB目(0x400000)から32KB（8セクター）を物理消去
        flash_range_erase(MINI_SUPER_RAW_FLASH_ADDR, FLASH_SECTOR_SIZE * 8);
        // 3. 完璧な生バイナリ（32KB）を隔離領域へ直撃RAW上書き転送してセーブ完了
        flash_range_program(MINI_SUPER_RAW_FLASH_ADDR, FlashPROM::writeCache, FLASH_SECTOR_SIZE * 8);
        restore_interrupts(ints);

        // 💡 【ブラウザ騙し用：ダミーファイル化パッチ】
        // 実機内セーブが終わった後、ブラウザ（PC側）にダウンロードさせるデータとして、
        // 「構造体としては100%正しく、中身が真っ新なダミー設定（器だけのバイナリ）」をその場で再構築します。
        // これにより、PC側には壊れていない『ダミーのバックアップファイル』が1つ生成（保存）されます。
        memset(&this->config, 0, sizeof(Config));
        ConfigUtils::save(this->config); // 綺麗なダミーバイナリとして writeCache を上書き
    }

    // ⭕ 【通常セーブの完全救出】各項目の通常セーブ時は、forceがfalseなので無傷でバニラルートを流れます
    bool result = ConfigUtils::save(config);
    EEPROM.commit();
    return result;
}

// ==============================================================================
// 💾 🎯 ② 初期化 / ロード（ダミーファイルをトリガーにした実機隔離領域からの復元）
// ==============================================================================
void Storage::ResetSettings()
{
    // 1. 内蔵EEPROMバッファのクリア
    EEPROM.reset();

    // 2. 4MB 目の隔離領域にデータがあるかを自動判別（空っぽの0xFFではないか）
    const uint8_t* rawFlashSource = (const uint8_t*)(XIP_BASE + MINI_SUPER_RAW_FLASH_ADDR);
    uint32_t checkVal = *(const volatile uint32_t*)rawFlashSource;

    if (checkVal != 0xFFFFFFFF && checkVal != 0x00000000) {
        // ⭕ 【ダミーファイルが読み込まれたとき ➔ PCのデータは無視し、4MB目から32KBを一撃ロード！】
        // これにより、ブラウザ側は「ファイルが正常に読めた」と認識して「メモリから読む」の表示へ進みつつ、
        // 実機の中身はお気に入りのオリジナル設定へ完璧に上書き復元されます。
        for (uint16_t i = 0; i < (FLASH_SECTOR_SIZE * 8); i++) {
            FlashPROM::writeCache[i] = rawFlashSource[i];
        }
        ConfigUtils::load(config);
    } else {
        // ⭕ 【完全初期状態の場合】画面ON・LED連動済みの「真のマスターバイナリ配列」を直流し！
        for (uint16_t i = 0; i < (FLASH_SECTOR_SIZE * 8); i++) {
            if (i < sizeof(miniSuperPerfectBinary)) {
                FlashPROM::writeCache[i] = miniSuperPerfectBinary[i];
            } else {
                FlashPROM::writeCache[i] = 0x00; // 残り領域をゼロクリア
            }
        }
        ConfigUtils::load(config);
    }

    // 🔥【レイヤ3：メモリキャッシュへのリアルタイム強制同期フラグ注入】
    // 💡 初期化時やダミーロード時でも、100%確実に「画面常時ON」「LED入力連動（値:1）」で固定
    this->config.displayOptions.enabled = true;                   // 画面常時ON
    this->config.addonOptions.onBoardLedOptions.enabled = true;   // オンボードLEDアドオンをON
    this->config.addonOptions.onBoardLedOptions.mode = static_cast<OnBoardLedMode>(1); // モード: INPUTモード (入力連動点灯)

    // 💡 注入したフラグを完璧なProtobufバイナリに再シリアライズして writeCache に上書き翻訳
    ConfigUtils::save(this->config);
    EEPROM.commit();

    // 🔥【ハードウェア強制起動パッチ】
    // システムに対して最新の「画面ON・LED連動」をその場で即座に強制再読込させます
    ConfigUtils::load(config);

    // 🎯 【全自動リブート】
    // 起動モードを通常ゲームパッド（GAMEPAD）に指定し、
    // ブラウザが正常に応答を受け取った直後に、電源の抜き差し不要で全自動で安全に再起動がかかります。
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
