import React from 'react';
import { useTranslation } from 'react-i18next';
import * as yup from 'yup';
import Section from '../Components/Section';

export const jinglePlayerScheme = {
    jinglePlayerOptions: yup.object().shape({
        enabled: yup.number().label('Enabled'),
        volume: yup.number().label('Volume'),
        selectedId: yup.number().label('Selected ID'),
    }),
};

export const jinglePlayerState = {
    jinglePlayerOptions: {
        enabled: 0,
        volume: 15,
        selectedId: 0,
    },
};

const JinglePlayer = ({ values, handleChange, handleCheckbox }) => {
    const { t } = useTranslation();
    const options = values?.jinglePlayerOptions || jinglePlayerState.jinglePlayerOptions;

    // 数値として確実に反映させるためのラッパー
    const handleNumberChange = (e) => {
        e.target.value = parseInt(e.target.value, 10);
        handleChange(e);
    };

    return (
        <Section title={t('Jingle Player Addon')}>
            <div className="row mb-3">
                <div className="col-sm-3">
                    <label className="form-label">{t('Enabled')}</label>
                    <input
                        className="form-check-input ms-2"
                        type="checkbox"
                        name="jinglePlayerOptions.enabled"
                        checked={Boolean(options.enabled)}
                        // handleCheckboxは内部で数値を扱うのでそのまま
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
                        // 文字列ではなく数値として送る
                        onChange={handleNumberChange}
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
