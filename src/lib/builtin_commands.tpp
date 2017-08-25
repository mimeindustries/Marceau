#ifdef ESP8266

void _getConfig(ArduinoJson::JsonObject &inJson, ArduinoJson::JsonObject &outJson){
  m->getConfig(inJson, outJson);
}

template <uint8_t CMD_COUNT>
void Marceau<CMD_COUNT>::getConfig(ArduinoJson::JsonObject &inJson, ArduinoJson::JsonObject &outJson){
  JsonObject& msg = outJson.createNestedObject("msg");
  const char *modes[] = {"OFF","STA","AP","APSTA"};
  msg["sta_ssid"] = settings.sta_ssid;
  msg["sta_dhcp"] = settings.sta_dhcp;
  msg["sta_rssi"] = MarceauWifi::getStaRSSI();
  if(!settings.sta_dhcp){
    msg["sta_fixedip"] = IPAddress(settings.sta_fixedip).toString();
    msg["sta_fixedgateway"] = IPAddress(settings.sta_fixedgateway).toString();
    msg["sta_fixednetmask"] = IPAddress(settings.sta_fixednetmask).toString();
  }
  msg["sta_ip"] = MarceauWifi::getStaIp().toString();
  msg["ap_ssid"] = settings.ap_ssid;
  msg["ap_encrypted"] = !!strlen(settings.ap_pass);
  msg["discovery"] = settings.discovery;
  msg["wifi_mode"] = modes[MarceauWifi::getWifiMode()];
}

void _setConfig(ArduinoJson::JsonObject &inJson, ArduinoJson::JsonObject &outJson){
  m->setConfig(inJson, outJson);
}

template <uint8_t CMD_COUNT>
void Marceau<CMD_COUNT>::setConfig(ArduinoJson::JsonObject &inJson, ArduinoJson::JsonObject &outJson){
  IPAddress addr;
  if(!(inJson.containsKey("arg") && inJson["arg"].is<JsonObject&>())) return;

  // Set the SSID to connect to
  if(inJson["arg"].asObject().containsKey("sta_ssid")){
    strcpy(settings.sta_ssid, inJson["arg"]["sta_ssid"]);
  }
  // Set the password for the SSID
  if(inJson["arg"].asObject().containsKey("sta_pass")){
    strcpy(settings.sta_pass, inJson["arg"]["sta_pass"]);
  }
  // Change the name of the built in access point
  if(inJson["arg"].asObject().containsKey("ap_ssid")){
    strcpy(settings.ap_ssid, inJson["arg"]["ap_ssid"]);
  }
  // Set the password for the access point
  if(inJson["arg"].asObject().containsKey("ap_pass")){
    strcpy(settings.ap_pass, inJson["arg"]["ap_pass"]);
  }
  // Set whether to use DHCP
  if(inJson["arg"].asObject().containsKey("sta_dhcp")){
    settings.sta_dhcp = inJson["arg"]["sta_dhcp"];
  }
  // Use a fixed IP address
  if(inJson["arg"].asObject().containsKey("sta_fixedip")){
    addr.fromString(inJson["arg"]["sta_fixedip"].asString());
    settings.sta_fixedip = addr;
  }
  // The DNS server to use for the fixed IP
  if(inJson["arg"].asObject().containsKey("sta_fixedgateway")){
    addr.fromString(inJson["arg"]["sta_fixedgateway"].asString());
    settings.sta_fixedgateway = addr;
  }
  // The netmask to use for the fixed IP
  if(inJson["arg"].asObject().containsKey("sta_fixednetmask")){
    addr.fromString(inJson["arg"]["sta_fixednetmask"].asString());
    settings.sta_fixednetmask = addr;
  }
  // The netmask to use for the fixed IP
  if(inJson["arg"].asObject().containsKey("sta_fixeddns1")){
    addr.fromString(inJson["arg"]["sta_fixeddns1"].asString());
    settings.sta_fixeddns1 = addr;
  }
  // The netmask to use for the fixed IP
  if(inJson["arg"].asObject().containsKey("sta_fixeddns2")){
    addr.fromString(inJson["arg"]["sta_fixeddns2"].asString());
    settings.sta_fixeddns2 = addr;
  }
  // The netmask to use for the fixed IP
  if(inJson["arg"].asObject().containsKey("discovery")){
    settings.discovery = inJson["arg"]["discovery"];
  }
  wifi.setupWifi();
  saveSettings();
}

void _resetConfig(ArduinoJson::JsonObject &inJson, ArduinoJson::JsonObject &outJson){
  m->resetConfig(inJson, outJson);
}

template <uint8_t CMD_COUNT>
void Marceau<CMD_COUNT>::resetConfig(ArduinoJson::JsonObject &inJson, ArduinoJson::JsonObject &outJson){
  settings.settingsVersion = 0;
  saveSettings();
  initSettings();
  wifi.setupWifi();
}

void _startWifiScan(ArduinoJson::JsonObject &inJson, ArduinoJson::JsonObject &outJson){
  MarceauWifi::startWifiScan();
}

void _freeHeap(ArduinoJson::JsonObject &inJson, ArduinoJson::JsonObject &outJson){
  outJson["msg"] = ESP.getFreeHeap();
}
#endif //ESP8266

void _uptime(ArduinoJson::JsonObject &inJson, ArduinoJson::JsonObject &outJson){
  outJson["msg"] = millis();
}

void _ping(ArduinoJson::JsonObject &inJson, ArduinoJson::JsonObject &outJson){}
