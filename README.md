# ♿ IoT-Enabled Smart Wheelchair (Secure Edge Computing)

<p align="center">
  <img src="https://img.shields.io/badge/C++-00599C?style=for-the-badge&logo=c%2B%2B&logoColor=white">
  <img src="https://img.shields.io/badge/Arduino-00979D?style=for-the-badge&logo=Arduino&logoColor=white">
  <img src="https://img.shields.io/badge/Network-Secure_Local_AP-red?style=for-the-badge">
  <img src="https://img.shields.io/badge/Saudi_Vision-2030-10a37f?style=for-the-badge">
</p>

An innovative IoT system designed for autonomous mobility and real-time health monitoring. This project serves as a **Secure Local Edge Node**, processing sensor data and providing a web-based dashboard without external internet dependency, ensuring privacy and high reliability for users.

---

## 🚀 Key Features

- 🧠 **Smart Navigation:** Real-time obstacle avoidance using Ultrasonic sensors and Servo scanning.
- 🛡️ **Secure Connectivity:** Standalone WPA2-encrypted Access Point (AP) for off-grid operations.
- 📊 **Live Telemetry:** Web-based dashboard for heart rate, GPS coordinates, and system status.
- 🆘 **Emergency SOS:** Instant alarm and GPS location sharing for rapid emergency response.
- ☀️ **Sustainable Energy:** Solar-powered battery management for extended range.

---

## 🎯 System Architecture

```text
Sensors (Perception) <-> ESP32 (Control) <-> Async Web Server (Application)
```

The system follows a modular architecture consisting of three main layers:
1. **Perception Layer:** Sensors (Ultrasonic, GPS, Pulse) gathering environmental and biological data.
2. **Control Layer:** ESP32 processing logic, managing the L298N Driver, and executing avoidance algorithms.
3. **Application Layer:** Async Web Server providing a real-time UI/Dashboard for monitoring and manual control.

---

## 📱 User Interface (Dashboard Showcase)

<div align="center">
  <table style="width: 100%; text-align: center;">
    <tr>
      <td style="padding: 10px; width: 50%;">
        <b>📱 Overview & Telemetry</b><br><br>
        <img src="https://github.com/user-attachments/assets/9ac3bf8c-598c-481c-9894-eee6ad8815d2" width="250"><br><br>
        <i>Real-time Dashboard</i>
      </td>
      <td style="padding: 10px; width: 50%;">
        <b>🪑 Chair Movement Control</b><br><br>
        <img src="https://github.com/user-attachments/assets/9ac3bf8c-598c-481c-9894-eee6ad8815d2" width="250"><br><br>
        <i>Manual & Auto Avoidance</i>
      </td>
    </tr>
    <tr>
      <td style="padding: 10px; width: 50%;">
        <b>🚨 SOS Emergency Alert</b><br><br>
        <img src="https://github.com/user-attachments/assets/9ac3bf8c-598c-481c-9894-eee6ad8815d2" width="250"><br><br>
        <i>Emergency Mode Activated</i>
      </td>
      <td style="padding: 10px; width: 50%;">
        <b>🗺️ GPS Location Tracking</b><br><br>
        <img src="https://github.com/user-attachments/assets/9ac3bf8c-598c-481c-9894-eee6ad8815d2" width="250"><br><br>
        <i>Live Map Navigation</i>
      </td>
    </tr>
  </table>
</div>

---

## 🧠 System Logic & Control Flow
The wheelchair employs a "Sense-Think-Act" cycle to ensure user safety:
- **Distance > 40cm:** Normal operation (Manual control via Dashboard).
- **Distance 25cm - 40cm:** Warning state (Buzzer alert + Speed reduction).
- **Distance < 25cm:** Critical state (**Auto-stop + Smart Avoidance Algorithm activated**).

---

## 🛠️ Engineering Challenges Overcome
- **Concurrency:** Managed sensor readings and WebServer requests simultaneously using non-blocking programming (avoiding `delay()`).
- **System Stability:** Implemented **Hardware Watchdog Timer (WDT)** to ensure the ESP32 recovers automatically from any software hangs.
- **Power Efficiency:** Optimized WiFi sleep modes to balance between dashboard responsiveness and battery longevity.

---

## 🚀 Getting Started

### Prerequisites
- **Hardware:** ESP32 Dev Board, L298N Driver, HC-SR04, NEO-6M GPS, Pulse Sensor.
- **Software:** Arduino IDE with ESP32 Board Support.
- **Libraries:** `ESPAsyncWebServer`, `ESP32Servo`, `TinyGPS++`.

### Installation & Setup

1. **Clone the Repo:**
```bash
git clone [https://github.com/MansourMutlaq/IoT-Smart-Wheelchair.git](https://github.com/MansourMutlaq/IoT-Smart-Wheelchair.git)
cd IoT-Smart-Wheelchair
```

2. **Flash the Code:**
```bash
# Upload 'Smart_Wheelchair_Core.ino' to your ESP32 using Arduino IDE
```

3. **Connect to WiFi:**
```text
SSID: IoT-Enabled Solar Wheelchair
Pass: Safe@Wheel2030
```

4. **Access Dashboard:**
```text
Open your browser and navigate to: [http://192.168.4.1](http://192.168.4.1)
```

---

## 👨‍💻 Development Team
Developed at **Qassim University** (College of Computer, IT Department) - **Grade: A+**

### 👥 Developers:
- **[Mansour Mutlaq Alharbi](https://www.linkedin.com/in/mansour-alharbi-129407350)** (Lead Developer | IoT Solutions & Security)
- **Saud Faisal Alfadda** (Hardware Integration)
- **Meshari Abdullah Alsaegh** (System Requirements)

### 🎓 Under the Supervision of:
- **Dr. Salim El-Khediri** (Academic Supervisor)

---

> *"Innovating for a sustainable and inclusive future, in alignment with Saudi Vision 2030."*
