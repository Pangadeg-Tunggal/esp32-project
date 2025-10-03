#!/bin/bash

echo "🔨 Building WebBT for ESP32 DevKitC V4"

# Check if PlatformIO is installed
if ! command -v pio &> /dev/null; then
    echo "❌ PlatformIO is not installed. Please install it first."
    echo "Visit: https://platformio.org/install"
    exit 1
fi

# Clean previous builds
echo "🧹 Cleaning previous builds..."
pio run --target clean

# Build the project
echo "🔧 Building project..."
pio run

# Check if build was successful
if [ $? -eq 0 ]; then
    echo "✅ Build successful!"
    echo "📁 Firmware: .pio/build/esp32dev/firmware.bin"
    echo "📊 Size: $(stat -f%z .pio/build/esp32dev/firmware.bin) bytes"
    
    # Optional: upload if parameter provided
    if [ "$1" == "upload" ]; then
        echo "📤 Uploading to ESP32..."
        pio run --target upload
    fi
else
    echo "❌ Build failed!"
    exit 1
fi
