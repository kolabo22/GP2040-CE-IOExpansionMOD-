#ifndef _JINGLE_PLAYER_H_
#define _JINGLE_PLAYER_H_

#include "gpaddon.h"
#include "storagemanager.h" // Storageクラスのために追加
#include "pico/stdlib.h"
#include "hardware/uart.h"

// 設定構造体の定義
struct JingleOptions {
    bool enabled;
    uint8_t volume;
};

class JinglePlayerAddon : public GPAddon {
public:
    virtual bool available() { 
        // getAddonSettings を getAddonOptions に修正
        return Storage::getInstance().getAddonOptions().jingleOptions.enabled; 
    }
    virtual void setup();
    virtual void process();
    virtual void preprocess() {}
    virtual std::string name() { return "JinglePlayer"; }

private:
    void playSelectedModeJingle();
    void setVolume(uint8_t volume);
    void play(uint16_t index);
    void sendCommand(uint8_t buf[10]); // 配列であることを明示

    uint8_t volume;
    bool _hasPlayedOnBoot;
    bool _wasConfigMode;
};

#endif
