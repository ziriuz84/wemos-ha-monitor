/**
 * Wemos D1 R2 - Home Assistant Monitor
 * 
 * Sketch to monitor Home Assistant sensors via REST API.
 * Reads configured sensor values and prints them to serial.
 * 
 * Requirements:
 * - ESP8266WiFi (included with ESP8266 Board Support)
 * - ESP8266HTTPClient (included with ESP8266 Board Support)
 * - ArduinoJson v6.x
 * 
 * Configuration: edit config.h with your credentials
 */

#include <ESP8266WiFi.h>
#include <ESP8266HTTPClient.h>
#include <WiFiClientSecure.h>
#include <ArduinoJson.h>
#include "config.h"

// Global objects for WiFi and HTTP connection
WiFiClient client;           // TCP client for HTTP connections
WiFiClientSecure clientSecure;  // TCP client for HTTPS connections
HTTPClient http;             // HTTP client for REST API requests

// Flag to determine if HTTPS should be used
bool useHTTPS = false;

/**
 * Hardware initialization and WiFi connection
 * Executed once at device startup
 */
void setup() {
  // Serial initialization for debugging (115200 baud)
  Serial.begin(115200);
  delay(1000);  // Wait for serial stabilization
  
  Serial.println("\n=== Wemos D1 R2 - Home Assistant Monitor ===");
  
  // WiFi mode configuration: Station Mode (connects to a network)
  WiFi.mode(WIFI_STA);
  WiFi.begin(WIFI_SSID, WIFI_PASSWORD);
  
  // Connection attempt with timeout
  Serial.print("Connecting to WiFi");
  int attempts = 0;
  const int MAX_ATTEMPTS = 30;  // Maximum 15 seconds (30 * 500ms)
  
  while (WiFi.status() != WL_CONNECTED && attempts < MAX_ATTEMPTS) {
    delay(500);
    Serial.print(".");
    attempts++;
  }
  
  // Check connection result
  if (WiFi.status() == WL_CONNECTED) {
    Serial.println("\nWiFi connected!");
    Serial.print("IP address: ");
    Serial.println(WiFi.localIP());
  } else {
    Serial.println("\nWiFi connection error!");
    // In case of error, loop() will attempt reconnection
    return;
  }
  
  // HTTPS configuration if necessary
  String baseUrl = String(HA_BASE_URL);
  if (baseUrl.startsWith("https://")) {
    useHTTPS = true;
    Serial.println("Configuring HTTPS...");
    
    // For development: disable certificate verification (DO NOT use in production!)
    // In production, you should load the server certificate
    clientSecure.setInsecure();  // Disable SSL certificate verification
    
    // Alternative for production (requires certificate):
    // clientSecure.setCACert(certificate);  // Load CA certificate
    
    Serial.println("HTTPS configured (certificate verification disabled)");
  } else {
    useHTTPS = false;
    Serial.println("Using HTTP");
  }
  
  delay(1000);  // Pause before starting readings
}

/**
 * Main loop: executed continuously after setup()
 * Periodically reads configured sensors from Home Assistant
 */
void loop() {
  // Check WiFi connection and automatic reconnection if necessary
  if (WiFi.status() != WL_CONNECTED) {
    Serial.println("WiFi disconnected, reconnecting...");
    WiFi.begin(WIFI_SSID, WIFI_PASSWORD);
    delay(5000);  // Wait before retrying
    return;  // Exit loop and retry on next cycle
  }
  
  // Read all configured sensors
  Serial.println("\n--- Reading Home Assistant sensors ---");
  
  for (int i = 0; i < NUM_SENSORS; i++) {
    readSensor(sensorConfigs[i]);
    delay(500);  // Pause between requests to avoid server overload
  }
  
  Serial.println("--- End of sensor reading ---\n");
  
  // Wait before next reading (configurable interval)
  delay(READ_INTERVAL * 1000);
}

/**
 * Reads the state of a sensor from Home Assistant via REST API
 * 
 * @param sensor Sensor configuration (label and entity ID)
 * 
 * Uses the /api/states/{entity_id} endpoint of Home Assistant
 * to get the current state of the specified sensor.
 */
void readSensor(SensorConfig sensor) {
  // Build Home Assistant API URL
  // Format: http://host:port/api/states/{entity_id} or https://host/api/states/{entity_id}
  String url = String(HA_BASE_URL);
  
  // Ensure URL doesn't have trailing slash
  if (url.endsWith("/")) {
    url.remove(url.length() - 1);
  }
  
  url += "/api/states/";
  url += String(sensor.entityId);
  
  // Debug: print URL (comment in production if it contains sensitive information)
  Serial.print("URL: ");
  Serial.println(url);
  
  // Initialize HTTP/HTTPS request
  if (useHTTPS) {
    http.begin(clientSecure, url);
  } else {
    http.begin(client, url);
  }
  
  // Timeout configuration
  http.setTimeout(10000);  // 10 seconds
  
  // Add authentication header (Long-Lived Access Token)
  http.addHeader("Authorization", "Bearer " + String(HA_ACCESS_TOKEN));
  http.addHeader("Content-Type", "application/json");
  
  // Debug: verify header
  Serial.print("Header Authorization: Bearer ");
  Serial.println(String(HA_ACCESS_TOKEN).substring(0, 20) + "...");  // Show only first 20 characters
  
  // Execute GET request
  int httpCode = http.GET();
  
  // Handle HTTP response
  if (httpCode > 0) {
    // Request completed (positive code = success or HTTP error)
    if (httpCode == HTTP_CODE_OK) {
      // Request successful (200 OK)
      String payload = http.getString();
      
      // Debug: show payload (comment if too long)
      // Serial.print("Payload: ");
      // Serial.println(payload);
      
      // Parse JSON response
      // 1024 byte buffer for JSON document (adjust if necessary)
      DynamicJsonDocument doc(1024);
      DeserializationError error = deserializeJson(doc, payload);
      
      if (!error) {
        // Parsing successful: extract data
        const char* state = doc["state"];  // State value
        const char* unit = doc["attributes"]["unit_of_measurement"];  // Unit of measurement (optional)
        
        // Formatted output: Label: value [unit]
        Serial.print(sensor.label);
        Serial.print(": ");
        Serial.print(state);
        
        // Add unit of measurement if present
        if (unit) {
          Serial.print(" ");
          Serial.print(unit);
        }
        Serial.println();
      } else {
        // Error during JSON parsing
        Serial.print(sensor.label);
        Serial.print(": JSON parsing error - ");
        Serial.println(error.c_str());
        Serial.print("Received payload: ");
        Serial.println(payload);
      }
    } else {
      // HTTP error (e.g. 400 Bad Request, 404 Not Found, 401 Unauthorized, etc.)
      String payload = http.getString();  // Read payload even on error
      
      Serial.print(sensor.label);
      Serial.print(": HTTP error ");
      Serial.println(httpCode);
      
      // Print error details if available
      if (payload.length() > 0) {
        Serial.print("Error details: ");
        Serial.println(payload);
      }
      
      // Help messages for common errors
      if (httpCode == 400) {
        Serial.println("  Hint: Verify that the entity ID is correct");
        Serial.println("  and that the URL is properly formatted");
      } else if (httpCode == 401) {
        Serial.println("  Hint: Verify that the access token is valid");
      } else if (httpCode == 404) {
        Serial.print("  Hint: Entity ID '");
        Serial.print(sensor.entityId);
        Serial.println("' not found in Home Assistant");
      }
    }
  } else {
    // Connection error (negative code)
    // Possible causes: timeout, DNS not resolved, server unreachable
    Serial.print(sensor.label);
    Serial.print(": Connection error (code: ");
    Serial.print(httpCode);
    Serial.println(")");
    
    // Help messages for common errors
    if (httpCode == HTTPC_ERROR_CONNECTION_REFUSED) {
      Serial.println("  Server unreachable or wrong port");
    } else if (httpCode == HTTPC_ERROR_CONNECTION_LOST) {
      Serial.println("  Connection lost during request");
    } else if (httpCode == HTTPC_ERROR_SEND_PAYLOAD_FAILED) {
      Serial.println("  Error sending request");
    } else if (httpCode == HTTPC_ERROR_NO_STREAM) {
      Serial.println("  Response stream error");
    }
  }
  
  // Close HTTP connection
  http.end();
}
