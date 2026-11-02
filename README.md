# SomNigil

### Adaptive Wearable Prototype for Sleep Distress Monitoring

SomNigil is an ESP32-based physiological monitoring prototype that combines PPG sensing and motion tracking to observe heart-rate trends and movement during rest.

> **Implementation status:** This repository contains the current embedded firmware prototype. The uploaded firmware implements heart-rate estimation, motion classification and OLED status display. SpO₂ computation, mobile-app notifications and clinically validated distress detection are not implemented in this firmware version.

## Hardware

| Component | Role |
|---|---|
| ESP32 | Main microcontroller |
| MAX30102 | PPG / infrared signal acquisition for heart-rate estimation |
| MPU6050 | 3-axis accelerometer and gyroscope; current firmware uses X/Y acceleration |
| SH1106 OLED | 128 × 64 status display, I²C address `0x3C` |

## Firmware Parameters

| Parameter | Configured value |
|---|---:|
| Serial baud rate | 115200 baud |
| MAX30102 sample rate | 400 samples/s |
| MAX30102 sample averaging | 4 samples |
| MAX30102 LED mode | 2 (red + IR) |
| MAX30102 pulse width | 411 µs |
| MAX30102 ADC range | 4096 |
| BPM moving-average window | 10 accepted beat intervals |
| OLED refresh interval | 500 ms |
| Accepted BPM range | 50–120 BPM (exclusive bounds in code) |

These are firmware configuration values, not claims of measured sensor accuracy or clinical performance.

## Current Firmware Behaviour

1. Initializes the ESP32 I²C interface, SH1106 OLED, MPU6050 and MAX30102.
2. Reads the MAX30102 infrared value and uses the SparkFun `checkForBeat()` routine to identify beat events.
3. Estimates instantaneous BPM from the time between detected beats:

   `BPM = 60,000 / inter-beat interval (ms)`

4. Maintains a 10-entry moving average of accepted BPM values.
5. Reads X/Y acceleration from the MPU6050 and classifies motion as `Calm` or `Restless` using a threshold of `0.10` on either axis.
6. Displays BPM, motion classification and system status on the OLED, refreshing every 500 ms.

## Status Logic in This Firmware

- `No Finger`: IR reading is below `50000`.
- `Alert`: averaged BPM is above `100`, or motion is classified as `Restless`.
- `Normal`: none of the above conditions apply.

This is a simple prototype rule set. It is not a medical alarm and must not be used to diagnose or rule out a medical condition.

## I²C Connections

The firmware uses the ESP32 default `Wire` I²C interface. With the common ESP32 Dev Module defaults:

| Signal | ESP32 GPIO |
|---|---:|
| SDA | 21 |
| SCL | 22 |

Connect the MAX30102, MPU6050 and SH1106 OLED to the shared I²C bus, observing each module's voltage and power requirements. Confirm the pin mapping for your specific ESP32 board.

## Required Arduino Libraries

Install these libraries through the Arduino IDE Library Manager:

- Adafruit GFX Library
- Adafruit SH110X
- MPU6050_light
- SparkFun MAX3010x Sensor Library (provides `MAX30105.h` and `heartRate.h`)

## Build and Run

1. Install the required libraries in Arduino IDE.
2. Select the ESP32 board matching your development module.
3. Connect the sensors and OLED to the I²C bus.
4. Open `src/SomNigil.ino`.
5. Select the correct serial port and upload the sketch.
6. Open Serial Monitor at `115200` baud.

## Repository Structure

```text
SomNigil/
├── README.md
├── src/
│   └── SomNigil.ino
└── docs/
    └── SomNigil-Project-Overview.pdf
```

## Current Limitations and Next Steps

- Add signal-quality checks and reject unreliable PPG intervals.
- Implement and validate SpO₂ estimation; the current sketch does not calculate SpO₂.
- Improve motion classification using all three accelerometer axes and calibrated baselines.
- Replace fixed alert thresholds with a validated personalized baseline algorithm.
- Add caregiver/mobile notifications if the communication hardware and application are implemented.
- Test sensor placement, sampling reliability, power consumption and long-duration operation.

## Disclaimer

SomNigil is an educational engineering prototype. It is not a certified medical device and does not provide medical diagnosis, emergency detection guarantees or clinical monitoring. Do not rely on it in place of professional medical care or emergency services.
