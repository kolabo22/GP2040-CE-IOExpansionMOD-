#ifndef _JINGLE_PLAYER_H_
#define _JINGLE_PLAYER_H_

#include "gpaddon.h"
#include "GamepadEnums.h"

class JinglePlayerAddon : public GPAddon {
public:
    virtual bool available() override;
    virtual void setup() override;
    virtual void process() override;
    virtual void preprocess() override;
		virtual void reinit() override;
    virtual void postprocess(bool reportSent) override;
    virtual std::string name() override { return "JinglePlayer"; }

private:
    void sendOneLineCommand(uint8_t cmd);
    void playJingleByMode();
    void setVolume(uint8_t volume);
    bool isPlaying();
    uint8_t lastInputMode;
    bool bootPlayed;
};

#endif
