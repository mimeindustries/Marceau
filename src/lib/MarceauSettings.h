#ifndef __MarceauSettings_h__
#define __MarceauSettings_h__

// Settings Constants
#define EEPROM_OFFSET 0
#define MAGIC_BYTE_1 0xF0
#define MAGIC_BYTE_2 0x0D
#define SETTINGS_VERSION 22
#define RESET_COUNTER 0

// Artie defaults
#define MOVE_CAL_DEFAULT  0
#define TURN_CAL_DEFAULT  0
#define RIGHT_CAL_DEFAULT  0
#define SLACK_CAL_DEFAULT 5
#define FIRST_BOOT 0

// Line following defaults
#define PID_P_DEFAULT 125
#define PID_I_DEFAULT 0
#define PID_D_DEFAULT 125
#define PID_SAMPLE_TIME_MS_DEFAULT 5 //determine how fast PID calcs are performed, in millisecs

#define IR_CAP_LEFT_DEFAULT   3000
#define IR_CAP_RIGHT_DEFAULT  4095
#define IR_WHITE_THRESHOLD_LEFT_DEFAULT   400
#define IR_WHITE_THRESHOLD_RIGHT_DEFAULT   400
#define IR_BIAS_LEFT_DEFAULT  0
#define IR_BIAS_RIGHT_DEFAULT  0

// Either 0 not dtected, 1 = detected cliff,  set to >1 to ignore digital sensor
#define CLIFF_R_WHITE_THRESHOLD_FRONT   0
#define CLIFF_R_WHITE_THRESHOLD_REAR    0


struct MarceauSettings {
  uint8_t      settingsVersion;
  char         sta_ssid[32+1];    // need space for 0 byte terminator
  char         sta_pass[64+1];
  bool         sta_dhcp;
  uint32_t     sta_fixedip;
  uint32_t     sta_fixedgateway;
  uint32_t     sta_fixednetmask;
  uint32_t     sta_fixeddns1;
  uint32_t     sta_fixeddns2;
  char         ap_ssid[32+1];
  char         ap_pass[64+1];
  bool         discovery;

  // Mirobot/Artie
  int          rst_count;
  unsigned int slackCalibration;
  float        moveCalibration;
  float        turnCalibration;
  int          rightCalibration;
  int          servoPos;
  int          firstboot;
  int          factoryTest;

  // Line following
  uint8_t  irPinsInverted;
  uint16_t irCap[2];
  uint16_t irWhiteThreshold[2];
  uint16_t irBias[2];
  uint16_t Kp, Ki, Kd;
  uint16_t pidTimeInterval = PID_SAMPLE_TIME_MS_DEFAULT;

  // IR Cliff sensors
  uint16_t cliff_irWhiteThreshold[2];

  // Colour remapping
  int8_t  colourRemap[7];  // 0=Black, 1 = white, 2=Red, 3=Green, 4=Blue, 5=Yellow, 6=Purple,  default to 0,1,2,3,4,5,6  i.e. no mapping

  // Colour calibration values
  uint16_t colourCRGB[7][4]; // crgb values for the seven colour sensor colours

};

#endif
