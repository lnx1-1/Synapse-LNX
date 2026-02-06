//
// Created by Codex on 04.02.2026.
//

#ifndef KALEO_DMX_INPUT_H
#define KALEO_DMX_INPUT_H

#include <Arduino.h>

class DmxInput {
public:
    static void init();

    static void setEnabled(bool enabled);

    static bool isEnabled();

    /**
     * @brief Copy the latest DMX frame into the provided buffer.
     * @param out Buffer to receive DMX data (channels only, start code removed).
     * @param outSize Size of the output buffer.
     * @param frameSize Output: number of bytes written into out.
     * @return True if a new frame was copied, false otherwise.
     */
    static bool readFrame(uint8_t *out, size_t outSize, size_t &frameSize);

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
