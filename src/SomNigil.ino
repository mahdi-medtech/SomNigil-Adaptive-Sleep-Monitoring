#include <Wire.h>
#include <Adafruit_GFX.h>
#include <Adafruit_SH110X.h>
#include <MPU6050_light.h>

#include "MAX30105.h"
#include "heartRate.h"

// OLED
Adafruit_SH1106G display = Adafruit_SH1106G(128, 64, &Wire);

// MPU6050
MPU6050 mpu(Wire);

// MAX30102
MAX30105 particleSensor;

// Heart Rate Variables
const byte RATE_SIZE = 10;
byte rates[RATE_SIZE];
byte rateSpot = 0;

long lastBeat = 0;

float beatsPerMinute = 0;
int beatAvg = 0;

void setup()
{
  Serial.begin(115200);
  Wire.begin();

  // Initialize BPM averaging array
  for (int i = 0; i < RATE_SIZE; i++)
  {
    rates[i] = 75;
  }

  // OLED
  display.begin(0x3C, true);
  display.clearDisplay();
  display.display();

  // MPU6050
  byte status = mpu.begin();

  while (status != 0)
  {
    Serial.println("MPU6050 Error");
    delay(1000);
  }

  // MAX30102
  if (!particleSensor.begin(Wire, I2C_SPEED_STANDARD))
  {
    Serial.println("MAX30102 not found");
    while (1);
  }

  particleSensor.setup(
  60,   // LED brightness
  4,    // sample average
  2,    // LED mode
  400,  // sample rate
  411,  // pulse width
  4096  // ADC range
  );

  particleSensor.setPulseAmplitudeRed(0x1F);
  particleSensor.setPulseAmplitudeGreen(0);

  Serial.println("System Ready");
}

unsigned long lastDisplayUpdate = 0;

void loop()
{
  // ===== MAX30102 ALWAYS RUNNING =====

  long irValue = particleSensor.getIR();

  if (checkForBeat(irValue))
  {
    if (lastBeat == 0)
    {
      lastBeat = millis();
    }
    else
    {
      long delta = millis() - lastBeat;
      lastBeat = millis();

      beatsPerMinute = 60.0 / (delta / 1000.0);

      if (beatsPerMinute > 50 && beatsPerMinute < 120)
      {
        rates[rateSpot++] = (byte)beatsPerMinute;
        rateSpot %= RATE_SIZE;

        beatAvg = 0;

        for (byte i = 0; i < RATE_SIZE; i++)
        {
          beatAvg += rates[i];
        }

        beatAvg /= RATE_SIZE;

        Serial.print("BPM = ");
        Serial.println(beatsPerMinute);

        Serial.print("AVG BPM = ");
        Serial.println(beatAvg);
      }
    }
  }

  // ===== MPU6050 =====

  mpu.update();

  float ax = mpu.getAccX();
  float ay = mpu.getAccY();

  String motionStatus = "Calm";

  if (abs(ax) > 0.10 || abs(ay) > 0.10)
  {
    motionStatus = "Restless";
  }

  // ===== STATUS =====

  String systemStatus = "Normal";

  if (irValue < 50000)
  {
    systemStatus = "No Finger";
  }
  else if (beatAvg > 100)
  {
    systemStatus = "Alert";
  }
  else if (motionStatus == "Restless")
  {
    systemStatus = "Alert";
  }

  // ===== OLED UPDATE ONLY EVERY 500ms =====

  if (millis() - lastDisplayUpdate > 500)
  {
    lastDisplayUpdate = millis();

    display.clearDisplay();

    display.setTextSize(1);
    display.setTextColor(SH110X_WHITE);

    display.setCursor(0, 0);
    display.print("BPM: ");
    display.println(beatAvg);

    display.setCursor(0, 20);
    display.print("Motion: ");
    display.println(motionStatus);

    display.setCursor(0, 40);
    display.print("Status: ");
    display.println(systemStatus);

    display.display();
  }
}