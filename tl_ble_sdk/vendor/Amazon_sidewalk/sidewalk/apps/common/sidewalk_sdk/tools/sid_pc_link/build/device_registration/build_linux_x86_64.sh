#!/bin/bash
# build_linux_x86_64.sh

# Get the root directory
ROOT_DIR="$(cd ../.. && pwd)"

# Build the Docker image for x86_64 architecture
docker build --platform linux/amd64 -t sid-build-linux-x86_64 -f Dockerfile.linux.x86_64 .

# Run the container to build the executable
docker run --rm --platform linux/amd64 -v "$ROOT_DIR:/app" sid-build-linux-x86_64 bash -c "
    # Install pip and dependencies
    apt-get update && apt-get install -y python3-pip

    # Install Python dependencies
    grep -v 'gatt;' /app/apps/device_registration/requirements.txt > /tmp/requirements_no_gatt.txt
    python3 -m pip install -r /tmp/requirements_no_gatt.txt
    python3 -m pip install pyinstaller

    # Build the executable
    cd /app/build/device_registration && python3 -m PyInstaller sid_device_registration.spec

    # Fix permissions
    chmod -R 777 /app/dist
"

echo "Linux x86_64 build completed. Executable is in dist/sid_device_registration"
