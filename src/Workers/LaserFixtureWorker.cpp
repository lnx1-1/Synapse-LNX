//
// Created by lnx on 14.08.2025.
//

#include "LaserFixtureWorker.h"

#include "../Drivers/I2C_Handler.h"
#include "../Drivers/PCA_9685.h"

namespace rampValues {
    //# Phase 1: Schneller Anlauf bis zur Mindestdrehzahl
    int start_pwm = 25; //# 0,75V
    int quick_target = 35; //# 1,25V - schnell erreichen
    int step_size = 3; // # 2% pro Schritt
    int quick_delay = 200 * 1000; //# 100ms zwischen Schritten (schnell!)

    //# Phase 2: Dann langsam zur Zieldrehzahl
    int final_target = 70; //# 1,75V
    int fine_step = 1; // # 1% pro Schritt
    int fine_delay = 500 * 1000; //# 300ms (langsamer)

    bool ramping = false; // # Flag, ob Ramp-Up aktiv ist
    int currentRampingValue = start_pwm; // # Aktueller PWM-Wert für Ramp-Up
    unsigned long lastTime = 0;
}

namespace autoTestValues {
    // Test-Parameter Arrays
    int test_start_values[] = {20, 22, 24, 25};
    int test_quick_targets[] = {33, 35, 36};
    int test_step_sizes[] = {2, 3, 4, 5};
    int test_quick_delays[] = {50 * 1000, 100 * 1000, 200 * 1000}; // in microseconds

    int current_test_index = 0;
    int total_tests = 0;
    bool auto_testing = false;
    unsigned long test_start_time = 0;
    unsigned long test_duration = 2000 * 1000; // 2 seconds in microseconds

    // Test state machine
    enum TestState {
        RUNNING,
        WAIT_AFTER_TEST,
        MOTOR_OFF_RESET
    };

    TestState current_state = RUNNING;
    unsigned long state_change_time = 0;

    // Current test combination
    int test_start_pwm, test_quick_target, test_step_size, test_quick_delay;
}

LaserFixtureWorker::LaserFixtureWorker(const Fixture &fixture) {
    _fixture = fixture;
    Log.verboseln("Setting up Laser Fixture Worker");

    _pca9685 = new PCA_9685(&Wire1, _fixture.i2cAddress);
    _pca9685->initPCA9685(1000); // Initialize the PCA9685 //TODO Set Freq

    _isConnected = I2C_Handler::isDeviceConnected(_fixture.i2cAddress, _fixture.toString());

    // Calculate total number of test combinations
    autoTestValues::total_tests = (sizeof(autoTestValues::test_start_values) / sizeof(int)) *
                                  (sizeof(autoTestValues::test_quick_targets) / sizeof(int)) *
                                  (sizeof(autoTestValues::test_step_sizes) / sizeof(int)) *
                                  (sizeof(autoTestValues::test_quick_delays) / sizeof(int));
}

int convertPWMToPCAValue(double rampingValueInPercent) {
    return static_cast<int>(round((rampingValueInPercent / 100.0) * 4095)); // Convert to PCA9685 value
}

void LaserFixtureWorker::startAutoTest() {
    Log.infoln("=== Starting Automatic Motor Parameter Test ===");
    Log.infoln("Total test combinations: %d", autoTestValues::total_tests);
    Log.infoln("Test sequence: 2s run -> 2s wait -> 2s motor off (reset) -> next test");
    Log.infoln("Total estimated time: %.1f minutes", (autoTestValues::total_tests * 6.0) / 60.0);
    Log.infoln("===============================================");

    autoTestValues::auto_testing = true;
    autoTestValues::current_test_index = 0;
    autoTestValues::current_state = autoTestValues::RUNNING;

    // Start first test
    setupNextTest();
}

void LaserFixtureWorker::setupNextTest() {
    if (autoTestValues::current_test_index >= autoTestValues::total_tests) {
        Log.infoln("=== Auto Test Completed! ===");
        autoTestValues::auto_testing = false;
        return;
    }

    // Calculate indices for current test combination
    int start_array_size = sizeof(autoTestValues::test_start_values) / sizeof(int);
    int target_array_size = sizeof(autoTestValues::test_quick_targets) / sizeof(int);
    int step_array_size = sizeof(autoTestValues::test_step_sizes) / sizeof(int);
    int delay_array_size = sizeof(autoTestValues::test_quick_delays) / sizeof(int);

    int start_idx = autoTestValues::current_test_index / (target_array_size * step_array_size * delay_array_size);
    int target_idx = (autoTestValues::current_test_index / (step_array_size * delay_array_size)) % target_array_size;
    int step_idx = (autoTestValues::current_test_index / delay_array_size) % step_array_size;
    int delay_idx = autoTestValues::current_test_index % delay_array_size;

    // Set current test parameters
    autoTestValues::test_start_pwm = autoTestValues::test_start_values[start_idx];
    autoTestValues::test_quick_target = autoTestValues::test_quick_targets[target_idx];
    autoTestValues::test_step_size = autoTestValues::test_step_sizes[step_idx];
    autoTestValues::test_quick_delay = autoTestValues::test_quick_delays[delay_idx];

    // Override ramp values with test values
    rampValues::start_pwm = autoTestValues::test_start_pwm;
    rampValues::quick_target = autoTestValues::test_quick_target;
    rampValues::step_size = autoTestValues::test_step_size;
    rampValues::quick_delay = autoTestValues::test_quick_delay;
    rampValues::final_target = autoTestValues::test_quick_target + 5; // Slightly higher than quick_target

    Log.infoln("--- Test %d/%d ---", autoTestValues::current_test_index + 1, autoTestValues::total_tests);
    Log.infoln("Start PWM: %d%%, Quick Target: %d%%, Step Size: %d%%, Delay: %d ms",
               autoTestValues::test_start_pwm,
               autoTestValues::test_quick_target,
               autoTestValues::test_step_size,
               autoTestValues::test_quick_delay / 1000);

    // Start ramping with test parameters
    rampValues::ramping = true;
    rampValues::currentRampingValue = rampValues::start_pwm;
    rampValues::lastTime = micros();
    autoTestValues::test_start_time = micros();
}

void LaserFixtureWorker::tick() {
    // Handle auto testing state machine
    if (autoTestValues::auto_testing) {
        unsigned long currentTime = micros();

        switch (autoTestValues::current_state) {
            case autoTestValues::RUNNING:
                // Check if current test duration exceeded
                if ((currentTime - autoTestValues::test_start_time) >= autoTestValues::test_duration) {
                    Log.infoln("Test %d completed. Entering 2s wait phase.", autoTestValues::current_test_index + 1);

                    // Stop ramping but keep motor running
                    rampValues::ramping = false;

                    // Switch to wait state
                    autoTestValues::current_state = autoTestValues::WAIT_AFTER_TEST;
                    autoTestValues::state_change_time = currentTime;
                    return;
                }
                break;

            case autoTestValues::WAIT_AFTER_TEST:
                // Wait 2 seconds after test completion
                if ((currentTime - autoTestValues::state_change_time) >= 2000000) {
                    // 2 seconds
                    Log.infoln("Wait phase completed. Stopping motor for 2s reset phase.");

                    // Turn motor off
                    _pca9685->setPWM(PWM_MOTOR_CHANNEL, 0);

                    // Switch to motor off reset state
                    autoTestValues::current_state = autoTestValues::MOTOR_OFF_RESET;
                    autoTestValues::state_change_time = currentTime;
                    return;
                }
                break;

            case autoTestValues::MOTOR_OFF_RESET:
                // Keep motor off for 2 seconds to reset driver
                if ((currentTime - autoTestValues::state_change_time) >= 2000000) {
                    // 2 seconds
                    Log.infoln("Reset phase completed. Moving to next test.");

                    // Move to next test
                    autoTestValues::current_test_index++;
                    autoTestValues::current_state = autoTestValues::RUNNING;
                    setupNextTest();
                    return;
                }
                break;
        }
    }

    // Normal ramping logic (only when in RUNNING state or not auto testing)
    if (!rampValues::ramping) {
        return;
    }

    unsigned long currentTime = micros();

    if (rampValues::currentRampingValue >= rampValues::final_target) {
        if (!autoTestValues::auto_testing) {
            Log.infoln("Ramp-Up completed. Reached final target: %d. Stopping Ramping", rampValues::final_target);
            rampValues::ramping = false; // Stop ramping
        }
        return;
    } else if (rampValues::currentRampingValue >= rampValues::start_pwm && rampValues::currentRampingValue <
               rampValues::quick_target) {
        // Phase 1: Quick Ramp-Up
        if ((currentTime - rampValues::lastTime) < rampValues::quick_delay) {
            return; // Wait for delay before next ramping step
        }
        _pca9685->setPWM(PWM_MOTOR_CHANNEL, convertPWMToPCAValue(rampValues::currentRampingValue));
        Log.verboseln("Quick Ramp-Up: Set PWM to %d", rampValues::currentRampingValue);
        rampValues::currentRampingValue += rampValues::step_size;
        rampValues::lastTime = currentTime; // Update last time for next tick
    } else if (rampValues::currentRampingValue >= rampValues::quick_target && rampValues::currentRampingValue <
               rampValues::final_target) {
        // Phase 2: Slow Ramp-Up
        if ((currentTime - rampValues::lastTime) < rampValues::fine_delay) {
            return;
        }
        _pca9685->setPWM(PWM_MOTOR_CHANNEL, convertPWMToPCAValue(rampValues::currentRampingValue));
        Log.verboseln("Fine Ramp-Up: Set PWM to %d", rampValues::currentRampingValue);
        rampValues::currentRampingValue += rampValues::fine_step;
        rampValues::lastTime = currentTime; // Update last time for next tick
    }
}

int lastVal = 0;

void LaserFixtureWorker::SendValues(const uint8_t *data, size_t size) {
    if (checkParamsValid(data, &size) && stateValid()) {
        if (size == 0) {
            Log.errorln("Received empty data array. Nothing to send.");
            return;
        }
        if (data[0] == lastVal) {
            Log.verboseln("Received same value as last time: %d. Ignoring.", data[0]);
            return; // Ignore if same value as last time
        } else {
            lastVal = data[0];
        }
        if (data[0] < 25) {
            Log.infoln("Received stop command. Stopping Motor.");
            rampValues::ramping = false; // Stop ramping
            autoTestValues::auto_testing = false; // Stop auto testing
            _pca9685->setPWM(PWM_MOTOR_CHANNEL, 0); // Set PWM to 0
            rampValues::currentRampingValue = rampValues::start_pwm;
        } else if (data[0] == 99) {
            Log.infoln("Starting Auto Test Mode");
            startAutoTest();
        } else if (data[0] > 25) {
            Log.infoln("Starting Ramp-Up");
            rampValues::ramping = true;
        }
    }
}
