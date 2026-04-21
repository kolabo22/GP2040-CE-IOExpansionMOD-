#ifndef JINGLE_PLAYER_H
#define JINGLE_PLAYER_H

#include "pico/stdlib.h"
#include "hardware/uart.h"

#define JQ8900_UART      uart1
#define JQ8900_TX_PIN    20
#define JQ8900_RX_PIN    21
#define JQ8900_BAUD      9600

class JinglePlayer {
public:
    void setup();
    void setVolume(uint8_t volume); // 0-30
    void play(uint16_t trackId);    // 1-65535
    void stop();
    
private:
    void sendCommand(uint8_t type, uint8_t* data, uint8_t len);
};

#endif
