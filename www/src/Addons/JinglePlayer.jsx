import React from 'react';
import { useTranslation } from 'react-i18next';
import * as yup from 'yup';

import Section from '../Components/Section';

// バリデーション：複雑な shape を避け、トップレベルで定義
export const jinglePlayerScheme = {
    jinglePlayerOptions: yup.object({
        enabled: yup.number().default(0),
        volume: yup.number().default(15),
    }),
};

// 初期値
export const jinglePlayerState = {
    jinglePlayerOptions: {
        enabled: 0,
        volume: 15,
    },
};

const JinglePlayer = ({ values, handleChange, handleCheckbox }) => {
    const { t } = useTranslation();

    // データの安全な取得（データが壊れている場合に備える）
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
                        // 0/1 を bool に変換してチェック状態を維持
                        checked={!!options.enabled}
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
                        value={options.volume ?? 15}
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
