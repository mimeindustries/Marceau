#ifdef ESP8266

#include "Arduino.h"
#include "MarceauWifi.h"

Ticker sta_tick;
//Ticker discovery_tick;

bool MarceauWifi::networkChanged = false;
bool MarceauWifi::wifiScanRequested = false;
bool MarceauWifi::wifiScanReady = false;
MarceauSettings *MarceauWifi::settings;

void WiFiEvent(WiFiEvent_t event) {
  switch(event) {
    case WIFI_EVENT_STAMODE_GOT_IP:
      //if(MarceauWifi::settings->discovery){
      //  send_discovery();
      //  discovery_tick.attach(300, send_discovery);
      //}
      MarceauWifi::networkChanged = true;
      break;
    case WIFI_EVENT_STAMODE_DISCONNECTED:
      //if(MarceauWifi::settings->discovery) discovery_tick.detach();
      MarceauWifi::networkChanged = true;
      break;
  }
}

MarceauWifi::MarceauWifi() {
  enabled = false;
  WiFi.mode(WIFI_OFF);
  wifiScanRequested = false;
}

void MarceauWifi::begin(MarceauSettings * _settings){
  settings = _settings;

  // Subscribe to state change events
  WiFi.onEvent(WiFiEvent);
  
  setupWifi();
  setupDNS();

  // Start the web server
  //new MirobotWeb();

  // Start the WebSocket server
  //beginWebSocket();

  enabled = true;
}

//void MarceauWifi::onMsg(dataHandler h){
//  setWsMsgHandler(h);
//}

void MarceauWifi::defautAPName(char *name){
  uint8_t mac[6];
  WiFi.softAPmacAddress(mac);
  sprintf(name, "Marceau-%02X%02X", mac[4], mac[5]);
}

IPAddress MarceauWifi::getStaIp(){
  return WiFi.localIP();
}

int32_t MarceauWifi::getStaRSSI(){
  return WiFi.RSSI();
}

WiFiMode MarceauWifi::getWifiMode(){
  return WiFi.getMode();
}

void MarceauWifi::setupWifi(){
  // Don't let the ESP SDK persist the settings since we do this ourselves
  WiFi.persistent(false);

  // Put the WiFi into AP_STA mode
  WiFi.mode(WIFI_AP_STA);

  // Reinitialise the WiFi
  WiFi.disconnect();

  // Set the hostname for DHCP
  WiFi.hostname(settings->ap_ssid);

  // Set up the access point
  if(strlen(settings->ap_pass)){
    WiFi.softAP(settings->ap_ssid, settings->ap_pass);
  }else{
    WiFi.softAP(settings->ap_ssid);
  }

  if(strlen(settings->sta_ssid)){
    // Configure the fixed IP if we're not using DHCP
    if(!settings->sta_dhcp && settings->sta_fixedip && settings->sta_fixedgateway && settings->sta_fixednetmask){
      WiFi.config(IPAddress(settings->sta_fixedip), IPAddress(settings->sta_fixedgateway), IPAddress(settings->sta_fixednetmask));
    }

    // Set up the STA connection
    WiFi.begin(settings->sta_ssid, settings->sta_pass);
    // Check if it's connected after 10 seconds
    sta_tick.attach(10, MarceauWifi::staCheck);
  }else{
    WiFi.mode(WIFI_AP);
  }
}

void MarceauWifi::startWifiScan(){
  wifiScanRequested = true;
  WiFi.scanNetworks(true, true);
}

void MarceauWifi::getWifiScanData(ArduinoJson::JsonArray &msg){
  int count = WiFi.scanComplete();
  if(count < 0) return;
  for (int i = 0; i < count; ++i){
    JsonArray& net = msg.createNestedArray();
    net.add(WiFi.SSID(i));
    net.add(WiFi.encryptionType(i) != ENC_TYPE_NONE);
    net.add(WiFi.RSSI(i));
  }
}

void MarceauWifi::setupDNS(){
  dnsServer.setErrorReplyCode(DNSReplyCode::ServerFailure);
  dnsServer.start(53, "local.mirobot.io", IPAddress(192, 168, 4, 1));
}

void MarceauWifi::loop(){
  if(!enabled) return;
  dnsServer.processNextRequest();
  if(wifiScanRequested && WiFi.scanComplete() >= 0){
    wifiScanRequested = false;
    wifiScanReady = true;
  }
}

void MarceauWifi::staCheck(){
  sta_tick.detach();
  if(!(uint32_t)WiFi.localIP()){
    WiFi.mode(WIFI_AP);
  }
}

void MarceauWifi::sendWebSocketMsg(ArduinoJson::JsonObject &outMsg){
  //sendWsMsg(outMsg);
}

#endif
