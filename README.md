# Pothole-Filler_Robot 🤖
An autonomous Arduino robot that detects potholes using ultrasonic sensors, calculates the center-point via a servo-scan, and fills them with material using a smart-fill dispensing system.

![Status](https://img.shields.io/badge/Status-Functional-success?style=for-the-badge&logo=arduino)
![Category](https://img.shields.io/badge/Category-Robotics-blue?style=for-the-badge)
![License](https://img.shields.io/badge/License-MIT-yellow?style=for-the-badge)
# Autonomous Pothole Detection & Repair Robot 🤖

An Arduino-based autonomous vehicle designed to detect road surface irregularities (potholes), calculate their center-point using a scanning ultrasonic array, and deploy filler material through a servo-controlled funnel system.

## 🚀 Overview

The robot operates using a **Finite State Machine (FSM)** to transition between scouting, precision scanning, and material dispensing. It utilizes a "Center-Average" alignment strategy to ensure the nozzle is positioned over the middle of the pothole before beginning the repair process.

### Key Features
* **🏎️ Dual-Rate Scanning:** Features a high-speed sweep during movement and a high-resolution 2° increment scan once a target is locked.
* **🧠 Smart Fill Logic:** Real-time depth monitoring ensures the funnel closes precisely when the surface is level ($<1\text{cm}$) or the safety timeout is reached.
* **🛡️ Obstacle Avoidance:** Equipped with a front-facing ultrasonic "eye" to prevent collisions during the scouting phase.
* **🧹 Noise Filtering:** Implements a software-level averaging filter to ignore road debris and sensor "jitter" for reliable detection.
---

## 🎯 Main Purpose
Road maintenance is often dangerous, expensive, and slow. The **primary goal** of this project is to create a low-cost, scalable robotic solution that can:
1.  **🔍 Monitor:** Automatically patrol streets to find potholes before they become major hazards.
2.  **📏 Measure:** Use ultrasonic precision to map the exact center and depth of a hole.
3.  **🛠️ Repair:** Deploy filler material precisely, ensuring the road surface is level and safe for vehicles.
4.  **🛡️ Protect:** Reduce the need for human workers to stand in active traffic zones.

---

## 🛠 Hardware Configuration

### Pin Mapping 🛠️
| Component | Pin | Description |
| :--- | :--- | :--- |
| **Nozzle Servo** | 2 | Pivots the sensor/nozzle assembly |
| **Funnel Servo** | 3 | Actuates the material release gate |
| **Pothole Trig** | 4 | Ultrasonic Trigger (Down-facing) |
| **Pothole Echo** | 7 | Ultrasonic Echo (Down-facing) |
| **Obstacle Trig** | 13 | Front-facing Trigger |
| **Obstacle Echo** | A0 | Front-facing Echo |
| **Buzzer** | 12 | Status alerts |
| **Motors (L298N)** | 5, 8, 9, 6, 10, 11 | ENA, IN1, IN2, ENB, IN3, IN4 |

### Technical Parameters ⚡
* **Baseline Ground Distance:** 6 cm (ground distance value may vary so change it accordingly!)
* **Detection Threshold:** 3 cm (depth) (detection threshold value may vary so change it accordingly!)
* **Obstacle Safety Distance:** 15 cm
* **Scan Range:** 50° to 105°

---

## ⚙️ How It Works (State Machine)

1.  **`MOVE_FAST_SCAN`**: The robot moves forward while the nozzle servo sweeps quickly. If the sensor detects a distance increase (a hole), it stops immediately.
2.  **`SLOW_FULL_SCAN`**: The robot performs a slow, thorough sweep across the pothole. It records every angle where a hole is detected.
3.  **`ALIGN`**: It calculates the **Average Angle** (center) of all detected hole points and points the nozzle directly at that spot.
4.  **`FILL`**: The funnel opens. The system monitors the depth in real-time. If the hole is filled to within 1cm of the ground level, or 6 seconds pass, the funnel closes.

---

## 💻 Installation

1.  **Clone the Repository:**
    ```bash
    git clone [https://github.com/FarhanDarooqui122/Pothole-Filler_Robot.git](https://github.com/FarhanFarooqui122/Pothole-Filler_Robot.git)
    ```
2.  **Hardware Setup:** Wire your Arduino according to the pin mapping table above.
3.  **Calibration:** * Change `GROUND_DISTANCE_CM` if your sensor is mounted higher than 6cm.
    * Adjust `MIN_POTHOLE_DEPTH_CM` to change sensitivity.
4.  **Upload:** Use the Arduino IDE to upload the code to your board.

---

## 📊 Logic Flow

The depth calculation logic follows this formula:
$$Depth = Distance_{measured} - Distance_{ground}$$

This allows the robot to ignore the road surface and only trigger when the distance significantly increases.

---

## 🛠️ Tech Stack & Tools

* **Language:** `C++ / Arduino Wire` 💻
* **Hardware:** `Atmel ATmega328P` (Arduino Uno) ⚡
* **Mechanical:** `Dual DC Gear Motors` + `Micro Servos` ⚙️
* **Sensing:** `HC-SR04 Ultrasonic Time-of-Flight` 📏
  
---

## 🗺️ Future Roadmap
- [ ] **🛰️ GPS Logging:** Mark pothole coordinates on Google Maps via a GSM/GPS module.
- [ ] **📸 Computer Vision:** Add an ESP32-Cam for AI-based crack detection.
- [ ] **🔋 Solar Integration:** Add solar panels for 24/7 autonomous scouting.
- [ ] **📱 Mobile App:** Bluetooth dashboard to monitor fill levels and battery life.

---
*Developed for autonomous infrastructure maintenance.*
