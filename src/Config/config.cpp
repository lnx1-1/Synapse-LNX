//
// Created by lnx on 13.08.2025.
//
#include "config.h"

int Config::LOG_LEVEL = ConfigDefaults::LOG_LEVEL;
int Config::Universe = ConfigDefaults::UNIVERSE;
IPAddress Config::Sys_ip = ConfigDefaults::IP;
IPAddress Config::Sys_subnet = ConfigDefaults::SUBNET;
IPAddress Config::Sys_gateway = ConfigDefaults::GATEWAY;
ConfigDefaults::InputMode Config::InputMode = ConfigDefaults::INPUT_MODE;

void Config::init() {
    load();
}

void Config::save() {
#ifdef ESP32
    Preferences prefs;
    prefs.begin("config", false);
    uint32_t ip = (uint32_t) Config::Sys_ip;
    uint32_t subnet = (uint32_t) Config::Sys_subnet;
    uint32_t gateway = (uint32_t) Config::Sys_gateway;
    prefs.putUInt("ip", ip);
    prefs.putUInt("subnet", subnet);
    prefs.putUInt("gateway", gateway);
    prefs.putInt("universe", Config::Universe);
    prefs.putInt("log_level", Config::LOG_LEVEL);
    prefs.putInt("input_mode", static_cast<int>(Config::InputMode));
    prefs.end();
#endif
}

void Config::load() {
#ifdef ESP32
    Preferences prefs;
    prefs.begin("config", true);

    Config::Sys_ip = IPAddress(prefs.getUInt("ip", (uint32_t) ConfigDefaults::IP));
    Config::Sys_subnet = IPAddress(prefs.getUInt("subnet", (uint32_t) ConfigDefaults::SUBNET));
    Config::Sys_gateway = IPAddress(prefs.getUInt("gateway", (uint32_t) ConfigDefaults::GATEWAY));

    Config::Universe = prefs.getInt("universe", ConfigDefaults::UNIVERSE);
    Config::LOG_LEVEL = prefs.getInt("log_level", ConfigDefaults::LOG_LEVEL);
    Config::InputMode = static_cast<ConfigDefaults::InputMode>(
        prefs.getInt("input_mode", static_cast<int>(ConfigDefaults::INPUT_MODE)));

    prefs.end();
#else
    Config::Sys_ip = ConfigDefaults::IP;
    Config::Sys_subnet = ConfigDefaults::SUBNET;
    Config::Sys_gateway = ConfigDefaults::GATEWAY;
    Config::Universe = ConfigDefaults::UNIVERSE;
    Config::LOG_LEVEL = ConfigDefaults::LOG_LEVEL;
    Config::InputMode = ConfigDefaults::INPUT_MODE;
#endif
}
