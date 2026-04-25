void JinglePlayerAddon::process() {
    // 起動直後の判定を少し遅らせるためのカウンター
    static uint32_t bootDelayCount = 0;

    // 1. 起動直後の再生（少し待ってから判定することでS2起動を確実に拾う）
    if (!_hasPlayedOnBoot) {
        if (bootDelayCount < 50) { // 約50回分（数十ms）待機
            bootDelayCount++;
            return;
        }

        // ここでようやく判定
        bool currentConfigMode = DriverManager::getInstance().isConfigMode();
        setVolume(this->volume);

        if (currentConfigMode) {
            play(21); // ここで21番が鳴るはず
        } else {
            playSelectedModeJingle();
        }

        _hasPlayedOnBoot = true;
        _wasConfigMode = currentConfigMode;
        return;
    }

    // 2. 設定モードからの復帰判定
    bool currentConfigMode = DriverManager::getInstance().isConfigMode();
    if (_wasConfigMode && !currentConfigMode) {
        playSelectedModeJingle();
    }
    _wasConfigMode = currentConfigMode;
}
