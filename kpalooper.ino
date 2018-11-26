#include "leds.h"
#include "conn.h"  // bluetooth
#include "buttons.h"


void setup()
{
  Serial.begin(115200);

  Serial.println("Bluefruit52 Central BLEMidi Example");
  Serial.println("-----------------------------------\n");
  
  leds_setup();
  
  buttons_setup();

  conn_setup();
  
  LEDBounce.reset();
}


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
}


void loop()
{
  button.check();

  if ( Bluefruit.Central.connected() )
  {
  } else {
    leds_loop();
  }
}
