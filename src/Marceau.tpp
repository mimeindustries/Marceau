static MarceauBase *m;

#include "lib/builtin_commands.tpp"

// Output handler for WebSocket clients. Serialised here, where ArduinoJson 5 is
// available, so MarceauWebSocket.cpp never needs it (see MarceauWeb.h).
static void sendWebSocketMsg(ArduinoJson::JsonObject &outMsg){
  if(!MarceauWebSocket::hasClients()) return;
  // Heap rather than a 2KB stack buffer (the ESP8266 loop stack is only 4KB),
  // which also means long messages are no longer truncated
  size_t len = outMsg.measureLength();
  std::unique_ptr<char[]> buffer(new char[len + 1]);
  outMsg.printTo(buffer.get(), len + 1);
  MarceauWebSocket::sendText(buffer.get(), len);
}

static Stream *serialOut = NULL;

static void sendSerialMsg(ArduinoJson::JsonObject &outMsg){
  // Reply on the stream passed to enableSerial(), not always on Serial
  if(serialOut == NULL) return;
  outMsg.printTo(*serialOut);
  serialOut->println();
}

template <uint8_t CMD_COUNT>
Marceau<CMD_COUNT>::Marceau(){
  serialEnabled = false;
  m = this;
  wifiEnabled = false;
  _s = NULL;
  last_char = 0;
  last_command = 0;
  serial_buffer_pos = 0;
  serial_buffer[0] = 0;
  ws_head = 0;
  ws_tail = 0;
  ws_dropped = false;
  ws_msg[0] = 0;
}

template <uint8_t CMD_COUNT>
void Marceau<CMD_COUNT>::begin(bool _resetSettings){
  // Set up the commands
  initCmds();
  initSettings();
  
  // Force a reset of the settings (useful for development)
  if (_resetSettings) {
    resetSettings();
  }

  if(wifiEnabled){
    // Start the WiFi
    wifi.begin(&settings);
    // Start the web server
    webServer.begin();
    // Start the WebSocket server
    socketServer.begin();
  }else{
    WiFi.mode(WIFI_OFF);
  }

}

template <uint8_t CMD_COUNT>
void Marceau<CMD_COUNT>::enableSerial(Stream &s){
  _s = &s;
  serialOut = &s;
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
  last_command = p.sendComplete();
}

template <uint8_t CMD_COUNT>
void Marceau<CMD_COUNT>::resetSettings(){
  Serial.println("Resetting settings.");
  settings.settingsVersion = SETTINGS_VERSION;

  settings.sta_ssid[0] = 0;
  settings.sta_pass[0] = 0;
  settings.sta_dhcp = true;
  settings.sta_fixedip = 0;
  settings.sta_fixedgateway = 0;
  settings.sta_fixednetmask = (uint32_t)IPAddress(255, 255, 255, 0);
  settings.sta_fixeddns1 = 0;
  settings.sta_fixeddns2 = 0;
  wifi.defaultAPName(settings.ap_ssid);
  settings.ap_pass[0] = 0;
  settings.discovery = true;
  settings.rst_count = RESET_COUNTER;

  // Artie defaults
  settings.slackCalibration = SLACK_CAL_DEFAULT;
  settings.moveCalibration = MOVE_CAL_DEFAULT;
  settings.turnCalibration = TURN_CAL_DEFAULT;
  settings.rightCalibration = RIGHT_CAL_DEFAULT;
  settings.firstboot = FIRST_BOOT;
  settings.factoryTest = 0;

  settings.irPinsInverted = false;
  settings.irCap[0] = IR_CAP_LEFT_DEFAULT;
  settings.irCap[1] = IR_CAP_RIGHT_DEFAULT;
  settings.irWhiteThreshold[0] = IR_WHITE_THRESHOLD_LEFT_DEFAULT;
  settings.irWhiteThreshold[1] = IR_WHITE_THRESHOLD_RIGHT_DEFAULT;
  settings.irBias[0] = IR_BIAS_LEFT_DEFAULT;
  settings.irBias[1] = IR_BIAS_RIGHT_DEFAULT;
  settings.Kp = PID_P_DEFAULT;
  settings.Ki = PID_I_DEFAULT;
  settings.Kd = PID_D_DEFAULT;
  settings.pidTimeInterval = PID_SAMPLE_TIME_MS_DEFAULT;

  settings.cliff_irWhiteThreshold[0] = CLIFF_R_WHITE_THRESHOLD_FRONT;
  settings.cliff_irWhiteThreshold[1] = CLIFF_R_WHITE_THRESHOLD_REAR;

  // 0=Red, 1=Green, 2=Blue, 3=Yellow, 4=Purple,  default to 0,1,2,3,4  i.e. no mapping
  settings.colourRemap[0] = 0;
  settings.colourRemap[1] = 1;
  settings.colourRemap[2] = 2;
  settings.colourRemap[3] = 3;
  settings.colourRemap[4] = 4;
  settings.colourRemap[5] = 5;
  settings.colourRemap[6] = 6;

  settings.colourCRGB[0][0] = 831;
  settings.colourCRGB[0][1] = 849;
  settings.colourCRGB[0][2] = 813;
  settings.colourCRGB[0][3] = 819;

  settings.colourCRGB[1][0] = 410;
  settings.colourCRGB[1][1] = 754;
  settings.colourCRGB[1][2] = 305;
  settings.colourCRGB[1][3] = 315;

  settings.colourCRGB[2][0] = 353;
  settings.colourCRGB[2][1] = 367;
  settings.colourCRGB[2][2] = 299;
  settings.colourCRGB[2][3] = 291;

  settings.colourCRGB[3][0] = 508;
  settings.colourCRGB[3][1] = 372;
  settings.colourCRGB[3][2] = 409;
  settings.colourCRGB[3][3] = 571;

  settings.colourCRGB[4][0] = 592;
  settings.colourCRGB[4][1] = 917;
  settings.colourCRGB[4][2] = 577;
  settings.colourCRGB[4][3] = 357;

  settings.colourCRGB[5][0] = 353;
  settings.colourCRGB[5][1] = 367;
  settings.colourCRGB[5][2] = 299;
  settings.colourCRGB[5][3] = 291;

  settings.colourCRGB[6][0] = 210;
  settings.colourCRGB[6][1] = 269;
  settings.colourCRGB[6][2] = 71;
  settings.colourCRGB[6][3] = 112;

  saveSettings();
}

template <uint8_t CMD_COUNT>
void Marceau<CMD_COUNT>::initSettings(){
  // The settings are stored after the two magic bytes, so reserve room for them too
  EEPROM.begin(EEPROM_OFFSET + 2 + sizeof(MarceauSettings));


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
  // Either this is the first boot, am updated settings version, or the settings are bad so let's reset them
  resetSettings();
}

template <uint8_t CMD_COUNT>
void Marceau<CMD_COUNT>::saveSettings(){
  EEPROM.write(EEPROM_OFFSET, MAGIC_BYTE_1);
  EEPROM.write(EEPROM_OFFSET + 1, MAGIC_BYTE_2);
  for (unsigned int t=0; t<sizeof(settings); t++){
    EEPROM.write(EEPROM_OFFSET + 2 + t, *((char*)&settings + t));
  }
  EEPROM.commit();
}

template <uint8_t CMD_COUNT>
void Marceau<CMD_COUNT>::initCmds(){
  // Set up the built in commands
  p.addCmd("ping",          _ping,          true);
  p.addCmd("uptime",        _uptime,        true);
  p.addCmd("freeHeap",      _freeHeap,      true);
  p.addCmd("getConfig",     _getConfig,     true);
  p.addCmd("setConfig",     _setConfig,     true);
  p.addCmd("resetConfig",   _resetConfig,   true);
  p.addCmd("startWifiScan", _startWifiScan, true);
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



static void handleWs(char *msg){
  m->handleWsMsg(msg);
}

template <uint8_t CMD_COUNT>
void Marceau<CMD_COUNT>::handleWsMsg(char * msg){
  // Called from the network context: just queue the message for loop()
  static_assert((WS_QUEUE_LENGTH & (WS_QUEUE_LENGTH - 1)) == 0, "WS_QUEUE_LENGTH must be a power of two");
  uint32_t len = strlen(msg) + 1; // include the terminator
  uint32_t head = ws_head;
  uint32_t used = head - ws_tail;
  if(len > sizeof(ws_msg) || len > WS_QUEUE_LENGTH - used){
    ws_dropped = true;
    return;
  }
  for(uint32_t i = 0; i < len; i++){
    ws_queue[(head + i) & (WS_QUEUE_LENGTH - 1)] = msg[i];
  }
  ws_head = head + len; // publish only after the bytes are written
}

template <uint8_t CMD_COUNT>
void Marceau<CMD_COUNT>::wsHandler(){
  if(ws_dropped){
    ws_dropped = false;
    p.sendError("Message dropped: busy or too long");
  }
  uint32_t tail = ws_tail;
  if(tail == ws_head) return;
  // Copy the oldest message out and free its space straight away, so new
  // messages can be queued while this one is being processed
  uint32_t i = 0;
  char c;
  do {
    c = ws_queue[(tail + i) & (WS_QUEUE_LENGTH - 1)];
    ws_msg[i++] = c;
  } while(c != 0);
  ws_tail = tail + i;

  Serial.println(ws_msg);
  p.processMsg(ws_msg);
  last_command = millis();
}

template <uint8_t CMD_COUNT>
void Marceau<CMD_COUNT>::enableWifi(){
  socketServer.onMsg(handleWs);
  p.addOutputHandler(sendWebSocketMsg);
  wifiEnabled = true;
}

template <uint8_t CMD_COUNT>
void Marceau<CMD_COUNT>::setHostname(const char * hostname){
  wifi.setHostname(hostname);
}

template <uint8_t CMD_COUNT>
void Marceau<CMD_COUNT>::setDefaultAPName(const char * apname){
  wifi.setDefaultAPName(apname);
}

template <uint8_t CMD_COUNT>
void Marceau<CMD_COUNT>::networkNotifier(){
  if(!MarceauWifi::networkChanged) return;
  MarceauWifi::networkChanged = false;
  StaticJsonBuffer<500> outBuffer;
  JsonObject& outMsg = outBuffer.createObject();
  _getConfig(outMsg, outMsg);
  p.notify("network", outMsg);
}

template <uint8_t CMD_COUNT>
void Marceau<CMD_COUNT>::wifiScanNotifier(){
//  int done = 0;
//  if(!MarceauWifi::wifiScanReady) return;
//  while(true){
//    MarceauWifi::wifiScanReady = false;
//    StaticJsonBuffer<500> outBuffer;
//    JsonObject& outMsg = outBuffer.createObject();
//    JsonArray& msg = outMsg.createNestedArray("msg");
//    done = wifi.getWifiScanData(msg, done);
//    if(done >= 0){
//      p.notify("wifiScan", outMsg);
//    }else{
//      break;
//    }
//  }
}


template <uint8_t CMD_COUNT>
void Marceau<CMD_COUNT>::notify(const char * status, ArduinoJson::JsonObject &msg){
  p.notify(status, msg);
}

template <uint8_t CMD_COUNT>
long Marceau<CMD_COUNT>::lastCommand(){
if (!last_command){
  last_command = 1;
} 

return last_command;
}

template <uint8_t CMD_COUNT>
void Marceau<CMD_COUNT>::loop(){
  if(wifiEnabled){
    wifi.loop();
    wsHandler();
    networkNotifier();
    wifiScanNotifier();
  }
  serialHandler();
}
