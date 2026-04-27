import React from 'react';
import { useTranslation } from 'react-i18next';
import * as yup from 'yup';

import FormSelect from '../Components/FormSelect';
import Section from '../Components/Section';

// バリデーションを緩和してセーブを通るようにする
export const jinglePlayerScheme = yup.object().shape({
    enabled: yup.boolean().label('Enabled'),
    volume: yup.number().label('Volume'),
});

const JinglePlayer = () => {
    const { t } = useTranslation();
    return (
        <Section title={t('Jingle Player Addon')}>
            <div className="row mb-3">
                <FormSelect
                    label={t('Enabled')}
                    name="enabled"
                    className="form-select-sm"
                    options={[
                        { label: t('Disabled'), value: false },
                        { label: t('Enabled'), value: true },
                    ]}
                />
            </div>
            <div className="row mb-3">
                <FormSelect
                    label={t('Volume (0-30)')}
                    name="volume"
                    className="form-select-sm"
                    options={Array.from({ length: 31 }, (_, i) => ({
                        label: i.toString(),
                        value: i,
                    }))}
                />
            </div>
        </Section>
    );
};

export default JinglePlayer;
