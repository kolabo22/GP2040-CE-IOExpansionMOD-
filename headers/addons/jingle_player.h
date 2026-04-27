#ifndef _JINGLE_PLAYER_H_
#define _JINGLE_PLAYER_H_

#include "gpaddon.h"
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
        return Storage::getInstance().getAddonSettings().jingleOptions.enabled; 
    }
    virtual void setup();
    virtual void process();
    virtual void preprocess() {}
    virtual std::string name() { return "JinglePlayer"; }

private:
    void playSelectedModeJingle();
    void setVolume(uint8_t volume);
    void play(uint16_t index);
    void sendCommand(uint8_t buf[10]); // 引数を10バイト配列に変更

    uint8_t volume;
    bool _hasPlayedOnBoot;
    bool _wasConfigMode;
};

#endif
