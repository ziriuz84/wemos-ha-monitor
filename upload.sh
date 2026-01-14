#!/bin/bash

###############################################################################
# Script di compilazione e caricamento per Wemos D1 R2 - Home Assistant Monitor
#
# Questo script:
# 1. Verifica l'installazione di arduino-cli
# 2. Configura la board Wemos D1 R2
# 3. Installa le librerie necessarie
# 4. Compila lo sketch
# 5. Carica lo sketch sul dispositivo
#
# Uso: ./upload.sh [porta_seriale]
# Esempio: ./upload.sh /dev/ttyUSB0
###############################################################################

# Colori per output
RED='\033[0;31m'
GREEN='\033[0;32m'
YELLOW='\033[1;33m'
BLUE='\033[0;34m'
NC='\033[0m' # No Color

# Configurazione
SKETCH_NAME="wemos_ha_monitor"
SKETCH_DIR="$(dirname "$(readlink -f "$0")")"
BOARD_FQBN="esp8266:esp8266:d1_mini"  # Wemos D1 R2 usa il core d1_mini
ARDUINO_CLI="arduino-cli"

# Porta seriale (può essere passata come argomento o auto-rilevata)
SERIAL_PORT="${1:-}"

###############################################################################
# Funzioni di utilità
###############################################################################

# Stampa messaggio di errore e termina
error_exit() {
    echo -e "${RED}❌ Errore: $1${NC}" >&2
    exit 1
}

# Stampa messaggio di successo
success_msg() {
    echo -e "${GREEN}✅ $1${NC}"
}

# Stampa messaggio informativo
info_msg() {
    echo -e "${BLUE}ℹ️  $1${NC}"
}

# Stampa messaggio di avviso
warning_msg() {
    echo -e "${YELLOW}⚠️  $1${NC}"
}

###############################################################################
# Verifica prerequisiti
###############################################################################

check_arduino_cli() {
    info_msg "Verifica installazione arduino-cli..."
    
    if ! command -v "$ARDUINO_CLI" &> /dev/null; then
        error_exit "arduino-cli non trovato!\n\n" \
                   "Installa arduino-cli:\n" \
                   "  curl -fsSL https://raw.githubusercontent.com/arduino/arduino-cli/master/install.sh | sh\n" \
                   "  oppure: sudo pacman -S arduino-cli (su Arch Linux)"
    fi
    
    success_msg "arduino-cli trovato: $($ARDUINO_CLI version | head -n1)"
}

###############################################################################
# Configurazione board e librerie
###############################################################################

setup_board() {
    info_msg "Configurazione board Wemos D1 R2..."
    
    # Aggiungi index per ESP8266 se non presente
    if ! "$ARDUINO_CLI" core list | grep -q "esp8266:esp8266"; then
        info_msg "Aggiunta board index ESP8266..."
        "$ARDUINO_CLI" core update-index --additional-urls https://arduino.esp8266.com/stable/package_esp8266com_index.json
        "$ARDUINO_CLI" core install esp8266:esp8266
    fi
    
    # Verifica che la board sia installata
    if ! "$ARDUINO_CLI" core list | grep -q "esp8266:esp8266"; then
        error_exit "Board ESP8266 non installata correttamente"
    fi
    
    success_msg "Board ESP8266 configurata"
}

install_libraries() {
    info_msg "Verifica librerie necessarie..."
    
    # Lista delle librerie richieste (già incluse con ESP8266 core)
    # ArduinoJson deve essere installata separatamente
    local libraries=("ArduinoJson@6.21.3")
    
    for lib in "${libraries[@]}"; do
        lib_name=$(echo "$lib" | cut -d'@' -f1)
        lib_version=$(echo "$lib" | cut -d'@' -f2)
        
        if "$ARDUINO_CLI" lib list | grep -q "^$lib_name"; then
            info_msg "Libreria $lib_name già installata"
        else
            info_msg "Installazione libreria $lib_name..."
            if [ -n "$lib_version" ]; then
                "$ARDUINO_CLI" lib install "$lib_name@$lib_version"
            else
                "$ARDUINO_CLI" lib install "$lib_name"
            fi
            
            if [ $? -eq 0 ]; then
                success_msg "Libreria $lib_name installata"
            else
                error_exit "Errore durante l'installazione di $lib_name"
            fi
        fi
    done
}

###############################################################################
# Rilevamento porta seriale
###############################################################################

detect_serial_port() {
    info_msg "Rilevamento porta seriale..."
    
    # Se la porta è stata passata come argomento, usala
    if [ -n "$SERIAL_PORT" ]; then
        if [ -e "$SERIAL_PORT" ]; then
            success_msg "Porta seriale: $SERIAL_PORT"
            return 0
        else
            error_exit "Porta seriale $SERIAL_PORT non trovata"
        fi
    fi
    
    # Auto-rilevamento porta seriale
    # Cerca porte USB comuni su Linux
    local ports=(
        "/dev/ttyUSB0"
        "/dev/ttyUSB1"
        "/dev/ttyACM0"
        "/dev/ttyACM1"
        "/dev/tty.wchusbserial*"
    )
    
    for port_pattern in "${ports[@]}"; do
        # Espandi il pattern (per gestire wildcard)
        for port in $port_pattern; do
            if [ -e "$port" ]; then
                SERIAL_PORT="$port"
                success_msg "Porta seriale rilevata: $SERIAL_PORT"
                return 0
            fi
        done
    done
    
    # Se non trovata, chiedi all'utente
    warning_msg "Nessuna porta seriale rilevata automaticamente"
    echo "Porte seriali disponibili:"
    ls -1 /dev/tty{USB,ACM}* 2>/dev/null || echo "  (nessuna trovata)"
    echo ""
    read -p "Inserisci la porta seriale (es. /dev/ttyUSB0): " SERIAL_PORT
    
    if [ -z "$SERIAL_PORT" ] || [ ! -e "$SERIAL_PORT" ]; then
        error_exit "Porta seriale non valida: $SERIAL_PORT"
    fi
    
    success_msg "Porta seriale: $SERIAL_PORT"
}

###############################################################################
# Compilazione e caricamento
###############################################################################

compile_sketch() {
    info_msg "Compilazione sketch..."
    
    "$ARDUINO_CLI" compile \
        --fqbn "$BOARD_FQBN" \
        --verbose \
        "$SKETCH_DIR"
    
    if [ $? -eq 0 ]; then
        success_msg "Compilazione completata con successo"
    else
        error_exit "Errore durante la compilazione"
    fi
}

upload_sketch() {
    info_msg "Caricamento sketch su $SERIAL_PORT..."
    
    # Verifica che la porta sia ancora disponibile
    if [ ! -e "$SERIAL_PORT" ]; then
        error_exit "Porta seriale $SERIAL_PORT non più disponibile"
    fi
    
    # Avviso per modalità bootloader se necessario
    warning_msg "Assicurati che il Wemos D1 R2 sia in modalità upload"
    warning_msg "(tieni premuto il pulsante BOOT durante il caricamento se necessario)"
    sleep 2
    
    "$ARDUINO_CLI" upload \
        --fqbn "$BOARD_FQBN" \
        --port "$SERIAL_PORT" \
        --verbose \
        "$SKETCH_DIR"
    
    if [ $? -eq 0 ]; then
        success_msg "Caricamento completato con successo!"
        info_msg "Apri il monitor seriale per vedere l'output (115200 baud)"
    else
        error_exit "Errore durante il caricamento"
    fi
}

###############################################################################
# Funzione principale
###############################################################################

main() {
    echo -e "${BLUE}"
    echo "╔════════════════════════════════════════════════════════════╗"
    echo "║  Wemos D1 R2 - Home Assistant Monitor                     ║"
    echo "║  Script di compilazione e caricamento                     ║"
    echo "╚════════════════════════════════════════════════════════════╝"
    echo -e "${NC}"
    
    # Verifica che lo sketch esista
    if [ ! -f "$SKETCH_DIR/$SKETCH_NAME.ino" ]; then
        error_exit "Sketch non trovato: $SKETCH_DIR/$SKETCH_NAME.ino"
    fi
    
    # Esegui i passaggi
    check_arduino_cli
    setup_board
    install_libraries
    detect_serial_port
    compile_sketch
    upload_sketch
    
    echo ""
    success_msg "Tutto completato con successo!"
    echo ""
    info_msg "Per aprire il monitor seriale:"
    echo "  arduino-cli monitor -p $SERIAL_PORT -c baudrate=115200"
    echo ""
}

###############################################################################
# Esecuzione
###############################################################################

# Esegui la funzione principale
main
