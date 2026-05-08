/********************************************************************************************************
 * @file    sid_ble_adapter.h
 *
 * @brief   This is the header file for BLE SDK
 *
 * @author  BLE GROUP
 * @date    06,2022
 *
 * @par     Copyright (c) 2022, Telink Semiconductor (Shanghai) Co., Ltd. ("TELINK")
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
/** @file sid_ble_service.h
 *  @brief Bluetooth low energy service API.
 */

#ifndef SID_PAL_BLE_ADPTER_H
#define SID_PAL_BLE_ADPTER_H


#define  BT_UUID_TYPE_16  0
#define  BT_UUID_TYPE_32  1
#define  BT_UUID_TYPE_128 2

#define UUID16_LO(u16_v) ((u16_v) & 0xFF)
#define UUID16_HI(u16_v) (((u16_v) >> 8) & 0xFF)

#define BT_GATT_CCC_NOTIFY 1
#define ARG_UNUSED(x) ((void)x)


#ifndef MIN
    #define MIN(a, b) ((a) < (b) ? (a) : (b))
#endif

struct bt_uuid {
    uint8_t  type;
};

struct bt_uuid_16 {
    struct bt_uuid uuid;
    uint16_t val;
};


struct bt_uuid_32 {
    struct bt_uuid uuid;
    uint32_t val;
};

struct bt_uuid_128 {
    struct bt_uuid uuid;
    uint8_t  val[16];
};


#define BT_UUID_DECLARE_128(val...) \
    ((struct bt_uuid_128 *) &(const struct bt_uuid_128) { \
        .uuid = { .type = BT_UUID_TYPE_128 }, \
        .val  = { val }, \
    })

#define BT_UUID_DECLARE_16(val...) \
    ((struct bt_uuid_16 *) &(const struct bt_uuid_16) { \
        .uuid = { .type = BT_UUID_TYPE_16 }, \
        .val  = { val }, \
    })



void * sid_malloc(size_t size);

void sid_free(void * ptr);


#define SID_ADV_MS_TO_INTERVAL(v) (uint16_t)((v * 8)/5)
#define AMA_ADV_OPTIONS (BT_LE_ADV_OPT_CONN)

#if defined(CONFIG_LOG_LEVEL_ERROR)&&(CONFIG_LOG_LEVEL_ERROR == 1)
#if TLKAPI_RTT_PRINT
#define TL_LOG_E(fmt, args...) \
    SEGGER_RTT_printf(0, fmt "\r\n", ##args)
#else
#define TL_LOG_E(fmt, args...) \
    tlk_printf(fmt "\r\n", ##args)
#endif
#else
#define TL_LOG_E(fmt, args...)
#endif

#if defined(CONFIG_LOG_LEVEL_INFO)&&(CONFIG_LOG_LEVEL_INFO == 1)
#if TLKAPI_RTT_PRINT
#define TL_LOG_I(fmt, args...) \
    SEGGER_RTT_printf(0, fmt "\r\n", ##args)
#else
#define TL_LOG_I(fmt, args...)                             \
    do                                                      \
    {                                                 \
            tlk_printf(fmt "\r\n", ##args);     \
    } while (0)
#endif
#else
#define TL_LOG_I(fmt, args...)
#endif

#if defined(CONFIG_LOG_LEVEL_DEBUG)&&(CONFIG_LOG_LEVEL_DEBUG == 1)
#if TLKAPI_RTT_PRINT
#define TL_LOG_D(fmt, args...) \
    SEGGER_RTT_printf(0, fmt "\r\n", ##args)
#else
#define TL_LOG_D(fmt, args...)                             \
    do                                                      \
    {                                                       \
            tlk_printf(fmt "\r\n", ##args);      \
    } while (0)
#endif
#else
#define TL_LOG_D(fmt, args...)
#endif


#if defined(CONFIG_LOG_LEVEL_WRN)&&(CONFIG_LOG_LEVEL_WRN == 1)
#if TLKAPI_RTT_PRINT
#define TL_LOG_W(fmt, args...) \
    SEGGER_RTT_printf(0, fmt "\r\n", ##args)
#else
#define TL_LOG_W(fmt, args...)                             \
    do                                                      \
    {                                                       \
            tlk_printf(fmt, ##args);      \
    } while (0)
#endif
#else
#define TL_LOG_W(fmt, args...)
#endif

#endif /* SID_PAL_BLE_ADPTER_H */
