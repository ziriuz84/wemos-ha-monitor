/**
 * Configuration file for Wemos D1 R2 - Home Assistant Monitor
 * 
 * Edit the following values according to your configuration:
 * - WiFi credentials
 * - Home Assistant URL and token
 * - List of sensors to monitor
 */

#ifndef CONFIG_H
#define CONFIG_H

// ============================================
// WIFI CONFIGURATION
// ============================================
// WiFi network name (SSID)
#define WIFI_SSID "NegriPominiMesh"

// WiFi network password
#define WIFI_PASSWORD "ohqda14sisfxrqg"

// ============================================
// HOME ASSISTANT CONFIGURATION
// ============================================
/**
 * Home Assistant base URL (without trailing slash)
 * 
 * IMPORTANT: If using a public domain with reverse proxy,
 * ensure the URL correctly points to the Home Assistant API.
 * 
 * Examples:
 * - Local HTTP: "http://192.168.1.100:8123"
 * - Local HTTPS: "https://192.168.1.100:8123"
 * - Local hostname: "http://homeassistant.local:8123"
 * - Public domain (standard port 443): "https://homeassistant.example.com"
 * - Public domain (custom port): "https://homeassistant.example.com:8123"
 * 
 * Note: For HTTPS you may need to configure
 *       SSL certificates (see WiFiClientSecure in code)
 * 
 * Note: If you receive 400 errors, verify that:
 *       1. The URL is correct and reachable
 *       2. The port is specified if different from 80/443
 *       3. The reverse proxy (if present) allows API access
 */
#define HA_BASE_URL "https://asdrubale.sirionegri.it"

/**
 * Home Assistant Long-Lived Access Token
 * 
 * How to generate:
 * 1. Open Home Assistant in browser
 * 2. Click on your user profile (bottom left)
 * 3. Scroll to "Access Token" section
 * 4. Click "Create Token"
 * 5. Assign a descriptive name (e.g. "Wemos Monitor")
 * 6. Copy the generated token and paste it below
 * 
 * IMPORTANT: Never share this token!
 */
#define HA_ACCESS_TOKEN "eyJhbGciOiJIUzI1NiIsInR5cCI6IkpXVCJ9.eyJpc3MiOiJhYjQxNWIxOTkwYmU0ZDQ1YTllOGVhMTNkOTY4N2UwNCIsImlhdCI6MTc2ODQzMDk0OCwiZXhwIjoyMDgzNzkwOTQ4fQ.TNd6R9yP4Sr5CKuUF0Xq7wxWsSVbEhdbRmQn5hyNmvY"

/**
 * Sensor reading interval (in seconds)
 * 
 * Determines how long to wait between one complete reading
 * of all sensors and the next.
 * 
 * Recommended values:
 * - 30-60 seconds for slowly changing sensors (temperature, humidity)
 * - 10-30 seconds for more dynamic sensors
 * - Avoid values too low (< 5s) to avoid overloading the server
 */
#define READ_INTERVAL 30

// ============================================
// SENSOR CONFIGURATION
// ============================================
/**
 * Data structure to configure a sensor
 * 
 * @param label     Label to display on serial (e.g. "Temperature")
 * @param entityId  Complete entity ID of the sensor in Home Assistant
 *                  (e.g. "sensor.temperature_living_room")
 */
struct SensorConfig {
  const char* label;      // Label to display on serial
  const char* entityId;   // Entity ID of the sensor in Home Assistant
};

/**
 * Number of sensors to monitor
 * 
 * IMPORTANT: This value MUST match the number
 *            of elements in the sensorConfigs[] array
 */
#define NUM_SENSORS 3

/**
 * Sensor configuration array
 * 
 * Edit this array to add, remove or modify
 * the sensors to monitor.
 * 
 * Format: {"Label", "entity_id"}
 * 
 * Entity ID examples:
 * - sensor.temperature_living_room
 * - sensor.humidity_bedroom
 * - binary_sensor.motion_entrance
 * - climate.thermostat
 * 
 * To find a sensor's entity ID:
 * 1. Go to Settings > Devices & Services
 * 2. Search for your sensor/device
 * 3. Click on the sensor to see details
 * 4. The entity ID is visible in the details
 */
SensorConfig sensorConfigs[NUM_SENSORS] = {
  {"Grid", "sensor.foxess_grid_consumption_power"},
  {"Solar", "sensor.solaredge_current_power"},
  {"Load", "sensor.foxess_load_power"}
};

// ============================================
// COMMON ENTITY ID EXAMPLES
// ============================================
// Replace examples in the array above with these formats:
//
// Sensors:
//   sensor.temperature_living_room
//   sensor.humidity_living_room
//   sensor.atmospheric_pressure
//   sensor.energy_consumption
//   sensor.brightness
//   sensor.air_quality
//
// Binary Sensors (on/off sensors):
//   binary_sensor.motion_living_room
//   binary_sensor.door_entrance
//   binary_sensor.smoke_detected
//
// Devices:
//   climate.thermostat
//   light.living_room_light
//   switch.switch
//   cover.bedroom_blind
// ============================================

#endif // CONFIG_H
