#include "Idmx_FixtureWorker.h"
#include "WorkerStub.h"
#include <Arduino.h>
#include <ArtnetETH.h>
#include <list>
#include <algorithm>
#include <Wire.h>
#include <ArduinoLog.h>
#include "Config/config.h"
#include <WiFi.h>

#include "FixtureFactory.h"
#include "Network/ETH_Connector.h"
#include "Network/CaptivePortal.h"
#include "Network/WebInterface.h"
#include "Drivers/I2C_Handler.h"
#include "Drivers/DmxInput.h"

#include "Util.h"

ArtnetETHReceiver artnetRecv;

unsigned long lastArtnetPacketTime = 0;
std::list<Idmx_FixtureWorker *> dmxFixtures;

namespace {
    constexpr size_t kDmxMaxSlotsWithStartCode = 513;

    size_t computeRequiredDmxReceiveSlots() {
        size_t maxChannel = 0;
        for (Idmx_FixtureWorker *worker: dmxFixtures) {
            if (worker == nullptr) {
                continue;
            }
            const uint16_t address = worker->_fixture.dmxAddress;
            const uint8_t channelCount = worker->_fixture.channelCount;
            if (address == 0 || channelCount == 0) {
                continue;
            }
            const size_t endChannel = static_cast<size_t>(address) + static_cast<size_t>(channelCount) - 1U;
            maxChannel = std::max(maxChannel, endChannel);
        }

        if (maxChannel == 0) {
            return kDmxMaxSlotsWithStartCode;
        }

        const size_t requiredSlots = maxChannel + 1U; // Include start code slot.
        if (requiredSlots > kDmxMaxSlotsWithStartCode) {
            Log.warningln("[DMX] Required slots %u exceed DMX max %u. Clamping.",
                          static_cast<unsigned>(requiredSlots),
                          static_cast<unsigned>(kDmxMaxSlotsWithStartCode));
        }
        return requiredSlots > kDmxMaxSlotsWithStartCode ? kDmxMaxSlotsWithStartCode : requiredSlots;
    }

    void updateDmxReceiveSlotsFromFixtures() {
        const size_t requiredSlots = computeRequiredDmxReceiveSlots();
        DmxInput::setReceiveSlotCount(requiredSlots);
    }
}

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

    updateDmxReceiveSlotsFromFixtures();
    DmxInput::setEnabled(Config::InputMode == ConfigDefaults::InputMode::DMX);
    CaptivePortal::begin(Config::CaptiveGraceMs, Config::CaptiveDurationMs);
}

void setup() {
    Serial.begin(hw_config::SerialBaud);

    Config::init();

    Log.begin(Config::LOG_LEVEL, &logRedirector, true);
    Log.infoln("------ Starting SYNAPSE LNX Node ------");
    Log.infoln("------ Log Level: %s ------", Util::logLevelToString(Config::LOG_LEVEL).c_str());

    delay(200); // Wait for HW init

    ETH_Connector::InitEth();

    WebInterface::init();
    CaptivePortal::begin(Config::CaptiveGraceMs, Config::CaptiveDurationMs);

    if (hw_config::I2C_Enable) {
        I2C_Handler::initI2C(false);
    } else {
        Log.infoln("[I2C] Disabled in hw_config (enable and adjust pins when needed).");
    }

    DmxInput::init();
    DmxInput::setEnabled(Config::InputMode == ConfigDefaults::InputMode::DMX);
    DmxInput::setFrameCallback(processDmxData);

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

    updateDmxReceiveSlotsFromFixtures();
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
    CaptivePortal::update();
    for (auto dmx_fixture: dmxFixtures) {
        dmx_fixture->tick();
    }

    if (Config::InputMode == ConfigDefaults::InputMode::DMX) {
        static bool dmxFallbackActive = false;
        DmxInput::handle();

        const uint32_t kDmxFallbackTimeoutMs = Config::DmxBlackoutTimeoutMs;
        if (DmxInput::isReceiving(kDmxFallbackTimeoutMs)) {
            dmxFallbackActive = false;
        } else {
            if (!dmxFallbackActive) {
                uint8_t blackout[512] = {};
                processDmxData(blackout, sizeof(blackout));
                dmxFallbackActive = true;
                Log.warningln("[DMX] Fallback active: blackout (no data for %lums)", kDmxFallbackTimeoutMs);
            }
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
    if (data == nullptr || size == 0) {
        return;
    }

    static unsigned long lastBoundsWarnMs = 0;
    const size_t frameSize = size;

    for (Idmx_FixtureWorker *worker: dmxFixtures) {
        if (worker->_fixture.dmxAddress == 0) {
            Log.errorln("[%s] Invalid DMX address 0", worker->_fixture.toString());
            continue;
        }

        // DMX addresses are 1-based.
        const size_t startIndex = static_cast<size_t>(worker->_fixture.dmxAddress - 1U);
        if (startIndex >= frameSize) {
            const unsigned long now = millis();
            if ((now - lastBoundsWarnMs) > 2000) {
                Log.warningln("[%s] DMX address %u outside received framesize %u (ignoring update)",
                              worker->_fixture.toString(),
                              static_cast<unsigned>(worker->_fixture.dmxAddress),
                              static_cast<unsigned>(frameSize));
                lastBoundsWarnMs = now;
            }
            continue;
        }

        const size_t availableChannels = frameSize - startIndex;
        const size_t count = std::min<size_t>(worker->_fixture.channelCount, availableChannels);
        if (count == 0) {
            continue;
        }

        worker->SendValues(&(data[startIndex]), count);
    }
}
