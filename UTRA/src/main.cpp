
#include <Arduino.h>

// New Mapping based on your move
const int IN1 = 9;  
const int IN2 = 10; 
const int IN3 = 6;  
const int IN4 = 5;  

void setup() {
  pinMode(IN1, OUTPUT);
  pinMode(IN2, OUTPUT);
  pinMode(IN3, OUTPUT);
  pinMode(IN4, OUTPUT);
  Serial.begin(9600);
}

void stopAll() {
  digitalWrite(IN1, LOW); digitalWrite(IN2, LOW);
  digitalWrite(IN3, LOW); digitalWrite(IN4, LOW);
}

void loop() {
  // TEST 1: ONLY LEFT (Motor A)
  Serial.println("Testing LEFT only...");
  digitalWrite(IN1, HIGH); 
  delay(2000);
  stopAll();
  delay(1000);

  // TEST 2: ONLY RIGHT (Motor B)
  Serial.println("Testing RIGHT only...");
  digitalWrite(IN3, HIGH); 
  delay(2000);
  stopAll();
  delay(1000);
}