/********************************************************************************************************
 * @file    sid_ble_adapter.c
 *
 * @brief   This is the source file for BLE SDK
 *
 * @author  BLE GROUP
 * @date    11,2025
 *
 * @par     Copyright (c) 2025, Telink Semiconductor (Shanghai) Co., Ltd. ("TELINK")
 *
 *          Licensed under the Apache License, Version 2.0 (the "License");
 *          you may not use this file except in compliance with the License.
 *          You may obtain a copy of the License at
 *
 *              http://www.apache.org/licenses/LICENSE-2.0
 *
 *          Unless required by applicable law or agreed to in writing, software
 *          distributed under the License is distributed on an "AS IS" BASIS,
 *          WITHOUT WARRANTIES OR CONDITIONS OF ANY KIND, either express or implied.
 *          See the License for the specific language governing permissions and
 *          limitations under the License.
 *
 *******************************************************************************************************/

/** @file sid_ble_adapter.c
 *  @brief Bluetooth low energy adapter implementation.
 */

#include "tl_common.h"
#include "drivers.h"
#include "stack/ble/ble.h"
#include "sid_ble_adapter.h"
#include <sid_error.h>
#include <sid_pal_ble_adapter_ifc.h>
#include <sid_ble_service.h>
#include <sid_ble_ama_service.h>
#if defined(CONFIG_SIDEWALK_VENDOR_SERVICE)
#include <sid_ble_vnd_service.h>
#endif /* CONFIG_SIDEWALK_VENDOR_SERVICE */
#if defined(CONFIG_SIDEWALK_LOGGING_SERVICE)
#include <sid_ble_log_service.h>
#endif /* CONFIG_SIDEWALK_LOGGING_SERVICE */
#include <sid_ble_adapter_callbacks.h>
#include <sid_ble_advert.h>
#include <sid_ble_connection.h>


#include <bt_app_callbacks.h>
#if FREERTOS_ENABLE
#include "tlk_riscv.h"
#include <FreeRTOS.h>
#include <task.h>
#endif

struct sid_pal_conn_param
{
    u16 min_conn_interval;
    u16 max_conn_interval;
    u16 slave_latency;
    u16 conn_sup_timeout;
} ;

struct sid_pal_adv_param
{
    bool               ble_adv_fast_enabled;
    bool               ble_adv_slow_enabled;
    uint16_t           ble_adv_fast_interval;
    uint16_t           ble_adv_fast_timeout;
    uint16_t           ble_adv_slow_interval;
    uint16_t           ble_adv_slow_timeout;
} ;

struct sid_pal_ble_adapter_ctx {
    struct sid_pal_adv_param current_adv_config;
    struct sid_pal_conn_param current_conn_config;
    sid_ble_cfg_conn_param_t last_conn_config;
};

static struct sid_pal_ble_adapter_ctx ctx = {};

static sid_error_t ble_adapter_init(const sid_ble_config_t *cfg);
static sid_error_t ble_adapter_start_service(void);
static sid_error_t ble_adapter_set_adv_data(uint8_t *data, uint8_t length);
static sid_error_t ble_adapter_start_advertisement(void);
static sid_error_t ble_adapter_stop_advertisement(void);
static sid_error_t ble_adapter_send_data(sid_ble_cfg_service_identifier_t id, uint8_t *data,
                     uint16_t length);
static sid_error_t ble_adapter_set_callback(const sid_pal_ble_adapter_callbacks_t *cb);
static sid_error_t ble_adapter_disconnect(void);
static sid_error_t ble_adapter_deinit(void);
static sid_error_t ble_adapter_get_rssi(int8_t *rssi);
static sid_error_t ble_adapter_get_tx_pwr(int16_t *tx_power);
static sid_error_t ble_adapter_set_tx_pwr(int16_t tx_power);
static sid_error_t ble_get_mac_addr(uint8_t *addr);
static void ble_received_data_result(sid_error_t result);
static void ble_notify_ama_state(bool is_active);
static sid_error_t ble_adapter_apply_user_config(sid_ble_user_config_t *cfg);
bool sid_ble_is_enable(void);
int sid_ble_advert_start_with_param(adv_inter_t intervalMin, adv_inter_t intervalMax, adv_type_t advType, own_addr_type_t ownAddrType,  \
        u8 peerAddrType, u8 *peerAddr, adv_chn_map_t adv_channelMap, adv_fp_type_t advFilterPolicy);
int sid_ble_get_adv_param(uint32_t* ble_adv_fast_interval, uint32_t* ble_adv_fast_timeout, \
        uint32_t * ble_adv_slow_interval, uint32_t*  ble_adv_slow_timeout);
int sid_ble_is_adv_init(void);
_attribute_ble_data_retention_ int16_t g_tx_pwr = 3;

static struct sid_pal_ble_adapter_interface ble_ifc = {
    .init = ble_adapter_init,
    .start_service = ble_adapter_start_service,
    .set_adv_data = ble_adapter_set_adv_data,
    .start_adv = ble_adapter_start_advertisement,
    .stop_adv = ble_adapter_stop_advertisement,
    .send = ble_adapter_send_data,
    .set_callback = ble_adapter_set_callback,
    .disconnect = ble_adapter_disconnect,
    .deinit = ble_adapter_deinit,
    .get_rssi = ble_adapter_get_rssi,
    .get_tx_pwr = ble_adapter_get_tx_pwr,
    .set_tx_pwr = ble_adapter_set_tx_pwr,
    .received_data_result = ble_received_data_result,
    .get_mac_addr = ble_get_mac_addr,
    .notify_ama_state = ble_notify_ama_state,
    .user_config = ble_adapter_apply_user_config,
};



static sid_error_t ble_adapter_update_adv_param(sid_ble_cfg_adv_param_t *adv_param)
{
    if (!adv_param) {
        return SID_ERROR_NULL_POINTER;
    }
    if(!sid_ble_is_enable())
    {
        return SID_ERROR_INVALID_STATE;
    }
    sid_ble_set_adv_param(adv_param->fast_interval,adv_param->fast_timeout,adv_param->slow_interval,adv_param->slow_timeout);
    sid_error_t ret_code = SID_ERROR_NONE;
    if (sid_ble_get_adv_state() !=0 ){
        ret_code = sid_ble_advert_stop();
        if (ret_code != SID_ERROR_NONE) {
            return ret_code;
        }
        ret_code = sid_ble_advert_start_with_param(adv_param->fast_interval,adv_param->fast_interval,ADV_TYPE_CONNECTABLE_UNDIRECTED, OWN_ADDRESS_RANDOM, 0, NULL, BLT_ENABLE_ADV_ALL, ADV_FP_NONE);
    }
    ctx.current_adv_config.ble_adv_fast_enabled  = adv_param->fast_enabled;
    ctx.current_adv_config.ble_adv_fast_interval = adv_param->fast_interval;
    ctx.current_adv_config.ble_adv_fast_timeout  = adv_param->fast_timeout;
    ctx.current_adv_config.ble_adv_slow_enabled  = adv_param->slow_enabled;
    ctx.current_adv_config.ble_adv_slow_interval = adv_param->slow_interval;
    ctx.current_adv_config.ble_adv_slow_timeout  = adv_param->slow_timeout;

    return ret_code;
}

static sid_error_t ble_adapter_get_adv_param(sid_ble_cfg_adv_param_t *adv_param)
{
    if(!sid_ble_is_enable())
    {
        return SID_ERROR_UNINITIALIZED;
    }

    if (sid_ble_is_adv_init() == 0) {
        //If ble_advertising not init, send out the user settings.
        adv_param->fast_interval = ctx.current_adv_config.ble_adv_fast_interval;
        adv_param->fast_timeout = ctx.current_adv_config.ble_adv_fast_timeout;
        adv_param->slow_interval = ctx.current_adv_config.ble_adv_slow_interval;
        adv_param->slow_timeout = ctx.current_adv_config.ble_adv_slow_timeout;
    } else {
        sid_ble_get_adv_param(&adv_param->fast_interval, &adv_param->slow_interval,&adv_param->fast_timeout,  &adv_param->slow_timeout);
     }

    return SID_ERROR_NONE;
}

int ble_adpter_set_last_conn_param(uint16_t min_conn_interval, uint16_t max_conn_interval,
uint16_t slave_latency, uint16_t conn_sup_timeout)
{
    ctx.last_conn_config.conn_sup_timeout  = conn_sup_timeout;
    ctx.last_conn_config.max_conn_interval = max_conn_interval;
    ctx.last_conn_config.min_conn_interval = min_conn_interval;
    ctx.last_conn_config.slave_latency = slave_latency;
    return 0;
}

static sid_error_t ble_adapter_get_last_conn_param(sid_ble_cfg_conn_param_t *conn_param)
{
    sid_ble_conn_params_t * param = sid_ble_conn_params_get();
    if (param == NULL) {
        return SID_ERROR_GENERIC;
    }

    *conn_param = ctx.last_conn_config;

    return SID_ERROR_NONE;
}

static sid_error_t ble_adapter_update_conn_param(sid_ble_cfg_conn_param_t *conn_param)
{
    if (!conn_param) {
        return SID_ERROR_NULL_POINTER;
    }

    if (!sid_ble_is_enable()) {
        return SID_ERROR_UNINITIALIZED;
    }

    struct sid_pal_conn_param user_conn_config;
    user_conn_config.min_conn_interval = conn_param->min_conn_interval;
    user_conn_config.max_conn_interval = conn_param->max_conn_interval;
    user_conn_config.slave_latency = conn_param->slave_latency;
    user_conn_config.conn_sup_timeout = conn_param->conn_sup_timeout;
    sid_ble_conn_params_t * param = sid_ble_conn_params_get();
    if (param == NULL) {
        return SID_ERROR_GENERIC;
    }

    if (param ->connHandle != 0xFFFF) {
//        u8 bls_l2cap_requestConnParamUpdate(u16 connHandle, u16 min_interval, u16 max_interval, u16 latency, u16 timeout);
        if (bls_l2cap_requestConnParamUpdate(param ->connHandle,conn_param->min_conn_interval,conn_param->max_conn_interval,conn_param->slave_latency,conn_param->conn_sup_timeout)!= 0 ) {
            return SID_ERROR_GENERIC;
        }
    }

    ctx.current_conn_config = user_conn_config;

    return SID_ERROR_NONE;
}


static sid_error_t ble_adapter_apply_adv_user_config(sid_ble_user_config_t *cfg)
{
    if (cfg->is_set) {
        return ble_adapter_update_adv_param(&cfg->adv_param);
    } else {
        return ble_adapter_get_adv_param(&cfg->adv_param);
    }
}

static sid_error_t ble_adapter_apply_conn_user_config(sid_ble_user_config_t *cfg)
{
    if (cfg->is_set) {
        return ble_adapter_update_conn_param(&cfg->conn_param);
    } else {
        return ble_adapter_get_last_conn_param(&cfg->conn_param);
    }
}

static sid_error_t ble_adapter_apply_all_user_config(sid_ble_user_config_t *cfg)
{
    sid_error_t ret_code = ble_adapter_apply_adv_user_config(cfg);
    if (ret_code == SID_ERROR_NONE) {
        ret_code = ble_adapter_apply_conn_user_config(cfg);
    }
    return ret_code;
}

static sid_error_t ble_adapter_apply_user_config(sid_ble_user_config_t *cfg)
{
    if (!cfg) {
        return SID_ERROR_NULL_POINTER;
    }
    switch (cfg->cfg_type) {
        case SID_BLE_USER_CFG_ADV:
            return ble_adapter_apply_adv_user_config(cfg);
        case SID_BLE_USER_CFG_CONN:
            return ble_adapter_apply_conn_user_config(cfg);
        case SID_BLE_USER_CFG_ADV_AND_CONN:
            return ble_adapter_apply_all_user_config(cfg);
        default:
            return SID_ERROR_INCOMPATIBLE_PARAMS;
    }
    return SID_ERROR_NONE;
}


static sid_error_t ble_get_mac_addr(uint8_t *addr)
{
   TL_LOG_D("ble_get_mac_addr ");
   //extern u8 ble_mac_public[];
   memset(addr,0,6);
//   addr[0] = ble_mac_public[5];
//   addr[1] = ble_mac_public[4];
//   addr[2] = ble_mac_public[3];
//   addr[3] = ble_mac_public[2];
//   addr[4] = ble_mac_public[1];
//   addr[5] = ble_mac_public[0];
//   memmove(addr,ble_mac_public,6);
   return 0;
}

static void ble_received_data_result(sid_error_t result)
{
    ARG_UNUSED(result);
    TL_LOG_D("ble_received_data_result = %d", result);
}

static void ble_notify_ama_state(bool is_active)
{
    ARG_UNUSED(is_active);
    TL_LOG_D("ble_notify_ama_state = %d", is_active);
}


static sid_error_t ble_adapter_get_rssi(int8_t *rssi)
{
    uint16_t sid_get_connhandle(void);
    uint16_t handler = sid_get_connhandle();
    if(handler != 0xFFFF)
    //*rssi = rf_get_real_time_rssi();
    *rssi = blc_ll_getAclLatestRSSI(sid_get_connhandle()) -110;

    else
        *rssi = 0;

    TL_LOG_D("BLE RSSI = %d", *rssi);
    return SID_ERROR_NONE;
}

static sid_error_t ble_adapter_get_tx_pwr(int16_t *tx_power)
{
    *tx_power = g_tx_pwr;
    TL_LOG_D("BLE get tx pwr: %d", *tx_power);
    return SID_ERROR_NONE;
}

static sid_error_t ble_adapter_set_tx_pwr(int16_t tx_power)
{
    rf_power_level_index_e idx;
    g_tx_pwr = tx_power;
    if(tx_power < -50)
        idx = RF_POWER_INDEX_N48p00dBm;
    else  if(tx_power < -25)
        idx = RF_POWER_INDEX_N21p00dBm;
    else  if(tx_power < -10)
        idx = RF_POWER_INDEX_N9p50dBm;
    else  if(tx_power < 0)
        idx = RF_POWER_INDEX_P0p00dBm;
    else  if(tx_power < 1)
        idx = RF_POWER_INDEX_P1p00dBm;
    else  if(tx_power < 2)
        idx = RF_POWER_INDEX_P2p00dBm;
    else  if(tx_power < 3)
        idx = RF_POWER_INDEX_P3p00dBm;
    else  if(tx_power < 4)
        idx = RF_POWER_INDEX_P4p00dBm;
    else  if(tx_power < 5)
        idx = RF_POWER_INDEX_P5p00dBm;
    else  if(tx_power < 6)
        idx = RF_POWER_INDEX_P6p00dBm;
    else  if(tx_power < 7)
        idx = RF_POWER_INDEX_P7p00dBm;
    else  if(tx_power < 8)
        idx = RF_POWER_INDEX_P8p00dBm;
    else  if(tx_power < 9)
        idx = RF_POWER_INDEX_P9p00dBm;
    else  if(tx_power < 10)
        idx = RF_POWER_INDEX_P10p00dBm;
    else
        idx = RF_POWER_INDEX_P10p50dBm;
    rf_set_power_level_index(idx);
    TL_LOG_D("BLE set tx pwr: %d %d", tx_power,idx);
    return SID_ERROR_NONE;
}


static sid_error_t ble_adapter_init(const sid_ble_config_t *cfg)
{
    TL_LOG_D("Sidewalk -> BLE");
    ARG_UNUSED(cfg);

    TL_LOG_I("Enable BT");
    int err_code;
    err_code = sid_ble_bt_enable();
    switch (err_code) {
    case -EALREADY:
    case 0:
        TL_LOG_I("BT initialized");
        break;
    default:
        TL_LOG_E("BT init failed (err %d)", err_code);
        return SID_ERROR_GENERIC;
    }

    err_code = sid_ble_advert_init();
    if (err_code) {
        TL_LOG_E("BT Advertisement failed (err: %d)", err_code);
        return SID_ERROR_GENERIC;
    }

    sid_ble_conn_init();
    TL_LOG_I("BT initialized finished");
    return SID_ERROR_NONE;
}

static sid_error_t ble_adapter_start_service(void)
{
    TL_LOG_D("Sidewalk -> BLE");
    return SID_ERROR_NONE;
}

static sid_error_t ble_adapter_set_adv_data(uint8_t *data, uint8_t length)
{
    TL_LOG_D("Sidewalk -> BLE");
    if (!data || 0 == length) {
        return SID_ERROR_INVALID_ARGS;
    }

    int err = sid_ble_advert_update(data, length);

    if (err) {
        TL_LOG_E("Advertising failed to update (err %d)", err);
        return SID_ERROR_GENERIC;
    }

    return SID_ERROR_NONE;
}

static sid_error_t ble_adapter_start_advertisement(void)
{
    TL_LOG_D("Sidewalk -> BLE");
    int err = sid_ble_advert_start();

    if (err) {
        TL_LOG_E("Advertising failed to start (err %d)", err);
        return SID_ERROR_GENERIC;
    }

    return SID_ERROR_NONE;
}

static sid_error_t ble_adapter_stop_advertisement(void)
{
    TL_LOG_D("Sidewalk -> BLE");
    int err = sid_ble_advert_stop();

    if (err) {
        TL_LOG_E("Advertising failed to stop (err %d)", err);
        return SID_ERROR_GENERIC;
    }

    return SID_ERROR_NONE;
}


static sid_ble_srv_params_t get_srv_params(sid_ble_cfg_service_identifier_t id)
{
    switch (id) {
    case AMA_SERVICE:
        return (sid_ble_srv_params_t){
            .connHandle = sid_ble_conn_params_get()->connHandle,
            .valueHandle = sid_ble_get_ama_service_ccc_valueHandle(),
            .isNotify = 1
        };

#if defined(CONFIG_SIDEWALK_VENDOR_SERVICE)
    case VENDOR_SERVICE:
        return (sid_ble_srv_params_t){
            .connHandle = sid_ble_conn_params_get()->connHandle,
            .valueHandle = sid_ble_get_vnd_service_ccc_valueHandle(),
            .isNotify = 1
        };
#endif /* CONFIG_SIDEWALK_VENDOR_SERVICE */
#if defined(CONFIG_SIDEWALK_LOGGING_SERVICE)
    case LOGGING_SERVICE:
        return (sid_ble_srv_params_t){
            .connHandle = sid_ble_conn_params_get()->connHandle,
            .valueHandle = sid_ble_get_log_service_ccc_valueHandle(),
            .isNotify = 1
        };
#endif /* CONFIG_SIDEWALK_LOGGING_SERVICE */
    default:
        return (sid_ble_srv_params_t){ .connHandle = 0xFFFF, .valueHandle = 0xFFFF, .isNotify = 0 };
    }
}


static sid_error_t ble_adapter_send_data(sid_ble_cfg_service_identifier_t id, uint8_t *data,
                     uint16_t length)
{
    TL_LOG_D("ble_adapter_send_data %d",length);
    sid_ble_srv_params_t srv_params = get_srv_params(id);
    if (srv_params.connHandle == 0xFFFF || srv_params.valueHandle == 0xFFFF) {
        return SID_ERROR_NOSUPPORT;
    }

    int err_code = sid_ble_send_data(&srv_params, data, length);
    if (-EINVAL == err_code) {
        TL_LOG_D("ble_adapter_send_data -EINVAL %d",-EINVAL);
        return SID_ERROR_INVALID_ARGS;
    } else if (0 > err_code) {
        TL_LOG_D("ble_adapter_send_data error %d",err_code);
        return SID_ERROR_GENERIC;
    }
    TL_LOG_D("ble_adapter_send_data success");
    return SID_ERROR_NONE;
}

static sid_error_t ble_adapter_set_callback(const sid_pal_ble_adapter_callbacks_t *cb)
{
    TL_LOG_D("Sidewalk -> BLE");
    sid_error_t err_code;

    if (!cb) {
        return SID_ERROR_NULL_POINTER;
    }

    err_code = sid_ble_adapter_notification_cb_set(cb->ind_callback);
    if (SID_ERROR_NONE != err_code) {
        return err_code;
    }

    err_code = sid_ble_adapter_data_cb_set(cb->data_callback);
    if (SID_ERROR_NONE != err_code) {
        return err_code;
    }

    err_code = sid_ble_adapter_notification_changed_cb_set(cb->notify_callback);
    if (SID_ERROR_NONE != err_code) {
        return err_code;
    }

    err_code = sid_ble_adapter_conn_cb_set(cb->conn_callback);
    if (SID_ERROR_NONE != err_code) {
        return err_code;
    }

    err_code = sid_ble_adapter_mtu_cb_set(cb->mtu_callback);
    if (SID_ERROR_NONE != err_code) {
        return err_code;
    }

    err_code = sid_ble_adapter_adv_start_cb_set(cb->adv_start_callback);
    if (SID_ERROR_NONE != err_code) {
        return err_code;
    }

    return SID_ERROR_NONE;
}

static sid_error_t ble_adapter_disconnect(void)
{
    TL_LOG_D("Sidewalk -> BLE:ble_disconnect");
    int err = sid_ble_conn_disconnect();

    if (err) {
        TL_LOG_E("Disconnection failed (err %d)", err);
        return SID_ERROR_GENERIC;
    }

    return SID_ERROR_NONE;
}

static sid_error_t ble_adapter_deinit(void)
{
    TL_LOG_D("Sidewalk -> BLE");
    sid_ble_conn_deinit();
    sid_ble_advert_deinit();
    int err = sid_ble_bt_disable();

    if (err) {
        TL_LOG_E("BT disable failed (error %d)", err);
        return SID_ERROR_GENERIC;
    }

    return SID_ERROR_NONE;
}

sid_error_t sid_pal_ble_adapter_create(sid_pal_ble_adapter_interface_t *handle)
{
    if (!handle) {
        return SID_ERROR_INVALID_ARGS;
    }

    *handle = &ble_ifc;

    return SID_ERROR_NONE;
}

#if FREERTOS_ENABLE
void * sid_malloc(size_t size)
{
    return pvPortMalloc(size);
}


void sid_free(void * ptr)
{
    vPortFree(ptr);
}
#else
void * sid_malloc(size_t size)
{
    return malloc(size);
}


void sid_free(void * ptr)
{
    free(ptr);
}


void vAssertCalled( const char * pcFile, unsigned long ulLine ){
    ( void ) pcFile; ( void ) ulLine;
    printf("assert fail: %s, %ld\r\n", pcFile, ulLine);
    for( ;; );
}
#endif
