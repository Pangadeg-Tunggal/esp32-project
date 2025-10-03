#!/data/data/com.termux/files/usr/bin/bash

echo "🚀 ESP32 Auto-Build Starting..."
git add .
git commit -m "Auto-build: $(date +'%Y-%m-%d %H:%M:%S')" 
git push origin main

echo "⏳ Build started on GitHub..."
echo "💤 Waiting 2 minutes for build to complete..."
sleep 120

echo "📥 Downloading firmware..."
wget -q -O firmware.zip "https://nightly.link/Pangadeg-Tunggal/esp32-project/workflows/build/main/firmware.zip"

if [ -f firmware.zip ]; then
    unzip -o firmware.zip
    echo "✅ Firmware ready: firmware.bin"
    echo "📍 Location: $(pwd)/firmware.bin"
    # Cek size file
    ls -la firmware.bin
else
    echo "❌ Build failed or still running..."
    echo "🔧 Check: https://github.com/Pangadeg-Tunggal/esp32-project/actions"
fi
