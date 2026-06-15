## V1.0.0.8(PR)

### Version

* SDK Version: tl_sidewalk_sdk V1.0.0.8
* Chip Version: 
    - TL3238E1:                  A0
* Hardware EVK Version:
    - TL3238E1:                  C1T388A20_V1.1
* Platform Version: 
    - TL3238E1:                  tl_platform_sdk V3.11.2
* Ble SDK Version: 
    - TL3238E1:                  tl_ble_sdk V4.0.4.7
* Toolchain Version:
    - TL3238E1:                  TL32 ELF MCULIB V5F GCC12.2  (IDE: [TelinkIoTStudio](https://www.telink-semi.com/development-tools))


### BREAKING CHANGES

   * N/A

### Features

    * Support anti-rollback efuse interface.
    * Update sidewalk stack to version 1.19.5.53.
    * Add the flash_erase_block_64k and flash_erase_block_32k interfaces.

### Bug Fixes
    
  * **Drivers**
      * **gpio**
        * (TL3238E1):Fix the unexpected interrupt issue that occurred during the initialization configuration of the GPIO.
          * Detailed description:When configured as rising-edge triggered, if the external GPIO input is at a high level, the gpio_set_irq initialization will result in an unexpected interrupt response.
          * After Fix:After the fix, this issue is no longer present.
          * Update recommendation: To use the GPIO edge-triggered interrupt function, the driver must be updated.
      * **rf**	
        * (TL3238E1) Fixed the problem that Δf2 index failed to meet spec requirements when testing TX FDEV performance of some chips.
          * Detailed description: 99.9% of Δf2 values are below 185KHz in TX FDEV test of partial chips.
          * After Fix: After repair, all TX performance indicators comply with specification requirements.
          * Update recommendation: Mandatory update.

### Refactoring

   * Optimize RAM; reduce RAM consumption by 21KB for the 900 demo.

### Performance Improvements

   * N/A

### Known issues

* N/A

### CodeSize

* TL323X
    - Compiling Amazon_sid_dut
        - Flash bin size: 394.16 KB
        - IRAM size: 93.79 KB
        - DRAM size: 20.17 KB
    - Compiling Amazon_sid_sbdt
        - Flash bin size: 330.60 KB
        - IRAM size: 88.97 KB
        - DRAM size: 15.38 KB
    - Compiling Amazon_diagnostics
        - Flash bin size:  147.49 KB
        - IRAM size: 56.27 KB
        - DRAM size: 10.22 KB
    - Compiling Amazon_sid_900
        - Flash bin size:  317.88 KB
        - IRAM size: 81.54 KB
        - DRAM size: 14.98 KB

**Note:** 
  * N/A


### 版本

* SDK Version: tl_sidewalk_sdk V1.0.0.8
* Chip Version: 
    - TL3238E1:                  A0
* Hardware EVK Version:
    - TL3238E1:                  C1T388A20_V1.1
* Platform Version: 
    - TL3238E1:                  tl_platform_sdk V3.11.2
* Ble SDK Version: 
    - TL3238E1:                  tl_ble_sdk V4.0.4.7
* Toolchain Version:
    - TL3238E1:                  TL32 ELF MCULIB V5F GCC12.2  (IDE: [TelinkIoTStudio](https://www.telink-semi.com/development-tools))


### BREAKING CHANGES

 * N/A
 
### Features

    * 支持读写anti-rollback 的相关接口
    * 升级sidewalk 协议栈到1.19.5.53 版本.
    * 增加 flash_erase_block_64k 和 flash_erase_block_32k 接口

### Bug Fixes
    
* **Drivers**
    * **gpio**
        * (TL323X):修复gpio在初始化配置过程中产生的一次非预期中断问题。(merge_requests/@2688)
          * 详细描述：当配置为上升沿触发时，外部gpio输入高电平，gpio_set_irq初始化完将导致一次非预期的中断响应。
          * 修复效果:修复后无该问题。
          * 更新建议：使用 GPIO沿中断触发功能必须更新驱动。
    * **rf**
        * (TL323X): 修复部分芯片测试tx fdev性能时Δf2指标未能达到spec要求。(merge_requests/@2656)(merge_requests/@2690)
        * 详细描述： 部分芯片tx fdev测试中Δf2 99.9%指标小于185KHz。
        * 修复效果: 修复后tx所有性能指标均能够符合spec要求。
        * 更新建议：必须更新。

### Refactoring

* 优化RAM，900 demo 减少使用RAM 21KB

### Performance Improvements

* N/A

### Known issues

* N/A

### CodeSize

* TL323X
    - Compiling Amazon_sid_dut
        - Flash bin size: 394.16 KB
        - IRAM size: 93.79 KB
        - DRAM size: 20.17 KB
    - Compiling Amazon_sid_sbdt
        - Flash bin size: 330.60 KB
        - IRAM size: 88.97 KB
        - DRAM size: 15.38 KB
    - Compiling Amazon_diagnostics
        - Flash bin size:  147.49 KB
        - IRAM size: 56.27 KB
        - DRAM size: 10.22 KB
    - Compiling Amazon_sid_900
        - Flash bin size:  317.88 KB
        - IRAM size: 81.54 KB
        - DRAM size: 14.98 KB

 **Note:** 
 .* N/A

## V1.0.0.7(PR)

### Version

* SDK Version: tl_sidewalk_sdk V1.0.0.7
* Chip Version: 
    - TL323X:                  A0
* Hardware EVK Version:
    - TL323X:                  C1T388A20_V1.1
* Platform Version: 
    - TL323X:                  tl_platform_sdk V3.11.0
* Ble SDK Version: 
    - TL323X:                  tl_ble_sdk V4.0.4.7
* Toolchain Version:
    - TL323X:                  TL32 ELF MCULIB V5F GCC12.2  (IDE: [TelinkIoTStudio](https://www.telink-semi.com/development-tools))


### BREAKING CHANGES

   * N/A

### Features

    * Support sec boot via sbdt
    * Support sid 900 demo for Sub-GHz (FSK/CSS),This feature supports CSS switching via button.

### Bug Fixes
    
  * **Drivers**
    * **rf**
         * (TL323X)Fixed the issue of inconsistent Tx power caused by the call order between rf_rx_performance_mode and power-setting functions (e.g., rf_set_power_level).
            * Detailed description:Two bits related to Tx power are written with fixed values in the rf_rx_performance_mode interface. Therefore, calling this function before or after power-setting functions (e.g., rf_set_power_level) will result in inconsistent Tx Power.
            * After Fix: Tx Power is independent of the calling sequence of rf_rx_performance_mode and set power functions, and RX performance is not affected.
            * Update Recommendation:Mandatory update.
    * **sys**
        * (TL323X) Fixed the issue where RF power-on operation caused system crash when the system was operating at high frequency.
            * Detailed description: Before setting analog register 0x7d, if system clock is at high frequencies, the system may crash.
            * After Fix: During the system init setting analog register 0x7d, at this moment, the system clock is 24M RC，the system runs normally, active current increases by about 110ua.
            * Update Recommendation: Mandatory update.

    * **adc**
        * (TL323X)Fixed the issue where the ADC vbat mode failed to accurately sample low voltages at normal temperatures.
            * Detailed description:The VBAT_MODE_BELOW_2V2_DETECT_EN configuration macro has been removed, and the internal implementation of sd_adc_calculate_voltage has been modified. The sd_adc_set_vbat_4p_calib_vref interface has been added to implement the 4-point calibration logic, which is compatible with the original 2-stage (curved and linear) calibration of A0. The lpc_vbat_vol_detect_deinit interface has also been added to disable the voltage detection function below 2.2V for VBAT.
            * After Fix: When the VBAT is sampled at voltages below 2.3V at room temperature, the error has been reduced from the original 100mv to 20mv.
            * Update Recommendation:It is recommended to update when using ADC.
        * (TL323X)Optimized GPIO 1/4 voltage divider sampling accuracy for 0mV~30mV.
            * Detailed description:Since GPIO performs with higher precision in 1/1 attenuation (no division) mode when capturing 0~30mV, the efuse_set_sd_adc_calib_value interface has been added to calibrate 1/1 mode sampling. Additionally, the sd_adc_div_switch_adjust_rescale interface was introduced to enable dynamic switching: the system now automatically switches to 1/1 mode for samples below 50mV to enhance accuracy, while forcing a switch back to 4/1 mode for samples exceeding 1000mV to ensure range coverage.
            * After Fix: The measurement error for voltages below 30mV has been significantly reduced from 10mV to approximately 3mV.
            * Update Recommendation:Recommended for applications requiring high-precision sampling in the 0~30mV range.
    * **efuse**
        * (TL323X)Fixed the risk of incorrect chip ID reading in the efuse_get_chip_id interface.
            * Detailed description: The eFuse clock was not configured for the efuse_get_chip_id interface, which may result in incorrect chip ID reading when called under different clock conditions.
            * After Fix: Calling this interface at any clock frequency carries no risk of a chip ID read error.
            * Update Recommendation: When using the efuse_get_chip_id interface, the `/proj_lib/*.a` file of the corresponding chip must be updated.
    * **gpio**
        * (TL323X)Fixed the issue where the GPIO IRQ mask could not be turned off.
            * Detailed description: After calling gpio_clr_irq_mask, the GPIO interrupt will continue to be triggered.
            * After Fix: Call the function gpio_clr_irq_mask, and the GPIO interrupt will no longer be triggered.
            * Update Recommendation:It is recommended to update when using GPIO IRQ.

### Refactoring

   * N/A

### Performance Improvements

   * N/A

### Known issues

* N/A

### CodeSize

* TL323X
    - Compiling Amazon_sid_dut
        - Flash bin size: 409.43 KB
        - IRAM size: 110.26 KB
        - DRAM size: 22.92 KB
    - Compiling Amazon_sid_sbdt
        - Flash bin size: 350.07 KB
        - IRAM size: 108.71 KB
        - DRAM size: 18.13 KB
    - Compiling Amazon_diagnostics
        - Flash bin size:  165.26 KB
        - IRAM size: 74.78 KB
        - DRAM size: 12.73 KB
    - Compiling Amazon_sid_900
        - Flash bin size:  337.18 KB
        - IRAM size: 101.19 KB
        - DRAM size: 17.73 KB

**Note:** 
  * Supports JTAG debugging, with PM function disabled by default.


### 版本

* SDK Version: tl_sidewalk_sdk V1.0.0.7
* Chip Version: 
    - TL323X:                  A0
* Hardware EVK Version:
    - TL323X:                  C1T388A20_V1.1
* Platform Version: 
    - TL323X:                  tl_platform_sdk V3.11.0
* Ble SDK Version: 
    - TL323X:                  tl_ble_sdk V4.0.4.7
* Toolchain Version:
    - TL323X:                  TL32 ELF MCULIB V5F GCC12.2  (IDE: [TelinkIoTStudio](https://www.telink-semi.com/development-tools))


### BREAKING CHANGES

 * N/A
 
### Features

    * 通过SBDT 升级的时候支持security boot 模式
    * 支持sid_900 demo ,支持 CSS/FSK 功能,支持通过按键进行CSS 模式切换.

### Bug Fixes
    
* **Drivers**
    * **rf**
        * (TL323X)修复了以解决rf_rx_performance_mode与set power函数调（如rf_set_power_level）用先后顺序引起的power不一致问题。
            * 详细描述：与Tx power相关的2个bit在rf_rx_performance_mode接口中被写入固定值，因此在set power函数（如rf_set_power_level）前后调用该函数会导致Tx Power不一致。
            * 修复效果：Tx Power与rf_rx_performance_mode/set power函数调用顺序无关，rx性能未受影响 。
            * 更新建议：必须更新。
    * **sys**
        * (TL323X) 修复了在系统高频运行时，RF上电操作导致系统崩溃的问题。
            * 详细描述：在设置模拟寄存器0x7d之前，如果系统时钟处于高频状态，系统可能会崩溃。
            * 修复效果：在系统初始化过程中设置模拟寄存器0x7d，此时系统时钟为24M RC，系统正常运行, active电流增加110ua左右。
            * 更新建议：必须更新。
    * **adc**
        * (TL323X)修复了ADC vbat模式常温下采样低电压不准的问题。
            * 详细描述：移除了VBAT_MODE_BELOW_2V2_DETECT_EN配置宏，修改了sd_adc_calculate_voltage内部实现，增加了sd_adc_set_vbat_4p_calib_vref 接口用于4点校准逻辑，兼容A0原来2段式（曲线和直线）校准，增加lpc_vbat_vol_detect_deinit 接口用于关闭VBAT2.2V以下电压检测功能。
            * 修复效果：常温下VBAT采样2.3V以下电压时，误差由原来100mv降低到20mv。
            * 更新建议：使用adc时建议更新。
        * (TL323X)优化 GPIO 4 分压采样 0mV~30mV 精度问题。
            * 详细描述：鉴于 GPIO 在 1 分压模式下采集 0~30mV 电压具有更高的精度，本次更新新增了 efuse_set_sd_adc_calib_value 接口，专门用于校准 1 分压模式下的采样偏置。同时引入 sd_adc_div_switch_adjust_rescale 接口实现分压模式的动态切换：当采样电压低于 50mV 时，系统将自动切换至 1 分压模式以提升精度；当采样电压高于 1000mV 时，则强制切换回 4 分压模式以保证量程。
            * 修复效果：30mV 以下极小电压的采集误差从原先的 10mV 显著降低至 3mV 左右
            * 更新建议：若应用场景涉及 0~30mV 极低电压的精确采集，建议同步此更新。
    * **efuse**
        * (TL323X)修复efuse_get_chip_id接口获取chip id错误的风险。
            * 详细描述：efuse_get_chip_id接口未配置efuse时钟，不同时钟下调用此接口会有获取chip id错误的风险。
            * 修复效果：在任意时钟下调用此接口无读错chip id的风险。
            * 更新建议：使用efuse_get_chip_id接口时必须更新对应芯片的`/proj_lib/*.a`。
    * **gpio**
        * (TL323X) 修复了gpio irq mask 无法关闭的问题。
            * 详细描述：调用gpio_clr_irq_mask后，gpio中断会继续触发。
            * 修复效果：调用函数gpio_clr_irq_mask，gpio中断不会继续触发。
            * 更新建议：使用gpio irq时建议更新。

### Refactoring

* N/A

### Performance Improvements

* N/A

### Known issues

* N/A

### CodeSize

* TL323X
    - Compiling Amazon_sid_dut
        - Flash bin size: 409.43 KB
        - IRAM size: 110.26 KB
        - DRAM size: 22.92 KB
    - Compiling Amazon_sid_sbdt
        - Flash bin size: 350.07 KB
        - IRAM size: 108.71 KB
        - DRAM size: 18.13 KB
    - Compiling Amazon_diagnostics
        - Flash bin size:  165.26 KB
        - IRAM size: 74.78 KB
        - DRAM size: 12.73 KB
    - Compiling Amazon_sid_900
        - Flash bin size:  337.18 KB
        - IRAM size: 101.19 KB
        - DRAM size: 17.73 KB

 **Note:** 
 * 默认支持jtag 调试，关闭了PM功能.

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

