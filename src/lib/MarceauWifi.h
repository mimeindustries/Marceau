#ifndef __MarceauWifi_h__
#define __MarceauWifi_h__

#include "Arduino.h"
#include <ESP8266WiFi.h>
#include <WiFiClient.h>
#include "lib/MarceauWeb.h"
#include "lib/MarceauWebSocket.h"
#include "lib/MarceauSettings.h"
#include <DNSServer.h>
#include <Ticker.h>
//#include "lib/Discovery.h"
#include "lib/ArduinoJson/ArduinoJson.h"
#include "lib/webFiles.h"

typedef void (* dataHandler) (char *);

struct MarceauSettings;

class MarceauWifi {
  public:
    MarceauWifi();
    void begin(MarceauSettings *);
    void loop();
    void defaultAPName(char*);
    static IPAddress getStaIp();
    static int32_t getStaRSSI();
    static WiFiMode getWifiMode();
    static void startWifiScan();
    void setupWifi();
    static bool networkChanged;
    static bool wifiScanReady;
    static MarceauSettings * settings;
    void getWifiScanData(ArduinoJson::JsonArray &);
    void setHostname(char * hostname);
    void setDefaultAPName(char * apname);
  private:
    bool enabled;
    char * hostname;
    char * _defaultAPName;
    static bool wifiScanRequested;
    DNSServer dnsServer;
    void setupDNS();
    static void staCheck();
};

#endif
