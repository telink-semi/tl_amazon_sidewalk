/********************************************************************************************************
 * @file    sid_log.c
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
/** @file sid_log.c
 *  @brief Log interface implementation.
 */
#include "tl_common.h"
#include "drivers.h"
#include "stack/ble/ble.h"
#include "sid_ble_adapter.h"
#include <sid_pal_log_ifc.h>
#include <stddef.h>
#include <stdio.h>

#if (FREERTOS_ENABLE)
    #include "tlk_riscv.h"
    #include <FreeRTOS.h>
    #include <task.h>
    #include <timers.h>
    #include <queue.h>
    #include <event_groups.h>
    #include "app_freertos.h"
#endif

#define LOG_FLUSH_SLEEP_PERIOD (5 * portTICK_PERIOD_MS)

#define MSG_LENGTH_MAX (CONFIG_SIDEWALK_LOG_MSG_LENGTH_MAX)

void sid_pal_log(sid_pal_log_severity_t severity, uint32_t num_args, const char *fmt, ...)
{
    ARG_UNUSED(num_args);

#if !defined(CONFIG_LOG)
    ARG_UNUSED(severity);
    ARG_UNUSED(fmt);
    return;
#endif /* !defined(CONFIG_LOG) */

    va_list args;
    va_start(args, fmt);

    char buf[MSG_LENGTH_MAX];
    vsnprintf(buf, sizeof(buf), fmt, args);

    switch (severity) {
    case SID_PAL_LOG_SEVERITY_ERROR:
        //TL_LOG_E("%s", buf);
        tlk_printf("%s\r\n", buf);
        break;
    case SID_PAL_LOG_SEVERITY_WARNING:
        tlk_printf("%s\r\n", buf);
        break;
    case SID_PAL_LOG_SEVERITY_INFO:
        tlk_printf("%s\r\n", buf);
        break;
    case SID_PAL_LOG_SEVERITY_DEBUG:
        tlk_printf("%s\r\n", buf);
        break;
    default:
        tlk_printf("%s\r\n", buf);
        TL_LOG_W("sid pal log unknown severity %d", severity);
        break;
    }

    va_end(args);
}

void sid_pal_hexdump(sid_pal_log_severity_t severity, const void *address, int length)
{
    switch (severity) {
    case SID_PAL_LOG_SEVERITY_ERROR:
        tlkapi_send_string_data(1, " ", address, length);
        break;
    case SID_PAL_LOG_SEVERITY_WARNING:
        tlkapi_send_string_data(1, " ", address, length);
        break;
    case SID_PAL_LOG_SEVERITY_INFO:
        tlkapi_send_string_data(1, " ", address, length);
        break;
    case SID_PAL_LOG_SEVERITY_DEBUG:
        tlkapi_send_string_data(1, " ", address, length);
        break;
    default:
        TL_LOG_W("sid pal log unknow severity %d", severity);
        tlkapi_send_string_data(1, "address ", address, length);
        break;
    }
}

void sid_pal_log_flush(void)
{
#if defined(CONFIG_LOG) && !defined(CONFIG_LOG_MODE_MINIMAL)
    /* Note: log_buffered_cnt is not supported in minimal log mode. */
//    while (log_buffered_cnt()) {
//        vTaskDelay(LOG_FLUSH_SLEEP_PERIOD);
//    }
    fflush(NULL);
#endif
}

char const *sid_pal_log_push_str(char *string)
{
    return string;
}

bool sid_pal_log_get_log_buffer(struct sid_pal_log_buffer *const log_buffer)
{
    ARG_UNUSED(log_buffer);
    TL_LOG_W("%s - not implemented (optional).", __func__);

    return false;
}

//sid_pal_log_severity_t sid_log_control_get_current_log_level(void)
//{
//    return (sid_pal_log_severity_t)SID_PAL_LOG_LEVEL;
//}
