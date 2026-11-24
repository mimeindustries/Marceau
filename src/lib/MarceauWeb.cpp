#include "Arduino.h"
#include "MarceauWeb.h"
#include "FS.h"
#ifdef ESP8266
  #include <ESP8266WiFi.h>
#endif
#ifdef ESP32
  #include <WiFi.h>
#endif
#include <LittleFS.h>
// ESP32Async/ESPAsyncWebServer (pulls in AsyncTCP or ESPAsyncTCP itself)
#include <ESPAsyncWebServer.h>
#include "webFiles.h"

AsyncWebServer server(80);
extern t_webfiles webFiles[];

// Defaults for sketches without built-in web files: an empty table. A sketch
// that defines webFiles/webFileCount (e.g. in a generated web.h) overrides
// these, because they're weak symbols.
__attribute__((weak)) t_webfiles webFiles[1] = {{"", "", 0, nullptr}};
__attribute__((weak)) uint8_t webFileCount = 0;
// The sketch defines this as uint8_t (see examples); reading it as an int read
// three bytes of unrelated memory as part of the count. On these little-endian
// chips this also works if a sketch defines it as an int with < 256 files.
extern uint8_t webFileCount;

class MarceauRequestHandler: public AsyncWebHandler {
  public:
    MarceauRequestHandler(){}
    // Must be const to override AsyncWebHandler::canHandle in ESPAsyncWebServer 3.x;
    // without it this silently never matches and every page is a 404
    bool canHandle(AsyncWebServerRequest *request) const override {
      return request->method() == HTTP_GET;
    }

    void handleRequest(AsyncWebServerRequest *request) override {
      uint8_t i;
      bool found = false;
      bool gzip = false;

      if(request->method() == HTTP_GET){
        String path = request->url();
        if(path.endsWith("/")) path += "index.html";

        // Loop through our files to find one that matches
        for(i=0; i<webFileCount; i++){

          if(path + ".gz"== webFiles[i].filename){
            found = true;
            gzip = true;
          }else if(path == webFiles[i].filename){
            found = true;
          }

          if(found){
            // If it's a match, send the file
            AsyncWebServerResponse *response = request->beginResponse_P(
              200,
              webFiles[i].mime,
              webFiles[i].content,
              webFiles[i].len
            );
            if(gzip) response->addHeader("Content-Encoding", "gzip");
            request->send(response);

            break;
          }
        }
        // If we get here, send a 404
        if(!found) request->send(404, "text/plain", "Not Found");
      }
    }
};

void MarceauWeb::begin() {
  // The web files live on LittleFS on both chips: it's power-loss safe and
  // the Arduino IDE 2.x upload plugin (arduino-littlefs-upload) builds
  // LittleFS images; there's no IDE 2.x plugin for SPIFFS.
#ifdef ESP32
  // Mounts the partition labelled "spiffs" (LittleFS's default label, so
  // partition tables don't change). No format on failure, so a failed mount
  // (e.g. an old SPIFFS image) never wipes the flash.
  fs::FS &webFS = LittleFS;
  bool res = LittleFS.begin(false, "/littlefs", 30); // up to 30 open files
  unsigned int totalBytes = res ? LittleFS.totalBytes() : 0;
  unsigned int usedBytes = res ? LittleFS.usedBytes() : 0;
#else
  // ESP8266: auto-format is off so a failed mount never wipes the flash.
  fs::FS &webFS = LittleFS;
  LittleFSConfig fsConfig;
  fsConfig.setAutoFormat(false);
  LittleFS.setConfig(fsConfig);
  bool res = LittleFS.begin();
  FSInfo fs_info = {};
  if(res) LittleFS.info(fs_info);
  unsigned int totalBytes = fs_info.totalBytes;
  unsigned int usedBytes = fs_info.usedBytes;
#endif

  Serial.println("[[[File system info]]]");
  Serial.print("Mounted:          ");
  Serial.println(res ? "yes" : "NO - upload the data folder");
  Serial.print("Total space:      ");
  Serial.print(totalBytes);
  Serial.println(" bytes");
  Serial.print("Total space used: ");
  Serial.print(usedBytes);
  Serial.println(" bytes");
  Serial.println();

  if(res){
    server.serveStatic("/a/", webFS, "/")
          .setLastModified("Wed, 20 Dec 2017 18:35:00 GMT");
  }

  server.addHandler(new MarceauRequestHandler());
  server.begin();
}
