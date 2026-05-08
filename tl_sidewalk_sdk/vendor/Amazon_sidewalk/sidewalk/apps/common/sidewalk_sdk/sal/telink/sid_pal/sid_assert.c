/********************************************************************************************************
 * @file    sid_assert.c
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
#include <sid_pal_assert_ifc.h>



#if defined(CONFIG_ASSERT_NO_FILE_INFO)
void sid_pal_assert(void)
{
     tlkapi_printf(APP_LOG_EN, "[ASSERT] \r\n",);
}

#else
void sid_pal_assert(int line, const char *file)
{
     tlkapi_printf(APP_LOG_EN, "[ASSERT] %s:%d\n",file,line);
}

#endif


