#include "Arduino.h"
#ifndef BRANCHE_H
#define BRANCHE_H

//#define DEBUG

#define HOLD_TIME 2000

enum Mode {
  LATCH,
  TOGGLE
};

enum Led {
  LED_GREEN,
  LED_RED,
  LED_OFF
};

class BernoulliGate {
public:

  BernoulliGate() {}
  ~BernoulliGate() {}

  void Init(
    Mode mode,
    int trigPin,
    uint8_t switchPin,
    uint8_t threshPin,
    int ledRedPin,
    int ledGreenPin,
    int outAPin,
    int outBPin) {
    mode_ = mode;
    trigPin_ = trigPin;
    switchPin_ = switchPin;
    threshPin_ = threshPin;
    ledRedPin_ = ledRedPin;
    ledGreenPin_ = ledGreenPin;
    outAPin_ = outAPin;
    outBPin_ = outBPin;
    state = false;
  }

  void setup() {
    pinMode(trigPin_, INPUT_PULLUP);
    pinMode(switchPin_, INPUT_PULLUP);
    pinMode(threshPin_, INPUT);
    // OUTPUTS
    pinMode(ledRedPin_, OUTPUT);
    pinMode(ledGreenPin_, OUTPUT);
    pinMode(outAPin_, OUTPUT);
    pinMode(outBPin_, OUTPUT);
  }

  bool coinToss() {
    return random(1024) < threshold;
  }

  void blink(int n, int dur, Led color) {
    while (n > 0) {
      delay(dur);
      updateLed(color);
      delay(dur);
      updateLed(LED_OFF);
      n--;
    }
  }

  void updateLed(Led l) {
    led_ = l;
    switch (led_) {
      case LED_GREEN:
        digitalWrite(ledRedPin_, LOW);
        digitalWrite(ledGreenPin_, HIGH);
        break;
      case LED_RED:
        digitalWrite(ledRedPin_, HIGH);
        digitalWrite(ledGreenPin_, LOW);
        break;
      case LED_OFF:
        digitalWrite(ledRedPin_, LOW);
        digitalWrite(ledGreenPin_, LOW);
        break;
    }
  }

  void treatSwitch(unsigned long currentTime) {
    static bool done = true;
    bool switchValue = digitalRead(switchPin_);
    if (switchValue != prevSwitch) {  // CHANGE
      if (switchValue == LOW)         // RISING EDGE
      {
#ifdef DEBUG
        Serial.println("Switch Rising Edge!");
#endif
        switchStart = currentTime;
        done = false;
        switchDown = true;
      } else {
#ifdef DEBUG
        Serial.println("Switch Falling Edge!");
#endif
        switchDown = false;
        switchStart = currentTime;
      }
    } else {
      if (currentTime - switchStart > HOLD_TIME && !done && switchDown) {
#ifdef DEBUG
        Serial.print("CHANGE MODE! ");
        Serial.println(currentTime - switchStart);
#endif
        done = true;
        switch (mode_) {
          case LATCH:
            mode_ = TOGGLE;
            blink(5, 100, LED_RED);
            break;
          case TOGGLE:
            mode_ = LATCH;
            blink(5, 100, LED_GREEN);
            break;
        }
      }
    }
    prevSwitch = switchValue;
  }

  Mode process(unsigned long currentTime) {
    treatSwitch(currentTime);
    threshold = 1023 - analogRead(threshPin_);
    bool trig = digitalRead(trigPin_);
    if (prevTrig != trig)  // WE'VE GOT A CHANGE
    {
      if (trig == LOW) {  // RISING EDGE
        switch (mode_) {
          case LATCH:
            if (coinToss()) {
              digitalWrite(outAPin_, LOW);
              digitalWrite(outBPin_, HIGH);
              updateLed(LED_RED);
            } else {
              digitalWrite(outAPin_, HIGH);
              digitalWrite(outBPin_, LOW);
              updateLed(LED_GREEN);
            }
            break;
          case TOGGLE:
            if (coinToss()) {  // CHANGE STATE
              state = !state;
              if (state) {
                digitalWrite(outAPin_, LOW);
                digitalWrite(outBPin_, HIGH);
                updateLed(LED_RED);
              } else {
                digitalWrite(outAPin_, HIGH);
                digitalWrite(outBPin_, LOW);
                updateLed(LED_GREEN);
              }
            } else {  // DON'T
            }
            break;
        }
      } else {  // FALLING EDGE
        switch (mode_) {
          case LATCH:
            digitalWrite(outAPin_, LOW);
            digitalWrite(outBPin_, LOW);
            updateLed(LED_OFF);
            break;
          case TOGGLE:
            // DO NOTHING
            break;
        }
      }
    }
    prevTrig = trig;

    return mode_;
  }

  int getSwitch() {
    return digitalRead(switchPin_);
  }

  int getThreshold() {
    return analogRead(threshPin_);
  }

  //private:
  Mode mode_ = LATCH;
  Led led_ = LED_OFF;
  int trigPin_;
  int outAPin_;
  int outBPin_;
  uint8_t switchPin_;
  uint8_t threshPin_;  // pot + CV
  int ledRedPin_;
  int ledGreenPin_;
  int threshold = 0;
  bool prevTrig = false;
  bool prevSwitch = false;
  unsigned long switchStart = 0;
  bool state = false;
  bool switchDown = false;
};
#endif
