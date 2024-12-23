# Global Glucose Monitor

A real-time glucose monitoring system using ESP32, Node-RED, and MQTT.

## Features

- Real-time glucose monitoring
- Global access through public MQTT broker
- Beautiful dashboard with charts and gauges
- Alert system for high/low glucose levels
- LED indicators on ESP32
- Data logging to CSV
- Moving average smoothing
- Trend detection
- NTP time synchronization

## Hardware Requirements

- ESP32 development board
- 3 LEDs (Red, Yellow, Green)
- Resistors for LEDs (220Ω)
- Breadboard and jumper wires

## Software Requirements

1. Arduino IDE with ESP32 support
2. Node-RED
3. Required Arduino Libraries:
   - PubSubClient
   - ArduinoJson
   - NTPClient
   - WiFiUdp

## LED Wiring

Connect LEDs to ESP32:
- Red LED: GPIO 12 (Critical alerts)
- Yellow LED: GPIO 14 (Warnings)
- Green LED: GPIO 27 (Normal)

Don't forget to use appropriate resistors!

## Setup Instructions

### 1. Install Required Software

```bash
# Install Node-RED
npm install -g node-red

# Install required Node-RED packages
npm install -g node-red-dashboard
```

### 2. Arduino Setup

1. Open Arduino IDE
2. Install required libraries:
   - Tools > Manage Libraries
   - Search and install:
     - PubSubClient
     - ArduinoJson
     - NTPClient

### 3. ESP32 Configuration

1. Open `esp32_code/glucose_sender.ino`
2. Update WiFi credentials:
   ```cpp
   const char* ssid = "YOUR_WIFI_SSID";
   const char* password = "YOUR_WIFI_PASSWORD";
   ```

### 4. Node-RED Setup

1. Start Node-RED:
   ```bash
   node-red
   ```

2. Open Node-RED dashboard:
   - Go to `http://localhost:1880`

3. Import the flow:
   - Menu > Import
   - Select `node-red-flow.json`
   - Click Import

4. Deploy the flow

5. View the dashboard:
   - Go to `http://localhost:1880/ui`

## Alert Thresholds

- Critical Low: < 70 mg/dL (Red LED)
- Low Warning: < 80 mg/dL (Yellow LED)
- Normal: 80-150 mg/dL (Green LED)
- High Warning: > 150 mg/dL (Yellow LED)
- Critical High: > 180 mg/dL (Red LED)

## Data Storage

Glucose readings are automatically saved to `glucose_data.csv` with:
- Timestamp
- Glucose value
- Device ID

## Accessing Globally

The dashboard is accessible from anywhere using the MQTT broker:
- Broker: broker.hivemq.com
- Port: 1883
- Topic: glucose_monitor/data

## Troubleshooting

1. If LEDs don't work:
   - Check wiring
   - Verify GPIO pins
   - Check resistor values

2. If MQTT connection fails:
   - Verify WiFi connection
   - Check broker settings
   - Ensure unique client ID

3. If time is incorrect:
   - Check NTP server connection
   - Verify timezone offset

## Contributing

Feel free to submit issues and enhancement requests!
