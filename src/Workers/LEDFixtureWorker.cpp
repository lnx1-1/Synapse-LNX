#include "LEDFixtureWorker.h"
#include "../Config/FixtureSettingsManager.h"

static ColorOrder stringToColorOrder(const String &order) {
    String upperOrder = order;
    upperOrder.toUpperCase();
    if (upperOrder == "RGB") return ColorOrder::RGB;
    if (upperOrder == "GRB") return ColorOrder::GRB;
    if (upperOrder == "BRG") return ColorOrder::BRG;
    if (upperOrder == "RBG") return ColorOrder::RBG;
    if (upperOrder == "GBR") return ColorOrder::GBR;
    if (upperOrder == "BGR") return ColorOrder::BGR;
    return ColorOrder::GRB; // Default
}

LEDFixtureWorker::LEDFixtureWorker(const Fixture &fixture) : Idmx_FixtureWorker(fixture) {
    configureSettings();
    setupDriver();
    _isConnected = true;
}

LEDFixtureWorker::~LEDFixtureWorker() {
    delete _driver;
}

void LEDFixtureWorker::configureSettings() {
    settingsManager->addSetting("led_count", "LED Count", &_settings.ledCount, "number");
    settingsManager->addSetting("data_pin", "Data Pin", &_settings.dataPin, "number");

    std::vector<String> orders = {"RGB", "GRB", "BRG", "RBG", "GBR", "BGR"};
    settingsManager->addSetting("color_order", "Color Order", &_settings.colorOrder, orders);

    settingsManager->load();
}

void LEDFixtureWorker::setupDriver() {
    Log.verboseln("[%s] Setting up LED Driver on Pin %d with %d LEDs (Order: %s)", _fixture.toString(),
                  _settings.dataPin,
                  _settings.ledCount, _settings.colorOrder.c_str());
    //Newly Setup Driver if already initialized
    delete _driver;

    _driver = WS28xxDriver::create(_settings.dataPin, _settings.ledCount, stringToColorOrder(_settings.colorOrder));
}

void LEDFixtureWorker::tick() {
    // Background tasks if any
}

void LEDFixtureWorker::SendValues(const uint8_t *data, size_t size) {
    if (checkParamsValid(data, &size) && stateValid()) {
        _driver->show(data, size);
    }
}

void LEDFixtureWorker::onSettingsChanged(const String &key) {
    if (key == "led_count" || key == "data_pin" || key == "color_order") {
        Log.infoln("LED Settings changed [%s], reconfiguring driver...", key.c_str());
        setupDriver();
    }
}
