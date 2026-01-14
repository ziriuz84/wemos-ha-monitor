# Wemos D1 R2 - Home Assistant Monitor

Arduino sketch to monitor Home Assistant sensors via serial output.

## Requirements

### Hardware
- Wemos D1 R2 (ESP8266)

### Software
- **arduino-cli** (command-line tool for Arduino)
  - Installation on Arch Linux: `sudo pacman -S arduino-cli`
  - Installation on other distributions: [arduino-cli install guide](https://arduino.github.io/arduino-cli/latest/installation/)

### Arduino Libraries
Libraries are automatically installed by the scripts. Manual installation:
- **ESP8266WiFi** (included with ESP8266 Board Support)
- **ESP8266HTTPClient** (included with ESP8266 Board Support)
- **ArduinoJson** (version 6.x) - by Benoit Blanchon

## Configuration

### 1. WiFi Configuration
Edit `config.h`:
```cpp
#define WIFI_SSID "your_wifi_network_name"
#define WIFI_PASSWORD "your_wifi_password"
```

### 2. Home Assistant Configuration
Edit `config.h`:
```cpp
#define HA_BASE_URL "http://192.168.1.100:8123"
#define HA_ACCESS_TOKEN "your_long_lived_access_token"
```

**How to get the Long-Lived Access Token:**
1. Open Home Assistant
2. Go to **User Profile** (bottom left)
3. Scroll to **Access Token**
4. Click **Create Token**
5. Copy the generated token and paste it in `config.h`

### 3. Sensor Configuration
Edit the `sensorConfigs` array in `config.h`:

```cpp
#define NUM_SENSORS 3  // Update the number of sensors

SensorConfig sensorConfigs[NUM_SENSORS] = {
  {"Temperature", "sensor.temperature_living_room"},
  {"Humidity", "sensor.humidity_living_room"},
  {"Pressure", "sensor.atmospheric_pressure"}
};
```

**Format:**
- First parameter: label to display on serial
- Second parameter: Entity ID of the sensor in Home Assistant

**How to find the Entity ID:**
1. Go to **Settings** > **Devices & Services**
2. Search for your sensor
3. The Entity ID is visible in the sensor details

### 4. Reading Interval
Edit in `config.h`:
```cpp
#define READ_INTERVAL 30  // seconds between readings
```

## Compilation and Upload

### Method 1: Automatic Script (Recommended)

The `upload.sh` script automatically handles:
- arduino-cli installation verification
- ESP8266 board configuration
- Required library installation
- Sketch compilation
- Device upload

**Usage:**
```bash
# Auto-detect serial port
./upload.sh

# Specify serial port manually
./upload.sh /dev/ttyUSB0
```

**Compilation only (no upload):**
```bash
./compile.sh
```

### Method 2: Arduino IDE

1. Select board: **Tools** > **Board** > **LOLIN(WEMOS) D1 R2 & mini**
2. Select the correct serial port
3. Upload the sketch

### Serial Monitor

To see the sketch output:
```bash
arduino-cli monitor -p /dev/ttyUSB0 -c baudrate=115200
```

Or use the Arduino IDE serial monitor (115200 baud).

## Serial Output

The sketch prints sensor values to serial (115200 baud):

```
=== Wemos D1 R2 - Home Assistant Monitor ===
Connecting to WiFi...
WiFi connected!
IP address: 192.168.1.50

--- Reading Home Assistant sensors ---
Temperature: 22.5 °C
Humidity: 45.0 %
Pressure: 1013.25 hPa
--- End of sensor reading ---
```

## Notes

- The sketch supports both HTTP and HTTPS (uses `WiFiClientSecure` for HTTPS)
- If using HTTPS, you may need to handle SSL certificates
- Reading interval is configurable via `READ_INTERVAL`
- The device automatically reconnects if WiFi disconnects
