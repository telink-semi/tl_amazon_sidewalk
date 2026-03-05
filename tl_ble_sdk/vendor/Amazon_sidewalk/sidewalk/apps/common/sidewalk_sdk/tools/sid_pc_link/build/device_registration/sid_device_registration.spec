# -*- mode: python ; coding: utf-8 -*-
block_cipher = None

a = Analysis(
    ['../../apps/device_registration/main.py'],
    pathex=['../..'],
    binaries=[],
    datas=[
        ('../../apps/device_registration/app_config.json', 'apps/device_registration'),
        ('../../apps/device_registration/lwa/templates', 'apps/device_registration/lwa/templates'),
        ('../../libs/protocol/sidewalk_builder', 'sidewalk_builder'),
    ],
    runtime_hooks=['force_bleak_hook.py'],
    hiddenimports=[
        'libs.device.transports.ble.macos.transport',
        'libs.cloud.http_client.registration_client',
        'libs.cloud.http_client.base_client',
        'libs.device.transports.transport',
        'libs.device.transports.transport_config',
        'libs.protocol.sidewalk_builder',
        'bleak',
        'flask',
        'webbrowser',
        'backoff',
        'requests',
        'coloredlogs',
    ],
    excludes=[],
    win_no_prefer_redirects=False,
    win_private_assemblies=False,
    cipher=block_cipher,
    noarchive=False
)

pyz = PYZ(
    a.pure,
    a.zipped_data,
    cipher=block_cipher
)

exe = EXE(
    pyz,
    a.scripts,
    a.binaries,
    a.zipfiles,
    a.datas,
    [],
    name='sid_device_registration',
    debug=False,
    bootloader_ignore_signals=False,
    strip=False,
    upx=True,
    upx_exclude=[],
    runtime_tmpdir=None,
    console=True,
    # Set environment variable to force using Bleak for BLE transport
    environ={'FORCE_BLEAK': 'true'},
)
