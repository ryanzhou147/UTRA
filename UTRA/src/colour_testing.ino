// Simple colour sensor test - moves forward when red is detected

// dc motor
const int LEFT1 = 9;
const int LEFT2 = 10;
const int RIGHT1 = 11;
const int RIGHT2 = 12;
const int ENA = 7;
const int ENB = 8;

// colour sensor
const int S0 = 2;
const int S1 = 3;
const int S2 = 4;
const int S3 = 5;
const int S_OUT = 6;

const int blackThreshold = 100;
const int whiteThreshold = 40;

void setup() {
  // dc motor
  pinMode(LEFT1, OUTPUT);
  pinMode(LEFT2, OUTPUT);
  pinMode(RIGHT1, OUTPUT);
  pinMode(RIGHT2, OUTPUT);
  pinMode(ENA, OUTPUT);
  pinMode(ENB, OUTPUT);
  digitalWrite(ENA, HIGH);
  digitalWrite(ENB, HIGH);

  // colour sensor
  pinMode(S0, OUTPUT);
  pinMode(S1, OUTPUT);
  pinMode(S2, OUTPUT);
  pinMode(S3, OUTPUT);
  pinMode(S_OUT, INPUT);

  // set PW scaling to 20%
  digitalWrite(S0, HIGH);
  digitalWrite(S1, LOW);

  Serial.begin(9600);
}

int getRedPW() {
  digitalWrite(S2, LOW);
  digitalWrite(S3, LOW);
  delay(10);
  return pulseIn(S_OUT, LOW, 10000);
}

void moveForward() {
  digitalWrite(LEFT1, HIGH);
  digitalWrite(LEFT2, LOW);
  digitalWrite(RIGHT1, HIGH);
  digitalWrite(RIGHT2, LOW);
}

void stopMotors() {
  digitalWrite(LEFT1, LOW);
  digitalWrite(LEFT2, LOW);
  digitalWrite(RIGHT1, LOW);
  digitalWrite(RIGHT2, LOW);
}

void loop() {
  int redPW = getRedPW();
  
  Serial.print("Red PW: ");
  Serial.print(redPW);

  // red is between white (<40) and black (>100)
  if (redPW >= whiteThreshold && redPW <= blackThreshold) {
    Serial.println(" - RED detected, moving forward");
    moveForward();
  } else {
    Serial.println(" - Not red, stopped");
    stopMotors();
  }

  delay(100);
}
