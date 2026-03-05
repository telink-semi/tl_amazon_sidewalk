/********************************************************************************************************
 * @file    sid_storage.c
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
/** @file sid_storage.c
 *  @brief Sidewalk nvm storage.
 */
#include "tl_common.h"
#include "drivers.h"
#include "stack/ble/ble.h"
#include "sid_ble_adapter.h"
#include <flashdb.h>
#include <sid_pal_storage_kv_ifc.h>
#include <stdint.h>
#include <stdio.h>

#define STORAGE_SERIAL_SIZE (32)
#define COMB_GROUP_KEY(group,key) (((uint32_t)(key<<16)) | group)

_attribute_ble_data_retention_ static uint8_t  init_flag = 0;
sid_error_t sid_pal_storage_kv_init()
{
    if(init_flag == 1)
        return 0;
    init_flag = 1;
    app_stoage_init(0);
    TL_LOG_D("Initialized KV storage");

    return SID_ERROR_NONE;
}

sid_error_t sid_pal_storage_kv_record_get(uint16_t group, uint16_t key, void *p_data, uint32_t len)
{
    if (!p_data) {
        return SID_ERROR_NULL_POINTER;
    }

    int read_len  = app_tag_stroage_get_data(COMB_GROUP_KEY(group,key),(uint8_t *)p_data,len,0);
//    TL_LOG_D("app_tag_stroage_get_data %d",read_len);
    if (read_len <= 0) {
        TL_LOG_E("app_tag_stroage_get_data fail 0x%x %d ",group,key);
        return SID_ERROR_NOT_FOUND;
    } else
        return SID_ERROR_NONE;
}

sid_error_t sid_pal_storage_kv_record_get_len(uint16_t group, uint16_t key, uint32_t *p_len)
{
    if (!p_len) {
        return SID_ERROR_NULL_POINTER;
    }

    int read_len  = app_tag_get_data_len(COMB_GROUP_KEY(group,key),0);

    if (read_len <= 0 )
    {
        TL_LOG_E("sid_pal_storage_kv_record_get_len fail 0x%x %d ",group,key);
        return SID_ERROR_NOT_FOUND;
    }
    else
    {
        *p_len = read_len;
        return SID_ERROR_NONE;
    }
}

sid_error_t sid_pal_storage_kv_record_set(uint16_t group, uint16_t key, void const *p_data,
                      uint32_t len)
{
    if (!p_data) {
        return SID_ERROR_NULL_POINTER;
    }
    if (len == 0) {
        return SID_ERROR_INVALID_ARGS;
    }

    int rc  = app_tag_stroage_set_data(COMB_GROUP_KEY(group,key),p_data,len,0);
    if (rc != 0) {
        TL_LOG_E("Failed to save record . Returned errno %d 0x%x %d", rc,group,key);
        return SID_ERROR_STORAGE_WRITE_FAIL;
    }

    return SID_ERROR_NONE;
}

sid_error_t sid_pal_storage_kv_record_delete(uint16_t group, uint16_t key)
{
    int rc  = app_tag_stroage_del_data(COMB_GROUP_KEY(group,key),0);
    if (rc == 0) {
        return SID_ERROR_NONE;
    }
    TL_LOG_E("Failed to delete record . Returned errno %d 0x%x %d", rc,group,key);
    return SID_ERROR_GENERIC;
}



sid_error_t sid_pal_storage_kv_group_delete(uint16_t group)
{
    int rc = app_tag_stroage_del_by_prefix(group,0);
    if (rc != 0) {
        TL_LOG_E("Failed to commit changes. Returned errno %d", rc);
        return SID_ERROR_GENERIC;
    }

    return SID_ERROR_NONE;
}
