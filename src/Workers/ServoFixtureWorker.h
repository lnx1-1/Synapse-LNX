//
// Created by lnx on 13.08.2025.
//

#ifndef MYSTICCREATURESFIGUR_WORKERDUMMY_H
#define MYSTICCREATURESFIGUR_WORKERDUMMY_H
#include <Wire.h>

#include "../Idmx_FixtureWorker.h"
#include "../PCA_9685.h"


// Servo-Konfiguration
#define SERVO_MIN_PULSE 150  // 1.5ms (0°) in 1/4096 Schritten
#define SERVO_MAX_PULSE 600  // 2.0ms (180°) in 1/4096 Schritten
#define SERVO_CHANNEL 0      // Kanal für den Servo
#define SERVO_UPDATE_FREQUENCY 50 // 50 Hz für Servos

class ServoFixtureWorker : public Idmx_FixtureWorker {
    PCA_9685 *_pca9685;

    void checkAndDisablePWM(int servoNum) const;

public:
    explicit ServoFixtureWorker(const Fixture &fixture);

    void SendValues(const uint8_t *data, size_t size) override;

    void setServoPosition(uint8_t channel, uint8_t value) const;


    void tick() override;
};


#endif //MYSTICCREATURESFIGUR_WORKERDUMMY_H
