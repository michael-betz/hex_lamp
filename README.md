# hex_lamp
Geometric lamp casting wild shadows

## Firmware
Based on 3 perlin noise generators doing a slow random walk, controlling H, S and V of each LED.

  * Install [PlatformIO](https://platformio.org/) 
  * Edit `firmware/platform.ini` to enter the wifi credentials.
  * Connect USB-serial cable to ESP8266
  * Hold the flash button 
  * Build / upload the firmware like this:

```bash
$ platformio run -t upload -t monitor
Processing hexlamp (platform: espressif8266; board: esp01; framework: arduino)
--------------------------------------------------------------------------------------------------------
Verbose mode can be enabled via `-v, --verbose` option
CONFIGURATION: https://docs.platformio.org/page/boards/espressif8266/esp01.html
PLATFORM: Espressif 8266 2.3.2 > Espressif Generic ESP8266 ESP-01 512k
HARDWARE: ESP8266 80MHz, 80KB RAM, 512KB Flash
PACKAGES: 
 - framework-arduinoespressif8266 2.20603.191216 (2.6.3) 
 - tool-esptool 1.413.0 (4.13) 
 - tool-esptoolpy 1.20800.0 (2.8.0) 
 - tool-mkspiffs 1.200.0 (2.0) 
 - toolchain-xtensa 2.40802.191122 (4.8.2)
Converting main.ino
LDF: Library Dependency Finder -> http://bit.ly/configure-pio-ldf
LDF Modes: Finder ~ chain, Compatibility ~ soft
Found 30 compatible libraries
Scanning dependencies...
Dependency Graph
|-- <Adafruit NeoPixel> 1.3.5
|-- <ESP8266WiFi> 1.0
Building in release mode
...
RAM:   [===       ]  33.6% (used 27552 bytes from 81920 bytes)
Flash: [======    ]  63.2% (used 274236 bytes from 434160 bytes)
Creating BIN file ".pio/build/hexlamp/firmware.bin" using ".pio/build/hexlamp/firmware.elf"
Configuring upload protocol...
AVAILABLE: espota, esptool
CURRENT: upload_protocol = esptool
Looking for upload port...
Use manually specified: /dev/ttyUSB0
Uploading .pio/build/hexlamp/firmware.bin
esptool.py v2.8
...
```
