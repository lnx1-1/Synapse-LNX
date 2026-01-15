//
// Created by lnx on 13.08.2025.
//


#ifndef MYSTICCREATURESFIGUR_ETH_CONNECTOR_H
#define MYSTICCREATURESFIGUR_ETH_CONNECTOR_H

#include <ETH.h>

#include "ArduinoLog.h"
#include "../Config/config.h"

#define ETH_ADDR        1
#define ETH_POWER_PIN   16//-1 //16 // Do not use it, it can cause conflict during the software reset.
#define ETH_POWER_PIN_ALTERNATIVE 16 //17
#define ETH_MDC_PIN    23
#define ETH_MDIO_PIN   18
#define ETH_TYPE       ETH_PHY_LAN8720
#define ETH_CLK_MODE_ESP32    ETH_CLOCK_GPIO17_OUT // ETH_CLOCK_GPIO0_IN

class ETH_Connector {
public:
    static void InitEth();
    static void WiFiEventHandler(arduino_event_id_t event);
    static bool isConnected();
private:
   static bool _isConnected;
};


#endif //MYSTICCREATURESFIGUR_ETH_CONNECTOR_H