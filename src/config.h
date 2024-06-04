#ifndef CONFIG_H
#define CONFIG_H
#include "Arduino.h"
#include <FastLED.h>

#define STAGE_DURATION 1000
#define NUM_STAGES 5
#define LEDS_PER_STAGE 2

#define FIRST_COLOR CRGB(255,0,0)
#define MIDDLE_COLOR CRGB(255,50,0)
#define LAST_COLOR CRGB(0,255,0)

//led strip configs
#define COLOR_ORDER GRB
#define CHIPSET WS2812B
#define VOLTS 5
#define MAX_AMPS 500 //value in milliamps
#define BRIGHTNESS 60

//crsf configs
#define CRSF_LOW 990
#define CRSF_MID 1500
#define CRSF_HIGH 2011
#define CRSF_FLEX_RANGE 100

//channels configs
#define MODE_CHANNEL 10
#define TRIGGER_CHANNEL 11

#define NUM_STRIPS 3
#define STRIP_OUT_1 13
// #define STRIP_OUT_2 5
// #define STRIP_OUT_3 18
// #define STRIP_OUT_4 4

#define NUM_STRIP_LEDS 50

struct ControlState {
  bool manualMode;//run through automated sequence or triggered by user
  bool triggered; //start light sequence trigger
};

struct StripConfig {
  int numLeds;
  CRGB color;
};

#endif