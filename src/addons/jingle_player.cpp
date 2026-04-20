#include "addons/jingle_player.h"
#include "ConfigManager.h"
#include "hardware/gpio.h"
#include "hardware/sync.h"
#include "pico/time.h"

// extern "C" で囲む（将来的にhttpdコールバック等を追加する場合の備え）
extern "C" {
    // 依存関係解決のため実体をハンドラより前に配置
}

bool JinglePlayerAddon::available() {
    // 本来はConfigManagerから有効フラグを取得
    return true; 
}

void JinglePlayerAddon::setup() {
    gpio_init(JINGLE_PLAYER_VPP_PIN);
    gpio_set_dir(JINGLE_PLAYER_VPP_PIN, GPIO_OUT);
    gpio_put(JINGLE_PLAYER_VPP_PIN, 1);

    gpio_init(JINGLE_PLAYER_BUSY_PIN);
    gpio_set_dir(JINGLE_PLAYER_BUSY_PIN, GPIO_IN);
    gpio_pull_up(JINGLE_PLAYER_BUSY_PIN);

    // 初期モード保持
    lastInputMode = (uint8_t)ConfigManager::getInstance().getOptions().inputMode;
    currentVolume = 20; // デフォルト音量

    // 起動時の初期化シーケンス
    setVolume(currentVolume);
    playJingleByMode();
}

void JinglePlayerAddon::process() {
    uint8_t currentMode = (uint8_t)ConfigManager::getInstance().getOptions().inputMode;
    
    // モードが変更された瞬間（ミニメニュー脱出後など）を検知
    if (currentMode != lastInputMode) {
        playJingleByMode();
        lastInputMode = currentMode;
    }
}

void JinglePlayerAddon::setVolume(uint8_t volume) {
    if (volume > 30) volume = 30;
    
    sendOneLineCommand(0x0A); // 数値クリア
    sleep_ms(5);
    
    // 2桁の数値を送る（例：20なら 0x02 -> 0x00）
    if (volume >= 10) {
        sendOneLineCommand(volume / 10);
        sleep_ms(5);
    }
    sendOneLineCommand(volume % 10);
    sleep_ms(5);
    
    sendOneLineCommand(0x0C); // 音量設定コマンド
}

void JinglePlayerAddon::playJingleByMode() {
    // 再生中の場合は停止させてから次を流す
    if (isPlaying()) {
        sendOneLineCommand(0x13); // Stop
        sleep_ms(10);
    }

    sendOneLineCommand(0x0A); // 数値クリア
    sleep_ms(5);
    sendOneLineCommand(lastInputMode + 1); // モード0->曲1
    sleep_ms(5);
    sendOneLineCommand(0x0B); // 選曲確定・再生
}

bool JinglePlayerAddon::isPlaying() {
    // JQ8900のBUSY端子は「再生中にHigh」
    return gpio_get(JINGLE_PLAYER_BUSY_PIN) == 1;
}

void JinglePlayerAddon::sendOneLineCommand(uint8_t addr) {
    uint32_t status = save_and_disable_interrupts();
    
    gpio_put(JINGLE_PLAYER_VPP_PIN, 0);
    sleep_us(4000); // Guide: 4ms

    for (int i = 0; i < 8; i++) {
        gpio_put(JINGLE_PLAYER_VPP_PIN, 1);
        if (addr & 0x01) {
            sleep_us(600); // 3:1 ratio
            gpio_put(JINGLE_PLAYER_VPP_PIN, 0);
            sleep_us(200);
        } else {
            sleep_us(200); // 1:3 ratio
            gpio_put(JINGLE_PLAYER_VPP_PIN, 0);
            sleep_us(600);
        }
        addr >>= 1;
    }
    gpio_put(JINGLE_PLAYER_VPP_PIN, 1);
    restore_interrupts(status);
void JinglePlayerAddon::preprocess() {}
void JinglePlayerAddon::postprocess(bool reportSent) {}
	}
