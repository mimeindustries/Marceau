#ifndef __Marceau_h__
#define __Marceau_h__

#include "Arduino.h"
#include "lib/CmdProcessor.h"
#include "lib/ArduinoJson/ArduinoJson.h"
#include "lib/MarceauSettings.h"
#include "lib/webFiles.h"
#include <EEPROM.h>
#include <atomic>
#include <memory>


#include "lib/MarceauWifi.h"
#include "lib/MarceauWeb.h"
#include "lib/MarceauWebSocket.h"

#define JSON_BUFFER_LENGTH 2048
#define SERIAL_BUFFER_LENGTH 2048
#define WS_QUEUE_LENGTH 2048   // must be a power of two

#define BUILTIN_CMDS 7

typedef void (* Fn) (ArduinoJson::JsonObject &, ArduinoJson::JsonObject &);
typedef void (* cb) ();

class MarceauBase
{
  public:
    // Pure virtual: these were declared but never defined, which only linked
    // because optimised builds happened to drop the references to MarceauBase's vtable
    virtual void handleWsMsg(char * msg) = 0;
    virtual void getConfig(ArduinoJson::JsonObject &inJson, ArduinoJson::JsonObject &outJson) = 0;
    virtual void setConfig(ArduinoJson::JsonObject &inJson, ArduinoJson::JsonObject &outJson) = 0;
    virtual void resetConfig(ArduinoJson::JsonObject &inJson, ArduinoJson::JsonObject &outJson) = 0;
    virtual ~MarceauBase() {}
};

template <uint8_t CMD_COUNT>
class Marceau: public MarceauBase{
  public:
    Marceau();
    void begin(bool resetSettings=false);
    void enableSerial(Stream &s);
    void loop();
    void addCmd(const char cmd[], Fn func, bool immediate);
    void cmdComplete();
    void resetSettings();
    void notify(const char * status, ArduinoJson::JsonObject &msg);
    MarceauSettings settings;
    boolean blocking;
    long last_command;

    MarceauWifi wifi;
    void enableWifi();
    void setHostname(const char * hostname);
    void setDefaultAPName(const char * apname);
    virtual void handleWsMsg(char * msg);
    virtual void getConfig(ArduinoJson::JsonObject &inJson, ArduinoJson::JsonObject &outJson);
    virtual void setConfig(ArduinoJson::JsonObject &inJson, ArduinoJson::JsonObject &outJson);
    virtual void resetConfig(ArduinoJson::JsonObject &inJson, ArduinoJson::JsonObject &outJson);
    void saveSettings();
    void sendError(const char* msg) { p.sendError(msg); }
    long lastCommand();

  private:
    void wait();
    void initSettings();
    void version(char);
    void initCmds();
    void serialHandler();
    void wsHandler();
    Stream *_s;
    unsigned long last_char;
    
    char serial_buffer[SERIAL_BUFFER_LENGTH];
    int serial_buffer_pos;
    bool wifiEnabled;
    bool serialEnabled;
    CmdProcessor<CMD_COUNT+BUILTIN_CMDS> p;

    // WebSocket messages arrive on the network task (ESP32) or in the SDK's
    // system context (ESP8266). They're queued here and processed from loop()
    // so commands never run concurrently with loop() or where delay() is illegal.
    // The queue is a single-producer/single-consumer ring of null-terminated
    // messages, so several short messages (e.g. a "stop" sent while a slow
    // command is still running) can wait their turn.
    // Only plain atomic loads/stores are used: the ESP8266 has no atomic
    // read-modify-write instructions.
    char ws_queue[WS_QUEUE_LENGTH];
    std::atomic<uint32_t> ws_head;   // total bytes written (producer only)
    std::atomic<uint32_t> ws_tail;   // total bytes consumed (consumer only)
    std::atomic<bool> ws_dropped;
    char ws_msg[JSON_BUFFER_LENGTH]; // message being processed
    void networkNotifier();
    void wifiScanNotifier();
    MarceauWeb webServer;
    MarceauWebSocket socketServer;
    bool penflag;

};

#include "Marceau.tpp"

#endif
