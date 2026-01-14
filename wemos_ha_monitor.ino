/**
 * Wemos D1 R2 - Home Assistant Monitor
 * 
 * Sketch per monitorare sensori Home Assistant tramite REST API.
 * Legge i valori dei sensori configurati e li stampa sulla seriale.
 * 
 * Requisiti:
 * - ESP8266WiFi (inclusa con ESP8266 Board Support)
 * - ESP8266HTTPClient (inclusa con ESP8266 Board Support)
 * - ArduinoJson v6.x
 * 
 * Configurazione: modificare config.h con le proprie credenziali
 */

#include <ESP8266WiFi.h>
#include <ESP8266HTTPClient.h>
#include <WiFiClientSecure.h>
#include <ArduinoJson.h>
#include "config.h"

// Oggetti globali per connessione WiFi e HTTP
WiFiClient client;           // Client TCP per connessioni HTTP
WiFiClientSecure clientSecure;  // Client TCP per connessioni HTTPS
HTTPClient http;             // Client HTTP per richieste REST API

// Flag per determinare se usare HTTPS
bool useHTTPS = false;

/**
 * Inizializzazione hardware e connessione WiFi
 * Eseguita una sola volta all'avvio del dispositivo
 */
void setup() {
  // Inizializzazione seriale per debug (115200 baud)
  Serial.begin(115200);
  delay(1000);  // Attesa stabilizzazione seriale
  
  Serial.println("\n=== Wemos D1 R2 - Home Assistant Monitor ===");
  
  // Configurazione modalità WiFi: Station Mode (si connette a una rete)
  WiFi.mode(WIFI_STA);
  WiFi.begin(WIFI_SSID, WIFI_PASSWORD);
  
  // Tentativo di connessione con timeout
  Serial.print("Connessione a WiFi");
  int attempts = 0;
  const int MAX_ATTEMPTS = 30;  // Massimo 15 secondi (30 * 500ms)
  
  while (WiFi.status() != WL_CONNECTED && attempts < MAX_ATTEMPTS) {
    delay(500);
    Serial.print(".");
    attempts++;
  }
  
  // Verifica esito connessione
  if (WiFi.status() == WL_CONNECTED) {
    Serial.println("\nWiFi connesso!");
    Serial.print("IP address: ");
    Serial.println(WiFi.localIP());
  } else {
    Serial.println("\nErrore connessione WiFi!");
    // In caso di errore, il loop() tenterà la riconnessione
    return;
  }
  
  // Configurazione HTTPS se necessario
  String baseUrl = String(HA_BASE_URL);
  if (baseUrl.startsWith("https://")) {
    useHTTPS = true;
    Serial.println("Configurazione HTTPS...");
    
    // Per sviluppo: disabilita verifica certificato (NON usare in produzione!)
    // In produzione, dovresti caricare il certificato del server
    clientSecure.setInsecure();  // Disabilita verifica certificato SSL
    
    // Alternativa per produzione (richiede certificato):
    // clientSecure.setCACert(certificate);  // Carica il certificato CA
    
    Serial.println("HTTPS configurato (verifica certificato disabilitata)");
  } else {
    useHTTPS = false;
    Serial.println("Usando HTTP");
  }
  
  delay(1000);  // Pausa prima di iniziare le letture
}

/**
 * Loop principale: eseguito continuamente dopo setup()
 * Legge periodicamente i sensori configurati da Home Assistant
 */
void loop() {
  // Verifica connessione WiFi e riconnessione automatica se necessario
  if (WiFi.status() != WL_CONNECTED) {
    Serial.println("WiFi disconnesso, riconnessione...");
    WiFi.begin(WIFI_SSID, WIFI_PASSWORD);
    delay(5000);  // Attesa prima di riprovare
    return;  // Esce dal loop e riprova al prossimo ciclo
  }
  
  // Lettura di tutti i sensori configurati
  Serial.println("\n--- Lettura sensori Home Assistant ---");
  
  for (int i = 0; i < NUM_SENSORS; i++) {
    readSensor(sensorConfigs[i]);
    delay(500);  // Pausa tra le richieste per evitare sovraccarico server
  }
  
  Serial.println("--- Fine lettura sensori ---\n");
  
  // Attesa prima della prossima lettura (intervallo configurabile)
  delay(READ_INTERVAL * 1000);
}

/**
 * Legge lo stato di un sensore da Home Assistant tramite REST API
 * 
 * @param sensor Configurazione del sensore (label ed entity ID)
 * 
 * Utilizza l'endpoint /api/states/{entity_id} di Home Assistant
 * per ottenere lo stato corrente del sensore specificato.
 */
void readSensor(SensorConfig sensor) {
  // Costruzione URL API Home Assistant
  // Formato: http://host:port/api/states/{entity_id} oppure https://host/api/states/{entity_id}
  String url = String(HA_BASE_URL);
  
  // Assicurati che l'URL non abbia trailing slash
  if (url.endsWith("/")) {
    url.remove(url.length() - 1);
  }
  
  url += "/api/states/";
  url += String(sensor.entityId);
  
  // Debug: stampa URL (commenta in produzione se contiene informazioni sensibili)
  Serial.print("URL: ");
  Serial.println(url);
  
  // Inizializzazione richiesta HTTP/HTTPS
  if (useHTTPS) {
    http.begin(clientSecure, url);
  } else {
    http.begin(client, url);
  }
  
  // Configurazione timeout
  http.setTimeout(10000);  // 10 secondi
  
  // Aggiunta header di autenticazione (Long-Lived Access Token)
  http.addHeader("Authorization", "Bearer " + String(HA_ACCESS_TOKEN));
  http.addHeader("Content-Type", "application/json");
  
  // Debug: verifica header
  Serial.print("Header Authorization: Bearer ");
  Serial.println(String(HA_ACCESS_TOKEN).substring(0, 20) + "...");  // Mostra solo primi 20 caratteri
  
  // Esecuzione richiesta GET
  int httpCode = http.GET();
  
  // Gestione risposta HTTP
  if (httpCode > 0) {
    // Richiesta completata (codice positivo = successo o errore HTTP)
    if (httpCode == HTTP_CODE_OK) {
      // Richiesta riuscita (200 OK)
      String payload = http.getString();
      
      // Debug: mostra payload (commenta se troppo lungo)
      // Serial.print("Payload: ");
      // Serial.println(payload);
      
      // Parsing della risposta JSON
      // Buffer di 1024 byte per il documento JSON (adattare se necessario)
      DynamicJsonDocument doc(1024);
      DeserializationError error = deserializeJson(doc, payload);
      
      if (!error) {
        // Parsing riuscito: estrazione dati
        const char* state = doc["state"];  // Valore dello stato
        const char* unit = doc["attributes"]["unit_of_measurement"];  // Unità di misura (opzionale)
        
        // Stampa formattata: Label: valore [unità]
        Serial.print(sensor.label);
        Serial.print(": ");
        Serial.print(state);
        
        // Aggiunta unità di misura se presente
        if (unit) {
          Serial.print(" ");
          Serial.print(unit);
        }
        Serial.println();
      } else {
        // Errore durante il parsing JSON
        Serial.print(sensor.label);
        Serial.print(": Errore parsing JSON - ");
        Serial.println(error.c_str());
        Serial.print("Payload ricevuto: ");
        Serial.println(payload);
      }
    } else {
      // Errore HTTP (es. 400 Bad Request, 404 Not Found, 401 Unauthorized, ecc.)
      String payload = http.getString();  // Leggi il payload anche in caso di errore
      
      Serial.print(sensor.label);
      Serial.print(": Errore HTTP ");
      Serial.println(httpCode);
      
      // Stampa dettagli errore se disponibili
      if (payload.length() > 0) {
        Serial.print("Dettagli errore: ");
        Serial.println(payload);
      }
      
      // Messaggi di aiuto per errori comuni
      if (httpCode == 400) {
        Serial.println("  Suggerimento: Verifica che l'entity ID sia corretto");
        Serial.println("  e che l'URL sia formattato correttamente");
      } else if (httpCode == 401) {
        Serial.println("  Suggerimento: Verifica che il token di accesso sia valido");
      } else if (httpCode == 404) {
        Serial.print("  Suggerimento: Entity ID '");
        Serial.print(sensor.entityId);
        Serial.println("' non trovato in Home Assistant");
      }
    }
  } else {
    // Errore di connessione (codice negativo)
    // Possibili cause: timeout, DNS non risolto, server non raggiungibile
    Serial.print(sensor.label);
    Serial.print(": Errore connessione (codice: ");
    Serial.print(httpCode);
    Serial.println(")");
    
    // Messaggi di aiuto per errori comuni
    if (httpCode == HTTPC_ERROR_CONNECTION_REFUSED) {
      Serial.println("  Server non raggiungibile o porta errata");
    } else if (httpCode == HTTPC_ERROR_CONNECTION_LOST) {
      Serial.println("  Connessione persa durante la richiesta");
    } else if (httpCode == HTTPC_ERROR_SEND_PAYLOAD_FAILED) {
      Serial.println("  Errore durante l'invio della richiesta");
    } else if (httpCode == HTTPC_ERROR_NO_STREAM) {
      Serial.println("  Errore nello stream di risposta");
    }
  }
  
  // Chiusura connessione HTTP
  http.end();
}
