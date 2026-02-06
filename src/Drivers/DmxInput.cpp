//
// Created by Codex on 04.02.2026.
//

#include "DmxInput.h"
#include <ArduinoLog.h>
#include "Config/hw_config.h"

#include <esp_dmx.h>
#include <cstring>
#include <freertos/FreeRTOS.h>
#include <freertos/portmacro.h>

namespace {
    constexpr dmx_port_t kDmxPort = DMX_NUM_1;
    constexpr size_t kMaxFrameSize = 513;

    bool initialized = false;
    volatile bool enabled = true;

    dmx_config_t dmxConfig = DMX_CONFIG_DEFAULT;
    dmx_personality_t personalities[] = {
        {1, "Default"}
    };

    uint8_t lastFrame[512];
    size_t lastFrameSize = 0;
    volatile bool frameReady = false;
    unsigned long lastPacketTime = 0;
    portMUX_TYPE frameMux = portMUX_INITIALIZER_UNLOCKED;

    bool pollForFrame() {
        dmx_packet_t packet = {};
        const size_t received = dmx_receive(kDmxPort, &packet, 0);
        if (received == 0) {
            return false;
        }
        if (packet.err != DMX_OK || packet.size < 2) {
            return false;
        }

        uint8_t raw[kMaxFrameSize];
        const size_t copySize = packet.size < kMaxFrameSize ? packet.size : kMaxFrameSize;
        dmx_read(kDmxPort, raw, copySize);

        if (packet.sc != DMX_SC || raw[0] != DMX_SC) {
            return false;
        }

        const size_t channelCount = (copySize - 1) > sizeof(lastFrame) ? sizeof(lastFrame) : (copySize - 1);
        portENTER_CRITICAL(&frameMux);
        std::memcpy(lastFrame, &raw[1], channelCount);
        lastFrameSize = channelCount;
        frameReady = true;
        lastPacketTime = millis();
        portEXIT_CRITICAL(&frameMux);
        return true;
    }
}

void DmxInput::init() {
    if (initialized) {
        return;
    }

    dmx_driver_install(kDmxPort, &dmxConfig, personalities, 1);
    dmx_set_pin(kDmxPort, hw_config::DMX_TX_Pin, hw_config::DMX_RX_Pin, hw_config::DMX_RTS_Pin);

    initialized = true;

    Log.infoln("[DMX] Input Handler initialized (esp_dmx, RX=%u, TX=%u, RTS=%u)",
               hw_config::DMX_RX_Pin, hw_config::DMX_TX_Pin, hw_config::DMX_RTS_Pin);
}

void DmxInput::setEnabled(bool enable) {
    enabled = enable;
}

bool DmxInput::isEnabled() {
    return enabled;
}

bool DmxInput::readFrame(uint8_t *out, size_t outSize, size_t &frameSize) {
    frameSize = 0;
    if (!enabled || !out || outSize == 0) {
        return false;
    }

    pollForFrame();

    bool hasFrame = false;
    portENTER_CRITICAL(&frameMux);
    if (frameReady && lastFrameSize > 0) {
        const size_t copySize = lastFrameSize < outSize ? lastFrameSize : outSize;
        std::memcpy(out, lastFrame, copySize);
        frameSize = copySize;
        frameReady = false;
        hasFrame = true;
    }
    portEXIT_CRITICAL(&frameMux);
    return hasFrame;
}

size_t DmxInput::copyLastFrame(uint8_t *out, size_t outSize) {
    if (!out || outSize == 0) {
        return 0;
    }
    size_t copySize = 0;
    portENTER_CRITICAL(&frameMux);
    copySize = lastFrameSize < outSize ? lastFrameSize : outSize;
    if (copySize > 0) {
        std::memcpy(out, lastFrame, copySize);
    }
    portEXIT_CRITICAL(&frameMux);
    return copySize;
}

bool DmxInput::isReceiving(uint32_t timeoutMs) {
    if (!enabled) {
        return false;
    }
    const unsigned long now = millis();
    if (lastPacketTime == 0) {
        return false;
    }
    return (now - lastPacketTime) < timeoutMs;
}

unsigned long DmxInput::lastPacketAgeMs() {
    if (lastPacketTime == 0) {
        return 0;
    }
    return millis() - lastPacketTime;
}
