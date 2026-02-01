/**
 * Challenge One Part Two - Standalone return navigation
 * Runs after Challenge One completes (at black center).
 * Sequence: Find wall -> Align -> Return to ramp -> Descend -> Find green -> Follow home
 */

#include <Arduino.h>
#include <Wire.h>

// --- Color enum (enum class to avoid conflict with challenge_one.cpp) ---
enum class PartTwoColor { red, green, blue, black, error };

// --- Constants ---
const int TURN_SPEED = 120;
const int DRIVE_SPEED = 180;
const int RAMP_SPEED = 220;
const int LINE_FOLLOW_SPEED = 100;
const int REVERSE_SPEED = 100;
const unsigned long FOLLOW_HOME_DURATION_MS = 8000;

// --- Ultrasonic sensor (HC-SR04) ---
const int TRIG_PIN = 2;
const int ECHO_PIN = 4;
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

// --- Color sensor ---
const int COLOR_SENSOR_SDA = A4;
const int COLOR_SENSOR_SCL = A5;

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
PartTwoColor currentColor = PartTwoColor::black;

// --- Forward declarations (static = file-local, no conflict with challenge_one.cpp) ---
static PartTwoColor getColor();
static bool isOnGreenLine();
static bool isOnRedSurface();
static bool isOnBlackTape();
static String getEnumColor(color c);
static void driveMotor(int leftPWM, int rightPWM);
static void stop();
static void turnLeft(int pwm = -1);
static void turnRight(int pwm = -1);
static float measureDistanceCm();
static void driveBackward(int speed);

// ========== Color sensor ==========
static PartTwoColor getColor() {
  static unsigned long lastRead = 0;
  static PartTwoColor stubColor = PartTwoColor::black;
  if (millis() - lastRead > 100) {
    lastRead = millis();
    int r = analogRead(A0);
    if (r < 200) stubColor = PartTwoColor::black;
    else if (r < 400) stubColor = PartTwoColor::blue;
    else if (r < 600) stubColor = PartTwoColor::green;
    else if (r < 800) stubColor = PartTwoColor::red;
    else stubColor = PartTwoColor::black;
  }
  return stubColor;
}

static bool isOnGreenLine() { return getColor() == PartTwoColor::green; }
static bool isOnRedSurface() { return getColor() == PartTwoColor::red; }
static bool isOnBlackTape() { return getColor() == PartTwoColor::black; }

static String getEnumColor(PartTwoColor c) {
  switch (c) {
    case PartTwoColor::red:   return "RED";
    case PartTwoColor::green: return "GREEN";
    case PartTwoColor::blue:  return "BLUE";
    case PartTwoColor::black: return "BLACK";
    default: return "ERROR";
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
