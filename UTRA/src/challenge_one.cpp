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
PathColour currentColor = PATH_WHITE;

// --- Forward declarations ---
PathColour getColour();
bool isColorFound();
bool isOnBlackTape();
bool isOnGreenLine();
bool isOnRedSurface();
String getEnumColor(PathColour c);
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

// ========== Color sensor (TCS3200) ==========
PathColour getColour() {
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

int getRedPW() {
  digitalWrite(S2, LOW);
  digitalWrite(S3, LOW);
  delay(10);
  int PW = pulseIn(S_OUT, LOW);
  return PW;
}

int getGreenPW() {
  digitalWrite(S2, HIGH);
  digitalWrite(S3, HIGH);
  delay(10);
  int PW = pulseIn(S_OUT, LOW);
  return PW;
}

int getBluePW() {
  digitalWrite(S2, LOW);
  digitalWrite(S3, HIGH);
  delay(10);
  int PW = pulseIn(S_OUT, LOW);
  return PW;
}

bool isColorFound() {
  PathColour c = getColour();
  return (c == PATH_BLACK);
}

bool isOnBlackTape() {
  return getColour() == PATH_BLACK;
}

bool isOnGreenLine() {
  return getColour() == PATH_GREEN;
}

bool isOnRedSurface() {
  return getColour() == PATH_RED;
}

String getEnumColor(PathColour c) {
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
void driveMotor(int leftPWM, int rightPWM) {
  int lp = constrain(abs(leftPWM), 0, 255);
  int rp = constrain(abs(rightPWM), 0, 255);

  if (ENA_PIN >= 0) { analogWrite(ENA_PIN, lp); } else { lp = 255; }
  if (ENB_PIN >= 0) { analogWrite(ENB_PIN, rp); } else { rp = 255; }

  // Left motor
  if (leftPWM >= 0) {
    digitalWrite(IN1, LOW);
    digitalWrite(IN2, lp > 0 ? HIGH : LOW);
  } else {
    digitalWrite(IN1, lp > 0 ? HIGH : LOW);
    digitalWrite(IN2, LOW);
  }

  // Right motor
  if (rightPWM >= 0) {
    digitalWrite(IN3, LOW);
    digitalWrite(IN4, rp > 0 ? HIGH : LOW);
  } else {
    digitalWrite(IN3, rp > 0 ? HIGH : LOW);
    digitalWrite(IN4, LOW);
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
  pinMode(S0, OUTPUT);
  pinMode(S1, OUTPUT);
  pinMode(S2, OUTPUT);
  pinMode(S3, OUTPUT);
  pinMode(S_OUT, INPUT);
  digitalWrite(S0, HIGH);
  digitalWrite(S1, LOW);

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
      currentColor = getColour();

      // Navigate through concentric zones: red -> green -> black
      while (colorChanges < 2) {
        while (getColour() == currentColor) { /* wait */ }
        currentColor = getColour();
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
