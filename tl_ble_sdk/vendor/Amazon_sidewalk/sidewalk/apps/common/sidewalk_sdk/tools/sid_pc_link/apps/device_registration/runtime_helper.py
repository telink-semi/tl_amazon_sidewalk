#
# Copyright 2025 Amazon.com, Inc. or its affiliates. All rights reserved.
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

import os
import sys
import json
import shutil


def get_resource_path(relative_path):
    """Get absolute path to resource, works for dev and for PyInstaller"""
    try:
        # PyInstaller creates a temp folder and stores path in _MEIPASS
        base_path = sys._MEIPASS
    except Exception:
        base_path = os.path.abspath(".")

    return os.path.join(base_path, relative_path)


def is_bundled():
    """Check if running as bundled application"""
    return getattr(sys, "frozen", False) and hasattr(sys, "_MEIPASS")


def get_app_paths():
    """Get application paths based on execution context"""
    if is_bundled():
        script_path = os.path.dirname(os.path.realpath(sys.executable))
        lwa_path = os.path.join(sys._MEIPASS, "apps/device_registration/lwa")
        workspace_root = os.path.dirname(script_path)
    else:
        script_path = os.path.dirname(os.path.realpath(__file__))
        lwa_path = "{base}/lwa".format(base=script_path)
        workspace_root = os.path.abspath("{base}/../..".format(base=script_path))

    return script_path, lwa_path, workspace_root


def get_config_path():
    """Get the path to the user-editable config file"""
    if is_bundled():
        # When bundled, place the config file next to the executable
        executable_dir = os.path.dirname(os.path.realpath(sys.executable))
        return os.path.join(executable_dir, "app_config.json")
    else:
        # In development, use the config file in the app directory
        script_path = os.path.dirname(os.path.realpath(__file__))
        return os.path.join(script_path, "app_config.json")


def ensure_user_config_exists():
    """Ensure that a user-editable config file exists"""
    user_config_path = get_config_path()

    # If the user config doesn't exist, create it from the embedded default
    if not os.path.exists(user_config_path):
        try:
            # Get the embedded default config
            default_config_path = get_resource_path(
                "apps/device_registration/app_config.json"
            )

            # Copy it to the user-editable location
            shutil.copy2(default_config_path, user_config_path)
            print(f"Created default configuration file at: {user_config_path}")
        except Exception as e:
            print(f"Warning: Could not create default config file: {e}")
            return False

    return True
