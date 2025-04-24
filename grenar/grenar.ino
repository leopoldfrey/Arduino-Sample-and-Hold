#include "branche.h"

//#define DEBUG

BernoulliGate branche1 = BernoulliGate();
BernoulliGate branche2 = BernoulliGate();

void setup() {
  //            mode, trigPin, switchPin, threshPin, ledPosPin, ledNegPin, outAPin, outBPin)
  branche1.Init(LATCH, 4, A2, A1, 1, 2, 3, 0);
  branche2.Init(LATCH, 7, A3, A0, 9, 8, 6, 5);
  branche1.setup();
  branche2.setup();

#ifdef DEBUG
  Serial.begin(115200);
#endif
}

void loop() {
  unsigned long currentTime = millis();
  branche1.process(currentTime);
  branche2.process(currentTime);
  /*
  // SERIAL DEBUG
  Serial.print("[A] t:");
  Serial.print(branche1.getThreshold());
  Serial.print(" s:");
  Serial.print(branche1.getSwitch());
  Serial.print(" -- [B] t:");
  Serial.print(branche2.getThreshold());
  Serial.print(" s:");
  Serial.print(branche2.getSwitch());
  Serial.println();//*/

  /* 
  //TEST LEDS
  delay(2000);
  digitalWrite(branche1.ledPosPin_, HIGH);
  digitalWrite(branche1.ledNegPin_, LOW);
  digitalWrite(branche2.ledPosPin_, HIGH);
  digitalWrite(branche2.ledNegPin_, LOW);
  delay(1000);
  digitalWrite(branche1.ledPosPin_, LOW);
  digitalWrite(branche1.ledNegPin_, HIGH);
  digitalWrite(branche2.ledPosPin_, LOW);
  digitalWrite(branche2.ledNegPin_, HIGH);
  //*/

  /*
  //TEST OUTA/B
  delay(2000);
  digitalWrite(branche1.outAPin_, HIGH);
  digitalWrite(branche1.outBPin_, LOW);
  digitalWrite(branche2.outAPin_, HIGH);
  digitalWrite(branche2.outBPin_, LOW);
  delay(2000);
  digitalWrite(branche1.outAPin_, LOW);
  digitalWrite(branche1.outBPin_, HIGH);
  digitalWrite(branche2.outAPin_, LOW);
  digitalWrite(branche2.outBPin_, HIGH);
  //*/

  /*
  // TEST SWITCHES
  digitalWrite(branche1.outAPin_, !digitalRead(branche1.switchPin_));
  digitalWrite(branche2.outAPin_, !digitalRead(branche2.switchPin_));
  //*/

  /*
  // TEST CV / THRESOLD !!!!! NEEDS 12V !!!!!
  if (branche1.getThreshold() < 512) {
    digitalWrite(branche1.outBPin_, HIGH);
    digitalWrite(branche1.outAPin_, LOW);

    digitalWrite(branche1.ledPosPin_, HIGH);
    digitalWrite(branche1.ledNegPin_, LOW);
  } else {
    digitalWrite(branche1.outBPin_, LOW);
    digitalWrite(branche1.outAPin_, HIGH);

    digitalWrite(branche1.ledPosPin_, LOW);
    digitalWrite(branche1.ledNegPin_, HIGH);
  }

  if (branche2.getThreshold() < 512) {
    digitalWrite(branche2.outBPin_, HIGH);
    digitalWrite(branche2.outAPin_, LOW);

    digitalWrite(branche2.ledPosPin_, HIGH);
    digitalWrite(branche2.ledNegPin_, LOW);
  } else {
    digitalWrite(branche2.outBPin_, LOW);
    digitalWrite(branche2.outAPin_, HIGH);

    digitalWrite(branche2.ledPosPin_, LOW);
    digitalWrite(branche2.ledNegPin_, HIGH);
  }//*/
}
