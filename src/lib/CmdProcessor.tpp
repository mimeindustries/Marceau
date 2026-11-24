template <int CMD_COUNT>
CmdProcessor<CMD_COUNT>::CmdProcessor(){
  this->in_process = false;
  current_id[0] = 0;
  tcom = 0;
  // cmd_counter and outputHandlers are deliberately NOT reset here. Sketches
  // register commands from other global objects' constructors (e.g. ArtieMax),
  // and C++ doesn't define whether those run before or after this one. A
  // global Marceau is zero-initialised before any constructor runs, so they
  // start at 0/NULL anyway; resetting them here would wipe commands that were
  // already added. This means Marceau must be a global (or static) object.
}

template <int CMD_COUNT>
bool CmdProcessor<CMD_COUNT>::addOutputHandler(jsonMsgHandler h){
  for(int i = 0; i< OUTPUT_HANDLER_COUNT; i++){
    if(outputHandlers[i] == NULL){
      outputHandlers[i] = h;
      return true;
    }
  }
  return false;
}

template <int CMD_COUNT>
void CmdProcessor<CMD_COUNT>::addCmd(const char cmd[], Fn func, bool immediate){
  if (cmd_counter == CMD_COUNT) {
    Serial.println("Too many commands defined");
    return;
  }
  _cmds[cmd_counter].cmd = cmd;
  _cmds[cmd_counter].func = func;
  _cmds[cmd_counter].immediate = immediate;
  cmd_counter++;
}

template <int CMD_COUNT>
boolean CmdProcessor<CMD_COUNT>::processMsg(char * msg){
  const char* cmd;
  const char* id;
  int cmd_num, i;
  // Heap rather than a 2KB stack buffer: the ESP8266 loop stack is only 4KB
  DynamicJsonBuffer incomingBuffer(256);
  DynamicJsonBuffer outgoingBuffer;
  JsonObject& outMsg = outgoingBuffer.createObject();
  
  JsonObject& inMsg = incomingBuffer.parseObject(msg);
  if(inMsg.success()){
    // Extract the command
    // Extract the ID (must be a string; anything else is treated as no ID)
    id = inMsg["id"].as<const char*>();
    if(id == NULL) id = "";

    // Extract the command (must be a string)
    cmd = inMsg["cmd"].as<const char*>();
    if(cmd == NULL){
      outMsg["msg"] = "No cmd";
      sendResponse("error", outMsg, *id);
      return true;
    }

    // Find the command
    cmd_num = -1;
    for(i = 0; i < cmd_counter; i++){
      if(!strcmp(cmd, _cmds[i].cmd)){
        cmd_num = i;
        break;
      }
    }

    // Process the command
    if(cmd_num >= 0){
      if(_cmds[cmd_num].immediate){
        (_cmds[cmd_num].func)(inMsg, outMsg);
        sendResponse("complete", outMsg, *id);
      }else{
        if(in_process){
          // the previous command hasn't finished, send an error
          outMsg["msg"] = "Previous command not finished";
          sendResponse("error", outMsg, *id);
        }else{
          // Copy the ID before running the command: the command may use outMsg
          // and the id is truncated to fit current_id
          strncpy(current_id, id, sizeof(current_id) - 1);
          current_id[sizeof(current_id) - 1] = 0;
          (_cmds[cmd_num].func)(inMsg, outMsg);
          in_process = true;
          sendResponse("accepted", outMsg, *id);
        }
      }
    }else{
      // the command isn't recognised, send an error
      outMsg["msg"] = "Command not recognised";
      sendResponse("error", outMsg, *id);
    }
  }else{
    //Error parsing
    outMsg["msg"] = "JSON parse error";
    sendResponse("error", outMsg, (const char &)"");
  }
  return true;
}

template <int CMD_COUNT>
unsigned long CmdProcessor<CMD_COUNT>::sendComplete(){
  if(in_process){
    in_process = false;
    StaticJsonBuffer<128> outBuffer;
    JsonObject& outMsg = outBuffer.createObject();
    sendResponse("complete", outMsg, *current_id);
    tcom = millis();
  }
 return tcom;
}

template <int CMD_COUNT>
void CmdProcessor<CMD_COUNT>::sendError(const char* msg) {
  StaticJsonBuffer<128> outBuffer;
  JsonObject& outMsg = outBuffer.createObject();
  outMsg["msg"] = msg;
  sendResponse("error", outMsg, (const char &)"");

}

   


template <int CMD_COUNT>
void CmdProcessor<CMD_COUNT>::sendResponse(const char status[], ArduinoJson::JsonObject &outMsg, const char &id){
  if(strlen(&id)){
    outMsg["id"] = &id;
  }
  outMsg["status"] = status;
  for(int i = 0; i< OUTPUT_HANDLER_COUNT; i++){
    if(outputHandlers[i] != NULL){
      outputHandlers[i](outMsg);
    }
  }
}

template <int CMD_COUNT>
void CmdProcessor<CMD_COUNT>::notify(const char id[], ArduinoJson::JsonObject &outMsg){
  sendResponse("notify", outMsg, *id);
}
