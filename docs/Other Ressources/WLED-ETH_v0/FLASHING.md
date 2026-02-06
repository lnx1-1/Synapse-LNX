WLED
====

You can easily install the latest WLED release on the board:

- connect the board to your computer using its USB-C port, providing a serial connection
- go to the [WLED install page](https://install.wled.me/)
- select version (e.g. latest release or beta)
- select Ethernet variant
- click on "Install"
- select serial port
- click on "Connect"
- press "INSTALL WLED"
- wait for 2 minutes to flash WLED

If you prefer to use [WLED binary releases](https://github.com/Aircoookie/WLED/releases) and [esptool](https://github.com/espressif/esptool) on the command line instead of the [web interface](https://install.wled.me/), here the commands (set the version to the one wished):

~~~
export WLED_VERSION="0.15.0"
export WLED_PATH=`echo $WLED_VERSION | sed 's/\./_/g'`
wget https://install.wled.me/bin/release_${WLED_PATH}/esp32_bootloader_v4.bin
wget https://install.wled.me/bin/release_${WLED_PATH}/WLED_${WLED_VERSION}_ESP32_audioreactive.bin
esptool.py --port /dev/ttyUSB0 erase_flash
esptool.py --port /dev/ttyUSB0 write_flash 0x0 ./esp32_bootloader_v4.bin
esptool.py --port /dev/ttyUSB0 write_flash 0x10000 WLED_${WLED_VERSION}_ESP32_audioreactive.bin
~~~

The standard WLED firmware (aka. Air Cookie (AC)) with Ethernet includes audio-reactive support, but not DMX (input or output), as of v0.15.0.

MM
==

[MoonModules](https://mm.kno.wled.ge/) (MM) is a fork of WLED.
It is often used to experiment with features that could then be released in WLED.
It includes the DMX support our board takes advantage of.

binary
------

[WLED install](https://wled-install.github.io/) allows to get the binaries, and install them using the web page.
Select board type / software version: ESP32 (4MB Flash, Ethernet, with Audio reactive Usermod).

Use the same instructions from previous WLED section to install the binary on the board.
Or just use the Security & Update -> Manual Update menu from previously installed WLED to upload the firmware binary.

There is no binary with DMX support, but it might include another usermod you might like.

get compiled
------------

[WLED compile](https://wled-compile.github.io/?lang=en) allows getting WLED compiled with custom configuration:

- use "Load configuration from a file"
- use the [configuration with DMX input support](firmware/cfg_wledmm_dmxin.json), or for [DMX output](firmware/cfg_wledmm_dmxout.json).
- go to Step 5, and click on "Start compile/build"
- download and install the binary file as described above, or use Step 6 to flash it

Currently there is no configuration possible supporting DMX input and output at the same time.
The configuration with DMX output does not support the Audio Reactive usermode, as this combination causes [crashes](https://github.com/wled/WLED/issues/4298)
The configuration with DMX input does support Audio Reactive.

compiling yourself
------------------

You can also [compile WLED MM yourself](https://kno.wled.ge/advanced/compiling-wled/) to customize the configuration and add DMX input.

Get the release:

~~~
export MM_RELEASE="0.14.1-beta.30"
wget https://github.com/MoonModules/WLED/archive/refs/tags/v$MM_RELEASE.tar.gz
tar xf v$MM_RELEASE.tar.gz
cd WLED-$MM_RELEASE
~~~

Or if you want the latest source:

~~~
git clone https://github.com/MoonModules/WLED MM_WLED
cd MM_WLED
git checkout mdev
~~~

Let's [define our device](https://wled-compile.github.io/) (e.g. the build environments):

~~~
cat > platformio_override.ini << EOF
[platformio]
; for WLED-ETH v0
default_envs = wled_eth_dmxout, wled_eth_dmxin

; for WLED-ETH v0 with DMX output support
; DMX output crashes with Audio Reactive, so we don't enable it
[env:wled_eth_dmxout]
board = esp32dev
platform = ${esp32.platform}
platform_packages = ${esp32.platform_packages}
build_unflags = ${common.build_unflags}
build_flags = ${common.build_flags_esp32}
  -D WLED_RELEASE_NAME=WLED-ETH
  -D DATA_PINS=14,4,12
  -D BTNPIN=39
  -D WLED_USE_ETHERNET
  -D WLED_AP_SSID='"WLED-ETH"'
  -D WLED_ENABLE_DMX
lib_deps = ${esp32.lib_deps}
monitor_filters = esp32_exception_decoder
board_build.partitions = ${esp32.default_partitions}

; for WLED-ETH v0 with DMX input support
; comes with Audio Reactive
[env:wled_eth]
; V4 adds DMX IN support
extends = esp32_4MB_V4_S_base
build_flags = ${esp32_4MB_V4_S_base.esp32_build_flags}
  -D WLED_RELEASE_NAME=WLED-ETH
  -D DATA_PINS=14,4,12
  -D BTNPIN=39
  -D WLED_USE_ETHERNET
  -D SR_DMTYPE=5 -D I2S_SDPIN=36 -D I2S_WSPIN=15 -D I2S_CKPIN=-1
  -D WLED_AP_SSID='"WLED-ETH"'
lib_deps = ${esp32_4MB_V4_S_base.esp32_lib_deps}
build_unflags = ${esp32_4MB_V4_S_base.build_unflags}
  -D USERMOD_ARTIFX
  -D USERMOD_AUTO_SAVE
  -D USERMOD_DALLASTEMPERATURE
  -D USE_ALT_DISPLAY
  -D USERMOD_FOUR_LINE_DISPLAY
  -D USERMOD_ROTARY_ENCODER_UI
  -D WLED_DISABLE_ESPNOW ; counter build_disable_sync_interfaces
EOF
~~~

To [compile](https://kno.wled.ge/advanced/compiling-wled/) the firmware:

~~~
platformio run
~~~

To [install the firmware](https://kno.wled.ge/basics/install-binary/) over UART:

~~~
# erase to start clean, not always needed but also removes previous configuration
esptool.py --port /dev/ttyUSB0 erase_flash
# .pio/build/wled_chain/bootloader.bin is also compiled, but it does not boot (and is way smaller)
wget https://github.com/Aircoookie/WLED/releases/download/v0.13.1/esp32_bootloader_v4.bin
esptool.py --port /dev/ttyUSB0 write_flash 0 esp32_bootloader_v4.bin
# now flash the compiled WLED
esptool.py --port /dev/ttyUSB0 --baud 921600 write_flash 0x10000 .pio/build/wled_eth_dmxin/firmware.bin
~~~

configuration
=============

If you manufactured and flashed the device yourself, you will also have to configure it.
Once flashed, the device will create a WiFi access point named WLED-AP.
The corresponding password it wled1234.
When connecting to it, you will be redirected to its home page where you can configure the WiFi to connect to your network.

To configure the device manually, for v0.29 and above (version and GPIO numbers are marked on the board):

- go to "TO THE CONTROLS!"
  - go to "Config" gear on top
  - go to "LED Preferences"
  - in the "Maximum PSU Current" field, enter 2500 (mA)
  - under "LED outputs", click twice on "+"
  - in "LED outputs" section 1, set "GPIO" to 14 and "Length" to 30
  - in "LED outputs" section 2, set "GPIO" to 4 and "Length" to 30
  - in "LED outputs" section 3, set "GPIO" to 12 and "Length" to 30
  - in Button 0 GPIO, set "39 (R/O)", "Pushbutton"
  - click on "Save" on top
- go to "User Interface"
  - in the "Server description" field, enter "WLED-ETH"
  - click on "Save" on top
- go to "WiFi Setup"
  - in the "AP SSID" field, enter "WLED-ETH"
  - click on "Save & Connect" on top
- if Audio Reactive is supported
  - go to "Usermods"
    - under "AudioReactive", tick "Enable"
    - click on "Save" on top
  - go to "Usermods"
    - under "AudioReactive" "Digitalmic", set "Type" to "Generic I2S PDM"
    - click on "Save" on top
  - go to "Usermods"
    - under "AudioReactive" "Digitalmic"
    - set "Pin I2S SD" to 36
    - set "Pin I2S WS" to 15
    - set "Pin I2S SCK" to "undefined"
    - click on "Save" on top
- if DMX output is supported, there is nothing to as as it uses hard coded pin 2 for TX
- if DMX input is supported, go to "Sync Interfaces"
  - under DMX Input Pins
  - set "DMX RX" to 35
  - set "DMX TX" to 2
  - set "DMX Enable" to 32
  - click on "Save" on top

The WiFi connection should drop, and the new "WLED-ETH" access point should appear, still with password "wled1234".
To test the LED outputs, go to "TO THE CONTROLS!", select the Effect mode "Rainbow" and see the LEDs connected to the outputs on the bottom of the board cycle through colors.
To test the microphone for audio-reactive effects, select the Effect mode "Gravimeter".
Clamp in your hands, and the LEDs should light up red and fade out.

To not have to configure every field by hand, go to the "Security & Updates" menu.
There you can backup the configuration and presents.
You can then upload them to the next device.

To update the `firmware.bin` firmware over HTTP using the command line (much faster than over UART):

~~~
curl --form "update=@firmware.bin" 'http://4.3.2.1/update'
~~~

To upload the `wled_cfg.json` configuration:

~~~
curl --form "data=@wled_cfg.json;type=application/json;name=data;filename=/cfg.json" 'http://4.3.2.1/upload'
~~~

To upload the `wled_presets.json` presets:

~~~
curl --form "data=@wled_presets.json;type=application/json;name=data;filename=/presets.json" 'http://4.3.2.1/upload'
~~~
