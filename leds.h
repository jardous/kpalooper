#include <Adafruit_NeoPixel.h>
#ifdef __AVR__
  #include <avr/power.h>
#endif

#define NEO_PIN 15
#define NEO_COUNT 1

#define LED_BRIGHTNESS 20
// Parameter 1 = number of pixels in strip
// Parameter 2 = Arduino pin number (most are valid)
// Parameter 3 = pixel type flags, add together as needed:
//   NEO_KHZ800  800 KHz bitstream (most NeoPixel products w/WS2812 LEDs)
//   NEO_KHZ400  400 KHz (classic 'v1' (not v2) FLORA pixels, WS2811 drivers)
//   NEO_GRB     Pixels are wired for GRB bitstream (most NeoPixel products)
//   NEO_RGB     Pixels are wired for RGB bitstream (v1 FLORA pixels, not v2)
//   NEO_RGBW    Pixels are wired for RGBW bitstream (NeoPixel RGBW products)
Adafruit_NeoPixel strip = Adafruit_NeoPixel(NEO_COUNT, NEO_PIN, NEO_RGB + NEO_KHZ800);

// IMPORTANT: To reduce NeoPixel burnout risk, add 1000 uF capacitor across
// pixel power leads, add 300 - 500 Ohm resistor on first pixel's data input
// and minimize distance between Arduino and first pixel.  Avoid connecting
// on a live circuit...if you must, connect GND first.


// Input a value 0 to 255 to get a color value.
// The colours are a transition r - g - b - back to r.
uint32_t Wheel(byte WheelPos) {
  WheelPos = 255 - WheelPos;
  if(WheelPos < 85) {
    return strip.Color(255 - WheelPos * 3, 0, WheelPos * 3);
  }
  if(WheelPos < 170) {
    WheelPos -= 85;
    return strip.Color(0, WheelPos * 3, 255 - WheelPos * 3);
  }
  WheelPos -= 170;
  return strip.Color(WheelPos * 3, 255 - WheelPos * 3, 0);
}


void colorWipe(uint32_t c, uint8_t wait) {
  for(uint16_t i=0; i<strip.numPixels(); i++) {
    strip.setPixelColor(i, c);
    strip.show();
    delay(wait);
  }
}

void rainbow(uint8_t wait) {
  uint16_t i, j;

  for(j=0; j<256; j++) {
    for(i=0; i<strip.numPixels(); i++) {
      strip.setPixelColor(i, Wheel((i+j) & 255));
    }
    strip.show();
    delay(wait);
  }
}

// Slightly different, this makes the rainbow equally distributed throughout
void rainbowCycle(uint8_t wait) {
  uint16_t i, j;

  for(j=0; j<256*5; j++) { // 5 cycles of all colors on wheel
    for(i=0; i< strip.numPixels(); i++) {
      strip.setPixelColor(i, Wheel(((i * 256 / strip.numPixels()) + j) & 255));
    }
    strip.show();
    delay(wait);
  }
}

//Theatre-style crawling lights.
void theaterChase(uint32_t c, uint8_t wait) {
  for (int j=0; j<10; j++) {  //do 10 cycles of chasing
    for (int q=0; q < 3; q++) {
      for (uint16_t i=0; i < strip.numPixels(); i=i+3) {
        strip.setPixelColor(i+q, c);    //turn every third pixel on
      }
      strip.show();

      delay(wait);

      for (uint16_t i=0; i < strip.numPixels(); i=i+3) {
        strip.setPixelColor(i+q, 0);        //turn every third pixel off
      }
    }
  }
}

//Theatre-style crawling lights with rainbow effect
void theaterChaseRainbow(uint8_t wait) {
  for (int j=0; j < 256; j++) {     // cycle all 256 colors in the wheel
    for (int q=0; q < 3; q++) {
      for (uint16_t i=0; i < strip.numPixels(); i=i+3) {
        strip.setPixelColor(i+q, Wheel( (i+j) % 255));    //turn every third pixel on
      }
      strip.show();

      delay(wait);

      for (uint16_t i=0; i < strip.numPixels(); i=i+3) {
        strip.setPixelColor(i+q, 0);        //turn every third pixel off
      }
    }
  }
}

void clearAllLeds() {
  // clear the leds
  for (uint16_t i=0; i < strip.numPixels(); i++) {
    strip.setPixelColor(i, strip.Color(0, 0, 0));
  }
}


void leds_setup() {
  // This is for Trinket 5V 16MHz, you can remove these three lines if you are not using a Trinket
  #if defined (__AVR_ATtiny85__)
    if (F_CPU == 16000000) clock_prescale_set(clock_div_1);
  #endif
  // End of trinket special code


  strip.begin();
  strip.setBrightness(LED_BRIGHTNESS);
  strip.show(); // Initialize all pixels to 'off'
}


void leds_loop() {
/*
  // Some example procedures showing how to display to the pixels:
  colorWipe(strip.Color(255, 0, 0), 50); // Red
  colorWipe(strip.Color(0, 255, 0), 50); // Green
  colorWipe(strip.Color(0, 0, 255), 50); // Blue
//colorWipe(strip.Color(0, 0, 0, 255), 50); // White RGBW
  // Send a theater pixel chase in...
  theaterChase(strip.Color(127, 127, 127), 50); // White
  theaterChase(strip.Color(127, 0, 0), 50); // Red
  theaterChase(strip.Color(0, 0, 127), 50); // Blue
*/
  rainbow(20);
  rainbowCycle(20);
  theaterChaseRainbow(50);
}


struct sLedTick {
  uint8_t index{3};
  unsigned long m_interval{20};
  uint16_t m_inc{0x11};
  unsigned long m_last_executed;
  long m_value{0xff};
  bool m_active{false};

  sLedTick(bool aActive) : m_active(aActive) {}

  void reset() {
    //Serial.println("Reset");
    m_active = true;
    m_value = 0xff;
    m_last_executed = millis();
  }

  void tick() {
    if (m_active) {
      if(millis() - m_last_executed > m_interval) {
        m_last_executed += m_interval;
        
        strip.setPixelColor(index, strip.Color(m_value, 0, 0));
        
        m_value -= m_inc;
        if (m_value < 0) {
          m_active = false;
          m_value = 0;
        }
      }
    }
  }
} ledTick{true};


struct sLedRainbow {
  uint8_t index{0};
  unsigned long m_interval{10};
  unsigned long m_last_executed;
  uint16_t m_value{0xff};
  bool m_active{false};

  sLedRainbow(bool am_active) : m_active(am_active) {}

  void reset() {
    m_active = true;
    m_value = 0;
    m_last_executed = millis();
  }

  void tick() {
    if (m_active) {
      if(millis() - m_last_executed > m_interval) {
        m_last_executed += m_interval;
        
        strip.setPixelColor(index, Wheel(m_value & 255));
        
        ++m_value;
        if (m_value > 255) m_value = 0;
        //strip.show();
      }
    }
  }
} ledRainbow{true};


struct AllLedRainbowDisconnected {
  unsigned long m_interval{5};
  unsigned long m_last_executed;

  bool m_active{false};

  uint16_t j{0}, i;

  AllLedRainbowDisconnected(bool aActive) : m_active(aActive) {}


  void reset() {
    m_active = true;
    j = 0;
    m_last_executed = millis();
  }
  
  void tick() {
    if (m_active) {
      if(millis() - m_last_executed > m_interval) {
        m_last_executed += m_interval;
      
        for(i=0; i< strip.numPixels(); i++) {
          strip.setPixelColor(i, Wheel(((i * 256 / strip.numPixels()) + j) & 255));
        }
        
        ++j;
        if (j > 256*5) j = 0;
      }
    }
  }
} ledDisconnectedRainbow{true};


void flash(int repeats=2) {
  uint32_t orig_color = strip.getPixelColor(0);

  for (int i=0; i<repeats; ++i) {
    strip.setPixelColor(0, strip.Color(0xFF, 0xFF, 0xFF));
    strip.show();
    delay(50);
    strip.setPixelColor(0, orig_color);
    strip.show();
    delay(50);
  }
}
