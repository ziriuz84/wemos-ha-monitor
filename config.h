/**
 * File di configurazione per Wemos D1 R2 - Home Assistant Monitor
 * 
 * Modificare i valori seguenti secondo la propria configurazione:
 * - Credenziali WiFi
 * - URL e token di Home Assistant
 * - Lista dei sensori da monitorare
 */

#ifndef CONFIG_H
#define CONFIG_H

// ============================================
// CONFIGURAZIONE WIFI
// ============================================
// Nome della rete WiFi (SSID)
#define WIFI_SSID "NegriPominiMesh"

// Password della rete WiFi
#define WIFI_PASSWORD "ohqda14sisfxrqg"

// ============================================
// CONFIGURAZIONE HOME ASSISTANT
// ============================================
/**
 * URL base di Home Assistant (senza trailing slash)
 * 
 * IMPORTANTE: Se usi un dominio pubblico con reverse proxy,
 * assicurati che l'URL punti correttamente all'API di Home Assistant.
 * 
 * Esempi:
 * - HTTP locale: "http://192.168.1.100:8123"
 * - HTTPS locale: "https://192.168.1.100:8123"
 * - Hostname locale: "http://homeassistant.local:8123"
 * - Dominio pubblico (porta standard 443): "https://homeassistant.example.com"
 * - Dominio pubblico (porta custom): "https://homeassistant.example.com:8123"
 * 
 * Nota: Per HTTPS potrebbe essere necessario configurare
 *       i certificati SSL (vedi WiFiClientSecure nel codice)
 * 
 * Nota: Se ricevi errori 400, verifica che:
 *       1. L'URL sia corretto e raggiungibile
 *       2. La porta sia specificata se diversa da 80/443
 *       3. Il reverse proxy (se presente) permetta l'accesso all'API
 */
#define HA_BASE_URL "https://asdrubale.sirionegri.it"

/**
 * Long-Lived Access Token di Home Assistant
 * 
 * Come generare:
 * 1. Apri Home Assistant nel browser
 * 2. Clicca sul tuo profilo utente (in basso a sinistra)
 * 3. Scorri fino alla sezione "Access Token"
 * 4. Clicca su "Crea Token"
 * 5. Assegna un nome descrittivo (es. "Wemos Monitor")
 * 6. Copia il token generato e incollalo qui sotto
 * 
 * IMPORTANTE: Non condividere mai questo token!
 */
#define HA_ACCESS_TOKEN "eyJhbGciOiJIUzI1NiIsInR5cCI6IkpXVCJ9.eyJpc3MiOiJhYjQxNWIxOTkwYmU0ZDQ1YTllOGVhMTNkOTY4N2UwNCIsImlhdCI6MTc2ODQzMDk0OCwiZXhwIjoyMDgzNzkwOTQ4fQ.TNd6R9yP4Sr5CKuUF0Xq7wxWsSVbEhdbRmQn5hyNmvY"

/**
 * Intervallo di lettura sensori (in secondi)
 * 
 * Determina quanto tempo attendere tra una lettura completa
 * di tutti i sensori e la successiva.
 * 
 * Valori consigliati:
 * - 30-60 secondi per sensori che cambiano lentamente (temperatura, umidità)
 * - 10-30 secondi per sensori più dinamici
 * - Evitare valori troppo bassi (< 5s) per non sovraccaricare il server
 */
#define READ_INTERVAL 30

// ============================================
// CONFIGURAZIONE SENSORI
// ============================================
/**
 * Struttura dati per configurare un sensore
 * 
 * @param label     Etichetta da mostrare sulla seriale (es. "Temperatura")
 * @param entityId  Entity ID completo del sensore in Home Assistant
 *                  (es. "sensor.temperatura_soggiorno")
 */
struct SensorConfig {
  const char* label;      // Etichetta da mostrare sulla seriale
  const char* entityId;   // Entity ID del sensore in Home Assistant
};

/**
 * Numero di sensori da monitorare
 * 
 * IMPORTANTE: Questo valore DEVE corrispondere al numero
 *             di elementi nell'array sensorConfigs[]
 */
#define NUM_SENSORS 3

/**
 * Array di configurazione sensori
 * 
 * Modificare questo array per aggiungere, rimuovere o modificare
 * i sensori da monitorare.
 * 
 * Formato: {"Label", "entity_id"}
 * 
 * Esempi di entity ID:
 * - sensor.temperatura_soggiorno
 * - sensor.umidita_camera
 * - binary_sensor.movimento_ingresso
 * - climate.termostato
 * 
 * Per trovare l'entity ID di un sensore:
 * 1. Vai su Impostazioni > Dispositivi e servizi
 * 2. Cerca il tuo sensore/dispositivo
 * 3. Clicca sul sensore per vedere i dettagli
 * 4. L'entity ID è visibile nei dettagli
 */
SensorConfig sensorConfigs[NUM_SENSORS] = {
  {"Grid", "sensor.foxess_grid_consumption_power"},
  {"Solare", "sensor.solaredge_current_power"},
  {"Carico", "sensor.foxess_load_power"}
};

// ============================================
// ESEMPI DI ENTITY ID COMUNI
// ============================================
// Sostituire gli esempi nell'array sopra con questi formati:
//
// Sensori:
//   sensor.temperatura_soggiorno
//   sensor.umidita_soggiorno
//   sensor.pressione_atmosferica
//   sensor.consumo_energia
//   sensor.luminosita
//   sensor.qualita_aria
//
// Binary Sensors (sensori on/off):
//   binary_sensor.movimento_soggiorno
//   binary_sensor.porta_ingresso
//   binary_sensor.fumo_rilevato
//
// Dispositivi:
//   climate.termostato
//   light.luce_soggiorno
//   switch.interruttore
//   cover.tapparella_camera
// ============================================

#endif // CONFIG_H
