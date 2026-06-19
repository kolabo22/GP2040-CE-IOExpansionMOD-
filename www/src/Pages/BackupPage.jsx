import React, { useState } from 'react';
import { Button, Alert } from 'react-bootstrap';
import { useTranslation } from 'react-i18next';
import Section from '../Components/Section';

export default function BackupPage() {
  const { t } = useTranslation();

  const [saveMessage, setSaveMessage] = useState('');
  const [loadMessage, setLoadMessage] = useState('');
  const [noticeMessage, setNoticeMessage] = useState('');

  // ❌ PCへのファイル保存(Blob)をカットし、C++の32KB RAW直流しルートへ信号ポスト
  const handleSave = async () => {
    try {
      const response = await fetch('/api/backup', { method: 'POST' });
      if (response.ok) {
        setSaveMessage("SUCCESS: その時点の全設定を実機内隔離聖域(4MB目)へRAW直流し保存しました！");
        setNoticeMessage('');
      } else {
        setNoticeMessage("SAVE FAILED: サーバーエラーが発生しました。");
      }
    } catch (error) {
      setNoticeMessage("SAVE FAILED: 通信に失敗しました。");
    }
    setTimeout(() => { setSaveMessage(''); setNoticeMessage(''); }, 5000);
  };

  // ❌ PCからのファイル選択を不要にし、実機内の4MB隔離領域から全設定を一撃で復元
  const handleFileSelect = async () => {
    if (window.confirm("PCからのファイル選択は不要です。実機内の4MB隔離領域から全設定を一撃で復元しますか？")) {
      try {
        const response = await fetch('/api/restore', { method: 'POST' });
        if (response.ok) {
          // 💡 画面に綺麗な緑文字でSUCCESSを出し、自動再起動はさせずに設定画面をキープ！
          setLoadMessage("RESTORE SUCCESS: 隔離領域から全設定を上書き復元しました！続けて設定可能です。");
          setNoticeMessage('');
        } else {
          setNoticeMessage("RESTORE FAILED: 復元に失敗しました。");
        }
      } catch (error) {
        setNoticeMessage("RESTORE FAILED: 通信に失敗しました。");
      }
      setTimeout(() => { setLoadMessage(''); setNoticeMessage(''); }, 5000);
    }
  };

  return (
    <Section title={t('BackupPage:backup-header-text', 'Data Backup and Recovery')}>
      <div className="mb-3">
        <p>PCへのファイル保存や読み込みは一切不要です。すべてRP2040実機内の隔離聖域（4MB目）で完結します。</p>
      </div>

      {saveMessage && <Alert variant="success">{saveMessage}</Alert>}
      {loadMessage && <Alert variant="success">{loadMessage}</Alert>}
      {noticeMessage && <Alert variant="danger">{noticeMessage}</Alert>}

      <div className="row">
        <div className="col-md-6 mb-3">
          <div className="p-3 border rounded bg-light">
            <h5>📥 お気に入りマスター設定として保存</h5>
            <p className="small text-muted">現在のすべての設定項目を、実機内の永久隔離聖域へRAW直流し保存します。</p>
            <Button variant="primary" onClick={handleSave}>
              実機内へ隔離保存
            </Button>
          </div>
        </div>

        <div className="col-md-6 mb-3">
          <div className="p-3 border rounded bg-light">
            <h5>📤 隔離聖域から一撃復元</h5>
            <p className="small text-muted">実機内4MB領域に保存されているマスター設定を現在の通常領域へ上書き復元します。自動再起動はかかりません。</p>
            <Button variant="warning" onClick={handleFileSelect}>
              実機内から復元
            </Button>
          </div>
        </div>
      </div>
    </Section>
  );
}
