//
// Created by lnx on 19.08.2025.
//

#ifndef MYSTICCREATURESFIGUR_FIXTUREFACTORY_H
#define MYSTICCREATURESFIGUR_FIXTUREFACTORY_H

#include "Idmx_FixtureWorker.h"
#include "Workers/LaserFixtureWorker.h"
#include "Workers/RelaisFixtureWorker.h"
#include "Workers/ServoFixtureWorker.h"

class FixtureFactory {
public:
    static Idmx_FixtureWorker *getNewFixture(const Fixture &fixture);
};


#endif //MYSTICCREATURESFIGUR_FIXTUREFACTORY_H
