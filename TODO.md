# Project TODOs

## Hardware & Pin Conflicts

- [ ] **GPIO 5 Conflict**: Resolve conflict between `I2C_SDA_Pin` and `Stepper PIN_DIRECTION`. (Both currently set to
  GPIO 5).
    - *Context*: Required when moving to final hardware layout or adding more modules.
- [ ] **I2C Re-Enable Plan (future)**: I2C is currently disabled because `PIN_ENABLE` uses GPIO17 and `PIN_DIRECTION`
  uses GPIO5. Before re-enabling I2C, pick a final pin map and update code + wiring.
    - [ ] Decide final I2C pins that do not conflict with Stepper + DMX + Ethernet on WT32-ETH01.
    - [ ] Option A (preferred if spare GPIOs exist): Move Stepper pins off the I/O header, free GPIO5/17/33 for I2C.
      - [ ] Update `PIN_STEP`, `PIN_DIRECTION`, `PIN_ENABLE` in `src/Workers/StepperFixtureWorker.h`.
      - [ ] Set `hw_config::I2C_SDA_Pin`/`I2C_SCL_Pin` in `src/Config/hw_config.h`.
    - [ ] Option B (if only I/O header pins are available for Stepper): Keep Stepper on GPIO33/5/17 and move I2C to
      alternative GPIOs (verify board exposure first).
    - [ ] Set `hw_config::I2C_Enable = true` and validate I2C scan without Stepper noise.
    - [ ] Update pin map in docs once finalized.

## Features & Improvements

- [ ] **DHCP & Network Fallback**:
    - [ ] Add `use_dhcp` setting in `Config`.
    - [ ] Implement detection of invalid static IP (e.g., gateway not reachable).
    - [ ] Logic: Static IP -> Fallback to DHCP -> Fallback to WiFi Access Point.
- [ ] **WiFi Captive Portal**:
    - [ ] Implement WiFi AP mode if Ethernet fails.
    - [ ] Integrate a DNS server to redirect all requests to the configuration page.
    - [ ] Add timeout for AP mode (auto-off after X minutes).
    - [ ] Simplify `CaptivePortal.html` or adapt current `WebInterface` for mobile-friendly setup.
- [ ] **DMX Fallback**:
    - [ ] Configurable timeout (ms).
    - [ ] Fallback mode: blackout or hold last.
- [ ] **DMX Driver Mode Switching**:
    - [ ] Cleanly enable/disable DMX driver when switching input modes.
- [ ] **OTA Updates (Over-The-Air)**:
    - [ ] Implement Web-based OTA (Upload `.bin` via browser).
    - [ ] (Optional) Add `ArduinoOTA` for developer convenience.
- [ ] **Gamma Correction**: Implement Gamma Correction for LEDs to improve color perception (linear DMX to non-linear
  brightness).
    - *Reference*: Check [WLED implementation](https://github.com/wled/WLED) for efficient lookup tables.
- [ ] **Advanced UI**: Improve Webinterface with more interactive elements (Real-time DMX monitor, etc.).

## Documentation

- [ ] Document global pin mapping once finalized.
