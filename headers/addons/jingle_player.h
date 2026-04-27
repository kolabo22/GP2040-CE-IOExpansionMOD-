#ifndef _JINGLE_PLAYER_H_
#define _JINGLE_PLAYER_H_

#include "gpaddon.h"
#include "storagemanager.h"
#include "pico/stdlib.h"
#include "hardware/uart.h"

// 一時的な構造体定義
struct JingleOptions {
    bool enabled;
    uint8_t volume;
};

class JinglePlayerAddon : public GPAddon {
public:
    // 設定が未定義の間は、ビルドを通すために true を返す
    virtual bool available() { return true; }
    virtual void setup();
    virtual void process();
    virtual void preprocess() {}
    virtual void postprocess(bool) {} // 追加：必須関数
    virtual void reinit() {}          // 追加：必須関数
    virtual std::string name() { return "JinglePlayer"; }

private:
    void playSelectedModeJingle();
    void setVolume(uint8_t volume);
    void play(uint16_t index);
    void sendCommand(uint8_t buf[10]);

    uint8_t volume;
    bool _hasPlayedOnBoot;
    bool _wasConfigMode;
};

#endif
