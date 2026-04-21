#ifndef JINGLE_PLAYER_H
#define JINGLE_PLAYER_H

#include "gpaddon.h" // GPAddonを継承するために必要
#include "pico/stdlib.h"
#include "hardware/uart.h"

#define JQ8900_UART      uart1
#define JQ8900_TX_PIN    20
#define JQ8900_RX_PIN    21
#define JQ8900_BAUD      9600

// GPAddon を継承するように変更
class JinglePlayer : public GPAddon {
public:
    void setup();
    void preprocess(); // 毎フレームの処理（Addonシステムに必要）
    void setVolume(uint8_t volume); // 0-30
    void play(uint16_t trackId);    // 1-65535
    void stop();
    
    // Addonシステム用の識別名
    virtual std::string name() { return "JinglePlayer"; }

private:
    void sendCommand(uint8_t type, uint8_t* data, uint8_t len);
    
    // 設定値を保持する変数
    bool enabled;
    uint8_t volume;
};

#endif
