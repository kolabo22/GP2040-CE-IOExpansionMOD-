#ifndef JINGLE_PLAYER_H
#define JINGLE_PLAYER_H

#include "gpaddon.h"
#include "pico/stdlib.h"
#include "hardware/uart.h"

// UART設定
#define JQ8900_UART      uart1
#define JQ8900_TX_PIN    20
#define JQ8900_RX_PIN    21
#define JQ8900_BAUD      9600

class JinglePlayerAddon : public GPAddon {
public:
    virtual void setup() override;
    virtual void preprocess() override;
    virtual void process() override;
    virtual void postprocess(bool) override;
    virtual void reinit() override;
    virtual bool available() override;
    virtual std::string name() override;

    void setVolume(uint8_t volume); 
    void play(uint16_t trackId);    
    void stop();
    
private:
    void sendCommand(uint8_t type, uint8_t* data, uint8_t len);
    void playSelectedModeJingle(); // 機種に応じた再生を行う内部関数

    bool enabled;
    uint8_t volume;
    bool _wasConfigMode; // 設定モード状態を保持するフラグ
};

#endif
