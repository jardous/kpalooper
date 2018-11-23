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



void leds_setup() {
  strip.begin();
  strip.setBrightness(LED_BRIGHTNESS);
  strip.show();  // Initialize all pixels to 'off'
}

void setLed(uint32_t rgb) {
  strip.setPixelColor(0, rgb);
  strip.show();
}

void setLed(uint16_t r=0, uint16_t g=0, uint16_t b=0) {
  setLed(strip.Color(r, g, b));
}


struct LEDBounce {
  uint8_t index{0};
  unsigned long m_interval{20};
  unsigned long m_last_executed;

  bool m_active{false};

  uint16_t j{0}, i, inc{0};

  uint32_t m_origColor;

  LEDBounce(bool aActive) : m_active(aActive) {}


  void reset() {
    m_origColor = strip.getPixelColor(index);
    m_active = true;
    j = 0;
    inc = 10;
    m_last_executed = millis();
  }

  void stop() {
    m_active = false;
    setLed(m_origColor);
  }
  
  void tick() {
    if (m_active) {
      if(millis() - m_last_executed > m_interval) {
        m_last_executed += m_interval;

        j = j + inc;
        if (j < 0) { j = 0; inc = -inc; }
        if (j > 255) { j = 255; inc = -inc; }

        strip.setPixelColor(index, strip.Color(0, 0, j));
        strip.show();
      }
    }
  }
} LEDBounce{false};


void leds_loop() {
  
  LEDBounce.tick();
}




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
