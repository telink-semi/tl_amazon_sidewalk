#!/bin/bash
# build_all.sh

# Get the root directory
ROOT_DIR="$(cd ../.. && pwd)"

# Create output directories
mkdir -p $ROOT_DIR/dist/sid_device_registration/linux_x86_64
mkdir -p $ROOT_DIR/dist/sid_device_registration/macos

# Copy README.md to dist folder
cp $ROOT_DIR/build/device_registration/README.md $ROOT_DIR/dist/sid_device_registration/

# Build for Linux (x86_64)
echo "Building for Linux (x86_64)..."
./build_linux_x86_64.sh
if [ -f "$ROOT_DIR/build/device_registration/dist/sid_device_registration" ]; then
    mv $ROOT_DIR/build/device_registration/dist/sid_device_registration $ROOT_DIR/dist/sid_device_registration/linux_x86_64/
    cp $ROOT_DIR/build/device_registration/app_config_public.json $ROOT_DIR/dist/sid_device_registration/linux_x86_64/app_config.json
fi

# Build for macOS (if on macOS)
if [[ "$OSTYPE" == "darwin"* ]]; then
    echo "Building for macOS..."
    ./build_macos.sh
    if [ -f "$ROOT_DIR/build/device_registration/dist/sid_device_registration" ]; then
        mv $ROOT_DIR/build/device_registration/dist/sid_device_registration $ROOT_DIR/dist/sid_device_registration/macos/
        cp $ROOT_DIR/build/device_registration/app_config_public.json $ROOT_DIR/dist/sid_device_registration/macos/app_config.json
    fi
else
    echo "Skipping macOS build (not on macOS)"
fi

echo "All builds completed. Executables are in dist/{linux_x86_64,macos} directories"
