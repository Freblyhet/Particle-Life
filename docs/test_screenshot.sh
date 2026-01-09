#!/bin/bash

echo "🧪 Testing Screenshot Functionality"
echo "=================================="

echo "1. Checking if ParticleLife is running..."
ps aux | grep -v grep | grep ParticleLife
if [ $? -eq 0 ]; then
    echo "✅ ParticleLife process found"
else
    echo "❌ ParticleLife not running"
    exit 1
fi

echo ""
echo "2. Testing window capture (focused window)..."
timestamp=$(date +"%Y%m%d_%H%M%S_%3N")
filename="../ParticleLifeScreenshots/test_screenshot_$timestamp.png"
screencapture -w "$filename" 2>/dev/null

if [ -f "$filename" ]; then
    echo "✅ Window screenshot captured: $filename"
    ls -la "$filename"
else
    echo "⚠️  Window capture failed, trying interactive..."
    screencapture -i "$filename" 2>/dev/null
    if [ -f "$filename" ]; then
        echo "✅ Interactive screenshot captured: $filename"
    else
        echo "❌ Screenshot test failed"
        exit 1
    fi
fi

echo ""
echo "3. Listing recent screenshots..."
ls -lt ../ParticleLifeScreenshots/ | head -5

echo ""
echo "🎉 Screenshot test completed!"