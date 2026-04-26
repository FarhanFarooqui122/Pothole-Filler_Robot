# Autonomous Pothole Detection & Repair Robot (PROTOTYPE) 🤖
![Arduino](https://img.shields.io/badge/Arduino-Functional-success?style=for-the-badge&logo=arduino)
![Category](https://img.shields.io/badge/Category-Robotics-blue?style=for-the-badge)
![License](https://img.shields.io/badge/License-MIT-yellow?style=for-the-badge)

---

An Arduino-based autonomous vehicle designed to detect road surface irregularities (potholes), calculate their center-point using a scanning ultrasonic array, and deploy filler material through a servo-controlled funnel system.

## 🚀 Overview (v2.0 Adaptive Update)

The robot operates using a **Finite State Machine (FSM)** to transition between scouting, precision scanning, and material dispensing. The v2.0 update introduces a "Center-Average" alignment strategy and a new **Hybrid Calibration System** to ensure the nozzle is positioned perfectly over the pothole, regardless of changing terrain.

### Key Features
* **🏎️ Dual-Rate Scanning:** Features a high-speed `filteredDistanceFast()` sweep (~6ms) during movement and a high-resolution 2° increment scan once a target is locked.
* **🧠 Adaptive Ground Calibration:** Automatically establishes a baseline on startup (15 readings) and maintains a **10-sample rolling average** to adjust to road changes in real-time.
* **🛡️ Confidence-Based Filtering:** Requires 3 consecutive depth detections to trigger a stop, effectively ignoring sensor "jitter" and small road debris.
* **🎯 Consecutive Streak Logic:** Identifies the longest unbroken cluster of depth detections during scans to find the true center, filtering out edge noise.
* **🧹 Smart Fill Logic:** Real-time depth monitoring ensures the funnel closes precisely when the surface is truly level (depth <= 0).

---

## 🎯 Main Purpose
Road maintenance is often dangerous, expensive, and slow. The **primary goal** of this project is to create a low-cost, scalable robotic solution that can:
1. **🔍 Monitor:** Automatically patrol streets to find potholes before they become major hazards.
2. **📏 Measure:** Use ultrasonic precision to map the exact center and depth of a hole.
3. **🛠️ Repair:** Deploy filler material precisely, ensuring the road surface is level and safe for vehicles.
4. **🛡️ Protect:** Reduce the need for human workers to stand in active traffic zones.

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
| **Buzzer** | 12 | Status & Calibration alerts |
| **Motors (L298N)** | 5, 8, 9, 6, 10, 11 | ENA, IN1, IN2, ENB, IN3, IN4 |

### Technical Parameters (v2.0) ⚡
* **Ground Calibration:** Startup (15 samples) + Rolling Average (10 samples)
* **Detection Confirmation:** 3 consecutive reads
* **Obstacle Safety Distance:** 15 cm
* **Scan Range:** 50° to 130°
* **Serial Baud Rate:** 115,200 (12x faster than v1.0)

---

## ⚙️ How It Works (WORKFLOW!)

The robot follows a precise 5-step logic loop to ensure high-quality road repairs:

1. **🔍 Scouting Phase:** The robot patrols in `MOVE_FAST_SCAN`. The nozzle servo performs a rapid sweep using optimized, low-latency sensing to keep the loop running fast.
2. **⚠️ Detection & Verification:** Once the sensor detects 3 consecutive depth changes, the robot brakes. Single spikes are ignored.
3. **🔬 Precision Verification:** The system enters `SLOW_FULL_SCAN`. It performs a high-resolution sweep to map the longest **consecutive streak** of depth detections, ensuring it isn't a false trigger.
4. **🎯 Center Alignment:** The robot calculates the center of the best streak. The nozzle servo then rotates to this specific **Center Angle** for perfect targeting.
5. **🏗️ Smart Repair:**
    * The **Funnel Gate** opens. Material is dropped while the sensor monitors level in real-time.
    * Once the hole is level (depth <= 0), the gate shuts.
    * **Safety Alert:** If a timeout is reached before leveling, the robot triggers an 8-beep alarm (Out of Material) and halts.

---

## 📊 Logic Overhaul (v1.0 vs v2.0)

| Feature | Old Logic (v1.0) | New Logic (v2.0) |
| :--- | :--- | :--- |
| **Ground Baseline** | Static (3cm) | **Adaptive (Startup + Rolling)** |
| **Sensing Speed** | 30ms+ blocking / loop | **~6ms blocking / loop** |
| **Detection Trigger** | Single reading | **3-sample confirmation** |
| **Pothole Targeting** | Total sum of angles | **Longest consecutive streak** |
| **Fill Completion** | 1cm tolerance | **0cm (Truly level)** |
| **Serial Speed** | 9600 Baud | **115200 Baud** |

---

## 🛠️ Tech Stack & Tools

| Layer | Detail |
|---|---|
| 💻 Language | `C++ / Arduino` |
| ⚡ Controller | `ATmega328P` (Arduino Uno) |
| ⚙️ Actuation | Dual `N20 DC Gear Motors` + `SG90 Servo Motors (180°)` |
| 📏 Sensing | `HC-SR04` Ultrasonic ToF with Circular Buffer Filtering |
| 🔋 Motor Driver | `L298N` Dual H-Bridge |
| 📟 Baud Rate | `115200` — optimized for real-time telemetry |

---

## 🗺️ Future Roadmap

* [ ] 🛰️ **GPS Logging** — Integrate a NEO-6M module to mark pothole coordinates on Google Maps
* [ ] 📸 **Computer Vision** — Upgrade to ESP32-Cam or Raspberry Pi for AI-based pothole classification
* [ ] 🔋 **Solar Integration** — Add on-board solar panels for extended 24/7 autonomous scouting
* [ ] 📱 **Mobile App** — Bluetooth-enabled dashboard to monitor filler material levels and battery health
* [ ] 🏗️ **Multi-Material Support** — Update funnel logic to handle different asphalt/gravel grain sizes

---

## 💻 Installation & Setup

1. **Clone the Repository:**
   ```bash
   git clone [https://github.com/FarhanFarooqui122/Pothole-Filler_Robot.git](https://github.com/FarhanFarooqui122/Pothole-Filler_Robot.git)
2. Open `autopatch.ino` in Arduino IDE
3. Select board: **Arduino Uno**
4. Select correct **COM port**
5. Click **Upload**

---
> Developed by **Farhan Farooqui** for autonomous infrastructure maintenance.
