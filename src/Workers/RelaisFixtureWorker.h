//
// Created by lnx on 15.08.2025.
//

#ifndef MYSTICCREATURESFIGUR_RELAISFIXTUREWORKER_H
#define MYSTICCREATURESFIGUR_RELAISFIXTUREWORKER_H
#include "Idmx_FixtureWorker.h"
#include "../Config/FixtureSettingsManager.h"


class RelaisFixtureWorker : public Idmx_FixtureWorker {
    /**
     * @brief Sets the state of the relays based on the provided bitmask.
     * @param bits A bitmask where each bit represents a relay state (on/off).
     */
    void setRelais(uint8_t bits) const;

public:
    /**
     * @brief Constructor for the Relais Fixture Worker.
     * @param fixture The Fixture configuration.
     */
    explicit RelaisFixtureWorker(const Fixture &fixture);

    /**
     * @brief Periodically called to update the relay states if needed.
     */
    void tick() override;

    /**
     * @brief Maps the received DMX data to relay bitmask and updates hardware.
     * @param data Pointer to the channel data.
     * @param size Number of channels.
     */
    void SendValues(const uint8_t *data, size_t size) override;
};


#endif //MYSTICCREATURESFIGUR_RELAISFIXTUREWORKER_H
