#include <Arduino.h>
#include <OneWire.h>
#include <DallasTemperature.h>

#define ONE_WIRE_BUS 4
#define BUTTON_PIN 12
#define LED_PIN 3
#define DOOR_OPEN_PIN 2
#define DOOR_CLOSE_PIN 15

void evaluateLed();
void setBlinkTimers();
void evaluateTemp();
void evaluateDoorMotors();

// Setup a oneWire instance to communicate with any OneWire devices
OneWire oneWire(ONE_WIRE_BUS);

unsigned long blinkOffMillis = 0;
unsigned long blinkOnMillis = 0;
unsigned long evaluateTempMillis = 0;

// Pass our oneWire reference to Dallas Temperature sensor
DallasTemperature sensors(&oneWire);

void setup()
{
  Serial.begin(115200);
  sensors.begin();

  pinMode(BUTTON_PIN, INPUT_PULLUP);

  pinMode(LED_PIN, OUTPUT);
  pinMode(DOOR_OPEN_PIN, OUTPUT);
  pinMode(DOOR_CLOSE_PIN, OUTPUT);

  digitalWrite(LED_PIN, LOW);
  digitalWrite(DOOR_CLOSE_PIN, LOW);
  digitalWrite(DOOR_OPEN_PIN, LOW);
}

void loop()
{
  evaluateLed();
  evaluateTemp();
  evaluateDoorMotors();
  delay(5);
}

void evaluateLed()
{
  if (digitalRead(BUTTON_PIN) == LOW)
  {
    setBlinkTimers();
  }

  if (blinkOnMillis < millis())
  {
    digitalWrite(LED_PIN, HIGH);
    blinkOnMillis = 0 - 1;
  }
  if (blinkOffMillis < millis())
  {
    digitalWrite(LED_PIN, LOW);
    blinkOffMillis = 0 - 1;
  }
}

void setBlinkTimers()
{
  blinkOnMillis = millis() + 1000;
  blinkOffMillis = millis() + 2000;
}

void evaluateTemp()
{
  if (evaluateTempMillis < millis())
  {
    sensors.requestTemperatures();
    float temperatureF = sensors.getTempFByIndex(0);
    Serial.print(temperatureF);
    Serial.println("F");
    evaluateTempMillis = millis() + 1000;
  }
}

void evaluateDoorMotors()
{
  // digitalWrite(DOOR_OPEN_PIN, HIGH);
  // delay(2000);
  // digitalWrite(DOOR_OPEN_PIN, LOW);
  // delay(1000);
  // digitalWrite(DOOR_CLOSE_PIN, HIGH);
  // delay(2000);
  // digitalWrite(DOOR_CLOSE_PIN, LOW);
  // delay(1000);
}