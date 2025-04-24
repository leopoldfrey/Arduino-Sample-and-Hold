#include "Arduino.h"
#ifndef BRANCHE_H
#define BRANCHE_H

#define HOLD_TIME 1000

enum Mode {
  LATCH,
  TOGGLE
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
    int ledPosPin,
    int ledNegPin,
    int outAPin,
    int outBPin) {
    mode_ = mode;
    trigPin_ = trigPin;
    switchPin_ = switchPin;
    threshPin_ = threshPin;
    ledPosPin_ = ledPosPin;
    ledNegPin_ = ledNegPin;
    outAPin_ = outAPin;
    outBPin_ = outBPin;
    state = false;
  }

  void setup() {
    pinMode(trigPin_, INPUT_PULLUP);
    pinMode(switchPin_, INPUT_PULLUP);
    pinMode(threshPin_, INPUT);
    // OUTPUTS
    pinMode(ledPosPin_, OUTPUT);
    pinMode(ledNegPin_, OUTPUT);
    pinMode(outAPin_, OUTPUT);
    pinMode(outBPin_, OUTPUT);
  }

  bool coinToss() {
    return random(1024) < threshold;
  }

  void treatSwitch(unsigned long currentTime) {
    static bool done = false;
    bool switchValue = digitalRead(switchPin_);
    if (switchValue != prevSwitch) {  // CHANGE
      if (switchValue == LOW)         // RISING EDGE
      {
        switchStart = currentTime;
        done = false;
      }
    } else {
      if (currentTime - switchStart > HOLD_TIME && !done) {
        done = true;
        switch (mode_) {
          case LATCH:
            mode_ = TOGGLE;
            digitalWrite(ledPosPin_, LOW);
            digitalWrite(ledNegPin_, LOW);
            break;
          case TOGGLE:
            mode_ = LATCH;
            digitalWrite(ledPosPin_, LOW);
            digitalWrite(ledNegPin_, LOW);
            break;
        }
      }
    }
    prevSwitch = switchValue;
  }


  void process(unsigned long currentTime) {
    treatSwitch(currentTime);
    threshold = 1023 - analogRead(threshPin_);
    bool trig = digitalRead(trigPin_);
    if (prevTrig != trig)  // WE'VE GOT A CHANGE
    {
      if (trig == LOW) {  // RISING EDGE
        switch (mode_) {
          case LATCH:
            if (coinToss()) {
              digitalWrite(ledPosPin_, HIGH);
              digitalWrite(ledNegPin_, LOW);

              digitalWrite(outAPin_, LOW);
              digitalWrite(outBPin_, HIGH);
            } else {
              digitalWrite(ledPosPin_, LOW);
              digitalWrite(ledNegPin_, HIGH);

              digitalWrite(outAPin_, HIGH);
              digitalWrite(outBPin_, LOW);
            }
            break;
          case TOGGLE:
            if (coinToss()) {  // CHANGE STATE
              state = !state;
              if (state) {
                digitalWrite(ledPosPin_, HIGH);
                digitalWrite(ledNegPin_, LOW);

                digitalWrite(outAPin_, LOW);
                digitalWrite(outBPin_, HIGH);
              } else {
                digitalWrite(ledPosPin_, LOW);
                digitalWrite(ledNegPin_, HIGH);

                digitalWrite(outAPin_, HIGH);
                digitalWrite(outBPin_, LOW);
              }
            } else {  // DON'T
            }


            //TODO
            break;
        }
      } else {  // FALLING EDGE
        switch (mode_) {
          case LATCH:
            digitalWrite(outAPin_, LOW);
            digitalWrite(outBPin_, LOW);
            digitalWrite(ledPosPin_, LOW);
            digitalWrite(ledNegPin_, LOW);
            break;
          case TOGGLE:
            // DO NOTHING
            break;
        }
      }
    }
    prevTrig = trig;
  }

  int getSwitch() {
    return digitalRead(switchPin_);
  }

  int getThreshold() {
    return analogRead(threshPin_);
  }

  //private:
  Mode mode_ = LATCH;
  int trigPin_;
  int outAPin_;
  int outBPin_;
  uint8_t switchPin_;
  uint8_t threshPin_;  // pot + CV
  int ledPosPin_;
  int ledNegPin_;
  int threshold = 0;
  bool prevTrig = false;
  bool prevSwitch = false;
  unsigned long switchStart = 0;
  bool state = false;
};
#endif
