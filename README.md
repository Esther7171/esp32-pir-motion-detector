# ESP32 PIR Motion Detector

A real-time motion detection system built with an ESP32 and HC-SR501 PIR sensor. When motion is detected, it sends an instant alert to a live web dashboard accessible from any device on the same WiFi network — no app install needed, works directly in your phone browser.

![Dashboard Preview](assets/dashboard_preview.png)

---

## Features

- Live web dashboard with radar animation
- Instant WebSocket alerts — no page refresh needed
- False positive detection and filtering
- Live motion timer — shows how long someone is in range
- Event log with start time, end time, and duration
- Works on iPhone and Android browsers
- Saves log to ESP32 flash memory (SPIFFS)
- Copy log and paste it anywhere to analyze false triggers

---

## What You Need

| Part | Details |
|---|---|
| ESP32 WROOM-32 | Any 30-pin variant works |
| HC-SR501 PIR Sensor | Infrared motion sensor |
| 3x Female-to-Female jumper wires | For connecting PIR to ESP32 |
| USB cable | To upload code from your computer |
| WiFi network | 2.4GHz only — ESP32 does not support 5GHz |

> **Total cost:** Around ₹600–800 if buying from local market or Amazon India

---

## Wiring

The HC-SR501 has 3 pins on the bottom edge. When the **white dome faces you**, the pins are ordered left to right: **VCC → OUT → GND**

| HC-SR501 Pin | ESP32 Pin | Wire Color (suggested) |
|---|---|---|
| VCC | VIN (5V) | Red |
| OUT (signal) | GPIO 13 | Blue or any color |
| GND | GND | Black |

<img width="752" height="468" alt="image" src="https://github.com/user-attachments/assets/579c1d9f-5479-4184-9cec-fad9b30eac49" />


> Use female-to-female jumper wires since both boards have male header pins.

```
HC-SR501                    ESP32
---------                   -----
  VCC  ─────────────────►  VIN
  OUT  ─────────────────►  GPIO 13
  GND  ─────────────────►  GND
```

> **Important:** Use VIN (5V), not 3.3V. The HC-SR501 needs 5V to operate correctly.

---

## PIR Sensor Setup

Before connecting, set up your PIR sensor correctly:

**1. Put the white dome back on**
The dome is the Fresnel lens — it focuses infrared onto the sensor. Always keep it on during use.

**2. Set the jumper to L mode**
There is a small black jumper on the board. Move it to the **L position** (single trigger). In H mode the sensor re-triggers itself every few seconds even with nothing moving, causing false alarms.

**3. Set the potentiometers**

| Knob | Label | Setting |
|---|---|---|
| Left | SX (Sensitivity) | Turn to midpoint for ~5m range |
| Right | TX (Time delay) | Turn fully counter-clockwise for minimum delay |

**4. Wait 30 seconds after powering on**
The PIR needs about 30 seconds to calibrate to room temperature. The code handles this automatically — it ignores all signals for the first 30 seconds.

---

## Software Setup

### Step 1 — Install Arduino IDE
Download from [arduino.cc/en/software](https://www.arduino.cc/en/software)

### Step 2 — Add ESP32 board support
1. Open Arduino IDE
2. Go to **File → Preferences**
3. In "Additional boards manager URLs" paste:
```
https://raw.githubusercontent.com/espressif/arduino-esp32/gh-pages/package_esp32_index.json
```
4. Go to **Tools → Board → Boards Manager**
5. Search `esp32` and install **esp32 by Espressif Systems**

### Step 3 — Install the WebSockets library
1. Go to **Tools → Manage Libraries**
2. Search `WebSockets`
3. Install **WebSockets by Markus Sattler**

### Step 4 — Select your board
Go to **Tools → Board → ESP32 Arduino → ESP32 Dev Module**

---

## Uploading the Code

1. Open `code/motion.ino` in Arduino IDE
2. Find these two lines near the top and edit them:

```cpp
const char* ssid     = "YOUR_WIFI_NAME";
const char* password = "YOUR_WIFI_PASSWORD";
```

Replace with your actual 2.4GHz WiFi network name and password.

> **Note:** ESP32 only supports 2.4GHz WiFi. If your router shows two networks (e.g. `MyWifi` and `MyWifi_5G`), use the one without `_5G`.

3. Connect ESP32 to your computer via USB
4. Go to **Tools → Port** and select the COM port (e.g. COM4 or COM6)
5. Click the **Upload button** (right arrow →)

> If upload gets stuck at `Connecting....` — hold the **BOOT** button on the ESP32 until uploading starts, then release.

6. Once uploaded, open **Tools → Serial Monitor**
7. Set baud rate to **115200**
8. Press the **EN (Reset)** button on the ESP32
9. You will see:

```
Connecting.....
Connected! IP: 192.168.1.42
```

---

## Opening the Dashboard

1. Make sure your phone is on the **same WiFi network** as the ESP32
2. Open **Safari (iPhone) or Chrome (Android)**
3. Type the IP address shown in Serial Monitor into the address bar (e.g. `192.168.1.42`)
4. The dashboard opens — **tap the screen once** to enable sound alerts

> **Tip for iPhone:** Add the page to your home screen via Share → Add to Home Screen for a fullscreen app-like experience.

---

## How the Dashboard Works

### Home tab
- **Radar** — animated radar sweep with a red blip when motion is detected
- **Real motion counter** — counts confirmed detections (green)
- **False alarms counter** — counts short spurious triggers (orange)
- **Live timer** — while someone is in the sensor zone, a live counter shows how long they have been there
- **Event log** — each event shows start time, end time, and duration

### Log tab
- Shows the raw log saved to the ESP32's flash memory
- Tap **Refresh** to load the latest log
- Tap **Copy all** then paste it anywhere to analyze your data
- Tap **Clear** to wipe the log

---

## Understanding the Event Log

```
=== Started, warming up 30s ===    ← boot, ignoring signals for 30s
=== Warmup done, now active ===    ← now monitoring
MOTION #1 | held:4200ms | time:45s uptime   ← real motion, lasted 4.2s
FALSE+ | held:12ms | time:52s uptime        ← too short, filtered out
MOTION #2 | held:7800ms | time:98s uptime   ← someone stood for 7.8s
```

**What "held" means:** How long the sensor output stayed HIGH — basically how long someone was in the detection zone. It is NOT distance. This sensor cannot measure distance, only motion.

**False positive rule:** Any trigger shorter than 500ms is automatically classified as a false alarm and shown in orange. Real human movement always takes longer than that.

---

## Sensor Specs

| Spec | Value |
|---|---|
| Model | HC-SR501 |
| Detection range | 3 to 7 metres (adjustable) |
| Detection angle | 110 degree cone |
| Operating voltage | 4.5V to 20V |
| Measures | Motion only — not distance |

---

## Troubleshooting

**Serial Monitor is blank after uploading**
Press the **EN** button on the ESP32 to restart it.

**Stuck at `Connecting....` during upload**
Hold the **BOOT** button while uploading, release when you see upload progress.

**WiFi not connecting**
Make sure you are using a 2.4GHz network, not 5GHz. Check that the SSID and password are exactly correct — they are case sensitive.

**Lots of false alarms on startup**
Normal — the PIR takes 30 seconds to calibrate. The code already ignores signals during warmup. If false alarms continue after warmup, move the jumper to **L position** on the PIR board.

**Dashboard not loading on phone**
Make sure your phone and ESP32 are on the same WiFi router. Open the exact IP shown in Serial Monitor in Safari or Chrome.

**Sound not working on iPhone**
Tap anywhere on the screen once first. iOS requires a user interaction before playing audio.

**COM port changed after unplugging**
Go to Tools → Port and select the new COM number. The board is the same, Windows just assigned a different port number.

---

## How False Positive Filtering Works

The PIR sensor sometimes fires briefly due to:
- Electrical noise on power-on
- Temperature changes in the room (AC, sunlight, fans)
- The sensor self-triggering in H jumper mode

The code measures how long each trigger lasts. If it is shorter than 500ms, it is flagged as a false positive. Real human motion always produces a trigger longer than 500ms because it takes at least half a second to move through the sensor's field of view.

---

## Built With

- ESP32 WROOM-32
- HC-SR501 PIR sensor
- Arduino IDE with ESP32 core by Espressif
- WebSockets library by Markus Sattler
- Vanilla HTML, CSS, JavaScript — no frameworks

---

## License

MIT License — free to use, modify, and share.

---

## Author

Built as a beginner IoT project. If this helped you, give it a star!
