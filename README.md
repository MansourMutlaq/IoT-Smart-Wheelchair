# ♿ IoT-Enabled Solar-Powered Smart Wheelchair

<div align="center">

<br />

<img src="https://github.com/user-attachments/assets/c0dc0aa0-ef29-4f8c-a6d7-dcd749d767f0" width="450" alt="Smart Wheelchair Logo">

<br />
<br />

<img src="https://img.shields.io/badge/C++-00599C?style=for-the-badge&logo=c%2B%2B&logoColor=white" alt="C++">
<img src="https://img.shields.io/badge/Arduino-00979D?style=for-the-badge&logo=Arduino&logoColor=white" alt="Arduino">
<img src="https://img.shields.io/badge/Network-Secure_Local_AP-red?style=for-the-badge" alt="Secure Local AP">
<img src="https://img.shields.io/badge/Saudi_Vision-2030-10a37f?style=for-the-badge" alt="Saudi Vision 2030">

<br />
<br />

*An innovative IoT system designed for autonomous mobility and real-time health monitoring. This project serves as a **Secure Local Edge Node**, processing sensor data and providing a web-based dashboard without external internet dependency, ensuring privacy and high reliability for users.*

</div>

## 🚀 Key Features

- 🧠 **Smart Navigation:** Real-time obstacle avoidance using Ultrasonic sensors and Servo scanning.
- 🛡️ **Secure Connectivity:** Standalone WPA2-encrypted Access Point (AP) for off-grid operations.
- 📊 **Live Telemetry:** Web-based dashboard for heart rate, GPS coordinates, and system status.
- 🆘 **Emergency SOS:** Instant alarm and GPS location sharing for rapid emergency response.
- ☀️ **Sustainable Energy:** Solar-powered battery management for extended range.

---


## 🛡️ Security & Network Infrastructure (CIA Triad Focus)

As an IT specialization project, the architecture was built on Cyber Security principles to ensure a robust **Secure Edge Node**:

* **Network Hardening:** Established a standalone, WPA2-PSK encrypted local network to isolate control traffic. This eliminates external "Man-in-the-Middle" (MitM) risks.
* **High Availability (Heartbeat):** Implemented a custom Heartbeat protocol between the UI and ESP32. If the link is severed, the system triggers an immediate emergency halt, protecting against physical DoS (Denial of Service) scenarios.
* **Data Integrity:** Used Non-blocking Asynchronous communication to ensure telemetry data (GPS, Pulse) is delivered accurately without interfering with the motor control loops.

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


## 🏗️ Hardware Prototype & Build

The wheelchair Prototype is built with a custom-engineered structure, integrated components, and sustainable power management.

<div align="center">
  <table style="width: 100%; border-collapse: collapse; border: none;">
    <tr>
      <td style="padding: 15px; border: none; text-align: center;">
        <h3 style="margin-bottom: 10px;">Final Prototype Overview</h3>
        <img src="https://github.com/user-attachments/assets/bdc8f377-eac0-47c1-9270-4f7b9e214f02" 
             style="width: 350px; height: 450px; object-fit: cover; border-radius: 15px; box-shadow: 0 4px 8px rgba(0,0,0,0.2);" 
             alt="Smart Wheelchair Overview">
      </td>
      <td style="padding: 15px; border: none; text-align: center;">
        <h3 style="margin-bottom: 10px;">Solar Power Integration</h3>
        <img src="https://github.com/user-attachments/assets/d00d56b0-d05d-4367-8fdc-9938f8b990d3" 
             style="width: 350px; height: 450px; object-fit: cover; border-radius: 15px; box-shadow: 0 4px 8px rgba(0,0,0,0.2);" 
             alt="Solar Panel Highlight">
      </td>
    </tr>
  </table>
</div>

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

---

## 🛡️ System Reliability & Failsafe Logic

The application features a proactive Connectivity Monitor. If the heartbeat between the Edge Node (ESP32) and the Dashboard is interrupted, the UI instantly switches to a high-alert Failsafe mode.

<div align="center">
  <table style="border-collapse: collapse; border: none; background-color: #ffffff; padding: 10px; border-radius: 20px; box-shadow: 0 8px 16px rgba(0,0,0,0.1);">
    <tr>
      <td style="border: none; text-align: center; padding: 20px;">
        <img src="https://github.com/user-attachments/assets/d4145b2b-53ed-44a7-8ae6-74e90ccf73c5" width="280" alt="Failsafe Connection Lost Mode" style="border-radius: 12px;">
        <br><br>
        <b style="color: #d32f2f;">🔴 Connection Loss Detection</b><br>
        <i style="font-size: 0.9em; color: #555;">Instant visual feedback and system lockout when the wheelchair is unreachable.</i>
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


## 🚧 Engineering Challenges & Strategic Solutions

Throughout the development phase, several technical hurdles were addressed to ensure system stability and user safety:

### 🛠️ Technical Hurdles Overcome
* **Asynchronous Concurrency:** Managed simultaneous sensor data polling (Ultrasonic & GPS) while maintaining a responsive Web Server on the ESP32. This was solved by utilizing non-blocking libraries to prevent CPU hang-ups.
* **Network Reliability (Heartbeat Mechanism):** Designed a custom "Heartbeat" failsafe. If the control device (Phone/Laptop) loses connection to the Edge Node, the system automatically triggers an emergency stop to prevent unguided movement.
* **Power Management:** Integrated solar charging logic to maintain stable voltage for the ESP32 and motor drivers, balancing the variable output from solar panels with the high current demands of the motors.

---


🚧 Technical Challenges & Engineering Solutions
Asynchronous Concurrency: Managed simultaneous sensor polling while maintaining a responsive Web Server. Solved by using non-blocking libraries to prevent CPU hangs.

Network Reliability (Heartbeat): Designed a failsafe that triggers an emergency stop if the connection between the controller and the Edge Node is severed.

Power Management: Integrated solar charging logic to maintain stable voltage for the ESP32 and motor drivers.

System Stability: Implemented Hardware Watchdog Timer (WDT) to ensure automatic recovery from any software hangs.

---


## 🚀 Future Roadmap (Scalability & Security)

To transition this project from a prototype to an enterprise-grade IoT solution, the following enhancements are planned:

### 🔐 Advanced Security & Connectivity
* **Cloud Integration (IoT Hub):** Migrating from a local Edge Node to a centralized Cloud environment (Azure IoT or AWS IoT) for remote monitoring and long-term data analytics.
* **Encrypted Telemetry:** Implementing TLS/SSL encryption for all data packets sent between the wheelchair and the dashboard to ensure patient data privacy.
* **5G/LTE Expansion:** Integrating a GSM/LTE module for ubiquitous connectivity, moving beyond the limitations of local WiFi range.

### 🧠 Intelligent Navigation
* **AI-Powered Obstacle Detection:** Replacing ultrasonic sensors with an AI-enabled camera (Computer Vision) for advanced terrain recognition and dynamic object avoidance.
* **Predictive Maintenance:** Using machine learning to analyze motor performance and battery health, predicting potential hardware failures before they occur.

---


## 👥 Project Team & Roles

Developed at **Qassim University** (IT Department) - **Final Grade: A+**

### 🥇 Technical Lead & Full-Stack Developer
* **[Mansour Mutlaq Alharbi](https://github.com/MansourMutlaq)**
  * *Primary Responsibility:* End-to-end system engineering, including hardware circuit integration, ESP32 firmware development, autonomous navigation logic, and the real-time web dashboard.

### 📚 Academic Contributors
* **Saud Faisal Alfadda** (Research & Final Presentation)
* **Meshari Abdullah Alsaegh** (Documentation & Final Presentation)

### 🎓 Academic Supervision
* **Dr. Salim El-Khediri**

---

> *"Innovating for a sustainable and inclusive future, in alignment with Saudi Vision 2030."*
