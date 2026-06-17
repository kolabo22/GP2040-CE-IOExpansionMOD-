import { useEffect, useState, useRef, useContext } from 'react';
import { AppContext } from '../Contexts/AppContext';
import { Button, Form, Col } from 'react-bootstrap';
import { useTranslation } from 'react-i18next';
import merge from 'lodash/merge';
import cloneDeep from 'lodash/cloneDeep';

import Section from '../Components/Section';
import WebApi from '../Services/WebApi';

const FILE_EXTENSION = '.gp2040';
const FILENAME = 'gp2040ce_backup_{DATE}' + FILE_EXTENSION;

const API_BINDING = {
	display: {
		get: WebApi.getDisplayOptions,
		set: WebApi.setDisplayOptions,
	},
	splash: {
		get: WebApi.getSplashImage,
		set: WebApi.setSplashImage,
	},
	gamepad: {
		get: WebApi.getGamepadOptions,
		set: WebApi.setGamepadOptions,
	},
	keyboard: {
		get: WebApi.getKeyMappings,
		set: WebApi.setKeyMappings,
	},
	led: { get: WebApi.getLedOptions, set: WebApi.setLedOptions },
	ledTheme: {
		get: WebApi.getCustomTheme,
		set: WebApi.setCustomTheme,
	},
	macros: {
		get: WebApi.getMacroAddonOptions,
		set: WebApi.setMacroAddonOptions,
	},
	pins: {
		get: WebApi.getPinMappings,
		set: WebApi.setPinMappings,
	},
	profiles: {
		get: WebApi.getProfileOptions,
		set: WebApi.setProfileOptions,
	},
	heTrigger: {
		get: WebApi.getHETriggerCalibrations,
		set: WebApi.setHETriggerCalibrations,
	},
	addons: {
		get: WebApi.getAddonsOptions,
		set: WebApi.setAddonsOptions,
	},
	// new api, add it here
	// "example":	{get: WebApi.getNewAPI,			set: WebApi.setNewAPI},
};

export default function BackupPage() {
	const inputFileSelect = useRef();

	const [optionState, setOptionStateData] = useState({});
	const [importOptions, setImportOptions] = useState({});
	const [exportOptions, setExportOptions] = useState({});

	const [noticeMessage, setNoticeMessage] = useState('');
	const [saveMessage, setSaveMessage] = useState('');
	const [loadMessage, setLoadMessage] = useState('');
	const { setLoading } = useContext(AppContext);

	const { t } = useTranslation('');

	useEffect(() => {
		async function fetchData() {
			let exportData = {};
			for (const [key, func] of Object.entries(API_BINDING)) {
				exportData[key] = await func.get(setLoading);
			}
			setOptionStateData(exportData);
		}
		fetchData();

		let defaults = {};
		for (const [key] of Object.entries(API_BINDING)) {
			defaults[key] = true;
		}
		setImportOptions(defaults);
		setExportOptions(defaults);
	}, []);

	const validateValues = (data, nextData) => {
		// Handle array cases - always use backup data for arrays to allow clearing
		if (Array.isArray(nextData)) {
			return nextData;
		}

		return merge(cloneDeep(data), nextData);
	};

	const setOptionsToAPIStorage = async (options) => {
		for (const [key, func] of Object.entries(API_BINDING)) {
			const values = options[key];
			if (values) {
				try {
					await func.set(values);
				} catch (error) {
					setNoticeMessage(`Failed to set ${key} options: ${error.message}`);
				}
			}
		}
	};

// ==============================================================================
// 🎯 【フロント側】Backup To File ボタンの完全ファイルレス化 (handleSave を差し替え)
// ==============================================================================
  const handleSave = async () => {
    try {
      // ❌ PCへのファイル生成・ダウンロード処理(Blob/a.click)は1行残らず全削除！
      // 実機側の /api/backup エンドポイントに対して、「4MB目にRAW退避しろ」という信号だけを送る
      const response = await fetch('/api/backup', { method: 'POST' });
      
      if (response.ok) {
        // ファイルを作らせず、画面上に「実機内への直流し成功」を明示
        setSaveMessage("SUCCESS: 設定を実機内隔離領域(4MB目)へRAW直流し固定しました！");
      } else {
        setNoticeMessage("SAVE FAILED: サーバーエラーが発生しました。");
      }
    } catch (error) {
      setNoticeMessage("SAVE FAILED: 通信に失敗しました。");
    }
    setTimeout(() => {
      setSaveMessage('');
      setNoticeMessage('');
    }, 5000);
  };

// ==============================================================================
// 🎯 【フロント側】Restore From File ボタンのファイルダイアログ完全消去 (handleFileSelect を差し替え)
// ==============================================================================
  const handleFileSelect = async (ev) => {
    // 💡 この関数が呼ばれる＝Loadボタンが押されたとき
    // ❌ PCのファイル選択ダイアログは完全に無視し、実機へのダミートリガーPOSTへ直撃させます
    if (window.confirm("PCからのファイル選択は不要です。実機内の4MB領域から設定を一撃ロードしますか？")) {
      try {
        // ファイルデータは1バイトも添付せず、中身空っぽのPOSTリクエスト（ダミートリガー信号）を直接送信
        const response = await fetch('/api/restore', { method: 'POST' });
        
        if (response.ok) {
          setLoadMessage("RESTORE SUCCESS: 実機内のお気に入りから上書き復元しました！");
          setNoticeMessage('');
        } else {
          setNoticeMessage("RESTORE FAILED: 復元に失敗しました。");
        }
      } catch (error) {
        setNoticeMessage("RESTORE FAILED: 通信に失敗しました。");
      }
      setTimeout(() => {
        setLoadMessage('');
        setNoticeMessage('');
      }, 5000);
    }
  };

	return (
		<>
			<Section title={t('BackupPage:save-header-text')}>
				<Col>
					<Form.Group className={'row mb-3'}>
						<div className={'col'}>
							{Object.entries(API_BINDING).map(([key]) => (
								<Form.Check
									id={`export_${key}`}
									key={`export_${key}`}
									label={t('BackupPage:save-export-option-label', {
										api: t(`BackupPage:api-${key}-text`),
									})}
									type={'checkbox'}
									checked={exportOptions[key] ?? false}
									onChange={() => {
										setExportOptions((prev) => ({
											...prev,
											[key]: !prev[key],
										}));
									}}
								/>
							))}
						</div>
					</Form.Group>
					<div
						style={{
							display: 'flex',
							flexDirection: 'row',
						}}
					>
						<Button type="submit" onClick={handleSave}>
							{t('Common:button-save-label')}
						</Button>
						<div
							style={{
								height: '100%',
								paddingLeft: 24,
								fontWeight: 600,
								color: 'darkcyan',
								alignSelf: 'center',
							}}
						>
							{saveMessage ? saveMessage : null}
						</div>
					</div>
				</Col>
			</Section>
			<Section title={t('BackupPage:load-header-text')}>
				<div className="alert alert-warning">
					{t(`BackupPage:pin-version-warning-text`)}
				</div>
				<Col>
					<Form.Group className={'row mb-3'}>
						<div className={'col'}>
							{Object.entries(API_BINDING).map(([key]) => (
								<Form.Check
									id={`import_${key}`}
									key={`import_${key}`}
									label={t('BackupPage:load-export-option-label', {
										api: t(`BackupPage:api-${key}-text`),
									})}
									type={'checkbox'}
									checked={importOptions[key] ?? false}
									onChange={() => {
										setImportOptions((prev) => ({
											...prev,
											[key]: !prev[key],
										}));
									}}
								/>
							))}
						</div>
					</Form.Group>
					<input
						ref={inputFileSelect}
						type={'file'}
						accept={FILE_EXTENSION}
						style={{ display: 'none' }}
						onChange={handleFileSelect.bind(this)}
					/>
					<div
						style={{
							display: 'flex',
							flexDirection: 'row',
						}}
					>
						<Button
							onClick={() => {
								inputFileSelect.current.click();
							}}
						>
							{t('Common:button-load-label')}
						</Button>
						<div
							style={{
								height: '100%',
								paddingLeft: 24,
								fontWeight: 600,
								color: 'darkcyan',
								alignSelf: 'center',
							}}
						>
							<span>{loadMessage ? loadMessage : null}</span>
							<span style={{ color: 'red', fontWeight: 'bold' }}>
								{noticeMessage ? noticeMessage : null}
							</span>
						</div>
					</div>
				</Col>
			</Section>
		</>
	);
}
