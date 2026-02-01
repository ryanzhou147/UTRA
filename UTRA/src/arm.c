// servo
#include <Servo.h>
Servo servo;
const int SERVOPIN = 7;

// ultrasonic sensor
const int TRIGPIN = A1;
const int ECHOPIN = A2;
long duration;
int distance;

// dc motor
const int LEFT1 = 8;
const int LEFT2 = 9;
const int RIGHT1 = 10;
const int RIGHT2 = 11;

// colour sensor
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

void setup()
{
  // ?
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
}

typedef enum {
  ARM_INACTIVE = 0,
  ARM_ACTIVE1 = 1,
  ARM_ACTIVE2 = 2,
  ARM_FINISHED = 3
} ArmState;
ArmState armstate = ARM_INACTIVE;

int counter = 0;

void loop()
{
  while (1)
  {
    switch (armstate)
    {
    case ARM_INACTIVE:
      if (PATH_BLUE == getColour())
      {
        pickup();
        armstate = ARM_ACTIVE1;
      }
      break;
    case ARM_ACTIVE1:
      counter++;
      if (counter > 1000)
      {
        counter = 0;
        armstate = ARM_ACTIVE2;
      }
      break;
    case ARM_ACTIVE2:
      if (PATH_BLUE == getColour())
      {
        dropoff();
        armstate = ARM_FINISHED
      }
      break;
    case ARM_FINISHED:
      break;
    }
    delay(7);
  }
}

// #define TEST1

// experimentally change these
const int turn_pickup_delay = 1;
const int box_pickup_delay = 1;
void pickup()
{
#ifdef TEST1
  const int repos_pickup_delay = 1;
  moveBackward();
  delay(repos_pickup_delay);
#endif
  moveRight();
  delay(turn_pickup_delay);
  moveForward();
  delay(box_pickup_delay);
  stopMotors();
  servo.write(30);
  delay(1); // time for arm to stabilize, experimentally figure this out
  moveBackward();
  delay(box_pickup_delay);
  moveLeft();
  delay(turn_pickup_delay);
#ifdef TEST1
  moveForward();
  delay(repos_pickup_delay);
#endif
  stopMotors();
}

// experimentally change these
const int turn_dropoff_delay = 1;
const int box_dropoff_delay = 1;
void dropoff()
{
#ifdef TEST1
  const int repos_dropoff_delay = 1;
  moveBackward();
  delay(repos_dropoff_delay);
#endif
  moveLeft();
  delay(turn_dropoff_delay);
  moveForward();
  delay(box_dropoff_delay);
  stopMotors();
  servo.write(0);
  delay(1); // time for arm to stabilize, experimnetally figure this out
  moveBackward();
  delay(box_dropoff_delay);
  moveRight();
  delay(turn_dropff_delay);
#ifdef TEST1
  moveForward();
  delay(repos_dropoff_delay);
#endif
  stopMotors();
}

// black threshold: all colours over 100
// white threshold: all colours under 40
// white = 0, red = 1, green = 2, blue = 3, black = 4
PathColour getColour()
{
  redPW = getRedPW();
  greenPW = getGreenPW();
  bluePW = getBluePW();

  if (redPW > blackThreshold && greenPW > blackThreshold && bluePW > blackThreshold)
  {
    return PATH_BLACK;
  }

  if (redPW < whiteThreshold && greenPW < whiteThreshold && bluePW < whiteThreshold)
  {
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

void moveForward() {
  digitalWrite(LEFT1, HIGH);
  digitalWrite(LEFT2, LOW);
  digitalWrite(RIGHT1, HIGH);
  digitalWrite(RIGHT2, LOW);
}

void moveBackward() {
  digitalWrite(LEFT1, LOW);
  digitalWrite(LEFT2, HIGH);
  digitalWrite(RIGHT1, LOW);
  digitalWrite(RIGHT2, HIGH);
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
  
  duration = pulseIn(ECHOPIN, HIGH);
  if (duration == 0) return 99999;
  return duration * 340 / 200;
}
