#include "WebInterface.h"
#include <ArduinoLog.h>
#include "Drivers/DmxInput.h"
#include <WiFi.h>
#include <list>

extern std::list<Idmx_FixtureWorker *> dmxFixtures;
extern unsigned long lastArtnetPacketTime;

// Re-init functions defined in main.cpp or similar
extern void reinitModules();

WebServer WebInterface::server(80);
WebLog WebInterface::webLog;
bool WebInterface::captivePortalActive = false;

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
    server.on("/dmx", HTTP_GET, handleDmxStatus);
    server.onNotFound(handleNotFound);
    server.begin();
    Log.infoln("Web Interface started on port 80");
}

void WebInterface::handle() {
    server.handleClient();
}

void WebInterface::handleRoot() {
    if (captivePortalActive) {
        server.send(200, "text/html", getCaptiveHtml());
        return;
    }
    server.send(200, "text/html", getHtml());
}

void WebInterface::handleNotFound() {
    if (captivePortalActive) {
        server.send(200, "text/html", getCaptiveHtml());
        return;
    }
    server.send(404, "text/plain", "Not found");
}

void WebInterface::setCaptivePortalActive(bool active) {
    captivePortalActive = active;
}

bool WebInterface::isCaptivePortalActive() {
    return captivePortalActive;
}

void WebInterface::handleSave() {
    if (server.hasArg("ip")) Config::Sys_ip.fromString(server.arg("ip"));
    if (server.hasArg("subnet")) Config::Sys_subnet.fromString(server.arg("subnet"));
    if (server.hasArg("gateway")) Config::Sys_gateway.fromString(server.arg("gateway"));
    if (server.hasArg("universe")) Config::Universe = server.arg("universe").toInt();
    if (server.hasArg("log_level")) Config::LOG_LEVEL = server.arg("log_level").toInt();
    if (server.hasArg("input_mode")) {
        Config::InputMode = static_cast<ConfigDefaults::InputMode>(server.arg("input_mode").toInt());
    }
    if (server.hasArg("cap_enabled")) Config::CaptiveEnabled = server.arg("cap_enabled") == "1";
    if (server.hasArg("cap_grace_ms")) Config::CaptiveGraceMs = server.arg("cap_grace_ms").toInt();
    if (server.hasArg("cap_duration_ms")) Config::CaptiveDurationMs = server.arg("cap_duration_ms").toInt();
    if (server.hasArg("cap_ssid")) Config::CaptiveSsid = server.arg("cap_ssid");
    if (server.hasArg("cap_pass")) Config::CaptivePass = server.arg("cap_pass");

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
    bool isArtNet = (Config::InputMode == ConfigDefaults::InputMode::ArtNet);
    bool receiving = false;
    if (isArtNet) {
        receiving = (lastArtnetPacketTime > 0) && (millis() - lastArtnetPacketTime < 2000);
        server.send(200, "text/plain", receiving ? "artnet_active" : "artnet_inactive");
    } else {
        receiving = DmxInput::isReceiving(2000);
        server.send(200, "text/plain", receiving ? "dmx_active" : "dmx_inactive");
    }
}

void WebInterface::handleDmxStatus() {
    const bool isDmxMode = (Config::InputMode == ConfigDefaults::InputMode::DMX);
    const bool receiving = DmxInput::isReceiving(2000);
    const unsigned long ageMs = DmxInput::lastPacketAgeMs();

    uint8_t snapshot[512];
    size_t size = DmxInput::copyLastFrame(snapshot, sizeof(snapshot));

    String html = "<!DOCTYPE html><html><head><title>SYNAPSE LNX - DMX Status</title>";
    html += "<style>";
    html += ":root{--bg0:#0b0f14;--bg1:#0f1b2b;--bg2:#082a2f;--panel:#111a24;--panel2:#0c141c;";
    html += "--text:#e7f1f7;--muted:#a8b5c2;--accent:#2bd9d9;--accent2:#ff8a3d;--ok:#2bd957;--bad:#ff5c5c;";
    html += "--line:#1f2d3b;--shadow:0 10px 30px rgba(0,0,0,.35)}";
    html += "body{font-family:'Space Grotesk','Exo 2','Trebuchet MS',sans-serif;margin:20px;";
    html += "color:var(--text);background:";
    html += "radial-gradient(900px 400px at 80% -10%, rgba(43,217,217,.18), transparent 60%),";
    html += "radial-gradient(700px 300px at 10% 0%, rgba(255,138,61,.15), transparent 55%),";
    html += "linear-gradient(140deg,var(--bg0),var(--bg1) 55%,var(--bg2));}";
    html += "h1,h2{color:var(--accent);letter-spacing:.02em}";
    html += "a{color:var(--accent2);text-decoration:none}";
    html += "a:hover{text-decoration:underline}";
    html += "table{border-collapse:collapse;width:100%;margin-bottom:20px;background:var(--panel);box-shadow:var(--shadow);}";
    html += "td,th{border:1px solid var(--line);padding:8px}";
    html += "th{background:linear-gradient(90deg,rgba(43,217,217,.2),rgba(255,138,61,.15));color:var(--text);text-align:left}";
    html += ".badge{display:inline-block;padding:4px 8px;border-radius:999px;color:white;font-weight:600}";
    html += ".ok{background-color:var(--ok)}";
    html += ".bad{background-color:var(--bad)}";
    html += "</style>";
    html += "<meta http-equiv='refresh' content='2'>";
    html += "</head><body>";

    html += "<h1>SYNAPSE LNX - DMX Input Status</h1>";
    html += "<p>Mode: <b>" + String(isDmxMode ? "DMX" : "Art-Net") + "</b></p>";
    html += "<p>Status: <span class='badge " + String(receiving ? "ok" : "bad") + "'>" +
            String(receiving ? "Receiving" : "No Data") + "</span></p>";
    html += "<p>Last Packet Age: " + String(ageMs) + " ms</p>";
    html += "<p>Frame Size: " + String(size) + " channels</p>";

    html += "<h2>Channels 1-32</h2>";
    html += "<table><tr><th>Channel</th><th>Value</th></tr>";
    size_t maxShow = size < 32 ? size : 32;
    for (size_t i = 0; i < maxShow; ++i) {
        html += "<tr><td>" + String(i + 1) + "</td><td>" + String(snapshot[i]) + "</td></tr>";
    }
    if (maxShow == 0) {
        html += "<tr><td colspan='2'>No DMX data captured yet.</td></tr>";
    }
    html += "</table>";

    html += "<p><a href='/'>Back to main</a></p>";
    html += "</body></html>";

    server.send(200, "text/html", html);
}

String WebInterface::getHtml() {
    String html = "<!DOCTYPE html><html><head><title>SYNAPSE LNX Control</title>";
    html += "<meta name='viewport' content='width=device-width, initial-scale=1'>";
    html += "<style>";
    html += ":root{--bg0:#0b0f14;--bg1:#0f1b2b;--bg2:#082a2f;--panel:#111a24;--panel2:#0c141c;";
    html += "--text:#e7f1f7;--muted:#a8b5c2;--accent:#2bd9d9;--accent2:#ff8a3d;--ok:#2bd957;--bad:#ff5c5c;";
    html += "--line:#1f2d3b;--shadow:0 10px 30px rgba(0,0,0,.35)}";
    html += "*{box-sizing:border-box}";
    html += "body{font-family:'Space Grotesk','Exo 2','Trebuchet MS',sans-serif;margin:20px;color:var(--text);";
    html += "background:";
    html += "radial-gradient(900px 400px at 80% -10%, rgba(43,217,217,.18), transparent 60%),";
    html += "radial-gradient(700px 300px at 10% 0%, rgba(255,138,61,.15), transparent 55%),";
    html += "linear-gradient(140deg,var(--bg0),var(--bg1) 55%,var(--bg2));}";
    html += "body:before{content:'';position:fixed;inset:0;pointer-events:none;opacity:.08;";
    html += "background:repeating-linear-gradient(135deg,rgba(43,217,217,.2) 0 1px,transparent 1px 14px)}";
    html += "h1,h2,h3{color:var(--accent);letter-spacing:.02em}";
    html += "h2{margin-top:28px}";
    html += "a{color:var(--accent2);text-decoration:none}";
    html += "a:hover{text-decoration:underline}";
    html += ".hero{display:flex;flex-wrap:wrap;align-items:center;justify-content:space-between;gap:16px;";
    html += "padding:16px 18px;border:1px solid var(--line);border-radius:14px;";
    html += "background:linear-gradient(120deg,rgba(17,26,36,.8),rgba(12,20,28,.9));box-shadow:var(--shadow)}";
    html += ".brand{font-size:1.6rem;font-weight:700;letter-spacing:.12em;text-transform:uppercase}";
    html += ".tagline{font-size:.9rem;letter-spacing:.3em;text-transform:lowercase;color:var(--muted)}";
    html += ".status-pill{white-space:nowrap;font-size:.85rem;padding:6px 12px;border-radius:999px;";
    html += "background:rgba(255,255,255,.06);border:1px solid var(--line)}";
    html += ".subtitle{margin:6px 0 0;color:var(--muted)}";
    html += "table{border-collapse:collapse;width:100%;margin-bottom:20px;background:var(--panel);";
    html += "box-shadow:var(--shadow)}";
    html += "td,th{border:1px solid var(--line);padding:12px}";
    html += "tr:nth-child(even){background-color:rgba(255,255,255,.02)}";
    html += "th{text-align:left;background:linear-gradient(90deg,rgba(43,217,217,.2),rgba(255,138,61,.15));color:var(--text)}";
    html += "input[type=text],input[type=number],select{padding:8px;width:100%;box-sizing:border-box;";
    html += "background:var(--panel2);color:var(--text);border:1px solid var(--line);border-radius:8px}";
    html += "button{background:linear-gradient(120deg,var(--accent),var(--accent2));color:#0b0f14;";
    html += "padding:10px 20px;border:none;border-radius:999px;cursor:pointer;margin:5px 0;font-weight:700}";
    html += "button:hover{filter:brightness(1.08)}";
    html += "#logContainer{background:#05080c;color:#7cffc7;padding:10px;height:300px;overflow-y:scroll;";
    html += "font-family:'JetBrains Mono','Fira Code',monospace;white-space:pre-wrap;border:1px solid var(--line);border-radius:10px}";
    html += ".note{color:var(--muted);font-size:.9rem}";
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
    html += "    const isArtNet = t.startsWith('artnet_');";
    html += "    const active = t.endsWith('_active');";
    html += "    const label = isArtNet ? 'Art-Net' : 'DMX';";
    html += "    s.innerText = label + ': ' + (active ? 'Receiving' : 'No Data');";
    html += "    s.style.backgroundColor = active ? '#2bd957' : '#ff5c5c';";
    html += "    s.style.color = 'white';";
    html += "  });";
    html += "}, 1000);";
    html += "</script>";
    html += "</head><body>";

    html += "<div class='hero'>";
    html += "<div>";
    html += "<div class='brand'>SYNAPSE LNX</div>";
    html += "<div class='tagline'>connect the chaos</div>";
    html += "<div class='subtitle'>Modulares Art-Net/DMX Steuer- und Orchestrierungssystem</div>";
    html += "</div>";
    html += "<div id='artnetStatus' class='status-pill'>Input: Checking...</div>";
    html += "</div>";

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
    html += "<tr><td>Input Source</td><td><select name='input_mode'>";
    html += String("<option value='0'") + (Config::InputMode == ConfigDefaults::InputMode::ArtNet ? " selected" : "") + ">Art-Net</option>";
    html += String("<option value='1'") + (Config::InputMode == ConfigDefaults::InputMode::DMX ? " selected" : "") + ">DMX</option>";
    html += "</select></td></tr>";
    html += "<tr><td colspan='2'><b>Captive Portal</b></td></tr>";
    html += "<tr><td>Enable</td><td><input type='checkbox' name='cap_enabled' value='1'" +
            String(Config::CaptiveEnabled ? " checked" : "") + "></td></tr>";
    html += "<tr><td>Grace (ms)</td><td><input type='text' name='cap_grace_ms' value='" +
            String(Config::CaptiveGraceMs) + "'></td></tr>";
    html += "<tr><td>Duration (ms)</td><td><input type='text' name='cap_duration_ms' value='" +
            String(Config::CaptiveDurationMs) + "'></td></tr>";
    html += "<tr><td>SSID Base</td><td><input type='text' name='cap_ssid' value='" +
            Config::CaptiveSsid + "'></td></tr>";
    html += "<tr><td>Password</td><td><input type='text' name='cap_pass' value='" +
            Config::CaptivePass + "'></td></tr>";
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
                html += "<tr><td>" + s.label + "</td><td>";
                if (s.type == "select") {
                    html += "<select name='" + inputName + "'>";
                    for (const auto &opt: s.options) {
                        html += "<option value='" + opt + "'" + (s.value == opt ? " selected" : "") + ">" + opt +
                                "</option>";
                    }
                    html += "</select>";
                } else {
                    html += "<input type='" + s.type + "' name='" + inputName + "' value='" + s.value + "'>";
                }
                html += "</td></tr>";
            }
            html += "</table>";
        }
    }

    html += "<p class='note'><i>Note: Changing hardware-related settings (Pin, LED Count, Color Order) may require a reboot or cause temporary flickering.</i></p>";
    html += "<button type='submit'>Save All Settings</button>";
    html += "</form>";

    html += "<h2>DMX Input Status</h2>";
    html += "<p><a href='/dmx'>Open DMX status page</a></p>";

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

String WebInterface::getCaptiveHtml() {
    String html = "<!DOCTYPE html><html><head><title>SYNAPSE LNX Setup</title>";
    html += "<meta name='viewport' content='width=device-width, initial-scale=1'>";
    html += "<style>";
    html += ":root{--bg0:#0b0f14;--bg1:#0f1b2b;--bg2:#082a2f;--panel:#111a24;--panel2:#0c141c;";
    html += "--text:#e7f1f7;--muted:#a8b5c2;--accent:#2bd9d9;--accent2:#ff8a3d;--line:#1f2d3b;";
    html += "--shadow:0 10px 30px rgba(0,0,0,.35)}";
    html += "body{font-family:'Space Grotesk','Exo 2','Trebuchet MS',sans-serif;margin:0;color:var(--text);";
    html += "background:";
    html += "radial-gradient(900px 400px at 80% -10%, rgba(43,217,217,.18), transparent 60%),";
    html += "radial-gradient(700px 300px at 10% 0%, rgba(255,138,61,.15), transparent 55%),";
    html += "linear-gradient(140deg,var(--bg0),var(--bg1) 55%,var(--bg2));}";
    html += ".wrap{max-width:560px;margin:0 auto;padding:20px;}";
    html += ".card{background:linear-gradient(120deg,rgba(17,26,36,.9),rgba(12,20,28,.95));";
    html += "border-radius:14px;padding:18px;border:1px solid var(--line);box-shadow:var(--shadow);}";
    html += ".brand{font-size:1.2rem;font-weight:700;letter-spacing:.12em;text-transform:uppercase;}";
    html += ".tagline{font-size:.8rem;letter-spacing:.3em;text-transform:lowercase;color:var(--muted);margin-bottom:10px;}";
    html += "h1{font-size:1.3rem;margin:0 0 8px;color:var(--accent);}";
    html += "label{display:block;font-size:.9rem;margin:12px 0 6px;color:var(--muted);}";
    html += "input,select{width:100%;padding:10px;border:1px solid var(--line);border-radius:8px;font-size:1rem;";
    html += "background:var(--panel2);color:var(--text);}";
    html += "button{margin-top:16px;width:100%;padding:12px;border:0;border-radius:999px;";
    html += "background:linear-gradient(120deg,var(--accent),var(--accent2));color:#0b0f14;font-size:1rem;font-weight:700;}";
    html += ".note{font-size:.85rem;color:var(--muted);margin-top:8px;}";
    html += "</style></head><body><div class='wrap'><div class='card'>";
    html += "<div class='brand'>SYNAPSE LNX</div>";
    html += "<div class='tagline'>connect the chaos</div>";
    html += "<h1>Setup Portal</h1>";
    html += "<p class='note'>Ethernet not ready. Configure network & input mode below.</p>";
    html += "<form action='/save' method='POST'>";
    html += "<label>IP Address</label><input name='ip' value='" + Config::Sys_ip.toString() + "'>";
    html += "<label>Subnet Mask</label><input name='subnet' value='" + Config::Sys_subnet.toString() + "'>";
    html += "<label>Gateway</label><input name='gateway' value='" + Config::Sys_gateway.toString() + "'>";
    html += "<label>Universe</label><input name='universe' type='number' value='" + String(Config::Universe) + "'>";
    html += "<label>Input Source</label><select name='input_mode'>";
    html += String("<option value='0'") + (Config::InputMode == ConfigDefaults::InputMode::ArtNet ? " selected" : "") + ">Art-Net</option>";
    html += String("<option value='1'") + (Config::InputMode == ConfigDefaults::InputMode::DMX ? " selected" : "") + ">DMX</option>";
    html += "</select>";
    html += "<label>Captive Portal Enabled</label><select name='cap_enabled'>";
    html += String("<option value='1'") + (Config::CaptiveEnabled ? " selected" : "") + ">Yes</option>";
    html += String("<option value='0'") + (!Config::CaptiveEnabled ? " selected" : "") + ">No</option>";
    html += "</select>";
    html += "<label>Grace (ms)</label><input name='cap_grace_ms' type='number' value='" + String(Config::CaptiveGraceMs) + "'>";
    html += "<label>Duration (ms)</label><input name='cap_duration_ms' type='number' value='" + String(Config::CaptiveDurationMs) + "'>";
    html += "<label>SSID Base</label><input name='cap_ssid' value='" + Config::CaptiveSsid + "'>";
    html += "<label>Password</label><input name='cap_pass' value='" + Config::CaptivePass + "'>";
    html += "<button type='submit'>Save & Apply</button>";
    html += "</form>";
    html += "<p class='note'>After saving, the device reinitializes networking.</p>";
    html += "</div></div></body></html>";
    return html;
}
