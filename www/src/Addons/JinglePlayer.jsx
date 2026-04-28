import React from 'react';
import { useTranslation } from 'react-i18next';
import * as yup from 'yup';
import Section from '../Components/Section';

export const jinglePlayerScheme = {
	jinglePlayerOptions: yup.object().shape({
		enabled: yup.number().label('Enabled'),
		volume: yup.number().label('Volume'),
	}),
};

export const jinglePlayerState = {
	jinglePlayerOptions: {
		enabled: 0,
		volume: 15,
	},
};

const JinglePlayer = ({ values, setFieldValue, handleCheckbox }) => {
	const { t } = useTranslation();
	
	// 安全な値の取得
	const options = values?.jinglePlayerOptions || jinglePlayerState.jinglePlayerOptions;

	return (
		<Section title={t('Jingle Player Addon')}>
			<div className="row mb-3">
				<div className="col-sm-3">
					<label className="form-label">{t('Enabled')}</label>
					<input
						className="form-check-input ms-2"
						type="checkbox"
						name="jinglePlayerOptions.enabled"
						checked={options.enabled === 1}
						onChange={() => handleCheckbox('jinglePlayerOptions.enabled')}
					/>
				</div>
			</div>
			<div className="row mb-3">
				<div className="col-sm-3">
					<label className="form-label">{t('Volume (0-30)')}</label>
					<select
						className="form-select form-select-sm"
						name="jinglePlayerOptions.volume"
						value={options.volume}
						// 直接数値として値をセットすることで AddonsConfigPage の変更検知を確実に通す
						onChange={(e) => setFieldValue('jinglePlayerOptions.volume', parseInt(e.target.value, 10))}
					>
						{Array.from({ length: 31 }, (_, i) => (
							<option key={i} value={i}>{i}</option>
						))}
					</select>
				</div>
			</div>
		</Section>
	);
};

export default JinglePlayer;
