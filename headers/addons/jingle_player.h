#ifndef JINGLE_PLAYER_H
#define JINGLE_PLAYER_H

#include "gpaddon.h"
#include "pico/stdlib.h"
#include "hardware/uart.h"

#define JQ8900_UART      uart1
#define JQ8900_TX_PIN    20
#define JQ8900_RX_PIN    21
#define JQ8900_BAUD      9600

class JinglePlayerAddon : public GPAddon {
public:
    void setup();
    void preprocess();
    void process(); // ← これを追加しないとエラーになります
    void setVolume(uint8_t volume); 
    void play(uint16_t trackId);    
    void stop();
    
    virtual std::string name() { return "JinglePlayer"; }

private:
    void sendCommand(uint8_t type, uint8_t* data, uint8_t len);
    
    bool enabled;
    uint8_t volume;
};

#endif
