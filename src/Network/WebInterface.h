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

private:
    static WebServer server;

    /**
     * @brief Endpoint for the root HTML page.
     */
    static void handleRoot();

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
     * @brief Generates the full HTML content for the web interface.
     * @return The HTML string.
     */
    static String getHtml();
};

#endif // WEB_INTERFACE_H
