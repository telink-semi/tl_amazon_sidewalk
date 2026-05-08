#!/bin/bash
# build_macos.sh

# Get the root directory
ROOT_DIR="$(cd ../.. && pwd)"

# Create virtual environment
python3 -m venv $ROOT_DIR/build/device_registration/venv-mac
source $ROOT_DIR/build/device_registration/venv-mac/bin/activate

# Install dependencies
pip install -r $ROOT_DIR/apps/device_registration/requirements.txt
pip install pyinstaller

# Build the executable
cd $ROOT_DIR/build/device_registration && pyinstaller sid_device_registration.spec

echo "macOS build completed. Executable is in dist/sid_device_registration"
