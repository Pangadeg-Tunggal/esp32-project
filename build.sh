#!/data/data/com.termux/files/usr/bin/bash

echo "🚀 ESP32 Auto-Build Starting..."
git add .
git commit -m "Auto-build: $(date +'%Y-%m-%d %H:%M:%S')" 
git push origin main

echo "⏳ Build started on GitHub..."
echo "💤 Waiting 2 minutes for build to complete..."
sleep 120

echo "📥 Downloading firmware..."
# Nanti ganti URL ini setelah repo dipush
echo "✅ After first push, edit build.sh with your repo URL"
echo "🔧 Check: https://github.com/Pangadeg-Tunggal/esp32-project/actions"
