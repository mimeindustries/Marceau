#include <Arduino.h>
#include "MarceauWifi.h"
#ifdef ESP32
  #if __has_include("esp_mac.h")
    #include "esp_mac.h"
  #else
    #include "esp_system.h"
  #endif
#endif

Ticker sta_tick;

volatile bool MarceauWifi::networkChanged = false;
volatile bool MarceauWifi::online = false;
bool MarceauWifi::wifiScanRequested = false;
bool MarceauWifi::wifiScanReady = false;
MarceauSettings *MarceauWifi::settings;

#ifdef ESP8266
WiFiEventHandler gotIpEventHandler, disconnectedEventHandler;
#endif


MarceauWifi::MarceauWifi() {
  enabled = false;
  online = false;
  hostname = NULL;
  _defaultAPName = NULL;
  // Don't touch the WiFi hardware here: this runs as a global constructor,
  // before the SDK is initialised (unsafe on ESP8266). Marceau::begin() turns
  // the WiFi off instead if it isn't enabled.
  wifiScanRequested = false;

}

void MarceauWifi::begin(MarceauSettings * _settings){
  settings = _settings;

  // Subscribe to state change events
#ifdef ESP8266
  gotIpEventHandler = WiFi.onStationModeConnected([](const WiFiEventStationModeConnected &event) {
    MarceauWifi::networkChanged = true;
    MarceauWifi::online = true;
  });
  disconnectedEventHandler = WiFi.onStationModeDisconnected([](const WiFiEventStationModeDisconnected &event) {
    MarceauWifi::networkChanged = true;
    MarceauWifi::online = false;
  });
#endif
#ifdef ESP32
  WiFi.onEvent([](WiFiEvent_t event, WiFiEventInfo_t info) {
    MarceauWifi::networkChanged = true;
    MarceauWifi::online = true;
  }, ARDUINO_EVENT_WIFI_STA_CONNECTED);   // SYSTEM_EVENT_* names were removed in ESP32 core 3.x
  WiFi.onEvent([](WiFiEvent_t event, WiFiEventInfo_t info) {
    MarceauWifi::networkChanged = true;
    MarceauWifi::online = false;
  }, ARDUINO_EVENT_WIFI_STA_DISCONNECTED);
#endif

  setupWifi();
  setupDNS();
  //WiFi.setSleep(false);

  enabled = true;
}

void MarceauWifi::setDefaultAPName(const char * apname){
  _defaultAPName = apname;
}

void MarceauWifi::defaultAPName(char * name){
  // name is settings.ap_ssid
  const size_t len = sizeof(((MarceauSettings*)0)->ap_ssid);
  if(_defaultAPName == NULL){
    uint8_t mac[6] = {0};
    #ifdef ESP32
      // Read the MAC from eFuse: the WiFi driver isn't running yet when the
      // settings are first reset, so WiFi.softAPmacAddress() would fail
      esp_read_mac(mac, ESP_MAC_WIFI_SOFTAP);
    #else
      WiFi.softAPmacAddress(mac);
    #endif
    snprintf(name, len, "Marceau-%02X%02X", mac[4], mac[5]);
  }else{
    strncpy(name, _defaultAPName, len - 1);
    name[len - 1] = 0;
  }
}

IPAddress MarceauWifi::getStaIp(){
  return WiFi.localIP();
}

int32_t MarceauWifi::getStaRSSI(){
  return WiFi.RSSI();
}

#ifdef ESP8266

 WiFiMode MarceauWifi::getWifiMode(){
  return WiFi.getMode();
 }
 
#endif

void MarceauWifi::setupWifi(){
  // Don't let the ESP SDK persist the settings since we do this ourselves
  WiFi.persistent(false);

  // Put the WiFi into AP_STA mode
  WiFi.mode(WIFI_AP_STA);

  // Reinitialise the WiFi
  WiFi.disconnect();

  // Set the hostname for DHCP
  #ifdef ESP8266
    WiFi.hostname(settings->ap_ssid);
  #endif
  #ifdef ESP32
    WiFi.setHostname(settings->ap_ssid);
  #endif
  // Set up the access point
  if(strlen(settings->ap_pass)){
    WiFi.softAP(settings->ap_ssid, settings->ap_pass);
  }else{
    WiFi.softAP(settings->ap_ssid);
  }

//  if(strlen(settings->sta_ssid)){
    // Configure the fixed IP if we're not using DHCP
//    if(!settings->sta_dhcp && settings->sta_fixedip && settings->sta_fixedgateway && settings->sta_fixednetmask){
//      WiFi.config(IPAddress(settings->sta_fixedip), IPAddress(settings->sta_fixedgateway), IPAddress(settings->sta_fixednetmask));
//    }

    // Set up the STA connection
  //  WiFi.begin(settings->sta_ssid, settings->sta_pass);
    // Check if it's connected after 10 seconds
//    sta_tick.attach(10, MarceauWifi::staCheck);
//  }else{
//    WiFi.mode(WIFI_AP);
//  }
}

void MarceauWifi::startWifiScan(){
//  wifiScanRequested = true;
//  WiFi.scanNetworks(true, true);
}

int MarceauWifi::getWifiScanData(ArduinoJson::JsonArray &msg, int done){
  bool rejected;
  bool sent;
  uint8_t chunk_size = 5;
  uint8_t item_index = 0;
  int count = WiFi.scanComplete();
  if(done == count -1) return -1;
  if(count >= 0){
    // We've got some results to send
    for (int i = 0; i < count; i++){
      // Check if we have already sent this SSID and filter if so
      rejected = false;
      for(int j=0; j< i; j++){
        if(WiFi.SSID(i) == WiFi.SSID(j)){
          rejected = true;
          break;
        }
      }
      // Filter this SSID if we've already sent it
      sent = (item_index < done);

      // Add it to the result if we're not filtering it
      if(WiFi.SSID(i) && WiFi.RSSI(i) && !rejected && !sent){
        JsonArray& net = msg.createNestedArray();
        net.add(WiFi.SSID(i));
        #ifdef ESP8266
          net.add(WiFi.encryptionType(i) != ENC_TYPE_NONE);
        #endif
        net.add(WiFi.RSSI(i));
      }

      if(!rejected) item_index++;

      // Return if we've hit this chunk size
      if(item_index > done + chunk_size -1){
        return item_index;
      }
    }
    if(msg.size() == 0){
      return -1;
    }else{
      return item_index;
    }
  }
  // Scan not complete (or failed)
  return -1;
}

void MarceauWifi::setupDNS(){
  if(hostname == NULL){
    hostname = "local.marceau.com";
  }
  dnsServer.setErrorReplyCode(DNSReplyCode::ServerFailure);
  dnsServer.start(53, hostname, IPAddress(192, 168, 4, 1));
}

void MarceauWifi::setHostname(const char * _hostname){
  hostname = _hostname;
}

void MarceauWifi::staCheck(){
  sta_tick.detach();
  if(!(uint32_t)WiFi.localIP()){
    WiFi.mode(WIFI_AP);
  }
}

void MarceauWifi::loop(){
  if(!enabled) return;
  dnsServer.processNextRequest();
  if(wifiScanRequested && WiFi.scanComplete() >= 0){
    wifiScanRequested = false;
    wifiScanReady = true;
  }
}
