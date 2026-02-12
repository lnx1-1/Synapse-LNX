//
// Created by lnx on 07.08.2025.
//

#ifndef MYSTICCREATURESFIGUR_CONFIG_H
#define MYSTICCREATURESFIGUR_CONFIG_H
#include <Arduino.h>
#include "ArduinoLog.h"

#ifdef ESP32
#include <Preferences.h>
#endif

namespace ConfigDefaults {
    const IPAddress IP{10, 0, 0, 2};
    const IPAddress SUBNET{255, 255, 255, 0};
    const IPAddress GATEWAY{10, 0, 0, 1};
    const int UNIVERSE = 1;
    const int LOG_LEVEL = LOG_LEVEL_NOTICE;
    enum class InputMode : uint8_t {
        ArtNet = 0,
        DMX = 1
    };
    const InputMode INPUT_MODE = InputMode::ArtNet;
    const bool CAPTIVE_ENABLE = true;
    const uint32_t CAPTIVE_GRACE_MS = 10000;
    const uint32_t CAPTIVE_DURATION_MS = 180000;
    const char CAPTIVE_SSID[] = "SYNAPSE-LNX-Setup";
    const char CAPTIVE_PASS[] = "connectchaos";
    const uint32_t DMX_BLACKOUT_TIMEOUT_MS = 2000;
}

class Config {
public:
    static int LOG_LEVEL; // Set the log level for debugging
    static int Universe;
    static IPAddress Sys_ip;
    static IPAddress Sys_subnet;
    static IPAddress Sys_gateway;
    static ConfigDefaults::InputMode InputMode;
    static bool CaptiveEnabled;
    static uint32_t CaptiveGraceMs;
    static uint32_t CaptiveDurationMs;
    static uint32_t DmxBlackoutTimeoutMs;
    static String CaptiveSsid;
    static String CaptivePass;

    static void init();

    static void save();

    static void load();
};


#endif //MYSTICCREATURESFIGUR_CONFIG_H
