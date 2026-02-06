# Abnahme-Test (DMX + Captive Portal)

Diese Checkliste ist kurz und praxisnah. Ziel ist: stabile Ethernet/DMX Funktion, sichere Fallbacks, sauberes Captive
Portal.

## 1. Boot ohne Ethernet

- [X] Gerät ohne Ethernet starten.
- [X] Nach ~10s erscheint AP `Kaleo-Setup-XXXXXX`.
- [X] Verbindung möglich, Captive-Portal Seite lädt.
- [X] AP schaltet sich nach der eingestellten Dauer (Default 180s) aus.

## 2. Boot mit Ethernet

- [X] Gerät mit Ethernet starten.
- [X] Kein AP sichtbar.
- [X] Web-UI über statische IP erreichbar.

## 3. Ethernet Link-Drop

- [X] Kabel ziehen -> Log zeigt "Disconnected".
- [X] Kabel rein -> Log zeigt "Connected / Got IP".
- [X] ArtNet läuft wieder.

## 4. Captive-Portal Config speichern

- [X] IP, Gateway, Universe ändern.
- [X] Speichern -> Gerät reinitialisiert, erreichbar unter neuer IP.

## 5. Input-Mode Umschalten

- [X] ArtNet -> DMX: ArtNet ignoriert, DMX verarbeitet.
- [X] DMX -> ArtNet: DMX deaktiviert, ArtNet verarbeitet.
- [X] Keine Fehlermeldungen im Log.

## 6. DMX Signalfluss

- [X] DMX rein -> Fixtures reagieren.
- [X] DMX-Statusseite zeigt Werte.

## 7. DMX Fallback (Blackout)

- [X] DMX Signal trennen.
- [X] Nach ~1s Blackout (bewegende Elemente stoppen).
- [X] DMX Signal wieder an -> normaler Betrieb.

## 8. Performance (ohne Web-UI)

- [X] Web-UI nicht geöffnet -> keine sichtbaren Aussetzer.

Optional:

- [X] /dmx Seite geöffnet -> weiterhin stabil.

## Extended Tests (Stabilität)

- [ ] Soak-Test: 2-4h DMX-Betrieb, keine Aussetzer oder Reboots.
- [ ] DMX-Stress: 512 Kanäle @ 44 Hz, keine Stotterer.
- [ ] Mode-Switch Belastung: mehrfach ArtNet ↔ DMX, keine Hänger.
- [ ] Power-Cycle: mehrere schnelle Neustarts, ETH init + Portal Verhalten ok.
- [ ] Falsche Netz-Config: Gateway/IP absichtlich falsch, Portal startet.
- [ ] Portal Timeout: AP läuft ab und geht sauber aus.
- [ ] Reiner DMX-Betrieb ohne Ethernet: keine unerwünschten Nebeneffekte.
