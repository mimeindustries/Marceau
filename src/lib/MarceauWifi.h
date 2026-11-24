#ifndef __MarceauWifi_h__
#define __MarceauWifi_h__

#include "Arduino.h"
#ifdef ESP8266
  #include <ESP8266WiFi.h>
  #include "WiFiClient.h"
#endif
#ifdef ESP32
  #include "WiFi.h"
#endif
#include "lib/MarceauSettings.h"
#include "DNSServer.h"
#include "Ticker.h"
#include "lib/ArduinoJson/ArduinoJson.h"

typedef void (* dataHandler) (char *);

struct MarceauSettings;

class MarceauWifi {
  public:
    MarceauWifi();
    static MarceauSettings * settings;
    // Written from the WiFi event task, read from loop()
    static volatile bool online;
    static volatile bool networkChanged;
    static bool wifiScanReady;
    void begin(MarceauSettings *);
    void loop();
    void defaultAPName(char*);
    static IPAddress getStaIp();
    static int32_t getStaRSSI();
    
    #ifdef ESP8266
      static WiFiMode getWifiMode();
    #endif
    
    static void startWifiScan();
    void setupWifi();
    int getWifiScanData(ArduinoJson::JsonArray &, int);
    void setHostname(const char * hostname);
    void setDefaultAPName(const char * apname);
  private:
    bool enabled;
    const char * hostname;
    const char * _defaultAPName;
    static bool wifiScanRequested;
    DNSServer dnsServer;
    void setupDNS();
    static void staCheck();
};
#endif
