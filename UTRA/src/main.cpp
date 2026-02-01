#include <Arduino.h>

void initChallengeOne();
void challengeOne();
bool isChallengeOneComplete();
void initChallengeTwo();
void challengeTwo();

bool runningPartTwo = false;

void setup() {
  Serial.begin(9600);
  Serial.println("UTRA: Challenge One starting...");
  initChallengeOne();
}

void loop() {
  if (!runningPartTwo) {
    challengeOne();
    if (isChallengeOneComplete()) {
      runningPartTwo = true;
      Serial.println("UTRA: Challenge One Part Two starting...");
      initChallengeTwo();
    }
  } else {
    challengeTwo();
  }
}
