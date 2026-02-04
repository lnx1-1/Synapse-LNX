#ifndef WS28XX_DRIVER_H
#define WS28XX_DRIVER_H

#include <Arduino.h>

enum class ColorOrder {
    RGB,
    GRB,
    BRG,
    RBG,
    GBR,
    BGR
};

class WS28xxDriver {
public:
    virtual ~WS28xxDriver() = default;

    virtual void show(const uint8_t *data, size_t size) = 0;

    static WS28xxDriver *create(uint8_t pin, uint16_t count, ColorOrder order);
};

#endif // WS28XX_DRIVER_H
