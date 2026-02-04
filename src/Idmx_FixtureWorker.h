#ifndef HW_WORKER_H
#define HW_WORKER_H

#include "Config/hw_config.h"
#include <Arduino.h>
#include <vector>

#include "ArduinoLog.h"

class FixtureSettingsManager;

struct FixtureSetting {
    String key;
    String label;
    String value;
    String type = "text";

    FixtureSetting(String k, String l, String v, String t = "text")
        : key(k), label(l), value(v), type(t) {
    }
};

class Idmx_FixtureWorker {
protected:
    bool _isConnected = false;

    /**
     * Checks if the parameters are valid for sending values.
     * Validates that data is not null, size is greater than zero, and size does not exceed the fixture's channel count.
     * If size exceeds channel count, it truncates size
     * @param data
     * @param size
     * @return False if data is null or size is zero, otherwise true
     */
    bool checkParamsValid(const uint8_t *data, size_t *size) const {
        if (size == nullptr || *size <= 0 || data == nullptr) {
            Log.errorln("[%s] Stub: No data to send or size is zero", _fixture.toString());
            return false;
        }
        if (*size > _fixture.channelCount) {
            Log.warningln("[%s] Stub: Size %zu exceeds DMX channel count %u, truncating data\n", _fixture.toString(),
                          *size, _fixture.channelCount);
            *size = _fixture.channelCount; // Truncate to channel count
        }
        return true;
    }

    /**
     * Checks if the parameters are valid for sending values.
     * @return True if Is Connected, otherwise false
     */
    bool stateValid() const {
        if (!isConnected()) {
            Log.errorln("[%s] Stub: Not connected to hardware,  can not send values", _fixture.toString());
            return false;
        }
        return true;
    }

public:
    /**
     * @brief Constructor for the Fixture Worker.
     * @param fixture The Fixture object containing configuration for this worker.
     */
    explicit Idmx_FixtureWorker(const Fixture &fixture);

    /**
     * @brief Virtual destructor for the Fixture Worker.
     */
    virtual ~Idmx_FixtureWorker();

    FixtureSettingsManager *settingsManager = nullptr;
    Fixture _fixture{};

    /**
     * @brief Periodically called method for background tasks or updates.
     * Implementation is optional for derived classes.
     */
    virtual void tick() {
    }

    /**
     * @brief Sends DMX-like values to the actual hardware.
     * This is a pure virtual function and must be implemented by derived classes.
     * @param data Pointer to the buffer containing the channel values.
     * @param size Number of channels (bytes) in the data buffer.
     */
    virtual void SendValues(const uint8_t *data, size_t size) = 0;

    /**
     * @brief Stops the fixture and typically sets all channels to zero.
     */
    virtual void stop() {
        if (_fixture.channelCount > 0) {
            auto *zeros = new uint8_t[_fixture.channelCount]{0};
            SendValues(zeros, _fixture.channelCount);
            delete[] zeros;
        }
    }

    /**
     * @brief Checks if the worker is currently connected to its hardware.
     * @return True if connected, false otherwise.
     */
    bool isConnected() const {
        return _isConnected;
    }

    /**
     * @brief Retrieves a list of configurable settings for this fixture.
     * @return A vector of FixtureSetting objects.
     */
    virtual std::vector<FixtureSetting> getSettings();

    /**
     * @brief Applies a specific setting value based on its key.
     * @param key The unique key identifying the setting.
     * @param value The new value to be applied as a string.
     */
    virtual void applySetting(String key, String value);

    /**
     * @brief Callback invoked when a setting has been modified.
     * @param key The key of the setting that changed.
     */
    virtual void onSettingsChanged(const String &key) {
    }
};


#endif
