#ifndef WEB_INTERFACE_H
#define WEB_INTERFACE_H

#include <WebServer.h>
#include <deque>
#include "../Config/config.h"
#include "../Idmx_FixtureWorker.h"

class WebLog : public Print {
public:
    /**
     * @brief Writes a character to the log buffer.
     * @param c The character to write.
     * @return 1 on success.
     */
    size_t write(uint8_t c) override;

    /**
     * @brief Retrieves the collected logs as a single string.
     * @return The logs stored in the buffer.
     */
    String getLogs();

    /**
     * @brief Clears the log buffer.
     */
    void clear();

    bool enabled = false;

private:
    std::deque<char> buffer;
    const size_t max_size = 5000;
};

class WebInterface {
public:
    /**
     * @brief Initializes the web server and defines API endpoints.
     */
    static void init();

    /**
     * @brief Handles incoming web server requests. Should be called in the main loop.
     */
    static void handle();

    static WebLog webLog;

    /**
     * @brief Enable or disable captive portal mode (serves minimal config UI).
     */
    static void setCaptivePortalActive(bool active);

    /**
     * @brief Returns true if captive portal mode is active.
     */
    static bool isCaptivePortalActive();

private:
    static WebServer server;
    static bool captivePortalActive;

    /**
     * @brief Endpoint for the root HTML page.
     */
    static void handleRoot();
    static void handleNotFound();

    /**
     * @brief Endpoint for saving settings.
     */
    static void handleSave();

    /**
     * @brief Endpoint for toggling the web log.
     */
    static void handleToggleLog();

    /**
     * @brief Endpoint for fetching the current log buffer.
     */
    static void handleGetLogs();

    /**
     * @brief Endpoint for checking device status.
     */
    static void handleStatus();

    /**
     * @brief Endpoint for DMX input status.
     */
    static void handleDmxStatus();

    /**
     * @brief Finalizes firmware upload and restarts the device on success.
     */
    static void handleFirmwareUpdate();

    /**
     * @brief Streams firmware upload chunks into the ESP32 update API.
     */
    static void handleFirmwareUpload();

    /**
     * @brief Generates the full HTML content for the web interface.
     * @return The HTML string.
     */
    static String getHtml();
};

#endif // WEB_INTERFACE_H
