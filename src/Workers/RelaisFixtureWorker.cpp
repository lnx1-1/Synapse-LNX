//
// Created by lnx on 15.08.2025.
//

#include "RelaisFixtureWorker.h"

#include "I2C_Handler.h"

#define XL9535_CONFIG_PORT 0x06
#define XL9535_CONFIG_OUTPUT 0x00
#define XL9535_OUTPUT_PORT 0x02
#define MOTOR_STOP 0b10
#define MOTOR_CLOSE 0b00
#define MOTOR_OPEN 0b11
#define LIMIT_SWITCH_PIN 4

bool limitPressed = false;
// Debouncing parameters
constexpr unsigned long DEBOUNCE_DELAY = 50; // 50ms debounce time
unsigned long lastDebounceTime = 0;
int lastButtonState = HIGH; // Previous reading from input pin
int buttonState = HIGH; // Current debounced state
int lastStableState = HIGH; // Last confirmed stable state

RelaisFixtureWorker::RelaisFixtureWorker(const Fixture &fixture) {
    _fixture = fixture;
    Log.verboseln("Setting up Relais Fixture Worker. Number of Channels: %d", _fixture.channelCount);

    pinMode(LIMIT_SWITCH_PIN, INPUT_PULLUP); // Enable internal pull-up resistor
    buttonState = digitalRead(LIMIT_SWITCH_PIN);
    Log.verboseln("Initial Limit State: %d", buttonState);

    _isConnected = I2C_Handler::isDeviceConnected(_fixture.i2cAddress, _fixture.toString());
    Wire1.beginTransmission(_fixture.i2cAddress);
    Wire1.write(XL9535_CONFIG_PORT);
    Wire1.write(XL9535_CONFIG_OUTPUT);
    int error = Wire1.endTransmission();
    if (error) {
        I2C_Handler::PrintError(error);
    } else {
        Log.infoln("Config of Relais Succesfull");
    }

    setRelais(MOTOR_STOP);
}

void RelaisFixtureWorker::tick() {
    int reading = digitalRead(LIMIT_SWITCH_PIN);

    // Check if the state has changed from the last reading
    if (reading != lastButtonState) {
        // Reset the debounce timer
        lastDebounceTime = millis();
    }

    // Check if enough time has passed since the last state change
    if ((millis() - lastDebounceTime) > DEBOUNCE_DELAY) {
        // If the state has been stable for the debounce period
        if (reading != buttonState) {
            buttonState = reading;
            limitPressed = (buttonState == HIGH);
            // Detect state transitions
            if (buttonState != lastStableState) {
                Log.verboseln("Switch State: %d", buttonState);

                if (limitPressed) {
                    Log.verboseln("Limit Pressed");
                } else {
                    Log.verboseln("Limit Released");
                }
                lastStableState = buttonState;
            }
        }
    }
    // Save the current reading for next iteration
    lastButtonState = reading;
}


void RelaisFixtureWorker::setRelais(uint8_t bits) const {
    Wire1.beginTransmission(_fixture.i2cAddress);
    Wire1.write(XL9535_OUTPUT_PORT);
    Wire1.write(bits);
    int error = Wire1.endTransmission();
    if (error) {
        I2C_Handler::PrintError(error);
    }
}

void printByte(uint8_t byte) {
    Serial.print("[");
    for (int i = 7; i >= 0; --i) {
        // von höchstwertigem Bit (MSB) zum niedrigsten (LSB)
        Serial.printf("%d", (byte >> i) & 1);
    }
    Serial.print("]");
}

uint8_t lastCMDVal = 0b10;

void setMotor() {
    if (data[0] < 85) {
        outCMD = outCMD | MOTOR_STOP; // Motor Stop
        Log.verboseln("Sending Motor Stop");
    } else if (data[0] < 170) {
        outCMD = outCMD | MOTOR_CLOSE; // Kugel Close
        Log.verboseln("Sending Motor Close");
    } else {
        if (limitPressed) {
            Log.verboseln("Limit Currently Pressed. Motor Open Ignored");
            outCMD = outCMD | MOTOR_STOP; // Motor Stop
            Log.verboseln("Sending Motor Stop");
        } else {
            outCMD = outCMD | MOTOR_OPEN; // Kugel Open
            Log.verboseln("Sending Motor Open");
        }
    }
}

void RelaisFixtureWorker::SendValues(const uint8_t *data, size_t size) {
    uint8_t outCMD = 0;
    // Log.verbose("RelaisFixtureWorker::SendValues: Size: %d ", size);
    if (!checkParamsValid(data, &size)) {
        return;
    }
    // Handle Channel 0 = Relais Actuator


    // Handle other Channels as simple On/Off Relais
    for (int i = 1; i < size && i < _fixture.channelCount; i++) {
        uint8_t commandBit = (data[i] > 127) ? 1 : 0; // Convert to binary command
        outCMD = outCMD | (commandBit << (i + 1)); // Because Relais 0+1 are for Motor
    }

    if (outCMD == lastCMDVal) {
        return;
    }
    // Log.verboseln("Sending: ");
    // printByte(outCMD);
    setRelais(outCMD);
    lastCMDVal = outCMD;
}
