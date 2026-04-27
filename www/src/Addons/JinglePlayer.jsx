import React from 'react';
import { useTranslation } from 'react-i18next';
import * as yup from 'yup';

import Section from '../Components/Section';

// AddonsConfigPage.tsx の schema に統合される定義
export const jinglePlayerScheme = {
    jinglePlayerOptions: yup.object().shape({
        enabled: yup.number().label('Enabled'),
        volume: yup.number().label('Volume'),
    }),
};

// AddonsConfigPage.tsx の DEFAULT_VALUES に統合される初期値
export const jinglePlayerState = {
    jinglePlayerOptions: {
        enabled: 0, // Formik管理下では bool ではなく 0/1 で扱うのが標準
        volume: 15,
    },
};

const JinglePlayer = ({ values, handleChange, handleCheckbox }) => {
    const { t } = useTranslation();
    return (
        <Section title={t('Jingle Player Addon')}>
            <div className="row mb-3">
                <div className="col-sm-3">
                    <label className="form-label">{t('Enabled')}</label>
                    <input
                        className="form-check-input ms-2"
                        type="checkbox"
                        name="jinglePlayerOptions.enabled"
                        checked={Boolean(values.jinglePlayerOptions.enabled)}
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
                        value={values.jinglePlayerOptions.volume}
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
