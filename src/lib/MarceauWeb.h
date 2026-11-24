#ifndef __MarceauWeb_h__
#define __MarceauWeb_h__

#include "Arduino.h"

// ESPAsyncWebServer is only included from MarceauWeb.cpp, never from this header:
// it includes ArduinoJson.h whenever an ArduinoJson library is installed, which
// clashes with the ArduinoJson 5 that Marceau bundles if both end up in one file.
class MarceauWeb {
  public:
    void begin();
};

#endif
