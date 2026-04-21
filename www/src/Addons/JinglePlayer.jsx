import React, { useEffect } from 'react';
import { useTranslation } from 'react-i18next';
import * as yup from 'yup';
import FormSelect from '../Components/FormSelect';

export const jinglePlayerScheme = {
	jinglePlayerOptions: yup.object().shape({
		enabled: yup.boolean().label('Enabled'),
		volume: yup.number().label('Volume').min(0).max(30).required(),
		selectedId: yup.number().label('Selected Jingle ID').min(1).max(21).required(),
	}),
};

export const jinglePlayerState = {
	jinglePlayerOptions: {
		enabled: false,
		volume: 15,
		selectedId: 1,
	},
};

const JinglePlayer = ({ values, errors, handleChange, setFieldValue }) => {
	const { t } = useTranslation();

	// 【重要】これがないと、新規追加した selectedId が空で送信されセーブに失敗します
	useEffect(() => {
		if (values.jinglePlayerOptions) {
			if (values.jinglePlayerOptions.selectedId === undefined) {
				setFieldValue('jinglePlayerOptions.selectedId', 1);
			}
			if (values.jinglePlayerOptions.volume === undefined) {
				setFieldValue('jinglePlayerOptions.volume', 15);
			}
		}
	}, []);

	const jingleOptions = Array.from({ length: 21 }, (_, i) => ({
		label: `Jingle ${i + 1}`,
		value: i + 1,
	}));

	return (
		<div className="card border-secondary mb-3">
			<div className="card-header">
				<strong>Jingle Player (UART Mode)</strong>
			</div>
			<div className="card-body">
				<div className="row mb-3">
					<div className="col-sm-6">
						<label className="form-label">Enabled</label>
						<div className="form-check form-switch">
							<input
								className="form-check-input"
								type="checkbox"
								name="jinglePlayerOptions.enabled"
								checked={values.jinglePlayerOptions?.enabled || false}
								onChange={handleChange}
							/>
						</div>
					</div>
					<div className="col-sm-6">
						<label className="form-label">Volume ({values.jinglePlayerOptions?.volume ?? 15})</label>
						<input
							type="range"
							className="form-range"
							name="jinglePlayerOptions.volume"
							min="0"
							max="30"
							value={values.jinglePlayerOptions?.volume ?? 15}
							onChange={handleChange}
						/>
					</div>
				</div>
				<div className="row">
					<div className="col-sm-6">
						<FormSelect
							label="Selected Jingle (0001.mp3 - 0021.mp3)"
							name="jinglePlayerOptions.selectedId"
							className="form-select"
							value={values.jinglePlayerOptions?.selectedId ?? 1}
							onChange={handleChange}
							error={errors.jinglePlayerOptions?.selectedId}
						>
							{jingleOptions.map((option) => (
								<option key={option.value} value={option.value}>
									{option.label}
								</option>
							))}
						</FormSelect>
					</div>
				</div>
			</div>
		</div>
	);
};

export default JinglePlayer;
