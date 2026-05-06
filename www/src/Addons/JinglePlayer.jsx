import React from 'react';
import { useTranslation } from 'react-i18next';
import * as yup from 'yup';
import Section from '../Components/Section';

// AddonsConfigPageのDEFAULT_VALUESにマージされる初期値
export const jinglePlayerState = {
	addonOptions: {
		jinglePlayerOptions: {
			enabled: 0,
			volume: 15,
		},
	},
};

// バリデーションスキーマも階層を合わせる
export const jinglePlayerScheme = {
	addonOptions: yup.object().shape({
		jinglePlayerOptions: yup.object().shape({
			enabled: yup.number().label('Enabled'),
			volume: yup.number().label('Volume'),
		}),
	}),
};

const JinglePlayer = ({ values, handleChange, handleCheckbox }) => {
	const { t } = useTranslation();
	
	// values内の深い階層を参照
	const options = values?.addonOptions?.jinglePlayerOptions || jinglePlayerState.addonOptions.jinglePlayerOptions;

	return (
		<Section title={t('Jingle Player Addon')}>
			<div className="row mb-3">
				<div className="col-sm-3">
					<label className="form-label">{t('Enabled')}</label>
					<input
						className="form-check-input ms-2"
						type="checkbox"
						// name属性を完全なパスで指定
						name="addonOptions.jinglePlayerOptions.enabled"
						checked={Boolean(options.enabled)}
						onChange={() => handleCheckbox('addonOptions.jinglePlayerOptions.enabled')}
					/>
				</div>
			</div>
			<div className="row mb-3">
				<div className="col-sm-3">
					<label className="form-label">{t('Volume (0-30)')}</label>
					<select
						className="form-select form-select-sm"
						name="addonOptions.jinglePlayerOptions.volume"
						value={options.volume}
						onChange={handleChange}
					>
						{Array.from({ length: 31 }, (_, i) => (
							<option key={i} value={i}>
								{i}
							</option>
						))}
					</select>
				</div>
			</div>
		</Section>
	);
};

export default JinglePlayer;
