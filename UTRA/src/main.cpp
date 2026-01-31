// Revised Mapping
const int IN1 = 9;  // Motor A - PWM
const int IN2 = 10; // Motor A - PWM
const int IN3 = 6;  // Motor B - PWM
const int IN4 = 5;  // Motor B - PWM

void setup() {
  pinMode(IN1, OUTPUT);
  pinMode(IN2, OUTPUT);
  pinMode(IN3, OUTPUT);
  pinMode(IN4, OUTPUT);
  
  Serial.begin(9600);
  // This will show up in your Bridge Server logs later!
  Serial.println("UTRA: Motors Initialized on PWM Pins"); 
}

// Simple test: Full speed forward
void loop() {
  // Motor A Forward
  analogWrite(IN1, 255); 
  digitalWrite(IN2, LOW);

  // Motor B Forward
  analogWrite(IN3, 255);
  digitalWrite(IN4, LOW);
  
  delay(2000);
  
  // Stop All
  digitalWrite(IN1, LOW);
  digitalWrite(IN3, LOW);
  delay(1000);
}