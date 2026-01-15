//
// Created by lnx on 14.08.2025.
//

#ifndef MYSTICCREATURESFIGUR_PCA_9685_H
#define MYSTICCREATURESFIGUR_PCA_9685_H
#include <Wire.h>

// PCA9685 Konfiguration

#define MODE1_REG 0x00
#define PRESCALE_REG 0xFE
#define PWM_BASE_REG 0x06

#define PWM_CHANNEL_WITH 4 // 4 Byte per Channel
#define PWM_MAX_FREQUENCY 1526 // Maximal Hz für PWM
#define OSC_CLOCK_FREQ 25000000 // 25 MHz für den PCA9685 - base clock frequency
#define CLOCK_COUNTER_MAXVAL 4096


class PCA_9685 {
    TwoWire *_wire = nullptr;
    uint8_t _i2CAddr = 0;


    void writeRegister(uint8_t reg, uint8_t value) const;

public:
    PCA_9685(TwoWire *wire, int8_t i2cAddr);

    void initPCA9685(uint updateFrequency) const;

    void setPWM(uint8_t channel, uint16_t value) const;

    void disablePWM(uint8_t channel) const;
};


#endif //MYSTICCREATURESFIGUR_PCA_9685_H
