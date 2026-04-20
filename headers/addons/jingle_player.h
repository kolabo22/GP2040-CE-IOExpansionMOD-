#ifndef _JINGLE_PLAYER_H_
#define _JINGLE_PLAYER_H_

#include "gp2040.h"
#include "interfaces/gpaddon.h"

#define JINGLE_PLAYER_VPP_PIN 21
#define JINGLE_PLAYER_BUSY_PIN 20

class JinglePlayerAddon : public GPAddon {
public:
    virtual bool available() override;
    virtual void setup() override;
    virtual void process() override;
    virtual uint16_t get_priority() override { return 100; } // 優先度設定

private:
    void sendOneLineCommand(uint8_t cmd);
    void playJingleByMode();
    void setVolume(uint8_t volume); // 0-30で指定
    
    bool isPlaying();
    uint8_t lastInputMode;
    uint8_t currentVolume; // Webコンフィグから渡される想定
};

#endif
