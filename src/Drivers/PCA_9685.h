//
// Created by lnx on 14.08.2025.
//

#ifndef MYSTICCREATURESFIGUR_PCA_9685_H
#define MYSTICCREATURESFIGUR_PCA_9685_H
#include <Wire.h>

// PCA9685 Servo Controller Konfiguration

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

    /**
     * @brief Writes a single byte to a register on the PCA9685.
     * @param reg The register address.
     * @param value The value to write.
     */
    void writeRegister(uint8_t reg, uint8_t value) const;

public:
    /**
     * @brief Constructor for the PCA9685 driver.
     * @param wire Pointer to the TwoWire (I2C) instance.
     * @param i2cAddr The I2C address of the PCA9685 chip.
     */
    PCA_9685(TwoWire *wire, int8_t i2cAddr);

    /**
     * @brief Initializes the PCA9685 with a specific PWM update frequency.
     * @param updateFrequency The target frequency in Hz (e.g., 50 for servos).
     */
    void initPCA9685(uint updateFrequency) const;

    /**
     * @brief Sets the PWM duty cycle for a specific channel.
     * @param channel The channel index (0-15).
     * @param value The 12-bit PWM value (0-4095).
     */
    void setPWM(uint8_t channel, uint16_t value) const;

    /**
     * @brief Disables PWM output on a specific channel.
     * @param channel The channel index (0-15).
     */
    void disablePWM(uint8_t channel) const;
};


#endif //MYSTICCREATURESFIGUR_PCA_9685_H
