/********************************************************************************************************
 * @file    app_att.c
 *
 * @brief   This is the source file for BLE SDK
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
#include "tl_common.h"
#include "drivers.h"
#include "stack/ble/ble.h"

#include "app.h"
#include "app_att.h"
#include "app_ui.h"
#include "sid_ble_uuid.h"

////////////////////////////////////////// peripheral-role ATT service concerned ///////////////////////////////////////////////
typedef struct
{
    /** Minimum value for the connection event (interval. 0x0006 - 0x0C80 * 1.25 ms) */
    u16 intervalMin;
    /** Maximum value for the connection event (interval. 0x0006 - 0x0C80 * 1.25 ms) */
    u16 intervalMax;
    /** Number of LL latency connection events (0x0000 - 0x03e8) */
    u16 latency;
    /** Connection Timeout (0x000A - 0x0C80 * 10 ms) */
    u16 timeout;
} gap_periConnectParams_t;

static const u16 clientCharacterCfgUUID = GATT_UUID_CLIENT_CHAR_CFG;

static const u16 userdesc_UUID = GATT_UUID_CHAR_USER_DESC;

static const u16 serviceChangeUUID = GATT_UUID_SERVICE_CHANGE;

static const u16 my_primaryServiceUUID = GATT_UUID_PRIMARY_SERVICE;

static const u16 my_characterUUID = GATT_UUID_CHARACTER;

static const u16 my_devNameUUID = GATT_UUID_DEVICE_NAME;

static const u16 my_gapServiceUUID = SERVICE_UUID_GENERIC_ACCESS;

static const u16 my_appearanceUUID = GATT_UUID_APPEARANCE;

static const u16 my_periConnParamUUID = GATT_UUID_PERI_CONN_PARAM;

static const u16 my_appearance = GAP_APPEARANCE_UNKNOWN;

static const u16 my_gattServiceUUID = SERVICE_UUID_GENERIC_ATTRIBUTE;

static const gap_periConnectParams_t my_periConnParameters = {20, 40, 0, 1000};

_attribute_ble_data_retention_ static u16 serviceChangeVal[2] = {0};

_attribute_ble_data_retention_ static u8 serviceChangeCCC[2] = {0, 0};

#define UUID16_TO_ARRAY(x) {x&0xFF, x>>16 }
/* Authentication Service */
const u16 ama_service_t  = AMA_SERVICE_UUID_VAL;
const unsigned char uuid_ama_sid_t[16] = { AMA_CHARACTERISTIC_UUID_VAL_WRITE };
const unsigned char uuid_ama_sid_ntf_t[16] = { AMA_CHARACTERISTIC_UUID_VAL_NOTIFY };

const unsigned char ama_log_service_t[2] = UUID16_TO_ARRAY(LOG_EXAMPLE_SERVICE_UUID_VAL);
const unsigned char uuid_ama_log_t[16] = { LOG_EXAMPLE_CHARACTERISTIC_UUID_VAL_WRITE };
const unsigned char uuid_ama_log_ntf_t[16] = { LOG_EXAMPLE_CHARACTERISTIC_UUID_VAL_NOTIFY };

const unsigned char ama_vnd_service_t[2] = UUID16_TO_ARRAY(VND_EXAMPLE_SERVICE_UUID_VAL);
const unsigned char uuid_ama_vnd_t[16] = { VND_EXAMPLE_CHARACTERISTIC_UUID_VAL_WRITE };
const unsigned char uuid_ama_vnd_ntf_t[16] = { VND_EXAMPLE_CHARACTERISTIC_UUID_VAL_NOTIFY };


typedef struct __attribute__((packed))
{
    u8  type;
    u8  rf_len;
    u16 l2capLen;
    u16 chanId;
    u8  opcode;
    u16 handle;
    u8  value;
} ble_rf_packet_att_write_t;

int ama_sid_writeData(u16 connHandle, app_ble_rf_packet_att_write_t *p);

int ama_sid_writeccc(u16 connHandle, app_ble_rf_packet_att_write_t *p);

int ama_log_writeData(u16 connHandle, app_ble_rf_packet_att_write_t *p);

int ama_log_writeccc(u16 connHandle, app_ble_rf_packet_att_write_t *p);

int ama_vnd_writeData(u16 connHandle, app_ble_rf_packet_att_write_t *p);

int ama_vnd_writeccc(u16 connHandle, app_ble_rf_packet_att_write_t *p);


static const u8 my_devName[] = {'T', 'L'};


//////////////////////// OTA //////////////////////////////////
static const u8                          my_OtaServiceUUID[16] = WRAPPING_BRACES(TELINK_OTA_UUID_SERVICE);
static const u8                          my_OtaUUID[16]        = WRAPPING_BRACES(TELINK_SPP_DATA_OTA);
_attribute_ble_data_retention_ static u8 my_OtaData            = 0x00;
_attribute_ble_data_retention_ static u8 my_OtaDataCCC[2]      = {0, 0};
static const u8                          my_OtaName[]          = {'O', 'T', 'A'};


//// GAP attribute values
static const u8 my_devNameCharVal[5] = {
    CHAR_PROP_READ,
    U16_LO(GenericAccess_DeviceName_DP_H),
    U16_HI(GenericAccess_DeviceName_DP_H),
    U16_LO(GATT_UUID_DEVICE_NAME),
    U16_HI(GATT_UUID_DEVICE_NAME)};
static const u8 my_appearanceCharVal[5] = {
    CHAR_PROP_READ,
    U16_LO(GenericAccess_Appearance_DP_H),
    U16_HI(GenericAccess_Appearance_DP_H),
    U16_LO(GATT_UUID_APPEARANCE),
    U16_HI(GATT_UUID_APPEARANCE)};
static const u8 my_periConnParamCharVal[5] = {
    CHAR_PROP_READ,
    U16_LO(CONN_PARAM_DP_H),
    U16_HI(CONN_PARAM_DP_H),
    U16_LO(GATT_UUID_PERI_CONN_PARAM),
    U16_HI(GATT_UUID_PERI_CONN_PARAM)};


//// GATT attribute values
static const u8 my_serviceChangeCharVal[5] = {
    CHAR_PROP_INDICATE,
    U16_LO(GenericAttribute_ServiceChanged_DP_H),
    U16_HI(GenericAttribute_ServiceChanged_DP_H),
    U16_LO(GATT_UUID_SERVICE_CHANGE),
    U16_HI(GATT_UUID_SERVICE_CHANGE)};

_attribute_ble_data_retention_ u8 AmasidInitV[16] = {0};
_attribute_ble_data_retention_ u8 AmasidntfInitV[1] = {0};

//_attribute_ble_data_retention_ u8 AmalogInitV[16] = {0};
//_attribute_ble_data_retention_ u8 AmalogntfInitV[1] = {0};
//
//_attribute_ble_data_retention_ u8 AmavndInitV[16] = {0};
//_attribute_ble_data_retention_ u8 AmavndntfInitV[1] = {0};

_attribute_ble_data_retention_ static u8 AmasidCCC[2] = {0, 0};
//_attribute_ble_data_retention_ static u8 AmalogCCC[2] = {0, 0};
//_attribute_ble_data_retention_ static u8 AmavndCCC[2] = {0, 0};

////////////////////////// AMA_SERVICE //////////////////////////////////
//static const u8 AmaSIDCharVal[1] = { CHAR_PROP_WRITE_WITHOUT_RSP};
//static const u8 AmaSIDntfCharVal[1] = { CHAR_PROP_NOTIFY };
//
//static const u8 AmaLogCharVal[1] = { CHAR_PROP_WRITE_WITHOUT_RSP};
//static const u8 AmaLOGntfCharVal[1] = { CHAR_PROP_NOTIFY };
//
//static const u8 AmaVndCharVal[1] = { CHAR_PROP_WRITE_WITHOUT_RSP};
//static const u8 AmaVndntfCharVal[1] = { CHAR_PROP_NOTIFY };

static const u8 AmaSIDCharVal[19] = {
        CHAR_PROP_WRITE_WITHOUT_RSP,
        U16_LO( AMA_SID_DP_H),
        U16_HI( AMA_SID_DP_H),
        AMA_CHARACTERISTIC_UUID_VAL_WRITE };

static const u8 AmaSIDntfCharVal[19] = {
        CHAR_PROP_NOTIFY,
        U16_LO(AMA_SIDNTF_DP_H),
        U16_HI(AMA_SIDNTF_DP_H),
        AMA_CHARACTERISTIC_UUID_VAL_NOTIFY };


//// OTA attribute values
static const u8 my_OtaCharVal[19] = {
    CHAR_PROP_READ | CHAR_PROP_WRITE_WITHOUT_RSP | CHAR_PROP_NOTIFY,
    U16_LO(OTA_CMD_OUT_DP_H),
    U16_HI(OTA_CMD_OUT_DP_H),
    TELINK_SPP_DATA_OTA,
};


#define AMA_SID_SERVICE          ((5) + (1))
#define AMA_LOG_SERVICE        0   //((5) + (1))
#define AMA_VND_SERVICE        0   //((5) + (1))
#define AMA_SERVICE_TOTAL      0   //(AMA_SID_SERVICE+AMA_LOG_SERVICE+AMA_VND_SERVICE)

// TM : to modify
static const attribute_t my_Attributes[] = {

        {ATT_END_H - 1 + AMA_SERVICE_TOTAL,   0,                     0,  0,                                         NULL,                                            NULL,                                              NULL,                                         NULL}, // total num of attribute


        // 0001 - 0007  gap
        {7,                                     ATT_PERMISSIONS_READ,  2,  2,                                         (u8 *)(size_t)(&my_primaryServiceUUID),          (u8 *)(size_t)(&my_gapServiceUUID),                0,                                            0   },
        {0,                                     ATT_PERMISSIONS_READ,  2,  sizeof(my_devNameCharVal),                 (u8 *)(size_t)(&my_characterUUID),               (u8 *)(size_t)(my_devNameCharVal),                 0,                                            0   },
        {0,                                     ATT_PERMISSIONS_READ,  2,  sizeof(my_devName),                        (u8 *)(size_t)(&my_devNameUUID),                 (u8 *)(size_t)(my_devName),                        0,                                            0   },
        {0,                                     ATT_PERMISSIONS_READ,  2,  sizeof(my_appearanceCharVal),              (u8 *)(size_t)(&my_characterUUID),               (u8 *)(size_t)(my_appearanceCharVal),              0,                                            0   },
        {0,                                     ATT_PERMISSIONS_READ,  2,  sizeof(my_appearance),                     (u8 *)(size_t)(&my_appearanceUUID),              (u8 *)(size_t)(&my_appearance),                    0,                                            0   },
        {0,                                     ATT_PERMISSIONS_READ,  2,  sizeof(my_periConnParamCharVal),           (u8 *)(size_t)(&my_characterUUID),               (u8 *)(size_t)(my_periConnParamCharVal),           0,                                            0   },
        {0,                                     ATT_PERMISSIONS_READ,  2,  sizeof(my_periConnParameters),             (u8 *)(size_t)(&my_periConnParamUUID),           (u8 *)(size_t)(&my_periConnParameters),            0,                                            0   },


        // 0008 - 000b gatt
        {4,                                     ATT_PERMISSIONS_READ,  2,  2,                                         (u8 *)(size_t)(&my_primaryServiceUUID),          (u8 *)(size_t)(&my_gattServiceUUID),               0,                                            0   },
        {0,                                     ATT_PERMISSIONS_READ,  2,  sizeof(my_serviceChangeCharVal),           (u8 *)(size_t)(&my_characterUUID),               (u8 *)(size_t)(my_serviceChangeCharVal),           0,                                            0   },
        {0,                                     ATT_PERMISSIONS_READ,  2,  sizeof(serviceChangeVal),                  (u8 *)(size_t)(&serviceChangeUUID),              (u8 *)(&serviceChangeVal),                         0,                                            0   },
        {0,                                     ATT_PERMISSIONS_RDWR,  2,  sizeof(serviceChangeCCC),                  (u8 *)(size_t)(&clientCharacterCfgUUID),         (u8 *)(serviceChangeCCC),                          0,                                            0   },


        ////////////////////////////////////// AMA_SERVICE /////////////////////////////////////////////////////
        {AMA_SID_SERVICE, ATT_PERMISSIONS_READ,   2,  2,  (u8 *)(size_t)(&my_primaryServiceUUID),    (u8 *)(size_t)(&ama_service_t),   0,  0},
        // Offset: 001 - 004,
//        {0,     ATT_PERMISSIONS_READ,   2,  sizeof(AmaSIDCharVal),           (u8 *)(size_t)(&my_characterUUID),          (u8 *)(size_t)(AmaSIDCharVal),               0,                      0   },                  // prop
//        {0,     ATT_PERMISSIONS_WRITE,  16, sizeof(AmasidInitV),             (u8 *)(size_t)(&uuid_ama_sid_t),              (u8 *)(size_t)(&AmasidInitV),                (att_readwrite_callback_t)&ama_sid_writeData,     NULL},                  // value
//
//        {0,     ATT_PERMISSIONS_READ,   2,  sizeof(AmaSIDntfCharVal),           (u8 *)(size_t)(&my_characterUUID),          (u8 *)(size_t)(AmaSIDntfCharVal),          0,                      0   },                  // prop
//        {0,     ATT_PERMISSIONS_WRITE,  16, sizeof(AmasidntfInitV),             (u8 *)(size_t)(&uuid_ama_sid_ntf_t),         (u8 *)(size_t)(&AmasidntfInitV),          NULL,     NULL},                  // value
//        {0,     ATT_PERMISSIONS_RDWR,   2,  sizeof(AmasidCCC),               (u8 *)(size_t)(&clientCharacterCfgUUID),        (u8 *)(AmasidCCC),                        (att_readwrite_callback_t)&ama_sid_writeccc,                       0   },                  // value

        {0,     ATT_PERMISSIONS_READ,   2,  sizeof(AmaSIDCharVal),           (u8 *)(size_t)(&my_characterUUID),          (u8 *)(size_t)(AmaSIDCharVal),               0,                      0   },                  // prop
        {0,     ATT_PERMISSIONS_WRITE,  16, sizeof(AmasidInitV),             (u8 *)(size_t)(&uuid_ama_sid_t),              (u8 *)(size_t)(&AmasidInitV),                (att_readwrite_callback_t)&ama_sid_writeData,     NULL},                  // value

        {0,     ATT_PERMISSIONS_READ,   2,  sizeof(AmaSIDntfCharVal),         (u8 *)(size_t)(&my_characterUUID),          (u8 *)(size_t)(AmaSIDntfCharVal),          0,                      0   },                  // prop
        {0,     ATT_PERMISSIONS_RDWR,   16, sizeof(AmasidntfInitV),           (u8 *)(size_t)(&uuid_ama_sid_ntf_t),         (u8 *)(size_t)(&AmasidntfInitV),          NULL,     NULL},                  // value
        {0,     ATT_PERMISSIONS_RDWR,   2,  sizeof(AmasidCCC),                (u8 *)(size_t)(&clientCharacterCfgUUID),        (u8 *)(AmasidCCC),                        (att_readwrite_callback_t)&ama_sid_writeccc,                       0   },                  // value


//        ////////////////////////////////////// AMA_SERVICE /////////////////////////////////////////////////////
//        {AMA_LOG_SERVICE, ATT_PERMISSIONS_READ,   2,  2,  (u8 *)(size_t)(&my_primaryServiceUUID),    (u8 *)(size_t)(&ama_log_service_t),   0,  0},
//        // Offset: 001 - 004,
//        {0,     ATT_PERMISSIONS_READ,   2,  sizeof(AmaLogCharVal),           (u8 *)(size_t)(&my_characterUUID),          (u8 *)(size_t)(AmaLogCharVal),               0,                      0   },                  // prop
//        {0,     ATT_PERMISSIONS_WRITE,  16, sizeof(AmalogInitV),             (u8 *)(size_t)(&uuid_ama_log_t),              (u8 *)(size_t)(&AmalogInitV),                (att_readwrite_callback_t)&ama_log_writeData,     NULL},                  // value
//
//        {0,     ATT_PERMISSIONS_READ,   2,  sizeof(AmaLOGntfCharVal),           (u8 *)(size_t)(&my_characterUUID),          (u8 *)(size_t)(AmaLOGntfCharVal),          0,                      0   },                  // prop
//        {0,     ATT_PERMISSIONS_WRITE,  16, sizeof(AmalogntfInitV),             (u8 *)(size_t)(&uuid_ama_log_ntf_t),         (u8 *)(size_t)(&AmalogntfInitV),          NULL,     NULL},                  // value
//        {0,     ATT_PERMISSIONS_RDWR,   2,  sizeof(AmalogCCC),               (u8 *)(size_t)(&clientCharacterCfgUUID),        (u8 *)(AmalogCCC),                        (att_readwrite_callback_t)&ama_log_writeccc,                       0   },                  // value
//
//        ////////////////////////////////////// AMA_SERVICE /////////////////////////////////////////////////////
//        {AMA_VND_SERVICE, ATT_PERMISSIONS_READ,   2,  2,  (u8 *)(size_t)(&my_primaryServiceUUID),    (u8 *)(size_t)(&ama_vnd_service_t),   0,  0},
//        // Offset: 001 - 004,
//        {0,     ATT_PERMISSIONS_READ,   2,  sizeof(AmaVndCharVal),           (u8 *)(size_t)(&my_characterUUID),          (u8 *)(size_t)(AmaVndCharVal),               0,                      0   },                  // prop
//        {0,     ATT_PERMISSIONS_WRITE,  16, sizeof(AmavndInitV),             (u8 *)(size_t)(&ama_vnd_service_t),              (u8 *)(size_t)(&AmavndInitV),                (att_readwrite_callback_t)&ama_vnd_writeData,     NULL},                  // value
//
//        {0,     ATT_PERMISSIONS_READ,   2,  sizeof(AmaVndntfCharVal),           (u8 *)(size_t)(&my_characterUUID),          (u8 *)(size_t)(AmaVndntfCharVal),          0,                      0   },                  // prop
//        {0,     ATT_PERMISSIONS_WRITE,  16, sizeof(AmavndntfInitV),             (u8 *)(size_t)(&uuid_ama_vnd_ntf_t),         (u8 *)(size_t)(&AmavndntfInitV),          NULL,     NULL},                  // value
//        {0,     ATT_PERMISSIONS_RDWR,   2,  sizeof(AmavndCCC),               (u8 *)(size_t)(&clientCharacterCfgUUID),        (u8 *)(AmavndCCC),                        (att_readwrite_callback_t)&ama_vnd_writeccc,                       0   },                  // value
        ////////////////////////////////////// OTA /////////////////////////////////////////////////////
        // 0036 - 0039
        {5,                                     ATT_PERMISSIONS_READ,  2,  16,                                        (u8 *)(size_t)(&my_primaryServiceUUID),          (u8 *)(size_t)(&my_OtaServiceUUID),                0,                                            0   },
        {0,                                     ATT_PERMISSIONS_READ,  2,  sizeof(my_OtaCharVal),                     (u8 *)(size_t)(&my_characterUUID),               (u8 *)(size_t)(my_OtaCharVal),                     0,                                            0   }, //prop
        {0,                                     ATT_PERMISSIONS_RDWR,  16, sizeof(my_OtaData),                        (u8 *)(size_t)(&my_OtaUUID),                     (&my_OtaData),                                     &otaWrite,                                    NULL}, //value
        {0,                                     ATT_PERMISSIONS_RDWR,  2,  sizeof(my_OtaDataCCC),                     (u8 *)(size_t)(&clientCharacterCfgUUID),         (u8 *)(my_OtaDataCCC),                             0,                                            0   }, //value
        {0,                                     ATT_PERMISSIONS_READ,  2,  sizeof(my_OtaName),                        (u8 *)(size_t)(&userdesc_UUID),                  (u8 *)(size_t)(my_OtaName),                        0,                                            0   },
  };


extern void ama_srv_notif_changed(uint16_t conn_handle, uint16_t handle,  uint16_t  value);
extern int ama_srv_on_write(uint16_t conn_handle, uint16_t handle, const void *buf, uint16_t len, uint16_t offset);
extern void log_srv_notif_changed(uint16_t conn_handle, uint16_t handle, uint16_t value);
extern int log_srv_on_write(uint16_t conn_handle, uint16_t handle, const void *buf, uint16_t len, uint16_t offset);
extern void vnd_srv_notif_changed(uint16_t conn_handle, uint16_t handle, uint16_t value);
extern int vnd_srv_on_write(uint16_t conn_handle, uint16_t handle, const void *buf, uint16_t len, uint16_t offset);

int ama_sid_writeData(u16 connHandle, app_ble_rf_packet_att_write_t *p)
{
    tlkapi_printf(APP_LOG_EN, "[APP] ama_sid_writeData ");
    u16  att_handle = p->handle;
    u8 * buf = &(p->value);
    u16 len = p->l2capLen - 3;
    ama_srv_on_write(connHandle,att_handle,buf,len,0);
    return 0;
}


int ama_sid_writeccc(u16 connHandle, app_ble_rf_packet_att_write_t *p)
{
    u16  att_handle = p->handle;
    u16 data = *((u16 *)&p->value);
    tlkapi_printf(APP_LOG_EN, "[APP] ama_sid_writeccc %x", data);
    ama_srv_notif_changed(connHandle,att_handle,data);
    return 0;
}


int ama_log_writeData(u16 connHandle, app_ble_rf_packet_att_write_t *p)
{
    u16  att_handle = p->handle;
    u8 * buf = &(p->value);
    u16 len = p->l2capLen - 3;
    log_srv_on_write(connHandle,att_handle,buf,len,0);
    return 0;
}

int ama_log_writeccc(u16 connHandle, app_ble_rf_packet_att_write_t *p)
{
    u16  att_handle = p->handle;
    u16 data = *((u16 *)&p->value);
    log_srv_notif_changed(connHandle,att_handle,data);
    return 0;
}


int ama_vnd_writeData(u16 connHandle, app_ble_rf_packet_att_write_t *p)
{
    u16  att_handle = p->handle;
    u8 * buf = &(p->value);
    u16 len = p->l2capLen - 3;
    vnd_srv_on_write(connHandle,att_handle,buf,len,0);
    return 0;
}

int ama_vnd_writeccc(u16 connHandle, app_ble_rf_packet_att_write_t *p)
{
    u16  att_handle = p->handle;
    u16 data = *((u16 *)&p->value);
    vnd_srv_notif_changed(connHandle,att_handle,data);
    return 0;
}

/**
 * @brief   GATT initialization.
 *          !!!Note: this function is used to register ATT table to BLE Stack.
 * @param   none.
 * @return  none.
 */
void my_gatt_init(void)
{
    bls_att_setAttributeTable((u8 *)(size_t)my_Attributes);
}
