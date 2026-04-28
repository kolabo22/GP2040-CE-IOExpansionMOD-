import React from 'react';
import { useTranslation } from 'react-i18next';
import * as yup from 'yup';
import Section from '../Components/Section';

// DEFAULT_VALUESに展開される初期状態
export const jinglePlayerState = {
	jinglePlayerOptions: {
		enabled: 0,
		volume: 15,
	},
};

// schemaに展開されるバリデーション
export const jinglePlayerScheme = {
	jinglePlayerOptions: yup.object().shape({
		enabled: yup.number().label('Enabled'),
		volume: yup.number().label('Volume'),
	}),
};

const JinglePlayer = ({ values, handleChange, handleCheckbox, setFieldValue }) => {
	const { t } = useTranslation();
	
	// valuesの中に jinglePlayerOptions がない場合に備える安全策
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
