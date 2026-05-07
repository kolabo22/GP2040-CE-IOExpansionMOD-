#ifndef _JINGLE_PLAYER_H_
#define _JINGLE_PLAYER_H_

#include "gpaddon.h"
#include "pico/stdlib.h"
#include "hardware/uart.h"
#include "storagemanager.h"

class JinglePlayerAddon : public GPAddon {
public:
    virtual bool available() { 
        return Storage::getInstance().getAddonOptions().jinglePlayerOptions.enabled; 
    }
    virtual void setup();
    virtual void process();
    virtual void preprocess() {}
    virtual void postprocess(bool reportSent);
    virtual void reinit();
    virtual std::string name() { return "JinglePlayer"; }

private:
    void playSelectedModeJingle();
    void setVolume(uint8_t volume);
    void play(uint16_t index);
    void runStateMachine();

    enum class PlayState { IDLE, WAIT_BOOT, SET_VOL, WAIT_VOL, PLAY, FINISHED };
    PlayState _state;
    uint32_t _timer;
    uint8_t _volume;
    bool _isConfig;
};

#endif
