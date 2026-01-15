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
#include "I2C_Handler.h"
#include "Workers/LaserFixtureWorker.h"
#include "Workers/RelaisFixtureWorker.h"

ArtnetReceiver artnetRecv;
uint16_t universe1 = Config::Universe; // 0 - 32767 Flache Addressierung

std::list<Idmx_FixtureWorker *> dmxFixtures;


void artnetReceiveCallback(const uint8_t *data, uint16_t size,
                           const ArtDmxMetadata &metadata,
                           const ArtNetRemoteInfo &remote);

void setup() {
    Serial.begin(hw_config::SerialBaud);
    Log.begin(Config::LOG_LEVEL, &Serial, true);

    Config::init();

    delay(200);

    ETH_Connector::InitEth();

    I2C_Handler::initI2C();

    artnetRecv.begin();
    Log.infoln("Art-Net Receiver Initialized");
    artnetRecv.subscribeArtDmxUniverse(universe1, artnetReceiveCallback);

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

    Log.infoln("Setup Done");
}

void loop() {
    for (auto dmx_fixture: dmxFixtures) {
        dmx_fixture->tick();
    }
    artnetRecv.parse(); // Continuously parse incoming Art-Net packets
}


void artnetReceiveCallback(const uint8_t *data, uint16_t size,
                           const ArtDmxMetadata &metadata,
                           const ArtNetRemoteInfo &remote) {
    Log.verboseln("[Artnet] Received data from Universe: %d, Device: %s ", metadata.universe,
                  remote.ip.toString().c_str());
    for (Idmx_FixtureWorker *worker: dmxFixtures) {
        int startIndex = worker->_fixture.dmxAddress - 1;
        // DMX addresses are 1-based, so we add 1 to the fixture's address
        int count = worker->_fixture.channelCount;
        if (startIndex >= 0 && (startIndex + count) < size) {
            worker->SendValues(&(data[startIndex]), worker->_fixture.channelCount);
        } else {
            Log.errorln("Error DMX Addr Values. Bounds not correct");
        }
    }
}
