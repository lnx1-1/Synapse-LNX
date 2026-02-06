//
// Created by Codex on 06.02.2026.
//

#include "CaptivePortal.h"
#include "ETH_Connector.h"
#include "WebInterface.h"
#include "Config/config.h"
#include <DNSServer.h>
#include <WiFi.h>

namespace {
    const IPAddress kApIp(2, 0, 0, 1);
    const IPAddress kApNetmask(255, 255, 255, 0);

    DNSServer dnsServer;
    bool active = false;
    unsigned long bootMs = 0;
    unsigned long startMs = 0;

    String buildSsid() {
        String mac = WiFi.macAddress();
        mac.replace(":", "");
        if (mac.length() >= 6) {
            mac = mac.substring(mac.length() - 6);
        }
        String base = Config::CaptiveSsid;
        base.trim();
        if (base.length() == 0) {
            base = "SYNAPSE-LNX-Setup";
        }
        return base + "-" + mac;
    }

    String getPassword() {
        String pass = Config::CaptivePass;
        pass.trim();
        if (pass.length() < 8) {
            Log.warningln("[CAPTIVE] Password too short, using default.");
            return "connectchaos";
        }
        return pass;
    }
}

void CaptivePortal::begin(uint32_t graceMs, uint32_t durationMs) {
    (void) graceMs;
    (void) durationMs;
    bootMs = millis();
    active = false;
}

bool CaptivePortal::isActive() {
    return active;
}

void CaptivePortal::start() {
    if (active) {
        return;
    }

    WiFi.mode(WIFI_AP);
    WiFi.softAPConfig(kApIp, kApIp, kApNetmask);
    const String ssid = buildSsid();
    const String pass = getPassword();
    WiFi.softAP(ssid.c_str(), pass.c_str());

    dnsServer.start(53, "*", kApIp);
    WebInterface::setCaptivePortalActive(true);

    active = true;
    startMs = millis();

    Log.infoln("[CAPTIVE] AP started: %s (pass: %s) IP: %s",
               ssid.c_str(), pass.c_str(), kApIp.toString().c_str());
}

void CaptivePortal::stop() {
    if (!active) {
        return;
    }
    dnsServer.stop();
    WiFi.softAPdisconnect(true);
    WiFi.mode(WIFI_MODE_NULL);
    WebInterface::setCaptivePortalActive(false);
    active = false;
    Log.infoln("[CAPTIVE] AP stopped");
}

void CaptivePortal::update() {
    const unsigned long now = millis();

    if (!Config::CaptiveEnabled) {
        if (active) {
            stop();
        }
        return;
    }

    if (active) {
        dnsServer.processNextRequest();
        if (ETH_Connector::isConnected()) {
            stop();
            return;
        }
        if (Config::CaptiveDurationMs > 0 && (now - startMs) > Config::CaptiveDurationMs) {
            stop();
            return;
        }
        return;
    }

    if (!ETH_Connector::isConnected() && Config::CaptiveGraceMs > 0 &&
        (now - bootMs) > Config::CaptiveGraceMs) {
        start();
    }
}
