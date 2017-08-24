#ifdef ESP8266
#include "Arduino.h"
#include "MarceauWeb.h"

AsyncWebServer server(80);
extern t_webfiles webFiles[];
extern int webFileCount;

class MarceauRequestHandler: public AsyncWebHandler {
  public:
    MarceauRequestHandler(){}
    bool canHandle(AsyncWebServerRequest *request){
      return request->method() == HTTP_GET;
    }

    void handleRequest(AsyncWebServerRequest *request){
      uint8_t i;
      bool found = false;

      if(request->method() == HTTP_GET){
        String path = request->url();
        if(path.endsWith("/")) path += "index.html";
        
        // Loop through our files to find one that matches
        for(i=0; i<webFileCount; i++){
          if(path == webFiles[i].filename){
            // If it's a match, send the file
            AsyncWebServerResponse *response = request->beginResponse_P(
              200,
              webFiles[i].mime,
              webFiles[i].content,
              webFiles[i].len
            );
            request->send(response);
            
            found = true;
            break;
          }
        }
        // If we get here, send a 404
        if(!found) request->send(404, "text/plain", "Not Found");
      }
    }
};

void MarceauWeb::begin() {
  server.addHandler(new MarceauRequestHandler());
	server.begin();
}

#endif
