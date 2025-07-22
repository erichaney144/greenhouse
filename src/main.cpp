#include <Arduino.h>

void setup()
{
  Serial.begin(115200);
  Serial.println("Test");
  pinMode(2, OUTPUT);
  pinMode(15, OUTPUT);
  digitalWrite(2, LOW);
  digitalWrite(15, LOW);
}

void loop()
{
  Serial.println("Test");
  digitalWrite(2, HIGH);
  delay(2000);
  digitalWrite(2, LOW);
  delay(1000);
  digitalWrite(15, HIGH);
  delay(2000);
  digitalWrite(15, LOW);
  delay(1000);
}