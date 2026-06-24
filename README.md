# ESP32 PIR Motion Detector with Live Web Dashboard

Build a real-time motion detection system using an ESP32 and HC-SR501 PIR sensor. Get instant alerts on your phone browser the moment motion is detected. No app install needed. Works on iPhone and Android over your home WiFi.

<div align="center">
<img width="460" height="998" alt="dashboard_preview" src="https://github.com/user-attachments/assets/cfeb1424-344f-4cf5-99fc-c4cc4bce8e1c" />
</div>
<<<<<<< HEAD

---

## What This Project Does

This project turns a cheap PIR sensor and an ESP32 into a smart motion detector with a live dashboard you can open on any phone or laptop. When someone walks into the sensor zone the dashboard lights up instantly with a radar animation and a chime sound. You can see exactly how long the motion lasted and whether it was real or a false alarm.

Everything runs locally on your WiFi network. No cloud service. No subscription. No app download.
=======
>>>>>>> 5c37caba093c882fb4ac32d2401a31ba665538e3

---

## Features

- Live radar dashboard with real-time blip animation
- Instant WebSocket alerts with no page refresh
- Smart false positive filtering based on signal duration
- Live motion timer that counts up while someone is in range
- Full event log showing start time, end time and duration of each detection
- Works in Safari on iPhone and Chrome on Android
- Saves log file to ESP32 flash memory so nothing is lost on restart
- One tap copy of the log so you can paste and analyze it anywhere

---

## Parts You Need

| Part | Details |
|---|---|
| ESP32 WROOM-32 | Any 30 pin variant works |
| HC-SR501 PIR Sensor | Infrared passive motion sensor |
| 3 Female to Female jumper wires | For connecting PIR to ESP32 |
| USB cable | To upload code from your computer |
| WiFi router | Must be 2.4GHz. ESP32 does not support 5GHz |

> **Total cost in India:** Around Rs 600 to 800 from local electronics market or Amazon

---

## Wiring Guide

The HC-SR501 has 3 pins on the bottom edge. Hold the sensor with the white dome facing you. The pins go left to right in this order: VCC then OUT then GND.

| HC-SR501 Pin | ESP32 Pin | Wire Color |
|---|---|---|
| VCC | VIN (5V) | Red |
| OUT | GPIO 13 | Yellow or any color |
| GND | GND | Black |

<<<<<<< HEAD
<img width="752" height="468" alt="wiring diagram" src="https://github.com/user-attachments/assets/579c1d9f-5479-4184-9cec-fad9b30eac49" />
=======
<img width="752" height="468" alt="image" src="https://github.com/user-attachments/assets/579c1d9f-5479-4184-9cec-fad9b30eac49" />


> Use female-to-female jumper wires since both boards have male header pins.
>>>>>>> 5c37caba093c882fb4ac32d2401a31ba665538e3

```
HC-SR501              ESP32
--------              -----
  VCC  ────────────► VIN
  OUT  ────────────► GPIO 13
  GND  ────────────► GND
```

Use female to female jumper wires since both boards have male header pins.

> Use VIN for 5V power. Do not use the 3.3V pin. The HC-SR501 needs at least 4.5V to work correctly.

---

## Setting Up the PIR Sensor

Do these steps before connecting anything.

**Step 1 - Put the white dome back on**

The white dome is the Fresnel lens. It focuses infrared radiation onto the sensor element. Always keep it on during use. Remove it only to identify the pins.

**Step 2 - Move the jumper to L position**

There is a small black plastic jumper on the PIR board near the pins. Move it to the L position. In H mode the sensor re-triggers itself automatically every few seconds even when nothing is moving. This causes constant false alarms. L mode fires once and waits.

**Step 3 - Adjust the potentiometers**

| Knob | Label | Recommended Setting |
|---|---|---|
| Left knob | SX (Sensitivity) | Turn to midpoint for around 5 meter range |
| Right knob | TX (Time delay) | Turn fully counter-clockwise for minimum hold time |

**Step 4 - Let it warm up**

The PIR sensor needs about 30 seconds after power on to calibrate itself to the room temperature. During this time it fires random signals. The code automatically ignores all signals for the first 30 seconds so you do not need to do anything.

---

## Software Setup

### Install Arduino IDE

Download and install from [arduino.cc/en/software](https://www.arduino.cc/en/software)

### Add ESP32 Board Support

1. Open Arduino IDE
2. Go to File then Preferences
3. Paste this URL in the Additional Boards Manager URLs field:

```
https://raw.githubusercontent.com/espressif/arduino-esp32/gh-pages/package_esp32_index.json
```

4. Go to Tools then Board then Boards Manager
5. Search for esp32 and install the package by Espressif Systems

### Install the WebSockets Library

1. Go to Tools then Manage Libraries
2. Search for WebSockets
3. Install WebSockets by Markus Sattler

### Select Your Board

Go to Tools then Board then ESP32 Arduino then ESP32 Dev Module

---

## Uploading the Code

1. Open `code/motion.ino` in Arduino IDE

2. Find these two lines near the top of the file:

```cpp
const char* ssid     = "YOUR_WIFI_NAME";
const char* password = "YOUR_WIFI_PASSWORD";
```

3. Replace the placeholder text with your actual 2.4GHz WiFi name and password. The values are case sensitive.

> If your router broadcasts two networks like MyWifi and MyWifi_5G then always use the one without 5G in the name.

4. Connect the ESP32 to your computer with a USB cable

5. Go to Tools then Port and select the correct COM port (usually COM4 or COM6 on Windows)

6. Click the Upload button (the right arrow at the top left)

> If the upload gets stuck at Connecting then hold the BOOT button on the ESP32 board until you see upload progress in the output panel then release it.

7. After upload finishes open Tools then Serial Monitor

8. Set the baud rate to 115200 using the dropdown at the bottom right

9. Press the EN button on the ESP32 to restart it

10. You will see this output:

```
Connecting.....
Connected! IP: 192.168.1.42
```

Note down the IP address shown. You will need it to open the dashboard.

---

## Opening the Dashboard on Your Phone

1. Connect your phone to the same WiFi network the ESP32 is connected to
2. Open Safari on iPhone or Chrome on Android
3. Type the IP address from Serial Monitor into the address bar (example: 192.168.1.42)
4. The dashboard opens in your browser
5. Tap anywhere on the screen once to enable sound alerts

> iPhone tip: Tap the Share button then Add to Home Screen. This gives you a fullscreen app-like experience with an icon on your home screen.

---

## How the Dashboard Works

### Home Tab

The home tab shows you everything at a glance.

The radar at the top shows a rotating sweep. When motion is detected a red dot appears on the radar with expanding ring animations and a soft three note chime plays. The center of the radar shows the total detection count and the hold time of the last event.

Below the radar are two stat cards. The green card shows confirmed real motion detections. The orange card shows false alarms that were filtered out.

The recent events list below shows each event with the start time, end time, total duration and whether it was real or a false alarm.

### Log Tab

The log tab shows the raw data saved to the ESP32 flash memory. Tap Refresh to load the latest entries. Tap Copy All to copy the entire log to your clipboard then paste it anywhere to analyze the data. Tap Clear to wipe the log and start fresh.

---

## Reading the Log File

```
=== Started, warming up 30s ===       device just powered on, ignoring signals
=== Warmup done, now active ===       now monitoring for real motion
MOTION #1 | held:4200ms | time:45s   real motion, person was there for 4.2 seconds
FALSE+ | held:12ms | time:52s        too brief, filtered as false alarm
MOTION #2 | held:7800ms | time:98s   someone stood in range for 7.8 seconds
```

**What held time means**

Held is how long the sensor output signal stayed HIGH after being triggered. It tells you how long someone was within the detection zone. It does not tell you how far away they were. This sensor cannot measure distance. It only detects changes in infrared radiation.

**How false alarms are detected**

Any trigger that lasts less than 500 milliseconds is automatically classified as a false alarm. Real human movement through a PIR detection zone always takes longer than half a second. Very short pulses are caused by electrical noise, temperature changes or the sensor warming up.

---

## Sensor Specifications

| Spec | Value |
|---|---|
| Model | HC-SR501 |
| Detection range | 3 to 7 meters adjustable |
| Detection angle | 110 degree cone |
| Operating voltage | 4.5V to 20V |
| Output | Digital HIGH or LOW only |
| Can measure distance | No |

---

## Troubleshooting

**Serial Monitor shows nothing after upload**
Press the EN button on the ESP32 to restart it. The Serial Monitor needs a restart to show output.

**Upload stuck at Connecting**
Hold the BOOT button on the ESP32 while the upload is trying to connect. Release it when you see percentage progress in the output.

**WiFi keeps failing to connect**
Check that you are using a 2.4GHz network. Check the SSID and password are exactly correct including uppercase and lowercase letters. Move the ESP32 closer to your router.

**False alarms happening constantly**
This is almost always caused by the jumper being in H mode. Move it to L position. Also wait at least 60 seconds after power on before testing because the sensor fires randomly during warmup.

**Dashboard not opening on phone**
Your phone and ESP32 must be on the same WiFi router. Type the full IP address exactly as shown in Serial Monitor. Make sure there is no http or www in front.

**No sound on iPhone**
iOS blocks audio until the user interacts with the page. Tap anywhere on the screen once and then the chime will work for the rest of the session.

**COM port number changed**
This is normal after unplugging and replugging the USB cable. Go to Tools then Port and select the new port number. The board itself has not changed.

---

## How False Positive Filtering Works

The HC-SR501 sometimes produces short spurious triggers caused by:

- Electrical noise during power on
- Room temperature changes from air conditioning or sunlight
- The sensor self-triggering when the jumper is in H mode
- Vibration or air movement from fans

The code measures the duration of every trigger. When the signal goes LOW the code checks how long it was HIGH. If the duration is below 500 milliseconds it sends a false alarm event to the dashboard shown in orange. If it is above 500 milliseconds it is classified as real motion shown in red.

This threshold works well in practice because a person moving through a 110 degree detection cone at any normal walking speed will always trigger the sensor for more than half a second.

---

## Tech Stack

- ESP32 WROOM-32 microcontroller
- HC-SR501 PIR passive infrared sensor
- Arduino IDE with ESP32 core by Espressif Systems
- WebSockets library by Markus Sattler for real-time communication
- SPIFFS for log file storage on ESP32 flash memory
- Vanilla HTML, CSS and JavaScript for the dashboard
- No external frameworks or cloud services

---

## License

MIT License. Free to use, modify and share.

---

## Author

Built as a beginner IoT project using parts under Rs 800.
If this project helped you please give it a star on GitHub.
