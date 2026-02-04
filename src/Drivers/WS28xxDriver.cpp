#include "WS28xxDriver.h"
#include <NeoPixelBus.h>

template<typename T_COLOR_FEATURE>
class WS28xxDriverImpl : public WS28xxDriver {
    NeoPixelBus<T_COLOR_FEATURE, NeoEsp32I2s1Ws2812xMethod> *_strip = nullptr;

public:
    WS28xxDriverImpl(uint8_t pin, uint16_t count) {
        _strip = new NeoPixelBus<T_COLOR_FEATURE, NeoEsp32I2s1Ws2812xMethod>(count, pin);
        _strip->Begin();
    }

    ~WS28xxDriverImpl() override {
        delete _strip;
    }

    void show(const uint8_t *data, size_t size) override {
        if (!_strip) return;

        uint16_t pixelCount = _strip->PixelCount();
        for (uint16_t i = 0; i < pixelCount; i++) {
            uint16_t base = i * 3;
            if (base + 2 < size) {
                // NeoPixelBus features handle the mapping internally
                _strip->SetPixelColor(i, RgbColor(data[base], data[base + 1], data[base + 2]));
            } else {
                _strip->SetPixelColor(i, RgbColor(0, 0, 0));
            }
        }
        _strip->Show();
    }
};

WS28xxDriver *WS28xxDriver::create(uint8_t pin, uint16_t count, ColorOrder order) {
    switch (order) {
        case ColorOrder::RGB: return new WS28xxDriverImpl<NeoRgbFeature>(pin, count);
        case ColorOrder::GRB: return new WS28xxDriverImpl<NeoGrbFeature>(pin, count);
        case ColorOrder::BRG: return new WS28xxDriverImpl<NeoBrgFeature>(pin, count);
        case ColorOrder::RBG: return new WS28xxDriverImpl<NeoRbgFeature>(pin, count);
        case ColorOrder::GBR: return new WS28xxDriverImpl<NeoGbrFeature>(pin, count);
        case ColorOrder::BGR: return new WS28xxDriverImpl<NeoBgrFeature>(pin, count);
        default: return new WS28xxDriverImpl<NeoGrbFeature>(pin, count);
    }
}
