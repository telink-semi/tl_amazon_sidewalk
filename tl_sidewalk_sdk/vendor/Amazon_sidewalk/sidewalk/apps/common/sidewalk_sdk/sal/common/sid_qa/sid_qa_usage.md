# Sidewalk QA CLI Documentation

### Overview
The following document outlines the usage of the SID QA CLI component. This component allows for testing
sid_api functions through command line and is used in QA to qualify Sidewalk releases. Any changes on the
sid_api that are expected to be tested by QA should be added as command line functionality to this component.

### CLI commands usage and format
Arguments to the command are shown in <>
```
    sid init <1,2,3,4,5,6,7,8>                      - initialize the sidewalk stack, 1 is SID_LINK_TYPE_1 (BLE)
                                                      2 is SID_LINK_TYPE_2 (FSK)
                                                      3 is SID_LINK_TYPE_3 (LoRa)
                                                      4 is SID_LINK_TYPE_1 | SID_LINK_TYPE_3 (BLE + LoRa)
                                                      5 is SID_LINK_TYPE_1 | SID_LINK_TYPE_2 (BLE + FSK)
                                                      6 is SID_LINK_TYPE_2 | SID_LINK_TYPE_3 (FSK + LoRa)
                                                      7 is SID_LINK_TYPE_1 | SID_LINK_TYPE_2 | SID_LINK_TYPE_3 (BLE + FSK + LoRa)
                                                      8 is SID_LINK_TYPE_ANY
                                                      This calls the sid_init() API.

    sid set_init_cfg <metrics> <fsk_ffs> <ffs_retry_period>
                                                    - Sets the init configuration of Sidewalk stack. command to be run before every sid init for the new setting to take effect
                                                       <metrics> enable/disable Sub-Ghz metrics reporting to the cloud services
                                                       <fsk_ffs> enable/disable of FFS of Sub-Ghz FSK link only. Does not apply for BLE
                                                       <ffs_retry_period> Retry periodicity of FFS over FSK in seconds

    sid deinit                                      - deinitialize the sidewalk stack, calls sid_deinit() API
    sid start  <link>                               - start the sidewalk stack, calls sid_start() API.
                                                      link value is optional, it can take the same values as for sid init command. If link value is not present the one
                                                      set with sid init will be used to call sid_start api.
    sid stop   <link>                               - stop the sidewalk stack, calls sid_stop() API.
                                                      link value is optional, it can take the same values as for sid init command. If link value is not present the one
                                                      set with sid init will be used to call sid_stop api.

    sid send -t <tv> -d <dv> -l <lm> -i <id> -o <low> -a <ack> <retry> <ttl> -r <data>
                                                    - send data over the SID_LINK_TYPE selected, calls the sid_put_msg()API.
                                                      Data field must always be placed at the end of command patametrs. If -r parameter is not preset
                                                      data filed is treated as ascii. Example usage:
                                                          - sid send TEST
                                                          - sid send -t 0 -d 2 TEST
                                                          - sid send -t 1 -d 1 -r 01AAFF02
                                                      Optional parametrs:
                                                    - t option force specific message type to be set in message descriptor
                                                        possible <tv> values:
                                                        0 - SID_MSG_TYPE_GET
                                                        1 - SID_MSG_TYPE_SET
                                                        2 - SID_MSG_TYPE_NOTIFY
                                                        3 - SID_MSG_TYPE_RESPONSE
                                                        If -t option is not used message type in message descriptor will be set to SID_MSG_TYPE_NOTIFY.
                                                        If message type is set to RESPONSE type ID for outgoing message will be set with value that was previously received with GET type message. Or it can be set with sid set_rsp_id command.
                                                    - d specifies the destination
                                                        possible <dv> values:
                                                        1 - SID_LINK_MODE_CLOUD
                                                        2 - SID_LINK_MODE_MOBILE
                                                        If -d option is not used link_mode in message descriptor will be set to SID_LINK_MODE_CLOUD.
                                                    - l link mask on which message should be sent, if not set LINK_TYPE_ANY will be used
                                                        <lm> link mask - for possible values see 'sid init' command
                                                    - i message id that needs to be used to send response. Valid only for messages of type response
                                                        <id> response id
                                                    - r data is interpreted hex string e.g. 010203AAFF
                                                    - o low laency configuration
                                                        possible <low> values:
                                                        0 - Default Setting. Send the message with low latency disabled.
                                                        1 - Send the message as a LoRa low latency message
                                                    - a configure parameters for transport ack:
                                                        <ack> - enable/disable ACK
                                                          1 - enable ACK
                                                          0 - disable ACK
                                                        <retry> - number of retry. 0 ~ 255
                                                        <ttl> - total seconds the stack holds the message in its queue. 0 ~ 65535
                                                    - f configure parameters for flood messages
                                                        <flood_msg> - below flood options are supported
                                                          0 - flood messages disabled/stop.
                                                          1 - continuous flood messages without ack/response from sidewalk network.
                                                          2 - back to back uplink messages with on_msg_sent called after each message sent.
                                                        <flood_period> - specifies the number of messages to send in flood.
                                                        <delay_sec> - specifies the delay in seconds to be applied in next message send after OOM error for any message send.
                                                    Example: sid send -r 0123 -f 1 1000 2
                                                              will send 1000 messages continuously without ack. if OOM error occurs for 501th message, then delay of 2 seconds will be applied and again message send will start from 502 onwards
                                                            sid send -r 0123 -f 2 500 3
                                                              will send 500 messages, with on_msg_sent called after each message send.
    sid factory_reset                               - factory reset the board, deleting all registration status. This calls the
                                                      sid_set_factory_reset() API.
    sid get_mtu <1,2,3>                             - get the MTU for the selected link type, 1 is SID_LINK_TYPE_1 (BLE),
                                                      2 is SID_LINK_TYPE_2 (FSK), 3 is SID_LINK_TYPE_3 (LoRa), 8 is SID_LINK_TYPE_ANY. This calls the sid_get_mtu() API.
    sid option <option> <val1>...<valN>             - Set link options. This calls the sid_option() API. Possible inputs for  "<option> <val1>...<valN>" are as follows:
                                                      "-lp_set <val1> .. <valN>" -  sets SID_LINK_TYPE_2 and SID_LINK_TYPE_3 (900 MHz) link profile and related parameters.
                                                        This API requires the device having network time (GCS) and returns an error code otherwise.
                                                        This API can be exercised only when the link is started otherwise, an error code is returned
                                                        At bootup time the API returns an error code prior to obtaining GCS.
                                                      "-lp_set 1 [<bcn_int_unit>]" - for SID_LINK2_PROFILE_1 with optional beacon interval unit parameter".
                                                        <bcn_int_unit> Specifies beacon interval unit, with minimum 1(default) and maximum 3, each unit is 10080ms.
                                                      "-lp_set 2 <rx_int> [<bcn_int_unit> <rx_duration>]" - for SID_LINK2_PROFILE_2 with optional rx_int and beacon interval unit and rx_duration parameters".
                                                        <rx_int> Specifies DL interval between rx opportunities in units of ms. The value must be a multiple of 63ms. When ommitted the default value of 63ms is used.
                                                        <bcn_int_unit> Specifies beacon interval unit, with minimum 1(default) and maximum 3, each unit is 10080ms.
                                                        <rx_duration> Specifies the duration of rx window opened after uplinks in seconds.
                                                          0 -> infinitely (default),
                                                          65535 -> uplink only.
                                                      "-sub_ghz_ctl <ctl_cmd>" - for user control commands with sub-ghz
                                                        <ctl_cmd> Specifies the user commands:
                                                          1 - for SID_LINK2_PROFILE_2 or SID_LINK3_PROFILE_A, temporary stop RX window, RX window will be opened upon next successful uplink.
                                                      "-lp_set 0x80 <rxwc>" - for SID_LINK3_PROFILE_A, where <rxwc> is the rx_window count parameter.
                                                      "-lp_set 0x81 <rxwc>" - for SID_LINK3_PROFILE_B, where <rxwc> is the rx_window count parameter.
                                                      "-lp_set 0x83 <rxwc>" - for SID_LINK3_PROFILE_D, where <rxwc> is the rx_window count parameter.
                                                          <rxwc> - (uint8) rx window count. 0 represents infinite windows.
                                                      "-lp_get_l2" - Gets link profile and associated parameters for SID_LINK_TYPE_2. Ex: "app: CMD: ERR: 0 Link_profile ID: 1 Wndw_cnt: 0".
                                                      "-lp_get_l3" - Gets link profile and associated parameters for SID_LINK_TYPE_3. Ex: "app: CMD: ERR: 0 Link_profile ID: 128 Wndw_cnt: 5".
                                                      "-d <0,1>" - filter duplicate message.
                                                          0 - filter duplicate message.
                                                          1 - don't filter duplicate message and notify to user.
                                                      "-gd" - Get filter duplicates configuration. Ex: "app: CMD: ERR: 0 Filter Duplicates: 0"
                                                      "-m <policy>" - Set link connection policy
                                                          <policy> value of the link connection policy
                                                          0 - SID_LINK_CONNECTION_POLICY_NONE
                                                          1 - SID_LINK_CONNECTION_POLICY_AUTO_CONNECT
                                                          2 - SID_LINK_CONNECTION_POLICY_MLM
                                                      "-gm" - Get current link connection policy
                                                      "-c <link> <enable> <priority> <timeout>" - Set Auto connect policy parameters per link
                                                          <link_type> - link on which the auto connect parameters need to be applied. valid values are only (1,2,3)
                                                          1 - SID_LINK_TYPE_1 (BLE)
                                                          2 - SID_LINK_TYPE_2 (FSK)
                                                          3 - SID_LINK_TYPE_3 (LoRa)
                                                          <enable> - enable/disable auto connect for the link type
                                                          0 - Disable auto connect
                                                          1 - Enable auto connect
                                                          <priority> - priority per link, valid values 0(Highest) to 255(Lowest), optional when disabling auto connect
                                                          <timeout> - total seconds the stack attempts to establish a connection on the link, optional when disabling auto connect
                                                       "-gc <link_type>" - Get Auto connect policy per link. Ex: "app: CMD: ERR: 0 AC Policy, link 1, enable 1 priority 0 timeout 30"
                                                       "-ml <policy>" - Set Multi link policy parameters
                                                            <policy> - The multi link policy that needs to be applied. valid values are only (0,1,2,3,4)
                                                            0 - SID_LINK_MULTI_LINK_POLICY_DEFAULT
                                                            1 - SID_LINK_MULTI_LINK_POLICY_POWER_SAVE
                                                            2 - SID_LINK_MULTI_LINK_POLICY_PERFORMANCE
                                                            3 - SID_LINK_MULTI_LINK_POLICY_LATENCY
                                                            4 - SID_LINK_MULTI_LINK_POLICY_RELIABILITY
                                                        "gml" - Get current configured Multi link policy. Ex: ""
                                                      "-st_get" - Get statistics, ex:
                                                        "app: CMD: ERR: 0 tx: 3, acks_sent 8, tx_fail: 0, retries: 4, dups: 6, acks_recv: 7 rx: 8"
                                                      "-st_clear" - Clear statistics ex:
                                                      "-gsi" - Get Sidewalk ID, ex:
                                                        "<info> app: CMD: ERR: 0 SIDEWALK_ID: BFFFFFC94E"
                                                      "-ble_cfg <cmd> <type> <v1> <v2> <v3> <v4> <v5> <v6> <v7> <v8>" - BLE configurations
                                                          <cmd>     - Get/Set command
                                                            0 - Get
                                                              advertising config ex: "app: CMD: ERR: 0 BLE adv, fast_int: 160(100ms), fast_timeout 0(0ms), slow_int 1600(1000ms), slow_timeout 0(0ms)"
                                                              connection config ex: "app: CMD: ERR: 0 BLE conn, min_int: 20(25ms), max_int 20(25ms), sl 24, timeout 400(4000ms)"
                                                            1 - Set
                                                          <type>    - Advertising / Connection configuration
                                                            0 - Advertising config
                                                            1 - Connection config
                                                            2 - Both advertising and connection config
                                                            3 - Inactivity timeout config
                                                          <v1> required for set commands with Inactivity timeout config.
                                                          <v1> <v2> <v3> <v4> only required for set commands with Advertising/Connection config.
                                                          <v1> <v2> <v3> <v4> <v5> <v6> <v7> <v8> required for set commands with both advertising and connection config.
                                                          for advertising config, <v1> <v2> <v3> <v4> corresponds to <fast_interval> <fast_timeout> <slow_interval> <slow_timeout> :
                                                            <fast_interval> - fast advertising interval in units, each unit = 0.625ms, range: [16,8192]
                                                            <fast_timeout>  - fast advertising timeout in units, each unit = 10ms, range: [0,65535], 0 means indefinite
                                                            <slow_interval> - slow advertising interval in units, each unit = 0.625ms, range: [800,8192]
                                                            <slow_timeout>  - slow advertising timeout in units, each unit = 10ms, range: [0,65535], 0 means indefinite
                                                          for connection config, <v1> <v2> <v3> <v4> corresponds to <min_int> <max_int> <sl> <sup_timeout> :
                                                            <min_int>     - min connection interval in units, each unit = 1.25ms, range: [0,3200]
                                                            <max_int>     - max connection interval in units, each unit = 1.25ms, range: [0,3200]
                                                            <sl>          - s-latency, range: [0,499]
                                                            <sup_timeout> - supervision timeout in units, each unit = 10ms, max is 3200units(32s)
                                                            Note: the parameters must satisfied
                                                              1. min_int <= max_int
                                                              2. max_int*1.25*(sl+1) < sup_timeout
                                                          for Inactivity config, <v1> corresponds to <inactivity timeout> in sec.
                                                          for Both advertising and connection config,  <v1> <v2> <v3> <v4> <v5> <v6> <v7> <v8> corresponds to <fast_interval> <fast_timeout> <slow_interval> <slow_timeout> <min_int> <max_int> <sl> <sup_timeout>
                                                      "-da_cfg <cmd> [<delayed_ack_time>]" - Get/Set the time for Delayed Ack using the delayed_ack_time parameter
                                                          <cmd> - Get/Set command
                                                            0 - Get:
                                                              prints the current value for delayed ack
                                                              Ex: "app: CMD: ERR: 0 The time for Delayed Ack: 0"
                                                                  "app: CMD: ERR: 0 The time for Delayed Ack: 20000"
                                                            1 - Set:
                                                          <delayed_ack_time> Set the time for delayed ack in milliseconds. Min: 0 msec / Max: 20000 msec
                                                      "-gwd_cfg <cmd=0,1> [<policy>] [<scan> <cont> <rssi_conn> <rssi_disc> <rssi_num> <bcn_miss> <bcn_skip> <use_load> <load_disc> <use_health> <use_rank>]"
                                                          The command is intended for getting and setting gw discovery policy parameters (FSK only)
                                                          <cmd> - Get/Set command
                                                            0 - Get Policy:
                                                              prints the current policy value, for example: "GWD policy: 0"
                                                              for the custom user policy, the command additionally displays its parameters, for example:
                                                              "GWD policy params: scan:30 cont:1 rssi_conn:-65 rssi_disc:-80 rssi_num:4 bcn_miss:3 bcn_skip:0 use_load:1 load_disc:0 use_health:1 use_rank:1"
                                                              "GWD policy backoff params:"
                                                              "   threshold_1:5 scan_1:30 sleep_1:60"
                                                              "   threshold_2:15 scan_2:30 sleep_2:600"
                                                              "   threshold_3:255 scan_3:30 sleep_3:3600"
                                                            1 - Set Policy:
                                                          <policy> value of the gw discovery policy
                                                            0 - SID_LINK_TYPE_2_GW_DISCOVERY_POLICY_DEFAULT
                                                            1 - SID_LINK_TYPE_2_GW_DISCOVERY_POLICY_OPTIMIZED_FOR_FAST_CONNECTION
                                                            2 - SID_LINK_TYPE_2_GW_DISCOVERY_POLICY_OPTIMIZED_FOR_RELIABILE_CONNECTION
                                                            3 - SID_LINK_TYPE_2_GW_DISCOVERY_POLICY_OPTIMIZED_FOR_POWER_SAVE
                                                            4 - SID_LINK_TYPE_2_GW_DISCOVERY_POLICY_CUSTOM
                                                          for SID_LINK_TYPE_2_GW_DISCOVERY_POLICY_CUSTOM policy the user must specify the following parameters:
                                                          <scan>       - Gateway discovery scanning timeout in seconds
                                                          <cont>       - Continue to scan even after scan_timeout_secs has expired: 0 - disable, 1 - enable
                                                          <rssi_conn>  - Minimum RSSI level at which connection is allowed, 0 means not used
                                                          <rssi_disc>  - RSSI level at which EP initiates disconnection from the connected gateway
                                                          <rssi_num>   - Number of beacons to calculate the average RSSI level, 0 means not used
                                                          <bcn_miss>   - The maximum number of consecutive beacons that can be missed before the EP notifies disconnection
                                                          <bcn_skip>   - The maximum number of beacons the EP can skip listening
                                                          <use_load>   - Use gateway load to calculate gw score: 0 - disable, 1 - enable
                                                          <load_disc>  - Gateway load at which EP initiates disconnection from the connected gateway, range: [0,3], 0 means not used
                                                          <use_health> - Use gateway connection health to calculate gw score: 0 - disable, 1 - enable
                                                          <use_rank>   - Use gateway transmit power rank to calculate gw score: 0 - disable, 1 - enable
                                                    "-gwd_cfg <cmd=3> <backoff_size> <threshold_1> <scan_1> <sleep_1> [<threshold_2> <scan_2> <sleep_2> <threshold_3> <scan_3> <sleep_3> <threshold_4> <scan_4> <sleep_4>]"
                                                          The command is intended for presetting of backoff params for gw discovery user policy (FSK only).
                                                          Use this command to set non-default backoff parameters for the custom user policy. The command should be called before setting the custom policy.
                                                          <cmd=3> - Preset backoff params for gw discovery user policy
                                                          <backoff_size> - backoff params array size, range: [1,4]
                                                          <threshold_N> - The retry threshold at which the specified parameters (<scan_N>, <sleep_N>) are applied, range: [1,255], 255 means infinity
                                                          <scan_N> - Scan duration in seconds, range: [1,65535]
                                                          <sleep_N> - Sleep time in seconds between scanning, range: [0,65535]
                                                    "-ble_con <is_set> <conn_policy> <policy_param>"
                                                        <is_set> - 0 or 1
                                                          0 - Get current connection policy
                                                          1 - Set BLE connection policy
                                                        <conn_policy> - 0, 1 or 2
                                                          0 - SID_BLE_CONN_POLICY_DEFAULT
                                                          1 - SID_BLE_CONN_POLICY_LONG_LIVED_CONNECTION
                                                          2 - SID_BLE_CONN_POLICY_OPTIMAL_ADVERTISING
                                                        <policy_param>  - 0 or 1(Needed only for Long lived connection policy)
                                                          0 - Optimizes BLE long lived connection for power
                                                          1 - Optimizes BLE long lived connection for latency
                                                    "-dlrv_cfg <is_set> <dlrv_time> [<nwsync_time>]"
                                                        <is_set> - 0 or 1
                                                          0 - Get current dlrv time
                                                          1 - Set dlrv time
                                                        <dlrv_time> - DL route validity time in mins, range: [1,1440]
                                                        <nwsync_time> - SID_LINK3_PROFILE_B nw sync time in mins, It can only be set if dlrv_time has been changed, range: [1,240]
                                                    "-ble_d2d_only <cmd> <enable> " - BLE D2D Config
                                                          <cmd>     - Get/Set command
                                                            0 - Get Current D2D state
                                                            1 - Set D2D state
                                                              <enable>
                                                                0 - Disable D2D
                                                                1 - Enable D2D
    sid last_status                                 - get last status of Sidewalk library. Result is printed in the same format as
                                                      EVENT SID STATUS. This call the sid_get_status() API.
    sid conn_req <0,1>                              - set the connection request bit in BLE beacon,
                                                      1 - set connection request bit
                                                      0 - clear connection request bit
                                                      This calls the sid_ble_bcn_connection_request() API.
    sid get_time <0>                                - get time from Sidewalk library,
                                                      0 - GPS time
                                                      This calls the sid_get_time() API.
    sid set_dst_id <id>                             - set message destination ID. This calls the sid_set_msg_dest_id() API.
    sid ble_only                                    - set ble only would force the semtech board into sleep mode.
                                                      deinit will reset ble_only flag to default(false).
    sid ble_last_conn                               - get the last ble connection parameters in use.
    sid pwr_meas_start <un> <ul> <sd> <mt> <mode>   - trigger power measurement mode, command takes control over main loop, allows to go to idle state and put
                                                      device into sleep mode, during command execution CLI is not processed and not available, every 5 seconds
                                                      command can add uplink packets to send queue
                                                      un    - number of uplink packets generated during command execution, max 255
                                                      ul    - length of one uplink packet, max 255, it accompanies with number of uplink packets
                                                      sd    - start delay time in seconds, 
                                                              in mode 1, after this time command will start to add ul packets to send queue  max 4000 sec
                                                              in mode 2, after this time the EN starts connection establishing process.
                                                      mt    - measurement time in seconds, time after which command will end, max 4000 sec, it accompanies with start delay time
                                                      mode  - indicates the behavior of the command
                                                                0 - sends packets defined by <un> and <ul> only. The interface shall be started and connection established e.g. for BLE
                                                                    by "sid start 1" and "sid conn_req 1" commands before the command is called.
                                                                    The value is applicable for all link types: BLE, FSK and LoRa.
                                                                1 - starts the interface and initiates the connection (like commands sid start 1, sid conn_req 1)
                                                                    before packets defined by <un> and <ul> are sent. I also stops the interface when the measurement is
                                                                    ended (like sid stop).
                                                                    The value is applicable for BLE link type only.
                                                                2 - perform a connection establishment for FSK/Lora and terminates after the FSK/Lora is linked.
   sid up_test -e <en> -l <lm> -s <size> -n <itr>   - Start Uplink test. Sends a fixed payload of messages at a periodicity and number of iterations below
    -p <period>  -a <ack> <retry> <ttl>
                                                      en - start = 1 stop = 0
                                                      lm - link mask. check sid send above
                                                      size - Size of the fixed messsage
                                                      itr  - Total number of messages that need to be sent
                                                      period - delay between one message and the other in seconds
                                                      <ack>  Ack required or not
                                                      <retry> Number of retries per message
                                                      <ttl> Total time to live in seconds
                                                      For example sid up_test -e 1 -l 1 -s 210 -n 10 -p 5 -a 1 3 30 will start the uplink test on BLE link with size of each message be 210 bytes
                                                      for a total of 10 messages with each message sent every 5 seconds with message parameters of ack required set to true, total retires set to 3
                                                      and time to live set to 30 seconds
                                                      sid up_test -e 0 will print the results of the test that was run above
    sid sdk_config                                  - Prints the various sid sdk config values
    sid network_coverage_set_cb                     - set the callback to get the gps co-ordinates from application
    sid network_coverage_config -i <ti> -d <duration> -t <test_type> -l <link> -r <role> -c <consent> -p <direction> -a <dest>
                                                    - Configures the network coverage parameters for running the test using sid network_coverage_test <options>
                                                      ti        - transmit interval in seconds
                                                      duration  - duration of test in seconds
                                                      test_type - type of test
                                                                    0  - ping ping with summary report
                                                                    1  - detailed test with notifying to mobile over d2d (Applicable only with mobile application)
                                                                    3  - sniffer (not supported)
                                                      link      - link to run the test
                                                                    0  - SUB_GHZ_LORA
                                                                    1  - SUB_GHZ_FSK
                                                                    2  - BLE (not supported)
                                                                    14 - AUTO_SUB_GHZ
                                                                    15 - AUTO_ALL_PHY (not supported)
                                                      role      - transmitter or receiver
                                                                    0  - receiver
                                                                    1  - transmitter
                                                      consent   - consent to use on device gps
                                                                    0  - no consent
                                                                    1  - consent
                                                      direction - direction of communication
                                                                    0  - unidirectional (not supported)
                                                                    1  - bidirectional
                                                      dest      - destination to send ping/pong messages
                                                                    40 - connectivity service
    sid network_coverage_test <1,2,3>               - start, stop the network coverage test and get the current status of the test
                                                      1 - start network coverage test with the configuration provided using sid network_coverage_config
                                                      2 - stop network coverage test
                                                      3 - check current status of network coverage test (in-progress / stopped)
    sid reg_inject -i <type> -e <error> -r <reset>    - Injects errors into the regsitration/key refresh process
                                                      type      - type of error to inject
                                                                    1 - WAN Key Write error (key is not written to flash, only to RAM)
                                                                    2 - App Key Write error (key is not written to flash, only to RAM)
                                                                    3 - Ecdh sig error (after key is written to RAM)
                                                                    4 - Drop Handshake command (before it is processed)
                                                                    5 - Skip Sending Handhshake response to cloud
                                                                    6 - Fail config complete
                                                      error     - Error code to inject
                                                      reset     - reset all values
    sid state -a <gw>                                  - Injects sid state change
                                                        0 -> SID_STATE_LINK_TYPE_1, AUTH, GW - gateway type, Remote - Cloud
                                                            - GW : SID_STATE_GATEWAY_TYPE_WAN_MOBILE_OPTED_OUT (15)
                                                                 : SID_STATE_GATEWAY_TYPE_WAN_MOBILE_OPTED_IN (13) 
                                                                 : SID_STATE_GATEWAY_TYPE_WAN_STATIC_OPTED_OUT (11)
                                                                 : SID_STATE_GATEWAY_TYPE_WAN_STATIC_OPTED_IN (9)
                                                                 : SID_STATE_GATEWAY_TYPE_PAN_MOBILE_OPTED_OUT(7)
                                                                 : SID_STATE_GATEWAY_TYPE_PAN_MOBILE_OPTED_IN (5)
                                                                 : SID_STATE_GATEWAY_TYPE_PAN_STATIC_OPTED_OUT (3)
                                                                 : SID_STATE_GATEWAY_TYPE_PAN_STATIC_OPTED_IN (1)

    sid set_time_sync_intervals <interval values in hours>   - Set the time sync intervals array.
                                                      Example- sid set_time_sync_intervals 2 8 48 96
                                                      Now, time sync will be triggered after 2 hours, then after 8 hours(from prev sync), then after 48 and so on.
                                                      Maximum 10 values can be provided in this array.
    sid get_time_sync_intervals                     - Get the current sync intervals array.
    sid set_ps1                                     - Turn on power saving while on Lora Link. Lora would be automatically de-linked and deinit.
    sid set_ps0                                     - Turn off power saving while on Lora Link. Lora would be automatically init and link. timesync should be able to retrive by the EP itselft without request from cloud.

    reboot                                          - implements a way to reboot the device given that the reboot function has been passed as a part of initialization

    sbdt init                                       - Initialize the sidewalk bulk data stack, this can only be done after sid init is done
    sbdt cancel <file_id> <reason>                  - This will send a cancel resp with the given file_id, with the given cancel reason
                                                            0x0 - None
                                                            0x1 - Generic
                                                            0x3 - File is too big
                                                            0x4 - No space for file on the device
                                                            0x5 - Device has low battery
                                                            0x9 - File verification failed
                                                            0xB - File already exists
                                                            0xE - Invalid block size
    sbdt deinit                                     - This will deinit bulk data transfer stack, will send cancel for any ongoing bulk data transfers
    sbdt cfg -p -r -fd <finalize resp delay> -fs <finalize response accept> -tr <transfer reequest accept> -trs <transfer request reject reasons>
                                                    - This will allow use to manipulate bulk data transfer, by rejecting or accepting transfers or implying successfull verification of file transfer
                                                    -p will print the existing values for the various cfg variables
                                                    -r will reset the cfg values back to default
                                                    -fd Will add a delay in responding to file transfer finalize request, it's in seconds
                                                    -fs Will allow user set success (0) or failure (1) the file transfer
                                                            0 - Sucess
                                                            1 - Failure
                                                    -tr Will allow user to accept (0) or reject (1) the upcoming file transfer
                                                            0 - Accept
                                                            1 - Reject
                                                    -trs Will allow user to set a reject reason
                                                            0x0 - None
                                                            0x1 - Generic
                                                            0x3 - File is too big
                                                            0x4 - No space for file on the device
                                                            0x5 - Device has low battery
                                                            0x9 - File verification failed
                                                            0xB - File already exists
                                                            0xE - Invalid block size
                                                    -br Delay the release of the buffer by ms
                                                            ms - Time to delay the release of the buffer
                                                    -fc Will allow user to fail a chunk
                                                            <num> - Fail the chunk number
                                                    -fb Will allow user to retry a block
                                                            <bk> - Retry a block


    sbdt stats <file_id>                            - Prints the progress of the transfer as a percent and current file offset
    sbdt params <file_id>                           - Prints the various parameters relavant to file transfer

    dult init                                       - Initializes sidewalk unwanted location tracking component
    dult deinit                                     - Deinitializes sidewalk unwanted location tracking
    dult config -r -p <PRODUCT_DATA> -m <MANUFACTURER_DATA> -o <MODEL> -i <IDENTIFIER> -f <MAJOR VERSION> <MINOR VERSION> <REVISION> -c <CATEGORY ID> -n <NETWORK ID> -b <BATTERY_TYPE> -a <ADV MODE>
                                                    - Will allow for configuration of various dult config information
                                                    - <No Arguments> Will print the existing/default config
                                                    -r reset the config to default
                                                    -p Set the product_data, maximum is 8 bytes
                                                    -m Set the manufacturer name, maximum is 64 bytes
                                                    -o Set the model name, maximum is 64 bytes
                                                    -i Set the device identifier, maximum is 32 bytes
                                                    -f Sets the major, minor, revision of the firmware, major max is 2 bytes, minor max is 1 byte and revision is 1 byte
                                                    -c Sets the category id see https://www.ietf.org/archive/id/draft-detecting-unwanted-location-trackers-01.html#name-accessory-category-value
                                                    -n Sets the network id, 1 - Apple, 2 - Google, 3 - Sidewalk (Tentative)
                                                    -b Sets the battery type, 0 = Powered, 1 = Non-rechargeable battery, 2 = Rechargeable battery
                                                    -a Sets the advertising mode, 0 will only advertise DULT in separated mode, 1 will advertise in both modes
                                                    -im Sets the initial proximity state, 0 is SEPARATED, 1 is NEAR mode
    dult set -r -m <proximity state> -o <ignore non owner events> -s <non owner find status> -i <enter identifier read mode> -l <BATTERY_LEVEL> 
                                                    - Configure the behaviour of DULT
                                                    - <No Arguments> Will print the existing/default config
                                                    -r reset to default
                                                    -m Set the proximity state, 0 - Separated mode, 1 - Near mode
                                                    -o Ignore if gatt requests for non owner find events such as Sound on/off etc, 0 - Do not ignore events respond with status defined by non_owner_find_status, 1 - ignore events
                                                    -s Status of the non owner find event, 0 - Failure, 1 - Success
                                                    -i Enter get identifier state of dult, if this is not set, then the GET_IDENTIFIER will fail
                                                    -l Sets the battery level, 0 = Full, 1 = Medium, 2 = Low, 3 = Critically low

    location init                                   - Initializes sidewalk location library
    location deinit                                 - deinitializes sidewalk location library
    location send <LOCATION_LEVEL>                  - Send a location uplink at the specified level 1-4. No level
                                                    specified will use an automatic scaling mode, using lowest power
                                                    mode first.
                                                    1 - BLE Uplink using Sidewalk Network location
                                                    2 - LoRa Triangulation (not supported)
                                                    3 - WiFi
                                                    4 - GNSS

    location send_buf <LOCATION_LEVEL>              - Send a test buffer via an available link without re-running a scan. The buffer is a 50 byte payload that does not contain a valid location.
                                                    3 - WiFi
                                                    4 - GNSS
    location scan <LOCATION_LEVEL>                  - Conduct location scan for levels 3-4 (WiFi and GNSS) without sending.
                                                    3 - WiFi
                                                    4 - GNSS
    location alm_start                              - start the almanac demodulation service to update the GNSS
                                                    almanac via satellite. The service cannot currently be stopped.

    gwscan start [<duration_sec>] [<max_gw_num>]    - Start GW scaning tool, this can only be done before sid init. This tool is only available for FSK-enabled builds.
                                                        Format: gwscan start [<duration_sec>] [<max_gw_num>]
                                                        Parameters:
                                                            <duration_sec> -[optional] This parameter is used to specify the scanning duration. Range: 1-65535, default value: 0 (infinity)
                                                            <max_gw_num>  -[optional] Maximum number of discovered gateways, range: 1-1000, default value: 100
                                                            If the parameters are not specified, then scanning will continue until it is stopped by stop command.
    gwscan stop                                     - Stop GW scaning tool and print results
                                                        Format: gwscan stop
    sid set_diag_log_cfg <log_sev> <sdk_log_size> <app_log_size>
                                                    - Sets the init configuration of sid diag logging. command to be run before every sid init for the new setting to take effect
                                                       <log_sev> Sets the sid_diag_log_level.
                                                        0 : SID_DIAG_LOG_LEVEL_NOT_SET (defaults to INFO)
                                                        1 : SID_DIAG_LOG_LEVEL_ERROR
                                                        2 : SID_DIAG_LOG_LEVEL_INFO
                                                        3 : SID_DIAG_LOG_LEVEL_DEBUG
                                                       <sdk_log_size> Size of Log buffer that the DUT application will allocate for SDK logs
                                                       <app_log_size> Size of Log buffer that the DUT application will allocate for application logs

### CLI commands response
Each command after calling sid API function will print return value and result of the API execution.
Sidewalk APIs are called only when all command input parameters are valid.
Each command can return generinc result:
```
CMD ERR: <value> - ERR value is the value returned by Sidewalk API (sid_error_t)
```

Some commands in result can include some additional information returned by Sidewalk APIs:

```
sid get_mtu 0
CMD ERR: 0 MTU: 255

sid send Test
CMD ERR: 0 TYPE: 0 ID: 4                        - TYPE - message type
                                                - ID - id associated with message

sid get_time 0
CMD: ERR: 0 SEC: 1308486906 NSEC: 857635511     - SEC - seconds
                                                - NSEC - nanoseconds
```

### CLI Sidewalk events

QA CLI will print event whenever one of the callbacks is invoked.

```
EVENT SID STATUS                - printed when Sidewalk library state changes. Invoked by on_status_change() callback e.g.
                                  EVENT SID STATUS: State: 1, Reg: 1, Time: 1, Link_Mask: 1
                                    State - state of Sidewalk library (0 - ready, 1 - not ready, 2 - error)
                                    Req - registration status (0 - registered, 1 - no registered)
                                    Time - time synchronization status (0 - synced, 1 - not synced)
                                    Link_Mask - this is a bit field describing which of the links is up. If the bit corresponding to
                                                a link is set it is up, otherwise it is down. For example to check if SID_LINK_TYPE1 is up
                                                (Link_Mask & SID_LINK_TYPE_1) must not be 0.
EVENT SID STATUS LINKS          - printed when Sidewalk library state changes. Invoked by on_status_change() callback. This expands the
                                  Link_Mask bit field to show which link types are currently available. e.g
                                  EVENT SID STATUS LINKS: LoRa: 1, FSK: 0, BLE: 1
                                  EVENT SID STATUS LINK 0 MODE 2
                                  EVENT SID STATUS LINK 1 MODE 0
                                  EVENT SID STATUS LINK 2 MODE 1
                                    LoRa - whether the LoRa link is available (0 - not available, 1 - available)
                                    FSK - whether the FSK link is available (0 - not available, 1 - available)
                                    BLE - whether the BLE link is available (0 - not available, 1 - available)
                                    LINK - (0 - BLE, 1 - FSK, 2 - LoRa)
                                    MODE - supported mode on respective links
                                           (0 - not available, 1 - Destination Cloud, 2 - Destination Mobile, 3 - Destination Mobile and cloud)
EVENT SID SEND STATUS           - printed when message was send to network or in case of send error. Invoked by on_msg_send callback e.g.
                                  EVENT SID SEND STATUS: SID ERR: 0, TYPE: 0, ID: 4
                                    SID ERR - Sidewalk error
                                    TYPE - message type
                                    ID - id associated with message
EVENT SID RECEIVED              - printed when message from network is received. Invoked by on_msg_received() callback e.g.
                                  EVENT SID RECEIVED: TYPE: 0, ID: 2, LEN: 5
                                  Data: 0102030405
                                    TYPE - message type
                                    ID - id associated with message
                                    LEN - length of message payload in bytes
                                    Data - payload in hex format
EVENT SID FACTORY RESET         - printed when library performed factory reset. Invoked by on_factory_reset callback.
EVENT SID RESET                 - printed after device bootup, this is not triggered by Sidewalk library but generated by QA CLI. This is needed for test framework.
```

### Typical Usage
Once the manufacturing page is flashed onto the device, the app which includes this functionality should be
flashed onto the board. The following commands should then be run:

```
// init sidewalk with SID_LINK_TYPE_2 (FSK)
sid init 2
sid start
```

At this point the board will either be ready to be registered over BLE or if registered already, will
attempt to trigger time sync in order to be able to send messages. Note: before any sid commands can
be run, sid init and sid start must be called.

### Typical Usage (Device Profile Settings)
The following commands will help to set and get the devie profile related information.
```
/**
 * Describes the profile type of the device
 */
enum sid_device_profile_id {
    /** Device Profile ID for Synchronous Network */
    SID_LINK2_PROFILE_1 = 0x01,
    SID_LINK2_PROFILE_2 = 0x02,

    /** Device Profile ID for Asynchronous Network */
    SID_LINK3_PROFILE_A = 0x80,
    SID_LINK3_PROFILE_B = 0x81,
    SID_LINK3_PROFILE_D = 0x83
};

/**
 * Describes the number of RX windows opened by the device
 */
enum sid_rx_window_count {
    /** Used to indicate device opens infinite RX windows */
    SID_RX_WINDOW_CNT_INFINITE = 0,
    /** Used to indicate device opens 5 RX windows */
    SID_RX_WINDOW_CNT_2 = 5,
    SID_RX_WINDOW_CNT_3 = 10,
    SID_RX_WINDOW_CNT_4 = 15,
    SID_RX_WINDOW_CNT_5 = 20,
   /** Used to indicate device is in continuous RX mode */
    SID_RX_WINDOW_CONTINUOUS = 0xFFFF,
};

/**
 * Describes the low latency setting for Asynchronous profile
 */
enum sid_link3_low_latency {
    SID_LINK3_LOW_LATENCY_DISABLE = 0,
    SID_LINK3_LOW_LATENCY_ENABLE = 1,
    SID_LINK3_LOW_LATENCY_LAST,
};

/**
 * Describes the frequency of RX windows opened by the device (in ms) in synchronous mode
 */
enum sid_link2_rx_window_separation_ms {
    /** Used to indicate device opens a RX window every 63 ms */
    SID_LINK2_RX_WINDOW_SEPARATION_1 = 63,
    /** Used to indicate device opens a RX window every 315 (63*5) ms */
    SID_LINK2_RX_WINDOW_SEPARATION_2 = (SID_LINK2_RX_WINDOW_SEPARATION_1 * 5),
    /** Used to indicate device opens a RX window every 630 (63*10) ms */
    SID_LINK2_RX_WINDOW_SEPARATION_3 = (SID_LINK2_RX_WINDOW_SEPARATION_1 * 10),
    /** Used to indicate device opens a RX window every 945 (63*15) ms */
    SID_LINK2_RX_WINDOW_SEPARATION_4 = (SID_LINK2_RX_WINDOW_SEPARATION_1 * 15),
    /** Used to indicate device opens a RX window every 2520 (63*40) ms */
    SID_LINK2_RX_WINDOW_SEPARATION_5 = (SID_LINK2_RX_WINDOW_SEPARATION_1 * 40),
    /** Used to indicate device opens a RX window every 3150 (63*50) ms */
    SID_LINK2_RX_WINDOW_SEPARATION_6 = (SID_LINK2_RX_WINDOW_SEPARATION_1 * 50),
    /** Used to indicate device opens a RX window every 5040 (63*80) ms */
    SID_LINK2_RX_WINDOW_SEPARATION_7 = (SID_LINK2_RX_WINDOW_SEPARATION_1 * 80),
};

/**
 * Set and get profile setting for synchronous network
 */
// Set device profile to SID_LINK2_PROFILE_1
sid option -lp_set 1
sid option -lp_get_l2
<info> app: CMD: ERR: 0 Link_profile ID: 1 Wndw_cnt: 0

// Set device profile to SID_LINK2_PROFILE_2 with DL interval 5040ms between rx opportunities (SID_LINK2_RX_WINDOW_SEPARATION_7)
sid option -lp_set 2 5040
sid option -lp_get_l2
<info> app: CMD: ERR: 0 Link_profile ID: 2 Wndw_cnt: 0 Rx_Int = 5040

/**
 * Set and get profile setting for asynchronous network
 */
// Set device profile to SID_LINK3_PROFILE_A with window count 5 (SID_RX_WINDOW_CNT_2) and SID_LINK3_LOW_LATENCY_DISABLE
sid option -lp_set 128 5 0
// Set device profile to SID_LINK3_PROFILE_A with window count 5 (SID_RX_WINDOW_CNT_2) and SID_LINK3_LOW_LATENCY_ENABLE
sid option -lp_set 128 5 1
sid option -lp_get_l3
<info> app: CMD: ERR: 0 Link_profile ID: 129 Wndw_cnt: 0 Low_Latency = 1

// Set device profile to SID_LINK3_PROFILE_B with infinite window count (SID_RX_WINDOW_CNT_INFINITE) and
// SID_LINK3_LOW_LATENCY_DISABLE
sid option -lp_set 129 0 0
// Set device profile to SID_LINK3_PROFILE_B with infinite window count (SID_RX_WINDOW_CNT_INFINITE) and
// SID_LINK3_LOW_LATENCY_ENABLE
sid option -lp_set 129 0 1
sid option -lp_get_l3
<info> app: CMD: ERR: 0 Link_profile ID: 129 Wndw_cnt: 0 Low_Latency = 1
```
