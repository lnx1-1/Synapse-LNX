# SYNAPSE LNX

connect the chaos

SYNAPSE LNX ist ein modulares, zentrales Steuer- und Orchestrierungssystem für komplexe Installationen.
Es fungiert als künstliches Nervensystem, das Licht, Medien, Maschinen, Sensoren und Netzwerkkomponenten miteinander verbindet.

Modular ArtNet firmware framework for ESP32 with flexible fixture modules - RGB, servos, relays, motors, and more.

## Documentation

- DMX wiring, pass-through, and direction control: `docs/DMX.md`

## Quickstart (Synapse)

1. Power the node and connect Ethernet if available.
2. If Ethernet is not connected, the setup hotspot starts after the captive grace time (default: `10` seconds).
3. Join the hotspot SSID `SYNAPSE-LNX-Setup-XXXXXX` (`XXXXXX` = last 6 MAC chars) with default password `connectchaos`.
4. Open `http://2.0.0.1/` and configure IP, input source, fixture/DMX settings, and captive settings (full web interface is available in captive mode).
5. Captive duration controls how long the hotspot stays open (default: `180` seconds). The portal also closes as soon as Ethernet connects. Set duration to `0` to keep it open until Ethernet connects.
6. For firmware upgrades, open the web UI and use **Firmware Update** to upload a new `.bin` image. The node reboots automatically after a successful flash.

## Build Environments

- Fast/default build: `pio run -e wt32-eth01`
- Strict warnings build: `pio run -e wt32-eth01-strict`

## Work in Progress

This project is currently under active development.

## License

This project is licensed under the GNU General Public License v3.0 - see the [LICENSE](LICENSE) file for details.
