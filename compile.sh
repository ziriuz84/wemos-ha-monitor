#!/bin/bash

###############################################################################
# Compilation script for Wemos D1 R2 - Home Assistant Monitor
#
# Compiles the sketch without uploading to device
#
# Usage: ./compile.sh
###############################################################################

# Colors for output
RED='\033[0;31m'
GREEN='\033[0;32m'
YELLOW='\033[1;33m'
BLUE='\033[0;34m'
NC='\033[0m' # No Color

# Configuration
SKETCH_NAME="wemos_ha_monitor"
SKETCH_DIR="$(dirname "$(readlink -f "$0")")"
BOARD_FQBN="esp8266:esp8266:d1_mini"
ARDUINO_CLI="arduino-cli"

# Utility functions
error_exit() {
    echo -e "${RED}❌ Error: $1${NC}" >&2
    exit 1
}

success_msg() {
    echo -e "${GREEN}✅ $1${NC}"
}

info_msg() {
    echo -e "${BLUE}ℹ️  $1${NC}"
}

# Check arduino-cli
if ! command -v "$ARDUINO_CLI" &> /dev/null; then
    error_exit "arduino-cli not found!"
fi

# Verify sketch exists
if [ ! -f "$SKETCH_DIR/$SKETCH_NAME.ino" ]; then
    error_exit "Sketch not found: $SKETCH_DIR/$SKETCH_NAME.ino"
fi

info_msg "Compiling sketch..."

"$ARDUINO_CLI" compile \
    --fqbn "$BOARD_FQBN" \
    --verbose \
    "$SKETCH_DIR"

if [ $? -eq 0 ]; then
    success_msg "Compilation completed successfully"
else
    error_exit "Error during compilation"
fi
