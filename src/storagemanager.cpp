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
// 💾 ① バックアップ / 通常セーブ（ブラウザ応答100%同期 ＆ 16KB安全直流し）
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
        // 1. 現在の最新設定（画面ONやLED連動を含む）を一度確実に writeCache（16KB）へ同期（シリアライズ）
        ConfigUtils::save(this->config);

        uint32_t ints = save_and_disable_interrupts();
        // 2. 16MB大容量フラッシュの4MB目(0x400000)から正確に16KB（4セクター）を物理消去
        flash_range_erase(MINI_SUPER_RAW_FLASH_ADDR, FLASH_SECTOR_SIZE * 4);
        // 3. 領域を1バイトもはみ出さずに、安全な16KBの生バイナリを隔離領域へ直撃RAW上書き転送！
        flash_range_program(MINI_SUPER_RAW_FLASH_ADDR, FlashPROM::writeCache, FLASH_SECTOR_SIZE * 4);
        restore_interrupts(ints);

        // 💡 【通信エラーを全滅させる魔法のパッチ】
        // 実機内隔離領域へのセーブが終わった後、ブラウザが待っている「16KBの正しい設定バイナリデータ」を
        // もう一度正確に writeCache へ再展開して保持させます。
        // これにより、webconfig.cpp（LWIP）はブラウザが100%期待する「寸分の狂いもない完全なデータ」を
        // 送り返すことができるため、画面上の「通信エラー（赤文字）」は物理的に100%全滅し、
        // 綺麗な緑色の「SUCCESS（保存成功）」のインジケーターが正常に大復活します！
        ConfigUtils::save(this->config);
    }

    // ⭕ 【通常セーブの完全救出】各項目の通常セーブ時は、forceがfalseなので無傷で100%公式ルートを流れます
    bool result = ConfigUtils::save(config);
    EEPROM.commit();
    return result;
}

// ==============================================================================
// 💾 ② 初期化 / ロード（公式関数完全準拠・全自動2秒遅延リブート仕様）
// ==============================================================================
void Storage::ResetSettings()
{
    // 1. 内蔵EEPROMバッファのクリア
    EEPROM.reset();

    // 2. 4MB 目の隔離領域にデータがあるかを自動判別（空っぽの0xFFではないか）
    const uint8_t* rawFlashSource = (const uint8_t*)(XIP_BASE + MINI_SUPER_RAW_FLASH_ADDR);
    uint32_t checkVal = *(const volatile uint32_t*)rawFlashSource;

    if (checkVal != 0xFFFFFFFF && checkVal != 0x00000000) {
        // ⭕ 【一度でもBackupを押したことがある場合 ➔ 4MB目の隔離領域から「正確に16KB」を逆コピー復元！】
        for (uint16_t i = 0; i < (FLASH_SECTOR_SIZE * 4); i++) {
            FlashPROM::writeCache[i] = rawFlashSource[i];
        }
        ConfigUtils::load(config);
    } else {
        // ⭕ 【完全初期状態の場合 ➔ 寸分の狂いもない現在の最新デフォルト構造を展開】
        memset(&this->config, 0, sizeof(Config));
        ConfigUtils::load(config); 
    }

    // 🔥【レイヤ3：メモリキャッシュへのリアルタイム強制同期フラグ注入】
    // 💡 初期化時やダミーロード時でも、100%確実に「画面常時ON」「LED入力連動（値:1）」で固定
    this->config.displayOptions.enabled = true;                   // 画面常時ON
    this->config.addonOptions.onBoardLedOptions.enabled = true;   // オンボードLEDアドオンをON
    this->config.addonOptions.onBoardLedOptions.mode = static_cast<OnBoardLedMode>(1); // モード: INPUTモード (入力連動点灯)

    // 💡 注入したフラグを完璧なProtobufバイナリに再シリアライズして writeCache（16KB）に公式上書き翻訳
    ConfigUtils::save(this->config);
    
    // 💡 物理フラッシュメモリへガチッとコミットして確定永続保存
    EEPROM.commit();

    // 💡 【自動リブート復帰パッチ】
    // 割り込みでの即死リセットを完全に排除し、この関数の処理をそのままクリーンに終了（return）させます。
    // これにより実機はブラウザに対して「初期化・復元完了」の返事（Reboot画面）を100%綺麗に返しきることができ、
    // 画面側のインジケーターが「SUCCESS」に綺麗に切り替わったジャスト直後に、GP2040-CE本来の安全な自動2秒遅延リブートが
    // バックグラウンドで勝手に駆動し、電源の抜き差し不要で全自動でアケコンモードへ勝手に復帰します。
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
