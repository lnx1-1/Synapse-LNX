#ifndef HW_CONFIG_H
#define HW_CONFIG_H

#include <Arduino.h>
#include <list>

enum class FixtureID {
    LaserModule,
    MovementModule,
    LightModule, Dummy, RelaisModule
};

struct Fixture {
    FixtureID id;
    uint8_t channelCount; // Current Implementation only allows less than 32 Channels per Device because i2c buffer len
    uint16_t dmxAddress;
    uint8_t i2cAddress;

    inline const char *toString() const {
        switch (id) {
            case FixtureID::LaserModule:
                return "LaserModule";
            case FixtureID::MovementModule:
                return "MovementModule";
            case FixtureID::LightModule:
                return "LightModule";
            case FixtureID::Dummy:
                return "DummyModule";
            case FixtureID::RelaisModule:
                return "RelaisModule";
            default:
                return "Unknown";
        }
    }
};

class hw_config {
public:
    static constexpr uint8_t I2C_SDA_Pin = 5; //32; //CFG Gelabelt auf ETH01
    static constexpr uint8_t I2C_SCL_Pin = 17; //33; //485_EN Gelabeld auf ETH01
    static constexpr uint I2C_Speed = 100000; // 100 kHz Slowest but most compatible speed
    static constexpr uint SerialBaud = 115200; // Serial communication speed
};

inline std::list<Fixture> getFixtures() {
    std::list<Fixture> fixtures;
    //DMX Adress Starting at 1
    auto dummyFixture = Fixture{FixtureID::Dummy, 3, 1, 0x40}; // Dummy fixture for testing
    auto RelaisFixture = Fixture{FixtureID::RelaisModule, 4, 1, 0x20}; // Dummy fixture for testing
    auto laserModule = Fixture{FixtureID::LaserModule, 1, 1, 0x40};
    auto movementModule = Fixture{FixtureID::MovementModule, 8, 0x9, 0x21};
    auto lightModule = Fixture{FixtureID::LightModule, 8, 0x19, 0x22};

    // fixtures.push_back(laserModule);
    // fixtures.push_back(movementModule);
    // fixtures.push_back(lightModule);
    // fixtures.push_back(RelaisFixture);
    // fixtures.push_back(dummyFixture);
    fixtures.push_back(Fixture{FixtureID::RelaisModule, 3, 1, 0x20}); // Dummy fixture for testing)
    fixtures.push_back(Fixture{FixtureID::MovementModule, 3, 0x4, 0x40});
    return fixtures;
}

#endif
