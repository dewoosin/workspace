#!/bin/bash
# Clean build script for GHOSTYPE ESP32

echo "Cleaning PlatformIO build files..."
rm -rf .pio
rm -rf .pioenvs
rm -rf .piolibdeps

echo "Build files cleaned. Run 'pio run' to rebuild."