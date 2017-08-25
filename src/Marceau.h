#ifndef __Marceau_h__
#define __Marceau_h__

#include "Arduino.h"
#include "lib/CmdProcessor.h"
#include "lib/ArduinoJson/ArduinoJson.h"
#include "lib/MarceauSettings.h"
#include "lib/webFiles.h"
#include <EEPROM.h>

#ifdef ESP8266
#include "lib/MarceauWifi.h"
#include "lib/MarceauWeb.h"
#include "lib/MarceauWebSocket.h"
#endif

#define SERIAL_BUFFER_LENGTH 180

typedef void (* Fn) (ArduinoJson::JsonObject &, ArduinoJson::JsonObject &);

#ifdef ESP8266
class wsHandler
{
public:
  virtual void handleWsMsg(char * msg);
};

template <uint8_t CMD_COUNT>
class Marceau: public wsHandler{
#endif
#ifdef AVR
template <uint8_t CMD_COUNT>
class Marceau{
#endif
  public:
    Marceau();
    void begin();
    void enableSerial(Stream &s);
    void loop();
    void addCmd(const char cmd[], Fn func, bool immediate);
    void cmdComplete();
    MarceauSettings settings;
    boolean blocking;
#ifdef ESP8266
    void enableWifi();
    void setHostname(char * hostname);
    void setDefaultAPName(char * apname);
    void handleWsMsg(char * msg);
#endif
  private:
    void wait();
    void initSettings();
    void saveSettings();
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
#ifdef ESP8266
    char * defaultAPName;
    void networkNotifier();
    void wifiScanNotifier();
    MarceauWifi wifi;
    MarceauWeb webServer;
    MarceauWebSocket socketServer;
#endif
};

#include "Marceau.tpp"

#endif
