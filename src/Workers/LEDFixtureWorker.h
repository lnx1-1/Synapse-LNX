#ifndef LED_FIXTURE_WORKER_H
#define LED_FIXTURE_WORKER_H

#include "../Idmx_FixtureWorker.h"
#include "../Drivers/WS28xxDriver.h"

class LEDFixtureWorker : public Idmx_FixtureWorker {
    WS28xxDriver *_driver = nullptr;

    struct Settings {
        uint16_t ledCount = 60;
        uint16_t dataPin = 2;
        String colorOrder = "GRB";
    } _settings;

public:
    explicit LEDFixtureWorker(const Fixture &fixture);

    ~LEDFixtureWorker() override;

    void tick() override;

    void SendValues(const uint8_t *data, size_t size) override;

    void onSettingsChanged(const String &key) override;

private:
    void configureSettings();

    void setupDriver();
};

#endif // LED_FIXTURE_WORKER_H
