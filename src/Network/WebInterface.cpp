#include "WebInterface.h"
#include <ArduinoLog.h>
#include <list>

extern std::list<Idmx_FixtureWorker *> dmxFixtures;
extern unsigned long lastArtnetPacketTime;

// Re-init functions defined in main.cpp or similar
extern void reinitModules();

WebServer WebInterface::server(80);
WebLog WebInterface::webLog;

size_t WebLog::write(uint8_t c) {
    if (enabled) {
        if (buffer.size() >= max_size) {
            buffer.pop_front();
        }
        buffer.push_back((char) c);
    }
    return 1;
}

String WebLog::getLogs() {
    String s = "";
    for (char c: buffer) {
        s += c;
    }
    return s;
}

void WebLog::clear() {
    buffer.clear();
}

void WebInterface::init() {
    server.on("/", handleRoot);
    server.on("/save", HTTP_POST, handleSave);
    server.on("/toggleLog", HTTP_POST, handleToggleLog);
    server.on("/getLogs", HTTP_GET, handleGetLogs);
    server.on("/status", HTTP_GET, handleStatus);
    server.begin();
    Log.infoln("Web Interface started on port 80");
}

void WebInterface::handle() {
    server.handleClient();
}

void WebInterface::handleRoot() {
    server.send(200, "text/html", getHtml());
}

void WebInterface::handleSave() {
    if (server.hasArg("ip")) Config::Sys_ip.fromString(server.arg("ip"));
    if (server.hasArg("subnet")) Config::Sys_subnet.fromString(server.arg("subnet"));
    if (server.hasArg("gateway")) Config::Sys_gateway.fromString(server.arg("gateway"));
    if (server.hasArg("universe")) Config::Universe = server.arg("universe").toInt();
    if (server.hasArg("log_level")) Config::LOG_LEVEL = server.arg("log_level").toInt();

    Config::save();

    for (int i = 0; i < server.args(); i++) {
        String argName = server.argName(i);
        if (argName.startsWith("fix_")) {
            String value = server.arg(i);
            for (auto worker: dmxFixtures) {
                String workerPrefix = "fix_" + String(worker->_fixture.toString()) + "_" + String(
                                          worker->_fixture.i2cAddress) + "_";
                if (argName.startsWith(workerPrefix)) {
                    String key = argName.substring(workerPrefix.length());
                    worker->applySetting(key, value);
                    break;
                }
            }
        }
    }

    server.send(200, "text/html",
                "<html><head><meta http-equiv='refresh' content='3;url=/'></head><body><h1>Settings saved.</h1><p>Re-initializing modules... Redirecting in 3 seconds...</p><a href='/'>Back manually</a></body></html>");

    delay(500);
    reinitModules();
}

void WebInterface::handleToggleLog() {
    webLog.enabled = !webLog.enabled;
    if (!webLog.enabled) webLog.clear();
    server.send(200, "text/plain", webLog.enabled ? "enabled" : "disabled");
}

void WebInterface::handleGetLogs() {
    server.send(200, "text/plain", webLog.getLogs());
}

void WebInterface::handleStatus() {
    bool receiving = (lastArtnetPacketTime > 0) && (millis() - lastArtnetPacketTime < 2000);
    server.send(200, "text/plain", receiving ? "active" : "inactive");
}

String WebInterface::getHtml() {
    String html = "<!DOCTYPE html><html><head><title>Kaleo ArtNet Node</title>";
    html += "<style>";
    html += "body { font-family: Arial; margin: 20px; background-color: #f4f4f9; color: #333; }";
    html += "h1, h2 { color: #4CAF50; }";
    html +=
            "table { border-collapse: collapse; width: 100%; margin-bottom: 20px; background-color: #fff; box-shadow: 0 2px 5px rgba(0,0,0,0.1); }";
    html += "td, th { border: 1px solid #ddd; padding: 12px; }";
    html += "tr:nth-child(even){background-color: #f9f9f9;}";
    html +=
            "th { padding-top: 12px; padding-bottom: 12px; text-align: left; background-color: #4CAF50; color: white; }";
    html += "input[type=text], select { padding: 8px; width: 100%; box-sizing: border-box; }";
    html +=
            "button { background-color: #4CAF50; color: white; padding: 10px 20px; border: none; border-radius: 4px; cursor: pointer; margin: 5px 0; }";
    html += "button:hover { background-color: #45a049; }";
    html +=
            "#logContainer { background-color: #000; color: #0f0; padding: 10px; height: 300px; overflow-y: scroll; font-family: monospace; white-space: pre-wrap; }";
    html += "</style>";
    html += "<script>";
    html += "function toggleLog() {";
    html += "  fetch('/toggleLog', {method: 'POST'}).then(r => r.text()).then(t => {";
    html += "    document.getElementById('logBtn').innerText = t === 'enabled' ? 'Stop Log' : 'Start Log';";
    html += "  });";
    html += "}";
    html += "setInterval(() => {";
    html += "  if(document.getElementById('logBtn').innerText === 'Stop Log') {";
    html += "    fetch('/getLogs').then(r => r.text()).then(t => {";
    html += "      const c = document.getElementById('logContainer');";
    html += "      c.innerText = t;";
    html += "      c.scrollTop = c.scrollHeight;";
    html += "    });";
    html += "  }";
    html += "}, 1000);";
    html += "setInterval(() => {";
    html += "  fetch('/status').then(r => r.text()).then(t => {";
    html += "    const s = document.getElementById('artnetStatus');";
    html += "    if (t === 'active') {";
    html += "      s.innerText = 'Art-Net: Receiving';";
    html += "      s.style.backgroundColor = '#4CAF50';";
    html += "      s.style.color = 'white';";
    html += "    } else {";
    html += "      s.innerText = 'Art-Net: No Data';";
    html += "      s.style.backgroundColor = '#f44336';";
    html += "      s.style.color = 'white';";
    html += "    }";
    html += "  });";
    html += "}, 1000);";
    html += "</script>";
    html += "</head><body>";

    html +=
            "<h1>Kaleo ArtNet Node <div id='artnetStatus' style='display:inline-block; font-size: 0.5em; vertical-align: middle; padding:5px 10px; border-radius:4px; background-color:#ccc;'>Art-Net: Checking...</div></h1>";

    html += "<h2>Settings</h2>";
    html += "<form action='/save' method='POST'>";
    html += "<table>";
    html += "<tr><th>Setting</th><th>Value</th></tr>";
    html += "<tr><td>IP Address</td><td><input type='text' name='ip' value='" + Config::Sys_ip.toString() +
            "'></td></tr>";
    html += "<tr><td>Subnet Mask</td><td><input type='text' name='subnet' value='" + Config::Sys_subnet.toString() +
            "'></td></tr>";
    html += "<tr><td>Gateway</td><td><input type='text' name='gateway' value='" + Config::Sys_gateway.toString() +
            "'></td></tr>";
    html += "<tr><td>Art-Net Universe</td><td><input type='text' name='universe' value='" + String(Config::Universe) +
            "'></td></tr>";

    html += "<tr><td>Log Level</td><td><select name='log_level'>";
    int levels[] = {
        LOG_LEVEL_SILENT, LOG_LEVEL_FATAL, LOG_LEVEL_ERROR, LOG_LEVEL_WARNING, LOG_LEVEL_NOTICE, LOG_LEVEL_TRACE,
        LOG_LEVEL_VERBOSE
    };
    const char *levelNames[] = {"Silent", "Fatal", "Error", "Warning", "Notice", "Trace", "Verbose"};
    for (int i = 0; i < 7; i++) {
        html += "<option value='" + String(levels[i]) + "'" + (Config::LOG_LEVEL == levels[i] ? " selected" : "") + ">"
                + levelNames[i] + "</option>";
    }
    html += "</select></td></tr>";
    html += "</table>";

    html += "<h2>Fixture Settings</h2>";
    for (auto const &worker: dmxFixtures) {
        auto settings = worker->getSettings();
        if (!settings.empty()) {
            html += "<h3>" + String(worker->_fixture.toString()) + " (I2C: " + String(worker->_fixture.i2cAddress) +
                    ")</h3>";
            html += "<table>";
            html += "<tr><th>Setting</th><th>Value</th></tr>";
            for (auto const &s: settings) {
                String inputName = "fix_" + String(worker->_fixture.toString()) + "_" +
                                   String(worker->_fixture.i2cAddress) + "_" + s.key;
                html += "<tr><td>" + s.label + "</td><td><input type='" + s.type + "' name='" + inputName + "' value='"
                        + s.value + "'></td></tr>";
            }
            html += "</table>";
        }
    }

    html += "<button type='submit'>Save All Settings</button>";
    html += "</form>";

    html += "<h2>Active Fixtures</h2>";
    html += "<table><tr><th>Type</th><th>DMX Address</th><th>Channels</th><th>Connected</th></tr>";
    for (auto const &worker: dmxFixtures) {
        html += "<tr>";
        html += "<td>" + String(worker->_fixture.toString()) + "</td>";
        html += "<td>" + String(worker->_fixture.dmxAddress) + "</td>";
        html += "<td>" + String((int) worker->_fixture.channelCount) + "</td>";
        html += "<td>" + String(worker->isConnected() ? "Yes" : "No") + "</td>";
        html += "</tr>";
    }
    html += "</table>";

    html += "<h2>Live Log</h2>";
    html += "<button id='logBtn' onclick='toggleLog()'>" + String(webLog.enabled ? "Stop Log" : "Start Log") +
            "</button>";
    html += "<div id='logContainer'>" + webLog.getLogs() + "</div>";

    html += "</body></html>";
    return html;
}
