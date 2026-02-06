#ifndef HW_CONFIG_H
#define HW_CONFIG_H

#include <Arduino.h>
#include <list>

enum class FixtureID {
    LaserModule,
    MovementModule,
    LightModule, Dummy, RelaisModule, StepperModule
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
            case FixtureID::StepperModule:
                return "StepperModule";
            default:
                return "Unknown";
        }
    }
};

class hw_config {
public:
    // Disable I2C while Stepper Enable uses GPIO17. Re-enable and change pins when I2C is needed.
    static constexpr bool I2C_Enable = false;
    static constexpr uint8_t I2C_SDA_Pin = 5; //32; //CFG Gelabelt auf ETH01
    static constexpr uint8_t I2C_SCL_Pin = 17; //33; //485_EN Gelabeld auf ETH01
    static constexpr uint I2C_Speed = 100000; // 100 kHz Slowest but most compatible speed
    static constexpr uint SerialBaud = 115200; // Serial communication speed
    static constexpr uint8_t DMX_TX_Pin = 2;
    static constexpr uint8_t DMX_RX_Pin = 35;
    static constexpr uint8_t DMX_RTS_Pin = 32;
};

inline std::list<Fixture> getFixtures() {
    std::list<Fixture> fixtures;
    //DMX Adress Starting at 1
    auto dummyFixture = Fixture{FixtureID::Dummy, 3, 1, 0x40}; // Dummy fixture for testing
    auto RelaisFixture = Fixture{FixtureID::RelaisModule, 4, 1, 0x20}; // Dummy fixture for testing
    auto laserModule = Fixture{FixtureID::LaserModule, 1, 1, 0x40};
    auto movementModule = Fixture{FixtureID::MovementModule, 8, 0x9, 0x21};
    auto lightModule = Fixture{FixtureID::LightModule, 8, 0x19, 0x22};
    auto StepperModule = Fixture{FixtureID::StepperModule, 8, 0x19, 0x22};

    // fixtures.push_back(laserModule);
    // fixtures.push_back(movementModule);
    // fixtures.push_back(lightModule);
    // fixtures.push_back(RelaisFixture);
    // fixtures.push_back(dummyFixture);
    // fixtures.push_back(Fixture{FixtureID::RelaisModule, 3, 1, 0x20});
    fixtures.push_back(Fixture{FixtureID::StepperModule, 2, 0x1, 0x0});
    fixtures.push_back(Fixture{FixtureID::LightModule, 180, 0x10, 0x0});
    return fixtures;
}

#endif
