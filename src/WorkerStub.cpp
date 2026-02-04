#include "WorkerStub.h"
#include "Config/FixtureSettingsManager.h"
#include <Arduino.h>

#include "ArduinoLog.h"


WorkerStub::WorkerStub(const Fixture &fixture, TwoWire &two_wire) : Idmx_FixtureWorker(fixture) {
    settingsManager->load();
    Log.infoln("[%s] Would setup i2c connection", _fixture.toString());
    _twoWire = &two_wire;
    //Testing if the i2c connection is working
    _twoWire->beginTransmission(_fixture.i2cAddress);
    int error = _twoWire->endTransmission();
    if (error) {
        Log.errorln("[%s] my i2c client is not reachable under addr: %s \n", _fixture.toString(),
                    String(_fixture.i2cAddress, HEX).c_str());
    } else {
        _isConnected = true;
        Log.infoln("[%s] I2c Client connected", _fixture.toString());
    }
}

void WorkerStub::SendValues(const uint8_t *data, size_t size) {
    if (isConnected() && data != nullptr && size > 0) {
        if (size > _fixture.channelCount) {
            Log.warningln("[%s] Stub: Size %zu exceeds DMX channel count %u, truncating data\n", _fixture.toString(),
                          size, _fixture.channelCount);
            size = _fixture.channelCount; // Truncate to channel count
        }
        Log.verboseln("[%s] Stub: Sending values to hw Equivalent: ", _fixture.toString());
        _twoWire->beginTransmission(_fixture.i2cAddress);
        for (size_t i = 0; i < size; ++i) {
            Log.verbose("%02x", data[i]);
            _twoWire->write(data[i]);
        }
        int err = _twoWire->endTransmission();
        if (err) {
            Log.errorln("Error While sending data to I2C device ErrrCode: %d", err);
        }
        Log.verboseln("");
    } else if (!isConnected()) {
        Log.errorln("[%s] Stub: Not connected to hardware, cannot send values", _fixture.toString());
    } else {
        Log.errorln("[%s] Stub: No data to send or size is zero", _fixture.toString());
    }
}
