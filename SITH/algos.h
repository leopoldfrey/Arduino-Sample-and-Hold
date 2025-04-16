#ifndef ALGOS_H
#define ALGOS_H

/********* ALGO SHTH ********/
// Sample and Hold / Track and Hold
void handleSampleAndHold() {
  if (isTriggered()) {
    if (extOrInt) {
      sample = map(inp, 0, 1023, minPot, maxPot);
    } else {
      sample = random(minPot, maxPot);
    }
  }
}

/********* ALGO PERLIN NOISE ********/
// Génère des valeurs basées sur le bruit de Perlin
// Exponential curve factors for frequency and noise buffer.
void handlePerlinNoise() {
  static long perlin_nx = 0;                  // Perlin noise buffer x read value
  static long perlin_ny = 0;                  // Perlin noise buffer y read value
  static int perlin_seed = random(0, 65536);  // Start with a new random Perlin noise Z-axis each time.
  static int perlin_offset = 0;
  static int perlin_val = 0;                           // current value from the perlin noise algorithm
  static unsigned long perlin_frequencyInterval = 10;  // << 4;    // Frequency timer (increase for slower rate)
  static unsigned long perlin_currentTime = 0;
  static unsigned long perlin_previousTime = 0;  // Store last time Frequency was updated
  static float perlin_freq = 0;
  perlin_offset = map(minPot, 0, 1024, -511, 512);  // minPot = offset
  perlin_currentTime = micros();

  if (perlin_currentTime - perlin_previousTime > perlin_frequencyInterval) {
    // Calcul de la fréquence
    if (isTriggered()) {
      if (extOrInt) {
        perlin_freq = sqrt(maxPot / 1024.f * (float)(inp / 1024.f)) * 450 + 100;
      } else {
        perlin_freq = sqrt(maxPot / 1024.f) * 450 + 100;
      }
    }

    // Mise à jour des coordonnées Perlin
    perlin_nx += fastExp2(perlin_freq / 32);
    perlin_val = inoise16(perlin_nx, ++perlin_ny, perlin_seed) >> 6;  // 16bit noise > 10bit value

    // Application de l'offset
    if (perlin_offset > 0) {
      perlin_val = constrain(perlin_val + perlin_offset, 0, 1024);
    } else {
      perlin_val = (perlin_val > abs(perlin_offset)) ? perlin_val + perlin_offset : 0;
    }

    // Mise à jour de l'échantillon
    sample = perlin_val;

    // Mise à jour du temps précédent
    perlin_previousTime = perlin_currentTime;
  }
}

/*********** ALGO BROWNIAN ********/
// Génère un mouvement brownien (marche aléatoire)
void handleBrownianMotion() {
  static float brownian_value = 512.0;  // Point de départ
  static float brownian_step = 10.0;    // Taille maximale du pas
  static float brownian_delta = 0.0;    // Variation aléatoire
  static float brownian_offset = 0.0;   // Décalage
  if (isTriggered()) {
    brownian_step = extOrInt
                      ? fastPow2(maxPot / 1024.f * (float)(inp / 1024.f)) * 50.f + 0.1f
                      : fastPow2(maxPot / 1024.f) * 50.f + 0.1f;
  }
  brownian_delta = ((int)(rgen.trand() % 201) - 100) / 100.0f * brownian_step;
  brownian_offset = map(minPot, 0, 1023, -512, 512);
  if (brownian_value + brownian_offset + brownian_delta < 0 || brownian_value + brownian_offset + brownian_delta > 1023) {
    brownian_delta = -brownian_delta;  // Inverser la direction du delta
  }
  brownian_value += brownian_delta;
  sample = constrain((int)brownian_value + brownian_offset, 0, 1023);
}

void handleBrownianMotionINT() {
  static int32_t brownian_value = 512000;  // Valeur *1000
  static int32_t brownian_step = 10000;
  static int32_t brownian_delta = 0;
  static int brownian_offset = 0;

  if (isTriggered()) {
    int32_t norm_inp = extOrInt 
                        ? ((int32_t)maxPot * (int32_t)inp * 1000) / (1024 * 1024L)
                        : ((int32_t)maxPot * 1000) / 1024;

    int32_t power = (norm_inp * norm_inp) / 1000;  // car *1000²
    brownian_step = power * 50 + 100;              // *1000, donc +0.1f = +100
  }

  brownian_delta = ((rgen.trand() % 201) - 100) * brownian_step / 100;

  brownian_offset = map(minPot, 0, 1023, -512, 512);

  int32_t tentative = brownian_value + brownian_delta + brownian_offset * 1000;

  if (tentative < 0 || tentative > 1023000) {
    brownian_delta = -brownian_delta;
  }

  brownian_value += brownian_delta;

  sample = constrain((brownian_value + brownian_offset * 1000) / 1000, 0, 1023);
}


/********** ALGO LORENZ ********/
// Génère des valeurs basées sur l'attracteur de Lorenz
void handleLorenzAttractor() {
  static float lorenz_x = 1.f;             // Valeur initiale pour x
  static float lorenz_y = 0.0328f;         // Valeur initiale pour y
  static float lorenz_z = 0.0078125f;      // Valeur initiale pour z
  static float lorenz_sigma = 10.0f;       // Paramètre sigma
  static float lorenz_rho = 28.0f;         // Paramètre rho
  static float lorenz_beta = 8.0f / 3.0f;  // Paramètre beta
  static float lorenz_dt = 0.01f;          // Intervalle de temps pour l'intégration
  static float lorenz_amplitude = 0.0f;    // Amplitude de sortie
  static float lorenz_dx = 0.f;
  static float lorenz_dy = 0.f;
  static float lorenz_dz = 0.f;  // Variables pour les dérivées

  if (isTriggered()) {
    lorenz_dt = extOrInt
                  ? fastPow1dot5((float)(maxPot / 1024.f) * (float)(inp / 1024.f)) * 0.02 + 0.00001
                  : fastPow1dot5(maxPot / 1024.f) * 0.02 + 0.00001;
  }
  lorenz_amplitude = (float)minPot;
  lorenz_dx = lorenz_sigma * (lorenz_y - lorenz_x) * lorenz_dt;
  lorenz_dy = (lorenz_x * (lorenz_rho - lorenz_z) - lorenz_y) * lorenz_dt;
  lorenz_dz = (lorenz_x * lorenz_y - lorenz_beta * lorenz_z) * lorenz_dt;
  lorenz_x += lorenz_dx;
  lorenz_y += lorenz_dy;
  lorenz_z += lorenz_dz;
  sample = constrain((int)(lorenz_z / 40.f * lorenz_amplitude), 0, 1023);
}

/********** ALGO DUST ********/
// Génère des impulsions aléatoires basées sur la densité
float generate_dust(float density) {
  // Calcul des paramètres
  float thresh = density + 0.0001f;  // Seuil basé sur la densité
  float scale = 1.f / thresh;       // Échelle

  // Générer un nombre aléatoire
  float z = rgen.frand();

  // Appliquer la logique de seuil et de mise à l'échelle
  if (z < thresh) {
    return z * scale;  // - 1.f;
  } else {
    return 0.f;
  }
}
void handleDust() {
  static float dust_density = 0.5;
  static int dust_scale = 1024;

  if (isTriggered()) {
    dust_density = extOrInt
                     ? fastPow5(maxPot / 1024.0f * (float)(inp / 1024.f))
                     : fastPow5(maxPot / 1024.0f);
  }
  dust_scale = minPot;  // minPot = offset
  sample = constrain((int)(generate_dust(dust_density) * dust_scale), 0, 1023);
}

int32_t generate_dustINT(int32_t density) {
  // Calcul des paramètres (conversion en entiers)
  int32_t thresh = density + 1;  // Seuil basé sur la densité (ajusté pour éviter la valeur 0)
  int32_t scale = 1024 / thresh;  // Échelle (utilisation d'entiers)

  // Générer un nombre aléatoire avec rgen.trand() et le limiter à la plage [0, 1023]
  int32_t z = rgen.trand() % 1024;  // Limiter la valeur à la plage [0, 1023]

  // Appliquer la logique de seuil et de mise à l'échelle
  if (z < thresh) {
    return (z * scale) / 1024;  // Retourner la valeur mise à l'échelle
  } else {
    return 0;  // Si pas dans le seuil, retourner 0
  }
}
void handleDustINT() {
  static int32_t dust_density = 512;  // Densité sous forme d'entier (valeur entre 0 et 1023)
  static int32_t dust_scale = 1024;   // Échelle de la poussière

  if (isTriggered()) {
    dust_density = extOrInt
                     ? (int32_t)(fastPow5((float)maxPot * (float)inp / (1024L * 1024)) * 1024)
                     : (int32_t)(fastPow5((float)maxPot / 1024) * 1024);
  }

  dust_scale = minPot;  // minPot = offset (utilisation de minPot comme échelle)

  // Utilisation de generate_dust et application de l'échelle
  int32_t dust_value = generate_dustINT(dust_density);
  sample = constrain(dust_value * dust_scale, 0, 1023);  // Calcul de l'échantillon
}


/*********** ALGO FLIP_NOISE *******/
// Génère un bruit blanc avec un seuil de basculement
void handleFlipNoise() {
  static float flip_thres = 0.1;  // Seuil
  static float flip_amp = 0.0;    // Amp
  static float flip_input = 0.0;  // Input
  static bool flip_prev = LOW;    // Pref

  if (isTriggered()) {
    flip_amp = extOrInt ? (float)minPot * (float)inp / 1024. : minPot;
  }
  flip_input = (float)(random(0, 32768) / 32768.0);
  flip_thres = fastPow5(maxPot / 1024.) + 0.0002;
  if (flip_input < flip_thres) {
    flip_prev = !flip_prev;
    sample = flip_prev ? flip_amp : 0;
  }
}

void handleFlipNoiseINT() {
  static int32_t flip_thres = 0;  // Seuil
  static int32_t flip_amp = 0;    // Amplitude
  static int32_t flip_input = 0;  // Entrée
  static bool flip_prev = LOW;    // Précedent

  if (isTriggered()) {
    flip_amp = extOrInt ? (int32_t)(minPot * inp) / 1024 : minPot;
  }

  flip_input = random(0, 1024);  // Valeur aléatoire entre 0 et 1023 (plage correcte)
  // Calcul du seuil, on évite l'utilisation de float
  flip_thres = (int32_t)(fastPow5(maxPot / 1024) * 1024) + 1;  // Seuil calculé en entier
  
  if (flip_input < flip_thres) {
    flip_prev = !flip_prev;
    sample = flip_prev ? flip_amp : 0;  // Si seuil atteint, changer l'état et appliquer l'amplitude
  }
}


/*********** ALG06 RAND_GATE *******/
void handleRandGate() {
  if (trigStage == NEW) {
    float gate_thres = extOrInt ? (float)maxPot / 1024. * (float)inp / 1024. : maxPot / 1024.;
    sample = (float)(random(0, 1024) / 1024.0) <= gate_thres ? random(minPot, 1024) : 0;
  } else if (trigStage == OFF) {
    sample = 0;
  }
}

/*********** ALG07 GATE_DELAY *******/
enum Stage {
  DELAY,  // DELAY AFTER INITIAL TRIG
  GATE,   // GATE HIGH AFTER DELAY
  WAIT,   // WAITING FOR TRIG
};
void handleGateDelay() {
  static Stage gateStage = WAIT;  // Stage
  static unsigned long gateDelayStart;     // Temps de début du délai
  static unsigned long gateDelay = 1000;   // Temps de délai
  static unsigned long gateLength = 1000;  // Temps du gate
  static unsigned long gateCurrentTime;

  gateCurrentTime = millis();
  switch (gateStage) {
    case WAIT:
      if (trigStage == NEW) {
        gateDelay = extOrInt ? (unsigned long)((fastPow2((float)maxPot / 1024.) + fastPow2((float)inp / 1024.)) * 1000.) : (unsigned long)(fastPow2((float)maxPot / 1024.) * 1000.);
        gateLength = (unsigned long)(pow((float)minPot / 1024., 1.25) * 2000.) + MINGATE;
        gateDelayStart = gateCurrentTime;
        gateStage = DELAY;
      }
      sample = 0;
      break;
    case DELAY:
      if (gateCurrentTime - gateDelayStart >= gateDelay) {  // END OF DELAY
        gateStage = GATE;
        sample = 1023;
      } else {
        sample = 0;
      }
      break;
    case GATE:
      if (gateCurrentTime - gateDelayStart >= gateDelay + gateLength) {  // END OF GATE
        gateStage = WAIT;
        sample = 0;
      } else {
        sample = 1023;
      }
      break;
  }
}

/*********** ALGO COMPARE *******/
void handleCompare() {
  if (isTriggered()) {
    if (inp > minPot && inp < maxPot) {
      sample = extOrInt ? 0 : 1023;
    } else {
      sample = extOrInt ? 1023 : 0;
    }
  } else {
    sample = 0;
  }
}

/*********** ALGO RYTHM QUANTIZER *******/
void handleQuantizer() {
  if (trigStage == NEW && inp > 200) {  // INPUT HIGH && HOLD HIGH
    sample = extOrInt ? inp : random(minPot, maxPot);
  } else if (trigStage == OFF) {
    sample = 0;
  }
}

/*********** ALGO CVRECORDER *******/
static const int loopBufferSize = 600;  // Taille ajustée pour le buffer
static int loopBuffer[loopBufferSize];  // Buffer pour stocker les échantillons
void handleLoopRecorder() {
  static unsigned long cvRecStepDuration = 100;
  static unsigned long cvRecLastStep = 0;
  static int playbackIndex = 0;     // Index pour la lecture
  static int recordedSize = 0;      // Taille réelle des données enregistrées
  static bool isRecording = false;  // Indique si on est en mode enregistrement

  cvRecStepDuration = map(maxPot, 0, 1023, 100, 10);
  if (trigStage == NEW || trigStage == OLD) {
    if (trigStage == NEW) {
      recordedSize = 1;  // Réinitialise la taille enregistrée
      isRecording = true;
    }
    if (millis() - cvRecLastStep > cvRecStepDuration) {
      // Enregistrer l'entrée dans le buffer
      sample = constrain(minPot + inp, 0, 1023);
      loopBuffer[recordedSize] = sample;                   // Enregistre l'entrée actuelle
      recordedSize = (recordedSize + 1) % loopBufferSize;  // Incrémente avec wrap-around
      cvRecLastStep = millis();
    }


  } else {
    // Si on était en mode enregistrement, passer en mode lecture
    if (isRecording) {
      isRecording = false;
      playbackIndex = 0;  // Réinitialise l'index de lecture
    }

    // Lecture en boucle si ext ON
    if (extOrInt) {
      if (millis() - cvRecLastStep > cvRecStepDuration) {
        sample = constrain(loopBuffer[playbackIndex], 0, 1023);  // Lit l'échantillon depuis le buffer && le contraint à min/max
        playbackIndex = (playbackIndex + 1) % recordedSize;      // Incrémente avec wrap-around limité à recordedSize
        cvRecLastStep = millis();
      }
    } else {
      sample = constrain(minPot + inp, 0, 1023);  // Si ni enregistrement ni lecture, échantillon à 0
    }
  }
}

/*********** ALGO TURING MACHINE *******/
const int turingBufferSize = 16;     // Taille du registre de la Turing Machine
int turingBuffer[turingBufferSize];  // Registre circulaire

void handleTuringMachine() {
  static int turingIndex = 0;          // Index actuel dans le registre
  static int turingLength = 8;         // Longueur active du registre (1 à turingBufferSize)
  static int turingProbability = 0.5;  // Probabilité de changement (0.0 à 1.0)

  turingLength = constrain((minPot >> 6) + 1, 1, turingBufferSize);  // Longueur entre 1 et turingBufferSize (1024 >> 6 = 16)
  turingProbability = maxPot;                                        // Probabilité entre 0.0 et 1.0
  if (trigStage == NEW) {
    // Générer un nouveau bit avec une probabilité de changement
    if (random(0, 1024) < turingProbability) {
      int newBit = extOrInt ? inp : random(0, 1024);
      turingBuffer[turingIndex] = newBit;
    }
    turingIndex = (turingIndex + 1) % turingLength;
    sample = turingBuffer[turingIndex];
  }
}

/*********** ALGO BURST *******/
#include <ArduinoTapTempo.h>
ArduinoTapTempo tapTempo;
void handleBurst() {
  static unsigned long tapCurrentTime = 0;
  static unsigned long tapLastBeat = 0;
  static unsigned long tapDuration = 0;
  static unsigned long stepDuration = 0;
  static byte stepCount = 0;
  static byte tapMult = 1;
  static const byte mult_options[8] = { 2, 3, 4, 5, 6, 8, 10, 12 };

  tapCurrentTime = millis();
  tapMult = map(maxPot, 0, 1023, 0, 7);  // Multiplier
  if (trigStage == NEW) {
    tapTempo.update(true);
    tapLastBeat = millis();
  } else if (trigStage == OFF) {
    tapTempo.update(false);
  }
  tapDuration = tapTempo.getBeatLength();
  stepDuration = (int)(tapDuration / (float)mult_options[tapMult]);
  /*if (tapTempo.onBeat()) {
    tapLastBeat = millis();
    sample = extOrInt ? inp : 1023;
    stepCount = 0;
    digitalWrite(ledPin, HIGH);
  } else */
  if (millis() - tapLastBeat >= stepDuration) {
    tapLastBeat = millis();
    sample = extOrInt ? inp : 1023;
    stepCount++;
    digitalWrite(ledPin, HIGH);
  } else if (millis() - tapLastBeat <= MINGATE + minPot / 4) {
    sample = extOrInt ? inp : 1023;
    digitalWrite(ledPin, HIGH);
  } else {
    sample = 0;
    digitalWrite(ledPin, LOW);
  }
}

#endif