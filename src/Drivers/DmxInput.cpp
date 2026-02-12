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
#include <freertos/task.h>

namespace {
    constexpr dmx_port_t kDmxPort = DMX_NUM_1;
    constexpr size_t kMaxFrameSize = 513;
    constexpr TickType_t kReceiveWaitTicks = DMX_TIMEOUT_TICK;
    constexpr uint32_t kReceiverTaskStackWords = 4096;
    constexpr UBaseType_t kReceiverTaskPriority = 1;

    bool initialized = false;
    volatile bool enabled = true;

    dmx_config_t dmxConfig = DMX_CONFIG_DEFAULT;
    dmx_personality_t personalities[] = {
        {1, "Default"}
    };

    uint8_t lastFrame[512];
    uint8_t rawFrame[kMaxFrameSize];
    uint8_t callbackFrame[512];
    size_t lastFrameSize = 0;
    size_t receiveSlotCount = kMaxFrameSize;
    volatile bool frameReady = false;
    unsigned long lastPacketTime = 0;
    portMUX_TYPE frameMux = portMUX_INITIALIZER_UNLOCKED;
    TaskHandle_t receiverTaskHandle = nullptr;
    DmxInput::FrameCallback frameCallback = nullptr;

    size_t clampReceiveSlots(size_t requested) {
        if (requested < 2) {
            return 2;
        }
        return requested > kMaxFrameSize ? kMaxFrameSize : requested;
    }

    /**
     * Async Task. Blocks on Receiving and Copies received bytes into buffer
     * Handles the reception and processing of DMX packets continuously. The task checks if the DMX driver is enabled
     * and installed, reads incoming DMX packets, validates them, and processes the data to update the last received
     * frame if a valid packet is received. The function logs errors periodically if packet reception fails.
     *
     * @param pvParameters Unused parameter, required for task function signature.
     */
    void dmxReceiverTask(void * /*pvParameters*/) {
        unsigned long lastErrorLogMs = 0;
        while (true) {
            if (!enabled || !dmx_driver_is_installed(kDmxPort) || !dmx_driver_is_enabled(kDmxPort)) {
                vTaskDelay(pdMS_TO_TICKS(20));
                continue;
            }

            dmx_packet_t packet = {};
            size_t expectedSlots = kMaxFrameSize;
            portENTER_CRITICAL(&frameMux);
            expectedSlots = receiveSlotCount;
            portEXIT_CRITICAL(&frameMux);

            const size_t receivedSize = dmx_receive_num(kDmxPort, &packet, expectedSlots, kReceiveWaitTicks);
            if (receivedSize == 0 || !enabled) {
                continue;
            }
            if (packet.size < 2) {
                continue;
            }

            if (packet.err != DMX_OK && packet.err != DMX_ERR_NOT_ENOUGH_SLOTS) {
                const unsigned long now = millis();
                if ((now - lastErrorLogMs) > 2000) {
                    Log.warningln("[DMX] RX packet error: %d (size=%u)", packet.err,
                                  static_cast<unsigned>(packet.size));
                    lastErrorLogMs = now;
                }
                continue;
            }

            const size_t copySize = packet.size < kMaxFrameSize ? packet.size : kMaxFrameSize;
            dmx_read(kDmxPort, rawFrame, copySize);

            //SC = StartCode
            if (packet.sc != DMX_SC || rawFrame[0] != DMX_SC) {
                continue;
            }

            const size_t channelCount = (copySize - 1) > sizeof(lastFrame) ? sizeof(lastFrame) : (copySize - 1);
            portENTER_CRITICAL(&frameMux);
            std::memcpy(lastFrame, &rawFrame[1], channelCount);
            lastFrameSize = channelCount;
            frameReady = true;
            lastPacketTime = millis();
            portEXIT_CRITICAL(&frameMux);
        }
    }
}

void DmxInput::init() {
    if (initialized) {
        return;
    }

    dmx_driver_install(kDmxPort, &dmxConfig, personalities, 1);
    dmx_set_pin(kDmxPort, hw_config::DMX_TX_Pin, hw_config::DMX_RX_Pin, hw_config::DMX_RTS_Pin);

    if (receiverTaskHandle == nullptr) {
        const BaseType_t taskOk = xTaskCreate(dmxReceiverTask, "dmx_rx", kReceiverTaskStackWords, nullptr,
                                              kReceiverTaskPriority, &receiverTaskHandle);
        if (taskOk != pdPASS) {
            Log.errorln("[DMX] Failed to create receiver task");
        }
    }

    initialized = true;

    Log.infoln("[DMX] Input Handler initialized (esp_dmx, RX=%u, TX=%u, RTS=%u)",
               hw_config::DMX_RX_Pin, hw_config::DMX_TX_Pin, hw_config::DMX_RTS_Pin);
}

void DmxInput::setEnabled(bool enable) {
    enabled = enable;
    if (!initialized) {
        return;
    }
    if (!dmx_driver_is_installed(kDmxPort)) {
        return;
    }
    if (enable) {
        if (!dmx_driver_is_enabled(kDmxPort)) {
            dmx_driver_enable(kDmxPort);
        }
    } else {
        if (dmx_driver_is_enabled(kDmxPort)) {
            dmx_driver_disable(kDmxPort);
        }
        portENTER_CRITICAL(&frameMux);
        frameReady = false;
        lastFrameSize = 0;
        lastPacketTime = 0;
        portEXIT_CRITICAL(&frameMux);
    }
}

void DmxInput::setReceiveSlotCount(size_t slotCount) {
    const size_t clamped = clampReceiveSlots(slotCount);
    portENTER_CRITICAL(&frameMux);
    receiveSlotCount = clamped;
    portEXIT_CRITICAL(&frameMux);
    Log.infoln("[DMX] RX expected slots set to %u", static_cast<unsigned>(clamped));
}

void DmxInput::setFrameCallback(FrameCallback callback) {
    frameCallback = callback;
}

void DmxInput::handle() {
    if (!enabled) {
        return;
    }

    size_t frameSize = 0;
    bool hasFrame = false;

    portENTER_CRITICAL(&frameMux);
    if (frameReady && lastFrameSize > 0) {
        const size_t copySize = lastFrameSize < sizeof(callbackFrame) ? lastFrameSize : sizeof(callbackFrame);
        std::memcpy(callbackFrame, lastFrame, copySize);
        frameSize = copySize;
        frameReady = false;
        hasFrame = true;
    }
    portEXIT_CRITICAL(&frameMux);

    if (hasFrame && frameCallback != nullptr) {
        frameCallback(callbackFrame, static_cast<uint16_t>(frameSize));
    }
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
    unsigned long lastSeen = 0;
    portENTER_CRITICAL(&frameMux);
    lastSeen = lastPacketTime;
    portEXIT_CRITICAL(&frameMux);

    const unsigned long now = millis();
    if (lastSeen == 0) {
        return false;
    }
    return (now - lastSeen) < timeoutMs;
}

unsigned long DmxInput::lastPacketAgeMs() {
    unsigned long lastSeen = 0;
    portENTER_CRITICAL(&frameMux);
    lastSeen = lastPacketTime;
    portEXIT_CRITICAL(&frameMux);

    if (lastSeen == 0) {
        return 0;
    }
    return millis() - lastSeen;
}
