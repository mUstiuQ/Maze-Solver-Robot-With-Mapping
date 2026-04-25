# 🤖 GestureBot — Wireless Hand-Gesture Controlled Robot

A 4WD wireless rover controlled through a gesture-based wearable glove, featuring proportional speed control, differential steering, ultrasonic proximity feedback, pivot turning, and gesture-triggered dance mode.

> **Control with your hand. Impress with a dance.**

---

## 📋 Table of Contents

- [Features](#-features)
- [Demo](#-demo)
- [System Architecture](#-system-architecture)
- [Hardware](#-hardware)
- [Wiring](#-wiring)
- [Software](#-software)
- [Installation](#-installation)
- [Configuration](#-configuration)
- [How It Works](#-how-it-works)
- [Dance Mode](#-dance-mode)
- [Troubleshooting](#-troubleshooting)
- [Project Structure](#-project-structure)
- [License](#-license)

---

## ✨ Features

| Feature | Description |
|---------|-------------|
| **Proportional Gesture Control** | MPU6050 reads hand tilt angle and maps it to PWM 85–225. Smooth acceleration, not binary ON/OFF |
| **Differential Steering** | Blends throttle + steering for car-like turning. Inner wheel slows proportionally |
| **Pivot Turning** | Zero-radius rotation — wheels spin in opposite directions when steering without throttle |
| **Proximity Buzzer** | HC-SR04 ultrasonic sensor with progressive beep rate. Closer = faster beeping (parking sensor style) |
| **Radio Failsafe** | Motors stop automatically within 1 second if the glove signal is lost |
| **Dance Mode** | Shake the glove to trigger a choreographed sequence: 360° spin → figure-8 patterns → 360° spin → beep finale |
| **Exponential Smoothing** | Low-pass filter (k=0.10) on gyro data eliminates hand tremor while maintaining responsiveness |
| **Real PWM Speed Control** | Uses L298N ENA/ENB pins with `analogWrite` for true analog motor speed — not just digital HIGH/LOW |

---

## 🎬 Demo

> *Insert demo video or GIF here*

---

## 🏗 System Architecture

```
┌─────────────────────┐         NRF24L01          ┌─────────────────────────┐
│     TRANSMITTER     │     250 kbps wireless      │        RECEIVER         │
│      (Glove)        │ ─────────────────────────► │        (Robot)          │
│                     │                            │                         │
│  Arduino Nano       │    Packet Structure:       │  Arduino Mega 2560      │
│  MPU6050 Gyro/Accel │    {                       │  L298N Motor Driver     │
│  NRF24L01 Radio     │      accel:  0-225         │  4x DC Motors (4WD)    │
│  9V Battery         │      brake:  0-225         │  HC-SR04 Ultrasonic    │
│                     │      steer: -135..+135     │  Buzzer                │
│                     │      dance:  true/false    │  3x 18650 Batteries    │
└─────────────────────┘    }                       └─────────────────────────┘
```

---

## 🔧 Hardware

### Bill of Materials

#### Transmitter (Glove)

| Component | Qty | Description |
|-----------|-----|-------------|
| Arduino Nano | 1 | MCU for gesture detection |
| MPU6050 | 1 | 6-axis gyroscope & accelerometer |
| NRF24L01 | 1 | 2.4GHz wireless transceiver |
| 470µF Capacitor | 1 | NRF24L01 power stabilization |
| 9V Battery | 1 | Power source |

#### Receiver (Robot)

| Component | Qty | Description |
|-----------|-----|-------------|
| Arduino Mega 2560 | 1 | Central controller |
| L298N Motor Driver | 1 | Dual H-bridge for motor control |
| DC Motors | 4 | 4WD propulsion |
| NRF24L01 | 1 | 2.4GHz wireless transceiver |
| HC-SR04 | 1 | Ultrasonic distance sensor |
| Active Buzzer | 1 | Audio feedback |
| 470µF Capacitor | 1 | NRF24L01 power stabilization |
| 3x 18650 Batteries | 1 pack | High-current power |

**Total cost: < €50** — all components off-the-shelf and fully reproducible.

---

## 🔌 Wiring

### Receiver — Arduino Mega

| Component | Pin Name | Arduino Pin | Notes |
|-----------|----------|-------------|-------|
| L298N | IN1 | D5 | Left motors direction A |
| L298N | IN2 | D4 | Left motors direction B |
| L298N | IN3 | D3 | Right motors direction A |
| L298N | IN4 | D2 | Right motors direction B |
| L298N | ENA | D9 | Left motors PWM speed (**remove jumper!**) |
| L298N | ENB | D10 | Right motors PWM speed (**remove jumper!**) |
| NRF24L01 | CE / CSN | D7 / D11 | Radio control |
| NRF24L01 | SCK / MOSI / MISO | 52 / 51 / 50 | Hardware SPI |
| NRF24L01 | VCC / GND | 3.3V / GND | Add 470µF capacitor! |
| HC-SR04 | Trig / Echo | A3 / A4 | Distance reading |
| Buzzer | VCC | D6 | Audio alerts |

### Transmitter — Arduino Nano

| Component | Pin Name | Arduino Pin |
|-----------|----------|-------------|
| NRF24L01 | CE / CSN | D7 / D8 |
| NRF24L01 | SCK / MOSI / MISO | D13 / D11 / D12 |
| MPU6050 | SDA / SCL | A4 / A5 |

---

## 💻 Software

### Dependencies

Install via Arduino IDE Library Manager:

- **RF24** by TMRh20
- **MPU6050_tockn**
- Servo (built-in)
- SPI (built-in)
- Wire (built-in)

---

## 🚀 Installation

1. **Assemble the hardware** using the wiring tables above
2. **Install all required libraries** in Arduino IDE
3. **Upload the code:**
   - `Transmitter/Transmitter.ino` → Arduino Nano (Glove)
   - `Receiver/Receiver.ino` → Arduino Mega (Robot)
4. **Calibrate:** Place glove on a flat surface → press reset on Nano → wait ~3 seconds for gyro offset calibration
5. **Drive:**
   - Tilt forward → accelerate
   - Tilt backward → reverse
   - Tilt left/right → steer
   - Steer without throttle → pivot turn (spin in place)
   - Shake hand → dance mode

---

## ⚙ Configuration

### Gesture Sensitivity (Transmitter)

```cpp
int deadzone = 22;       // Degrees of tilt before response starts
int maxAngle = 45;       // Degrees at full speed/steer
int minPWM = 85;         // Minimum PWM to overcome motor inertia
int maxPWM = 225;        // Maximum motor speed
int steerPower = 135;    // Maximum steering intensity
float FILTER_K = 0.10;   // Smoothing factor (lower = smoother, higher = faster response)
```

### Proximity Buzzer (Receiver)

```cpp
// Buzzer starts beeping below this distance (cm)
// Beep interval: 50ms at 5cm, 500ms at threshold
if (dist > 0 && dist < 20) { ... }
```

### Motor Direction Fix

If motors spin the wrong way, swap the `HIGH`/`LOW` logic in the `motors()` function for the affected side instead of rewiring.

---

## 🧠 How It Works

1. **Glove** reads hand tilt angles from MPU6050 (pitch = speed, roll = steering)
2. Raw angles pass through an **exponential smoothing filter** (k=0.10) to eliminate tremor
3. Filtered angles are mapped through **deadzones** (22°) into PWM values (85–225)
4. A **data packet** `{accel, brake, steer, dance}` is transmitted via NRF24L01 at 250kbps
5. **Robot** receives the packet and computes differential motor speeds
6. **L298N** driver applies direction via IN1–IN4 and speed via **analogWrite on ENA/ENB**
7. **HC-SR04** continuously measures distance — buzzer beeps progressively when objects are near
8. If **no radio packet** is received for 3 seconds → motors shut down automatically (failsafe)

---

## 💃 Dance Mode

Triggered by **shaking the glove** (accelerometer detects > 2.5G total acceleration).

### Choreography Sequence (~6.2 seconds)

| Time | Action |
|------|--------|
| 0.0s – 0.8s | 360° spin right |
| 0.8s – 1.0s | Pause |
| 1.0s – 2.6s | Figure-8 pattern #1 |
| 2.6s – 2.8s | Pause |
| 2.8s – 4.4s | Figure-8 pattern #2 |
| 4.4s – 4.6s | Pause |
| 4.6s – 5.4s | 360° spin left |
| 5.4s – 6.2s | Double beep finale |

- Uses **edge detection** — dance triggers once per shake, doesn't loop
- Shake again after completion to retrigger
- All timing values and PWM speeds are tunable for different surfaces

---

## 🔍 Troubleshooting

| Problem | Solution |
|---------|----------|
| Robot stutters or resets | Add 470µF capacitor to NRF24L01 VCC/GND |
| Motors spin wrong direction | Swap HIGH/LOW in `motors()` function — don't rewire |
| Robot doesn't move at low tilt | Increase `minPWM` value |
| Constant buzzer beeping | Object detected within 20cm, or sensor reading noise — check sensor mounting |
| No response from robot | Check that both TX and RX use the same radio address and data rate |
| Dance mode won't stop | Already fixed with edge detection — update to latest code |
| Robot stops after a few seconds | Transmitter not powered on — failsafe activating. Power glove first |

---

## 📄 License

This project is open source. Feel free to use, modify, and share.
