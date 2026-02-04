//
// Created by lnx on 13.08.2025.
//

#ifndef MYSTICCREATURESFIGUR_WORKERDUMMY_H
#define MYSTICCREATURESFIGUR_WORKERDUMMY_H
#include <Wire.h>

#include "../Idmx_FixtureWorker.h"
#include "../Drivers/PCA_9685.h"


// Servo-Konfiguration
#define SERVO_MIN_PULSE 150  // 1.5ms (0°) in 1/4096 Schritten
#define SERVO_MAX_PULSE 600  // 2.0ms (180°) in 1/4096 Schritten
#define SERVO_CHANNEL 0      // Kanal für den Servo
#define SERVO_UPDATE_FREQUENCY 50 // 50 Hz für Servos

class ServoFixtureWorker : public Idmx_FixtureWorker {
    PCA_9685 *_pca9685;

    /**
     * @brief Internal helper to check if a servo should be disabled to save power or prevent jitter.
     * @param servoNum The index of the servo to check.
     */
    void checkAndDisablePWM(int servoNum) const;

public:
    /**
     * @brief Constructor for the Servo Fixture Worker.
     * @param fixture The Fixture configuration.
     */
    explicit ServoFixtureWorker(const Fixture &fixture);

    /**
     * @brief Processes the DMX data and moves the servos to the corresponding positions.
     * @param data Pointer to the buffer containing position values (0-255).
     * @param size Number of channels.
     */
    void SendValues(const uint8_t *data, size_t size) override;

    /**
     * @brief Sets the position of a specific servo channel.
     * @param channel The PCA9685 channel the servo is connected to.
     * @param value The target position value (0-255).
     */
    void setServoPosition(uint8_t channel, uint8_t value) const;

    /**
     * @brief Periodically called to handle timed servo movements or power management.
     */
    void tick() override;
};


#endif //MYSTICCREATURESFIGUR_WORKERDUMMY_H