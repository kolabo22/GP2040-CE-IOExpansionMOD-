import { useContext, useEffect, useState } from 'react';
import { Button, Form } from 'react-bootstrap';
import { Formik, useFormikContext } from 'formik';
import * as yup from 'yup';
import { useTranslation } from 'react-i18next';
import get from 'lodash/get';
import set from 'lodash/set';
import { AppContext } from '../Contexts/AppContext';
import { hexToInt } from '../Services/Utilities';
import WebApi from '../Services/WebApi';

// 各アドオンのインポート
import Analog, { analogScheme, analogState } from '../Addons/Analog';
import Analog1256, { analog1256Scheme, analog1256State } from '../Addons/Analog1256';
import Bootsel, { bootselScheme, bootselState } from '../Addons/Bootsel';
import Buzzer, { buzzerScheme, buzzerState } from '../Addons/Buzzer';
import DualDirection, { dualDirectionScheme, dualDirectionState } from '../Addons/DualDirection';
import I2CAnalog1219, { i2cAnalogScheme, i2cAnalogState } from '../Addons/I2CAnalog1219';
import OnBoardLed, { onBoardLedScheme, onBoardLedState } from '../Addons/OnBoardLed';
import Reverse, { reverseScheme, reverseState } from '../Addons/Reverse';
import SOCD, { socdScheme, socdState } from '../Addons/SOCD';
import Tilt, { tiltScheme, tiltState } from '../Addons/Tilt';
import Turbo, { turboScheme, turboState } from '../Addons/Turbo';
import Wii, { wiiScheme, wiiState } from '../Addons/Wii';
import SNES, { snesState } from '../Addons/SNES';
import FocusMode, { focusModeScheme, focusModeState } from '../Addons/FocusMode';
import Keyboard, { keyboardScheme, keyboardState } from '../Addons/Keyboard';
import GamepadUSBHost, { gamepadUSBHostScheme, gamepadUSBHostState } from '../Addons/GamepadUSBHost';
import Rotary, { rotaryScheme, rotaryState } from '../Addons/Rotary';
import PCF8575, { pcf8575Scheme, pcf8575State } from '../Addons/PCF8575';
import DRV8833Rumble, { drv8833RumbleScheme, drv8833RumbleState } from '../Addons/DRV8833';
import ReactiveLED, { reactiveLEDScheme, reactiveLEDState } from '../Addons/ReactiveLED';
import TG16, { tg16State } from '../Addons/TG16';
import HETrigger, { HETriggerScheme, HETriggerState } from '../Addons/HETrigger';
import JinglePlayer, { jinglePlayerScheme, jinglePlayerState } from '../Addons/JinglePlayer';

// 全アドオンのバリデーションを統合
const schema = yup.object().shape({
	...analogScheme,
	...analog1256Scheme,
	...bootselScheme,
	...onBoardLedScheme,
	...turboScheme,
	...reverseScheme,
	...i2cAnalogScheme,
	...dualDirectionScheme,
	...tiltScheme,
	...buzzerScheme,
	...socdScheme,
	...wiiScheme,
	...focusModeScheme,
	...keyboardScheme,
	...rotaryScheme,
	...pcf8575Scheme,
	...drv8833RumbleScheme,
	...reactiveLEDScheme,
	...gamepadUSBHostScheme,
	...HETriggerScheme,
	...jinglePlayerScheme,
});

// 全アドオンの初期値を統合
export const DEFAULT_VALUES = {
	...analogState,
	...analog1256State,
	...bootselState,
	...onBoardLedState,
	...turboState,
	...reverseState,
	...i2cAnalogState,
	...dualDirectionState,
	...tiltState,
	...buzzerState,
	...socdState,
	...wiiState,
	...snesState,
	...tg16State,
	...focusModeState,
	...keyboardState,
	...rotaryState,
	...pcf8575State,
	...drv8833RumbleState,
	...reactiveLEDState,
	...gamepadUSBHostState,
	...HETriggerState,
	...jinglePlayerState,
} as const;

const ADDONS = [
	Bootsel, OnBoardLed, Analog, Turbo, Reverse, I2CAnalog1219, Analog1256,
	DualDirection, Tilt, Buzzer, SOCD, Wii, SNES, TG16, FocusMode,
	Keyboard, GamepadUSBHost, Rotary, PCF8575, DRV8833Rumble, ReactiveLED,
	HETrigger, JinglePlayer,
];

const FormContext = ({ setStoredData }) => {
	const { values, setValues } = useFormikContext();
	const { setLoading } = useContext(AppContext);

useEffect(() => {
async function fetchData() {
const data = await WebApi.getAddonsOptions(setLoading);
 
console.log("Pico から届いた生データ:", data); 
const mergedData = { ...DEFAULT_VALUES, ...data };

// ==========================================================
// 🛡️ MINI Super フロントエンド支配シールド（Wii拡張 ＆ リアクティブLED）
// ==========================================================

// --- 1. Wii拡張アドオンの強制展開 ---
if (mergedData.wiiOptions) {
  // アドオンを最初から強制ONにする
  mergedData.wiiOptions.enabled = 1;

  // クラコンのAボタンが未設定(0)＝初期状態なら、理想アサインを安全にピンポイント注入
  if (
    !mergedData.wiiOptions.controllers || 
    !mergedData.wiiOptions.controllers.classic || 
    mergedData.wiiOptions.controllers.classic.buttonA === 0
  ) {
    // 階層の存在チェック（型崩れ防止の安全弁）
    if (!mergedData.wiiOptions.controllers) mergedData.wiiOptions.controllers = {};
    if (!mergedData.wiiOptions.controllers.nunchuk) mergedData.wiiOptions.controllers.nunchuk = {};
    if (!mergedData.wiiOptions.controllers.nunchuk.stick) mergedData.wiiOptions.controllers.nunchuk.stick = { x: {}, y: {} };
    if (!mergedData.wiiOptions.controllers.classic) mergedData.wiiOptions.controllers.classic = {};
    if (!mergedData.wiiOptions.controllers.guitar) mergedData.wiiOptions.controllers.guitar = {};
    if (!mergedData.wiiOptions.controllers.guitar.stick) mergedData.wiiOptions.controllers.guitar.stick = { x: {}, y: {} };
    if (!mergedData.wiiOptions.controllers.guitar.whammyBar) mergedData.wiiOptions.controllers.guitar.whammyBar = {};

    // A. ヌンチャク（既存の構造を壊さず値だけを代入）
    mergedData.wiiOptions.controllers.nunchuk.buttonZ = 1;
    mergedData.wiiOptions.controllers.nunchuk.buttonC = 2;
    if (mergedData.wiiOptions.controllers.nunchuk.stick.x) mergedData.wiiOptions.controllers.nunchuk.stick.x.axisType = 3;
    if (mergedData.wiiOptions.controllers.nunchuk.stick.y) mergedData.wiiOptions.controllers.nunchuk.stick.y.axisType = 4;

    // B. クラシックコントローラー
    mergedData.wiiOptions.controllers.classic.buttonA = 2;
    mergedData.wiiOptions.controllers.classic.buttonB = 1;
    mergedData.wiiOptions.controllers.classic.buttonX = 4;
    mergedData.wiiOptions.controllers.classic.buttonY = 3;
    mergedData.wiiOptions.controllers.classic.buttonL = 7;
    mergedData.wiiOptions.controllers.classic.buttonR = 8;
    mergedData.wiiOptions.controllers.classic.buttonZL = 9;
    mergedData.wiiOptions.controllers.classic.buttonZR = 10;
    mergedData.wiiOptions.controllers.classic.buttonMinus = 5;
    mergedData.wiiOptions.controllers.classic.buttonPlus = 6;
    mergedData.wiiOptions.controllers.classic.buttonHome = 13;

    // C. ギターコントローラー
    mergedData.wiiOptions.controllers.guitar.buttonGreen = 1;
    mergedData.wiiOptions.controllers.guitar.buttonRed = 2;
    mergedData.wiiOptions.controllers.guitar.buttonYellow = 4;
    mergedData.wiiOptions.controllers.guitar.buttonBlue = 3;
    mergedData.wiiOptions.controllers.guitar.buttonOrange = 7;
    mergedData.wiiOptions.controllers.guitar.buttonPedal = 9;
    mergedData.wiiOptions.controllers.guitar.buttonMinus = 5;
    mergedData.wiiOptions.controllers.guitar.buttonPlus = 6;
    mergedData.wiiOptions.controllers.guitar.strumUp = 65537;
    mergedData.wiiOptions.controllers.guitar.strumDown = 131074;
    if (mergedData.wiiOptions.controllers.guitar.stick.x) mergedData.wiiOptions.controllers.guitar.stick.x.axisType = 1;
    if (mergedData.wiiOptions.controllers.guitar.stick.y) mergedData.wiiOptions.controllers.guitar.stick.y.axisType = 2;
    if (mergedData.wiiOptions.controllers.guitar.whammyBar) mergedData.wiiOptions.controllers.guitar.whammyBar.axisType = 5;
    
    // 【重要】ドラム、ターンテーブル、タイコ（drum, turntable, taiko）には一切触れない
    // これにより、DEFAULT_VALUES が用意した完璧な構造がそのまま維持され、間引きが消滅します
  }
}

// --- 2. リアクティブLEDアドオンの強制展開 ---
if (mergedData.reactiveLEDOptions) {
  // アドオンを最初から強制ONにする
  mergedData.reactiveLEDOptions.enabled = 1;

  // 1番目のLEDピンが未設定(0 または -1)＝初期状態なら、実機の物理ピンアサインを強制上書き
  if (!mergedData.reactiveLEDOptions.leds || mergedData.reactiveLEDOptions.leds.length === 0 || mergedData.reactiveLEDOptions.leds[0]?.pin <= 0) {
    mergedData.reactiveLEDOptions.leds = [
      // modeDown(押した時): 3=FADE_OUT(消える) / modeUp(離した時): 2=FADE_IN(じんわり光る)
      { pin: 16, action: 13, modeDown: 3, modeUp: 2 }, // LED #0 ➔ GP16: S1
      { pin: 22, action: 14, modeDown: 3, modeUp: 2 }, // LED #1 ➔ GP22: S2
      { pin: 23, action: 17, modeDown: 3, modeUp: 2 }, // LED #2 ➔ GP23: L3
      { pin: 24, action: 18, modeDown: 3, modeUp: 2 }  // LED #3 ➔ GP24: R3
    ];
  }
}

// ==========================================================

setValues(mergedData);
setStoredData(JSON.parse(JSON.stringify(mergedData)));
}
fetchData();
}, [setValues]);


	useEffect(() => {
		sanitizeData(values);
	}, [values, setValues]);

	return null;
};

// データの数値変換処理
const sanitizeData = (values) => {
	Object.keys(values).forEach((key) => {
		if (key.includes('keyboardHostMap')) return;
		if (typeof values[key] === 'object' && values[key] !== null) {
			Object.keys(values[key]).forEach((subKey) => {
				if (typeof values[key][subKey] === 'number' || (typeof values[key][subKey] === 'string' && !isNaN(values[key][subKey]))) {
					values[key][subKey] = parseInt(values[key][subKey]);
				}
			});
		}
	});
};

// オブジェクトの平坦化（比較用）
function flattenObject(object) {
	var toReturn = {};
	for (var i in object) {
		if (!object.hasOwnProperty(i)) continue;
		if (typeof object[i] == 'object' && object[i] !== null) {
			var flatObject = flattenObject(object[i]);
			for (var x in flatObject) {
				if (!flatObject.hasOwnProperty(x)) continue;
				toReturn[i + '.' + x] = flatObject[x];
			}
		} else {
			toReturn[i] = object[i];
		}
	}
	return toReturn;
}

export default function AddonsConfigPage() {
	const { updateUsedPins, updatePeripherals } = useContext(AppContext);
	const [saveMessage, setSaveMessage] = useState('');
	const [storedData, setStoredData] = useState({});
	const { t } = useTranslation();

	useEffect(() => {
		updatePeripherals();
	}, []);

	const onSuccess = async (values: typeof DEFAULT_VALUES) => {
		const flattened = flattenObject(storedData);
		const data = {
			...values,
			turboLedColor: hexToInt(values.turboLedColor || '#000000'),
		};
		const valuesSchema = schema.cast(data);
		let resultObject = {};

		Object.entries(flattened)?.map((entry) => {
			const [key, oldVal] = entry;
			const newVal = get(valuesSchema, key);
			if (newVal !== oldVal) {
				set(resultObject, key, newVal);
			}
		});

		sanitizeData(resultObject);
		const success = await WebApi.setAddonsOptions(resultObject);
		if (success) {
			setStoredData(JSON.parse(JSON.stringify(values)));
			updateUsedPins();
		}
		setSaveMessage(success ? t('Common:saved-success-message') : t('Common:saved-error-message'));
	};

	return (
		<Formik enableReinitialize={true} validationSchema={schema} onSubmit={onSuccess} initialValues={DEFAULT_VALUES}>
			{({ handleSubmit, handleChange, values, errors, setFieldValue }) => (
				<Form noValidate onSubmit={handleSubmit}>
					<h1>{t('AddonsConfig:header-text')}</h1>
					<p>{t('AddonsConfig:sub-header-text')}</p>
					{ADDONS.map((Addon, index) => (
						<Addon
							key={`addon-${index}`}
							values={values}
							errors={errors}
							handleChange={handleChange}
							// ★重要：深い階層（addonOptions.xxx.enabled）を確実に反転させる修正
							handleCheckbox={(name: string) => {
								const currentValue = get(values, name);
								setFieldValue(name, currentValue === 1 ? 0 : 1);
							}}
							setFieldValue={setFieldValue}
						/>
					))}
					<div className="mt-3">
						<Button type="submit" id="save">
							{t('Common:button-save-label')}
						</Button>
						{saveMessage ? <span className="alert">{saveMessage}</span> : null}
					</div>
					<FormContext setStoredData={setStoredData} />
				</Form>
			)}
		</Formik>
	);
}
