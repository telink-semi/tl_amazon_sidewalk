# Sidewalk Config CLI Documentation

## Root Commands
 - `print`        - [Print Child Commands](#print-child-commands)
 - `mac`          - [Mac Child Commands](#mac-child-commands)
 - `metrics`      - [Metrics Child Commands](#metrics-child-commands)
 - `tx`           - [Tx Child Commands](#tx-child-commands)
 - `time`         - [Time Child Commands](#time-child-commands)
 - `help`         - Commands help


## Print Child Commands
```
print - Print values from device
    fwversion     - Print FW version
    mfg           - Print mfg values
    metrics       - Print GW/EN metrics
                    EN only
                        print metrics <category_id> prints metrics for category
                        available category_id defined in
                            projects/sid/sal/common/internal/sid_metrics_defines/sid_metrics_defines.h
    clear_metrics - Clear GW/EN metrics
                    EN only
                        print clear_metrics <category_id> <metrics_id>, clears metrics for category
                        available category_id defined in
                            projects/sid/sal/common/internal/sid_metrics_defines/sid_metrics_defines.h
    txid          - Print Obfuscated device ID (TX-ID)
    get_cap       - Print current capability and config
                    print get_cap <value>
                    value can be of the following
                        1:  Print current capability
                        2:  Print current config
                        3:  Print current capability and config
    clear_cap     - Clear current capability and config
                    print clear_cap <value>
                    value can be of the following
                        1:  Clear current capability
                        2:  Clear current config
                        3:  Clear current capability and config
```

## Mac Child Commands
```
 mac - MAC config commands
    gcfg    - Get MAC config parameters
    scfg    - Set MAC config parameters
                Set GroupNum - mac scfg N 5
                Set PAN ID - mac scfg P 0xA4C5387B00
                Set Dev ID - mac scfg I 0xA00008EAAF
                Set LDRChannel - mac scfg R 0 (range 0-7)
                Set Region Code - mac scfg H <region_code> (REGION_US915 - 1, REGION_EU868 - 2)
                - GW only
                    Set GatewaySidewalkConsent - mac scfg Y 1 (enable 1, disable 0)
                - EN only
                    Set MaxBeaconMiss - mac scfg M 3
                    Set PairingStateStatus - mac scfg J 1 (enable 1, disable 0)
                    Set HomePANScanInterval - mac scfg Q 60 (in seconds, maximum 65535)
                    Set GroupPeriodicity - mac scfg E 1 (range 1-10)
                    Set UnicastPeriodicity - mac scfg V 4 (range 1-10)
                    Set GroupOffset - mac scfg O 1 (range 0-9)
                    Set UnicastOffset - mac scfg W 1 (range 0-9)
                    Set Group Wakeup Schedule - mac scfg G 3 (TX/RX disabled - 0, only TX enabled - 1, only RX enabled - 2, TX/RX enabled - 3)
                    Set Unicast Wakeup Schedule - mac scfg D 3 (TX/RX disabled - 0, only TX enabled - 1, only RX enabled - 2, TX/RX enabled - 3)
                    Start forcing FSK end node to sync to desired GW - mac scfg C 1 0xA00008EAAF
                    Stop forcing FSK end node to to desired GW  - mac scfg C 0
                    Set continuous FSK tx slot - mac scfg T 1 (enable 1, disable 0)
                    Set Low Latency parameters - mac scfg L 1 2 4 0 (Enabled -1, Latency - 2, Rate limit - 4, Repetitions - 0)
    skey    - Set security keys
                Set PAN master key - mac skey PM 0x00112233445566778899AABBCCDDEEFF (16bytes)
                Set WAN master key - mac skey WM 0x00112233445566778899AABBCCDDEEFF (16bytes)
                Set Temporary Unicast Key(GW only) - mac skey TU 0x00112233445566778899AABBCCDDEEFF (16bytes)
                Set Temporary App Key(GW only)- mac skey TA 0x00112233445566778899AABBCCDDEEFF (16bytes)
    rcfg    - Reset MAC config parameters. The device factory resets and all the configuration values
                 are reset to factory defaults
    log     - Set Log Level for Sub-modules (default loglevel=2) (1 = enable)
                To enable beacon logs: mac log 0 1 <optional: log_level>
                To enable probe logs:    mac log 1 1 <optional: log_level>
                To enable hdr      logs:    mac log 2 1 <optional: log_level>
                To enable ldr        logs:    mac log 3 1 <optional: log_level>
                To enable dfu logs on gateway: mac log 5 1 <optional: log_level>
                To enable event list logs: mac log 5 1 <optional: log_level>
```

## Metrics Child Commands
```
metrics - Set Metrics values
    set         - Set value of metrics data
                    For GW
                        metrics set 1 <metric_param> <value> (set GW Metrics param to value)
                        metric_param can be of the following
                            0:  num_tx_beacon
                            1:  total_time_spent_tx_beacon (ms)
                            2:  fsk_tx_pkts
                            3:  fsk_tx_fail
                            4:  fsk_ack_not_received
                            5:  fsk_rx_pkts
                            6:  fsk_tx_ack_pkts
                            7:  fsk_crc_error
                            8:  total_time_spent_fsk_transmit (ms)
                            9:  total_time_spent_fsk_receive (ms)
                            10: total_time_spent_fsk_listen (ms)
                            11: lora_tx_drop
                            12: lora_tx_pkts
                            13: lora_rx_pkts
                            14: lora_rx_err
                            15: lora_rx_hdr_err
                            16: lora_rx_preamble_err
                            17: total_time_spent_lora_transmit (ms)
                            18: total_time_spent_lora_receive (ms)
                            19: total_time_spent_lora_listen (ms)
                            20: phy_radio_err
                            21: ll_hdr_decode_err
                            22: nw_hdr_decode_err
                            23: nw_security_err
                            24: app_hdr_decode_err
                            25: unexpected_rx_err
                            26: oversized_pkt

                        metrics set 2 <channel_num> <sample_cnt> <avg_rssi_value> <avg_rssi_std>
                            (set Noise Floor Metrics param to value)

                    For EN
                        metrics set <category_id> <metric_id> <value> Set metrics_fwk_v2 values
                            available category_id and correspond metric_id defined in
                                projects/sid/sal/common/internal/sid_metrics_defines/sid_metrics_defines.h
                        metrics config_set <action> <category_id> <metric_id> Set cfgs to metrics_fwk_v2 bitmask.
                            available actions defined in
                                projects/sid/sal/common/internal/sid_metrics_core_ifc/sid_metrics_core_ifc.h
                            available category_id and correspond metric_id defined in
                                projects/sid/sal/common/internal/sid_metrics_defines/sid_metrics_defines.h
    send        - Send metrics data to cloud
                    For GW
                        metrics send (1: GW Metrics (deprecated), 2: Noise Floor Metrics)
                    For EN
                        metrics send (triggers default flow for periodic metrics collection and transmit to cloud)
```

## Tx Child Commands
```
tx - Transmit with radio
    rnet        - Transmit RingNet encoded frame
                    tx rnet 1 (send command based on the rnet_app frame settings)
                    tx rnet 0 (clear command)
    rnet_app    - Form RingNet Application Layer Command
                    A: dst_frmt (0: cloud, 1: devID, 3: groupID)
                    B: src_frmt
                    C: ack req (1: yes, 0: no)
                    D: dst devID(5 hex ID, like 0xBFFFFF60BA)
                    E: Auth code
                    F: enc_ena(1: yes, 0: no)
                    G: response req (1: yes, 0: no)
                    H: seq #
                    I: sec_ena (1: yes, 0: no)
                    J: seq #
                    K: link for TX (1:app, 2:radio, 3:BLE, 4:srl, 5:stack)
                    L: retries
                    M: expl data len
                    N: suppress_broadcast_ack (1: yes, 0: no)
                    S: src devID
                    T: id
                    U: class id
                    V: flip data
                    W: data hdr
                    X: status code
                    Y: data len
                    Z: proto ver
                    a: sleepy device
    dcr         - Set DCR duty cycle
                    tx dcr <duty cycle 1-100 in percentage>
                    tx dcr 0 (print current DCR setting)
```

## Time Child Commands
```
time - Send/Set time sync command and params
    gcs_param     - Set GCS parameters to sync
                    get GCS if no parameter
                    <time_out s> <cld sync interval s>  <max retry> set gcs
    get_time      - Send GET_TIME from
                       <devid>
                       cld if no parameter
    set_time      - Set system time <tv_sec> <drift_ms>
    now           - Print current time
    set_offset    - Set time offset <offset_ms> <drift_offset_ms>
```

## TPC Child Commands
```
tpc lora - LoRa TPC get/set commands
    scfg          - Set LoRa TPC config parameters
                    [-p <trial_check_period_min> <trial_probe_period_min> <cooldown_period_min>] <enabled>
                    <enabled> - enabled/disabled TPC
                        1 - enabled
                        0 - disabled
                    -p configure period parameters:
                        <trial_check_period_min> - The minimum check period in minutes in the trial phase
                        <trial_probe_period_min> - The minimum probe period in minutes in the trial phase
                        <cooldown_period_min> - The cooldown period in minutes to avoid probing with reduced TX power
                        valid range: <trial_check_period_min> < <trial_probe_period_min> < <cooldown_period_min>
    gcfg          - Get LoRa TPC config parameters
    status        - Get LoRa TPC current status

tpc fsk - FSK TPC EP/GW get/set commands
    scfg          - Set EP/GW FSK TPC config parameters
                    <enabled> {EP:[<retry_pkt_thres> <retry_pwr_thres> <force_tx_pwr>]
                               GW:[<minimum_snr>]}
                    <enabled> - enabled/disabled EP/GW TPC
                    EP:
                        2 - enabled no GW TPC only
                        1 - enabled all
                        0 - disabled
                    GW:
                        1 - enabled
                        0 - disabled
                    - configure parameters:
                    EP:
                        <retry_pkt_thres> - The EP PKT counter threshold to trigger retry adjustment the EP PWR
                        <retry_pwr_thres> - The EP PWR threshold to trigger retry adjustment the EP PWR
                        <force_tx_pwr> - Force EP TX PWR, disable: <= 0, PWR Range: 1 ~ 22.
                    GW:
                        <minimum_snr> - The GW minimum target SNR for PWR adjustment EP
    gcfg          - Get FSK TPC config parameters
    status        - Get FSK TPC current status
```
