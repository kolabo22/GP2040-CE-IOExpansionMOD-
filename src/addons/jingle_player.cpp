void JinglePlayerAddon::process() {
    static uint32_t bootDelay = 0;

    // 起動直後の再生（約100ms程度待ってから判定）
    if (!_hasPlayedOnBoot) {
        if (bootDelay < 200) { // ループ回数で待機
            bootDelay++;
            return;
        }
        
        bool isConfig = DriverManager::getInstance().isConfigMode();
        setVolume(this->volume);
        if (isConfig) {
            play(21); // 設定モード：21番
        } else {
            playSelectedModeJingle(); // 通常：機種別
        }
        _hasPlayedOnBoot = true;
        _wasConfigMode = isConfig;
        return;
    }

    // 設定モードからの復帰（WebUIを閉じた時）
    bool currentConfig = DriverManager::getInstance().isConfigMode();
    if (_wasConfigMode && !currentConfig) {
        playSelectedModeJingle();
    }
    _wasConfigMode = currentConfig;
}
