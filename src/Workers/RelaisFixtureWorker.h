//
// Created by lnx on 15.08.2025.
//

#ifndef MYSTICCREATURESFIGUR_RELAISFIXTUREWORKER_H
#define MYSTICCREATURESFIGUR_RELAISFIXTUREWORKER_H
#include "Idmx_FixtureWorker.h"


class RelaisFixtureWorker : public Idmx_FixtureWorker {
    void setRelais(uint8_t bits) const;

public:
    explicit RelaisFixtureWorker(const Fixture &fixture);

    void tick() override;

    void SendValues(const uint8_t *data, size_t size) override;
};


#endif //MYSTICCREATURESFIGUR_RELAISFIXTUREWORKER_H
