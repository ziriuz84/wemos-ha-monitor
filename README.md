# Wemos D1 R2 - Home Assistant Monitor

Sketch Arduino per monitorare sensori Home Assistant tramite seriale.

## Requisiti

### Hardware
- Wemos D1 R2 (ESP8266)

### Software
- **arduino-cli** (strumento da linea di comando per Arduino)
  - Installazione su Arch Linux: `sudo pacman -S arduino-cli`
  - Installazione su altre distribuzioni: [arduino-cli install guide](https://arduino.github.io/arduino-cli/latest/installation/)

### Librerie Arduino
Le librerie vengono installate automaticamente dagli script. Manualmente:
- **ESP8266WiFi** (inclusa con ESP8266 Board Support)
- **ESP8266HTTPClient** (inclusa con ESP8266 Board Support)
- **ArduinoJson** (versione 6.x) - di Benoit Blanchon

## Configurazione

### 1. Configurazione WiFi
Modifica in `config.h`:
```cpp
#define WIFI_SSID "nome_rete_wifi"
#define WIFI_PASSWORD "password_wifi"
```

### 2. Configurazione Home Assistant
Modifica in `config.h`:
```cpp
#define HA_BASE_URL "http://192.168.1.100:8123"
#define HA_ACCESS_TOKEN "il_tuo_long_lived_access_token"
```

**Come ottenere il Long-Lived Access Token:**
1. Apri Home Assistant
2. Vai su **Profilo Utente** (in basso a sinistra)
3. Scorri fino a **Access Token**
4. Clicca su **Crea Token**
5. Copia il token generato e incollalo in `config.h`

### 3. Configurazione Sensori
Modifica l'array `sensorConfigs` in `config.h`:

```cpp
#define NUM_SENSORS 3  // Aggiorna il numero di sensori

SensorConfig sensorConfigs[NUM_SENSORS] = {
  {"Temperatura", "sensor.temperatura_soggiorno"},
  {"Umidità", "sensor.umidita_soggiorno"},
  {"Pressione", "sensor.pressione_atmosferica"}
};
```

**Formato:**
- Primo parametro: etichetta da mostrare sulla seriale
- Secondo parametro: Entity ID del sensore in Home Assistant

**Come trovare l'Entity ID:**
1. Vai su **Impostazioni** > **Dispositivi e servizi**
2. Cerca il tuo sensore
3. L'Entity ID è visibile nei dettagli del sensore

### 4. Intervallo di lettura
Modifica in `config.h`:
```cpp
#define READ_INTERVAL 30  // secondi tra una lettura e l'altra
```

## Compilazione e Upload

### Metodo 1: Script automatico (consigliato)

Lo script `upload.sh` gestisce automaticamente:
- Verifica installazione arduino-cli
- Configurazione board ESP8266
- Installazione librerie necessarie
- Compilazione dello sketch
- Caricamento sul dispositivo

**Uso:**
```bash
# Auto-rilevamento porta seriale
./upload.sh

# Specifica porta seriale manualmente
./upload.sh /dev/ttyUSB0
```

**Solo compilazione (senza upload):**
```bash
./compile.sh
```

### Metodo 2: Arduino IDE

1. Seleziona la scheda: **Tools** > **Board** > **LOLIN(WEMOS) D1 R2 & mini**
2. Seleziona la porta seriale corretta
3. Carica lo sketch

### Monitor seriale

Per vedere l'output dello sketch:
```bash
arduino-cli monitor -p /dev/ttyUSB0 -c baudrate=115200
```

Oppure usa il monitor seriale dell'IDE Arduino (115200 baud).

## Output Serial

Lo sketch stampa sulla seriale (115200 baud) i valori dei sensori:

```
=== Wemos D1 R2 - Home Assistant Monitor ===
Connessione a WiFi...
WiFi connesso!
IP address: 192.168.1.50

--- Lettura sensori Home Assistant ---
Temperatura: 22.5 °C
Umidità: 45.0 %
Pressione: 1013.25 hPa
--- Fine lettura sensori ---
```

## Note

- Lo sketch supporta sia HTTP che HTTPS (per HTTPS usa `WiFiClientSecure`)
- Se usi HTTPS, potresti dover gestire i certificati SSL
- L'intervallo di lettura è configurabile tramite `READ_INTERVAL`
- Il dispositivo si riconnette automaticamente se il WiFi si disconnette
