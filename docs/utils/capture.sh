#!/bin/bash

# Particle Life Screenshot Utility
# Quick access to screenshot functions

PROJECT_DIR="$(cd "$(dirname "${BASH_SOURCE[0]}")/.." && pwd)"
PYTHON_SCRIPT="$PROJECT_DIR/utils/screenshot_capture.py"

echo "🎬 Particle Life Screenshot Utility"
echo "=================================="

if [ ! -f "$PYTHON_SCRIPT" ]; then
    echo "❌ Python script not found: $PYTHON_SCRIPT"
    exit 1
fi

# Check if Python 3 is available
if ! command -v python3 &> /dev/null; then
    echo "❌ Python 3 is required but not installed"
    exit 1
fi

# Make the Python script executable
chmod +x "$PYTHON_SCRIPT"

# Show menu if no arguments
if [ $# -eq 0 ]; then
    echo "Available options:"
    echo "  1) Capture Particle Life window"
    echo "  2) Interactive selection"
    echo "  3) Timed capture (5 screenshots, 3s apart)"
    echo "  4) Guided sequence capture"
    echo "  5) Generate README markdown"
    echo ""
    read -p "Choose option (1-5): " choice
    
    case $choice in
        1) python3 "$PYTHON_SCRIPT" ;;
        2) python3 "$PYTHON_SCRIPT" --select ;;
        3) python3 "$PYTHON_SCRIPT" --timed 3 5 ;;
        4) python3 "$PYTHON_SCRIPT" --sequence ;;
        5) python3 "$PYTHON_SCRIPT" --update-readme ;;
        *) echo "❌ Invalid choice"; exit 1 ;;
    esac
else
    # Pass arguments directly to Python script
    python3 "$PYTHON_SCRIPT" "$@"
fi