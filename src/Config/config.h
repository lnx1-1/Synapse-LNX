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
    const int LOG_LEVEL = LOG_LEVEL_VERBOSE;
}

class Config {
public:
    static int LOG_LEVEL; // Set the log level for debugging
    static int Universe;
    static IPAddress Sys_ip;
    static IPAddress Sys_subnet;
    static IPAddress Sys_gateway;

    static void init();

    static void save();

    static void load();
};


#endif //MYSTICCREATURESFIGUR_CONFIG_H
