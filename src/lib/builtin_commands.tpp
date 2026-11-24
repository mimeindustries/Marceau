static void _getConfig(ArduinoJson::JsonObject &inJson, ArduinoJson::JsonObject &outJson){
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
  // msg["wifi_mode"] = modes[MarceauWifi::getWifiMode()];
}

// Copy a JSON string value into a fixed-size buffer, ignoring non-strings and
// truncating anything too long so it can't overrun the settings struct
static void copyJsonString(char *dest, size_t destLen, ArduinoJson::JsonVariant val){
  const char *src = val.as<const char*>();
  if(src == NULL) return;
  strncpy(dest, src, destLen - 1);
  dest[destLen - 1] = 0;
}

static void _setConfig(ArduinoJson::JsonObject &inJson, ArduinoJson::JsonObject &outJson){
  m->setConfig(inJson, outJson);
}

template <uint8_t CMD_COUNT>
void Marceau<CMD_COUNT>::setConfig(ArduinoJson::JsonObject &inJson, ArduinoJson::JsonObject &outJson){
  IPAddress addr;
  if(!(inJson.containsKey("arg") && inJson["arg"].is<JsonObject&>())) return;

  // Set the SSID to connect to
  if(inJson["arg"].asObject().containsKey("sta_ssid")){
    copyJsonString(settings.sta_ssid, sizeof(settings.sta_ssid), inJson["arg"]["sta_ssid"]);
  }
  // Set the password for the SSID
  if(inJson["arg"].asObject().containsKey("sta_pass")){
    copyJsonString(settings.sta_pass, sizeof(settings.sta_pass), inJson["arg"]["sta_pass"]);
  }
  // Change the name of the built in access point
  if(inJson["arg"].asObject().containsKey("ap_ssid")){
    copyJsonString(settings.ap_ssid, sizeof(settings.ap_ssid), inJson["arg"]["ap_ssid"]);
  }
  // Set the password for the access point
  if(inJson["arg"].asObject().containsKey("ap_pass")){
    copyJsonString(settings.ap_pass, sizeof(settings.ap_pass), inJson["arg"]["ap_pass"]);
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
  // The gateway address to use for the fixed IP
  if(inJson["arg"].asObject().containsKey("sta_fixedgateway")){
    addr.fromString(inJson["arg"]["sta_fixedgateway"].asString());
    settings.sta_fixedgateway = addr;
  }
  // The netmask to use for the fixed IP
  if(inJson["arg"].asObject().containsKey("sta_fixednetmask")){
    addr.fromString(inJson["arg"]["sta_fixednetmask"].asString());
    settings.sta_fixednetmask = addr;
  }
  // The first address to use for the fixed DNS
  if(inJson["arg"].asObject().containsKey("sta_fixeddns1")){
    addr.fromString(inJson["arg"]["sta_fixeddns1"].asString());
    settings.sta_fixeddns1 = addr;
  }
  // The second address to use for the fixed DNS
  if(inJson["arg"].asObject().containsKey("sta_fixeddns2")){
    addr.fromString(inJson["arg"]["sta_fixeddns2"].asString());
    settings.sta_fixeddns2 = addr;
  }
  // Only touch the WiFi if it has been started (wifi.settings is NULL otherwise)
  if(wifiEnabled) wifi.setupWifi();
  saveSettings();
}

static void _resetConfig(ArduinoJson::JsonObject &inJson, ArduinoJson::JsonObject &outJson){
  m->resetConfig(inJson, outJson);
}

template <uint8_t CMD_COUNT>
void Marceau<CMD_COUNT>::resetConfig(ArduinoJson::JsonObject &inJson, ArduinoJson::JsonObject &outJson){
  resetSettings();
  ESP.restart();
}

static void _startWifiScan(ArduinoJson::JsonObject &inJson, ArduinoJson::JsonObject &outJson){
  MarceauWifi::startWifiScan();
}

static void _freeHeap(ArduinoJson::JsonObject &inJson, ArduinoJson::JsonObject &outJson){
  outJson["msg"] = ESP.getFreeHeap();
}

static void _uptime(ArduinoJson::JsonObject &inJson, ArduinoJson::JsonObject &outJson){
  outJson["msg"] = millis();
}

static void _ping(ArduinoJson::JsonObject &inJson, ArduinoJson::JsonObject &outJson){}
