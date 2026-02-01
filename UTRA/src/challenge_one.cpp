/**
 * Challenge One - Sequential navigation stages (Part 1)
 * COLOR SEQUENCE: Green line -> Black lines (ramp) -> Red (platform) -> Green (boundary) -> Black (center)
 * Stage 1: Follow green line
 * Stage 2: Detect black (ramp base), ascend until red
 * Stage 3: Red = on platform
 * Stage 4: Navigate Red -> Green boundary -> Black center -> DONE
 * Part Two (wall scan, return home) runs from challenge_one_part_two.cpp
 */

#include <Arduino.h>
#include <Wire.h>

// --- Color enum ---
enum color { red, green, blue, black, error };

// --- Constants ---
const int TURN_SPEED = 120;
const int DRIVE_SPEED = 180;
const int RAMP_SPEED = 220;
const int LINE_FOLLOW_SPEED = 100;
const int turnBackTimeOffset = 900;
const int COLOR_CHANGES_TO_CENTER = 5;
const int COLOR_CHANGES_FOR_EDGES = 3;

// --- Motor pins (L298N) ---
const int IN1 = 9;   // Left motor direction
const int IN2 = 10;  // Left motor direction
const int IN3 = 6;   // Right motor direction
const int IN4 = 5;   // Right motor direction
const int ENA_PIN = 11;  // Left motor PWM (-1 if jumper on)
const int ENB_PIN = 3;   // Right motor PWM (-1 if jumper on)

// --- Color sensor (TCS34725 via I2C) - simplified stub when no hardware ---
const int COLOR_SENSOR_SDA = A4;
const int COLOR_SENSOR_SCL = A5;
bool colorSensorReady = false;

// --- State machine ---
enum class ChallengeOneState {
  STAGE1_GREEN_PATH,       // Follow green line on ground
  STAGE2_RAMP_ASCENT,      // Black lines = ramp base, ascend until red
  STAGE3_PLATFORM_DETECTED,// Red = on platform
  STAGE4_PLATFORM_NAV,     // Red -> Green boundary -> Black center
  DONE
};

ChallengeOneState challengeOneState = ChallengeOneState::STAGE1_GREEN_PATH;
unsigned long startTime = 0;
unsigned long timestampOne = 0;
unsigned long timestampTwo = 0;
unsigned long centerTime = 0;
int colorChanges = 0;
color currentColor = error;

// --- Forward declarations ---
color getColor();
bool isColorFound();
bool isOnBlackTape();
bool isOnGreenLine();
bool isOnRedSurface();
String getEnumColor(color c);
void driveMotor(int leftPWM, int rightPWM);
void drive();
void stop();
void reverse();
void turnLeft(int pwm = -1);
void turnRight(int pwm = -1);
void followGreenLine();
void followBlackLine();
void followBlackTape();
bool followBlackTapeUntilCenter();
bool isAtRampTop();
void timeSave();

// ========== Color sensor ==========
// Uses raw I2C to read TCS34725, or returns stub when unavailable.
// To use Adafruit TCS34725 library, replace this section and add lib_deps.
color getColor() {
  // Stub: simulate color detection for development without hardware.
  // In production, read from TCS34725 (I2C) and classify by RGB thresholds.
  static unsigned long lastRead = 0;
  static color stubColor = black;
  if (millis() - lastRead > 100) {
    lastRead = millis();
    int r = analogRead(A0);  // Use A0 as placeholder; replace with sensor read
    if (r < 200) stubColor = black;
    else if (r < 400) stubColor = blue;
    else if (r < 600) stubColor = green;
    else if (r < 800) stubColor = red;
    else stubColor = error;
  }
  return stubColor;
}

bool isColorFound() {
  color c = getColor();
  return (c == black);
}

bool isOnBlackTape() {
  return getColor() == black;
}

bool isOnGreenLine() {
  return getColor() == green;
}

bool isOnRedSurface() {
  return getColor() == red;
}

String getEnumColor(color c) {
  switch (c) {
    case red:   return "RED";
    case green: return "GREEN";
    case blue:  return "BLUE";
    case black: return "BLACK";
    default:    return "ERROR";
  }
}

// ========== Motor control ==========
void driveMotor(int leftPWM, int rightPWM) {
  int lp = constrain(abs(leftPWM), 0, 255);
  int rp = constrain(abs(rightPWM), 0, 255);

  if (ENA_PIN >= 0) { analogWrite(ENA_PIN, lp); } else { lp = 255; }
  if (ENB_PIN >= 0) { analogWrite(ENB_PIN, rp); } else { rp = 255; }

  // Left motor
  if (leftPWM >= 0) {
    digitalWrite(IN1, lp > 0 ? HIGH : LOW);
    digitalWrite(IN2, LOW);
  } else {
    digitalWrite(IN1, LOW);
    digitalWrite(IN2, lp > 0 ? HIGH : LOW);
  }

  // Right motor
  if (rightPWM >= 0) {
    digitalWrite(IN3, rp > 0 ? HIGH : LOW);
    digitalWrite(IN4, LOW);
  } else {
    digitalWrite(IN3, LOW);
    digitalWrite(IN4, rp > 0 ? HIGH : LOW);
  }
}

void drive() {
  driveMotor(DRIVE_SPEED, DRIVE_SPEED);
}

void stop() {
  driveMotor(0, 0);
}

void reverse() {
  driveMotor(-DRIVE_SPEED, -DRIVE_SPEED);
  delay(200);
  driveMotor(0, 0);
}

void turnLeft(int pwm /* = -1 */) {
  int s = (pwm >= 0) ? pwm : TURN_SPEED;
  driveMotor(-s, s);
}

void turnRight(int pwm /* = -1 */) {
  int s = (pwm >= 0) ? pwm : TURN_SPEED;
  driveMotor(s, -s);
}

// ========== Line following ==========
void followGreenLine() {
  if (isOnGreenLine()) {
    driveMotor(LINE_FOLLOW_SPEED, LINE_FOLLOW_SPEED);
  } else {
    turnLeft(LINE_FOLLOW_SPEED);  // Lost line: rotate to find it
  }
}

void followBlackLine() {
  if (isOnBlackTape()) {
    driveMotor(LINE_FOLLOW_SPEED, LINE_FOLLOW_SPEED);
  } else {
    turnLeft(LINE_FOLLOW_SPEED);
  }
}

void followBlackTape() {
  followBlackLine();  // Alias
}

bool followBlackTapeUntilCenter() {
  followBlackTape();
  // Center detected when we've seen sustained black (simplified)
  static int blackCount = 0;
  if (isOnBlackTape()) {
    blackCount++;
    if (blackCount > 20) { blackCount = 0; return true; }
  } else {
    blackCount = 0;
  }
  return false;
}

bool isAtRampTop() {
  return false;
}

// ========== Utility ==========
void timeSave() {
  // Placeholder for turn-back timing (from last year)
}

// ========== Initialization ==========
void initChallengeOne() {
  pinMode(IN1, OUTPUT);
  pinMode(IN2, OUTPUT);
  pinMode(IN3, OUTPUT);
  pinMode(IN4, OUTPUT);
  if (ENA_PIN >= 0) pinMode(ENA_PIN, OUTPUT);
  if (ENB_PIN >= 0) pinMode(ENB_PIN, OUTPUT);

  challengeOneState = ChallengeOneState::STAGE1_GREEN_PATH;
}

bool isChallengeOneComplete() {
  return challengeOneState == ChallengeOneState::DONE;
}

// ========== Main challenge state machine ==========
void challengeOne() {
  timeSave();

  switch (challengeOneState) {
    // --- STAGE 1: Initial path following (green line) ---
    case ChallengeOneState::STAGE1_GREEN_PATH: {
      if (isOnBlackTape()) {
        stop();
        delay(500);
        challengeOneState = ChallengeOneState::STAGE2_RAMP_ASCENT;
      } else {
        followGreenLine();
      }
      break;
    }

    // --- STAGE 2: Ramp detection and ascent (black lines -> red) ---
    case ChallengeOneState::STAGE2_RAMP_ASCENT: {
      if (isOnRedSurface()) {
        stop();
        delay(500);
        challengeOneState = ChallengeOneState::STAGE3_PLATFORM_DETECTED;
      } else if (isOnBlackTape()) {
        driveMotor(RAMP_SPEED, RAMP_SPEED);  // On black: ascend ramp
      } else {
        turnLeft(RAMP_SPEED);  // Lost line: search for it
      }
      break;
    }

    // --- STAGE 3: Platform detected (red surface) ---
    case ChallengeOneState::STAGE3_PLATFORM_DETECTED: {
      stop();
      delay(300);
      challengeOneState = ChallengeOneState::STAGE4_PLATFORM_NAV;
      break;
    }

    // --- STAGE 4: Platform navigation (Red -> Green boundary -> Black center) ---
    case ChallengeOneState::STAGE4_PLATFORM_NAV: {
      driveMotor(DRIVE_SPEED, DRIVE_SPEED);
      colorChanges = 0;
      currentColor = getColor();

      // Navigate through concentric zones: red -> green -> black
      while (colorChanges < 2) {
        while (getColor() == currentColor) { /* wait */ }
        currentColor = getColor();
        colorChanges++;
      }
      // Now on black center zone - Part Two (challenge_one_part_two) continues from here
      stop();
      delay(500);
      challengeOneState = ChallengeOneState::DONE;
      break;
    }

    case ChallengeOneState::DONE:
      driveMotor(0, 0);
      break;
  }
}
