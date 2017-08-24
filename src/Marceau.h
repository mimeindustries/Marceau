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

class wsHandler
{
public:
  virtual void handleWsMsg(char * msg);
};

template <uint8_t CMD_COUNT>
class Marceau: public wsHandler{
  public:
    Marceau();
    void begin();
    void enableSerial(Stream &s);
    void loop();
    void enableWifi();
    void addCmd(const char cmd[], Fn func, bool immediate);
    void cmdComplete();
    void handleWsMsg(char * msg);
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
    MarceauWeb webServer;
    MarceauWebSocket socketServer;
};

#include "Marceau.tpp"

#endif
