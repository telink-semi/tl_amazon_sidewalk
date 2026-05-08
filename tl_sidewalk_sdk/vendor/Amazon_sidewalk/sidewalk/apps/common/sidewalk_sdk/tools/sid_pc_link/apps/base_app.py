#
# Copyright 2021 Amazon.com, Inc. or its affiliates. All rights reserved.
#
# AMAZON PROPRIETARY/CONFIDENTIAL
#
# You may not use this file except in compliance with the terms and conditions
# set forth in the accompanying LICENSE.txt file.
#
# THESE MATERIALS ARE PROVIDED ON AN "AS IS" BASIS. AMAZON SPECIFICALLY
# DISCLAIMS, WITH RESPECT TO THESE MATERIALS, ALL WARRANTIES, EXPRESS,
# IMPLIED, OR STATUTORY, INCLUDING THE IMPLIED WARRANTIES OF MERCHANTABILITY,
# FITNESS FOR A PARTICULAR PURPOSE, AND NON-INFRINGEMENT.
#

import sys
import os
import json

APPS_BASE_PATH = os.path.dirname(os.path.realpath(__file__))
LIBRARY_BASE_PATH = "{base}/{libs}".format(base=APPS_BASE_PATH, libs="../libs")
CURR_WS = os.getcwd()
_libs = []
_config_map = {}


def add_lib(path):
    global _libs
    _libs.append(path)


def load_dependencies():
    global _libs
    for library in _libs:
        lib_path = "{path}/{lib}".format(path=LIBRARY_BASE_PATH, lib=library)
        print("Adding library {}".format(lib_path))
        sys.path.append(lib_path)


def load_config():
    global _config_map

    # Try to use runtime_helper if available
    try:
        # First, try to import from the device_registration package
        try:
            from apps.device_registration.runtime_helper import (
                get_config_path,
                ensure_user_config_exists,
            )

            # Ensure the user config exists (creates it from default if needed)
            ensure_user_config_exists()
            # Get the path to the user-editable config file
            app_config_path = get_config_path()
        except ImportError:
            # If that fails, try to import from the current directory
            try:
                sys.path.append(
                    os.path.dirname(os.path.dirname(os.path.realpath(__file__)))
                )
                from device_registration.runtime_helper import (
                    get_config_path,
                    ensure_user_config_exists,
                )

                # Ensure the user config exists (creates it from default if needed)
                ensure_user_config_exists()
                # Get the path to the user-editable config file
                app_config_path = get_config_path()
            except ImportError:
                # Fall back to the original behavior
                app_config_path = "{base_path}/app_config.json".format(
                    base_path=CURR_WS
                )
    except Exception as e:
        print(f"Warning: Could not use runtime_helper: {e}")
        # Fall back to the original behavior
        app_config_path = "{base_path}/app_config.json".format(base_path=CURR_WS)

    print(f"Loading configuration from: {app_config_path}")
    try:
        with open(app_config_path, "r") as app_config_file:
            app_config_json = json.load(app_config_file)
    except FileNotFoundError:
        # Try one more fallback - look in the app's directory
        app_dir = os.path.dirname(os.path.dirname(os.path.realpath(__file__)))
        app_config_path = os.path.join(
            app_dir, "device_registration", "app_config.json"
        )
        print(f"Trying fallback configuration path: {app_config_path}")
        with open(app_config_path, "r") as app_config_file:
            app_config_json = json.load(app_config_file)

    for item in app_config_json.items():
        key, value = item
        # Initialize the value as None by default
        _config_map.update({key: None})
        # Give precedence to the environment configuration
        if value is not None and os.getenv(key) is None:
            os.environ.update({key: str(value)})
            _config_map.update({key: str(value)})
        else:
            if os.getenv(key) is not None:
                _config_map.update({key: os.getenv(key)})

    # If there is any configuration present in the environment
    # Write it back to the config file if we can
    try:
        with open(app_config_path, "w") as app_config_file:
            app_config_file.write(json.dumps(_config_map, indent=4))
    except (PermissionError, FileNotFoundError):
        # If we can't write to the file, just log a message
        print(f"Warning: Could not write configuration back to {app_config_path}")


def get_config():
    return _config_map


#########################################################
# Load the configuration when this module is imported
#########################################################
# Add the base apps path
load_config()
