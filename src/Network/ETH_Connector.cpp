//
// Created by lnx on 13.08.2025.
//

#include "ETH_Connector.h"

bool ETH_Connector::_isConnected = false;

void ETH_Connector::InitEth() {
    Log.infoln("+++ Setting up Ethernet connection... +++");
    WiFi.onEvent(WiFiEventHandler);
    // Initialize Ethernet with corrected parameters for ESP32-ETH01
    // Note: Try different clock modes if this doesn't work
    ETH.begin(ETH_ADDR, ETH_POWER_PIN, ETH_MDC_PIN, ETH_MDIO_PIN, ETH_TYPE, ETH_CLK_MODE_ESP32); // Enable ETH
    // init Network ETH
    ETH.config(Config::Sys_ip, Config::Sys_gateway, Config::Sys_subnet);

    Log.infoln("+++ ETH Initialized. Sitting on IP: %s +++", ETH.localIP().toString().c_str());
}

void ETH_Connector::WiFiEventHandler(arduino_event_id_t event) {
    switch (event) {
        case ARDUINO_EVENT_ETH_START:
            Log.infoln("ETH Started");
            //set eth hostname here
            ETH.setHostname("esp32-ethernet");
            break;
        case ARDUINO_EVENT_ETH_CONNECTED:
            Log.infoln("ETH Connected");
            break;
        case ARDUINO_EVENT_ETH_GOT_IP:
            Log.info("ETH MAC: %s, IPv4: %s, Speed: %d Mbps\n", ETH.macAddress().c_str(),
                     ETH.localIP().toString().c_str(), ETH.linkSpeed());


            _isConnected = true;
            break;
        case ARDUINO_EVENT_ETH_DISCONNECTED:
            Log.infoln("ETH Disconnected");
            _isConnected = false;
            break;
        case ARDUINO_EVENT_ETH_STOP:
            Log.infoln("ETH Stopped");
            _isConnected = false;
            break;
        default:
            break;
    }
}

bool ETH_Connector::isConnected() {
    return _isConnected;
}
