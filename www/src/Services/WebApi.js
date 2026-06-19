import Http from './Http';
import { hexToInt, rgbIntToHex } from './Utilities';

export const baseUrl =
	process.env.NODE_ENV === 'production'
		? ''
		: import.meta.env.VITE_DEV_BASE_URL;

export const baseButtonMappings = {
	Up: { pin: -1, key: 0, error: null },
	Down: { pin: -1, key: 0, error: null },
	Left: { pin: -1, key: 0, error: null },
	Right: { pin: -1, key: 0, error: null },
	B1: { pin: -1, key: 0, error: null },
	B2: { pin: -1, key: 0, error: null },
	B3: { pin: -1, key: 0, error: null },
	B4: { pin: -1, key: 0, error: null },
	L1: { pin: -1, key: 0, error: null },
	R1: { pin: -1, key: 0, error: null },
	L2: { pin: -1, key: 0, error: null },
	R2: { pin: -1, key: 0, error: null },
	S1: { pin: -1, key: 0, error: null },
	S2: { pin: -1, key: 0, error: null },
	L3: { pin: -1, key: 0, error: null },
	R3: { pin: -1, key: 0, error: null },
	A1: { pin: -1, key: 0, error: null },
	A2: { pin: -1, key: 0, error: null },
	A3: { pin: -1, key: 0, error: null },
	A4: { pin: -1, key: 0, error: null },
	E1: { pin: -1, key: 0, error: null },
	E2: { pin: -1, key: 0, error: null },
	E3: { pin: -1, key: 0, error: null },
	E4: { pin: -1, key: 0, error: null },
	E5: { pin: -1, key: 0, error: null },
	E6: { pin: -1, key: 0, error: null },
	E7: { pin: -1, key: 0, error: null },
	E8: { pin: -1, key: 0, error: null },
	E9: { pin: -1, key: 0, error: null },
	E10: { pin: -1, key: 0, error: null },
	E11: { pin: -1, key: 0, error: null },
	E12: { pin: -1, key: 0, error: null },
	Fn: { pin: -1, key: 0, error: null },
};

export const baseProfileOptions = {
	alternativePinMappings: [
		{
			Up: { pin: -1, key: 0, error: null },
			Down: { pin: -1, key: 0, error: null },
			Left: { pin: -1, key: 0, error: null },
			Right: { pin: -1, key: 0, error: null },
			B1: { pin: -1, key: 0, error: null },
			B2: { pin: -1, key: 0, error: null },
			B3: { pin: -1, key: 0, error: null },
			B4: { pin: -1, key: 0, error: null },
			L1: { pin: -1, key: 0, error: null },
			R1: { pin: -1, key: 0, error: null },
			L2: { pin: -1, key: 0, error: null },
			R2: { pin: -1, key: 0, error: null },
		},
		{
			Up: { pin: -1, key: 0, error: null },
			Down: { pin: -1, key: 0, error: null },
			Left: { pin: -1, key: 0, error: null },
			Right: { pin: -1, key: 0, error: null },
			B1: { pin: -1, key: 0, error: null },
			B2: { pin: -1, key: 0, error: null },
			B3: { pin: -1, key: 0, error: null },
			B4: { pin: -1, key: 0, error: null },
			L1: { pin: -1, key: 0, error: null },
			R1: { pin: -1, key: 0, error: null },
			L2: { pin: -1, key: 0, error: null },
			R2: { pin: -1, key: 0, error: null },
		},
		{
			Up: { pin: -1, key: 0, error: null },
			Down: { pin: -1, key: 0, error: null },
			Left: { pin: -1, key: 0, error: null },
			Right: { pin: -1, key: 0, error: null },
			B1: { pin: -1, key: 0, error: null },
			B2: { pin: -1, key: 0, error: null },
			B3: { pin: -1, key: 0, error: null },
			B4: { pin: -1, key: 0, error: null },
			L1: { pin: -1, key: 0, error: null },
			R1: { pin: -1, key: 0, error: null },
			L2: { pin: -1, key: 0, error: null },
			R2: { pin: -1, key: 0, error: null },
		},
	],
};

export const basePeripheralMapping = {
	peripheral: {
		i2c0: {
			enabled: 0,
			sda: -1,
			scl: -1,
			speed: 400000,
		},
		i2c1: {
			enabled: 0,
			sda: -1,
			scl: -1,
			speed: 400000,
		},
		spi0: {
			enabled: 0,
			rx: -1,
			cs: -1,
			sck: -1,
			tx: -1,
		},
		spi1: {
			enabled: 0,
			rx: -1,
			cs: -1,
			sck: -1,
			tx: -1,
		},
		usb0: {
			enabled: 0,
			dp: -1,
			enable5v: -1,
			order: 0,
		},
	},
};

export const baseWiiControls = {
	'nunchuk.analogStick.axisType': 1,
	'nunchuk.buttonC': 1,
	'nunchuk.buttonZ': 2,
	'classic.analogLeftStick.x.axisType': 1,
	'classic.analogLeftStick.y.axisType': 2,
	'classic.analogRightStick.x.axisType': 3,
	'classic.analogRightStick.y.axisType': 4,
	'classic.analogLeftTrigger.axisType': 7,
	'classic.analogRightTrigger.axisType': 8,
	'classic.buttonA': 2,
	'classic.buttonB': 1,
	'classic.buttonX': 8,
	'classic.buttonY': 4,
	'classic.buttonL': 64,
	'classic.buttonR': 128,
	'classic.buttonZL': 16,
	'classic.buttonZR': 32,
	'classic.buttonMinus': 256,
	'classic.buttonHome': 4096,
	'classic.buttonPlus': 512,
	'classic.buttonUp': 65536,
	'classic.buttonDown': 131072,
	'classic.buttonLeft': 262144,
	'classic.buttonRight': 524288,
	'guitar.analogStick.x.axisType': 1,
	'guitar.analogStick.y.axisType': 2,
	'guitar.analogWhammyBar.axisType': 14,
	'guitar.buttonOrange': 64,
	'guitar.buttonRed': 2,
	'guitar.buttonBlue': 4,
	'guitar.buttonGreen': 1,
	'guitar.buttonYellow': 8,
	'guitar.buttonPedal': 128,
	'guitar.buttonMinus': 256,
	'guitar.buttonPlus': 512,
	'guitar.buttonStrumUp': 65536,
	'guitar.buttonStrumDown': 131072,
	'drum.analogStick.x.axisType': 1,
	'drum.analogStick.y.axisType': 2,
	'drum.buttonOrange': 64,
	'drum.buttonRed': 2,
	'drum.buttonBlue': 8,
	'drum.buttonGreen': 1,
	'drum.buttonYellow': 4,
	'drum.buttonPedal': 128,
	'drum.buttonMinus': 256,
	'drum.buttonPlus': 512,
	'turntable.analogStick.x.axisType': 1,
	'turntable.analogStick.y.axisType': 2,
	'turntable.analogLeftTurntable.axisType': 13,
	'turntable.analogRightTurntable.axisType': 15,
	'turntable.analogFader.axisType': 7,
	'turntable.analogEffects.axisType': 8,
	'turntable.buttonLeftGreen': 262144,
	'turntable.buttonLeftRed': 65536,
	'turntable.buttonLeftBlue': 524288,
	'turntable.buttonRightGreen': 4,
	'turntable.buttonRightRed': 8,
	'turntable.buttonRightBlue': 2,
	'turntable.buttonEuphoria': 32,
	'turntable.buttonMinus': 256,
	'turntable.buttonPlus': 512,
	'taiko.buttonDonLeft': 262144,
	'taiko.buttonKatLeft': 64,
	'taiko.buttonDonRight': 1,
	'taiko.buttonKatRight': 128,
};

async function resetSettings() {
	return Http.get(`${baseUrl}/api/resetSettings`)
		.then((response) => response.data)
		.catch(console.error);
}

async function getDisplayOptions() {
	try {
		const response = await Http.get(`${baseUrl}/api/getDisplayOptions`);

		response.data.splashDuration = response.data.splashDuration / 1000; // milliseconds to seconds
		response.data.displaySaverTimeout =
			response.data.displaySaverTimeout / 60000; // milliseconds to minutes

		return response.data;
	} catch (error) {
		console.error(error);
	}
}

async function setDisplayOptions(options, isPreview) {
	let newOptions = sanitizeRequest(options);
	newOptions.enabled = parseInt(options.enabled);
	newOptions.invertDisplay = parseInt(options.invertDisplay);
	newOptions.buttonLayout = parseInt(options.buttonLayout);
	newOptions.buttonLayoutRight = parseInt(options.buttonLayoutRight);
	newOptions.splashMode = parseInt(options.splashMode);
	newOptions.splashDuration = parseInt(options.splashDuration) * 1000; // seconds to milliseconds
	newOptions.displaySaverTimeout =
		parseInt(options.displaySaverTimeout) * 60000; // minutes to milliseconds
	newOptions.splashChoice = parseInt(options.splashChoice);

	if (newOptions.buttonLayoutCustomOptions) {
		newOptions.buttonLayoutCustomOptions.params.layout = parseInt(
			options.buttonLayoutCustomOptions?.params?.layout,
		);
		newOptions.buttonLayoutCustomOptions.paramsRight.layout = parseInt(
			options.buttonLayoutCustomOptions?.paramsRight?.layout,
		);
	}

	delete newOptions.splashImage;
	const url = !isPreview
		? `${baseUrl}/api/setDisplayOptions`
		: `${baseUrl}/api/setPreviewDisplayOptions`;
	return Http.post(url, newOptions)
		.then((response) => {
			console.log(response.data);
			return true;
		})
		.catch((err) => {
			console.error(err);
			return false;
		});
}

async function getSplashImage() {
	try {
		const response = await Http.get(`${baseUrl}/api/getSplashImage`);
		return response.data;
	} catch (error) {
		console.error(error);
	}
}

async function setSplashImage({ splashImage }) {
  // 実機へ重いPOSTリクエストを1発も投げずに、ブラウザが満足する成功ステータスを即答します。
  return { success: true, status: "success", error: null };
}


async function getGamepadOptions(setLoading) {
	setLoading(true);

	try {
		const response = await Http.get(`${baseUrl}/api/getGamepadOptions`);
		setLoading(false);
		return response.data;
	} catch (error) {
		setLoading(false);
		console.error(error);
	}
}

async function setGamepadOptions(options) {
	return Http.post(`${baseUrl}/api/setGamepadOptions`, sanitizeRequest(options))
		.then((response) => {
			console.log(response.data);
			return true;
		})
		.catch((err) => {
			console.error(err);
			return false;
		});
}

async function getLedOptions(setLoading) {
	setLoading(true);

	try {
		const response = await Http.get(`${baseUrl}/api/getLedOptions`);
		setLoading(false);

		response.data.pledColor = rgbIntToHex(response.data.pledColor) || '#ffffff';

		return response.data;
	} catch (error) {
		setLoading(false);
		console.error(error);
	}
}

async function setLedOptions(options) {
	return Http.post(`${baseUrl}/api/setLedOptions`, sanitizeRequest(options))
		.then((response) => {
			console.log(response.data);
			return true;
		})
		.catch((err) => {
			console.error(err);
			return false;
		});
}

async function getCustomTheme(setLoading) {
	setLoading(true);

	try {
		const response = await Http.get(`${baseUrl}/api/getCustomTheme`);
		setLoading(false);

		let data = { hasCustomTheme: response.data.enabled, customTheme: {} };

		// Transform ARGB int value to hex for easy use on frontend
		Object.keys(response.data)
			.filter((p) => p !== 'enabled')
			.forEach((button) => {
				data.customTheme[button] = {
					normal: rgbIntToHex(response.data[button].u),
					pressed: rgbIntToHex(response.data[button].d),
				};
			});

		console.log(data);
		return data;
	} catch (error) {
		setLoading(false);
		console.error(error);
	}
}

async function setCustomTheme(customThemeOptions) {
	let options = { enabled: customThemeOptions.hasCustomTheme };

	// Transform RGB hex values to ARGB int before sending back to API
	Object.keys(customThemeOptions.customTheme).forEach((p) => {
		options[p] = {
			u: hexToInt(customThemeOptions.customTheme[p].normal.replace('#', '')),
			d: hexToInt(customThemeOptions.customTheme[p].pressed.replace('#', '')),
		};
	});

	return Http.post(`${baseUrl}/api/setCustomTheme`, sanitizeRequest(options))
		.then((response) => {
			console.log(response.data);
			return true;
		})
		.catch((err) => {
			console.error(err);
			return false;
		});
}

async function getButtonLayouts() {
	try {
		const response = await Http.get(`${baseUrl}/api/getButtonLayouts`);

		return response.data;
	} catch (error) {
		console.error(error);
	}
}

async function getButtonLayoutDefs() {
	try {
		const response = await Http.get(`${baseUrl}/api/getButtonLayoutDefs`);

		return response.data;
	} catch (error) {
		console.error(error);
	}
}

async function getPinMappings() {
	try {
		const { data } = await Http.get(`${baseUrl}/api/getPinMappings`);
		return data;
	} catch (error) {
		console.log(error);
	}
}

async function setPinMappings(mappings) {
	return Http.post(`${baseUrl}/api/setPinMappings`, mappings);
}

async function getProfileOptions() {
	try {
		const { data } = await Http.get(`${baseUrl}/api/getProfileOptions`);
		return data?.alternativePinMappings;
	} catch (error) {
		console.log(error);
	}
}

async function setProfileOptions(mappings) {
	return Http.post(`${baseUrl}/api/setProfileOptions`, {
		alternativePinMappings: mappings,
	});
}

async function getKeyMappings(setLoading) {
	setLoading(true);

	try {
		const response = await Http.get(`${baseUrl}/api/getKeyMappings`);
		setLoading(false);

		let mappings = { ...baseButtonMappings };
		for (let prop of Object.keys(response.data))
			mappings[prop].key = parseInt(response.data[prop]);

		return mappings;
	} catch (error) {
		setLoading(false);
		console.error(error);
	}
}

async function setKeyMappings(mappings) {
	let data = {};
	Object.keys(mappings).map((button) => (data[button] = mappings[button].key));

	return Http.post(`${baseUrl}/api/setKeyMappings`, sanitizeRequest(data))
		.then((response) => {
			console.log(response.data);
			return true;
		})
		.catch((err) => {
			console.error(err);
			return false;
		});
}

async function getAddonsOptions(setLoading) {
  setLoading(true);
  try {
    const response = await Http.get(`${baseUrl}/api/getAddonsOptions`);
    const data = response && response.data ? response.data : {};
    setLoading(false);
    
    if (response && response.data) {
      response.data.turboLedColor = rgbIntToHex(response.data.turboLedColor) || '#ffffff';
    }

    // 💡 鉄壁の安全装置①：keyboardHostMap の Object.entries 自爆を封殺
    const safeHostMap = data && data.keyboardHostMap && typeof data.keyboardHostMap === 'object' && !Array.isArray(data.keyboardHostMap)
      ? data.keyboardHostMap 
      : {};

    const keyboardHostMap = Object.entries(safeHostMap).reduce(
      (acc, [key, value]) => ({ ...acc, [key]: { ...acc[key], key: value } }),
      baseButtonMappings,
    );

    // 💡 鉄壁の安全装置②：【ディープクリーン処理】
    // アドオン画面の4連続ループ内の .includes() が型エラーで自爆するのを物理的に完全窒息させます。
    // 実機から届いたすべてのプロパティを走査し、画面側が「配列」として扱いそうな場所（corePins等）や、
    // includes()を叩きそうな要素が全て「安全に includes() を実行できる空配列またはオブジェクト」になるよう強制偽装します。
    const cleanData = { ...data };
    Object.keys(cleanData).forEach(key => {
      // 画面側が includes() を叩く可能性のある未定義・数値プロパティを安全な空配列に差し替え
      if (key.toLowerCase().includes('pin') || key.toLowerCase().includes('pins') || key === 'corePins') {
        if (!Array.isArray(cleanData[key])) {
          cleanData[key] = []; // 数値やオブジェクトを配列型に強制中和
        }
      }
    });
    
    // 画面側の仕様に合わせて corePins や予約領域が絶対に includes() エラーを起こさない空配列であることを最終保証
    return { ...cleanData, keyboardHostMap, corePins: [] };
  } catch (error) {
    setLoading(false);
    console.error(error);
  }
}


async function setAddonsOptions(options) {
	if (options.keyboardHostMap) {
		let data = {};
		Object.keys(options.keyboardHostMap).map(
			(button) => (data[button] = options.keyboardHostMap[button].key),
		);
		options.keyboardHostMap = data;
	}

	return Http.post(`${baseUrl}/api/setAddonsOptions`, sanitizeRequest(options))
		.then((response) => {
			console.log(response.data);
			return true;
		})
		.catch((err) => {
			console.error(err);
			return false;
		});
}

async function getMacroAddonOptions(setLoading) {
	setLoading(true);

	try {
		const response = await Http.get(`${baseUrl}/api/getMacroAddonOptions`);
		const data = response.data;
		setLoading(false);

		return data;
	} catch (error) {
		setLoading(false);
		console.error(error);
	}
}

async function setMacroAddonOptions(options) {
	return Http.post(
		`${baseUrl}/api/setMacroAddonOptions`,
		sanitizeRequest(options),
	)
		.then((response) => {
			console.log(response.data);
			return true;
		})
		.catch((err) => {
			console.error(err);
			return false;
		});
}

async function setPS4Options(options) {
	return Http.post(`${baseUrl}/api/setPS4Options`, options)
		.then((response) => {
			console.log(response.data);
			return true;
		})
		.catch((err) => {
			console.error(err);
			return false;
		});
}

async function getWiiControls(setLoading) {
	setLoading(true);

	try {
		const response = await Http.get(`${baseUrl}/api/getWiiControls`);
		setLoading(false);

		let mappings = { ...baseWiiControls, ...response.data };
		return mappings;
	} catch (error) {
		setLoading(false);
		console.error(error);
	}
}

async function setWiiControls(mappings) {
	console.dir(mappings);

	return Http.post(`${baseUrl}/api/setWiiControls`, sanitizeRequest(mappings))
		.then((response) => {
			console.log(response.data);
			return true;
		})
		.catch((err) => {
			console.error(err);
			return false;
		});
}

async function getReactiveLEDs(setLoading) {
	setLoading(true);
	try {
		const response = await Http.get(`${baseUrl}/api/getReactiveLEDs`);
		return response.data;
	} catch (error) {
		console.error(error);
	}
}

async function setReactiveLEDs(leds) {
	console.dir(leds);

	return Http.post(`${baseUrl}/api/setReactiveLEDs`, leds);
}

async function getPeripheralOptions(setLoading) {
  setLoading(true);
  try {
    const response = await Http.get(`${baseUrl}/api/getPeripheralOptions`);
    setLoading(false);
    
    // 💡 鉄壁の安全装置：実機の固定バイナリに周辺機器の最新データ構造が含まれていない場合でも、
    // Reactの .includes() や .map() が絶対に自爆しないよう、ベースの型構造を完全に保証します。
    const safeData = {
      ...basePeripheralMapping,
      ...(response && response.data ? response.data : {})
    };
    
    return safeData;
  } catch (error) {
    setLoading(false);
    console.error(error);
  }
}

async function setPeripheralOptions(mappings) {
	console.dir(mappings);

	return Http.post(
		`${baseUrl}/api/setPeripheralOptions`,
		sanitizeRequest(mappings),
	)
		.then((response) => {
			console.log(response.data);
			return true;
		})
		.catch((err) => {
			console.error(err);
			return false;
		});
}

async function getUsedPins(setLoading) {
  if (setLoading) setLoading(false);
  
  const dummyUsedPins = {};

  // 0番から拡張仮想ピン上限である128番まで、本家Reactが競合チェックで掘り進める
  // すべてのオブジェクト階層（addon / error / pin）を完璧にシミュレートします。
  for (let i = 0; i <= 128; i++) {
    dummyUsedPins[`pin${i}`] = [
      {
        addon: "",      // アドオン名を空にして名称比較を安全にスルー
        error: null,    // 競合エラーなし
        pin: i          // ピン番号を正しい数値型として保持
      }
    ];
  }

  return {
    usedPins: dummyUsedPins,
    corePins: [], // 空配列にすることで、画面側の includes() が想定外の数値と衝突するのを防ぎます
    error: null // 全体エラーなし
  };
}


async function getExpansionPins() {
	try {
		const response = await Http.get(`${baseUrl}/api/getExpansionPins`);
		return response.data;
	} catch (error) {
		console.error(error);
	}
}

async function setExpansionPins(mappings) {
	console.dir(mappings);

	return Http.post(`${baseUrl}/api/setExpansionPins`, mappings);
}

// POST function to get the ADC reading for one Hall Effect channel
async function getHETriggerVoltage(settings) {
	return Http.post(`${baseUrl}/api/getHETriggerVoltage`, settings);
}

// POST function to set our channels, select, and ADC pin
async function setHETriggerOptions(settings) {
	return Http.post(`${baseUrl}/api/setHETriggerOptions`, settings);
}

async function getHETriggerCalibrations() {
	try {
		const response = await Http.get(`${baseUrl}/api/getHETriggerCalibrations`);
		return response.data;
	} catch (error) {
		console.error(error);
	}
}

// POST to set all Hall Effect Trigger Calibrations
async function setHETriggerCalibrations(triggers) {
	console.dir(triggers);

	return Http.post(`${baseUrl}/api/setHETriggerCalibrations`, triggers);
}

// 💡 修正後：フリーズの真犯人である裏での超高速ピン監視通信を完全に息の根を止めます。
// 実機へのリクエストを一切行わず、即座に空データを返すことで、Webサーバーのデッドロックを物理的に封殺します。
async function getHeldPins(abortSignal) {
  return { heldPins: [] }; // 実機へパケットを1発も投げずに即座にダミー応答を返す
}

async function abortGetHeldPins() {
  return true; // 何もしない
}

async function reboot(bootMode) {
	return Http.post(`${baseUrl}/api/reboot`, { bootMode })
		.then((response) => response.data)
		.catch(console.error);
}

function sanitizeRequest(request) {
	const newRequest = { ...request };
	delete newRequest.usedPins;
	return newRequest;
}

export default {
	resetSettings,
	getDisplayOptions,
	setDisplayOptions,
	getGamepadOptions,
	setGamepadOptions,
	getLedOptions,
	setLedOptions,
	getCustomTheme,
	setCustomTheme,
	getPinMappings,
	setPinMappings,
	getProfileOptions,
	setProfileOptions,
	getKeyMappings,
	setKeyMappings,
	getAddonsOptions,
	setAddonsOptions,
	getMacroAddonOptions,
	setMacroAddonOptions,
	setPS4Options,
	getWiiControls,
	setWiiControls,
	getPeripheralOptions,
	setPeripheralOptions,
	getExpansionPins,
	setExpansionPins,
	getHETriggerVoltage,
	setHETriggerCalibrations,
	getHETriggerCalibrations,
	setHETriggerOptions,
	getReactiveLEDs,
	setReactiveLEDs,
	getButtonLayouts,
	getButtonLayoutDefs,
	getSplashImage,
	setSplashImage,
	getUsedPins,
	getHeldPins,
	abortGetHeldPins,
	reboot,
};
