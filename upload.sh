#!/bin/bash

###############################################################################
# Compilation and upload script for Wemos D1 R2 - Home Assistant Monitor
#
# This script:
# 1. Verifies arduino-cli installation
# 2. Configures Wemos D1 R2 board
# 3. Installs required libraries
# 4. Compiles the sketch
# 5. Uploads the sketch to the device
#
# Usage: ./upload.sh [serial_port]
# Example: ./upload.sh /dev/ttyUSB0
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
BOARD_FQBN="esp8266:esp8266:d1_mini"  # Wemos D1 R2 uses d1_mini core
ARDUINO_CLI="arduino-cli"

# Serial port (can be passed as argument or auto-detected)
SERIAL_PORT="${1:-}"

###############################################################################
# Utility functions
###############################################################################

# Print error message and exit
error_exit() {
    echo -e "${RED}❌ Error: $1${NC}" >&2
    exit 1
}

# Print success message
success_msg() {
    echo -e "${GREEN}✅ $1${NC}"
}

# Print info message
info_msg() {
    echo -e "${BLUE}ℹ️  $1${NC}"
}

# Print warning message
warning_msg() {
    echo -e "${YELLOW}⚠️  $1${NC}"
}

###############################################################################
# Prerequisites check
###############################################################################

check_arduino_cli() {
    info_msg "Checking arduino-cli installation..."
    
    if ! command -v "$ARDUINO_CLI" &> /dev/null; then
        error_exit "arduino-cli not found!\n\n" \
                   "Install arduino-cli:\n" \
                   "  curl -fsSL https://raw.githubusercontent.com/arduino/arduino-cli/master/install.sh | sh\n" \
                   "  or: sudo pacman -S arduino-cli (on Arch Linux)"
    fi
    
    success_msg "arduino-cli found: $($ARDUINO_CLI version | head -n1)"
}

###############################################################################
# Board and library configuration
###############################################################################

setup_board() {
    info_msg "Configuring Wemos D1 R2 board..."
    
    # Add ESP8266 index if not present
    if ! "$ARDUINO_CLI" core list | grep -q "esp8266:esp8266"; then
        info_msg "Adding ESP8266 board index..."
        "$ARDUINO_CLI" core update-index --additional-urls https://arduino.esp8266.com/stable/package_esp8266com_index.json
        "$ARDUINO_CLI" core install esp8266:esp8266
    fi
    
    # Verify board is installed
    if ! "$ARDUINO_CLI" core list | grep -q "esp8266:esp8266"; then
        error_exit "ESP8266 board not installed correctly"
    fi
    
    success_msg "ESP8266 board configured"
}

install_libraries() {
    info_msg "Checking required libraries..."
    
    # List of required libraries (already included with ESP8266 core)
    # ArduinoJson must be installed separately
    local libraries=("ArduinoJson@6.21.3")
    
    for lib in "${libraries[@]}"; do
        lib_name=$(echo "$lib" | cut -d'@' -f1)
        lib_version=$(echo "$lib" | cut -d'@' -f2)
        
        if "$ARDUINO_CLI" lib list | grep -q "^$lib_name"; then
            info_msg "Library $lib_name already installed"
        else
            info_msg "Installing library $lib_name..."
            if [ -n "$lib_version" ]; then
                "$ARDUINO_CLI" lib install "$lib_name@$lib_version"
            else
                "$ARDUINO_CLI" lib install "$lib_name"
            fi
            
            if [ $? -eq 0 ]; then
                success_msg "Library $lib_name installed"
            else
                error_exit "Error installing $lib_name"
            fi
        fi
    done
}

###############################################################################
# Serial port detection
###############################################################################

detect_serial_port() {
    info_msg "Detecting serial port..."
    
    # If port was passed as argument, use it
    if [ -n "$SERIAL_PORT" ]; then
        if [ -e "$SERIAL_PORT" ]; then
            success_msg "Serial port: $SERIAL_PORT"
            return 0
        else
            error_exit "Serial port $SERIAL_PORT not found"
        fi
    fi
    
    # Auto-detect serial port
    # Search common USB ports on Linux
    local ports=(
        "/dev/ttyUSB0"
        "/dev/ttyUSB1"
        "/dev/ttyACM0"
        "/dev/ttyACM1"
        "/dev/tty.wchusbserial*"
    )
    
    for port_pattern in "${ports[@]}"; do
        # Expand pattern (to handle wildcards)
        for port in $port_pattern; do
            if [ -e "$port" ]; then
                SERIAL_PORT="$port"
                success_msg "Serial port detected: $SERIAL_PORT"
                return 0
            fi
        done
    done
    
    # If not found, ask user
    warning_msg "No serial port automatically detected"
    echo "Available serial ports:"
    ls -1 /dev/tty{USB,ACM}* 2>/dev/null || echo "  (none found)"
    echo ""
    read -p "Enter serial port (e.g. /dev/ttyUSB0): " SERIAL_PORT
    
    if [ -z "$SERIAL_PORT" ] || [ ! -e "$SERIAL_PORT" ]; then
        error_exit "Invalid serial port: $SERIAL_PORT"
    fi
    
    success_msg "Serial port: $SERIAL_PORT"
}

###############################################################################
# Compilation and upload
###############################################################################

compile_sketch() {
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
}

upload_sketch() {
    info_msg "Uploading sketch to $SERIAL_PORT..."
    
    # Verify port is still available
    if [ ! -e "$SERIAL_PORT" ]; then
        error_exit "Serial port $SERIAL_PORT no longer available"
    fi
    
    # Warning for bootloader mode if necessary
    warning_msg "Make sure Wemos D1 R2 is in upload mode"
    warning_msg "(hold BOOT button during upload if necessary)"
    sleep 2
    
    "$ARDUINO_CLI" upload \
        --fqbn "$BOARD_FQBN" \
        --port "$SERIAL_PORT" \
        --verbose \
        "$SKETCH_DIR"
    
    if [ $? -eq 0 ]; then
        success_msg "Upload completed successfully!"
        info_msg "Open serial monitor to see output (115200 baud)"
    else
        error_exit "Error during upload"
    fi
}

###############################################################################
# Main function
###############################################################################

main() {
    echo -e "${BLUE}"
    echo "╔════════════════════════════════════════════════════════════╗"
    echo "║  Wemos D1 R2 - Home Assistant Monitor                     ║"
    echo "║  Compilation and upload script                            ║"
    echo "╚════════════════════════════════════════════════════════════╝"
    echo -e "${NC}"
    
    # Verify sketch exists
    if [ ! -f "$SKETCH_DIR/$SKETCH_NAME.ino" ]; then
        error_exit "Sketch not found: $SKETCH_DIR/$SKETCH_NAME.ino"
    fi
    
    # Execute steps
    check_arduino_cli
    setup_board
    install_libraries
    detect_serial_port
    compile_sketch
    upload_sketch
    
    echo ""
    success_msg "All completed successfully!"
    echo ""
    info_msg "To open serial monitor:"
    echo "  arduino-cli monitor -p $SERIAL_PORT -c baudrate=115200"
    echo ""
}

###############################################################################
# Execution
###############################################################################

# Execute main function
main
