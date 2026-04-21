import React, { useEffect } from 'react';
import { useTranslation } from 'react-i18next';
import * as yup from 'yup';
import FormSelect from '../Components/FormSelect';

export const jinglePlayerScheme = {
	jinglePlayerOptions: yup.object().shape({
		enabled: yup.boolean().label('Enabled'),
		volume: yup.number().label('Volume').min(0).max(30).required(),
		selectedId: yup.number().label('Selected Jingle ID').min(0).max(20).required(),
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

	// ページ読み込み時にデータ構造が古い（selectedIdがない等）場合、初期値を流し込む
	useEffect(() => {
		if (values.jinglePlayerOptions && values.jinglePlayerOptions.selectedId === undefined) {
			setFieldValue('jinglePlayerOptions.selectedId', 1);
		}
	}, [values, setFieldValue]);

	const jingleOptions = Array.from({ length: 20 }, (_, i) => ({
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
						<label className="form-label">Volume ({values.jinglePlayerOptions?.volume || 0})</label>
						<input
							type="range"
							className="form-range"
							name="jinglePlayerOptions.volume"
							min="0"
							max="30"
							value={values.jinglePlayerOptions?.volume || 0}
							onChange={handleChange}
						/>
					</div>
				</div>

				<div className="row">
					<div className="col-sm-6">
						<FormSelect
							label="Selected Jingle (0001.mp3 - 0020.mp3)"
							name="jinglePlayerOptions.selectedId"
							className="form-select"
							value={values.jinglePlayerOptions?.selectedId || 1}
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
