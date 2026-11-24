#ifndef __MirobotWebSocket_h__
#define __MirobotWebSocket_h__

#include "Arduino.h"

// See MarceauWeb.h for why ESPAsyncWebServer isn't included here.

typedef void (* dataHandler) (char *);

class MarceauWebSocket {
  public:
    void begin();
    void onMsg(dataHandler h);
    static bool hasClients();
    static void sendText(const char *text, size_t len);
};

#endif
