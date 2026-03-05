# Sidewalk Device Registration Tool

A standalone binary for registering Sidewalk-compatible devices to the Sidewalk network from a PC.

## Overview

This tool allows you to register Sidewalk-supported devices to the Sidewalk network using LWA token authentication.

## Pre-requisites

- Manufacturing information should be available on the device to perform Sidewalk registration with the cloud.
- An active account on developer.amazon.com with a security profile for Login With Amazon
  - If you don't have a security profile, follow the `Create a New Security Profile` step at https://developer.amazon.com/docs/login-with-amazon/register-other-platforms-cbl-docs.html#create-a-new-security-profile
  - Add http://localhost:8000/ to the Allowed Origins for the security profile
- For Linux: Bluetooth adapter with proper permissions
- For macOS: Bluetooth enabled and permissions granted

## Installation

No installation required! Simply download the appropriate binary for your platform:

- **Linux**: `linux_x86_64/sid_device_registration`
- **macOS**: `macos/sid_device_registration`

Make the binary executable if needed:

```bash
chmod +x linux_x86_64/sid_device_registration  # For Linux
chmod +x macos/sid_device_registration  # For macOS
```

## Configuration

The tool uses a configuration file named `app_config.json` that should be placed in the same directory as the executable. The first time you run the tool, it will automatically create a default configuration file if one doesn't exist.

### Configuration File Parameters

| Entry                  | Description                                                | Default | Possible      | Comments                                           |
|-----------------------|-----------------------------------------------------------|---------|--------------|---------------------------------------------------|
| BLUETOOTH_ADAPTER     | Name of the Bluetooth adapter to use                       | hci0    | hci0, hci1   | Run hcitool devices to get adapter info            |
| COMMAND_TIMEOUT       | Time in seconds to wait for device response                | 2       | Any value    |                                                   |
| ENDPOINT_ID           | 32 byte Sidewalk Manufacturing Serial Number (SMSN) value  | -       | -            | Can be read from AWS Console              |
| LWA_TOKEN             | LWA token to authenticate calls to cloud endpoints         | -       | -            | Retrieved using steps in LWA Token section         |
| REFRESH_TOKEN         | Refresh token to retrieve a new LWA token                  | -       | -            | Retrieved using steps in LWA Token section         |
| SCAN_RETRIES          | Number of BLE scan retries                                 | 5       | Any value    |                                                   |
| BLUETOOTH_SCAN_TIMEOUT| Timeout for BLE scanning in seconds                        | 5       | Any value    |                                                   |

## Usage

### Basic Help

```bash
./sid_device_registration -h
```

This will display the available commands and options.

### Using LWA Token

#### Obtaining LWA and Refresh Tokens

1. To retrieve an LWA Token:
   ```bash
   # Fetching LWA token only
   ./sid_device_registration --lwa --client-id [your client ID]

   # Fetching LWA Token and Refresh Token
   ./sid_device_registration --lwa-cg --client-id [your client ID] --client-secret [your client secret]

   # Refreshing your LWA Token
   ./sid_device_registration --refresh-token
   ```

2. The first two options will launch a web browser and request your Amazon Developer account credentials.

3. Client ID and Client Secret can be fetched from developer.amazon.com under Security Profile → Web Settings → Client ID/Client Secret.

**Note: LWA token is valid for 1 hour, Refresh Token is valid forever. LWA token must be refreshed after it expires.**

#### Executing Device Registration with LWA Token

1. Edit the `app_config.json` file in the same directory as the executable:
   (Example)
   (IMP: The ENDPOINT_ID will be null and needs to be added, its the SMSN of the device)
   ```json
   {
     "BLUETOOTH_ADAPTER": "hci0",
     "COMMAND_TIMEOUT": "2",
     "ENDPOINT_ID": "3E3069414973133E681C7AB5145B6C539C1E7401B99FBA8FA7D148D9EC73BF33",
     "LWA_TOKEN": "Bearer Atza|testtoken-dummy_I61E10Wu7O2AImNV2eXOi6yuG7yOJ5dmtxuzshHwqHnw5XeCdD1ZHELXI0kDPM4iK4MJmV1k4TsEtqz2kBhdl9_40wB3bq13_wou_VZTwoxTsZ6NxbZ67e447yShMS3WWcZ9JSr3A9mfLTR6c-y__MPfzUV6EAgyEWxhoo4B-H9q52NmP0eqsEe59EXQ9d6gMHC3UrySdpDo8i21es6dP4dZWW8MSUoiLAXtyChHvY-m94X1f9TNZZs1RcNWZhvsPkyNgSnAaHAGuKwIjfxMO8yyz57WS8bU2RobBpo_fomWflay_6X7Zh8MP6bMPZqU3Ir3-zXRxAq7chPpQiq1SsizvJ4qDl5nyKsACGCERzFqpOhH-testtoken",
     "REFRESH_TOKEN": null,
     "SCAN_RETRIES": "5",
     "BLUETOOTH_SCAN_TIMEOUT": "5"
   }
   ```

2. Run the registration command:
   ```bash
   ./sid_device_registration -r
   ```

## Troubleshooting

### Configuration Issues

If you encounter configuration-related errors, check that:
- The `app_config.json` file exists in the same directory as the executable
- The configuration file contains valid JSON
- All required fields are present and correctly formatted

### BLE Issues

If you encounter BLE-related issues:
- On Linux, ensure that Bluetooth is enabled and that you have the necessary permissions
- On macOS, ensure that Bluetooth is enabled and that you have granted the necessary permissions to the application

### Common Commands for Debugging

- Get Bluetooth adapter information:
  ```bash
  hcitool devices
  ```

- Scan for BLE devices:
  ```bash
  sudo hcitool lescan
  ```

## License

This project is licensed under the terms specified in the accompanying LICENSE.txt file.
