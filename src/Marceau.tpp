#ifdef ESP8266
//void handleWsMsg(char * msg){
//  cmdProcessor.processMsg(msg);
//}
#endif //ESP8266

void sendSerialMsg(ArduinoJson::JsonObject &outMsg){
  outMsg.printTo(Serial);
  Serial.println();
}

template <uint8_t CMD_COUNT>
Marceau<CMD_COUNT>::Marceau(){
  serialEnabled = false;
  wifiEnabled = false;
}

template <uint8_t CMD_COUNT>
void Marceau<CMD_COUNT>::begin(){
  this->initSettings();
  // Set up the commands
  this->initCmds();
}

template <uint8_t CMD_COUNT>
void Marceau<CMD_COUNT>::enableSerial(Stream &s){
  _s = &s;
  // Add the output handler for responses
  p.addOutputHandler(sendSerialMsg);
  // Enable serial processing
  serialEnabled = true;
}

template <uint8_t CMD_COUNT>
void Marceau<CMD_COUNT>::addCmd(const char cmd[], Fn func, bool immediate){
  p.addCmd(cmd, func, immediate);
}

template <uint8_t CMD_COUNT>
void Marceau<CMD_COUNT>::cmdComplete(){
  p.sendComplete();
}

template <uint8_t CMD_COUNT>
void Marceau<CMD_COUNT>::enableWifi(){
#ifdef ESP8266
//  wifi.begin(&settings);
//  wifi.onMsg(handleWsMsg);
//  cmdProcessor.addOutputHandler(wifi.sendWebSocketMsg);
//  wifiEnabled = true;
#endif //ESP8266
}

template <uint8_t CMD_COUNT>
void Marceau<CMD_COUNT>::initSettings(){
  if(EEPROM.read(EEPROM_OFFSET) == MAGIC_BYTE_1 && EEPROM.read(EEPROM_OFFSET + 1) == MAGIC_BYTE_2 && EEPROM.read(EEPROM_OFFSET + 2) == SETTINGS_VERSION){
    // We've previously written something valid to the EEPROM
    for (unsigned int t=0; t<sizeof(settings); t++){
      *((char*)&settings + t) = EEPROM.read(EEPROM_OFFSET + 2 + t);
    }
    // Sanity check the values to make sure they look correct
    if(settings.settingsVersion == SETTINGS_VERSION){
      // The values look OK so let's leave them as they are
      return;
    }
  }
  // Either this is the first boot or the settings are bad so let's reset them
  settings.settingsVersion = SETTINGS_VERSION;
#ifdef ESP8266
  settings.sta_ssid[0] = 0;
  settings.sta_pass[0] = 0;
  settings.sta_dhcp = true;
  settings.sta_fixedip = 0;
  settings.sta_fixedgateway = 0;
  //settings.sta_fixednetmask = (uint32_t)IPAddress(255, 255, 255, 0);
  settings.sta_fixeddns1 = 0;
  settings.sta_fixeddns2 = 0;
  //MirobotWifi::defautAPName(settings.ap_ssid);
  settings.ap_pass[0] = 0;
  settings.discovery = true;
#endif //ESP8266
  saveSettings();
}

template <uint8_t CMD_COUNT>
void Marceau<CMD_COUNT>::saveSettings(){
  EEPROM.write(EEPROM_OFFSET, MAGIC_BYTE_1);
  EEPROM.write(EEPROM_OFFSET + 1, MAGIC_BYTE_2);
  for (unsigned int t=0; t<sizeof(settings); t++){
    EEPROM.write(EEPROM_OFFSET + 2 + t, *((char*)&settings + t));
  }
#ifdef ESP8266
  EEPROM.commit();
#endif //ESP8266
}

template <uint8_t CMD_COUNT>
void Marceau<CMD_COUNT>::initCmds(){
}

template <uint8_t CMD_COUNT>
void Marceau<CMD_COUNT>::serialHandler(){
  int s;
  if(!serialEnabled) return;
  // process incoming data
  s = _s->available();
  if (s > 0){
    for(int i = 0; i<s; i++){
      last_char = millis();
      char incomingByte = _s->read();
      // Handle as a stream of commands
      if((incomingByte == '\r' || incomingByte == '\n') && serial_buffer_pos && p.processMsg(serial_buffer)){
        // It's been successfully processed as a line
        serial_buffer_pos = 0;
      }else{
        // Not a line to process so store for processing
        serial_buffer[serial_buffer_pos++] = incomingByte;
        if(serial_buffer_pos == SERIAL_BUFFER_LENGTH) serial_buffer_pos = 0;
        serial_buffer[serial_buffer_pos] = 0;
      }
    }
  }else{
    //reset the input buffer if nothing is received for 1/2 second to avoid things getting messed up
    if(millis() - last_char >= 500){
      serial_buffer_pos = 0;
    }
  }
}

template <uint8_t CMD_COUNT>
void Marceau<CMD_COUNT>::checkReady(){
  /*
  if(cmdProcessor.in_process && ready()){
    cmdProcessor.sendComplete();
  }
  */
}

template <uint8_t CMD_COUNT>
void Marceau<CMD_COUNT>::loop(){
#ifdef ESP8266
  if(wifiEnabled){
    //wifi.run();
  }
#endif //ESP8266
  serialHandler();
  checkReady();
}
