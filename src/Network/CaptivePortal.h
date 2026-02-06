//
// Created by Codex on 06.02.2026.
//

#ifndef KALEO_CAPTIVE_PORTAL_H
#define KALEO_CAPTIVE_PORTAL_H

#include <Arduino.h>

class CaptivePortal {
public:
    /**
     * @brief Initialize captive portal state (does not start AP yet).
     * @param graceMs Time to wait after boot before starting AP if Ethernet is not connected.
     * @param durationMs How long the AP should stay active before shutting down.
     */
    static void begin(uint32_t graceMs = 10000, uint32_t durationMs = 180000);

    /**
     * @brief Call periodically from loop to manage DNS and timeouts.
     */
    static void update();

    /**
     * @brief Returns true if AP portal is currently active.
     */
    static bool isActive();

private:
    static void start();
    static void stop();
};

#endif // KALEO_CAPTIVE_PORTAL_H
