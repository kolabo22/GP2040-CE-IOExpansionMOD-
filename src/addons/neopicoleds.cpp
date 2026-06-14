/*
 * SPDX-License-Identifier: MIT
 * SPDX-FileCopyrightText: Copyright (c) 2021 Jason Skuby (mytechtoybox.com)
 */

#include "animationstation.h"
#include "animationstorage.h"
#include "storagemanager.h"
#include "NeoPico.h"
#include "pixel.h"
#include "playerleds.h"
#include "gp2040.h"
#include "addons/neopicoleds.h"
#include "addons/pleds.h"
#include "usbdriver.h"
#include "enums.h"
#include "helper.h"

#define FRAME_MAX 100
#define AL_ROW 5
#define AL_COL 8
#define AL_STATIC_COLOR_COUNT 14
#define AL_EFFECT_MODE_MAX 5
#define CHASE_LIGHTS_TURN_ON 4

const RGB alCustomStaticTheme[AL_ROW][AL_COL] = {
 {{ColorRed, ColorOrange, ColorYellow, ColorGreen, ColorBlue, ColorIndigo, ColorViolet, ColorWhite},
 {ColorOrange, ColorRed, ColorGreen, ColorYellow, ColorIndigo, ColorBlue, ColorWhite, ColorViolet},
 {ColorYellow, ColorOrange, ColorRed, ColorIndigo, ColorBlue, ColorGreen, ColorViolet, ColorWhite},
 {ColorGreen, ColorOrange, ColorYellow, ColorRed, ColorWhite, ColorIndigo, ColorViolet, ColorBlue},
 {ColorWhite, ColorIndigo, ColorViolet, ColorOrange, ColorBlue, ColorGreen, ColorYellow, ColorRed}}
};

const RGB alCustomStaticColors[AL_STATIC_COLOR_COUNT] {
 ColorBlack, ColorWhite, ColorRed, ColorOrange, ColorYellow,
 ColorLimeGreen, ColorGreen, ColorSeafoam, ColorAqua, ColorSkyBlue,
 ColorBlue, ColorPurple, ColorPink, ColorMagenta 
};

const std::string BUTTON_LABEL_UP = "Up";
const std::string BUTTON_LABEL_DOWN = "Down";
const std::string BUTTON_LABEL_LEFT = "Left";
const std::string BUTTON_LABEL_RIGHT = "Right";
const std::string BUTTON_LABEL_B1 = "B1";
const std::string BUTTON_LABEL_B2 = "B2";
const std::string BUTTON_LABEL_B3 = "B3";
const std::string BUTTON_LABEL_B4 = "B4";
const std::string BUTTON_LABEL_L1 = "L1";
const std::string BUTTON_LABEL_R1 = "R1";
const std::string BUTTON_LABEL_L2 = "L2";
const std::string BUTTON_LABEL_R2 = "R2";
const std::string BUTTON_LABEL_S1 = "S1";
const std::string BUTTON_LABEL_S2 = "S2";
const std::string BUTTON_LABEL_L3 = "L3";
const std::string BUTTON_LABEL_R3 = "R3";
const std::string BUTTON_LABEL_A1 = "A1";
const std::string BUTTON_LABEL_A2 = "A2";

static std::vector<uint8_t> EMPTY_VECTOR;
uint32_t rgbPLEDValues[4];

PLEDAnimationState getXInputAnimationNEOPICO(uint16_t ledState) {
	PLEDAnimationState anim = {0, PLED_ANIM_NONE, PLED_SPEED_OFF};
	if (ledState == XINPUT_PLED_BLINKALL || ledState == XINPUT_PLED_ROTATE) {
		anim.state = (PLED_STATE_LED1 | PLED_STATE_LED2 | PLED_STATE_LED3 | PLED_STATE_LED4);
		anim.animation = PLED_ANIM_BLINK; anim.speed = PLED_SPEED_FAST;
	} else if (ledState == XINPUT_PLED_ON1) {
		anim.state = PLED_STATE_LED1; anim.animation = PLED_ANIM_SOLID;
	} else if (ledState == XINPUT_PLED_ON2) {
		anim.state = PLED_STATE_LED2; anim.animation = PLED_ANIM_SOLID;
	} else if (ledState == XINPUT_PLED_ON3) {
		anim.state = PLED_STATE_LED3; anim.animation = PLED_ANIM_SOLID;
	} else if (ledState == XINPUT_PLED_ON4) {
		anim.state = PLED_STATE_LED4; anim.animation = PLED_ANIM_SOLID;
	}
	return anim;
}

PLEDAnimationState getXBoneAnimationNEOPICO(Gamepad *g) {
	PLEDAnimationState anim = {(PLED_STATE_LED1|PLED_STATE_LED2|PLED_STATE_LED3|PLED_STATE_LED4), PLED_ANIM_OFF};
	if (g->auxState.playerID.ledValue == 1) anim.animation = PLED_ANIM_SOLID;
	return anim;
}

PLEDAnimationState getPS3AnimationNEOPICO(uint16_t ledState) {
	PLEDAnimationState anim = {0, PLED_ANIM_NONE, PLED_SPEED_OFF};
	if (ledState != 0) { anim.state = PLED_STATE_LED1; anim.animation = PLED_ANIM_SOLID; }
	return anim;
}

PLEDAnimationState getPS4AnimationNEOPICO(uint32_t fOn, uint32_t fOff) {
	PLEDAnimationState anim = {(PLED_STATE_LED1|PLED_STATE_LED2|PLED_STATE_LED3|PLED_STATE_LED4), PLED_ANIM_SOLID, PLED_SPEED_OFF};
	if (fOn > 0 || fOff > 0) { anim.animation = PLED_ANIM_BLINK_CUSTOM; anim.speedOn = fOn; anim.speedOff = fOff; }
	return anim;
}

PLEDAnimationState getSwitchProAnimationNEOPICO(uint16_t ledState) {
	PLEDAnimationState anim = {0, PLED_ANIM_NONE, PLED_SPEED_OFF};
	if (ledState & 0x01) anim.state |= PLED_STATE_LED1;
	if (anim.state != 0) anim.animation = PLED_ANIM_SOLID;
	return anim;
}
bool NeoPicoLEDAddon::available() {
	LEDOptions& ledOptions = Storage::getInstance().getLedOptions();
	if (!ledOptions.isConfigured) {
		ledOptions.dataPin = 27; // GP27をデータ線に固定
	}
	return isValidPin(ledOptions.dataPin);
}

void NeoPicoLEDAddon::setup() {
	LEDOptions& ledOptions = Storage::getInstance().getLedOptions();

	// 【MINI Super仕様】未設定時のボタン配列・ケース配線をメモリ上へ完全にマッピング
	if (!ledOptions.isConfigured) {
		ledOptions.ledFormat = LED_FORMAT_GRB;
		ledOptions.ledLayout = BUTTON_LAYOUT_ARCADE;
		ledOptions.ledsPerButton = 1;

		// 【8ボタン独自の並び順】配線順インデックスのダイレクト注入（✖・〇・R2・L2・L1・R1・△・▢）
		ledOptions.indexB1 = 0; // ✖ ボタン (インデックス 0)
		ledOptions.indexB2 = 1; // 〇 ボタン (インデックス 1)
		ledOptions.indexR2 = 2; // R2 ボタン (インデックス 2)
		ledOptions.indexL2 = 3; // L2 ボタン (インデックス 3)
		ledOptions.indexL1 = 4; // L1 ボタン (インデックス 4)
		ledOptions.indexR1 = 5; // R1 ボタン (インデックス 5)
		ledOptions.indexB4 = 6; // △ ボタン (インデックス 6)
		ledOptions.indexB3 = 7; // ▢ ボタン (インデックス 7)

		// レバー有りアケコンのため、方向キーや使わない機能ボタンはすべて【-1（LEDなし）】にして完全除外
		ledOptions.indexUp = -1;    ledOptions.indexDown = -1;
		ledOptions.indexLeft = -1;  ledOptions.indexRight = -1;
		ledOptions.indexS1 = -1;    ledOptions.indexS2 = -1; // S2はリアクティブLEDに任せるため除外
		ledOptions.indexL3 = -1;    ledOptions.indexR3 = -1;
		ledOptions.indexA1 = -1;    ledOptions.indexA2 = -1;

		ledOptions.brightnessMaximum = 80; // 最大輝度を80に制限（省電力）
		ledOptions.caseRGBType = CASE_RGB_TYPE_AMBIENT; // 常時点灯アンビエント
		ledOptions.caseRGBIndex = 14; // 9〜13番を省電力消灯するためケースLEDは14番からスタート
		ledOptions.caseRGBCount = 34; // ケースLEDの総数は34個
	}

	Gamepad * gamepad = Storage::getInstance().GetProcessedGamepad();
	gamepad->auxState.playerID.enabled = true;
	gamepad->auxState.sensors.statusLight.enabled = true;

	if ( ledOptions.pledType == PLED_TYPE_RGB ) {
		neoPLEDs = new NeoPicoPlayerLEDs();
	}

	uint8_t buttonCount = setupButtonPositions();
	std::vector<std::vector<Pixel>> pixels = createLEDLayout(static_cast<ButtonLayout>(ledOptions.ledLayout), ledOptions.ledsPerButton, buttonCount);
	matrix.setup(pixels, ledOptions.ledsPerButton);
	ledCount = matrix.getLedCount();
	buttonLedCount = ledCount;

	if (ledOptions.pledType == PLED_TYPE_RGB && PLED_COUNT > 0)
		ledCount += PLED_COUNT;

	const TurboOptions& turboOptions = Storage::getInstance().getAddonOptions().turboOptions;
	if (turboOptions.turboLedType == PLED_TYPE_RGB)
		ledCount += 1;

	if (ledOptions.caseRGBType != CASE_RGB_TYPE_NONE ) {
		ledCount += (int)ledOptions.caseRGBCount;
	}

	neopico.Setup(ledOptions.dataPin, ledCount, static_cast<LEDFormat>(ledOptions.ledFormat), pio0, 0);
	neopico.Off();

	Animation::format = static_cast<LEDFormat>(ledOptions.ledFormat);
 
	const AnimationOptions & animationOptions = Storage::getInstance().getAnimationOptions();
	as.ConfigureBrightness(ledOptions.brightnessMaximum, ledOptions.brightnessSteps);
	as.SetMatrix(matrix);
	as.SetMode(animationOptions.baseAnimationIndex);
	as.SetBrightness(animationOptions.brightness);

	nextRunTime = make_timeout_time_ms(0);
	lastAmbientAction = HOTKEY_LEDS_NONE;

	alBrightnessBreathX = 1.00f;
	breathLedEffectCycle = 0;
	alReverse = false;
	alCurrentFrame = 0;
	alFrameToRGB = 0;
	alFrameSpeed = 2;
	ambientLight.r = 0x00;
	ambientLight.g = 0x00;
	ambientLight.b = 0x00;
	nextRunTimeAmbientLight = make_timeout_time_ms(0);

	chaseLightIndex = ledOptions.caseRGBIndex;
	chaseLightMaxIndexPos = ledCount;
	multipleOfButtonLedsCount = (ledOptions.caseRGBCount) / (buttonLedCount);
	remainderOfButtonLedsCount = (ledOptions.caseRGBCount) % (buttonLedCount);
	alLinkageStartIndex = ledOptions.caseRGBIndex;
}
void NeoPicoLEDAddon::ambientLightCustom() {
	const AnimationOptions& options = Storage::getInstance().getAnimationOptions();
	const LEDOptions& ledOptions = Storage::getInstance().getLedOptions();
	uint8_t alStartIndex = ledOptions.caseRGBIndex;
	int maxFrame = (int)ledOptions.caseRGBCount;

	if (maxFrame > FRAME_MAX - alStartIndex) maxFrame = FRAME_MAX - alStartIndex;

	switch(options.ambientLightEffectsCountIndex) {
		case AL_CUSTOM_EFFECT_STATIC_COLOR: 
			for(int i = 0; i < maxFrame; i++) {
				frame[alStartIndex + i] = alCustomStaticColors[options.alCustomStaticColorIndex].value(Animation::format, options.alStaticColorBrightnessCustomX);
			}
			break;
		case AL_CUSTOM_EFFECT_GRADIENT: 
			alFrameToRGB = 255 - alCurrentFrame;
			ambientLight.r = (alFrameToRGB < 85) ? (255 - alFrameToRGB * 3) : 0;
			ambientLight.g = (alFrameToRGB >= 85 && alFrameToRGB < 170) ? ((alFrameToRGB - 85) * 3) : ((alFrameToRGB >= 170) ? (255 - (alFrameToRGB - 170) * 3) : 0);
			ambientLight.b = (alFrameToRGB < 85) ? (alFrameToRGB * 3) : ((alFrameToRGB < 170) ? (255 - (alFrameToRGB - 85) * 3) : 0);

			if (alReverse) {
				alCurrentFrame -= options.ambientLightGradientSpeed;
				if(alCurrentFrame < 0) { alCurrentFrame = 1; alReverse = false; }
			} else {
				alCurrentFrame += options.ambientLightGradientSpeed;
				if(alCurrentFrame > 255) { alCurrentFrame = 254; alReverse = true; }
			}
			for(int i = 0; i < maxFrame; i++) frame[alStartIndex + i] = ambientLight.value(Animation::format, options.alGradientBrightnessCustomX);
			break;
		case AL_CUSTOM_EFFECT_CHASE: 
			if(time_reached(nextRunTimeAmbientLight)){
				chaseLightIndex = (chaseLightIndex + 1 >= chaseLightMaxIndexPos) ? alStartIndex : chaseLightIndex + 1;
				nextRunTimeAmbientLight = make_timeout_time_ms(options.ambientLightChaseSpeed);
			}
			for(int j = 0; j < maxFrame; j++) frame[alStartIndex + j] = 0x0;
			for(int i = 0; i < CHASE_LIGHTS_TURN_ON && chaseLightIndex + i < chaseLightMaxIndexPos; i++) {
				frame[chaseLightIndex + i] = ColorWhite.value(Animation::format, options.alChaseBrightnessCustomX);
			}
			break;
		case AL_CUSTOM_EFFECT_BREATH:
			alBrightnessBreathX = alReverse ? (alBrightnessBreathX + 0.02f) : (alBrightnessBreathX - 0.02f);
			if(alBrightnessBreathX > 1.0f) { alBrightnessBreathX = 1.0f; alReverse = false; }
			if(alBrightnessBreathX < 0.0f) { alBrightnessBreathX = 0.0f; alReverse = true; }
			for(int i = 0; i < maxFrame; i++) frame[alStartIndex + i] = ColorBlue.value(Animation::format, alBrightnessBreathX);
			break;
		case AL_CUSTOM_EFFECT_STATIC_THEME:
			for(int i = 0; i < maxFrame; i++) frame[alStartIndex + i] = alCustomStaticTheme[options.alCustomStaticThemeIndex][i % AL_COL].value(Animation::format, options.alStaticBrightnessCustomThemeX);
			break;
		default: break;
	}
}

void NeoPicoLEDAddon::ambientLightLinkage() {
	float prB = as.GetLinkageModeOfBrightnessX();
	for(int i = 0; i < multipleOfButtonLedsCount; i++){
		for(int j = 0; j < buttonLedCount; j++) frame[alLinkageStartIndex + i*buttonLedCount + j] = as.linkageFrame[j].value(Animation::format, prB);
	}
}

void NeoPicoLEDAddon::process() {
	const LEDOptions& ledOptions = Storage::getInstance().getLedOptions();
	if (!isValidPin(ledOptions.dataPin) || !time_reached(this->nextRunTime)) return;

	const TurboOptions& turboOptions = Storage::getInstance().getAddonOptions().turboOptions;
	Gamepad * gamepad = Storage::getInstance().GetProcessedGamepad();
	GamepadHotkey action = animationHotkeys(gamepad);

	if (ledOptions.pledType == PLED_TYPE_RGB && neoPLEDs != nullptr) {
		animationState = getXInputAnimationNEOPICO(gamepad->auxState.playerID.ledValue);
		if (animationState.animation != PLED_ANIM_NONE) neoPLEDs->animate(animationState);
	}

	if (action != HOTKEY_LEDS_NONE) as.HandleEvent(action);

	uint32_t buttonState = gamepad->state.dpad << 16 | gamepad->state.buttons;
	vector<Pixel> pressed;
	for (auto row : matrix.pixels) {
		for (auto pixel : row) { if (buttonState & pixel.mask) pressed.push_back(pixel); }
	}

	if (pressed.size() > 0) as.HandlePressed(pressed); else as.ClearPressed();

	as.Animate();
	if (ledOptions.turnOffWhenSuspended && get_usb_suspended()) as.DimBrightnessTo0(); else as.SetBrightness(as.GetBrightness());
	as.ApplyBrightness(&frame[0]);

	// 【MINI Super仕様】9〜13番省電力消灯ゾーン（マスク処理）
	for (int i = 8; i <= 13; i++) {
		frame[i] = 0x00000000;
	}

	if (ledOptions.caseRGBIndex >= 0 && ledOptions.caseRGBCount > 0) {
		ambientHotkeys(gamepad);
		if (ledOptions.caseRGBType == CASE_RGB_TYPE_AMBIENT) this->ambientLightCustom();
		else if (ledOptions.caseRGBType == CASE_RGB_TYPE_LINKED) this->ambientLightLinkage();
	}

	neopico.SetFrame(frame);
	neopico.Show();
	this->nextRunTime = make_timeout_time_ms(intervalMS);
}

std::vector<uint8_t> * NeoPicoLEDAddon::getLEDPositions(string button, std::vector<std::vector<uint8_t>> *positions) {
	int buttonPosition = buttonPositions[button];
	return (buttonPosition < 0) ? &EMPTY_VECTOR : &positions->at(buttonPosition);
}

std::vector<std::vector<Pixel>> NeoPicoLEDAddon::generatedLEDButtons(std::vector<std::vector<uint8_t>> *positions) {
	std::vector<std::vector<Pixel>> pixels = {
		{ PIXEL(BUTTON_LABEL_B3, GAMEPAD_MASK_B3), PIXEL(BUTTON_LABEL_B1, GAMEPAD_MASK_B1) },
		{ PIXEL(BUTTON_LABEL_B4, GAMEPAD_MASK_B4), PIXEL(BUTTON_LABEL_B2, GAMEPAD_MASK_B2) },
		{ PIXEL(BUTTON_LABEL_R1, GAMEPAD_MASK_R1), PIXEL(BUTTON_LABEL_R2, GAMEPAD_MASK_R2) },
		{ PIXEL(BUTTON_LABEL_L1, GAMEPAD_MASK_L1), PIXEL(BUTTON_LABEL_L2, GAMEPAD_MASK_L2) }
	};
	return pixels;
}

std::vector<std::vector<Pixel>> NeoPicoLEDAddon::generatedLEDStickless(vector<vector<uint8_t>> *positions) { return generatedLEDButtons(positions); }
std::vector<std::vector<Pixel>> NeoPicoLEDAddon::generatedLEDWasd(std::vector<std::vector<uint8_t>> *positions) { return generatedLEDButtons(positions); }
std::vector<std::vector<Pixel>> NeoPicoLEDAddon::generatedLEDWasdFBM(std::vector<std::vector<uint8_t>> *positions) { return generatedLEDButtons(positions); }

std::vector<std::vector<Pixel>> NeoPicoLEDAddon::createLEDLayout(ButtonLayout layout, uint8_t ledsPerPixel, uint8_t ledButtonCount) {
	vector<vector<uint8_t>> positions(ledButtonCount);
	for (int i = 0; i != ledButtonCount; i++) {
		positions[i].resize(ledsPerPixel);
		for (int l = 0; l != ledsPerPixel; l++) positions[i][l] = (i * ledsPerPixel) + l;
	}
	return generatedLEDButtons(&positions);
}

uint8_t NeoPicoLEDAddon::setupButtonPositions() {
	const LEDOptions& ledOptions = Storage::getInstance().getLedOptions();
	buttonPositions.clear();
	buttonPositions.emplace(BUTTON_LABEL_B1, ledOptions.indexB1);
	buttonPositions.emplace(BUTTON_LABEL_B2, ledOptions.indexB2);
	buttonPositions.emplace(BUTTON_LABEL_B3, ledOptions.indexB3);
	buttonPositions.emplace(BUTTON_LABEL_B4, ledOptions.indexB4);
	buttonPositions.emplace(BUTTON_LABEL_L1, ledOptions.indexL1);
	buttonPositions.emplace(BUTTON_LABEL_R1, ledOptions.indexR1);
	buttonPositions.emplace(BUTTON_LABEL_L2, ledOptions.indexL2);
	buttonPositions.emplace(BUTTON_LABEL_R2, ledOptions.indexR2);
	uint8_t buttonCount = 0;
	for (auto const& bp : buttonPositions) { if (bp.second > -1) buttonCount++; }
	return buttonCount;
}

GamepadHotkey NeoPicoLEDAddon::animationHotkeys(Gamepad *gamepad) {
	if (gamepad->pressedS1() && gamepad->pressedS2()) {
		if (gamepad->pressedB3()) { gamepad->state.buttons &= ~GAMEPAD_MASK_B3; return HOTKEY_LEDS_ANIMATION_UP; }
		if (gamepad->pressedB1()) { gamepad->state.buttons &= ~GAMEPAD_MASK_B1; return HOTKEY_LEDS_ANIMATION_DOWN; }
	}
	return HOTKEY_LEDS_NONE;
}

void NeoPicoLEDAddon::ambientHotkeys(Gamepad *gamepad) {
	bool reqSave = false;
	AnimationOptions & animationOptions = Storage::getInstance().getAnimationOptions();
	if(gamepad->pressedS2() && !gamepad->pressedS1()) { 
		if(gamepad->pressedL1()) {
			animationOptions.ambientLightEffectsCountIndex = (animationOptions.ambientLightEffectsCountIndex + 1) % AL_EFFECT_MODE_MAX;
			reqSave = true;
		}
	}
	if (reqSave) EventManager::getInstance().triggerEvent(new GPStorageSaveEvent(false));
}
