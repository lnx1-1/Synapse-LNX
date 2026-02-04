#include "Idmx_FixtureWorker.h"
#include "Config/FixtureSettingsManager.h"

Idmx_FixtureWorker::Idmx_FixtureWorker(const Fixture &fixture) : _fixture(fixture) {
    String namespaceName = "fix_" + String(fixture.toString()) + "_" + String(fixture.i2cAddress);
    settingsManager = new FixtureSettingsManager(namespaceName);
    settingsManager->addSetting("dmx_addr", "DMX Address", &_fixture.dmxAddress, "number");
}

Idmx_FixtureWorker::~Idmx_FixtureWorker() {
    delete settingsManager;
}

std::vector<FixtureSetting> Idmx_FixtureWorker::getSettings() {
    if (settingsManager) {
        return settingsManager->getSettingsMetadata();
    }
    return {};
}


void Idmx_FixtureWorker::applySetting(String key, String value) {
    if (settingsManager) {
        if (settingsManager->applySetting(key, value)) {
            settingsManager->save();
            onSettingsChanged(key);
        }
    }
}
