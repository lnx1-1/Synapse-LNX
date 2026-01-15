//
// Created by lnx on 07.08.2025.
//

#ifndef MYSTICCREATURESFIGUR_CONFIG_H
#define MYSTICCREATURESFIGUR_CONFIG_H
#include "ArduinoLog.h"

namespace ConfigDefaults {
    const IPAddress IP{192, 168, 178, 50};
    const IPAddress SUBNET{255, 255, 255, 0};
    const IPAddress GATEWAY{192, 168, 178, 2};
}

class Config {
public:
    static constexpr int LOG_LEVEL = LOG_LEVEL_VERBOSE; // Set the log level for debugging
    static constexpr int Universe = 1;
    static IPAddress Sys_ip;
    static IPAddress Sys_subnet;
    static IPAddress Sys_gateway;

    static void init();
};


#endif //MYSTICCREATURESFIGUR_CONFIG_H
