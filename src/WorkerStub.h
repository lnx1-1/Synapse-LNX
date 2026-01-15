#ifndef WORKER_STUB_H
#define WORKER_STUB_H

#include <Wire.h>

#include "Idmx_FixtureWorker.h"
#include "Config/hw_config.h"

class WorkerStub : public Idmx_FixtureWorker {
public:
    WorkerStub(const Fixture &fixture, TwoWire &two_wire);

    void SendValues(const uint8_t *data, size_t size) override;

    TwoWire *_twoWire;
};

#endif
