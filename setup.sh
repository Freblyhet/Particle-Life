#!/bin/bash

# Particle Life Setup Script
# This script automates the build process for macOS/Linux

set -e  # Exit on error

echo "======================================"
echo "  Particle Life - Setup Script"
echo "======================================"
echo ""

# Check for CMake
if ! command -v cmake &> /dev/null; then
    echo "Error: CMake is not installed."
    echo "Please install CMake first:"
    echo "  macOS: brew install cmake"
    echo "  Linux: sudo apt-get install cmake"
    exit 1
fi

# Check for compiler
if ! command -v c++ &> /dev/null; then
    echo "Error: C++ compiler not found."
    echo "Please install build tools first:"
    echo "  macOS: xcode-select --install"
    echo "  Linux: sudo apt-get install build-essential"
    exit 1
fi

echo "Checking dependencies..."
echo "  CMake: $(cmake --version | head -n1)"
echo "  Compiler: $(c++ --version | head -n1)"
echo ""

# Create build directory
if [ -d "build" ]; then
    echo "Cleaning existing build directory..."
    rm -rf build
fi

echo "Creating build directory..."
mkdir -p build
cd build

# Configure
echo ""
echo "Configuring project with CMake..."
cmake .. -DCMAKE_BUILD_TYPE=Release

# Build
echo ""
echo "Building project..."
cmake --build . -j$(nproc 2>/dev/null || sysctl -n hw.ncpu 2>/dev/null || echo 4)

echo ""
echo "======================================"
echo "  Build Complete!"
echo "======================================"
echo ""
echo "To run the simulation:"
echo "  cd build && ./ParticleLife"
echo ""
echo "Or simply run: ./run.sh"
echo ""
