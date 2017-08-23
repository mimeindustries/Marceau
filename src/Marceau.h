#ifndef Marceau_h
#define Marceau_h

#include "Arduino.h"
#include "lib/CmdProcessor.h"
#include "lib/ArduinoJson/ArduinoJson.h"
#include <EEPROM.h>

#ifdef ESP8266
//#include "lib/MirobotWifi.h"
//#include "lib/MirobotWebSocket.h"
#endif

#define SERIAL_BUFFER_LENGTH 180

#define EEPROM_OFFSET 0
#define MAGIC_BYTE_1 0xF0
#define MAGIC_BYTE_2 0x0D
#define SETTINGS_VERSION 1

struct MarceauSettings {
  uint8_t      settingsVersion;
#ifdef ESP8266
  char         sta_ssid[32];
  char         sta_pass[64];
  bool         sta_dhcp;
  uint32_t     sta_fixedip;
  uint32_t     sta_fixedgateway;
  uint32_t     sta_fixednetmask;
  uint32_t     sta_fixeddns1;
  uint32_t     sta_fixeddns2;
  char         ap_ssid[32];
  char         ap_pass[64];
  bool         discovery;
#endif
};

typedef void (* Fn) (ArduinoJson::JsonObject &, ArduinoJson::JsonObject &);

template <uint8_t CMD_COUNT>
class Marceau {
  public:
    Marceau();
    void begin();
    void enableSerial(Stream &s);
    void loop();
    void enableWifi();
    void addCmd(const char cmd[], Fn func, bool immediate);
    void cmdComplete();
    MarceauSettings settings;
    boolean blocking;
  private:
    void wait();
#ifdef ESP8266
    void networkNotifier();
    void wifiScanNotifier();
#endif
    void initSettings();
    void saveSettings();
    void checkReady();
    void version(char);
    void initCmds();
    void serialHandler();
    Stream *_s;
    unsigned long last_char;
    char serial_buffer[SERIAL_BUFFER_LENGTH];
    int serial_buffer_pos;
    bool wifiEnabled;
    bool serialEnabled;
    CmdProcessor<CMD_COUNT> p;
};

#include "Marceau.tpp"

#endif
