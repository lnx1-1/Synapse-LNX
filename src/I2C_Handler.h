//
// Created by lnx on 14.08.2025.
//

#ifndef MYSTICCREATURESFIGUR_I2C_HANDLER_H
#define MYSTICCREATURESFIGUR_I2C_HANDLER_H
#include <Wire.h>


class I2C_Handler {
public:
    static void initI2C();

    static int scanForI2CDevices(TwoWire *wire);

    static void PrintError(int errCode);

    static bool isDeviceConnected(uint8_t I2cAddr, const String &deviceName);
};


#endif //MYSTICCREATURESFIGUR_I2C_HANDLER_H
