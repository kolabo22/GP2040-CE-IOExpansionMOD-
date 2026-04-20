import React from 'react';
import { useTranslation } from 'react-i18next';
import { FormCheck, Row, Col } from 'react-bootstrap';
import * as yup from 'yup'; // ← これを追加
import Section from '../Components/Section';

export const jinglePlayerScheme = {
    JinglePlayerEnabled: yup.number().required(),
    jingleVolume: yup.number().min(0).max(30).required(),
};

export const jinglePlayerState = {
    JinglePlayerEnabled: 0,
    jingleVolume: 20,
};

const JinglePlayer = ({ values, errors, handleChange, handleCheckbox }) => {
    const { t } = useTranslation();
    return (
        <Section title={t('AddonsConfig:jingle-player-label')}>
            <Row>
                <FormCheck
                    label={t('Common:switch-enabled')}
                    type="switch"
                    id="JinglePlayerEnabled"
                    isInvalid={!!errors.JinglePlayerEnabled}
                    checked={Boolean(values.JinglePlayerEnabled)}
                    onChange={() => handleCheckbox('JinglePlayerEnabled')}
                />
            </Row>
            {Boolean(values.JinglePlayerEnabled) && (
                <Row className="mt-3">
                    <Col md="6">
                        <label className="form-label">{t('AddonsConfig:jingle-volume-label')}</label>
                        <div className="d-flex align-items-center">
                            <input
                                type="range"
                                className="form-range flex-grow-1"
                                name="jingleVolume"
                                min="0"
                                max="30"
                                value={values.jingleVolume}
                                onChange={handleChange}
                            />
                            <span className="ms-2 badge bg-primary">{values.jingleVolume}</span>
                        </div>
                    </Col>
                    <Col md="12" className="mt-2">
                        <p className="text-muted small">
                            {t('AddonsConfig:jingle-player-description')}
                            <br />
                            <strong>Pins:</strong> VPP: GPIO 21 | BUSY: GPIO 20
                        </p>
                    </Col>
                </Row>
            )}
        </Section>
    );
};

export default JinglePlayer;
