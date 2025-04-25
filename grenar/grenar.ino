#include <EEPROM.h>
#include "branche.h"

//#define DEBUG

BernoulliGate branche1 = BernoulliGate();
BernoulliGate branche2 = BernoulliGate();

Mode prevMode1 = LATCH;
Mode prevMode2 = LATCH;
Mode mode1 = LATCH;
Mode mode2 = LATCH;

// Script state & storage variables.
// Expected version string for this firmware.
const char SCRIPT_NAME[] = "Grenar";
const byte SCRIPT_VER = 2;

struct State {
  // Version check.
  char script[sizeof(SCRIPT_NAME)];
  byte version;
  // State variables.
  byte mode1;
  byte mode2;
};
State state;

// Initialize script state from EEPROM or default values.
void InitState() {
  // Read previously put state from EEPROM memory. If it doesn't exist or
  // match version, then populate the State struct with default values.
  EEPROM.get(0, state);
  // Check if the data in memory matches expected values.
  if ((strcmp(state.script, SCRIPT_NAME) != 0) || (state.version != SCRIPT_VER)) {
#ifdef DEBUG
    Serial.println("INIT STATE");
#endif
    // DEFAULT INIT
    strcpy(state.script, SCRIPT_NAME);
    state.version = SCRIPT_VER;

    state.mode1 = 0;  // LATCH
    state.mode2 = 0;  // LATCH
  } else {
#ifdef DEBUG
    Serial.print("EEPROM State ");
    Serial.print(state.script);
    Serial.print(" ");
    Serial.println(state.version);
    Serial.print("Read Mode");
    Serial.print(" 1:");
    Serial.print(state.mode1);
    Serial.print(" 2:");
    Serial.println(state.mode2);
#endif
  }

  mode1 = state.mode1 == 0 ? LATCH : TOGGLE;
  mode2 = state.mode2 == 0 ? LATCH : TOGGLE;
  prevMode1 = mode1;
  prevMode2 = mode2;
}

void SaveState() {
  state.mode1 = mode1 == LATCH ? 0 : 1;
  state.mode2 = mode2 == LATCH ? 0 : 1;

#ifdef DEBUG
    Serial.print("Write Mode 1:");
    Serial.print(state.mode1);
    Serial.print(" 2:");
    Serial.println(state.mode2);
#endif
  EEPROM.put(0, state);
}

void setup() {

#ifdef DEBUG
  Serial.begin(115200);
  Serial.println();
  Serial.println();
  Serial.println();
#endif
  // READ EEPROM STATE
  InitState();
  //            mode, trigPin, switchPin, threshPin, ledRedPin, ledGreenPin, outAPin, outBPin)
  branche1.Init(mode1, 4, A3, A1, 1, 2, 3, 0);
  branche2.Init(mode2, 7, A2, A0, 9, 8, 6, 5);
  branche1.setup();
  branche2.setup();
}

void loop() {
  unsigned long currentTime = millis();
  mode1 = branche1.process(currentTime);
  mode2 = branche2.process(currentTime);
  if (prevMode1 != mode1 || prevMode2 != mode2)
    SaveState();

  prevMode1 = mode1;
  prevMode2 = mode2;

  /*
  // SERIAL DEBUG
  // Adjust trimmer to get 1023 at min and 0 at max with no CV
  // Adjust trimmer to get ~0 with 5V in (and pot at min)
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
