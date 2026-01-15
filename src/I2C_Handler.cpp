//
// Created by lnx on 14.08.2025.
//

#include "I2C_Handler.h"

#include <Wire.h>

#include "ArduinoLog.h"
#include "Config/hw_config.h"
#include "Network/ETH_Connector.h"

void I2C_Handler::initI2C() {
    bool i2cInitState = Wire1.begin(hw_config::I2C_SDA_Pin, hw_config::I2C_SCL_Pin, hw_config::I2C_Speed);
    if (i2cInitState) {
        Log.infoln("I2C Initialized on SDA: %d, SCL %d, @Speed: %d", hw_config::I2C_SDA_Pin,
                   hw_config::I2C_SCL_Pin, hw_config::I2C_Speed);
        while (0 == scanForI2CDevices(&Wire1)) {
            sleep(2); // Wait for 2 seconds before scanning again
        }
    } else {
        Log.warningln("I2C Initialization Failed");
    }
}


int I2C_Handler::scanForI2CDevices(TwoWire *wire) {
    int nDevices = 0;

    Log.infoln("~~~ Scanning I2C Devices ~~~");

    for (byte address = 1; address < 128; address++) {
        wire->beginTransmission(address);
        byte error = wire->endTransmission();

        switch (error) {
            case 0:
                Log.info("I2C device found at address: ");
                char hexAddress[3];
                hexAddress[0] = '\0';
                snprintf(hexAddress, sizeof(hexAddress), "%02x", address);
                Log.infoln(hexAddress);
                nDevices++;
                break;
            case 1:
                //Log.infoln("data too long to fit in transmit buffer ");
                break;
            case 2:
                //Log.infoln("received NACK on transmit of address");
                break;
            case 3:
                //Log.infoln("received NACK on transmit of data");
                break;
            case 4:
                Log.info("Unknown error at address 0x");
                if (address < 16) {
                    Log.info("0");
                }
                Log.info("%02x", address);
                Log.infoln("  :");
                break;
            default:
                Log.infoln("No I2C device found at address %d . Error Code: %d", address, error);;
        }
    }

    if (nDevices == 0) {
        Log.infoln(" - No I2C devices found -");
    }
    Log.infoln("~~~ Scanning done ~~~\n");
    return nDevices;
}


void I2C_Handler::PrintError(int errCode) {
    switch (errCode) {
        case 0:
            Log.info("All good, no error");
            break;
        case 1:
            Log.errorln("data too long to fit in transmit buffer ");
            break;
        case 2:
            Log.errorln("received NACK on transmit of address");
            break;
        case 3:
            Log.errorln("received NACK on transmit of data");
            break;
        default:
            Log.errorln("Unknown error. Code: %d", errCode);
    }
}


bool I2C_Handler::isDeviceConnected(const uint8_t I2cAddr, const String &deviceName) {
    Log.infoln("[%s] Setting up i2c connection to I2C Device at: %s", deviceName.c_str(), String(I2cAddr, HEX).c_str());
    Wire1.beginTransmission(I2cAddr);
    if (int error = Wire1.endTransmission()) {
        PrintError(error);
        Log.errorln("[%s] my i2c client is not reachable under addr: %s \n", deviceName.c_str(),
                    String(I2cAddr, HEX).c_str());
        return false;
    }

    return true;
    Log.infoln("[%s] I2c Client connected", deviceName.c_str());
}


