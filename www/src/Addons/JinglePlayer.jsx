import React from 'react';
import { useTranslation } from 'react-i18next';
import * as yup from 'yup';
import Section from '../Components/Section';

// AddonsConfigPageのDEFAULT_VALUESに正しくマージされるための定義
export const jinglePlayerState = {
	jinglePlayerOptions: { // ここは AddonsConfigPage側で addonOptions内に展開されるならこのままでOK、もし直下なら addonOptionsを外側に足す
		enabled: 0,
		volume: 15,
	},
};

export const jinglePlayerScheme = {
	jinglePlayerOptions: yup.object().shape({
		enabled: yup.number().label('Enabled'),
		volume: yup.number().label('Volume'),
	}),
};

const JinglePlayer = ({ values, handleChange, handleCheckbox }) => {
	const { t } = useTranslation();
	
	// config.protoの構造に合わせ、addonOptions 内を参照するように変更
	const options = values?.addonOptions?.jinglePlayerOptions || { enabled: 0, volume: 15 };

	return (
		<Section title={t('Jingle Player Addon')}>
			<div className="row mb-3">
				<div className="col-sm-3">
					<label className="form-label">{t('Enabled')}</label>
					<input
						className="form-check-input ms-2"
						type="checkbox"
						// nameに "addonOptions." を追加することでPicoが保存対象として認識する
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
