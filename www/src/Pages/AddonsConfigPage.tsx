import React, { useContext, useEffect, useState } from 'react';
import { Button, Form } from 'react-bootstrap';
import { useTranslation } from 'react-i18next';
import { Formik } from 'formik';
import * as yup from 'yup';
import { AppContext } from '../Contexts/AppContext';
import WebApi from '../Services/WebApi';

// ファイル名の大文字小文字をリポジトリの実態に合わせて修正
import Analog from '../Addons/Analog';
import BoardLed from '../Addons/BoardLed'; // LEDではなくLed
import BootselButton from '../Addons/BootselButton';
import BuzzerSpeaker from '../Addons/BuzzerSpeaker';
import DualInput from '../Addons/DualInput';
import ExtraButtonConfiguration from '../Addons/ExtraButtonConfiguration';
import I2CAnalog1219 from '../Addons/I2CAnalog1219';
import I2CDisplay from '../Addons/I2CDisplay';
import KeyboardHost from '../Addons/KeyboardHost';
import PlayerNumber from '../Addons/PlayerNumber';
import PS4Mode from '../Addons/PS4Mode';
import ReverseInput from '../Addons/ReverseInput';
import SliderInput from '../Addons/SliderInput';
import SNESInput from '../Addons/SNESInput';
import SOCDSelectionSlider from '../Addons/SOCDSelectionSlider';
import Tilt from '../Addons/Tilt';
import TouchpadDataConfiguration from '../Addons/TouchpadDataConfiguration';
import TurboInput from '../Addons/TurboInput';
import WiiExtension from '../Addons/WiiExtension';
import HETrigger from '../Addons/HETrigger';
import JinglePlayer, { jinglePlayerScheme, jinglePlayerState } from '../Addons/JinglePlayer';

const DEFAULT_VALUES = {
	analogOptions: { enabled: 0 },
	boardLedOptions: { enabled: 0, dataPin: -1, ledFormat: 0, ledLayout: 0, ledsPerButton: 0, brightnessMaximum: 255, brightnessSteps: 5 },
	bootselButtonOptions: { enabled: 0, buttonMask: 0 },
	buzzerSpeakerOptions: { enabled: 0, buzzerPin: -1, buzzerVolume: 100 },
	dualInputOptions: { enabled: 0 },
	extraButtonOptions: { enabled: 0 },
	i2cAnalog1219Options: { enabled: 0, i2cAddress: 0x40, i2cBlock: 0 },
	displayOptions: { enabled: 0, i2cAddress: 0x3c, i2cBlock: 0, i2cSpeed: 400000, buttonLayout: 0, buttonLayoutRight: 0, splashMode: 0, splashDuration: 0, displaySaverTimeout: 0, invertDisplay: 0, flipDisplay: 0 },
	keyboardHostOptions: { enabled: 0, pinDplus: -1, pinDminus: -1, pin5V: -1, mouseMovement: 0 },
	playerNumberOptions: { enabled: 0, number: 1 },
	ps4ModeOptions: { enabled: 0 },
	reverseOptions: { enabled: 0, buttonPin: -1, ledPin: -1, action: 0 },
	sliderOptions: { enabled: 0, pinLS: -1, pinRS: -1 },
	snesInputOptions: { enabled: 0, clockPin: -1, latchPin: -1, dataPin: -1 },
	socdSliderOptions: { enabled: 0 },
	tiltOptions: { enabled: 0, pin1: -1, pin2: -1, pin3: -1, pin4: -1 },
	touchpadDataOptions: { enabled: 0 },
	turboOptions: { enabled: 0, buttonPin: -1, ledPin: -1, shotCount: 20, shmupMode: 0, shmupMixMode: 0, shmupAlwaysOn1: 0, shmupAlwaysOn2: 0, shmupAlwaysOn3: 0, shmupAlwaysOn4: 0, shmupBtn1Pin: -1, shmupBtn2Pin: -1, shmupBtn3Pin: -1, shmupBtn4Pin: -1, shmupBtnMask1: 0, shmupBtnMask2: 0, shmupBtnMask3: 0, shmupBtnMask4: 0 },
	wiiextOptions: { enabled: 0, i2cBlock: 0 },
	jinglePlayerOptions: { enabled: 0, volume: 15 },
};

const schema = yup.object().shape({
	analogOptions: yup.object().shape({ enabled: yup.number().label('Enabled') }),
	boardLedOptions: yup.object().shape({ enabled: yup.number().label('Enabled'), dataPin: yup.number().label('Data Pin'), ledFormat: yup.number().label('LED Format'), ledLayout: yup.number().label('LED Layout'), ledsPerButton: yup.number().label('LEDs Per Button'), brightnessMaximum: yup.number().label('Max Brightness'), brightnessSteps: yup.number().label('Brightness Steps') }),
	bootselButtonOptions: yup.object().shape({ enabled: yup.number().label('Enabled'), buttonMask: yup.number().label('Button Mask') }),
	buzzerSpeakerOptions: yup.object().shape({ enabled: yup.number().label('Enabled'), buzzerPin: yup.number().label('Buzzer Pin'), buzzerVolume: yup.number().label('Buzzer Volume') }),
	dualInputOptions: yup.object().shape({ enabled: yup.number().label('Enabled') }),
	extraButtonOptions: yup.object().shape({ enabled: yup.number().label('Enabled') }),
	i2cAnalog1219Options: yup.object().shape({ enabled: yup.number().label('Enabled'), i2cAddress: yup.number().label('I2C Address'), i2cBlock: yup.number().label('I2C Block') }),
	displayOptions: yup.object().shape({ enabled: yup.number().label('Enabled'), i2cAddress: yup.number().label('I2C Address'), i2cBlock: yup.number().label('I2C Block'), i2cSpeed: yup.number().label('I2C Speed'), buttonLayout: yup.number().label('Button Layout'), buttonLayoutRight: yup.number().label('Button Layout Right'), splashMode: yup.number().label('Splash Mode'), splashDuration: yup.number().label('Splash Duration'), displaySaverTimeout: yup.number().label('Display Saver Timeout'), invertDisplay: yup.number().label('Invert Display'), flipDisplay: yup.number().label('Flip Display') }),
	keyboardHostOptions: yup.object().shape({ enabled: yup.number().label('Enabled'), pinDplus: yup.number().label('Pin D+'), pinDminus: yup.number().label('Pin D-'), pin5V: yup.number().label('Pin 5V'), mouseMovement: yup.number().label('Mouse Movement') }),
	playerNumberOptions: yup.object().shape({ enabled: yup.number().label('Enabled'), number: yup.number().label('Number') }),
	ps4ModeOptions: yup.object().shape({ enabled: yup.number().label('Enabled') }),
	reverseOptions: yup.object().shape({ enabled: yup.number().label('Enabled'), buttonPin: yup.number().label('Button Pin'), ledPin: yup.number().label('LED Pin'), action: yup.number().label('Action') }),
	sliderOptions: yup.object().shape({ enabled: yup.number().label('Enabled'), pinLS: yup.number().label('LS Pin'), pinRS: yup.number().label('RS Pin') }),
	snesInputOptions: yup.object().shape({ enabled: yup.number().label('Enabled'), clockPin: yup.number().label('Clock Pin'), latchPin: yup.number().label('Latch Pin'), dataPin: yup.number().label('Data Pin') }),
	socdSliderOptions: yup.object().shape({ enabled: yup.number().label('Enabled') }),
	tiltOptions: yup.object().shape({ enabled: yup.number().label('Enabled'), pin1: yup.number().label('Pin 1'), pin2: yup.number().label('Pin 2'), pin3: yup.number().label('Pin 3'), pin4: yup.number().label('Pin 4') }),
	touchpadDataOptions: yup.object().shape({ enabled: yup.number().label('Enabled') }),
	turboOptions: yup.object().shape({ enabled: yup.number().label('Enabled'), buttonPin: yup.number().label('Button Pin'), ledPin: yup.number().label('LED Pin'), shotCount: yup.number().label('Shot Count'), shmupMode: yup.number().label('Shmup Mode'), shmupMixMode: yup.number().label('Shmup Mix Mode'), shmupAlwaysOn1: yup.number().label('Shmup Always On 1'), shmupAlwaysOn2: yup.number().label('Shmup Always On 2'), shmupAlwaysOn3: yup.number().label('Shmup Always On 3'), shmupAlwaysOn4: yup.number().label('Shmup Always On 4'), shmupBtn1Pin: yup.number().label('Shmup Button 1 Pin'), shmupBtn2Pin: yup.number().label('Shmup Button 2 Pin'), shmupBtn3Pin: yup.number().label('Shmup Button 3 Pin'), shmupBtn4Pin: yup.number().label('Shmup Button 4 Pin'), shmupBtnMask1: yup.number().label('Shmup Button 1 Mask'), shmupBtnMask2: yup.number().label('Shmup Button 2 Mask'), shmupBtnMask3: yup.number().label('Shmup Button 3 Mask'), shmupBtnMask4: yup.number().label('Shmup Button 4 Mask') }),
	wiiextOptions: yup.object().shape({ enabled: yup.number().label('Enabled'), i2cBlock: yup.number().label('I2C Block') }),
	jinglePlayerOptions: yup.object().shape({ enabled: yup.number().label('Enabled'), volume: yup.number().label('Volume') }),
});

const sanitizeData = (values) => {
	const sanitized = { ...values };
	Object.keys(sanitized).forEach((key) => {
		if (key.includes('Options') && typeof sanitized[key] === 'object') {
			Object.keys(sanitized[key]).forEach((subKey) => {
				if (typeof sanitized[key][subKey] === 'string' && !isNaN(sanitized[key][subKey])) {
					sanitized[key][subKey] = parseInt(sanitized[key][subKey], 10);
				}
			});
		}
	});
	return sanitized;
};

export default function AddonsConfigPage() {
	const { setLoading } = useContext(AppContext);
	const [saveMessage, setSaveMessage] = useState(null);
	const [initialValues, setInitialValues] = useState(DEFAULT_VALUES);
	const { t } = useTranslation();

	useEffect(() => {
		async function fetchData() {
			const options = await WebApi.getAddonOptions();
			setInitialValues({ ...DEFAULT_VALUES, ...options });
			setLoading(false);
		}
		fetchData();
	}, [setLoading]);

	const onSuccess = async (values) => {
		setSaveMessage(null);
		const sanitizedValues = sanitizeData(values);
		const success = await WebApi.setAddonOptions(sanitizedValues);
		setSaveMessage(success ? t('Common:saved-success-message') : t('Common:saved-error-message'));
	};

	return (
		<Formik enableReinitialize={true} initialValues={initialValues} validationSchema={schema} onSubmit={onSuccess}>
			{({ values, handleChange, handleCheckbox, setFieldValue, handleSubmit }) => (
				<Form noValidate onSubmit={handleSubmit}>
					<Analog values={values} handleChange={handleChange} />
					<TurboInput values={values} handleChange={handleChange} handleCheckbox={handleCheckbox} />
					<SliderInput values={values} handleChange={handleChange} />
					<SOCDSelectionSlider values={values} handleChange={handleChange} />
					<ReverseInput values={values} handleChange={handleChange} />
					<PS4Mode values={values} handleChange={handleChange} handleCheckbox={handleCheckbox} />
					<I2CDisplay values={values} handleChange={handleChange} handleCheckbox={handleCheckbox} setFieldValue={setFieldValue} />
					<I2CAnalog1219 values={values} handleChange={handleChange} />
					<TouchpadDataConfiguration values={values} handleChange={handleChange} handleCheckbox={handleCheckbox} />
					<WiiExtension values={values} handleChange={handleChange} />
					<SNESInput values={values} handleChange={handleChange} />
					<BuzzerSpeaker values={values} handleChange={handleChange} />
					<PlayerNumber values={values} handleChange={handleChange} />
					<DualInput values={values} handleChange={handleChange} />
					<ExtraButtonConfiguration values={values} handleChange={handleChange} />
					<Tilt values={values} handleChange={handleChange} handleCheckbox={handleCheckbox} />
					<BoardLed values={values} handleChange={handleChange} />
					<BootselButton values={values} handleChange={handleChange} />
					<KeyboardHost values={values} handleChange={handleChange} setFieldValue={setFieldValue} />
					<HETrigger values={values} handleChange={handleChange} handleCheckbox={handleCheckbox} setFieldValue={setFieldValue} />
					<JinglePlayer values={values} handleChange={handleChange} handleCheckbox={handleCheckbox} setFieldValue={setFieldValue} />
					<div className="mt-3">
						<Button type="submit">{t('Common:button-save-label')}</Button>
						{saveMessage && <span className="ms-3">{saveMessage}</span>}
					</div>
				</Form>
			)}
		</Formik>
	);
}
