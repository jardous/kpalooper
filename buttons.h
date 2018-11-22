#include <AceButton.h>
#include <ButtonConfig.h>
#include <AdjustableButtonConfig.h>
using namespace ace_button;

// The pin number attached to the button.
const int BUTTON_PIN = A0;


// One button wired to the pin at BUTTON_PIN. Automatically uses the default
// ButtonConfig. The alternative is to call the AceButton::init() method in
// setup() below.
AceButton button(BUTTON_PIN);


typedef enum {EMPTY, RECORD, PLAY, STOP, RESUME} LooperState;
LooperState state;


int same_cc_cnt = 0;
// The event handler for the button.
void handleEvent(AceButton* /* button */, uint8_t eventType, uint8_t buttonState) {

  // Print out a message for all events.
  Serial.print(F("handleEvent(): eventType: "));
  Serial.print(eventType);
  Serial.print(F("; buttonState: "));
  Serial.println(buttonState);

  // Control the LED only for the Pressed and Released events.
  // Notice that if the MCU is rebooted while the button is pressed down, no
  // event is triggered and the LED remains off.
  switch (eventType) {
    case AceButton::kEventClicked:
      same_cc_cnt = 0;
      if (state == EMPTY) {
        state = RECORD;
        Serial.println("RECORDING");
        // Rec/Play/Overdub CC#99 V125, CC#98 V88, CC#06 V0, CC#38 V1 (on press), V0 (on release)
        //sendNrpn(125, 88, 0, 1);
        sendNrpn(16088, 1);  // -- just start recording

        flash();
        uint32_t color = strip.Color(0xFF, 0, 0);
        strip.setPixelColor(0, color);
        strip.show();
      } else if (state == STOP) {
        state = PLAY;
        Serial.println("PLAY");
        //sendNrpn(125, 88, 0, 0);
        //sendNrpn(125, 88, 0, 1);
        sendNrpn(16093, 0);
        sendNrpn(16093, 1);

        flash();
        uint32_t color = strip.Color(0, 0xFF, 0);
        strip.setPixelColor(0, color);
        strip.show();
        
      } else if (state == PLAY) {
        state = RECORD;
        Serial.println("OVERDUB");
        //sendNrpn(125, 88, 0, 1);
        sendNrpn(16088, 1);

        flash();
        uint32_t color = strip.Color(0xFF, 0, 0);
        strip.setPixelColor(0, color);
        strip.show();
        
      } else if (state == RECORD) {
        state = PLAY;
        Serial.println("PLAYING after RECORD");
        //sendNrpn(125, 88, 0, 0);
        sendNrpn(16088, 0);

        flash();
        uint32_t color = strip.Color(0, 0xFF, 0);
        strip.setPixelColor(0, color);
        strip.show();
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
        uint32_t color = strip.Color(0, 0, 0);
        strip.setPixelColor(0, color);
        strip.show();

        return;
      }
      
      if (same_cc_cnt == 2) {
        state = EMPTY;
        Serial.println("state is EMPTY");

        flash();
        uint32_t color = strip.Color(0, 0, 0);
        strip.setPixelColor(0, color);
        strip.show();
        
      } else {
        state = STOP;
        Serial.println("state is STOP");

        flash();
        uint32_t color = strip.Color(0x44, 0x44, 0x44);
        strip.setPixelColor(0, color);
        strip.show();
      }

      break;
  }
}



void buttons_setup() {
  // Button uses the built-in pull up register.
  pinMode(BUTTON_PIN, INPUT_PULLUP);

  // Configure the ButtonConfig with the event handler, and enable all higher
  // level events.
  ButtonConfig* buttonConfig = button.getButtonConfig();
  buttonConfig->setEventHandler(handleEvent);
  buttonConfig->setClickDelay(400);
  
  buttonConfig->setFeature(ButtonConfig::kFeatureClick);
  //buttonConfig->setFeature(ButtonConfig::kFeatureDoubleClick);
  buttonConfig->setFeature(ButtonConfig::kFeatureLongPress);
  //buttonConfig->setFeature(ButtonConfig::kFeatureRepeatPress);
}
