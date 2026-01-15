//
// Created by lnx on 14.08.2025.
//

#ifndef MYSTICCREATURESFIGUR_LASERFIXTUREWORKER_H
#define MYSTICCREATURESFIGUR_LASERFIXTUREWORKER_H
#include "Idmx_FixtureWorker.h"
#include "PCA_9685.h"

#define PWM_MOTOR_CHANNEL 0

class LaserFixtureWorker : public Idmx_FixtureWorker {
    PCA_9685 *_pca9685;

public:
    explicit LaserFixtureWorker(const Fixture &fixture);

    void startAutoTest();

    void setupNextTest();

    void tick() override;

    void SendValues(const uint8_t *data, size_t size) override;
};


#endif //MYSTICCREATURESFIGUR_LASERFIXTUREWORKER_H
