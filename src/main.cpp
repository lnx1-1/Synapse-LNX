#include "Idmx_FixtureWorker.h"
#include "WorkerStub.h"
#include <Arduino.h>
#include <ArtnetETH.h>
#include <list>
#include <Wire.h>
#include <ArduinoLog.h>
#include "Config/config.h"
#include <WiFi.h>

#include "FixtureFactory.h"
#include "Network/ETH_Connector.h"
#include "Network/WebInterface.h"
#include "Drivers/I2C_Handler.h"
#include "Drivers/DmxInput.h"
#include <cstring>

#include "Util.h"

ArtnetETHReceiver artnetRecv;

unsigned long lastArtnetPacketTime = 0;
std::list<Idmx_FixtureWorker *> dmxFixtures;
uint8_t dmxInputFrame[512];

class LogRedirector : public Print {
public:
    size_t write(uint8_t c) override {
        Serial.write(c);
        WebInterface::webLog.write(c);
        return 1;
    }
};

LogRedirector logRedirector;

void artnetReceiveCallback(const uint8_t *data, uint16_t size,
                           const ArtDmxMetadata &metadata,
                           const ArtNetRemoteInfo &remote);

void processDmxData(const uint8_t *data, uint16_t size);

void reinitModules() {
    Log.infoln(" -----   Re-initializing modules... -----");
    ETH_Connector::InitEth();
    artnetRecv.unsubscribeArtDmxUniverses();
    Log.infoln("[ArtNet] Unsubscribed from Universes");

    for (auto worker: dmxFixtures) {
        worker->stop();
    }

    artnetRecv.subscribeArtDmxUniverse(Config::Universe, artnetReceiveCallback);
    Log.infoln("[ArtNet] Re-Subscribed to Universe %d", Config::Universe);
    Log.begin(Config::LOG_LEVEL, &logRedirector, true);

    DmxInput::setEnabled(Config::InputMode == ConfigDefaults::InputMode::DMX);
}

void setup() {
    Serial.begin(hw_config::SerialBaud);

    Config::init();

    Log.begin(Config::LOG_LEVEL, &logRedirector, true);
    Log.infoln("------ Starting Kaleo Node ------");
    Log.infoln("------ Log Level: %s ------", Util::logLevelToString(Config::LOG_LEVEL).c_str());

    delay(200); // Wait for HW init

    ETH_Connector::InitEth();

    WebInterface::init();

    I2C_Handler::initI2C(false);

    DmxInput::init();
    DmxInput::setEnabled(Config::InputMode == ConfigDefaults::InputMode::DMX);

    artnetRecv.begin();
    artnetRecv.subscribeArtDmxUniverse(Config::Universe, artnetReceiveCallback);
    Log.infoln("[ArtNet] Receiver Initialized. Subscribed to Universe %d", Config::Universe);

    // Init Fixtures
    Log.infoln("------ Initializing Fixtures...");
    auto fixtures = getFixtures();
    for (const auto &fixture: fixtures) {
        // DMX_FixtureWorker *fix = new WorkerStub(fixture, i2cWire);

        Idmx_FixtureWorker *fix = FixtureFactory::getNewFixture(fixture);

        if (fix->isConnected()) {
            Log.infoln("[%s] initialized successfully.", fix->_fixture.toString());
            dmxFixtures.push_back(fix);
        } else {
            Log.errorln("Failed to initialize Fixture %s", fix->_fixture.toString());
        }
    }

    Log.infoln("------ Setup Done ------");
}

void checkTickTime() {
    static unsigned long lastLoopMs = 0;
    static unsigned long lastSlowLoopWarnMs = 0;
    const unsigned long now = millis();

    if (lastLoopMs != 0) {
        const unsigned long loopDelta = now - lastLoopMs;
        if (Config::InputMode == ConfigDefaults::InputMode::DMX) {
            constexpr unsigned long kSlowLoopThresholdMs = 40;
            constexpr unsigned long kWarnIntervalMs = 2000;
            if (loopDelta > kSlowLoopThresholdMs && (now - lastSlowLoopWarnMs) > kWarnIntervalMs) {
                Log.warningln("[DMX] Loop slow: %lums (threshold %lums). DMX may stutter.",
                              loopDelta, kSlowLoopThresholdMs);
                lastSlowLoopWarnMs = now;
            }
        }
    }
    lastLoopMs = now;
}

void loop() {
    checkTickTime();

    WebInterface::handle();
    for (auto dmx_fixture: dmxFixtures) {
        dmx_fixture->tick();
    }

    if (Config::InputMode == ConfigDefaults::InputMode::DMX) {
        size_t frameSize = 0;
        if (DmxInput::readFrame(dmxInputFrame, sizeof(dmxInputFrame), frameSize)) {
            processDmxData(dmxInputFrame, static_cast<uint16_t>(frameSize));
        }
    }

    artnetRecv.parse(); // Continuously parse incoming Art-Net packets
}


void artnetReceiveCallback(const uint8_t *data, uint16_t size,
                           const ArtDmxMetadata &metadata,
                           const ArtNetRemoteInfo &remote) {
    if (Config::InputMode != ConfigDefaults::InputMode::ArtNet) {
        return;
    }
    lastArtnetPacketTime = millis();
    Log.verboseln("[Artnet] Received data from Universe: %d, Device: %s ", metadata.universe,
                  remote.ip.toString().c_str());
    processDmxData(data, size);
}

void processDmxData(const uint8_t *data, uint16_t size) {
    for (Idmx_FixtureWorker *worker: dmxFixtures) {
        int startIndex = worker->_fixture.dmxAddress - 1;
        // DMX addresses are 1-based, so we add 1 to the fixture's address
        auto count = worker->_fixture.channelCount;
        count = std::min<uint16_t>(count, size);
        if (startIndex >= 0) {
            worker->SendValues(&(data[startIndex]), count);
        } else {
            Log.errorln("Error DMX Addr Values. Bounds not correct");
        }
    }
}


