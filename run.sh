#!/bin/bash

# Simple run script for Particle Life

cd "$(dirname "$0")/build" 2>/dev/null || {
    echo "Build directory not found. Running setup first..."
    cd "$(dirname "$0")"
    ./setup.sh
    cd build
}

./ParticleLife
