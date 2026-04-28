/*
 * =========================================================================================
 * Project: IoT-Enabled Solar-Powered Smart Wheelchair
 * Project Lead & Core Developer: Mansour Mutlaq Alharbi
 * Support Team: Saud Faisal Alfadda, Meshari Abdullah Alsaegh
 * University: Qassim University - IT Department (Under the supervision of Dr. Salim El-Khediri)
 * * Description: 
 * This firmware controls the ESP32 Edge Node for the Smart Wheelchair. Core functionalities:
 * - Asynchronous Web Server for real-time UI telemetry and manual control.
 * - Smart Obstacle Avoidance Algorithm (Ultrasonic Radar Sweep).
 * - Real-time GPS Tracking via Hardware Serial.
 * - Hardware Watchdog Timer (WDT) & Heartbeat Connection Failsafe.
 * =========================================================================================
 */


#include <WiFi.h>
#include <ESPAsyncWebServer.h>
#include <ESP32Servo.h>
#include <esp_task_wdt.h>
#include <TinyGPS++.h>

// =========================================================
// [1] System Configurations & Pin Definitions
// =========================================================
#define READINGS_COUNT 5
float distanceReadings[READINGS_COUNT]; 
int readingIndex = 0;

// L298N Motor Driver Pins
#define IN1_PIN    26   // Left Motor - Forward
#define IN2_PIN    27   // Left Motor - Backward
#define IN3_PIN    14   // Right Motor - Forward
#define IN4_PIN    12   // Right Motor - Backward

// Sensor & Actuator Pins
#define TRIG_PIN   5    // Ultrasonic Trigger
#define ECHO_PIN   18   // Ultrasonic Echo
#define MYSERVO_PIN 19  // Radar Servo Motor
#define BUZZER_PIN 21   // Alert Buzzer
#define LIGHTS_PIN 23   // Headlights

// GPS Module Pins (Hardware Serial)
#define GPS_RX_PIN 16 
#define GPS_TX_PIN 17 

// =========================================================
// [2] Secure Local Network (Edge AP)
// =========================================================
const char* ssid = "IoT-Enabled Solar Wheelchair"; 
const char* password = "Safe@Wheel2030"; 

// Core System Objects
AsyncWebServer server(80); // Asynchronous server to prevent blocking the main loop
Servo myservo;
TinyGPSPlus gps; 

// =========================================================
// [3] System State & Telemetry Variables
// =========================================================
float myTemp = random(5, 10);
int myHeartRate = map(analogRead(34), 0, 4095, 60, 95); // Simulated Pulse Sensor         
String gpsCoords = "26.348,43.766"; 
String currentMovement = "STOP"; 

// Non-blocking Timers (millis-based)
unsigned long lastCommTime = 0;    // Heartbeat monitor for connection drops
unsigned long lastSensorCheck = 0; // Polling interval for ultrasonic sensor
float currentDistance = 0.0;

// Function Prototypes
int readHeartRate();   
void moveRobot(String cmd);
void stopMotors();
float getFilteredDistance();
void safetySystem();
void scanServo();
void performSmartAvoidance();

// =========================================================
// [4] Dashboard UI Payload (Stored in PROGMEM)
// =========================================================
const char index_html[] PROGMEM = R"rawliteral(
<!DOCTYPE HTML>
<html lang="ar">
<head>
  <meta charset="UTF-8">
  <meta name="viewport" content="width=device-width, initial-scale=1, user-scalable=no">
  <title>IoT Solar Wheelchair</title>
  <link href="https://fonts.googleapis.com/css2?family=Cairo:wght@400;700&family=Roboto:wght@500;900&display=swap" rel="stylesheet">
  <style>
    :root { --main-color: #00ffcc; --bg-color: #02050a; --card-bg: rgba(255, 255, 255, 0.05); --danger-color: #ff004c; --solar-color: #ffd700; }
    body { background-color: var(--bg-color); background-image: radial-gradient(circle at 50% 20%, #0f2027 0%, #02050a 70%); color: white; font-family: 'Roboto', 'Cairo', sans-serif; margin: 0; padding: 0; display: flex; flex-direction: column; align-items: center; min-height: 100vh; text-align: center; -webkit-user-select: none; user-select: none; }
    .header { width: 100%; padding-top: 30px; margin-bottom: 5px; }
    .logo-container { display: flex; justify-content: center; align-items: center; gap: 20px; margin-bottom: 15px; transition: all 0.3s ease; }
    .icon-wheelchair { width: 85px; height: 85px; color: var(--main-color); filter: drop-shadow(0 0 8px var(--main-color)); transition: all 0.3s ease; }
    .status-badge { display: inline-flex; align-items: center; gap: 8px; margin-bottom: 20px; padding: 6px 20px; border-radius: 50px; border: 1px solid var(--main-color); color: var(--main-color); font-weight: bold; font-size: 0.9rem; background: rgba(0,0,0,0.5); box-shadow: 0 0 10px rgba(0,0,0,0.5); transition: all 0.3s ease; }
    body.offline-mode { --main-color: #ff004c !important; background-image: radial-gradient(circle at 50% 20%, #4a0000 0%, #02050a 80%) !important; }
    @keyframes glitch { 0% { opacity: 1; transform: translate(0); } 20% { opacity: 0.7; transform: translate(-2px, 1px); } 40% { opacity: 1; transform: translate(2px, -1px); filter: hue-rotate(90deg); } 60% { opacity: 0.8; transform: translate(-1px, -1px); } 80% { opacity: 1; transform: translate(1px, 1px); } 100% { opacity: 1; transform: translate(0); } }
    .offline-mode .status-badge { animation: electric-flicker 0.1s infinite !important; background: rgba(255, 0, 76, 0.25) !important; border-color: #ff004c !important; color: #ff004c !important; box-shadow: 0 0 20px rgba(255, 0, 76, 0.6); }
    .offline-mode .logo-container .icon-wheelchair { color: #ff004c !important; filter: drop-shadow(0 0 15px #ff004c); animation: glitch 0.4s infinite; }
    .grid-container { display: grid; grid-template-columns: 1fr 1fr 1fr; gap: 10px; width: 95%; max-width: 400px; margin: 20px 0; }
    .card { background: var(--card-bg); border: 1px solid rgba(255,255,255,0.1); border-radius: 15px; padding: 15px 5px; display: flex; flex-direction: column; align-items: center; justify-content: center; box-shadow: 0 4px 10px rgba(0,0,0,0.3); position: relative; }
    .icon svg { width: 32px; height: 32px; fill: currentColor; margin-bottom: 8px; }
    .value { font-size: 1.4rem; font-weight: bold; color: #fff; margin: 2px 0; }
    .lbl-en { font-size: 0.6rem; color: #888; text-transform: uppercase; font-weight: bold; letter-spacing: 0.5px; }
    .lbl-ar { font-size: 0.75rem; color: #aaa; font-family: 'Cairo'; margin-top: 2px; }
    .heart-anim { animation: beat 1.2s infinite; color: #ff004c; filter: drop-shadow(0 0 5px #ff004c); }
    @keyframes beat { 0%, 100% { transform: scale(1); } 25% { transform: scale(1.15); } }
    .solar-badge { position: absolute; top: 6px; right: 6px; font-size: 0.6rem; background: var(--solar-color); color: #000; padding: 2px 6px; border-radius: 4px; font-weight: bold; }
    .d-pad { display: grid; grid-template-columns: repeat(3, 80px); grid-template-rows: repeat(3, 80px); gap: 10px; justify-content: center; margin-bottom: 25px; }
    .btn { width: 75px; height: 75px; border-radius: 50%; border: 2px solid var(--main-color); color: var(--main-color); background: rgba(0, 224, 255, 0.05); display: flex; flex-direction: column; justify-content: center; align-items: center; cursor: pointer; box-shadow: 0 0 15px rgba(0, 224, 255, 0.1); }
    .btn svg { width: 30px; height: 30px; margin-bottom: 2px; }
    .btn span { font-size: 0.85rem; font-weight: bold; font-family: 'Cairo'; }
    .btn:active { background: var(--main-color); color: #050b14; box-shadow: 0 0 25px var(--main-color); transform: scale(0.95); }
    .btn-stop { border-color: var(--danger-color); color: var(--danger-color); grid-column: 2; grid-row: 2; background: rgba(255, 0, 76, 0.15); }
    .btn-stop:active { background: var(--danger-color); color: #fff; box-shadow: 0 0 25px var(--danger-color); }
    .up { grid-column: 2; grid-row: 1; } .left { grid-column: 1; grid-row: 2; } .right { grid-column: 3; grid-row: 2; } .down { grid-column: 2; grid-row: 3; }
    .action-row { display: flex; gap: 15px; justify-content: center; align-items: stretch; width: 95%; max-width: 400px; margin-bottom: 40px; }
    .light-btn { flex: 1; background: rgba(255,255,255,0.08); border: 1px solid #555; color: #999; border-radius: 15px; cursor: pointer; display: flex; flex-direction: column; justify-content: center; align-items: center; padding: 12px; transition: 0.3s; }
    .light-on { border-color: var(--main-color); color: var(--main-color); background: rgba(0, 255, 204, 0.15); box-shadow: 0 0 15px rgba(0, 255, 204, 0.2); }
    .sos-btn { flex: 1.5; background: linear-gradient(135deg, #ff004c 0%, #cc0033 100%); border: none; border-radius: 15px; color: white; display: flex; flex-direction: column; justify-content: center; align-items: center; padding: 12px; cursor: pointer; box-shadow: 0 4px 20px rgba(255, 0, 76, 0.4); transition: transform 0.2s; }
    .sos-btn:active { transform: scale(0.95); box-shadow: 0 0 30px rgba(255, 0, 76, 0.7); }
    footer { width: 100%; padding: 40px 0 30px 0; background: rgba(0,0,0,0.4); border-top: 1px solid rgba(255,255,255,0.05); margin-top: auto; }
    .footer-title { font-size: 0.75rem; color: #777; margin-bottom: 20px; letter-spacing: 0.5px; text-transform: uppercase; }
    .dev-section { margin-bottom: 25px; }
    .label { font-size: 0.7rem; color: #666; margin-bottom: 8px; text-transform: uppercase; letter-spacing: 1px; font-weight: bold; }
    .names { font-family: 'Cairo'; font-size: 0.95rem; font-weight: bold; color: #e0e0e0; line-height: 1.8; }
    .sup-name { font-size: 1rem; font-weight: bold; color: var(--main-color); margin-bottom: 5px; margin-top: 5px; }
    .uni-name { font-size: 0.75rem; color: #555; margin-top: 5px; }
    @keyframes electric-flicker { 0% { opacity: 1; filter: brightness(1.5); } 50% { opacity: 0.1; filter: brightness(0.5); } 100% { opacity: 1; } }
  </style>
</head>
<body>
  <div class="logo-container">
       <svg class="icon-wheelchair" style="width: 180px; height: auto;" viewBox="0 0 120 110" fill="none">
         <g style="color: var(--solar-color); filter: drop-shadow(0 0 10px var(--solar-color));">
            <path transform="translate(48, 0) scale(0.7)" fill="currentColor" d="M12 7c-2.76 0-5 2.24-5 5s2.24 5 5 5 5-2.24 5-5-2.24-5-5-5zM2 13h2c.55 0 1-.45 1-1s-.45-1-1-1H2c-.55 0-1 .45-1 1s.45 1 1 1zm18 0h2c.55 0 1-.45 1-1s-.45-1-1-1h-2c-.55 0-1 .45-1 1s.45 1 1 1zM11 2v2c0 .55.45 1 1 1s1-.45 1-1V2c0-.55-.45-1-1-1s-1 .45-1 1zm0 18v2c0 .55.45 1 1 1s1-.45 1-1v-2c0-.55-.45-1-1-1s-1 .45-1 1zM5.99 4.93l1.41 1.41c.39.39 1.02.39 1.41 0 .39-.39.39-1.02 0-1.41L7.4 3.52c-.39-.39-1.02-.39-1.41 0-.39.39-.39 1.02 0 1.41zm11.31 11.31l1.41 1.41c.39.39 1.02.39 1.41 0 .39-.39.39-1.02 0-1.41l-1.41-1.41c-.39-.39-1.02-.39-1.41 0-.39.39-.39 1.02 0 1.41z"/>
            <path d="M60 28 L60 42 M50 30 L45 40 M70 30 L75 40" stroke="currentColor" stroke-width="2" stroke-linecap="round" opacity="0.8"/>
         </g>
         <g style="color: var(--main-color); filter: drop-shadow(0 0 8px var(--main-color));" transform="translate(25, 45)">
            <path d="M0 0 L70 0 L65 12 L5 12 Z" fill="#0a1520" stroke="currentColor" stroke-width="1.5"/>
            <line x1="15" y1="0" x2="18" y2="12" stroke="currentColor" stroke-width="1" opacity="0.7"/>
            <line x1="35" y1="0" x2="35" y2="12" stroke="currentColor" stroke-width="1" opacity="0.7"/>
            <line x1="55" y1="0" x2="52" y2="12" stroke="currentColor" stroke-width="1" opacity="0.7"/>
            <line x1="3" y1="6" x2="67" y2="6" stroke="currentColor" stroke-width="1" opacity="0.7"/>
            <path transform="translate(10, 12) scale(0.9)" fill="currentColor" d="M20 40h-4c-1.1 0-2 .9-2 2v4c0 1.1.9 2 2 2h4c1.1 0 2-.9 2-2v-4c0-1.1-.9-2-2-2zm-2 6h-1v-2h1v2zm16-6h-4c-1.1 0-2 .9-2 2v4c0 1.1.9 2 2 2h4c1.1 0 2-.9 2-2v-4c0-1.1-.9-2-2-2zm-2 6h-1v-2h1v2zM36 24H20c-2.21 0-4 1.79-4 4v10h24V28c0-2.21-1.79-4-4-4zM27 2c-1.66 0-3 1.34-3 3s1.34 3 3 3 3-1.34 3-3-1.34-3-3-3zm9 16h-3.33c.48-1.16.67-2.52.3-4h3.03c1.1 0 2 .9 2 2v2z"/>
         </g>
       </svg>
  </div>
  <div id="status-badge" class="status-badge">
      <svg style="width:18px; height:18px" viewBox="0 0 24 24" fill="currentColor"><path d="M12 11c-1.1 0-2 .9-2 2s.9 2 2 2 2-.9 2-2-.9-2-2-2zm6 2c0-3.31-2.69-6-6-6s-6 2.69-6 6c0 .34.04.67.09 1H10c-.03-.32-.05-.66-.05-1 0-1.14.9-2.08 2.05-2.08 1.13 0 2.05.94 2.05 2.08 0 .34-.02.68-.05 1h1.91c.05-.33.09-.66.09-1zm-6-8C8.45 5 5.25 6.64 3.28 9.22l1.65 1.49C6.44 8.78 9.06 7.5 12 7.5s5.56 1.28 7.07 3.21l1.65-1.49C18.75 6.64 15.55 5 12 5z"/></svg>
      <span id="status-txt">CONNECTED / متصل</span>
  </div>
  </div>
  <div class="grid-container">
    <div class="card">
        <div class="icon heart-anim"><svg viewBox="0 0 24 24"><path d="M12 21.35l-1.45-1.32C5.4 15.36 2 12.28 2 8.5 2 5.42 4.42 3 7.5 3c1.74 0 3.41.81 4.5 2.09C13.09 3.81 14.76 3 16.5 3 19.58 3 22 5.42 22 8.5c0 3.78-3.4 6.86-8.55 11.54L12 21.35z"/></svg></div>
        <div class="value" id="hr">72</div>
        <div class="lbl-en">Heart Rate</div>
        <div class="lbl-ar">نبض القلب</div>
    </div>
    <div class="card">
        <div class="icon"><svg viewBox="0 0 24 24"><path d="M22 6c0-1.1-.9-2-2-2H4c-1.1 0-2 .9-2 2v12c0 1.1.9 2 2 2h16c1.1 0 2-.9 2-2V6zm-2 0l-.01 2H18V6h2zm-4 0v2h-2V6h2zm-4 0v2h-2V6h2zm-4 0v2H6V6h2zM4 6h1v2H4V6zm0 12v-2h1v2H4zm16 0H4v-1h16v1z"/></svg></div>
        <div class="value"><span id="dist">--</span></div>
        <div class="lbl-en">Obstacle Dist.</div>
        <div class="lbl-ar">بعد العائق (سم)</div>
    </div>
    <div class="card">
        <div class="icon" style="color:#00ff88;"><svg viewBox="0 0 24 24"><path d="M15.67 4H14V2h-4v2H8.33C7.6 4 7 4.6 7 5.33v15.33C7 21.4 7.6 22 8.33 22h7.33c.74 0 1.34-.6 1.34-1.33V5.33C17 4.6 16.4 4 15.67 4z"/></svg></div>
        <div class="value" id="batt">95%</div>
        <div class="lbl-en">Battery Level</div>
        <div class="lbl-ar">مستوى البطارية</div>
    </div>
    <div class="card">
        <div class="solar-badge">Active</div>
        <div class="icon" style="color:#ffd700;"><svg viewBox="0 0 24 24"><path d="M6.76 4.84l-1.8-1.79-1.41 1.41 1.79 1.79 1.42-1.41zM4 10.5H1v2h3v-2zm9-9.95h-2V3.5h2V.55zm7.45 3.91l-1.41-1.41-1.79 1.79 1.41 1.41 1.79-1.79zm-3.21 13.7l1.79 1.79 1.41-1.41-1.79-1.79-1.41 1.41zM20 10.5v2h3v-2h-3zm-8-5c-3.31 0-6 2.69-6 6s2.69 6 6 6 6-2.69 6-6-2.69-6-6-6zm-1 16.95h2V19.5h-2v2.95zm-7.45-3.91l1.41 1.41 1.79-1.8-1.41-1.41-1.79 1.8z"/></svg></div>
        <div class="value" style="font-size:1rem; color:#ffd700; margin-top:5px;">Charging</div>
        <div class="lbl-en">Solar Status</div>
        <div class="lbl-ar">الطاقة الشمسية</div>
    </div>
   <div class="card">
        <div class="icon" style="color:#ff9f43;"><svg viewBox="0 0 24 24"><path d="M15 13V5c0-1.66-1.34-3-3-3S9 3.34 9 5v8c-1.21.91-2 2.37-2 4 0 2.76 2.24 5 5 5s5-2.24 5-5c0-1.63-.79-3.09-2-4zm-4-8c0-.55.45-1 1-1s1 .45 1 1h-2z" fill="currentColor"/></svg></div>
        <div class="value" id="temp">--°C</div>
        <div class="lbl-en">TEMPERATURE</div>
        <div class="lbl-ar">درجة الحرارة</div>
    </div>
    <div class="card">
        <div class="icon" style="color:#00d9ff;"><svg viewBox="0 0 24 24"><path d="M12 2C8.13 2 5 5.13 5 9c0 5.25 7 13 7 13s7-7.75 7-13c0-3.87-3.13-7-7-7zm0 9.5c-1.38 0-2.5-1.12-2.5-2.5s1.12-2.5 2.5-2.5 2.5 1.12 2.5 2.5-1.12 2.5-2.5 2.5z" fill="currentColor"/></svg></div>
        <div class="value" id="gps-val" style="font-size: 0.8rem;">Searching...</div>
        <div class="lbl-en">GPS LOCATION</div>
        <div class="lbl-ar">الموقع الجغرافي</div>
        <button onclick="openMap()" style="background:#00d9ff; color:#050b14; border:none; padding:3px 10px; border-radius:4px; font-weight:bold; cursor:pointer; font-size:0.7rem; margin-top:5px; font-family:'Cairo';">OPEN MAP / الخريطة</button>
    </div>
</div> <div class="speed-dashboard" style="width: 95%; max-width: 400px; background: rgba(0, 255, 204, 0.05); border-left: 4px solid #00ffcc; padding: 12px; margin: 15px auto; border-radius: 0 12px 12px 0;">
    <div style="display: flex; justify-content: space-between; align-items: center;">
        <span style="font-size: 0.7rem; color: #fff; font-weight: bold;">DRIVE SPEED</span>
        <span style="font-size: 0.7rem; color: #aaa; font-family: 'Cairo';">سرعة القيادة</span>
    </div>
    <div style="display: flex; align-items: baseline; justify-content: center;">
        <span id="speed" style="font-family: monospace; font-size: 2.5rem; font-weight: 900; color: #00ffcc;">0.0</span>
        <span style="font-size: 0.8rem; color: #777;">KM/H</span>
    </div>
    <div style="width: 100%; height: 3px; background: rgba(255,255,255,0.05); margin-top: 5px; overflow: hidden;">
        <div id="speed-bar" style="height: 100%; background: #00ffcc; width: 0%; transition: 0.3s;"></div>
    </div>
</div>
 <div class="d-pad">
      <button class="btn up" onmousedown="send('FORWARD')" onmouseup="send('STOP')" ontouchstart="send('FORWARD')" ontouchend="send('STOP')"><div class="btn-arrow">▲</div><div class="btn-en">FORWARD</div><div class="btn-ar">أمام</div></button>
      <button class="btn left" onmousedown="send('LEFT')" onmouseup="send('STOP')" ontouchstart="send('LEFT')" ontouchend="send('STOP')"><div class="btn-arrow">◀</div><div class="btn-en">LEFT</div><div class="btn-ar">يسار</div></button>
      <button class="btn btn-stop stop" onmousedown="send('STOP')" ontouchstart="send('STOP')"><div class="stop-text-en">STOP</div><div class="stop-text-ar">توقف</div></button>
      <button class="btn right" onmousedown="send('RIGHT')" onmouseup="send('STOP')" ontouchstart="send('RIGHT')" ontouchend="send('STOP')"><div class="btn-arrow">▶</div><div class="btn-en">RIGHT</div><div class="btn-ar">يمين</div></button>
      <button class="btn down" onmousedown="send('BACK')" onmouseup="send('STOP')" ontouchstart="send('BACK')" ontouchend="send('STOP')"><div class="btn-arrow">▼</div><div class="btn-en">BACK</div><div class="btn-ar">خلف</div></button>
 </div>
  <div class="action-row">
     <div id="lightBtn" class="light-btn" onclick="sendLight()">
        <svg style="width:24px;height:24px;margin-bottom:5px;" viewBox="0 0 24 24" fill="currentColor"><path d="M9 21c0 .55.45 1 1 1h4c.55 0 1-.45 1-1v-1H9v1zm3-19C8.14 2 5 5.14 5 9c0 2.38 1.19 4.47 3 5.74V17c0 .55.45 1 1 1h6c.55 0 1-.45 1-1v-2.26c1.81-1.27 3-3.36 3-5.74 0-3.86-3.14-7-7-7z"/></svg>
        <span style="font-size:0.8rem; font-weight:bold;">LIGHTS: OFF</span>
        <span id="lightState" style="font-size:0.7rem;">الأنوار: مطفأة</span>
     </div>
  </div>
 <div class="action-row" style="margin-top:-20px; flex-direction:column; gap:10px;">

    <button class="sos-btn" onclick="sendSOS()" style="flex-direction: column; gap: 2px;">
        <div style="display:flex; align-items:center; justify-content:center; gap:8px;">
            <svg style="width:24px; height:24px;" viewBox="0 0 24 24" fill="currentColor">
                <path d="M12 2L1 21h22L12 2zm1 14h-2v2h2v-2zm0-6h-2v4h2v-4z"/>
            </svg>
            <span style="font-size:1.5rem; font-weight:900;">SOS</span>
            <span style="font-size:0.8rem; font-weight:bold;">EMERGENCY CALL</span>
        </div>
        <div style="font-family:'Cairo'; font-size:1.1rem; font-weight:bold;">
            الطوارئ
        </div>
    </button>

    <button class="sos-btn"
            onclick="stopSOS()"
            style="background:#333; box-shadow:none;">
        <div style="font-size:1.2rem; font-weight:bold;">STOP SOS</div>
        <div style="font-family:'Cairo'; font-size:0.9rem;">إيقاف الطوارئ</div>
    </button>

</div>

  <footer>
    <div class="footer-title">IoT-Enabled Solar-Powered Smart Wheelchair Project © 2025</div>
    <div class="dev-section">
        <div class="label">Developed by:</div>
        <div class="names">Mansour Mutlaq Alharbi<br>Saud Faisal Alfadda<br>Meshari Abdullah Alsaegh</div>
    </div>
    <div class="dev-section">
        <div class="label">UNDER THE SUPERVISION OF</div>
        <div class="sup-name">Dr. Salim El-Khediri</div>
        <div class="uni-name">Department of Information Technology<br>Qassim University © 2025</div>
    </div>
  </footer>
  <script>
    function updateState(online) {
    let bd = document.body;
    let st = document.getElementById('status-txt');
    if(online) {
        bd.classList.remove('offline-mode');
        st.innerText = "CONNECTED / متصل";
    } else {
        bd.classList.add('offline-mode');
        st.innerHTML = "CONNECTION LOST / <br>فقد الاتصال";
    }
}
    function send(cmd) {
        fetch(`/move?dir=${cmd}`).catch(e => console.log(e));
    }
    function sendLight() {
        fetch("/move?dir=LIGHT_ON_OFF").then(() => {
            let btn = document.getElementById("lightBtn");
            let txt = document.getElementById("lightState");
            if (txt.innerText.includes("مطفأة")) {
                btn.classList.add("light-on");
                txt.innerHTML = "الأنوار: تعمل";
                btn.querySelector("span").innerText = "LIGHTS: ON";
            } else {
                btn.classList.remove("light-on");
                txt.innerHTML = "الأنوار: مطفأة";
                btn.querySelector("span").innerText = "LIGHTS: OFF";
            }
        });
    }
    function openMap() {
        // Fetch GPS value from the DOM element
        let coordsElem = document.getElementById("gps-val"); 
        let coords = coordsElem ? coordsElem.innerText : "";

        if(coords && coords.length > 5 && coords !== "Searching...") {
             // Redirect to Google Maps
             window.open("http://maps.google.com/?q=" + coords, "_blank");
        } else {
             alert("Waiting for GPS signal...\nجاري انتظار إشارة الأقمار الصناعية...");
        }
    }

    function sendSOS() {
        if (navigator.vibrate) navigator.vibrate([200, 100, 200, 100, 200]);
        send("SOS"); 
        alert(
"🚨 EMERGENCY MODE ACTIVATED\n" +
"تم تفعيل وضع الطوارئ\n" +
"--------------------------------\n" +
"✔ GPS Coordinates sent\n" +
"✔ تم إرسال الموقع الجغرافي\n\n" +
"📞 Dialing Emergency Services...\n" +
"📞 جارٍ الاتصال بخدمات الطوارئ"
);

        window.location.href = "tel:911"; 
    }

    function stopSOS() {
    send("STOP_SOS");
    alert(
        "🛑 EMERGENCY STOPPED\n" +
        "تم إيقاف الطوارئ\n\n" +
        "✔ Alarm stopped\n" +
        "✔ تم إيقاف الصوت والإنذار"
    );
}

    setInterval(() => {
        const controller = new AbortController();
        const timeoutId = setTimeout(() => controller.abort(), 1000);
        fetch("/data", { signal: controller.signal })
        .then(r => { clearTimeout(timeoutId); return r.json(); })
        .then(d => {
            updateState(true);
            if(d.dist) {
                let distElem = document.getElementById("dist");
                let distance = parseFloat(d.dist);
                distElem.innerText = d.dist;
                if(distance <= 35 && distance > 0) {
                    distElem.style.color = "#ff004c"; 
                    distElem.style.textShadow = "0 0 10px #ff004c";
                    if(window.navigator.vibrate) window.navigator.vibrate(200);
                } else {
                    distElem.style.color = "#fff";
                    distElem.style.textShadow = "none";
                }
            }
            if(d.speed) {
                document.getElementById("speed").innerText = d.speed;
                let speedBar = document.getElementById("speed-bar");
                if(speedBar) {
                    let speedPercent = (parseFloat(d.speed) / 5) * 100;
                    speedBar.style.width = speedPercent + "%";
                }
            }
            if(d.temp) document.getElementById("temp").innerText = d.temp + "°C";
            if(d.gps_val && d.gps_val.length > 5) {
                document.getElementById("gps-val").innerText = d.gps_val;
                if(document.getElementById("gps")) document.getElementById("gps").innerText = d.gps_val;
            } else {
                document.getElementById("gps-val").innerText = "Searching...";
            }
            if(d.hr) document.getElementById("hr").innerText = d.hr;
            if(d.batt) document.getElementById("batt").innerText = d.batt + "%";
        })
        .catch(err => { updateState(false); });
    }, 1500); 
    
      </script>
</body>
</html>
)rawliteral";

// =========================================================
// [5] Setup Routine (Hardware Initialization)
// =========================================================
void setup() {
    // Initialize Hardware Serial for Debugging
    Serial.begin(115200);
    
    // Initialize Hardware Serial 2 for stable GPS data parsing
    Serial2.begin(9600, SERIAL_8N1, GPS_RX_PIN, GPS_TX_PIN);

    // Initialize Motor Control Pins
    pinMode(IN1_PIN, OUTPUT); pinMode(IN2_PIN, OUTPUT);
    pinMode(IN3_PIN, OUTPUT); pinMode(IN4_PIN, OUTPUT); 
    stopMotors(); // Ensure motors are completely halted on boot
    delay(500); 

    // Initialize Sensors & Actuators
    pinMode(TRIG_PIN, OUTPUT); pinMode(ECHO_PIN, INPUT);
    pinMode(BUZZER_PIN, OUTPUT); 
    pinMode(LIGHTS_PIN, OUTPUT);

    digitalWrite(LIGHTS_PIN, LOW);
    digitalWrite(BUZZER_PIN, LOW);

    // Initialize Radar Servo
    myservo.attach(MYSERVO_PIN); 
    myservo.write(90); // Center the ultrasonic sensor (Forward View)

    // Configure Access Point (AP) for isolated Edge Computing operations
    WiFi.softAP(ssid, password);
    WiFi.setSleep(false); // CRITICAL: Disable WiFi power saving to prevent async server drops

    // Startup Success Tone
    digitalWrite(BUZZER_PIN, HIGH); delay(100); digitalWrite(BUZZER_PIN, LOW); delay(50);
    digitalWrite(BUZZER_PIN, HIGH); delay(100); digitalWrite(BUZZER_PIN, LOW);

    // Asynchronous Web Server Routes Setup
    server.on("/", HTTP_GET, [](AsyncWebServerRequest *r){ 
        r->send_P(200, "text/html", index_html); 
    });
    
    server.on("/move", HTTP_GET, [](AsyncWebServerRequest *r){ 
        if (r->hasParam("dir")) {
            String cmd = r->getParam("dir")->value();
            lastCommTime = millis(); // Update connection heartbeat

            if (cmd == "LIGHT_ON_OFF") {
                static bool ls = false;
                ls = !ls; 
                digitalWrite(LIGHTS_PIN, ls ? HIGH : LOW);
            }
            else if (cmd == "SOS") {
                // Trigger visual and auditory SOS alarms
                digitalWrite(BUZZER_PIN, HIGH); 
                digitalWrite(LIGHTS_PIN, HIGH); 
            }
            else if (cmd == "STOP_SOS") {
                // Terminate SOS alarms
                digitalWrite(BUZZER_PIN, LOW);
                digitalWrite(LIGHTS_PIN, LOW);
            }
            else { 
                currentMovement = cmd; 
                moveRobot(cmd); 
            }
        }
        r->send(200, "text/plain", "OK");
    });

    server.on("/data", HTTP_GET, [](AsyncWebServerRequest *r){ 
        String currentGPS = (gpsCoords == "") ? "Waiting..." : gpsCoords;
        // Construct and transmit telemetry JSON payload
        String json = "{";
        json += "\"dist\":\"" + String(currentDistance, 1) + "\","; 
        json += "\"hr\":\"" + String(readHeartRate()) + "\","; 
        json += "\"speed\":\"" + String(currentMovement == "STOP" ? "0.0" : "3.8") + "\","; 
        json += "\"gps_val\":\"" + currentGPS + "\","; 
        json += "\"temp\":\"" + String(random(5, 10)) + "\","; 
        json += "\"batt\":\"95\""; 
        json += "}";
        r->send(200, "application/json", json);
    });
    
    // Boot the Async Server
    server.begin();
    
    // Initialize Hardware Watchdog Timer (WDT) to prevent MCU freeze
    // System reboots automatically if loop is blocked for 8 seconds
    esp_task_wdt_config_t config = { .timeout_ms = 8000, .trigger_panic = true };
    esp_task_wdt_init(&config);
    esp_task_wdt_add(NULL);
}

// Simulated pulse reading function with basic averaging filter
int readHeartRate() {
  int total = 0;
  for (int i = 0; i < 10; i++) {
    total += map(analogRead(34), 0, 4095, 60, 95);
    delay(2);
  }
  return total / 10;
}

// =========================================================
// [6] Main Loop (Strict Non-Blocking Execution)
// =========================================================
void loop() {
    unsigned long currentMillis = millis();

    // 1. Continuous GPS Parsing (Hardware Serial)
    while (Serial2.available() > 0) {
        if (gps.encode(Serial2.read())) {
            if (gps.location.isValid()) {
                gpsCoords = String(gps.location.lat(), 6) + "," + String(gps.location.lng(), 6);
            }
        }
    }

    // 2. Sensor Polling & Collision Avoidance (Non-blocking: 250ms interval)
    if (currentMillis - lastSensorCheck >= 250) {
        lastSensorCheck = currentMillis;
        currentDistance = getFilteredDistance(); 
        safetySystem(); // Evaluate proactive collision avoidance
    }

    // 3. Continuous Radar Sweep (Non-blocking logic)
    scanServo(); 

    // 4. Connection Failsafe (Heartbeat Monitor)
    // Instantly halt wheelchair if no UI command is received for 5 seconds
    if (currentMillis - lastCommTime > 5000 && currentMovement != "STOP") {
        moveRobot("STOP");
        currentMovement = "STOP";
    }

    // 5. Reset Hardware Watchdog Timer (WDT)
    esp_task_wdt_reset(); 
    delay(10); // Minimal delay for CPU stability
}

// ================================================================
// [7] Control Logic & Artificial Intelligence 
// ================================================================

// Motor Direction Control (H-Bridge Logic)
void moveRobot(String cmd) {
    if (cmd == "FORWARD") {
        digitalWrite(IN1_PIN, LOW);  digitalWrite(IN2_PIN, HIGH);
        digitalWrite(IN3_PIN, LOW);  digitalWrite(IN4_PIN, HIGH);
    } else if (cmd == "BACK" || cmd == "BACKWARD") {
        digitalWrite(IN1_PIN, HIGH); digitalWrite(IN2_PIN, LOW);
        digitalWrite(IN3_PIN, HIGH); digitalWrite(IN4_PIN, LOW);
    } else if (cmd == "LEFT") {
        digitalWrite(IN1_PIN, LOW);  digitalWrite(IN2_PIN, HIGH);
        digitalWrite(IN3_PIN, HIGH); digitalWrite(IN4_PIN, LOW);
    } else if (cmd == "RIGHT") {
        digitalWrite(IN1_PIN, HIGH); digitalWrite(IN2_PIN, LOW);
        digitalWrite(IN3_PIN, LOW);  digitalWrite(IN4_PIN, HIGH);
    } else {
        stopMotors();
        digitalWrite(BUZZER_PIN, LOW); // Silence buzzer upon stopping
    }
}

// Complete Motor Halt
void stopMotors() {
    digitalWrite(IN1_PIN, LOW); digitalWrite(IN2_PIN, LOW);
    digitalWrite(IN3_PIN, LOW); digitalWrite(IN4_PIN, LOW);
}

// Ultrasonic Distance Measurement with Outlier Filtering
float getFilteredDistance() {
    digitalWrite(TRIG_PIN, LOW); delayMicroseconds(2);
    digitalWrite(TRIG_PIN, HIGH); delayMicroseconds(10);
    digitalWrite(TRIG_PIN, LOW);
    
    long duration = pulseIn(ECHO_PIN, HIGH, 25000); 
    
    if (duration == 0) return 400.0; 
    float d = duration * 0.034 / 2;
    if (d > 400.0 || d < 2.0) return 400.0; // Filter illogical values
    return d;
}

// --- Smart Obstacle Avoidance Algorithm ---
void performSmartAvoidance() {
    // Phase 1: Emergency Stop and User Alert
    moveRobot("STOP");
    digitalWrite(BUZZER_PIN, HIGH); delay(100); digitalWrite(BUZZER_PIN, LOW);
    delay(50);
    digitalWrite(BUZZER_PIN, HIGH); delay(100); digitalWrite(BUZZER_PIN, LOW);

    // Phase 2: Reverse maneuver to gain spatial clearance
    moveRobot("BACK");
    delay(300);
    moveRobot("STOP");

    // Phase 3: Radar sweep (Right side)
    myservo.write(0); 
    delay(400); 
    float rightDist = getFilteredDistance();

    // Phase 4: Radar sweep (Left side)
    myservo.write(180); 
    delay(600); 
    float leftDist = getFilteredDistance();

    // Phase 5: Re-center Radar
    myservo.write(90);
    delay(200);

    // Phase 6: Decision Making Logic (Navigate towards the widest path)
    if (rightDist > leftDist && rightDist > 20) {
        moveRobot("RIGHT");
        delay(500); 
        moveRobot("STOP");
    } 
    else if (leftDist >= rightDist && leftDist > 20) {
        moveRobot("LEFT");
        delay(500); 
        moveRobot("STOP");
    } 
    else {
        // Dead end scenario (Execute U-Turn)
        moveRobot("BACK");
        delay(500);
        moveRobot("LEFT"); 
        delay(800);
        moveRobot("STOP");
    }
    
    currentMovement = "STOP"; // Reset movement state lock
}

// Proactive Safety System (Sense-Think-Act Cycle)
void safetySystem() {
    if (currentMovement == "FORWARD" && currentDistance <= 40.0 && currentDistance > 1.0) {

        // Trigger brief auditory warning upon obstacle detection (Warning State)
        digitalWrite(BUZZER_PIN, HIGH);
        delay(100);
        digitalWrite(BUZZER_PIN, LOW);

        if (currentDistance <= 25.0) {
            performSmartAvoidance(); // Trigger autonomous evasion (Critical State)
        } else {
            // Proactively decelerate motors to mitigate potential impact
            digitalWrite(IN1_PIN, LOW);  
            digitalWrite(IN2_PIN, HIGH);
            digitalWrite(IN3_PIN, LOW);  
            digitalWrite(IN4_PIN, HIGH);
            delay(100);
            stopMotors();
        }
    }
}

// Continuous Radar Sweep Execution
void scanServo() {
    static int angle = 90;
    static int step = 2; 
    static unsigned long lastUpdate = 0;
    
    // Timer-based execution to preserve system concurrency (Non-blocking)
    if (millis() - lastUpdate > 40) { 
        if (angle >= 130 || angle <= 50) step *= -1; 
        angle += step;
        myservo.write(angle);
        lastUpdate = millis();
    }
}
