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
			if (data && data.buttonPressColorCooldownTimeInMs && typeof data.buttonPressColorCooldownTimeInMs === 'object') {
				data.buttonPressColorCooldownTimeInMs = 0;
			}

			// 1. まず公式の初期値マージを終わらせる
			let mergedData = { ...DEFAULT_VALUES, ...data };

			// 🔥【主客逆転】組み立てられた器の上に、バックアップのフラットデータをそのまま100%最優先で被せる
			const savedJson = localStorage.getItem('restore_raw_json');
			if (savedJson) {
				try {
					const fileData = JSON.parse(savedJson);
					if (fileData) {
						mergedData = JSON.parse(JSON.stringify({ ...mergedData, ...fileData }));
						console.log('✅ [Addons Sync] Force Deep Updated.');
					}
				} catch (e) {}
			}

			setValues(mergedData);
			setStoredData(JSON.parse(JSON.stringify(mergedData)));
		}


		fetchData();
	}, [setValues]);

	useEffect(() => {
		sanitizeData(values);
	}, [values, setValues]);

	// 🔥【後出しジャンケンシステム】
	// 公式の実機ロードが完全に終わったその後、バックアップデータを上から安全に覆い被せる！
	useEffect(() => {
		const savedJson = localStorage.getItem('restore_raw_json');
		if (savedJson) {
			try {
				const fileData = JSON.parse(savedJson);
				if (fileData) {
					// 1. 公式が実機からロードし終わった現在のFormikの値（values）をベースにする
					let currentValues = { ...values };

					// 2. その上にバックアップJSON直下のアドオン項目、および子オブジェクト群を安全に結合
					if (fileData.wiiOptions && currentValues.wiiOptions) currentValues.wiiOptions = { ...currentValues.wiiOptions, ...fileData.wiiOptions };
					if (fileData.keyboardMapping && currentValues.keyboardMapping) currentValues.keyboardMapping = { ...currentValues.keyboardMapping, ...fileData.keyboardMapping };
					if (fileData.playerNumberOptions && currentValues.playerNumberOptions) currentValues.playerNumberOptions = { ...currentValues.playerNumberOptions, ...fileData.playerNumberOptions };
					
					// ルート直下のアドオン有効化フラグ群なども丸ごとディープマージ
					currentValues = JSON.parse(JSON.stringify({ ...currentValues, ...fileData }));

					// 3. 鉄壁のセーフティ（オブジェクト型バグの消滅）
					if (currentValues && currentValues.buttonPressColorCooldownTimeInMs && typeof currentValues.buttonPressColorCooldownTimeInMs === 'object') {
						currentValues.buttonPressColorCooldownTimeInMs = 0;
					}

					// 4. 新しく生まれ変わった本物の完全体データをFormikに「後出し」で強制認知させる！
					setValues(currentValues);
					console.log('🔮 [Addons Late Sync] Backup successfully overlaid after official load.');
				}
			} catch (e) {
				console.error('Addons late sync error', e);
			}
		}
	}, [setValues]); // 💡 画面が開いて公式のロードが落ち着いたタイミングで1回だけ確実に発動

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
