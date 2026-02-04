//
// Created by lnx on 14.08.2025.
//

#ifndef MYSTICCREATURESFIGUR_LASERFIXTUREWORKER_H
#define MYSTICCREATURESFIGUR_LASERFIXTUREWORKER_H
#include "Idmx_FixtureWorker.h"
#include "../Drivers/PCA_9685.h"
#include "../Config/FixtureSettingsManager.h"

#define PWM_MOTOR_CHANNEL 0

class LaserFixtureWorker : public Idmx_FixtureWorker {
    PCA_9685 *_pca9685;

public:
    /**
     * @brief Constructor for the Laser Fixture Worker.
     * @param fixture The Fixture configuration.
     */
    explicit LaserFixtureWorker(const Fixture &fixture);

    /**
     * @brief Initiates an automated test sequence for the laser fixture.
     */
    void startAutoTest();

    /**
     * @brief Configures the next step in the test sequence.
     */
    void setupNextTest();

    /**
     * @brief Periodically called to handle the laser's internal logic and testing.
     */
    void tick() override;

    /**
     * @brief Processes DMX data to control laser power or motor speed.
     * @param data Pointer to the buffer containing control values.
     * @param size Number of channels.
     */
    void SendValues(const uint8_t *data, size_t size) override;
};


#endif //MYSTICCREATURESFIGUR_LASERFIXTUREWORKER_H
