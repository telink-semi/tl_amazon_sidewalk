#include <sid_sdk_config.h>
#include "sidewalk/sidewalk_worker.h"

// FreeRTOS related
#include "tlk_riscv.h"
#include <FreeRTOS.h>
#include <task.h>
#include <timers.h>
#include <queue.h>
#include <semphr.h>
#include <event_groups.h>
#include "app_freertos.h"
// FreeRTOS related end

#include "sid_ble_adapter.h"
#include "app_config.h"
#include "sidewalk/message_encoder.h"
#include "app_ble_config.h"
#include "app_subGHz_config.h"

#include "sid_api.h"
#include <sid_pal_common_ifc.h>
#include "app_mfg_config.h"

// sidewalk worker instance
_attribute_ble_data_retention_ struct sid_handle *handle;
_attribute_ble_data_retention_ struct sid_config  config;
_attribute_ble_data_retention_ struct sid_status  last_status;

// Free RTOS stuff
#define QUEUE_LEN         (5)
#define THREAD_STACK_SIZE (4 * 1024)
_attribute_ble_data_retention_ static TaskHandle_t  xSidWorkerHandle = NULL;
_attribute_ble_data_retention_ static uint8_t       xQueueStorage[QUEUE_LEN * sizeof(sidewalkWorkerJobItem_t)];
_attribute_ble_data_retention_ static StaticQueue_t xQueueBuf;
_attribute_ble_data_retention_ static QueueHandle_t xQueue = NULL;


// time sync config
#define NUM_OF_INTERVALS (4)
static uint16_t                    default_sync_intervals_h[] = {2, 4, 8, 12};
static struct sid_time_sync_config default_time_sync_config;

// Sidewalk Job handlers

static void handle_job_init_platform(void)
{
    sid_error_t           retVal;
    platform_parameters_t platform_parameters = {
        .mfg_store_region.addr_start = sid_mfg_get_start_addr(),
        .mfg_store_region.addr_end   = sid_mfg_get_end_addr(),
#if CONFIG_SIDEWALK_SUBGHZ_SUPPORT
        .platform_init_parameters.radio_cfg = (radio_sx126x_device_config_t*)get_radio_cfg(),
#endif
    };
    TL_LOG_I("[SIDW] MFG start set to:0x%x", sid_mfg_get_start_addr());
    retVal = sid_platform_init(&platform_parameters);
    if (retVal != SID_ERROR_NONE) {
        TL_LOG_E("[SIDW] Failed to init platform. Err:%d", retVal);
    }
}

static void handle_job_bootstrap(void)
{
    sid_error_t retVal;
    if (handle != NULL) {
        TL_LOG_I("[SIDW] bootstrap failed. Handle taken.");
        return;
    }

    config.link_mask = SID_LINK_TYPE_1; // Start with BLE

    retVal = sid_init(&config, &handle);
    if (retVal) {
        TL_LOG_E("[SIDW] sid init failed with code %d ", retVal);
        return;
    }

    retVal = sid_start(handle, config.link_mask);
    if (retVal) {
        TL_LOG_E("[SIDW] sid start failed wit code %d", retVal);
    }

    if (config.link_mask & SID_LINK_TYPE_1) {
        enum sid_link_connection_policy set_policy = SID_LINK_CONNECTION_POLICY_AUTO_CONNECT;

        retVal = sid_option(handle, SID_OPTION_SET_LINK_CONNECTION_POLICY, &set_policy, sizeof(set_policy));
        if (retVal) {
            TL_LOG_E("[SIDW] sid_option(SID_OPTION_SET_LINK_CONNECTION_POLICY) failed %d", retVal);
        }

        struct sid_link_auto_connect_params ac_params = {.link_type = SID_LINK_TYPE_1, .enable = true, .priority = 0, .connection_attempt_timeout_seconds = 30};

        retVal = sid_option(handle, SID_OPTION_SET_LINK_POLICY_AUTO_CONNECT_PARAMS, &ac_params, sizeof(ac_params));
        if (retVal) {
            TL_LOG_E("sid option multi link policy err %d", retVal);
        }
    }
}

static void handle_job_sid_loop(void)
{
    sid_error_t retVal;

    if (handle == NULL) {
        TL_LOG_I("[SIDW] Sid not initialized.");
        return;
    }

    retVal = sid_process(handle);

    if (retVal) {
        TL_LOG_E("[SIDW] sid_process failed with %d ", retVal);
    }
}

static void handle_job_send_message(void *jc)
{
    app_message_to_encode_t *input_msg = (app_message_to_encode_t *)jc;
    int                      retVal;
    sid_error_t              sidRet;
    static uint8_t           sidMesgBuff[SID_MESSAGE_BUFF_SIZE];
    struct sid_msg           msgBody       = {.data = sidMesgBuff};
    struct sid_msg_desc      msgDesc       = {0};
    app_message_to_send_t    messageToSend = {.msgBody = &msgBody, .msgDesc = &msgDesc};

    retVal = encode_message(input_msg, &messageToSend);

    sidRet = sid_put_msg(handle, messageToSend.msgBody, messageToSend.msgDesc);
    if (sidRet) {
        TL_LOG_E("[SIDW] sid_put_msg() failed with %d", sidRet);
        return;
    }
}

static void handle_job_alter_link_type(void *jc)
{
    sid_error_t e;
    uint32_t *linkType_p = (uint32_t*) jc;
    struct sid_link_auto_connect_params ac_params = {
            .link_type = *linkType_p,
            .enable = true,
            .priority = 0,
            .connection_attempt_timeout_seconds = 30
        };

    if( (*linkType_p == SID_LINK_TYPE_1) || (*linkType_p == SID_LINK_TYPE_2) || (*linkType_p == SID_LINK_TYPE_3) )  {
        sid_process(handle);
        sid_deinit(handle);
        config.link_mask = *linkType_p;
        e = sid_init(&config, &handle);
        if (e) {
            TL_LOG_E("RAT_change sid_init() err %d", (int)e);
        }

        e = sid_start(handle, *linkType_p);
        if (e) {
            TL_LOG_E("RAT_change sid_start() err %d", (int)e);
        }

        enum sid_link_connection_policy set_policy = SID_LINK_CONNECTION_POLICY_AUTO_CONNECT;
        e = sid_option(handle, SID_OPTION_SET_LINK_CONNECTION_POLICY, &set_policy, sizeof(set_policy));
        if (e) {
            TL_LOG_E("RAT_change sid_option(1) err %d", (int)e);
        }

        ac_params.link_type = *linkType_p;
        e = sid_option(handle, SID_OPTION_SET_LINK_POLICY_AUTO_CONNECT_PARAMS, &ac_params, sizeof(ac_params));
        if (e) {
            TL_LOG_E("RAT_change sid_option(2) err %d", (int)e);
        }

        if(*linkType_p == SID_LINK_TYPE_2) {
            // increase responsivity of FSK by making rx interval desner
            struct sid_device_profile_unicast_params unicast_params = {
            .device_profile_id = SID_LINK2_PROFILE_2,
            .unicast_window_interval.sync_rx_interval_ms = SID_LINK2_RX_WINDOW_SEPARATION_1,
            .wakeup_type = SID_TX_AND_RX_WAKEUP };

            e = sid_option(handle, SID_OPTION_900MHZ_SET_DEVICE_PROFILE, &unicast_params, sizeof(unicast_params));
            if (e) {
                TL_LOG_E("RAT_change sid_option(3) err %d", (int)e);
            }
        }

    }
    else {
        TL_LOG_E("[Trying to set unknown link type:%d]", *linkType_p);
    }
}



static void sidewalk_worker_thread(void *p)
{
    sidewalkWorkerJobItem_t jobItem;
    TL_LOG_I("[SIDW] Sidewalk worker is up.");

    while (1) {
        if (xQueueReceive(xQueue, &jobItem, portMAX_DELAY) == pdTRUE) {
            switch (jobItem.jobType) {
            case SIDEWALK_WORKER_DO_PLATFORM_INIT:
                handle_job_init_platform();
                break;
            case SIDEWALK_WORKER_DO_BOOTSTRAP:
                handle_job_bootstrap();
                break;
            case SIDEWALK_WORKER_DO_SID_LOOP:
                handle_job_sid_loop();
                break;
            case SIDEWALK_WORKER_DO_SEND_MESSAGE:
                handle_job_send_message(jobItem.jobContext);
                break;
            case SIDEWALK_WORKER_DO_CHANGE_LINK:
                handle_job_alter_link_type(jobItem.jobContext);
                break;
            default:
                break;
            }
        }
    }
}

void sidewalk_woker_start(struct sid_event_callbacks *callbackList_p)
{
    // Initialize configuration
    default_time_sync_config.adaptive_sync_intervals_h = default_sync_intervals_h;
    default_time_sync_config.num_intervals             = NUM_OF_INTERVALS;


    config.link_mask               = 0;
    config.dev_ch.type             = SID_END_DEVICE_TYPE_STATIC;
    config.dev_ch.power_type       = SID_END_DEVICE_POWERED_BY_BATTERY_AND_LINE_POWER;
    config.dev_ch.qualification_id = 0x0001,

    config.callbacks           = callbackList_p;
    config.link_config         = app_get_ble_config();
    config.sub_ghz_link_config = app_get_sub_ghz_config();
    config.log_config          = NULL;
    config.time_sync_config    = &default_time_sync_config;

    xQueue = xQueueCreateStatic(QUEUE_LEN, sizeof(sidewalkWorkerJobItem_t), xQueueStorage, &xQueueBuf);

    xTaskCreate(sidewalk_worker_thread, "sid_worker", THREAD_STACK_SIZE, NULL, CONFIG_SIDEWALK_THREAD_PRIORITY, &xSidWorkerHandle);
}

void sidewalk_worker_take_job(sidewalkWorkerJobType_t jobType, void *jobContext)
{
    sidewalkWorkerJobItem_t jobItem;
    BaseType_t              xHigherPriorityTaskWoken = pdFALSE;
    BaseType_t              retVal;
    extern u32              xPortIsInsideInterrupt(void);

    jobItem.jobType    = jobType;
    jobItem.jobContext = jobContext;

    if (xPortIsInsideInterrupt()) {
        retVal = xQueueSendFromISR(xQueue, &jobItem, &xHigherPriorityTaskWoken);
        portYIELD_FROM_ISR(xHigherPriorityTaskWoken);
    } else {
        TickType_t timeout = (CONFIG_SIDEWALK_THREAD_QUEUE_TIMEOUT_VALUE > 0) ? pdMS_TO_TICKS(CONFIG_SIDEWALK_THREAD_QUEUE_TIMEOUT_VALUE) : 0;
        retVal             = xQueueSend(xQueue, &jobItem, timeout);
    }
}
