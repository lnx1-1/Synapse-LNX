//
// Created by lnx on 13.08.2025.
//
#include "config.h"

// Config.cpp
IPAddress Config::Sys_ip;
IPAddress Config::Sys_subnet;
IPAddress Config::Sys_gateway;

void Config::init() {
    Sys_ip = ConfigDefaults::IP;
    Sys_subnet = ConfigDefaults::SUBNET;
    Sys_gateway = ConfigDefaults::GATEWAY;
}
