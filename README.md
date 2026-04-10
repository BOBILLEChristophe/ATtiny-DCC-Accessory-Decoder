# ATtiny-DCC-Accessory-Decoder
A compact DCC accessory decoder for ATtiny85 (45-25) driving servos for signals and turnouts.

# 🚂 ATtiny-DCC-Accessory-Decoder

**A compact DCC accessory decoder for ATtiny85 driving servos (signals, turnouts).**

---

## 📖 Overview

This project implements a **DCC accessory decoder** based on an **ATtiny85**, capable of controlling a **servo motor** for applications such as:

* railway signals (mechanical semaphore, light signals)
* turnouts (points)
* small actuators requiring two positions

The decoder listens to **NMRA DCC accessory commands** and converts them into smooth servo movements, while also driving status LEDs.

---

## ✨ Features

* ✅ Compatible with **NMRA DCC accessory protocol**
* ✅ Works with **Rocrail**, **JMRI**, **iTrain**, etc.
* ✅ Uses a low-cost **ATtiny85**
* ✅ Smooth servo movement (adjustable speed)
* ✅ Two-state control (OPEN / CLOSED)
* ✅ LED indication (RED / GREEN)
* ✅ Lightweight and efficient (no blocking delays)
* ✅ Based on a **state machine**

---

## 🧠 How it works

The decoder listens to DCC packets using the `DCC_Decoder` library.

When an accessory command is received:

1. The DCC address is reconstructed according to NMRA rules
2. The address is compared to the configured decoder address
3. The output state is extracted (ON/OFF)
4. The servo moves smoothly to the target position
5. LEDs are updated accordingly

---

## ⚙️ Configuration

### 🔧 Main parameters

```cpp
constexpr uint16_t DCC_ADDRESS = 150;

constexpr uint16_t ANGLE_OPEN = 800;
constexpr uint16_t ANGLE_CLOSED = 1800;

constexpr uint8_t SERVO_SPEED = 5;
```

| Parameter      | Description                                  |
| -------------- | -------------------------------------------- |
| `DCC_ADDRESS`  | Accessory address                            |
| `ANGLE_OPEN`   | Servo position (microseconds) for open state |
| `ANGLE_CLOSED` | Servo position for closed state              |
| `SERVO_SPEED`  | Movement speed (lower = faster)              |

---

## 🔌 Hardware

### 🧩 Required components

* ATtiny85
* Servo motor (SG90 or equivalent)
* 2 LEDs (red + green)
* Resistors for LEDs
* DCC signal input (track or booster output)

---

### 🔗 Pinout

| ATtiny85 Pin | Function              |
| ------------ | --------------------- |
| PB0          | Servo signal          |
| PB2          | DCC input (interrupt) |
| PB3          | Red LED               |
| PB4          | Green LED             |

---

## 🚦 Behavior

| DCC Command | Servo  | LED      |
| ----------- | ------ | -------- |
| OFF (0)     | OPEN   | Green ON |
| ON (1)      | CLOSED | Red ON   |

---

## 🖥️ Software dependencies

* Servo_ATTinyCore
* DCC_Decoder

---

## 🧪 Tested with

* Rocrail
* DCC command stations (via CAN / TCP gateway)
* Custom ESP32-based gateway

---

## 🔧 Rocrail configuration example

* Protocol: **NMRA-DCC**
* Address: **150**
* Accessory type: **Switch**

No specific port configuration required depending on the setup.

---

## 🏗️ Architecture

The firmware is based on a simple **state machine**:

```text
WAIT_COMMAND → APPLY_COMMAND → MOVING → HOLD
```

* **WAIT_COMMAND**: idle
* **APPLY_COMMAND**: update target position
* **MOVING**: smooth servo movement
* **HOLD**: detach servo to reduce power consumption

---

## ⚡ Optimization notes

* Non-blocking timing using `millis()`
* Servo detached after movement to reduce jitter and power usage
* Minimal RAM/flash footprint (ideal for ATtiny85)

---

