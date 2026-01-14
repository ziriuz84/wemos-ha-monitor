#!/bin/bash

###############################################################################
# Script di compilazione per Wemos D1 R2 - Home Assistant Monitor
#
# Compila lo sketch senza caricarlo sul dispositivo
#
# Uso: ./compile.sh
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
BOARD_FQBN="esp8266:esp8266:d1_mini"
ARDUINO_CLI="arduino-cli"

# Funzioni di utilità
error_exit() {
    echo -e "${RED}❌ Errore: $1${NC}" >&2
    exit 1
}

success_msg() {
    echo -e "${GREEN}✅ $1${NC}"
}

info_msg() {
    echo -e "${BLUE}ℹ️  $1${NC}"
}

# Verifica arduino-cli
if ! command -v "$ARDUINO_CLI" &> /dev/null; then
    error_exit "arduino-cli non trovato!"
fi

# Verifica che lo sketch esista
if [ ! -f "$SKETCH_DIR/$SKETCH_NAME.ino" ]; then
    error_exit "Sketch non trovato: $SKETCH_DIR/$SKETCH_NAME.ino"
fi

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
