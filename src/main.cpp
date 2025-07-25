#include <Arduino.h>
#include <OneWire.h>
#include <DallasTemperature.h>
#include <cmath>

#define ONE_WIRE_BUS 4
#define BUTTON_PIN 23
#define LED_PIN 3
#define DOOR_OPEN_PIN 15
#define DOOR_CLOSE_PIN 2
#define DOOR_OPEN_TEMP 90
#define DOOR_CLOSE_TEMP 80

void evaluateLed();
void evaluateTemp();
void evaluateDoorMotors();
void evaluateSleep();

// Setup a oneWire instance to communicate with any OneWire devices
OneWire oneWire(ONE_WIRE_BUS);

uint8_t tempBlinkMode = 0; // 0 = off, 1 = hundreds, 2 = tens, 3 = ones
uint8_t tempBlinkHundredsIdx = 0;
uint8_t tempBlinkTensIdx = 0;
uint8_t tempBlinkOnesIdx = 0;

unsigned long blinkOffMillis = ULONG_MAX;
unsigned long blinkOnMillis = ULONG_MAX;
unsigned long evaluateTempMillis = 0;
unsigned long evaluateMotorMillis = 0;
unsigned long motorOffMillis = ULONG_MAX;
unsigned long buttonDebounceMillis = 0;
unsigned long sleepAtMillis = ULONG_MAX;

uint8_t nextMotorDirection = DOOR_OPEN_PIN;

// Pass our oneWire reference to Dallas Temperature sensor
DallasTemperature sensors(&oneWire);

void setup()
{
  Serial.begin(115200);
  Serial.println("Starting up...");

  esp_sleep_enable_timer_wakeup(180 * 1000 * 1000); // sleep for 180 seconds
  sleepAtMillis = millis() + (10 * 1000);           // start sleeping 10 seconds from now

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
  evaluateSleep();
}

void evaluateLed()
{
  bool tempBlinkNext = false;
  if (digitalRead(BUTTON_PIN) == LOW)
  {
    tempBlinkMode = 1;
    tempBlinkHundredsIdx = 0;
    tempBlinkTensIdx = 0;
    tempBlinkOnesIdx = 0;
    tempBlinkNext = true;
  }

  if (blinkOnMillis < millis())
  {
    digitalWrite(LED_PIN, HIGH);
    blinkOnMillis = ULONG_MAX;
  }
  if (blinkOffMillis < millis())
  {
    digitalWrite(LED_PIN, LOW);
    blinkOffMillis = ULONG_MAX;
    tempBlinkNext = tempBlinkMode != 0;
  }

  if (tempBlinkNext)
  {
    if (tempBlinkMode == 1)
    {
      uint8_t hundreds = ((int)sensors.getTempFByIndex(0)) / 100;
      if (tempBlinkHundredsIdx < hundreds)
      {
        blinkOnMillis = millis() + 200;
        blinkOffMillis = millis() + 350;
        tempBlinkHundredsIdx++;
      }
      else
      {
        tempBlinkMode = 2;
        blinkOffMillis = millis() + 700;
      }
    }
    else if (tempBlinkMode == 2)
    {
      uint8_t tens = (((int)sensors.getTempFByIndex(0)) % 100) / 10;
      if (tempBlinkTensIdx < tens)
      {
        blinkOnMillis = millis() + 200;
        blinkOffMillis = millis() + 350;
        tempBlinkTensIdx++;
      }
      else
      {
        tempBlinkMode = 3;
        blinkOffMillis = millis() + 700;
      }
    }
    else if (tempBlinkMode == 3)
    {
      uint8_t ones = ((int)sensors.getTempFByIndex(0)) % 10;
      if (tempBlinkOnesIdx < ones)
      {
        blinkOnMillis = millis() + 200;
        blinkOffMillis = millis() + 350;
        tempBlinkOnesIdx++;
      }
      else
      {
        tempBlinkMode = 0;
      }
    }
  }
}

void evaluateTemp()
{
  if (evaluateTempMillis < millis() && tempBlinkMode == 0)
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
  // Prevent button bounce glitches
  if (buttonDebounceMillis > millis())
  {
    return;
  }
  else
  {
    // Prevent glitches when millis() overflows
    buttonDebounceMillis = 0;
  }

  // Button is pressed.  Turn on the motor.
  if (digitalRead(BUTTON_PIN) == LOW)
  {
    digitalWrite(nextMotorDirection, HIGH);
    motorOffMillis = millis() + 100;
    buttonDebounceMillis = millis() + 50;
  }

  // Button is not pressed and it's time to turn off the motor
  if (digitalRead(BUTTON_PIN) == HIGH && motorOffMillis < millis())
  {
    // Turn the motor off and switch directions for next time
    digitalWrite(DOOR_OPEN_PIN, LOW);
    digitalWrite(DOOR_CLOSE_PIN, LOW);
    motorOffMillis = ULONG_MAX;
    nextMotorDirection = (nextMotorDirection == DOOR_OPEN_PIN) ? DOOR_CLOSE_PIN : DOOR_OPEN_PIN;
  }

  if (evaluateMotorMillis < millis())
  {
    sensors.requestTemperatures();
    float temperatureF = sensors.getTempFByIndex(0);
    Serial.print(temperatureF);
    Serial.println("F");
    if (temperatureF > DOOR_OPEN_TEMP)
    {
      Serial.println("Opening door for 5 seconds");
      digitalWrite(DOOR_OPEN_PIN, HIGH);
      motorOffMillis = millis() + 5000;
      sleepAtMillis += 8000;
    }
    if (temperatureF < DOOR_CLOSE_TEMP)
    {
      Serial.println("Closing door for 5 seconds");
      digitalWrite(DOOR_CLOSE_PIN, HIGH);
      motorOffMillis = millis() + 5000;
      sleepAtMillis += 8000;
    }
    evaluateMotorMillis = millis() + 60000;
  }
}

void evaluateSleep()
{
  if (sleepAtMillis < millis())
  {
    Serial.println("Going to sleep...");
    esp_deep_sleep_start();
  }
}