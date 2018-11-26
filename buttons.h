#include <AceButton.h>
#include <ButtonConfig.h>
#include <AdjustableButtonConfig.h>
using namespace ace_button;

// The pin number attached to the mode switching button.
const int MODE_BUTTON_PIN = A1;
// The pin number attached to the controller button.
const int BUTTON_PIN = A0;


// One button wired to the pin at BUTTON_PIN. Automatically uses the default
// ButtonConfig. The alternative is to call the AceButton::init() method in
// setup() below.
AceButton buttonMode(MODE_BUTTON_PIN);
AceButton buttonController(BUTTON_PIN);

typedef enum {STOMP, LOOPER} ControllerMode;
ControllerMode mode = STOMP;

typedef enum {EMPTY, RECORD, PLAY, STOP, RESUME} LooperState;
LooperState state = EMPTY;



bool lastX = false;
bool lastDelay = false;
void handleEventStomp(AceButton* /* button */, uint8_t eventType, uint8_t buttonState) {

  // Print out a message for all events.
  Serial.print(F("STOMP handleEvent(): eventType: "));
  Serial.print(eventType);
  Serial.print(F("; buttonState: "));
  Serial.println(buttonState);

  switch (eventType) {
    case AceButton::kEventClicked:
        // Stomp X on/off
        lastX = !lastX;
        sendControlChange(X, lastX);

        flash();
        setLed(0xFF && lastX, 0xFF && lastDelay, 0);
      break;

    case AceButton::kEventLongPressed:
        // Delay on/off
        lastDelay = !lastDelay;
        sendControlChange(X, lastX);

        flash();
        setLed(0xFF && lastX, 0xFF && lastDelay, 0);
      break;
  }
}



int same_cc_cnt = 0;
// The event handler for the button.
void handleEventLooper(AceButton* /* button */, uint8_t eventType, uint8_t buttonState) {

  // Print out a message for all events.
  Serial.print(F("LOOPE handleEvent(): eventType: "));
  Serial.print(eventType);
  Serial.print(F("; buttonState: "));
  Serial.println(buttonState);

  switch (eventType) {
    case AceButton::kEventClicked:
      same_cc_cnt = 0;
      if (state == EMPTY) {
        state = RECORD;
        Serial.println("RECORDING");
        //sendNrpn(125, 88, 0, 1);
        sendNrpn(16088, 1);  // -- just start recording

        flash();
        setLed(0xFF, 0, 0);
      
      } else if (state == STOP) {
        state = PLAY;
        Serial.println("PLAY");
        sendNrpn(16093, 0);
        sendNrpn(16093, 1);

        flash();
        setLed(0, 0xFF, 0);
        
      } else if (state == PLAY) {
        state = RECORD;
        Serial.println("OVERDUB");
        //sendNrpn(125, 88, 0, 1);
        sendNrpn(16088, 1);

        flash();
        setLed(0xFF, 0, 0);
        
      } else if (state == RECORD) {
        state = PLAY;
        Serial.println("PLAYING after RECORD");
        //sendNrpn(125, 88, 0, 0);
        sendNrpn(16088, 0);

        flash();
        setLed(0, 0xFF, 0);
      }
      break;

    case AceButton::kEventLongPressed:
      same_cc_cnt += 1;
      // stop / 3x for erase
      // Stop/Erase: CC#99 V125, CC#98 V89, CC#06 V0, CC#38 V1 (on press), V0 (on release)
      //sendNrpn(125, 89, 0, 1);
      sendNrpn(16094, 1);

      if (state == EMPTY) {
        Serial.println("EMPTY - nothing to do here");
        same_cc_cnt = 0;

        flash();
        setLed(0, 0, 0);

        return;
      }
      
      if (same_cc_cnt == 2) {
        state = EMPTY;
        Serial.println("state is EMPTY");

        flash();
        setLed(0, 0, 0);
        
      } else {
        state = STOP;
        Serial.println("state is STOP");

        flash();
        setLed(0x44, 0x44, 0x44);
      }

      break;
  }
}



ControllerMode lastMode = STOMP;
void handleEventMode(AceButton* /* button */, uint8_t eventType, uint8_t buttonState) {

  // Print out a message for all events.
  Serial.print(F("STOMP handleEvent(): eventType: "));
  Serial.print(eventType);
  Serial.print(F("; buttonState: "));
  Serial.println(buttonState);

  switch (eventType) {
    
    case AceButton::kEventLongPressed:
      
      if (mode == STOMP) {
        mode = LOOPER;
      } else {  // looper
        mode == STOMP;
      }

      setMode(mode);
      
      break;
  }
}



void setMode(ControllerMode newMode) {

  // CONTROL
  // Configure the ButtonConfig with the event handler, and enable all higher
  // level events.
  ButtonConfig* buttonConfig = buttonController.getButtonConfig();
  if (newMode == STOMP)
    buttonConfig->setEventHandler(handleEventStomp);
  else
    buttonConfig->setEventHandler(handleEventLooper);
  //buttonConfig->setClickDelay(400);
  
  buttonConfig->setFeature(ButtonConfig::kFeatureClick);
  //buttonConfig->setFeature(ButtonConfig::kFeatureDoubleClick);
  buttonConfig->setFeature(ButtonConfig::kFeatureLongPress);
  //buttonConfig->setFeature(ButtonConfig::kFeatureRepeatPress);
}


void buttons_setup() {

  // MODE
  // Button uses the built-in pull up register.
  pinMode(MODE_BUTTON_PIN, INPUT_PULLUP);
  // Configure the ButtonConfig with the event handler, and enable all higher
  // level events.
  ButtonConfig* buttonConfig = buttonMode.getButtonConfig();
  buttonConfig->setEventHandler(handleEventMode);
  //buttonConfig->setClickDelay(400);
  
  buttonConfig->setFeature(ButtonConfig::kFeatureClick);
  buttonConfig->setFeature(ButtonConfig::kFeatureLongPress);


  // CONTROL
  pinMode(BUTTON_PIN, INPUT_PULLUP);
  setMode(STOMP);
}