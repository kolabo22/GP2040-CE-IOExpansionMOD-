import React from 'react';
import { useTranslation } from 'react-i18next';
import * as yup from 'yup';
import Section from '../Components/Section';

// schema定義
export const jinglePlayerScheme = {
    jinglePlayerOptions: yup.object().shape({
        enabled: yup.number().label('Enabled'),
        volume: yup.number().label('Volume'),
    }),
};

// 初期値定義
export const jinglePlayerState = {
    jinglePlayerOptions: {
        enabled: 0,
        volume: 15,
    },
};

const JinglePlayer = ({ values, handleChange, handleCheckbox }) => {
    const { t } = useTranslation();
    
    // values から安全に jinglePlayerOptions を取り出す
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
                        // Formikの数値管理(0/1)に合わせてチェック状態を制御
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
                            <option key={i} value={i}>{i}</option>
                        ))}
                    </select>
                </div>
            </div>
        </Section>
    );
};

export default JinglePlayer;
