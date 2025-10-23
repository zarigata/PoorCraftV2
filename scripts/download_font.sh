#!/bin/bash
# Download a free font for PoorCraft UI rendering
# Uses Roboto Regular from Google Fonts

set -e

SCRIPT_DIR="$(cd "$(dirname "${BASH_SOURCE[0]}")" && pwd)"
PROJECT_ROOT="$(dirname "$SCRIPT_DIR")"
FONTS_DIR="$PROJECT_ROOT/client/src/main/resources/fonts"
TARGET_FILE="$FONTS_DIR/default.ttf"

# Create fonts directory
mkdir -p "$FONTS_DIR"

echo "Downloading Roboto font..."

# Try curl first, fallback to wget
if command -v curl &> /dev/null; then
    DOWNLOAD_CMD="curl -L -o"
elif command -v wget &> /dev/null; then
    DOWNLOAD_CMD="wget -O"
else
    echo "ERROR: Neither curl nor wget found. Please install one of them."
    exit 1
fi

# Download Roboto font from Google Fonts GitHub
TEMP_ZIP="/tmp/roboto-font.zip"
ROBOTO_URL="https://github.com/google/roboto/releases/download/v2.138/roboto-unhinted.zip"

echo "Downloading from: $ROBOTO_URL"
$DOWNLOAD_CMD "$TEMP_ZIP" "$ROBOTO_URL"

# Extract the font
echo "Extracting font..."
if command -v unzip &> /dev/null; then
    unzip -q -o "$TEMP_ZIP" "Roboto-Regular.ttf" -d "/tmp/"
    mv "/tmp/Roboto-Regular.ttf" "$TARGET_FILE"
else
    echo "ERROR: unzip not found. Please install unzip."
    rm -f "$TEMP_ZIP"
    exit 1
fi

# Cleanup
rm -f "$TEMP_ZIP"

# Verify file exists and has reasonable size
if [ -f "$TARGET_FILE" ]; then
    FILE_SIZE=$(stat -f%z "$TARGET_FILE" 2>/dev/null || stat -c%s "$TARGET_FILE" 2>/dev/null)
    if [ "$FILE_SIZE" -gt 100000 ] && [ "$FILE_SIZE" -lt 1000000 ]; then
        echo "✓ Font downloaded successfully: $TARGET_FILE"
        echo "  Size: $((FILE_SIZE / 1024)) KB"
        exit 0
    else
        echo "ERROR: Downloaded file has unexpected size: $FILE_SIZE bytes"
        exit 1
    fi
else
    echo "ERROR: Font file not created"
    exit 1
fi
