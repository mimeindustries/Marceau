#include "Arduino.h"
#include "MarceauWebSocket.h"
#ifdef ESP8266
  #include <ESP8266WiFi.h>
#endif
#ifdef ESP32
  #include <WiFi.h>
#endif
// ESP32Async/ESPAsyncWebServer (pulls in AsyncTCP or ESPAsyncTCP itself)
#include <ESPAsyncWebServer.h>

AsyncWebServer wsServer(8899);
AsyncWebSocket ws("/websocket");

dataHandler handler = NULL;

void onEvent(AsyncWebSocket * server, AsyncWebSocketClient * client, AwsEventType type, void * arg, uint8_t *data, size_t len){
  if(type == WS_EVT_DATA){
    //data packet
    AwsFrameInfo * info = (AwsFrameInfo*)arg;
    if(info->final && info->index == 0 && info->len == len){
      //the whole message is in a single frame and we got all of it's data
      if(info->opcode == WS_TEXT){
        // ESPAsyncWebServer null-terminates text frame data for us (and
        // writing data[len] ourselves would be past the end of the buffer)
        if(handler) handler((char *)data);
      }
    }
  }
}

void MarceauWebSocket::begin() {
  ws.onEvent(onEvent);
  wsServer.addHandler(&ws);
	wsServer.begin();
}

void MarceauWebSocket::onMsg(dataHandler h){
  handler = h;
}

bool MarceauWebSocket::hasClients(){
  return ws.count() > 0;
}

void MarceauWebSocket::sendText(const char *text, size_t len){
  ws.textAll(text, len);
}
