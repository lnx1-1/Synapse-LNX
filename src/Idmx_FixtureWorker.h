#ifndef HW_WORKER_H
#define HW_WORKER_H

#include "Config/hw_config.h"
#include <Arduino.h>

#include "ArduinoLog.h"

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
                          size, _fixture.channelCount);
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
    virtual ~Idmx_FixtureWorker() = default;

    virtual void tick() {
    }

    Fixture _fixture{};

    virtual void SendValues(const uint8_t *data, size_t size) = 0;

    bool isConnected() const {
        return _isConnected;
    }
};


#endif
