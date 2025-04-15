#ifndef UTILS_H
#define UTILS_H

float fastPow2(float x) {
  return x * x;
}
float fastPow5(float x) {
  return x * x * x * x * x;
}
float fastPow1dot5(float x) {
  return x * sqrt(x);
}
float fastExp2(float x) {
  // Pré-calculer la constante ln(2)
  const float log2 = log(2);
  return exp(x * log2);
}
unsigned long inoise16(unsigned long x, unsigned long y, unsigned int z) {
  // Utilisation d'une méthode de bruit 3D avec des transformations et un mélange
  x = (x << 13) ^ x;
  y = (y << 13) ^ y;
  z = (z << 13) ^ z;

  unsigned long long hash = (x + y * 57 + z * 131) & 0xFFFFFFFF;

  hash = (hash * (hash * hash * 15731 + 789221) + 1376312589) & 0x7fffffff;
  return (unsigned short)(hash >> 16);  // Retourner un bruit sur 16 bits
}

struct RGen {  // Implémentation de RGen
  uint32_t s1, s2, s3;
  void init(uint32_t seed) {
    // Initialisation des états internes
    seed = (seed ^ 0x5DEECE66D) & 0xFFFFFFFF;  // Mélange le seed
    s1 = 1243598713U ^ seed;
    if (s1 < 2) s1 = 1243598713U;
    s2 = 3093459404U ^ seed;
    if (s2 < 8) s2 = 3093459404U;
    s3 = 1821928721U ^ seed;
    if (s3 < 16) s3 = 1821928721U;
  }
  // Générateur de nombres aléatoires (Tausworthe)
  uint32_t trand() {
    s1 = ((s1 & 0xFFFFFFFE) << 12) ^ (((s1 << 13) ^ s1) >> 19);
    s2 = ((s2 & 0xFFFFFFF8) << 4) ^ (((s2 << 2) ^ s2) >> 25);
    s3 = ((s3 & 0xFFFFFFF0) << 17) ^ (((s3 << 3) ^ s3) >> 11);
    return s1 ^ s2 ^ s3;
  }
  // Génère un float entre 0.0 et 0.999...
  float frand() {
    uint32_t randomBits = trand() >> 9;  // Réduit la précision
    return (randomBits / 8388608.0f);    // Normalise entre 0.0 et 0.999...
  }
};
RGen rgen;

/************ LED BLINK ************ */

// Variables globales pour gérer le clignotement non bloquant
bool isBlinking = false;       // Indique si le clignotement est en cours
unsigned long blinkStartTime;  // Temps de début du clignotement
int blinkCountRemaining;       // Nombre de clignotements restants
bool ledState = LOW;           // État actuel de l'LED

// Simple function to blink an LED a specified number of times
void blinkLED(int blinkCount) {
  isBlinking = true;                     // Démarre le clignotement
  blinkStartTime = millis();             // Enregistre le temps de début
  blinkCountRemaining = blinkCount * 2;  // Chaque clignotement a deux états (ON et OFF)
  ledState = LOW;                        // Assure que l'LED commence éteinte
  digitalWrite(ledPin, ledState);        // Éteint l'LED
}

void handleBlink() {
  if (isBlinking) {
    unsigned long currentTime = millis();
    if (currentTime - blinkStartTime >= BLINK) {  // Vérifie si le délai est écoulé
      blinkStartTime = currentTime;               // Réinitialise le temps de début
      ledState = !ledState;                       // Change l'état de l'LED
      digitalWrite(ledPin, ledState);             // Met à jour l'état de l'LED
      blinkCountRemaining--;                      // Réduit le nombre de clignotements restants

      if (blinkCountRemaining <= 0) {  // Si tous les clignotements sont terminés
        isBlinking = false;            // Arrête le clignotement
        digitalWrite(ledPin, LOW);     // Assure que l'LED est éteinte
      }
    }
  }
}

/**************** LONG PRESS ****************************/
// pour basculer en mode de choix de l'algo, on reste appuyer HOLDTIME puis on clique le nombre de fois de l'algo, attendre 2*HOLDTIME pour sortir du choix 
void checkLongPress() {
  static unsigned long startTime = 0;
  static unsigned long lastCountTime = 0;
  static byte algoCount = 0;

  if (minPot != 0 || maxPot != 0) return;  // Vérifie uniquement si minPot et maxPot sont à 0
  if (trigStage == NEW) {
    //Serial.println(F("NEW PRESS "));
    startTime = millis();
    if (algoChoose) {
      algoCount++;
    }
  } else if (trigStage == OLD) {
    if (millis() - startTime > HOLDTIME) {
      if (!algoChoose) {
#if DEBUG
        Serial.println(F("ALGOCHOOSE ON"));
#endif
        blinkLED(2);
        algoChoose = true;
        algoCount = 0;
      }
    }
  } else {
    if (prevStage != trigStage) {
      //Serial.println(F("Just released"));
      lastCountTime = millis();
    }
    if (algoChoose && (millis() - lastCountTime > 2 * HOLDTIME)) {
      blinkLED(algo);
      algoChoose = false;
      algo = (algoCount - 1 + MAX_ALGO) % MAX_ALGO;
#if DEBUG
      Serial.print(F("ALGOCHOOSE OFF "));
      Serial.println(algo);
#endif
      if (algo != algoFromEEPROM) {
        EEPROM.write(eepromAddress, algo);
        algoFromEEPROM = algo;
      }
    }
  }
  prevStage = trigStage;
}

/*************** TRIGGER TEST **************$*/
// vrai si trig est actif et (on est en track&hold ou rising)
bool isTriggered() {
  return trig && (shth || trig != prevTrig);
}

#endif