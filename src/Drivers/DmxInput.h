//
// Created by Codex on 04.02.2026.
//

#ifndef KALEO_DMX_INPUT_H
#define KALEO_DMX_INPUT_H

#include <Arduino.h>

class DmxInput {
public:
    using FrameCallback = void (*)(const uint8_t *data, uint16_t size);

    static void init();

    static void setEnabled(bool enabled);

    /**
     * @brief Set the number of DMX slots (including start code) expected by the receiver.
     * Values are clamped to [2..513].
     */
    static void setReceiveSlotCount(size_t slotCount);

    /**
     * @brief Register callback invoked when a new DMX frame is available.
     */
    static void setFrameCallback(FrameCallback callback);

    /**
     * @brief Dispatches buffered DMX frames to the registered callback.
     * Call this from loop().
     */
    static void handle();

    /**
     * @brief Copy the last captured DMX channels without consuming the frame-ready flag.
     */
    static size_t copyLastFrame(uint8_t *out, size_t outSize);

    /**
     * @brief Check whether DMX packets were received within the timeout.
     */
    static bool isReceiving(uint32_t timeoutMs = 2000);

    /**
     * @brief Age of last packet in ms (0 if none received yet).
     */
    static unsigned long lastPacketAgeMs();
};

#endif // KALEO_DMX_INPUT_H
