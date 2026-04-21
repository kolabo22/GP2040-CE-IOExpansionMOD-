import React from 'react';
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

const JinglePlayer = ({ values, errors, handleChange }) => {
    const { t } = useTranslation();

    // データの安全な取得（データがない場合はデフォルト値を使う）
    const options = values.jinglePlayerOptions || jinglePlayerState.jinglePlayerOptions;

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
                                checked={options.enabled}
                                onChange={handleChange}
                            />
                        </div>
                    </div>

                    <div className="col-sm-6">
                        <label className="form-label">Volume ({options.volume})</label>
                        <input
                            type="range"
                            className="form-range"
                            name="jinglePlayerOptions.volume"
                            min="0"
                            max="30"
                            value={options.volume}
                            onChange={handleChange}
                        />
                    </div>
                </div>

                <div className="row">
                    <div className="col-sm-6">
                        <FormSelect
                            label="Selected Jingle (0001.mp3 - 0020.mp3)"
                            name="jinglePlayerOptions.selectedId"
                            value={options.selectedId}
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
