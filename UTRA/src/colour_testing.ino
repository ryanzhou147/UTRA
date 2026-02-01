// Simple colour sensor test - moves forward when red is detected

// dc motor
const int ENA = 8;
const int ENB = 13;
const int IN1 = 9;   // Left motor
const int IN2 = 10;
const int IN3 = 11;  // Right motor
const int IN4 = 12;

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
  pinMode(IN1, OUTPUT);
  pinMode(IN2, OUTPUT);
  pinMode(IN3, OUTPUT);
  pinMode(IN4, OUTPUT);
  pinMode(ENA, OUTPUT);
  pinMode(ENB, OUTPUT);
  digitalWrite(ENA, HIGH);  // Enable left motor
  digitalWrite(ENB, HIGH);  // Enable right motor

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
  digitalWrite(IN1, LOW);
  digitalWrite(IN2, HIGH);
  digitalWrite(IN3, LOW);
  digitalWrite(IN4, HIGH);
}

void stopMotors() {
  digitalWrite(IN1, LOW);
  digitalWrite(IN2, LOW);
  digitalWrite(IN3, LOW);
  digitalWrite(IN4, LOW);
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
