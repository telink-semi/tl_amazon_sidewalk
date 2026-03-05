/********************************************************************************************************
 * @file    sid_ble_advert.c
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
#include "tl_common.h"
#include "drivers.h"
#include "stack/ble/ble.h"
#include "sid_ble_adapter.h"
#include <sid_ble_advert.h>
#include <sid_ble_uuid.h>
#include <sid_ble_uuid.h>
#include <bt_app_callbacks.h>
#if (FREERTOS_ENABLE)
    #include "tlk_riscv.h"
    #include <FreeRTOS.h>
    #include <task.h>
    #include <timers.h>
    #include <queue.h>
    #include <semphr.h>
    #include <event_groups.h>
    #include "app_freertos.h"
#endif


static  uint8_t adv_set = 0;

/**
 * @brief Advertising data items values size in bytes.
 * @note Need to be up to be keep up to date manually.
 */
#define AD_FLAGS_LEN 1
#define AD_SERVICES_LEN 2
#define AD_NAME_SHORT_LEN 2
#define AD_TLV_TYPE_AND_LENGTH 2
#define AD_TLV_LEN(x) (x + AD_TLV_TYPE_AND_LENGTH)
#define ADV_MAX_ADV_DATA_LEN 31

#define AD_MANUF_DATA_LEN_MAX                                                                      \
    (ADV_MAX_ADV_DATA_LEN - AD_TLV_LEN(AD_FLAGS_LEN) -   AD_TLV_LEN(AD_NAME_SHORT_LEN) -      \
     AD_TLV_LEN(AD_SERVICES_LEN) - AD_TLV_TYPE_AND_LENGTH)

enum adv_data_items { ADV_DATA_FLAGS, ADV_DATA_SERVICES, ADV_DATA_MANUF_DATA, ADV_DATA_NAME };

typedef enum { SID_BLE_ADV_DISABLE, BLE_ADV_FAST, BLE_ADV_SLOW } sid_ble_adv_state_t;

//static void change_advertisement_interval(struct work_delayable *work);

//K_WORK_DELAYABLE_DEFINE(change_adv_work, change_advertisement_interval);

_attribute_ble_data_retention_ static  adv_state = SID_BLE_ADV_DISABLE;
_attribute_ble_data_retention_ static  uint8_t sid_device_name[] = "SID_APP";
_attribute_ble_data_retention_ static  uint8_t sid_name_len = 7;
_attribute_ble_data_retention_ static  uint8_t adv_name_real_len = 7;
_attribute_ble_data_retention_ static uint8_t adv_len =   AD_TLV_LEN(AD_FLAGS_LEN) +        \
         AD_TLV_LEN(AD_SERVICES_LEN) + AD_TLV_LEN(AD_NAME_SHORT_LEN) ;

_attribute_ble_data_retention_ uint8_t adv_data[31 + 1] = {
    /* Flags */
    0x02, 1, 0x6,
    /* 16-bit Service UUID */
    0x03, DT_COMPLETE_LIST_16BIT_SERVICE_UUID,
        UUID16_LO(AMA_SERVICE_UUID_VAL), UUID16_HI(AMA_SERVICE_UUID_VAL),

//    0x03, DT_COMPLETE_LOCAL_NAME,
//            'T', 'L',
};

_attribute_ble_data_retention_ uint8_t sd[] = {
    0x08, DT_COMPLETE_LOCAL_NAME,
        'S', 'I','D', '_','A','P', 'P',
};

_attribute_ble_data_retention_ adv_inter_t fast_interval = SID_ADV_MS_TO_INTERVAL(CFG_SIDEWALK_BLE_ADV_INT_FAST) ;
_attribute_ble_data_retention_ adv_inter_t slow_interval = SID_ADV_MS_TO_INTERVAL((CFG_SIDEWALK_BLE_ADV_INT_SLOW+CFG_SIDEWALK_BLE_ADV_INT_PRECISION));
_attribute_ble_data_retention_ uint32_t fast_duration =  CFG_SIDEWALK_BLE_ADV_INT_TRANSITION;
_attribute_ble_data_retention_ uint32_t slow_duration =  0;

#if (FREERTOS_ENABLE)
typedef struct {
    TimerHandle_t   hTimer;
    void (*work)(struct work_delayable *work);
    void *user_data;
    bool pending;
} work_delayable;

static work_delayable s_dwork;

static void change_advertisement_interval( work_delayable* work);


void work_delayable_deinit(work_delayable *dwork)
{
    if(dwork->hTimer != NULL)
    xTimerDelete(dwork->hTimer,0);
    dwork->hTimer = NULL;
    dwork->pending = false;
}

static void work_timer_cb(TimerHandle_t timer)
{
    ARG_UNUSED(timer);
    s_dwork.pending = false;
    s_dwork.work(NULL);
}

void work_delayable_init(work_delayable *dwork,
                         void (*work_cb)(work_delayable *))
{
    dwork->work = work_cb;
    dwork->hTimer = xTimerCreate("adv_delay",                                                 /* Text name. */
                                                          100, /* Timer period. */
                                                          false,                                                                           /* Disable auto reload. */
                                                          dwork,                                                                   /* ID as tagContext */
                                                          work_timer_cb);
    if(NULL == dwork->hTimer  )
        configASSERT(0);

}

void work_delayable_submit(work_delayable *dwork, uint32_t delay_ms)
{
    taskENTER_CRITICAL();
    if (dwork->pending)
        xTimerStop(dwork->hTimer, 0);
    dwork->pending = true;
    if(dwork->hTimer != NULL)
    {
        xTimerChangePeriod(dwork->hTimer, pdMS_TO_TICKS(delay_ms), 0);
    }
    taskEXIT_CRITICAL();
}

bool work_delayable_cancel(work_delayable *dwork)
{
    bool was_pending;
    taskENTER_CRITICAL();
    was_pending = dwork->pending;
    if (was_pending)
        dwork->pending = false;
    taskEXIT_CRITICAL();

    if (was_pending && dwork->hTimer != NULL)
        xTimerStop(dwork->hTimer, 0);

    return was_pending;
}


#endif

/**
 * @brief The function copy manufacturing data to static buffer used in BLE avertising.
 *
 * @param data buffer with data to copy.
 * @param data_len number of bytes to copy.
 *
 * @return number of bytes written to manufacuring data in avertising.
 */
static uint8_t advert_manuf_data_copy(uint8_t *data, uint8_t data_len)
{
    uint16_t ama_id= BT_COMP_ID_AMA;
    uint8_t ama_id_len = sizeof(ama_id);
    uint8_t new_data_len = MIN(data_len, AD_MANUF_DATA_LEN_MAX - ama_id_len);
    uint8_t name_len = ADV_MAX_ADV_DATA_LEN - AD_TLV_LEN(AD_FLAGS_LEN) - AD_TLV_LEN(AD_SERVICES_LEN) - AD_TLV_LEN(new_data_len + ama_id_len) - AD_TLV_TYPE_AND_LENGTH;
    adv_data[AD_TLV_LEN(AD_FLAGS_LEN) + AD_TLV_LEN(AD_SERVICES_LEN)  ] = new_data_len + ama_id_len + 1,
    adv_data[AD_TLV_LEN(AD_FLAGS_LEN) + AD_TLV_LEN(AD_SERVICES_LEN) + 1  ] = 0xFF,
    memcpy(&adv_data[AD_TLV_LEN(AD_FLAGS_LEN) + AD_TLV_LEN(AD_SERVICES_LEN) + 2 ],&ama_id, ama_id_len);
    memcpy(&adv_data[AD_TLV_LEN(AD_FLAGS_LEN) + AD_TLV_LEN(AD_SERVICES_LEN) + ama_id_len + 2],data, new_data_len);

    if(name_len !=  0 )
    {
        adv_name_real_len = MIN(name_len ,sid_name_len);
        adv_data[AD_TLV_LEN(AD_FLAGS_LEN) + AD_TLV_LEN(AD_SERVICES_LEN)  + AD_TLV_LEN(new_data_len + ama_id_len)] = adv_name_real_len +1 ;
        adv_data[AD_TLV_LEN(AD_FLAGS_LEN) + AD_TLV_LEN(AD_SERVICES_LEN) + 1 + AD_TLV_LEN(new_data_len + ama_id_len) ] = DT_SHORTENED_LOCAL_NAME,
        memcpy(&adv_data[AD_TLV_LEN(AD_FLAGS_LEN) + AD_TLV_LEN(AD_SERVICES_LEN) + AD_TLV_LEN(new_data_len + ama_id_len) + 2],sid_device_name, adv_name_real_len);
    }
    else
    {
        adv_name_real_len = 0;
    }

    adv_len =   AD_TLV_LEN(AD_FLAGS_LEN) +        \
             AD_TLV_LEN(AD_SERVICES_LEN) + AD_TLV_LEN( new_data_len + ama_id_len);
    if(adv_name_real_len !=0)
        adv_len += AD_TLV_LEN(adv_name_real_len);

    TL_LOG_I("advert_manuf_data_copy %d %d %d %d %d ",new_data_len,ama_id_len,data_len,AD_MANUF_DATA_LEN_MAX,adv_len);
    return new_data_len + ama_id_len;

}


#if (FREERTOS_ENABLE)
static void change_advertisement_interval(work_delayable* work)
{
    //return ;
    ARG_UNUSED(work);
#else
    static void change_advertisement_interval(void)
    {
#endif
    TL_LOG_I("Change advertisement interval");

    if (BLE_ADV_FAST == adv_state) {
        int err = 0;
        err = blc_ll_setAdvEnable(BLC_ADV_DISABLE);
        if (err) {
            adv_state = SID_BLE_ADV_DISABLE;
            TL_LOG_E("Failed to stop fast adv errno %d ", err);
            return;
        }
        err =  blc_ll_setAdvParam(slow_interval, \
                slow_interval,  \
                ADV_TYPE_CONNECTABLE_UNDIRECTED, OWN_ADDRESS_RANDOM, 0, NULL, BLT_ENABLE_ADV_ALL, ADV_FP_NONE);
        if (err) {
            adv_state = SID_BLE_ADV_DISABLE;
            TL_LOG_E("Failed to delete adv set %d %d",slow_interval,err);
            return;
        }
        TL_LOG_E("adv config %d \r\n", slow_interval);
        err =  blc_ll_setAdvData(adv_data, adv_len);
        if (err) {
            adv_state = SID_BLE_ADV_DISABLE;
            TL_LOG_E("Failed to set adv data to slow adv errno %d ", err);
            return;
        }
        err =  blc_ll_setScanRspData(sd, sizeof(sd));
        if (err) {
            adv_state = SID_BLE_ADV_DISABLE;
            TL_LOG_E("Failed to set rsp data to slow adv errno %d ", err);
            return;
        }
        err =  blc_ll_setAdvEnable(BLC_ADV_ENABLE);
        if (err) {
            adv_state = SID_BLE_ADV_DISABLE;
            TL_LOG_E("Failed to start slow adv errno %d ", err);
            return;
        }
        adv_state = BLE_ADV_SLOW;
        TL_LOG_D("Change succesful");
    }
}

int sid_ble_advert_init(void)
{
    TL_LOG_I("sid_ble_advert_init %d",adv_set);
    if (adv_set == 0) {
        #if (FREERTOS_ENABLE)
        work_delayable_init(&s_dwork, change_advertisement_interval);
        #endif
        adv_set  = 1;
    }

    return 0;
}

int sid_ble_advert_deinit(void)
{
    TL_LOG_I("sid_ble_advert_deinit %d",adv_set);
    if (adv_set == 1) {
        #if (FREERTOS_ENABLE)
        work_delayable_deinit(&s_dwork);
        #endif
        blc_ll_setAdvEnable(BLC_ADV_DISABLE);
        adv_state = SID_BLE_ADV_DISABLE;
        adv_set = 0;
    }
    return 0;
}

void sid_ble_advert_notify_connection(void)
{
    TL_LOG_D("Conneciton has been made, cancel change adv");
    #if (FREERTOS_ENABLE)
    work_delayable_cancel(&s_dwork);
    #endif
    adv_state = SID_BLE_ADV_DISABLE;
}


int sid_ble_advert_start_with_param(adv_inter_t intervalMin, adv_inter_t intervalMax, adv_type_t advType, own_addr_type_t ownAddrType, u8 peerAddrType, u8 *peerAddr, adv_chn_map_t adv_channelMap, adv_fp_type_t advFilterPolicy)
{

    int err = 0;

    err =  blc_ll_setAdvParam(intervalMin , \
            intervalMax,  \
            advType, ownAddrType, peerAddrType, peerAddr, adv_channelMap, advFilterPolicy);
    if (err) {
        adv_state = SID_BLE_ADV_DISABLE;
        TL_LOG_E("Failed to blc_ll_setAdvParam %d %d %d %d %d ",err,intervalMin,intervalMax, \
                ADV_TYPE_CONNECTABLE_UNDIRECTED,    OWN_ADDRESS_PUBLIC);
        return -1;
    }
    TL_LOG_E("adv param config %d %d", intervalMin,intervalMax);
    err =  blc_ll_setAdvData(adv_data, adv_len);
    if (err) {
        adv_state = SID_BLE_ADV_DISABLE;
        TL_LOG_E("Failed to set adv data to slow adv errno %d ", err);
        return -1;
    }
    tlkapi_send_string_data(1, "adv data ", adv_data, adv_len);
    err =  blc_ll_setScanRspData(sd, sizeof(sd));
    if (err) {
        adv_state = SID_BLE_ADV_DISABLE;
        TL_LOG_E("Failed to set rsp data to slow adv errno %d ", err);
        return -1;
    }
    err =  blc_ll_setAdvEnable(BLC_ADV_ENABLE);
    if (err) {
        adv_state = SID_BLE_ADV_DISABLE;
        TL_LOG_E("Failed to start slow adv errno %d ", err);
        return -1;
    }
    adv_state = BLE_ADV_FAST;
    #if (FREERTOS_ENABLE)
    if(fast_duration != 0)
    work_delayable_submit(&s_dwork,fast_duration*10);
    #endif
    return err;
}


int sid_ble_advert_start(void)
{
    TL_LOG_D("sid_ble_advert_start\r\n");
    int err = 0;
    // make sure to always start with fast advertising set
    sid_ble_advert_deinit();
    sid_ble_advert_init();

    err =  sid_ble_advert_start_with_param(fast_interval , \
            fast_interval,  \
            ADV_TYPE_CONNECTABLE_UNDIRECTED, OWN_ADDRESS_RANDOM, 0, NULL, BLT_ENABLE_ADV_ALL, ADV_FP_NONE);
    if (err) {
        adv_state = SID_BLE_ADV_DISABLE;
        TL_LOG_E("Failed to blc_ll_setAdvParam %d %d %d %d %d ",err,SID_ADV_MS_TO_INTERVAL(CFG_SIDEWALK_BLE_ADV_INT_FAST),SID_ADV_MS_TO_INTERVAL((CFG_SIDEWALK_BLE_ADV_INT_FAST+CFG_SIDEWALK_BLE_ADV_INT_PRECISION)), \
                ADV_TYPE_CONNECTABLE_UNDIRECTED,    OWN_ADDRESS_PUBLIC);
        return -1;
    }
    adv_state = BLE_ADV_FAST;

    return err;
}

int sid_ble_advert_stop(void)
{
    #if (FREERTOS_ENABLE)
    work_delayable_cancel(&s_dwork);
    #endif
    int err =  blc_ll_setAdvEnable(BLC_ADV_DISABLE);

    if (0 == err) {
        adv_state = SID_BLE_ADV_DISABLE;
    }
    TL_LOG_D("sid_ble_advert_stop %d ", err);
    return err;
}

int sid_ble_advert_update(uint8_t *data, uint8_t data_len)
{
    sid_ble_adv_state_t state = adv_state;

    if (!data || 0 == data_len) {
        return -EINVAL;
    }
    TL_LOG_D("sid_ble_advert_update %d ", data_len);
    tlkapi_send_string_data(1, "user data ", data, data_len);
    advert_manuf_data_copy(data, data_len);
    tlkapi_send_string_data(1, "adv data ", adv_data, adv_len);
    int err = 0;

    u8 mac_random_static[6] = {0x1,0x2,0x33,0x57,0x55,0x40};
    generateRandomNum(5, mac_random_static);
    mac_random_static[5] = 0x40;
    err = blc_ll_setRandomAddr(mac_random_static);
    if(err)
        TL_LOG_E("blc_ll_setRandomAddr 0x%x", err);
    if (SID_BLE_ADV_DISABLE != state) {
        /* Update currently advertised set, the other one will be set on start/transition */
        err =  blc_ll_setAdvData(adv_data, adv_len);
        TL_LOG_D("blc_ll_setAdvData %d %d", err,adv_len);
    }

    return err;
}

int sid_ble_get_adv_state(void)
{
    return adv_state;
}

int sid_ble_is_adv_init(void)
{
    return adv_set;
}

int sid_ble_set_adv_param(uint32_t ble_adv_fast_interval, uint32_t ble_adv_fast_timeout, \
        uint32_t  ble_adv_slow_interval, uint32_t  ble_adv_slow_timeout)
{

     TL_LOG_D("sid_ble_set_adv_param %d %d %d %d",ble_adv_fast_interval,ble_adv_slow_interval,ble_adv_fast_timeout,ble_adv_slow_timeout);
     fast_interval =  ble_adv_fast_interval ;
     slow_interval =  ble_adv_slow_interval;
     fast_duration =  ble_adv_fast_timeout;
     slow_duration =  ble_adv_slow_timeout;
     return 0;
}

int sid_ble_get_adv_param(uint32_t* ble_adv_fast_interval, uint32_t* ble_adv_fast_timeout, \
        uint32_t * ble_adv_slow_interval, uint32_t*  ble_adv_slow_timeout)
{
     *ble_adv_fast_interval = fast_interval ;
     *ble_adv_slow_interval = slow_interval;
     *ble_adv_fast_timeout =  fast_duration;
     *ble_adv_slow_timeout  = slow_duration;
     return 0;
}


