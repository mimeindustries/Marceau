Marceau
=======

Marceau is a library that encapsulates all of the generic functionality of Mirobot and extracts it into a useful library to use to build other products on, including the MeArm WiFi and ArtieMax. It has the following functionality:
 * Asynchronous JSON message processing (Mirobot protocol) over both Serial and WiFi
 * WiFi access point setup and management
 * Captive portal (DNS) setup
 * Web server setup, serving files built into the firmware and from a LittleFS filesystem
 * WebSocket server setup

It's easy to add new commands to Marceau for use with your own projects; see the examples.

Supported boards
----------------

ESP32 (Arduino core 2.x or 3.x) and ESP8266.

Dependencies
------------

The web and WebSocket servers use the maintained [ESP32Async](https://github.com/ESP32Async) libraries, which must be installed alongside Marceau (the Arduino Library Manager offers to install them automatically):

 * [ESP Async WebServer](https://github.com/ESP32Async/ESPAsyncWebServer) 3.6.0 or later
 * [Async TCP](https://github.com/ESP32Async/AsyncTCP) 3.3.2 or later (ESP32)
 * [ESP Async TCP](https://github.com/ESP32Async/ESPAsyncTCP) 2.0.0 or later (ESP8266)

Uninstall any older copies by me-no-dev (e.g. `AsyncTCP`, `ESPAsyncTCP`, `ESP Async WebServer` 1.x) so the compiler doesn't pick them up instead.

Marceau bundles its own copy of ArduinoJson 5 and its command API (`ArduinoJson::JsonObject &`) uses it, so a sketch that uses Marceau can't also include ArduinoJson 6 or 7 in the same file. Having ArduinoJson 6 or 7 installed is fine: Marceau keeps the web server (which includes it when present) out of its own headers.

Using Marceau
-------------

```cpp
#include <Marceau.h>

Marceau<2> marcel;   // the number is how many commands you'll add

void setup() {
  marcel.addCmd("version", versionFn, true);   // true: completes immediately
  marcel.addCmd("delay", delayFn, false);      // false: completes later, see cmdComplete()
  marcel.enableSerial(Serial);
  marcel.enableWifi();
  marcel.begin();
}

void loop() {
  marcel.loop();
}
```

 * **Declare the `Marceau` object as a global** (or `static`). Commands may be added from other global objects' constructors, before `marcel` itself has been constructed; that relies on global objects starting zeroed.
 * **Call `marcel.loop()` often.** Serial and WebSocket commands are both run from it.
 * A non-immediate command reports `accepted`; call `marcel.cmdComplete()` from `loop()` when it has finished to send `complete`. Calling it from inside the command function has no effect.
 * Replies go to the stream passed to `enableSerial()` and to every connected WebSocket client (`ws://<device>:8899/websocket`).

Web files and the filesystem
----------------------------

The web server on port 80 serves two sets of files:

 * **Files built into the firmware**, if the sketch defines the table Marceau serves from, typically in a generated `web.h` (see `examples/SimpleDemoWifi`). Without one, Marceau uses an empty table:

   ```cpp
   t_webfiles webFiles[] = { {"/index.html", "text/html", len, data}, ... };
   uint8_t webFileCount = 2;   // must be uint8_t
   ```

   A request for `/x` is also matched by a file named `/x.gz`, which is sent gzip-encoded. `/` serves `/index.html`.
 * **Files on the filesystem**, under the URL prefix `/a/` (e.g. `/a/1.wav` is `/1.wav` on the filesystem).

### LittleFS (replaces SPIFFS)

The filesystem is **LittleFS** on both ESP32 and ESP8266. Earlier versions used SPIFFS, which is deprecated, isn't power-loss safe, and has no upload plugin for Arduino IDE 2.x.

 * **ESP32:** Marceau mounts the partition labelled `spiffs`, which is LittleFS's default label, so existing partition tables work unchanged (keep the data partition's name and subtype as `spiffs`). Up to 30 files can be open at once.
 * **ESP8266:** the filesystem size comes from the board's Flash Size option.
 * **No format on failure.** If the partition doesn't hold a LittleFS filesystem, it isn't mounted and is left untouched. Only the built-in files are served, and the serial console shows:

   ```
   [[[File system info]]]
   Mounted:          NO - upload the data folder
   ```

**Moving a device from SPIFFS to LittleFS:** a SPIFFS image won't mount, so after updating the firmware upload the data folder again as a LittleFS image. Firmware that reads files itself must use `LittleFS` too, e.g. `LittleFS.open()`, or `AudioFileSourceLittleFS` in place of `AudioFileSourceSPIFFS` for ESP8266Audio.

**Uploading the data folder:**

 * **Arduino IDE 2.x:** install the [arduino-littlefs-upload](https://github.com/earlephilhower/arduino-littlefs-upload) plugin, close the Serial Monitor, then press `Ctrl+Shift+P` (`Cmd+Shift+P` on macOS) and run *Upload LittleFS*. It uploads the sketch's `data` folder; check the robot's serial output afterwards shows `Mounted: yes`.
 * **Command line (ESP32):** build an image with `mklittlefs`, which comes with the ESP32 core, then write it to the data partition with `esptool`. The size and offset must match your partition table; for example, for a 0x5E0000 byte partition at 0x210000:

   ```bash
   mklittlefs -c data -b 4096 -p 256 -s 0x5E0000 littlefs.bin
   ```

   ```bash
   esptool --chip esp32 --port COM5 write-flash 0x210000 littlefs.bin
   ```

Changes in 0.1.0
----------------

Changes that affect existing sketches:

 * **Filesystem:** LittleFS replaces SPIFFS (see above); devices need their data folder uploaded again.
 * **Libraries:** the bundled copies of ESPAsyncWebServer, AsyncTCP and ESPAsyncTCP were removed; install the ESP32Async libraries listed under Dependencies.
 * **WebSocket commands run from `loop()`.** They used to run on the network task, concurrently with `loop()` (and on ESP8266 where `delay()` isn't allowed). Incoming messages are queued, up to 2KB in total; if the queue is full or a message is too long, the sender gets `Message dropped: busy or too long`.
 * **`setHostname()`, `setDefaultAPName()` and `notify()` take `const char *`**, so string literals can be passed directly.
 * **Serial replies go to the stream passed to `enableSerial()`**, rather than always to `Serial`.
 * **`webFileCount` must be declared `uint8_t`.** Marceau used to read it as an `int`.
 * **ESP32 Arduino core 3.x** is supported as well as 2.x, and **ESP8266** is supported.

Fixes:

 * Crashes on malformed commands (e.g. a non-string `cmd`), and buffer overflows from long `id` values and from long SSIDs or passwords in `setConfig`.
 * `setConfig` crashed when WiFi wasn't enabled.
 * A message without `cmd` stalled serial input.
 * Settings storage was 2 bytes short.
 * The default AP name could be random on ESP32: the MAC address is now read from eFuse, before WiFi starts.
 * WebSocket replies are no longer truncated at 2KB.

Known limitations
-----------------

 * **WiFi station mode is disabled:** the device always runs as an access point. `setConfig` still stores `sta_ssid` and `sta_pass`, but the connection code in `MarceauWifi.cpp` is commented out.
 * There's no authentication. Anyone who can connect to the device can send any command, so set an access point password.
