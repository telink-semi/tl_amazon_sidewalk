#!/bin/bash
# copy_and_run_on_linux.sh

# Get the root directory
ROOT_DIR="$(cd ../.. && pwd)"

# Define the remote host
REMOTE_HOST="ubuntu@192.168.30.241"

# Create a temporary directory on the remote host
ssh $REMOTE_HOST "mkdir -p ~/sid_pc_link_dist"

# Copy the built executable and configuration files to the remote host
scp $ROOT_DIR/dist/linux_x86_64/sid_device_registration $ROOT_DIR/dist/linux_x86_64/app_config.json $REMOTE_HOST:~/sid_pc_link_dist/

# SSH into the remote host and run the registration command with elevated privileges
ssh -t $REMOTE_HOST "cd ~/sid_pc_link_dist && sudo ./sid_device_registration -r"
