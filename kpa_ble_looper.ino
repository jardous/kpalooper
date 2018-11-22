#include "leds.h"
#include "conn.h"  // bluetooth
#include "buttons.h"






void setup()
{
  Serial.begin(115200);//115200);

  Serial.println("Bluefruit52 Central BLEMidi Example");
  Serial.println("-----------------------------------\n");

  //MIDI.begin(MIDI_CHANNEL_OMNI);
  //MIDI.turnThruOff(); // !!!!

  state = EMPTY;

  leds_setup();
  
  buttons_setup();

  conn_setup();
}



long int tick = millis();
/**
   Callback invoked when Midi received data
   @param midi_svc Reference object to the service where the data
   arrived. In this example it is clientMidi
*/
void blemidi_rx_callback(BLEClientMidi& midi_svc)
{
  Serial.print("[RX]: ");

  while (midi_svc.available())
  {
    Serial.print((char)midi_svc.read(), HEX);
    Serial.print(" ");
  }

  Serial.println();

  tick = millis();
}



bool lastDelayVal = 0;
void loop()
{
  button.check();
  
  ledRainbow.tick();
  //strip.show();

  if ( Bluefruit.Central.connected() )
  {
    // Not discovered yet
    if ( clientMidi.discovered() )
    {
      byte ch;
      char *p;
      // Discovered means in working state
      // Get Serial input and send to Peripheral
      if ( Serial.available() )
      {
        delay(2); // delay a bit for all characters to arrive

        char str[20 + 1] = { 0 };
        Serial.readBytes(str, 20);
        Serial.println(str);

        p = str;
        while (p < str + strlen(str)) {
          ch = strtol(p, &p, 16);
          clientMidi.write(ch);
        }
        delay(1000);
      }

      /*
            kpaSendControlChange(26, lastDelayVal); // delay on
            lastDelayVal = !lastDelayVal;
            delay(1000);
      */
    }
  }
  //delay(1000);
}
