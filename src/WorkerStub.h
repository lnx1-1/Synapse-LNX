#ifndef WORKER_STUB_H
#define WORKER_STUB_H

#include <Wire.h>

#include "Idmx_FixtureWorker.h"
#include "Config/hw_config.h"

class WorkerStub : public Idmx_FixtureWorker {
public:
    /**
     * @brief Constructor for the Worker Stub.
     * @param fixture The Fixture configuration.
     * @param two_wire Reference to the I2C bus to be used (though currently just a stub).
     */
    WorkerStub(const Fixture &fixture, TwoWire &two_wire);

    /**
     * @brief Stub implementation of SendValues. Logs the data but does not interact with hardware.
     * @param data Pointer to the buffer containing the channel values.
     * @param size Number of channels.
     */
    void SendValues(const uint8_t *data, size_t size) override;

    TwoWire *_twoWire;
};

#endif