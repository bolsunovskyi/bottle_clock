#include <GyverButton.h>
#include "RTClib.h"
#include "timer.h"
#include <FastLED.h>

#define DATA_PIN 4
#define BTN_PIN 3
#define NUM_LEDS 64

#define M1_START 0
#define M2_START 15
#define H1_START 34
#define H2_START 49

RTC_DS1307 rtc;
GButton btn(BTN_PIN, LOW_PULL, NORM_OPEN);
timerMinim dotTimer(500);
timerMinim blinkTimer(1000);
bool hoursSetup = false;
bool minutesSetup = false;
bool blink = true;
int changeHrs, changeMins;

CRGB colors[7] = {CRGB(122, 4, 235), CRGB( 235, 52, 134), CRGB(41, 191, 196), CRGB(81, 66, 245), CRGB(245, 182, 66), CRGB(54, 209, 65), CRGB(0, 79, 189)};
int currentColor = 0;
CRGB leds[NUM_LEDS];
int nums[10][13] = {
  {0, 1, 2, 3, 4, 5, 9, 10, 11, 12, 13, 14, -1}, // 0
  {14, 13, 12, 11, 10, -1, -1, -1, -1, -1, -1, -1, -1}, // 1
  {4, 9, 14, 13, 12, 7, 2, 1, 0, 5, 10, -1, -1}, // 2
  {4, 9, 14, 13, 12, 7, 2, 11, 10, 5, 0, -1, -1}, // 3
  {4, 3, 2, 7, 12, 13, 14, 11, 10, -1, -1, -1, -1}, // 4
  {14, 9, 4, 3, 2, 7, 12, 11, 10, 5, 0, -1, -1}, // 5
  {14, 9, 4, 3, 2, 7, 12, 11, 10, 5, 0, 1, -1}, // 6
  {4, 9, 14, 13, 12, 11, 10, -1, -1, -1, -1, -1, -1}, // 7
  {0, 1, 2, 3, 4, 5, 7, 9, 10, 11, 12, 13, 14}, // 8
  {0, -1, 2, 3, 4, 5, 7, 9, 10, 11, 12, 13, 14}, // 9
};

void setup() {
  // put your setup code here, to run once:
  Serial.begin(9600);
  if (! rtc.begin()) {
   Serial.println("Couldn't find RTC");
  }

  if (! rtc.isrunning()) {
   Serial.println("RTC is NOT running!");
   rtc.adjust(DateTime(F(__DATE__), F(__TIME__)));
  }

  FastLED.setBrightness(30);
  FastLED.addLeds<NEOPIXEL, DATA_PIN>(leds, NUM_LEDS);
}

void setTime(int h2, int m2) {
  int m1 = 0;
  int h1 = 0;

  if (h2 > 9) {
    h1 = h2 / 10;
    h2 = h2 % 10;
  }

  if (m2 > 9) {
    m1 = m2 / 10;
    m2 = m2 % 10;
  }

  Serial.print(h1);
  Serial.print(h2);
  Serial.print(":");
  Serial.print(m1);
  Serial.println(m2);

  setBlack(M1_START, M1_START + 15);
  setBlack(M2_START, M2_START + 15);
  setBlack(H1_START, H1_START + 15);
  setBlack(H2_START, H2_START + 15);

  setDigit(m1, M1_START);
  setDigit(m2, M2_START);
  setDigit(h1, H1_START);
  setDigit(h2, H2_START);
  FastLED.show();
}

void setDigit(int digit, int digit_offset) {
  for(int j=0;j<13;j++) {
    int led_n = nums[digit][j];
    if (led_n > -1) {
      leds[led_n + digit_offset] = colors[currentColor];
    }
  }
}

void setBlack(int begin, int end) {
  for(int j=begin;j<end;j++) {
    leds[j] = CRGB::Black;
  }
}

void ledBlink() {
  if (blink) {
    leds[30] = CRGB::Black;
    leds[31] = CRGB::Black;
    leds[32] = colors[currentColor];
    leds[33] = colors[currentColor];
    FastLED.show();
    blink = false;
  } else {
    leds[30] = colors[currentColor];
    leds[31] = colors[currentColor];
    leds[32] = CRGB::Black;
    leds[33] = CRGB::Black;
    FastLED.show();
    blink = true;
  }
}

void loop() {
  // put your main code here, to run repeatedly:
  btn.tick();

  if (btn.isClick()) {
    Serial.println("click");

    if (hoursSetup) {
      changeHrs += 1;
      if (changeHrs > 23) {
        changeHrs = 0;
      }
      Serial.print(changeHrs);
      Serial.print(" ");
      Serial.println(changeMins);
      setTime(changeHrs, changeMins);
    } else if (minutesSetup) {
      changeMins += 1;
      if (changeMins > 59) {
        changeMins = 0;
      }

      Serial.print(changeHrs);
      Serial.print(" ");
      Serial.println(changeMins);
      setTime(changeHrs, changeMins);
    } else {
      currentColor += 1;
      if (currentColor > 6) {
        currentColor = 0;
      }
      dotTimer.reset();
    }
  }

  if (btn.isHolded()) {
    Serial.println("holded");
    
    if (!hoursSetup && !minutesSetup) {
      hoursSetup = true;
      minutesSetup = false;
      DateTime now = rtc.now();
      changeHrs = now.hour();
      changeMins = now.minute();
      leds[30] = CRGB::Black;
      leds[31] = CRGB::Black;
      leds[32] = CRGB::Red;
      leds[33] = CRGB::Red;
      FastLED.show();
      
      Serial.println("start to change hours");
      Serial.print(changeHrs);
      Serial.print(" ");
      Serial.print(changeMins);
    } else if(hoursSetup && !minutesSetup) {
      hoursSetup = false;
      minutesSetup = true;
      Serial.println("start to change minutes");
      Serial.print(changeHrs);
      Serial.print(" ");
      Serial.print(changeMins);

      leds[30] = CRGB::Red;
      leds[31] = CRGB::Red;
      leds[32] = CRGB::Black;
      leds[33] = CRGB::Black;
      FastLED.show();
      
    } else if(!hoursSetup && minutesSetup) {
      hoursSetup = false;
      minutesSetup = false;
      Serial.println("setup done");
      Serial.print(changeHrs);
      Serial.print(" ");
      Serial.println(changeMins);

      rtc.adjust(DateTime(2019, 12, 05, changeHrs, changeMins, 0));
    }
  }

 if (dotTimer.isReady() && !hoursSetup && !minutesSetup) {
    DateTime now = rtc.now();
    setTime(now.hour(), now.minute());
 }

 if (blinkTimer.isReady() && !hoursSetup && !minutesSetup) {
  ledBlink();
 }
}
