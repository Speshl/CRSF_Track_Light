#include <Arduino.h>
#include <CrsfSerial.h>
#include <config.h>
// Pass any HardwareSerial port
// "Arduino" users (atmega328) can not use CRSF_BAUDRATE, as the atmega does not support it
// and should pass 250000, but then also must flash the receiver with RCVR_UART_BAUD=250000
// Also note the atmega only has one Serial, so logging to Serial must be removed
CrsfSerial crsf(Serial2, CRSF_BAUDRATE); //Arduino IOT use Serial1, ESP32 use Serial2, Serial is debug console

CRGB led_strips[NUM_STRIPS][NUM_STRIP_LEDS];

ControlState newState;
ControlState currentState;

unsigned long lastUpdateTime = millis();

bool gotNewPacket = false;

unsigned long triggerTime = 0;
bool triggerComplete = true;


void setup();
void loop();
void getUpdatedControlState();
void clearStrips();
bool getDiff(ControlState a, ControlState b);
void updateAllStrips(int start, int numLeds, CRGB color);
void showUpdates();

int main() {
  init();
  setup();
  while(1) {
      loop();
  }
  return 0;
}

void setup() {
  randomSeed(analogRead(0));

  Serial.begin(460800);
  // while(!Serial){
  //   ;
  // }
  Serial.println("starting up...");

  crsf.begin();
  crsf.onPacketChannels = &getUpdatedControlState;

  FastLED.addLeds<CHIPSET,STRIP_OUT_1,COLOR_ORDER>(led_strips[0],NUM_STRIP_LEDS);
  // FastLED.addLeds<CHIPSET,STRIP_OUT_2,COLOR_ORDER>(led_strips[1],NUM_STRIP_LEDS);
  // FastLED.addLeds<CHIPSET,STRIP_OUT_3,COLOR_ORDER>(led_strips[2],NUM_STRIP_LEDS);

  FastLED.setMaxPowerInVoltsAndMilliamps(VOLTS, MAX_AMPS);
  FastLED.setBrightness(BRIGHTNESS);
  FastLED.clear();
  FastLED.show();

  Serial.println("start up complete");
}


void getUpdatedControlState(){
  //Serial.println("Got Commands");
  int mode = crsf.getChannel(MODE_CHANNEL);
  int trigger = crsf.getChannel(TRIGGER_CHANNEL);

  if(mode > CRSF_MID+CRSF_FLEX_RANGE){
    newState.manualMode = true;
  }else{
    newState.manualMode = false;
  }

  if(trigger > CRSF_MID+CRSF_FLEX_RANGE){
    newState.triggered = true;
  }else{
    newState.triggered = false;
  }

  gotNewPacket = true;
}

void loop() {
  crsf.loop();//detect new data from crsf input, calls getUpdatedControlState on each new packet

  unsigned long currentTime = millis();

  //determine what state we should be in based on the new packet
  if(gotNewPacket){
    gotNewPacket = false;
    if(getDiff(newState, currentState)){
      
      //going from manual to auto
      if(newState.manualMode == false && currentState.manualMode == true){
        clearStrips();
        triggerTime = currentTime;
        triggerComplete = true;
      }

      //going from auto to manual
      if(newState.manualMode == true && currentState.manualMode == false){
        clearStrips();
        triggerTime = 0;
        triggerComplete = true;
      }

      //in manual and just got triggered
      if(newState.manualMode == true && newState.triggered == true && currentState.triggered == false){
        //ClearStrips();
        triggerTime = currentTime;
        triggerComplete = false;
      }

      //in manual and trigger just released
      if(currentState.manualMode == true && currentState.triggered == true && newState.triggered == false){
        clearStrips();
        triggerComplete = true;
      }
      currentState = newState; //apply updates to the state
    }
  }

  //automate trigger in auto mode
  if(!currentState.manualMode){
    if(triggerComplete && currentTime - triggerTime > STAGE_DURATION * (2*NUM_STAGES)){
      Serial.println("Auto Triggering");
      clearStrips();
      triggerTime = currentTime;
      triggerComplete = false;
      currentState.triggered = true;
    }
  }

  //update the strips based on the current stage
  // if(currentState.triggered && !triggerComplete){
  //   for(int i=0; i<NUM_STAGES+1; i++){
  //     if((currentTime - triggerTime > i*STAGE_DURATION) && (currentTime - triggerTime) < (i+1)*STAGE_DURATION){
  //       if(i == 0){//stage 1
  //         Serial.println("First Stage");
  //         updateAllStrips(i*LEDS_PER_STAGE, LEDS_PER_STAGE, FIRST_COLOR);
  //       }else if(i > 0 && i < NUM_STAGES-1){//stage 2-(n-1)
  //         Serial.println("Middle Stage");
  //         updateAllStrips(i*LEDS_PER_STAGE, LEDS_PER_STAGE, MIDDLE_COLOR);
  //       }else if(i == NUM_STAGES-1){//stage n
  //         Serial.println("Last Stage");
  //         updateAllStrips(i*LEDS_PER_STAGE, LEDS_PER_STAGE, LAST_COLOR);
  //       }else{
  //         Serial.println("End Stage");
  //         clearStrips();
  //         triggerComplete = true;
  //         currentState.triggered = false;
  //       }
  //     }
  //   }
  // }

//reversed
  if(currentState.triggered && !triggerComplete){
    for(int i=0; i<NUM_STAGES+1; i++){
      if((currentTime - triggerTime > i*STAGE_DURATION) && (currentTime - triggerTime) < (i+1)*STAGE_DURATION){
        if(i == 0){//stage 1
          Serial.println("First Stage");
          updateAllStrips((NUM_STAGES * LEDS_PER_STAGE) - (i*LEDS_PER_STAGE) - 1, LEDS_PER_STAGE, FIRST_COLOR);
        }else if(i > 0 && i < NUM_STAGES-1){//stage 2-(n-1)
          Serial.println("Middle Stage");
          updateAllStrips((NUM_STAGES * LEDS_PER_STAGE) - (i*LEDS_PER_STAGE) - 1, LEDS_PER_STAGE, MIDDLE_COLOR);
        }else if(i == NUM_STAGES-1){//stage n
          Serial.println("Last Stage");
          updateAllStrips((NUM_STAGES * LEDS_PER_STAGE) - (i*LEDS_PER_STAGE) - 1, LEDS_PER_STAGE, LAST_COLOR);
        }else{
          Serial.println("End Stage");
          clearStrips();
          triggerComplete = true;
          currentState.triggered = false;
        }
      }
    }
  }

}

bool getDiff(ControlState a, ControlState b){
  return a.manualMode != b.manualMode || a.triggered != b.triggered;
}

void updateAllStripsReverse(int start, int numLeds, CRGB color){
  for(int i=0; i<NUM_STRIPS; i++){
    for(int j=start; j>start-numLeds; j--){
      led_strips[i][j] = color;
    }
  }
  showUpdates();
}

void updateAllStrips(int start, int numLeds, CRGB color){
  for(int i=0; i<NUM_STRIPS; i++){
    for(int j=start; j<start-numLeds; j++){
      led_strips[i][j] = color;
    }
  }
  showUpdates();
}

void showUpdates(){
  FastLED.show();
}

void clearStrips(){
  FastLED.clear();
  FastLED.show();
}