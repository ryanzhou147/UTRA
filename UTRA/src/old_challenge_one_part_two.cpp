/**
 * Challenge One Part Two - Standalone return navigation
 * Runs after Challenge One completes (at black center).
 * Sequence: Find wall -> Align -> Return to ramp -> Descend -> Find green -> Follow home
 */

#include <Arduino.h>

// --- TCS3200 color sensor pins ---
const int S0 = 2;
const int S1 = 3;
const int S2 = 4;
const int S3 = 5;
const int S_OUT = 6;
const int blackThreshold = 100;
const int whiteThreshold = 40;
int redPW = 0;
int greenPW = 0;
int bluePW = 0;

typedef enum {
  PATH_WHITE = 0,
  PATH_RED = 1,
  PATH_GREEN = 2,
  PATH_BLUE = 3,
  PATH_BLACK = 4
} PathColour;

// --- Constants ---
const int TURN_SPEED = 120;
const int DRIVE_SPEED = 180;
const int RAMP_SPEED = 220;
const int LINE_FOLLOW_SPEED = 100;
const int REVERSE_SPEED = 100;
const unsigned long FOLLOW_HOME_DURATION_MS = 8000;

// --- Ultrasonic sensor (HC-SR04) ---
// Note: TRIG/ECHO use pins 7,8 to avoid conflict with TCS3200 (S0=2, S2=4)
const int TRIG_PIN = 7;
const int ECHO_PIN = 8;
const int SCAN_STEP_DEGREES = 12;
const int NUM_SCAN_STEPS = 360 / SCAN_STEP_DEGREES;
const unsigned long SCAN_STEP_DURATION_MS = 250;
const unsigned long ULTRASONIC_TIMEOUT_US = 30000;

// --- Motor pins (L298N) ---
const int IN1 = 9;
const int IN2 = 10;
const int IN3 = 6;
const int IN4 = 5;
const int ENA_PIN = 11;
const int ENB_PIN = 3;

// --- State machine ---
enum class ChallengeTwoState {
  FIND_WALL_ANGLE,
  ALIGN_TO_WALL,
  RETURN_TO_RAMP,
  DESCEND_RAMP,
  FIND_GREEN_LINE,
  FOLLOW_GREEN_HOME,
  DONE
};

ChallengeTwoState challengeTwoState = ChallengeTwoState::FIND_WALL_ANGLE;
unsigned long startTime = 0;
unsigned long centerTime = 0;
int wallAngleDegrees = 0;
int scanStepIndex = 0;
unsigned long scanStepStartTime = 0;
float scanDistances[NUM_SCAN_STEPS];
bool scanRotationPhase = true;
int colorChanges = 0;
PathColour currentColor = PATH_BLACK;

// --- Forward declarations (static = file-local, no conflict with challenge_one.cpp) ---
static PathColour getColour();
static bool isOnGreenLine();
static bool isOnRedSurface();
static bool isOnBlackTape();
static String getEnumColor(PathColour c);
static void driveMotor(int leftPWM, int rightPWM);
static void stop();
static void turnLeft(int pwm = -1);
static void turnRight(int pwm = -1);
static float measureDistanceCm();
static void driveBackward(int speed);

// ========== Color sensor (TCS3200) ==========
static PathColour getColour() {
  redPW = getRedPW();
  greenPW = getGreenPW();
  bluePW = getBluePW();

  if (redPW > blackThreshold && greenPW > blackThreshold && bluePW > blackThreshold) {
    return PATH_BLACK;
  }

  if (redPW < whiteThreshold && greenPW < whiteThreshold && bluePW < whiteThreshold) {
    return PATH_WHITE;
  }

  int maxColor = min(redPW, min(greenPW, bluePW));
  if (maxColor == redPW) {
    return PATH_RED;
  }
  else if (maxColor == greenPW) {
    return PATH_GREEN;
  }
  else {
    return PATH_BLUE;
  }
}

static int getRedPW() {
  digitalWrite(S2, LOW);
  digitalWrite(S3, LOW);
  delay(10);
  int PW = pulseIn(S_OUT, LOW);
  return PW;
}

static int getGreenPW() {
  digitalWrite(S2, HIGH);
  digitalWrite(S3, HIGH);
  delay(10);
  int PW = pulseIn(S_OUT, LOW);
  return PW;
}

static int getBluePW() {
  digitalWrite(S2, LOW);
  digitalWrite(S3, HIGH);
  delay(10);
  int PW = pulseIn(S_OUT, LOW);
  return PW;
}

static bool isOnGreenLine() { return getColour() == PATH_GREEN; }
static bool isOnRedSurface() { return getColour() == PATH_RED; }
static bool isOnBlackTape() { return getColour() == PATH_BLACK; }

static String getEnumColor(PathColour c) {
  switch (c) {
    case PATH_RED:   return "RED";
    case PATH_GREEN: return "GREEN";
    case PATH_BLUE:  return "BLUE";
    case PATH_BLACK: return "BLACK";
    case PATH_WHITE: return "WHITE";
    default:         return "ERROR";
  }
}

// ========== Motor control ==========
static void driveMotor(int leftPWM, int rightPWM) {
  int lp = constrain(abs(leftPWM), 0, 255);
  int rp = constrain(abs(rightPWM), 0, 255);
  if (ENA_PIN >= 0) analogWrite(ENA_PIN, lp); else lp = 255;
  if (ENB_PIN >= 0) analogWrite(ENB_PIN, rp); else rp = 255;

  if (leftPWM >= 0) {
    digitalWrite(IN1, lp > 0 ? HIGH : LOW);
    digitalWrite(IN2, LOW);
  } else {
    digitalWrite(IN1, LOW);
    digitalWrite(IN2, lp > 0 ? HIGH : LOW);
  }
  if (rightPWM >= 0) {
    digitalWrite(IN3, rp > 0 ? HIGH : LOW);
    digitalWrite(IN4, LOW);
  } else {
    digitalWrite(IN3, LOW);
    digitalWrite(IN4, rp > 0 ? HIGH : LOW);
  }
}

static void stop() { driveMotor(0, 0); }

static void turnLeft(int pwm = -1) {
  int s = (pwm >= 0) ? pwm : TURN_SPEED;
  driveMotor(-s, s);
}

static void turnRight(int pwm = -1) {
  int s = (pwm >= 0) ? pwm : TURN_SPEED;
  driveMotor(s, -s);
}

static void driveBackward(int speed) {
  driveMotor(-speed, -speed);
}

// ========== Ultrasonic ==========
static float measureDistanceCm() {
  digitalWrite(TRIG_PIN, LOW);
  delayMicroseconds(2);
  digitalWrite(TRIG_PIN, HIGH);
  delayMicroseconds(10);
  digitalWrite(TRIG_PIN, LOW);
  long duration = pulseIn(ECHO_PIN, HIGH, ULTRASONIC_TIMEOUT_US);
  if (duration <= 0) return 999.0f;
  return (duration / 2.0f) / 29.1f;
}

// ========== Initialization ==========
void initChallengeTwo() {
  pinMode(IN1, OUTPUT);
  pinMode(IN2, OUTPUT);
  pinMode(IN3, OUTPUT);
  pinMode(IN4, OUTPUT);
  if (ENA_PIN >= 0) pinMode(ENA_PIN, OUTPUT);
  if (ENB_PIN >= 0) pinMode(ENB_PIN, OUTPUT);
  pinMode(TRIG_PIN, OUTPUT);
  pinMode(ECHO_PIN, INPUT);
  pinMode(S0, OUTPUT);
  pinMode(S1, OUTPUT);
  pinMode(S2, OUTPUT);
  pinMode(S3, OUTPUT);
  pinMode(S_OUT, INPUT);
  digitalWrite(S0, HIGH);
  digitalWrite(S1, LOW);

  challengeTwoState = ChallengeTwoState::FIND_WALL_ANGLE;
  scanStepIndex = 0;
  scanRotationPhase = true;
  scanStepStartTime = millis();
}

// ========== Challenge Two state machine ==========
void challengeTwo() {
  switch (challengeTwoState) {
    case ChallengeTwoState::FIND_WALL_ANGLE: {
      if (scanRotationPhase) {
        driveMotor(TURN_SPEED, -TURN_SPEED);
        if ((millis() - scanStepStartTime) >= SCAN_STEP_DURATION_MS) {
          driveMotor(0, 0);
          delay(50);
          scanDistances[scanStepIndex] = measureDistanceCm();
          scanStepIndex++;
          if (scanStepIndex >= NUM_SCAN_STEPS) {
            float minDist = 999.0f;
            int minIdx = 0;
            for (int i = 0; i < NUM_SCAN_STEPS; i++) {
              if (scanDistances[i] < minDist && scanDistances[i] > 2.0f) {
                minDist = scanDistances[i];
                minIdx = i;
              }
            }
            wallAngleDegrees = minIdx * SCAN_STEP_DEGREES;
            startTime = millis();
            centerTime = (wallAngleDegrees * SCAN_STEP_DURATION_MS) / SCAN_STEP_DEGREES;
            challengeTwoState = ChallengeTwoState::ALIGN_TO_WALL;
          } else {
            scanStepStartTime = millis();
          }
        }
      }
      break;
    }

    case ChallengeTwoState::ALIGN_TO_WALL: {
      driveMotor(TURN_SPEED, -TURN_SPEED);
      if ((millis() - startTime) >= centerTime) {
        driveMotor(0, 0);
        delay(500);
        challengeTwoState = ChallengeTwoState::RETURN_TO_RAMP;
      }
      break;
    }

    case ChallengeTwoState::RETURN_TO_RAMP: {
      driveBackward(REVERSE_SPEED);
      if (isOnRedSurface()) {
        stop();
        delay(500);
        challengeTwoState = ChallengeTwoState::DESCEND_RAMP;
      }
      break;
    }

    case ChallengeTwoState::DESCEND_RAMP: {
      driveBackward(RAMP_SPEED);
      if (isOnGreenLine()) {
        stop();
        delay(500);
        challengeTwoState = ChallengeTwoState::FIND_GREEN_LINE;
      }
      break;
    }

    case ChallengeTwoState::FIND_GREEN_LINE: {
      if (isOnGreenLine()) {
        stop();
        delay(300);
        startTime = millis();
        challengeTwoState = ChallengeTwoState::FOLLOW_GREEN_HOME;
      } else {
        turnLeft(LINE_FOLLOW_SPEED);
      }
      break;
    }

    case ChallengeTwoState::FOLLOW_GREEN_HOME: {
      if (isOnGreenLine()) {
        driveBackward(LINE_FOLLOW_SPEED);
      } else {
        turnLeft(LINE_FOLLOW_SPEED);
      }
      if ((millis() - startTime) >= FOLLOW_HOME_DURATION_MS) {
        stop();
        challengeTwoState = ChallengeTwoState::DONE;
      }
      break;
    }

    case ChallengeTwoState::DONE:
      stop();
      break;
  }
}

// For standalone Part Two only: uncomment below and exclude challenge_one.cpp + main.cpp from build
/*
void setup() {
  Serial.begin(9600);
  Serial.println("UTRA: Challenge One Part Two starting...");
  initChallengeTwo();
}
void loop() { challengeTwo(); }
*/
