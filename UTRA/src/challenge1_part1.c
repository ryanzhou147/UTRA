// servo
//#include <Servo.h>
//Servo servo;
//const int SERVOPIN = A3;

// ultrasonic sensor
const int TRIGPIN = A4;
const int ECHOPIN = A5;
long duration;
int distance;

// dc motor
const int LEFT1 = 10;
const int LEFT2 = 9;
const int RIGHT1 = 12;
const int RIGHT2 = 11;

// colour sensor
const int S0 = 2;
const int S1 = 3;
const int S2 = 4;
const int S3 = 5;
const int S_OUT = 6;
const int blackThreshold = 80;
const int whiteThreshold = 10;
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
  // servo.attach(SERVOPIN);
  // servo.write(0);

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
  delay(2000);
}

typedef enum
{
  STATE_ROBOT_START = 0,
  STATE_CHECK_RIGHT = 1,
  STATE_CHECK_LEFT = 2,
  STATE_FOLLOW_GREEN = 3,
  STATE_FOLLOW_RAMP = 4,
  STATE_BLOCK_PICKUP = 5,
  STATE_BLOCK_DROPOFF = 6,
  STATE_FINISHED = 7
} RobotState;
RobotState robotstate = STATE_ROBOT_START;

int rightcounter = 0;
int second = 0;

void loop()
{
  PathColour colour = getColour();
  int cm = getDistance();

  switch (robotstate)
  {
  case STATE_ROBOT_START:
    if (colour == PATH_BLUE || colour == PATH_GREEN || colour == PATH_BLACK)
    {
      moveForward();
      delay(150);
      stopMotors();
      if (second == 1)
      {
        delay(10);
      }
      else
      {
        delay(150);
      }
    }
    else if (colour == PATH_WHITE)
    {
      second = 1;
      robotstate = STATE_CHECK_RIGHT;
    }
    else if (colour == PATH_BLUE)
    {
      // add flag for pickup dropoff
      robotstate = STATE_BLOCK_PICKUP;
    }
    break;

  case STATE_CHECK_RIGHT:
    // rotate up to 90 degrees right
    moveRight();
    delay(120);
    stopMotors();
    delay(150);

    if (colour == PATH_GREEN)
    {
      robotstate = STATE_ROBOT_START;
    }

    rightcounter++;
    if (rightcounter >= 25)
    {
      rightcounter = 0;
      robotstate = STATE_CHECK_LEFT;
    }
    break;

  case STATE_CHECK_LEFT:
    moveLeft();
    delay(120);
    stopMotors();
    delay(150);

    if (colour == PATH_GREEN)
    {
      robotstate = STATE_ROBOT_START;
    }
    break;
  }
}

// black threshold: all colours over 100
// white threshold: all colours under 40
// white = 0, red = 1, green = 2, blue = 3, black = 4
PathColour getColour()
{
  redPW = getRedPW();
  greenPW = getGreenPW();
  bluePW = getBluePW();
  /*
  Serial.print(redPW);
  Serial.print(" ");
  Serial.print(greenPW);
  Serial.print(" ");
  Serial.print(bluePW);
  Serial.print(" ");*/

  if (redPW > blackThreshold && greenPW > blackThreshold && bluePW > blackThreshold)
  {
    return PATH_BLACK;
  }

  if (redPW < whiteThreshold && greenPW < whiteThreshold && bluePW < whiteThreshold)
  {
    return PATH_WHITE;
  }

  int maxColor = min(redPW, min(greenPW, bluePW));
  if (maxColor == redPW)
  {
    return PATH_RED;
  } 
  else if (maxColor == greenPW)
  {
    return PATH_GREEN;
  } 
  else
  {
    return PATH_BLUE;
  }
}

int getRedPW()
{
  digitalWrite(S2, LOW);
  digitalWrite(S3, LOW);
  delay(10);
  int PW = pulseIn(S_OUT, LOW);
  return PW;
}

int getGreenPW()
{
  digitalWrite(S2, HIGH);
  digitalWrite(S3, HIGH);
  delay(10);
  int PW = pulseIn(S_OUT, LOW);
  return PW;
}

int getBluePW()
{
  digitalWrite(S2, LOW);
  digitalWrite(S3, HIGH);
  delay(10);
  int PW = pulseIn(S_OUT, LOW);
  return PW;
}

void moveForward()
{
  //Serial.println("fwd");
  digitalWrite(LEFT1, HIGH);
  digitalWrite(LEFT2, LOW);
  digitalWrite(RIGHT1, HIGH);
  digitalWrite(RIGHT2, LOW);
}

void moveBackward()
{
  digitalWrite(LEFT1, LOW);
  digitalWrite(LEFT2, HIGH);
  digitalWrite(RIGHT1, LOW);
  digitalWrite(RIGHT2, HIGH);
}

void moveLeft()
{
  digitalWrite(RIGHT1, HIGH);
  digitalWrite(RIGHT2, LOW);
  digitalWrite(LEFT1, LOW);
  digitalWrite(LEFT2, HIGH);
}

void moveRight()
{
  digitalWrite(LEFT1, HIGH);
  digitalWrite(LEFT2, LOW);
  digitalWrite(RIGHT1, LOW);
  digitalWrite(RIGHT2, HIGH);
}

void stopMotors()
{
  digitalWrite(LEFT1, LOW);
  digitalWrite(LEFT2, LOW);
  digitalWrite(RIGHT1, LOW);
  digitalWrite(RIGHT2, LOW);
}

int getDistance()
{
  digitalWrite(TRIGPIN, LOW);
  delayMicroseconds(2);
  digitalWrite(TRIGPIN, HIGH);
  delayMicroseconds(10);
  digitalWrite(TRIGPIN, LOW);
  
  duration = pulseIn(ECHOPIN, HIGH);
  if (duration == 0) return 99999;
  return duration * 340 / 200;
}
