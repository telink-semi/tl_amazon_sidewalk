## V1.0.0.6(PR)

### Version

* SDK Version: tl_sidewalk_sdk V1.0.0.6
* Chip Version: 
    - TL323X:                  A0
* Hardware EVK Version:
    - TL323X:                  C1T388A20_V1.1
* Platform Version: 
    - TL323X:                  tl_platform_sdk V3.10.0
* Ble SDK Version: 
    - TL323X:                  tl_ble_sdk V4.0.4.6
* Toolchain Version:
    - TL323X:                  TL32 ELF MCULIB V5F GCC12.2  (IDE: [TelinkIoTStudio](https://www.telink-semi.com/development-tools))

### Note

   * N/A
   
### BREAKING CHANGES

   * Modify the address of the burned mfg file to 0xF5000

### Features

    * Support Location feature(Only for BLE)

### Bug Fixes
    
    * Fix the issue where the advertising interval is incorrect..
    * Fix the issue where SBDT transmission fails over AWS.

### Refactoring

   * N/A

### Performance Improvements

   * N/A

### Known issues

* N/A

### CodeSize

* TL323X
    - Compiling Amazon_sid_dut
        - Flash bin size: 412 KB
        - IRAM size: 111.3 KB
        - DRAM size: 22.90 KB
    - Compiling Amazon_sid_sbdt
        - Flash bin size: 352 KB
        - IRAM size: 109.42 KB
        - DRAM size: 18.15 KB
    - Compiling Amazon_diagnostics
        - Flash bin size:  165 KB
        - IRAM size: 74.1 KB
        - DRAM size: 12.75 KB


### 版本

* SDK Version: tl_sidewalk_sdk V1.0.0.6
* Chip Version: 
    - TL323X:                  A0
* Hardware EVK Version:
    - TL323X:                  C1T388A20_V1.1
* Platform Version: 
    - TL323X:                  tl_platform_sdk V3.10.0
* Ble SDK Version: 
    - TL323X:                  tl_ble_sdk V4.0.4.6
* Toolchain Version:
    - TL323X:                  TL32 ELF MCULIB V5F GCC12.2  (IDE: [TelinkIoTStudio](https://www.telink-semi.com/development-tools))


### BREAKING CHANGES

 * 修改烧录mfg 文件位置为0xF5000
 
### Features

    * 支持定位功能

### Bug Fixes
    
    * 修复adv 广播间隔不正确的问题
    * 修改dut demo 在通过AWS 进行sbdt 测试出现异常问题

### Refactoring

* N/A

### Performance Improvements

   * N/A

### Known issues

* N/A

### CodeSize

* TL323X
    - Compiling Amazon_sid_dut
        - Flash bin size: 412 KB
        - IRAM size: 111.3 KB
        - DRAM size: 22.90 KB
    - Compiling Amazon_sid_sbdt
        - Flash bin size: 352 KB
        - IRAM size: 109.42 KB
        - DRAM size: 18.15 KB
    - Compiling Amazon_diagnostics
        - Flash bin size:  165 KB
        - IRAM size: 74.1 KB
        - DRAM size: 12.75 KB

 **Note:** 
 * N/A 
 
 
 
## V0.0.2.0(ER)

### Version

* SDK Version: tl_sidewalk_sdk V0.0.2.0
* Chip Version: 
    - TL323X:                  A0
* Hardware EVK Version:
    - TL323X:                  C1T388A20_V1.1
* Platform Version: 
    - TL323X:                  tl_platform_sdk V3.10.0
* Ble SDK Version: 
    - TL323X:                  tl_ble_sdk V4.0.4.6
* Toolchain Version:
    - TL323X:                  TL32 ELF MCULIB V5F GCC12.2  (IDE: [TelinkIoTStudio](https://www.telink-semi.com/development-tools))

### Note

   * Abnormal operations during the upgrade process of the SBDT demo may cause system exceptions.
   
### BREAKING CHANGES

   * N/A

### Features
  * Support for the Sidewalk SBDT demo.
  * Support for the Sidewalk DUT demo.
  * Support for the Sidewalk diagnostics demo.

    
### CodeSize

* TL323X
    - Compiling Amazon_sid_dut
        - Flash bin size: 392 KB
        - IRAM size: 103.65 KB
        - DRAM size: 21.49 KB
    - Compiling Amazon_sid_sbdt
        - Flash bin size: 381 KB
        - IRAM size: 126.43 KB
        - DRAM size: 18.12 KB
    - Compiling Amazon_diagnostics
        - Flash bin size:  112.71 KB
        - IRAM size: 75.48 KB
        - DRAM size: 12.46 KB

### 版本

* SDK Version: tl_sidewalk_sdk V0.0.2.0
* Chip Version: 
    - TL323X:                  A0
* Hardware EVK Version:
    - TL323X:                  C1T388A20_V1.1
* Platform Version: 
    - TL323X:                  tl_platform_sdk V3.10.0
* Ble SDK Version: 
    - TL323X:                  tl_ble_sdk V4.0.4.6
* Toolchain Version:
    - TL323X:                  TL32 ELF MCULIB V5F GCC12.2  (IDE: [TelinkIoTStudio](https://www.telink-semi.com/development-tools))

### Note
   * SBDT demo 存在升级过程中的异常操作会导致系统异常

### Features
  * 支持 Sidewalk SBDT demo.
  * 支持 Sidewalk DUT demo.
  * 支持 Sidewalk diagnostics demo.

### Performance Improvements

   * N/A

### Known issues

* N/A

### CodeSize

* TL323X
    - Compiling Amazon_sid_dut
        - Flash bin size: 392 KB
        - IRAM size: 103.65 KB
        - DRAM size: 21.49 KB
    - Compiling Amazon_sid_sbdt
        - Flash bin size: 381 KB
        - IRAM size: 126.43 KB
        - DRAM size: 18.12 KB
    - Compiling Amazon_diagnostics
        - Flash bin size:  112.71 KB
        - IRAM size: 75.48 KB
        - DRAM size: 12.46 KB



## tl_amazon_sidewalk(FR)

### Version

* SDK Version: tl_amazon_sidewalk V0.0.1.0
* Chip Version: 
  - TL323X			A0
* Driver Version: 
  - TL323X			tl_platform_sdk V3.8.0
* Toolchain Version:
  - TL323X:         TL32 ELF MCULIB V5F GCC12.2  (IDE: TelinkIoTStudio)

### Hardware
* TL323X: C1T388A20_V1.1
  
### Note
* The system clock must be at least 32M.
* Battery Voltage Check is a very important function for mass production. The user must check the battery voltage to prevent abnormal writing or erasing Flash at a low voltage.
* Flash protection is a critical function for mass production. 
	- Flash protection is enabled by default in SDK. User must enable this function on their mass production application. 
	- Users should use the "Unlock" command in the Telink BDT tool for Flash access during the development and debugging phase.
	- Flash protection demonstration in SDK is a reference design based on sample code. Considering that the user's final application may be different from the sample code, 
	for example, the user's final firmware size is bigger, or the user has a different OTA design, or the user needs to store more data in some other area of Flash, 
	all these differences imply that Flash protection reference design in SDK can not be directly used on user's mass production application without any change. 
	User should refer to sample code, understand the principles and methods, and then change and implement a more appropriate mechanism according to their application if needed.
* This is a function release version that has only undergone R&D testing. 
### Bug Fixes
   * N/A

### BREAKING CHANGES 	
   * N/A

### Features
* **Chip**
  - Support TL323X chip.
* **Demo & Library**
  - Provide 1 basic BLE demos and feature test examples.
	- Amazon_dut_demo supports 1 ACL Peripheral devices and sidewalk.
* **FreeRTOS**
  - Support FreeRTOS in Amazon_dut_demo. 

### Refactoring
   * N/A

### Performance Improvements
   * N/A

### Known issues
* **General BLE function**
  - When connecting to ACL central, The ACL peripheral device may fail with a low probability. This issue will be fixed in the next version.
  - When the bin size is larger than 256K, please change the OTA startup address using API--blc_ota_setFirmwareSizeAndBootAddress. The API needs to be placed before sys_init().
  - There is a small probability of failure during OTA, this issue will be fixed in the next version.

