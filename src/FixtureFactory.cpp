//
// Created by lnx on 19.08.2025.
//

#include "FixtureFactory.h"


Idmx_FixtureWorker *FixtureFactory::getNewFixture(const Fixture &fixture) {
    Idmx_FixtureWorker *returnVal = nullptr;
    switch (fixture.id) {
        case FixtureID::LaserModule:
            returnVal = new LaserFixtureWorker(fixture);
            break;
        case FixtureID::StepperModule:
            returnVal = new StepperFixtureWorker(fixture);
            break;
        case FixtureID::LightModule:
            Log.errorln("FixtureFactory: Light Module Not implemented yet!");
            break;
        case FixtureID::Dummy:
            Log.errorln("FixtureFactory: Not implemented yet!");
            break;
        case FixtureID::RelaisModule:
            returnVal = new RelaisFixtureWorker(fixture);
            break;
        default:
            Log.errorln("FixtureFactory: Not implemented yet!");
    }
    return returnVal;
}
