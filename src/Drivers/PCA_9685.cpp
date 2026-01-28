//
// Created by lnx on 14.08.2025.
//

#include "PCA_9685.h"

#include <Wire.h>

#include "ArduinoLog.h"
#include "I2C_Handler.h"


void PCA_9685::initPCA9685(uint updateFrequency) const {
  if (updateFrequency < 1 || updateFrequency > PWM_MAX_FREQUENCY) {
    Log.errorln("Invalid update frequency: %llu. Must be between 1 and %d Hz.", updateFrequency, PWM_MAX_FREQUENCY);
    return;
  }

  // Sleep-Modus aktivieren
  writeRegister(MODE1_REG, 0x10);
  double calc = static_cast<double>(OSC_CLOCK_FREQ) / (CLOCK_COUNTER_MAXVAL * updateFrequency) - 1;
  if (calc < 0 || calc > 255) {
    Log.errorln("Error While Initializing PCA9685. Wrong Prescaler Calculation: %f", calc);
    return;
  }
  auto prescaler = static_cast<uint8_t>(round(calc));
  // Berechnung des Prescalers // Berechnet: 25MHz/(4096*50Hz) - 1 = 121.09
  writeRegister(PRESCALE_REG, prescaler);

  // Sleep-Modus deaktivieren und Auto-Increment aktivieren
  writeRegister(MODE1_REG, 0xA0);

  // Alle Kanäle initial ausschalten
  for (int channel = 0; channel < 16; channel++) {
    setPWM(channel, 0);
  }
  Log.infoln("Init Done of PCA9685. Update Freq: %d Hz, Prescaler: %d", updateFrequency, prescaler);
}


void PCA_9685::setPWM(uint8_t channel, uint16_t value) const {
  if (_wire == nullptr) {
    Log.errorln("Wire is not initialized. Call initPCA9685 first.");
    return;
  }
  // Register-Adresse berechnen
  uint8_t reg = PWM_BASE_REG + PWM_CHANNEL_WITH * channel;

  // PWM-Wert schreiben (4 Bytes: ON_L, ON_H, OFF_L, OFF_H)
  _wire->beginTransmission(_i2CAddr);
  _wire->write(reg); // Startregister
  _wire->write(0); // ON_L (immer 0)
  _wire->write(0); // ON_H (immer 0)
  _wire->write(value & 0xFF); // OFF_L
  _wire->write((value >> 8) & 0x0F); // OFF_H Only the lower 4 bits are used
  if (const int err = _wire->endTransmission()) {
    I2C_Handler::PrintError(err);
  }
}

void PCA_9685::disablePWM(uint8_t channel) const {
  if (_wire == nullptr) {
    Log.errorln("Wire is not initialized. Call initPCA9685 first.");
    return;
  }

  // Register-Adresse berechnen
  uint8_t reg = PWM_BASE_REG + PWM_CHANNEL_WITH * channel;

  // PWM komplett ausschalten durch setzen des FULL_OFF Bits
  _wire->beginTransmission(_i2CAddr);
  _wire->write(reg); // Startregister
  _wire->write(0x00); // ON_L
  _wire->write(0x00); // ON_H
  _wire->write(0x00); // OFF_L
  _wire->write(0x10); // OFF_H - Bit 4 (FULL_OFF) gesetzt
  if (const int err = _wire->endTransmission()) {
    I2C_Handler::PrintError(err);
  }
}

void PCA_9685::writeRegister(uint8_t reg, uint8_t value) const {
  // Log.verboseln("Begin Transmission to Reg: %d Value: %d, Addr: %d", reg, value, I2CAddr);
  _wire->beginTransmission(_i2CAddr);
  _wire->write(reg);
  _wire->write(value);
  int err = _wire->endTransmission();
  if (err != 0) {
    I2C_Handler::PrintError(err);
  }
}

PCA_9685::PCA_9685(TwoWire *wire, int8_t i2cAddr) {
  if (wire == nullptr || i2cAddr == 0) {
    Log.errorln("Invalid Wire or I2C Address for PCA9685 initialization.");
    return;
  }
  _wire = wire;
  _i2CAddr = i2cAddr;
}
