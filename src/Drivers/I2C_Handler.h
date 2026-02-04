//
// Created by lnx on 14.08.2025.
//

#ifndef MYSTICCREATURESFIGUR_I2C_HANDLER_H
#define MYSTICCREATURESFIGUR_I2C_HANDLER_H
#include <Wire.h>


class I2C_Handler {
public:
    /**
     * @brief Initializes the I2C bus(es) used by the application.
     * @param scanAndWaitForDevices If true, the system will scan for devices and may wait if none are found.
     */
    static void initI2C(bool scanAndWaitForDevices = true);

    /**
     * @brief Scans a specific I2C bus for connected devices and logs their addresses.
     * @param wire Pointer to the TwoWire instance to scan.
     * @return The number of devices found on the bus.
     */
    static int scanForI2CDevices(TwoWire *wire);

    /**
     * @brief Prints a human-readable error message corresponding to an I2C error code.
     * @param errCode The error code returned by Wire.endTransmission().
     */
    static void PrintError(int errCode);

    /**
     * @brief Checks if a device with a specific I2C address is connected.
     * @param I2cAddr The I2C address of the device.
     * @param deviceName A descriptive name for the device (used for logging).
     * @return True if the device is found, false otherwise.
     */
    static bool isDeviceConnected(uint8_t I2cAddr, const String &deviceName);
};


#endif //MYSTICCREATURESFIGUR_I2C_HANDLER_H
