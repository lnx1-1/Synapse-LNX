//
// Created by lnx on 28.01.2026.
//

#include "StepperFixtureWorker.h"
#include "FastAccelStepper.h"

void StepperFixtureWorker::configureSettings() {
    settingsManager->addSetting("max_speed", "Max Speed (Steps/s)", &_settings.maxSpeed, "number");
    settingsManager->addSetting("accel", "Acceleration (Steps/s^2)", &_settings.acceleration, "number");
    settingsManager->addSetting("pulses", "Pulses Per Revolution", &_settings.pulsesPerRevolution, "number");
    settingsManager->load();
}

float StepperFixtureWorker::calculateRPM(float stepsPerSecond) const {
    return (stepsPerSecond / _settings.pulsesPerRevolution) * 60.0f;
}

StepperFixtureWorker::StepperFixtureWorker(const Fixture &fixture) : Idmx_FixtureWorker(fixture) {
    configureSettings();

    Log.verboseln("Setting up Stepper Fixture Worker on Step: %d Dir: %d using RMT", PIN_STEP,
                  PIN_DIRECTION);
    _engine = FastAccelStepperEngine();
    _engine.init();

    _stepper = _engine.stepperConnectToPin(PIN_STEP);
    // pinMode(PIN_STEP, OPEN_DRAIN);
    if (_stepper) {
        pinMode(PIN_ENABLE, OUTPUT);
        setDriverEnabled(false);
        _stepper->setDirectionPin(PIN_DIRECTION);
        _stepper->setSpeedInUs(1000000 / _settings.maxSpeed); // Initial speed
        _stepper->setAcceleration(static_cast<int32_t>(_settings.acceleration));
    }
    _isConnected = true; // Stepper on pins is always "connected" in this context
}


void StepperFixtureWorker::tick() {
    if (0 == _stepper->getCurrentSpeedInMilliHz(true)) {
        setDriverEnabled(false);
    }
}

void StepperFixtureWorker::SendValues(const uint8_t *data, size_t size) {
    if (checkParamsValid(data, &size) && stateValid()) {
        if (size >= 2) {
            const float speedFactor = static_cast<float>(_settings.maxSpeed) / 255.0;
            const float scaledSpeed = speedFactor * data[0];

            auto DMX_SpeedVal = data[0];
            auto DMX_DirVal = data[1];


            if (_last_DMXVals == std::pair{data[0], data[1]}) return;


            if (DMX_SpeedVal == 0) {
                _stepper->stopMove();
                Log.traceln("Zero Speed - Going to Stop");
            } else {
                setDriverEnabled(true);
                auto delayInUs = static_cast<uint32_t>(1000000.0f / scaledSpeed);
                _stepper->setSpeedInUs(delayInUs);

                float rpm = calculateRPM(scaledSpeed);

                if (DMX_DirVal >= 128) {
                    Log.traceln("Mov Right Speed [%F] RPM [%F]", scaledSpeed, rpm);
                    _stepper->runForward();
                } else {
                    Log.traceln("Mov Left Speed [%F] RPM [%F]", scaledSpeed, rpm);
                    _stepper->runBackward();
                }
            }
            _last_DMXVals = std::pair{data[0], data[1]};
        }
    }
}

void StepperFixtureWorker::onSettingsChanged(const String &key) {
    if (key == "accel" && _stepper) {
        _stepper->setAcceleration(static_cast<int32_t>(_settings.acceleration));
    }
}

void StepperFixtureWorker::setDriverEnabled(bool enabled) {
    if (_driverEnabled == enabled) {
        return;
    }
    _driverEnabled = enabled;
    if (PIN_ENABLE_ACTIVE_LOW) {
        digitalWrite(PIN_ENABLE, enabled ? LOW : HIGH);
        Log.noticeln("Motor Driver %s", enabled ? "Enabled" : "Disabled");
    } else {
        digitalWrite(PIN_ENABLE, enabled ? HIGH : LOW);
    }
}
