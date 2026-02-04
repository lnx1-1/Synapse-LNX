//
// Created by lnx on 19.08.2025.
//

#ifndef MYSTICCREATURESFIGUR_FIXTUREFACTORY_H
#define MYSTICCREATURESFIGUR_FIXTUREFACTORY_H

#include "Idmx_FixtureWorker.h"
#include "Workers/LaserFixtureWorker.h"
#include "Workers/RelaisFixtureWorker.h"
#include "Workers/ServoFixtureWorker.h"
#include "Workers/StepperFixtureWorker.h"

class FixtureFactory {
public:
    /**
     * @brief Factory method to create a new FixtureWorker based on the fixture's type.
     * @param fixture The Fixture configuration.
     * @return A pointer to the created Idmx_FixtureWorker, or nullptr if the type is unknown.
     */
    static Idmx_FixtureWorker *getNewFixture(const Fixture &fixture);
};


#endif //MYSTICCREATURESFIGUR_FIXTUREFACTORY_H
