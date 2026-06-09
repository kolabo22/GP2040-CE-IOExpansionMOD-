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

void Storage::init() {
	systemFlashSize = System::getPhysicalFlash(); // System Flash Size must be called once
	EEPROM.start();
	ConfigUtils::load(config);

	// ============================================================================
	// 【LOCK】MINI Super 専用：起動時ストレージ強制同期
	// ============================================================================
	
	// 0. ボード名をシステムに強制固定
	strncpy(this->config.boardConfig, "MINI Super", sizeof(this->config.boardConfig) - 1);
	this->config.boardConfig[sizeof(this->config.boardConfig) - 1] = '\0';
	
	// 1. 基本入力・SOCD・4方向レバー・5msデバウンスの強制上書き
	this->config.gamepadOptions.inputMode = INPUT_MODE_GENERIC;
	this->config.gamepadOptions.has_inputMode = true;
	this->config.gamepadOptions.socdMode = SOCD_MODE_NEUTRAL;
	this->config.gamepadOptions.has_socdMode = true;
	this->config.gamepadOptions.dpadMode = DPAD_MODE_DIGITAL;
	this->config.gamepadOptions.has_dpadMode = true;
	this->config.gamepadOptions.debounceDelay = 5;
	this->config.gamepadOptions.has_debounceDelay = true;
	this->config.gamepadOptions.fourWayMode = true;
	this->config.gamepadOptions.has_fourWayMode = true;

	// 2. 周辺機器設定（I2C0, I2C1の強制アクティブ有効化）
	this->config.peripheralOptions.blockI2C0.enabled = true;
	this->config.peripheralOptions.blockI2C0.sda = 0;
	this->config.peripheralOptions.blockI2C0.scl = 1;
	this->config.peripheralOptions.blockI2C0.speed = 400000;
	this->config.peripheralOptions.has_blockI2C0 = true;

	this->config.peripheralOptions.blockI2C1.enabled = true;
	this->config.peripheralOptions.blockI2C1.sda = 18;
	this->config.peripheralOptions.blockI2C1.scl = 19;
	this->config.peripheralOptions.blockI2C1.speed = 400000;
	this->config.peripheralOptions.has_blockI2C1 = true;

	this->config.peripheralOptions.blockSPI0.enabled = false;
	this->config.peripheralOptions.has_blockSPI0 = true;

	// 3. 各種アドオン機能の強制チェックON（WebConfig同期型）
	this->config.addonOptions.turboOptions.enabled = true;
	this->config.addonOptions.turboOptions.has_enabled = true;

	this->config.addonOptions.onBoardLedOptions.enabled = true;
	this->config.addonOptions.onBoardLedOptions.has_enabled = true;

	this->config.addonOptions.wiiOptions.enabled = true;
	this->config.addonOptions.wiiOptions.has_enabled = true;

	this->config.addonOptions.reactiveLEDOptions.enabled = true;
	this->config.addonOptions.reactiveLEDOptions.has_enabled = true;

	// PCF8575 IOエクスパンダー 16ピンのメモリ強制書き換え
	this->config.addonOptions.pcf8575Options.enabled = true;
	this->config.addonOptions.pcf8575Options.has_enabled = true;
	this->config.addonOptions.pcf8575Options.pins[0].action  = GpioAction::BUTTON_PRESS_A3;
	this->config.addonOptions.pcf8575Options.pins[1].action  = GpioAction::BUTTON_PRESS_A2;
	this->config.addonOptions.pcf8575Options.pins[2].action  = GpioAction::BUTTON_PRESS_E1;
	this->config.addonOptions.pcf8575Options.pins[3].action  = GpioAction::BUTTON_PRESS_E2;
	this->config.addonOptions.pcf8575Options.pins[4].action  = GpioAction::BUTTON_PRESS_E3;
	this->config.addonOptions.pcf8575Options.pins[5].action  = GpioAction::BUTTON_PRESS_E4;
	this->config.addonOptions.pcf8575Options.pins[6].action  = GpioAction::BUTTON_PRESS_E5;
	this->config.addonOptions.pcf8575Options.pins[7].action  = GpioAction::BUTTON_PRESS_E6;
	this->config.addonOptions.pcf8575Options.pins[8].action  = GpioAction::BUTTON_PRESS_A4;
	this->config.addonOptions.pcf8575Options.pins[9].action  = GpioAction::BUTTON_PRESS_L3;
	this->config.addonOptions.pcf8575Options.pins[10].action = GpioAction::BUTTON_PRESS_R3;
	this->config.addonOptions.pcf8575Options.pins[11].action = GpioAction::BUTTON_PRESS_S1;
	this->config.addonOptions.pcf8575Options.pins[12].action = GpioAction::BUTTON_PRESS_A1;
	this->config.addonOptions.pcf8575Options.pins[13].action = GpioAction::NONE;
	this->config.addonOptions.pcf8575Options.pins[14].action = GpioAction::BUTTON_PRESS_E7;
	this->config.addonOptions.pcf8575Options.pins[15].action = GpioAction::BUTTON_PRESS_E8;
	for (int p = 0; p < 16; p++) {
		this->config.addonOptions.pcf8575Options.pins[p].direction = GpioDirection::GPIO_DIRECTION_INPUT;
		this->config.addonOptions.pcf8575Options.pins[p].has_action = true;
		this->config.addonOptions.pcf8575Options.pins[p].has_direction = true;
	}
	this->config.addonOptions.pcf8575Options.pins_count = 16;

	// 4. RGB LED 輝度・データピン固定
	this->config.ledOptions.dataPin = 27;
	this->config.ledOptions.has_dataPin = true;
	this->config.ledOptions.brightnessMaximum = 80;
	this->config.ledOptions.has_brightnessMaximum = true;
	this->config.ledOptions.brightnessSteps = 10;
	this->config.ledOptions.has_brightnessSteps = true;
	this->config.ledOptions.turnOffWhenSuspended = true;
	this->config.ledOptions.has_turnOffWhenSuspended = true;

	// 5. ディスプレイ構成（OLED）のロック
	this->config.displayOptions.enabled = true;
	this->config.displayOptions.has_enabled = true;
	this->config.displayOptions.buttonLayout = BUTTON_LAYOUT_STICK;
	this->config.displayOptions.has_buttonLayout = true;
	this->config.displayOptions.buttonLayoutRight = BUTTON_LAYOUT_VLXB; 
	this->config.displayOptions.has_buttonLayoutRight = true;
	this->config.displayOptions.splashMode = SplashMode::SPLASH_MODE_STATIC; 
	this->config.displayOptions.has_splashMode = true;
	this->config.displayOptions.splashDuration = 7000;
	this->config.displayOptions.has_splashDuration = true;
	this->config.displayOptions.displaySaverTimeout = 600000;
	this->config.displayOptions.has_displaySaverTimeout = true;
	this->config.displayOptions.displaySaverMode = static_cast<DisplaySaverMode>(2); // 雪モード
	this->config.displayOptions.has_displaySaverMode = true;
	
	this->config.displayOptions.inputHistoryEnabled = true;
	this->config.displayOptions.has_inputHistoryEnabled = true;
	this->config.displayOptions.inputHistoryLength = 21;
	this->config.displayOptions.has_inputHistoryLength = true;
	this->config.displayOptions.inputHistoryCol = 0;
	this->config.displayOptions.has_inputHistoryCol = true;
	this->config.displayOptions.inputHistoryRow = 7;
	this->config.displayOptions.has_inputHistoryRow = true;

	// 6. 【最重要】強制セーブフラグ(true)で即時保存を実行
	this->save(true);
}

/**
 * @brief Save the config, but only if it is safe to (as in USB host is not being used.)
 */
bool Storage::save()
{
	return save(false);
}

/**
 * @brief Save the config; if forcing a save is requested, or if USB host is not enabled, this will write to flash.
 */
bool Storage::save(const bool force) {
	// Conditions for saving:
	//   1. Force = True
	//   2. Input Mode NOT (PS4/PS5 with USB enabled)
	// Save will disconnect USB host, which is okay for gamepad and keyboard hosts
	if (!force &&
		PeripheralManager::getInstance().isUSBEnabled(0) &&
		(DriverManager::getInstance().getInputMode() == INPUT_MODE_PS4 ||
			DriverManager::getInstance().getInputMode() == INPUT_MODE_PS5) &&
		((PS4Driver*)DriverManager::getInstance().getDriver())->getDongleAuthRequired() == true ) {
		return false;
	}

	return ConfigUtils::save(config);
}

void Storage::ResetSettings()
{
	EEPROM.reset();
	watchdog_reboot(0, SRAM_END, 2000);
}

bool Storage::setProfile(const uint32_t profileNum)
{
	uint32_t profileCeiling = config.profileOptions.gpioMappingsSets_count + 1;
	
	// is this profile defined?
	if (profileNum >= 1 && profileNum <= profileCeiling) {
		// is this profile enabled?
		// profile 1 (core) is always enabled, others we must check
		if (profileNum == 1 || config.profileOptions.gpioMappingsSets[profileNum-2].enabled) {
			// Update the profile number - reinit will be triggered automatically in gp2040.cpp
			this->config.gamepadOptions.profileNumber = profileNum;
			return true;
		}
	}
	// if we get here, the requested profile doesn't exist or isn't enabled, so don't change it
	return false;
}

void Storage::nextProfile()
{
	uint32_t profileCeiling = config.profileOptions.gpioMappingsSets_count + 1;
	uint32_t requestedProfile = (this->config.gamepadOptions.profileNumber % profileCeiling) + 1;
	while (!setProfile(requestedProfile)) {
		// if the set failed, try again with the next in the sequence
		requestedProfile = (requestedProfile % profileCeiling) + 1;
	}
}
void Storage::previousProfile()
{
	uint32_t profileCeiling = config.profileOptions.gpioMappingsSets_count + 1;
	uint32_t requestedProfile = this->config.gamepadOptions.profileNumber > 1 ?
			config.gamepadOptions.profileNumber - 1 : profileCeiling;
	while (!setProfile(requestedProfile)) {
		// if the set failed, try again with the next in the sequence
		requestedProfile = requestedProfile > 1 ? requestedProfile - 1 : profileCeiling;
	}
}

/**
 * @brief Return the current profile label.
 */
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
		// assign the functional pin to the profile pin if:
		// 1: there was a profile to load
		// 2: the new action isn't RESERVED or ASSIGNED_TO_ADDON (profiles can't affect special addons)
		// 3: the old action isn't RESERVED or ASSIGNED_TO_ADDON (profiles can't affect special addons)
		// else use whatever is in the core mapping
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

void Storage::SetGamepad(Gamepad * newpad)
{
	gamepad = newpad;
}

Gamepad * Storage::GetGamepad()
{
	return gamepad;
}

void Storage::SetProcessedGamepad(Gamepad * newpad)
{
	processedGamepad = newpad;
}

Gamepad * Storage::GetProcessedGamepad()
{
	return processedGamepad;
}
