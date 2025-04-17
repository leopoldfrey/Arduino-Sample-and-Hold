#include <EEPROM.h>
#include <FastLED.h>

#define DEBUG 1

const byte MAX_ALGO = 15;
const unsigned long HOLDTIME = 2000;
const unsigned long BLINK = 150;
const unsigned long MINGATE = 20;

/********** PIN NUMBERS **********/
const int minPin = A1;
const int maxPin = A2;
const int trigIn = A3;       // Pin connected to the button and the trig in jack
const int ledPin = A5;       // Pin connected to the LED
const int signalInput = A6;  // Input signal to sample or track.
const int SHTHpin = A0;      //switch between 5v and GND to decide sample or track.
const int extOrIntPin = A4;

/********* GLOBAL VARIABLES ************/
#if DEBUG
char buffer[100];
#endif
int sample;
int prevSample;
bool shth = false;
bool trig = false;
bool prevTrig = false;
bool extOrInt = false;  // true = lecture input externe, false = source interne
int maxPot = 1023;      // filter
int minPot = 0;
int inp = 0;

/******* ALGO *******/
const byte eepromAddress = 0;  //the place in the eeprom to save the selectedFunction, if changing too often (100000+ times) then change this adress because that address might have been worned out ;)
byte algoFromEEPROM;           //to make sure we dont write to eeprom too often.
byte algo = 0;
bool algoChoose = false;

/******** TRGI STAGE *************/
enum TrigStage {
  NEW,
  OLD,
  OFF,
};
TrigStage trigStage = OFF;
TrigStage prevStage = OFF;

#define ALGO_SHTH 0
#define ALGO_PERLIN 1  // Based on PerlinNoise.ino @author Adam Wonak (https://github.com/awonak/HagiwoModulove/blob/main/SyncLFO/PerlinNoise/PerlinNoise.ino)
#define ALGO_BROWNIAN 2
#define ALGO_LORENZ 3
#define ALGO_DUST 4  // Based on SuperCollider for max sc.dust2~ (https://github.com/sbl/sc-max/blob/master/source/projects/sc.dust2_tilde/sc.dust2_tilde.cpp)
#define ALGO_FLIPNOISE 5
#define ALGO_RANDGATE 6
#define ALGO_GATEDELAY 7
#define ALGO_COMPARE 8
#define ALGO_RYTHMQUANT 9
#define ALGO_CVRECORDER 10
#define ALGO_TURING 11
#define ALGO_BURST 12
#define ALGO_CRACKLE 13  // Based on SuperCollider for max sc.crackle~ (https://github.com/sbl/sc-max/blob/master/source/projects/sc.crackle_tilde/sc.crackle_tilde.cpp)
#define ALGO_CALIB 14

#include "utils.h"
#include "algos.h"

/********* SETUP ***********/

void setup() {
#if DEBUG
  Serial.begin(115200);
#endif

  DDRD = 0b11111111;
  DDRB = 0b11111111;
  pinMode(SHTHpin, INPUT_PULLUP);
  pinMode(extOrIntPin, INPUT_PULLUP);
  pinMode(trigIn, INPUT_PULLUP);
  pinMode(ledPin, OUTPUT);

  algo = EEPROM.read(eepromAddress) % MAX_ALGO;
  algoFromEEPROM = algo;

#if DEBUG
  Serial.println(algo);
#endif

  // init turing buffer
  for (int i = 0; i < turingBufferSize; i++) {
    turingBuffer[i] = random(0, 1024);
  }
  // init looper buffer
  for (int i = 0; i < loopBufferSize; i++) {
    loopBuffer[i] = 0;
  }
  // init randomseeds
  randomSeed(analogRead(A6));
  rgen.init(analogRead(A6));
  // init tap tempo
  tapTempo.setBPM(90);
  tapTempo.setMinBPM(30);
  tapTempo.setMaxBPM(300);
  //lastCountTime = millis();
}

/********** LOOP *********/

void loop() {
  // read inputs
  minPot = analogRead(minPin);
  maxPot = analogRead(maxPin);
  extOrInt = digitalRead(extOrIntPin) == HIGH;
  inp = analogRead(signalInput);
  shth = digitalRead(SHTHpin) == LOW;
  trig = digitalRead(trigIn) == LOW;
  if (isTriggered()) {
    switch (trigStage) {
      case OFF:
        trigStage = NEW;
        break;
      case NEW:
        if (!isBlinking)
          digitalWrite(ledPin, HIGH);
        trigStage = OLD;
      case OLD:
        if (!isBlinking)
          digitalWrite(ledPin, LOW);
        break;
    }
  } else {
    trigStage = OFF;
  }

  if (!algoChoose) {
    switch (algo) {
      case ALGO_SHTH:
        handleSampleAndHold();
        break;
      case ALGO_PERLIN:
        handlePerlinNoise();
        break;
      case ALGO_BROWNIAN:
        handleBrownianMotion();
        break;
      case ALGO_LORENZ:
        handleLorenzAttractor();
        break;
      case ALGO_DUST:
        handleDust();
        break;
      case ALGO_FLIPNOISE:
        handleFlipNoise();
        break;
      case ALGO_RANDGATE:
        handleRandGate();
        break;
      case ALGO_GATEDELAY:
        handleGateDelay();
        break;
      case ALGO_COMPARE:
        handleCompare();
        break;
      case ALGO_RYTHMQUANT:
        handleQuantizer();
        break;
      case ALGO_CVRECORDER:
        handleLoopRecorder();
        break;
      case ALGO_TURING:
        handleTuringMachine();
        break;
      case ALGO_BURST:
        handleBurst();
        break;
      case ALGO_CRACKLE:
        handleCrackle();
        break;
      case ALGO_CALIB:
        handleCalib();
        break;
    }

    if (isTriggered() || algo == ALGO_CVRECORDER || algo == ALGO_BURST || algo == ALGO_GATEDELAY) {
      PORTD = lowByte(sample);
      PORTB = highByte(sample);
      digitalWrite(ledPin, sample > 512);
#if DEBUG
      snprintf(buffer, sizeof(buffer), "%d %d", algo, sample);
      Serial.println(buffer);
#endif
    } else if (algo == ALGO_RANDGATE || algo == ALGO_RYTHMQUANT) {
      PORTD = 0;
      PORTB = 0;
      digitalWrite(ledPin, LOW);
    }
    prevSample = sample;
    prevTrig = trig;
  }

  checkLongPress();
  handleBlink();
}