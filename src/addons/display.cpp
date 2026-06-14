/*
 * SPDX-License-Identifier: MIT
 * SPDX-FileCopyrightText: Copyright (c) 2024 OpenStickCommunity (gp2040-ce.info)
 */

#include "addons/display.h"
#include "GamepadState.h"
#include "enums.h"
#include "storagemanager.h"
#include "pico/stdlib.h"
#include "drivermanager.h"
#include "usbdriver.h"
#include "version.h"
#include "config.pb.h"
#include "class/hid/hid.h"

bool DisplayAddon::available() {
	// 1. 参照元を非constにして取得（WebConfigデータ構造体）
	DisplayOptions& options = Storage::getInstance().getDisplayOptions();

	// 2. WebConfigに保存データがない（未設定リセット）場合のみ初期値をダイレクト上書き
	if (!options.isConfigured) {
		options.enabled = true;
		options.i2cBlock = 0;                 // i2c0 を強制指定 (SDA:GP0, SCL:GP1)
		options.size = 0;                     // 0: 128x64 解像度
		options.flip = 0;                     // 画面反転なし
		options.invert = false;               // 白黒反転なし
		options.contrast = 128;               // 【MINI Super仕様】明るさ半分（焼き付き防止・省電力）
		options.displaySaverTimeout = 600000; // スクリーンセーバー起動時間 (10分: 600000ms)
		
		// スプラッシュ画面設定 (Protobuf列挙型をキャスト代入して安全に適合)
		options.splashMode = static_cast<SplashMode>(1); // 1: 標準ロゴモード
		options.splashDuration = 7000;        // スプラッシュ表示時間 7秒 (7000ms)
	}

	bool result = false;
	// create the gfx interface
	gpDisplay = new GPGFX();
	gpOptions = gpDisplay->getAvailableDisplay(GPGFX_DisplayType::DISPLAY_TYPE_NONE);
	if ( gpOptions.displayType != GPGFX_DisplayType::DISPLAY_TYPE_NONE ) {
		if ( options.enabled ) {
			result = true;
		} else {
			// Power off our display if its available but disabled in config
			gpOptions.size = options.size;
			gpOptions.orientation = options.flip;
			gpOptions.inverted = options.invert;
			gpOptions.font.fontData = GP_Font_Standard;
			gpOptions.font.width = 6;
			gpOptions.font.height = 8;
			gpOptions.contrast = options.contrast;
			gpDisplay->init(gpOptions);
			setDisplayPower(0);
			delete gpDisplay;
			result = false;
		}
	} else { // No display, delete our GPGFX
		delete gpDisplay;
	}
	return result;
}

void DisplayAddon::setup() {
	// setupでも未設定時の値を完全に保証する
	DisplayOptions& options = Storage::getInstance().getDisplayOptions();
	if (!options.isConfigured) {
		options.enabled = true;
		options.i2cBlock = 0;
		options.size = 0;
		options.flip = 0;
		options.invert = false;
		options.contrast = 128;
		options.displaySaverTimeout = 600000;
		options.splashMode = static_cast<SplashMode>(1);
		options.splashDuration = 7000;
	}

	// Setup GPGFX Options
	if (gpOptions.displayType != GPGFX_DisplayType::DISPLAY_TYPE_NONE) {
		gpOptions.size = options.size;
		gpOptions.orientation = options.flip;
		gpOptions.inverted = options.invert;
		gpOptions.font.fontData = GP_Font_Standard;
		gpOptions.font.width = 6;
		gpOptions.font.height = 8;
		gpOptions.contrast = options.contrast;
	} else {
		return;
	}

	// Setup GPGFX
	gpDisplay->init(gpOptions);
	displaySaverTimer = options.displaySaverTimeout;
	displaySaverTimeout = displaySaverTimer;
	configMode = DriverManager::getInstance().isConfigMode();
	turnOffWhenSuspended = options.turnOffWhenSuspended;
	displaySaverMode = options.displaySaverMode;
	prevValues = Storage::getInstance().GetGamepad()->debouncedGpio;

	// set current display mode
	if (!configMode) {
		if (Storage::getInstance().getDisplayOptions().splashMode != static_cast<SplashMode>(SPLASH_MODE_NONE)) {
			currDisplayMode = DisplayMode::SPLASH;
		} else {
			currDisplayMode = DisplayMode::BUTTONS;
		}
	} else {
		currDisplayMode = DisplayMode::CONFIG_INSTRUCTION;
	}
	gpScreen = nullptr;
	updateDisplayScreen();
	setMenuMappings();

	EventManager::getInstance().registerEventHandler(GP_EVENT_PROFILE_CHANGE, GPEVENT_CALLBACK(this->handleProfileChange(event)));
	EventManager::getInstance().registerEventHandler(GP_EVENT_RESTART, GPEVENT_CALLBACK(this->handleSystemRestart(event)));
	EventManager::getInstance().registerEventHandler(GP_EVENT_MENU_NAVIGATE, GPEVENT_CALLBACK(this->handleMenuNavigation(event)));
	EventManager::getInstance().registerEventHandler(GP_EVENT_SYSTEM_ERROR, GPEVENT_CALLBACK(this->handleSystemError(event)));
}

bool DisplayAddon::updateDisplayScreen() {
	if ( gpScreen != nullptr ) {
		gpScreen->shutdown();
		delete gpScreen; // Virtual deconstructor
		gpScreen = nullptr;
	}
	switch(currDisplayMode) {
		case CONFIG_INSTRUCTION:
			gpScreen = new ConfigScreen(gpDisplay);
			break;
		case SPLASH:
			gpScreen = new SplashScreen(gpDisplay);
			break;
		case MAIN_MENU:
			gpScreen = new MainMenuScreen(gpDisplay);
			break;
		case BUTTONS:
			gpScreen = new ButtonLayoutScreen(gpDisplay);
			break;
		case PIN_VIEWER:
			gpScreen = new PinViewerScreen(gpDisplay);
			break;
		case DISPLAY_SAVER:
			gpScreen = new DisplaySaverScreen(gpDisplay);
			break;
		case STATS:
			gpScreen = new StatsScreen(gpDisplay);
			break;
		case SYSTEM_ERROR:
			gpScreen = new SystemErrorScreen(gpDisplay, errorMessage);
			break;
		case RESTART:
			gpScreen = new RestartScreen(gpDisplay, bootMode);
			break;
		default:
			gpScreen = nullptr;
			break;
	}
	if (gpScreen == nullptr )
		return false;
	gpScreen->init();
	prevDisplayMode = currDisplayMode;
	nextDisplayMode = currDisplayMode;
	return true;
}

bool DisplayAddon::isDisplayPowerOff()
{
	Gamepad * gamepad = Storage::getInstance().GetGamepad();
	if (turnOffWhenSuspended && get_usb_suspended()) {
		if (displayIsPowerOn)
			setDisplayPower(0);
		return true;
	} else {
		if (!displayIsPowerOn)
			setDisplayPower(1);
	}

	if (!displaySaverTimeout) return false;
	float diffTime = getMillis() - prevMillis;
	displaySaverTimer -= diffTime;
	if (!!displaySaverTimeout && (gamepad->state.buttons || gamepad->state.dpad)) {
		displaySaverTimer = displaySaverTimeout;
		setDisplayPower(1);
	} else if (!!displaySaverTimeout && displaySaverTimer <= 0) {
		if (displaySaverMode == DisplaySaverMode::DISPLAY_SAVER_DISPLAY_OFF) {
			setDisplayPower(0);
		} else {
			if (currDisplayMode != DISPLAY_SAVER) {
				currDisplayMode = DISPLAY_SAVER;
				updateDisplayScreen();
			}
		}
	}
	prevMillis = getMillis();
	return ((!!displaySaverTimeout && displaySaverTimer <= 0) && (displaySaverMode == DisplaySaverMode::DISPLAY_SAVER_DISPLAY_OFF));
}

void DisplayAddon::setDisplayPower(uint8_t status)
{
	if (displayIsPowerOn != status) {
		displayIsPowerOn = status;
		gpDisplay->getDriver()->setPower(status);
	}
}

void DisplayAddon::setMenuMappings()
{
	mapMenuToggle = new GamepadButtonMapping(0);
	mapMenuSelect = new GamepadButtonMapping(0);
	GpioMappingInfo* pinMappings = Storage::getInstance().getProfilePinMappings();
	for (Pin_t pin = 0; pin < (Pin_t)NUM_BANK0_GPIOS; pin++) {
		switch (pinMappings[pin].action) {
			case GpioAction::MENU_NAVIGATION_TOGGLE: mapMenuToggle->pinMask |= 1 << pin; break;
			case GpioAction::MENU_NAVIGATION_SELECT: mapMenuSelect->pinMask |= 1 << pin; break;
			default: break;
		}
	}
}

void DisplayAddon::process() {
	// If GPDisplay is not loaded or we're in standard mode with display power off enabled
	if (gpDisplay->getDriver() == nullptr || (!configMode && isDisplayPowerOff())) {
		return;
	}
	// Core0 requested a new display mode
	if (nextDisplayMode != currDisplayMode ) {
		currDisplayMode = nextDisplayMode;
		updateDisplayScreen();
	}
	int8_t screenReturn = gpScreen->update();
	gpScreen->draw();
	if (!configMode && screenReturn < 0) {
		Mask_t values = Storage::getInstance().GetGamepad()->debouncedGpio;
		if (prevValues != values) {
			if ((values & mapMenuToggle->pinMask) || (values & mapMenuSelect->pinMask)) {
				if (currDisplayMode != DisplayMode::MAIN_MENU) {
					screenReturn = DisplayMode::MAIN_MENU;
				}
			}
			prevValues = values;
		}
	}
	// -1 = we do not change state
	if (screenReturn >= 0) {
		// Screen wants to change to something else
		if (screenReturn != currDisplayMode) {
			currDisplayMode = (DisplayMode)screenReturn;
			updateDisplayScreen();
		}
	}
}

const DisplayOptions& DisplayAddon::getDisplayOptions() {
	return Storage::getInstance().getDisplayOptions();
}

void DisplayAddon::handleProfileChange(GPEvent* e)
{
	delete mapMenuToggle;
	delete mapMenuSelect;
	mapMenuToggle = nullptr;
	mapMenuSelect = nullptr;
	setMenuMappings();
}

void DisplayAddon::handleSystemRestart(GPEvent* e) {
	nextDisplayMode = DisplayMode::RESTART;
	bootMode = (uint32_t)((GPRestartEvent*)e)->bootMode;
}

void DisplayAddon::handleMenuNavigation(GPEvent* e) {
	// Swap between main menu and buttons if we press toggle
	if (((GPMenuNavigateEvent*)e)->menuAction == GpioAction::MENU_NAVIGATION_TOGGLE) {
		if (currDisplayMode == BUTTONS) {
			nextDisplayMode = MAIN_MENU;
		} else if (currDisplayMode == MAIN_MENU) {
			nextDisplayMode = BUTTONS;
		}
	} else if (currDisplayMode == MAIN_MENU) {
		((MainMenuScreen*)gpScreen)->updateEventMenuNavigation(((GPMenuNavigateEvent*)e)->menuAction);
	}
}

void DisplayAddon::handleSystemError(GPEvent* e) {
	currDisplayMode = SYSTEM_ERROR;
	errorMessage = ((GPSystemErrorEvent*) e)->errorMessage;
}
