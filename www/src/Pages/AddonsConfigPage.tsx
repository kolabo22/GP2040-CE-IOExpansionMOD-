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
		<Formik 
			enableReinitialize={true} 
			validationSchema={schema} 
			onSubmit={onSuccess} 
			initialValues={{
				...DEFAULT_VALUES,
				addonOptions: {
					...DEFAULT_VALUES?.addonOptions,
					// 1. Wii拡張アドオンの初期値を理想型に強制適用
					wiiOptions: {
						enabled: 1,
						controllers: {
							nunchuk: {
								buttonZ: 1, buttonC: 2,
								stick: { x: { axisType: 3 }, y: { axisType: 4 } }
							},
							classic: {
								buttonA: 2, buttonB: 1, buttonX: 4, buttonY: 3,
								buttonL: 7, buttonR: 8, buttonZL: 9, buttonZR: 10,
								buttonMinus: 5, buttonPlus: 6, buttonHome: 13
							},
							guitar: {
								buttonGreen: 1, buttonRed: 2, buttonYellow: 4, buttonBlue: 3, buttonOrange: 7, buttonPedal: 9, buttonMinus: 5, buttonPlus: 6,
								strumUp: 65537, strumDown: 131074,
								stick: { x: { axisType: 1 }, y: { axisType: 2 } },
								whammyBar: { axisType: 5 }
							},
							// ドラム、ターンテーブル、タイコもFormikが要求する型を完全維持して間引きを防止
							drum: {
								buttonRed: 0, buttonBlue: 0, buttonGreen: 0, buttonYellow: 0, buttonOrange: 0, buttonBass: 0, buttonMinus: 0, buttonPlus: 0,
								stick: { x: { axisType: 0 }, y: { axisType: 0 } }
							},
							turntable: {
								buttonLeftGreen: 0, buttonLeftRed: 0, buttonLeftBlue: 0, buttonRightGreen: 0, buttonRightRed: 0, buttonRightBlue: 0, buttonCrossfader: 0,
								stick: { x: { axisType: 0 }, y: { axisType: 0 } }
							},
							taiko: {
								buttonDonLeft: 0, buttonKatLeft: 0, buttonDonRight: 0, buttonKatRight: 0
							}
						}
					},
					// 2. リアクティブLEDアドオンの初期値を物理ピン・フェード仕様に強制適用
					reactiveLEDOptions: {
						enabled: 1,
						leds: [
							// modeDown(押した時): 3=FADE_OUT(消える) / modeUp(離した時): 2=FADE_IN(じんわり光る)
							{ pin: 16, action: 13, modeDown: 3, modeUp: 2 }, // LED #0 ➔ GP16: S1
							{ pin: 22, action: 14, modeDown: 3, modeUp: 2 }, // LED #1 ➔ GP22: S2
							{ pin: 23, action: 17, modeDown: 3, modeUp: 2 }, // LED #2 ➔ GP23: L3
							{ pin: 24, action: 18, modeDown: 3, modeUp: 2 }  // LED #3 ➔ GP24: R3
						]
					}
				}
			}}
		>
			{({ handleSubmit, handleChange, values, errors, setFieldValue }) => (
				<Form noValidate onSubmit={handleSubmit}>
