// servo
#include <Servo.h>
Servo servo;
const int SERVOPIN = A3;

// ultrasonic sensor
const int TRIGPIN = A4;
const int ECHOPIN = A5;
long duration;

// dc motor
const int LEFT1 = 9;
const int LEFT2 = 10;
const int RIGHT1 = 11;
const int RIGHT2 = 12;
const int ENA = 7;  // Enable pin for left motor
const int ENB = 8;  // Enable pin for right motor

// colour sensor
const int S0 = 2;
const int S1 = 3;
const int S2 = 4;
const int S3 = 5;
const int S_OUT = 6;
const int blackThreshold = 100;
const int whiteThreshold = 40;
int redPW = 0;

typedef enum {
  PATH_WHITE = 0,
  PATH_RED = 1,
  PATH_BLACK = 2
} PathColour;

typedef enum {
  STATE_FOLLOW_RED = 0,
  STATE_AVOID_LEFT,
  STATE_AVOID_FORWARD1,
  STATE_AVOID_RIGHT1,
  STATE_AVOID_FORWARD2,
  STATE_AVOID_RIGHT2,
  STATE_AVOID_FORWARD3,
  STATE_ALIGN_LEFT,
  STATE_TURN_RIGHT_INTERSECTION,
  STATE_END
} RobotState;

typedef enum {
  TURN_LEFT_DIR = 0,
  TURN_RIGHT_DIR = 1
} TurnDir;

const int DISTANCE_THRESHOLD_CM = 20;
const int OBSTACLE_DEBOUNCE_HITS = 3;
const int WHITE_STREAK_HITS = 6;
const unsigned long TURN_LEFT_MS = 450;
const unsigned long TURN_RIGHT_MS = 450;
const unsigned long FORWARD1_MS = 700;
const unsigned long FORWARD2_MS = 700;
const unsigned long FORWARD3_MS = 900;
const unsigned long ALIGN_LEFT_MS = 350;
const unsigned long INTERSECTION_RIGHT_MS = 450;
const unsigned long COLOR_PULSE_TIMEOUT_US = 10000;
const unsigned long ECHO_PULSE_TIMEOUT_US = 30000;

RobotState robotState = STATE_FOLLOW_RED;
TurnDir lastTurnDir = TURN_LEFT_DIR;
unsigned long stateStartMs = 0;
int obstacleHitCount = 0;
int whiteStreakCount = 0;

void setRobotState(RobotState state);
void followRed(PathColour color);
bool isObstacleDetected(int cm);
bool isRed(PathColour color);
PathColour getColour();
int getRedPW();
void moveForward();
void moveLeft();
void moveRight();
void stopMotors();
int getDistance();

void setRobotState(RobotState state) {
  robotState = state;
  stateStartMs = millis();
  if (state != STATE_FOLLOW_RED) {
    obstacleHitCount = 0;
  }
  if (state != STATE_FOLLOW_RED) {
    whiteStreakCount = 0;
  }
  Serial.print("State: ");
  Serial.println(state);
}

bool isObstacleDetected(int cm) {
  if (cm > 0 && cm < DISTANCE_THRESHOLD_CM) {
    obstacleHitCount++;
  } else {
    obstacleHitCount = 0;
  }
  return obstacleHitCount >= OBSTACLE_DEBOUNCE_HITS;
}

bool isRed(PathColour color) {
  return color == PATH_RED;
}

void followRed(PathColour color) {
  if (color == PATH_RED) {
    moveForward();
  } else if (color == PATH_WHITE) {
    if (lastTurnDir == TURN_LEFT_DIR) {
      moveLeft();
      lastTurnDir = TURN_LEFT_DIR;
    } else {
      moveRight();
      lastTurnDir = TURN_RIGHT_DIR;
    }
  } else {
    stopMotors();
  }
}

// black threshold: red pulse over 100
// white threshold: red pulse under 40
// white = 0, red = 1, black = 2
PathColour getColour()
{
  redPW = getRedPW();

  if (redPW > blackThreshold) {
    return PATH_BLACK;
  }

  if (redPW < whiteThreshold) {
    return PATH_WHITE;
  }

  return PATH_RED;
}

int getRedPW() {
  digitalWrite(S2, LOW);
  digitalWrite(S3, LOW);
  delay(10);
  int PW = pulseIn(S_OUT, LOW, COLOR_PULSE_TIMEOUT_US);
  return PW;
}

void moveForward() {
  digitalWrite(LEFT1, HIGH);
  digitalWrite(LEFT2, LOW);
  digitalWrite(RIGHT1, HIGH);
  digitalWrite(RIGHT2, LOW);
}

void moveLeft() {
  digitalWrite(RIGHT1, HIGH);
  digitalWrite(RIGHT2, LOW);
  digitalWrite(LEFT1, LOW);
  digitalWrite(LEFT2, HIGH);
}

void moveRight() {
  digitalWrite(LEFT1, HIGH);
  digitalWrite(LEFT2, LOW);
  digitalWrite(RIGHT1, LOW);
  digitalWrite(RIGHT2, HIGH);
}

void stopMotors() {
  digitalWrite(LEFT1, LOW);
  digitalWrite(LEFT2, LOW);
  digitalWrite(RIGHT1, LOW);
  digitalWrite(RIGHT2, LOW);
}

int getDistance() {
  digitalWrite(TRIGPIN, LOW);
  delayMicroseconds(2);
  digitalWrite(TRIGPIN, HIGH);
  delayMicroseconds(10);
  digitalWrite(TRIGPIN, LOW);
  
  duration = pulseIn(ECHOPIN, HIGH, ECHO_PULSE_TIMEOUT_US);
  if (duration == 0) return 99999;
  return duration * 340 / 200;
}

void setup()
{
  servo.attach(SERVOPIN);
  servo.write(0);

  // ultrasonic sensor
  pinMode(TRIGPIN, OUTPUT);
  pinMode(ECHOPIN, INPUT_PULLUP);

  // dc motor
  pinMode(LEFT1, OUTPUT);
  pinMode(LEFT2, OUTPUT);
  pinMode(RIGHT1, OUTPUT);
  pinMode(RIGHT2, OUTPUT);
  pinMode(ENA, OUTPUT);
  pinMode(ENB, OUTPUT);
  digitalWrite(ENA, HIGH);  // Enable left motor
  digitalWrite(ENB, HIGH);  // Enable right motor
  
  // colour sensor
  pinMode(S0, OUTPUT);
  pinMode(S1, OUTPUT);
  pinMode(S2, OUTPUT);
  pinMode(S3, OUTPUT);

  // set PW scaling to 20%
  digitalWrite(S0, HIGH);
  digitalWrite(S1, LOW);

  // set sensor output as input
  pinMode(S_OUT, INPUT);

  Serial.begin(9600);
  setRobotState(STATE_FOLLOW_RED);
}

void loop()
{
  PathColour color = getColour();
  int cm = getDistance();

  Serial.print("Color: ");
  switch (color) {
    case PATH_WHITE:
      Serial.println("WHITE");
      break;
    case PATH_RED:
      Serial.println("RED");
      break;
    case PATH_BLACK:
      Serial.println("BLACK");
      break;
  }

  switch (robotState) {
    case STATE_FOLLOW_RED:
      if (color == PATH_BLACK) {
        stopMotors();
        setRobotState(STATE_END);
        break;
      }

      if (color == PATH_WHITE) {
        whiteStreakCount++;
        if (whiteStreakCount >= WHITE_STREAK_HITS) {
          setRobotState(STATE_TURN_RIGHT_INTERSECTION);
          break;
        }
      } else {
        whiteStreakCount = 0;
      }

      if (isObstacleDetected(cm)) {
        stopMotors();
        delay(100);
        setRobotState(STATE_AVOID_LEFT);
        break;
      }
      followRed(color);
      break;

    case STATE_AVOID_LEFT:
      moveLeft();
      if (millis() - stateStartMs >= TURN_LEFT_MS) {
        setRobotState(STATE_AVOID_FORWARD1);
      }
      break;

    case STATE_AVOID_FORWARD1:
      moveForward();
      if (isRed(color)) {
        setRobotState(STATE_ALIGN_LEFT);
      } else if (millis() - stateStartMs >= FORWARD1_MS) {
        setRobotState(STATE_AVOID_RIGHT1);
      }
      break;

    case STATE_AVOID_RIGHT1:
      moveRight();
      if (millis() - stateStartMs >= TURN_RIGHT_MS) {
        setRobotState(STATE_AVOID_FORWARD2);
      }
      break;

    case STATE_AVOID_FORWARD2:
      moveForward();
      if (isRed(color)) {
        setRobotState(STATE_ALIGN_LEFT);
      } else if (millis() - stateStartMs >= FORWARD2_MS) {
        setRobotState(STATE_AVOID_RIGHT2);
      }
      break;

    case STATE_AVOID_RIGHT2:
      moveRight();
      if (millis() - stateStartMs >= TURN_RIGHT_MS) {
        setRobotState(STATE_AVOID_FORWARD3);
      }
      break;

    case STATE_AVOID_FORWARD3:
      moveForward();
      if (isRed(color)) {
        setRobotState(STATE_ALIGN_LEFT);
      } else if (millis() - stateStartMs >= FORWARD3_MS) {
        // keep going forward until red is found
        stateStartMs = millis();
      }
      break;

    case STATE_ALIGN_LEFT:
      moveLeft();
      if (millis() - stateStartMs >= ALIGN_LEFT_MS) {
        setRobotState(STATE_FOLLOW_RED);
      }
      break;

    case STATE_TURN_RIGHT_INTERSECTION:
      moveRight();
      if (millis() - stateStartMs >= INTERSECTION_RIGHT_MS) {
        setRobotState(STATE_FOLLOW_RED);
      }
      break;

    case STATE_END:
      stopMotors();
      break;
  }
}