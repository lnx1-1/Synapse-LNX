//
// Created by lnx on 28.01.2026.
//

#ifndef KALEO_MODULARARTNETNODE_STEPPERFIXTUREWORKER_H
#define KALEO_MODULARARTNETNODE_STEPPERFIXTUREWORKER_H
#include "FastAccelStepper.h"
#include "Idmx_FixtureWorker.h"
#include "../Config/FixtureSettingsManager.h"

#define PIN_STEP 33
#define PIN_DIRECTION 5

class StepperFixtureWorker : public Idmx_FixtureWorker {
    FastAccelStepperEngine _engine{};
    FastAccelStepper *_stepper{};

    struct Settings {
        uint32_t maxSpeed = 1000;
        uint32_t acceleration = 500;
        uint32_t pulsesPerRevolution = 1600;
    } _settings;

public:
    /**
     * @brief Configures the stepper engine and motor parameters based on internal settings.
     */
    void configureSettings();

    /**
     * @brief Constructor for the Stepper Fixture Worker.
     * @param fixture The Fixture configuration.
     */
    explicit StepperFixtureWorker(const Fixture &fixture);

    /**
     * @brief Periodically called to handle stepper motion and non-blocking updates.
     */
    void tick() override;

    /**
     * @brief Processes DMX data to control stepper position, speed, or other parameters.
     * @param data Pointer to the buffer containing control values.
     * @param size Number of channels.
     */
    void SendValues(const uint8_t *data, size_t size) override;

    /**
     * @brief Callback invoked when a stepper-specific setting has been changed.
     * Reconfigures the stepper if necessary.
     * @param key The key of the setting that changed.
     */
    void onSettingsChanged(const String &key) override;
};
#endif //KALEO_MODULARARTNETNODE_STEPPERFIXTUREWORKER_H
