//
// Created by lnx on 13.08.2025.
//

#include "ServoFixtureWorker.h"

#include <Wire.h>

#include "ArduinoLog.h"
#include "../Drivers/I2C_Handler.h"

#define HOLD_TIME_MS 700
#define numberOfServos 2


struct servoState {
    unsigned long lastMoveTime = 0; // in Millis
    uint8_t lastValue = 0;
    bool pwm_enabled = false;
};

servoState servos[numberOfServos];


ServoFixtureWorker::ServoFixtureWorker(const Fixture &fixture) : Idmx_FixtureWorker() {
    _pca9685 = new PCA_9685(&Wire1, fixture.i2cAddress);
    _fixture = fixture;
    Log.verboseln("Setting up PWM Controller");

    _pca9685->initPCA9685(SERVO_UPDATE_FREQUENCY); // Initialize the PCA9685
    for (int i = 0; i < numberOfServos; ++i) {
        servos[i].lastMoveTime = 0;
        servos[i].lastValue = 0;
        servos[i].pwm_enabled = false;
    }

    _isConnected = I2C_Handler::isDeviceConnected(_fixture.i2cAddress, _fixture.toString());
}

void ServoFixtureWorker::SendValues(const uint8_t *data, size_t size) {
    if (checkParamsValid(data, &size) && stateValid()) {
        for (size_t i = 0; i < min(static_cast<int>(size),numberOfServos); ++i) {
            if (data[i] == servos[i].lastValue) {
                continue; // Ignore if same value as last time
            }
            servos[i].lastValue = data[i];
            setServoPosition(i, data[i]);
        }
    }
}

void ServoFixtureWorker::setServoPosition(uint8_t channel, uint8_t value) const {
    Log.verboseln("Setting Servo %d Position at %d", channel, value);
    // Wert von 0-255 auf Servo-Pulsbreite mappen
    uint16_t pulseWidth = map(value, 0, 255, SERVO_MIN_PULSE, SERVO_MAX_PULSE);

    _pca9685->setPWM(channel, pulseWidth);
    servos[channel].pwm_enabled = true;
    servos[channel].lastMoveTime = millis();
    // Log.verboseln("Kanal %d: Wert=%d -> Puls=%d", channel, value, pulseWidth);
}


void ServoFixtureWorker::tick() {
    for (int i = 0; i < numberOfServos; ++i) {
        checkAndDisablePWM(i);
    }
}

void ServoFixtureWorker::checkAndDisablePWM(int servoNum) const {
    if (!servos[servoNum].pwm_enabled) {
        return;
    }

    if (millis() - servos[servoNum].lastMoveTime > HOLD_TIME_MS) {
        _pca9685->disablePWM(servoNum);
        Log.verboseln("Disabled PWM for Servo %d", servoNum);
        servos[servoNum].pwm_enabled = false;
    }
}
