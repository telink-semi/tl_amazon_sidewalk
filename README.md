# Telink Sidewalk SDK

Amazon Sidewalk SDK for the Telink TL3238E1 SoC, targeting the AIOT-DK1 development kit with ML3238S module.

## Supported Demos

| Demo | Description |
|------|-------------|
| **Amazon_sid_dut** | Sidewalk Device Under Test — interactive CLI for BLE messaging |
| **Amazon_sid_sbdt** | Sidewalk Bulk Data Transfer |
| **Amazon_diagnostics** | Sidewalk diagnostics |
| **Amazon_sid_900** | Sidewalk 900 demo for multi phy  |
| **Sensor monitoring** | Temperature + button demo with AWS web app |

## Hardware Requirements

- **AIOT-DK1 motherboard** with **ML3238S module** (TL3238E1 A0 chip)
- **Amazon Echo 4** (or other [compatible Sidewalk gateway](https://docs.sidewalk.amazon/introduction/sidewalk-gateways.html))
- UART-USB converter (optional, for debug logs)
- Male-female Arduino jumper cables
- VPN-capable Wi-Fi router (required outside North America)

## Software Requirements

- [Telink VS Code Extension](https://www.telink-semi.com/development-tools) — build, flash, and debug
- [Burning and Debugging Tool (BDT)](https://www.telink-semi.com/development-tools) — firmware programming
- Python 3.9+ with virtual environment
- [AWS CLI](https://aws.amazon.com/cli/) configured with IAM credentials
- Git

## Quick Start

```bash
# 1. Clone and checkout
git clone https://github.com/telink-semi/tl_amazon_sidewalk
cd tl_amazon_sidewalk

# 2. Open in VS Code
code .
```

3. **Build** — Open the Telink extension sidebar → Project Outline → select TL323X → click the rocket icon next to your desired demo
4. **Flash** — Use BDT multi-address download:
   - Firmware binary → `0x00000000`
   - MFG blob → `0x000F5000`
5. **Test** — Power the board via USB-C (J13), press SW2 to reset

See [Getting Started](https://www.telink-semi.com/products/amazon-sidewalk) for the full walkthrough including AWS cloud setup and MFG blob generation.

## Repository Structure

```
tl_amazon_sidewalk/
├── README.md
├── doc/                              # Documentation and release notes
│   ├── tl_amazon_sidewalk_Release_Note.md
│   ├── tl_platform_sdk_Release_Note.md
│   └── vscode_extention_user_guide.md
├── tl_sidewalk_sdk/
│   ├── vendor/
│   │   ├── Amazon_sidewalk/          # Sidewalk integration
│   │   │   ├── samples/              # Demo application code
│   │   │   ├── sidewalk/             # Sidewalk protocol stack (apps(PALs) + DPK impl)
│   │   │   ├── lib/                  # Pre-built libraries (v5/v5f, debug/release)
│   │   │   └── mbedtls/              # Crypto library
│   │   └── common/                   # Shared vendor code
│   ├── stack/                        # BLE and 2.4G protocol stacks
│   ├── drivers/TL323X/               # Chip drivers
│   ├── 3rd-party/freertos-V5/        # FreeRTOS
│   ├── proj_lib/                     # Pre-built platform libraries
│   ├── boot/                         # Boot code
│   └── CMakeLists.txt                # Build system
└── .gitlab-ci.yml                    # CI configuration
```

## Current Version

| Component | Version |
|-----------|---------|
| Sidewalk SDK | V1.0.0.9 (PR) |
| Chip | TL3238E1 A0/A1 |
| EVK | C1T388A20_V1.1 |
| Platform SDK | V3.11.3 |
| BLE SDK | V4.0.4.7 |
| Toolchain | TL32 ELF MCULIB V5F GCC12.2 |

## Documentation

- [Handbook](https://doc.telink-semi.cn/doc/en/software/res/sdk/ble/tl_ble_sdk_multi_connection_en/tl_ble_sdk_multi_connection_en/) — BLE stack user guide
- [Quick Start](https://doc.telink-semi.cn/doc/application_note/sidewalk/userguide/AN-26032600-E_Amazon_Sidewalk_Quick_Start_Guide.pdf) — Amazon Quick start
- [Guide](https://doc.telink-semi.cn/doc/application_note/sidewalk/userguide/AN-26051800-E_Amazon_Sidewalk_User_Manual.pdf) — Amazon sidewalk guide
- [Architecture Guide](doc/telink_sidewalk_architecture_guide.md) — Amazon sidewalk SDK architecture
- [Release Notes](doc/tl_amazon_sidewalk_Release_Note.md) — Version history

## Important Notes

- **Region**: Amazon Sidewalk is currently available in the US, Canada, and Mexico. Developers outside North America need a VPN-capable router. 
- **Flash protection** is enabled by default. Use the BDT "Unlock" command during development.
- **System clock** must be at least 32 MHz.
- **Battery voltage check** is critical for mass production to prevent abnormal Flash writes at low voltage.

## License

See [license_list.txt](license_list.txt) for third-party license information.
