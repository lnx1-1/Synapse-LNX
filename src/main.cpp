#include "Idmx_FixtureWorker.h"
#include "WorkerStub.h"
#include <Arduino.h>
#include <ArtnetETH.h>
#include <list>
#include <Wire.h>
#include <ArduinoLog.h>
#include "Config/config.h"
#include "Workers/ServoFixtureWorker.h"
#include <WiFi.h>

#include "FixtureFactory.h"
#include "Network/ETH_Connector.h"
#include "Network/WebInterface.h"
#include "Drivers/I2C_Handler.h"
#include "Workers/LaserFixtureWorker.h"
#include "Workers/RelaisFixtureWorker.h"

ArtnetETHReceiver artnetRecv;
unsigned long lastArtnetPacketTime = 0;

std::list<Idmx_FixtureWorker *> dmxFixtures;

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

void reinitModules() {
    Log.infoln("Re-initializing modules...");
    ETH_Connector::InitEth();
    artnetRecv.unsubscribeArtDmxUniverses();
    Log.infoln("[ArtNet] Unsubscribed from Universes");

    for (auto worker: dmxFixtures) {
        worker->stop();
    }

    artnetRecv.subscribeArtDmxUniverse(Config::Universe, artnetReceiveCallback);
    Log.infoln("[ArtNet] Re-Subscribed to Universe %d", Config::Universe);
    Log.begin(Config::LOG_LEVEL, &logRedirector, true);
}

void setup() {
    Serial.begin(hw_config::SerialBaud);

    Config::init();

    Log.begin(Config::LOG_LEVEL, &logRedirector, true);

    delay(200);

    ETH_Connector::InitEth();

    WebInterface::init();

    I2C_Handler::initI2C(false);

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

void loop() {
    WebInterface::handle();
    for (auto dmx_fixture: dmxFixtures) {
        dmx_fixture->tick();
    }
    artnetRecv.parse(); // Continuously parse incoming Art-Net packets
}

void printDMXData(const uint8_t *data, uint16_t size) {
    Log.verboseln("DMX Data: |");
    for (uint16_t i = 0; i < size; ++i) {
        Log.verbose("%d", data[i]);
        if (i < size - 1) {
            Log.verbose("|");
        }
    }
    Log.verboseln("|");
}

void artnetReceiveCallback(const uint8_t *data, uint16_t size,
                           const ArtDmxMetadata &metadata,
                           const ArtNetRemoteInfo &remote) {
    lastArtnetPacketTime = millis();
    Log.traceln("[Artnet] Received data from Universe: %d, Device: %s ", metadata.universe,
                remote.ip.toString().c_str());
    for (Idmx_FixtureWorker *worker: dmxFixtures) {
        int startIndex = worker->_fixture.dmxAddress - 1;
        // DMX addresses are 1-based, so we add 1 to the fixture's address
        int count = worker->_fixture.channelCount;
        if (startIndex >= 0 && (startIndex + count) < size) {
            // printDMXData(&data[startIndex], worker->_fixture.channelCount);
            worker->SendValues(&(data[startIndex]), worker->_fixture.channelCount);
        } else {
            Log.errorln("Error DMX Addr Values. Bounds not correct");
        }
    }
}


