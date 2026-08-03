#include "sidewalk/message_encoder.h"
#include "sidewalk/sid_demo_types.h"
#include "sensor_monitoring/app_err.h"
#include "app_config.h"
#include "sid_ble_adapter.h"

int encode_message(app_message_to_encode_t *input_msg, app_message_to_send_t *output_msg) {
    struct sid_parse_state SIDparseState;
    struct sid_demo_msg_desc appDescriptor;
    struct sid_demo_msg appMsg;

    uint8_t payloadBuff[APP_PAYLOAD_BUFF_SIZE];

    switch(input_msg->messageType) {
        case APP_MESSAGE_CAPABILITIES:
            TL_LOG_I("[MSG] Encoding APP_MESSAGE_CAPABILITIES");
            TL_LOG_I("link_type:%d", input_msg->data.capabilities.deviceCapabilities.link_type);
            TL_LOG_I("temp_sensor:%d", input_msg->data.capabilities.deviceCapabilities.temp_sensor);
            TL_LOG_I("button_id_arr:%d", input_msg->data.capabilities.deviceCapabilities.button_id_arr);
            TL_LOG_I("num_buttons:%d", input_msg->data.capabilities.deviceCapabilities.num_buttons);
            TL_LOG_I("led_id_arr:%d", input_msg->data.capabilities.deviceCapabilities.led_id_arr);
            TL_LOG_I("num_leds:%d", input_msg->data.capabilities.deviceCapabilities.num_leds);

            sid_parse_state_init(&SIDparseState, payloadBuff, APP_PAYLOAD_BUFF_SIZE);
            sid_demo_app_capability_discovery_notification_serialize(&SIDparseState, &input_msg->data.capabilities.deviceCapabilities);
            if (SIDparseState.ret_code != SID_ERROR_NONE) {
                TL_LOG_E("[MSG] APP_MESSAGE_CAPABILITIES encoding failed!");
                return EINVAL;
            }
            appDescriptor.status_hdr_ind = false,
            appDescriptor.opc            = SID_DEMO_MSG_TYPE_NOTIFY,
            appDescriptor.cmd_class      = SID_DEMO_APP_CLASS,
            appDescriptor.cmd_id         = SID_DEMO_APP_CLASS_CMD_CAP_DISCOVERY_ID,

            output_msg->msgDesc->link_type = input_msg->linkType;
            output_msg->msgDesc->type      = SID_MSG_TYPE_NOTIFY;
            output_msg->msgDesc->link_mode = SID_LINK_MODE_CLOUD;

            appMsg.payload = payloadBuff;
            appMsg.payload_size = SIDparseState.offset;
            break;

        case APP_MESSAGE_BUTTONS_REPORT:
            TL_LOG_I("[MSG] Encoding APP_MESSAGE_BUTTONS_REPORT");
            sid_parse_state_init(&SIDparseState, payloadBuff, APP_PAYLOAD_BUFF_SIZE);
            sid_demo_app_action_notification_serialize(&SIDparseState, &input_msg->data.buttonsReport.buttonsNotification);
            if (SIDparseState.ret_code != SID_ERROR_NONE) {
                TL_LOG_E("[MSG] APP_MESSAGE_BUTTONS_REPORT encoding failed!");
                return EINVAL;
            }
            appDescriptor.status_hdr_ind = false,
            appDescriptor.opc            = SID_DEMO_MSG_TYPE_NOTIFY,
            appDescriptor.cmd_class      = SID_DEMO_APP_CLASS,
            appDescriptor.cmd_id         = SID_DEMO_APP_CLASS_CMD_ACTION,

            output_msg->msgDesc->link_type                            = input_msg->linkType;
            output_msg->msgDesc->type                                 = SID_MSG_TYPE_NOTIFY;
            output_msg->msgDesc->link_mode                            = SID_LINK_MODE_CLOUD;
            output_msg->msgDesc->msg_desc_attr.tx_attr.ttl_in_seconds = TTL_SECONDS;
            output_msg->msgDesc->msg_desc_attr.tx_attr.num_retries    = NUM_RETRIES;
            output_msg->msgDesc->msg_desc_attr.tx_attr.request_ack    = true;

            appMsg.payload = payloadBuff;
            appMsg.payload_size = SIDparseState.offset;
            break;

        case APP_MESSAGE_SENSORS_REPORT:
            TL_LOG_I("[MSG] Encoding APP_MESSAGE_SENSORS_REPORT");
            sid_parse_state_init(&SIDparseState, payloadBuff, APP_PAYLOAD_BUFF_SIZE);
            sid_demo_app_action_notification_serialize(&SIDparseState, &input_msg->data.sensorsReport.sensorsNotification);
            if (SIDparseState.ret_code != SID_ERROR_NONE) {
                TL_LOG_D("[MSG] APP_MESSAGE_SENSORS_REPORT encoding failed!");
                return EINVAL;
            }

            appDescriptor.status_hdr_ind = false,
            appDescriptor.opc            = SID_DEMO_MSG_TYPE_NOTIFY;
            appDescriptor.cmd_class      = SID_DEMO_APP_CLASS;
            appDescriptor.cmd_id         = SID_DEMO_APP_CLASS_CMD_ACTION;

            output_msg->msgDesc->link_type = input_msg->linkType;
            output_msg->msgDesc->type      = SID_MSG_TYPE_NOTIFY;
            output_msg->msgDesc->link_mode = SID_LINK_MODE_CLOUD;
            output_msg->msgDesc->msg_desc_attr.tx_attr.ttl_in_seconds = TTL_SECONDS;
            output_msg->msgDesc->msg_desc_attr.tx_attr.num_retries    = NUM_RETRIES;
            output_msg->msgDesc->msg_desc_attr.tx_attr.request_ack    = true;

            appMsg.payload = payloadBuff;
            appMsg.payload_size = SIDparseState.offset;
            break;

        case APP_MESSAGE_LEDS_RESPONSE:
            TL_LOG_I("[MSG] Encoding APP_MESSAGE_LEDS_RESPONSE");
            sid_parse_state_init(&SIDparseState, payloadBuff, APP_PAYLOAD_BUFF_SIZE);
            sid_demo_app_action_resp_serialize(&SIDparseState, &input_msg->data.ledsResponse.LEDresponse);

            appDescriptor.status_hdr_ind = true;
            appDescriptor.status_code    = SID_ERROR_NONE;
            appDescriptor.opc            = SID_DEMO_MSG_TYPE_RESP;
            appDescriptor.cmd_class      = SID_DEMO_APP_CLASS;
            appDescriptor.cmd_id         = SID_DEMO_APP_CLASS_CMD_ACTION;

            output_msg->msgDesc->link_type = input_msg->linkType;
            output_msg->msgDesc->type      = SID_MSG_TYPE_NOTIFY;
            output_msg->msgDesc->link_mode = SID_LINK_MODE_CLOUD;
            output_msg->msgDesc->msg_desc_attr.tx_attr.ttl_in_seconds = TTL_SECONDS;
            output_msg->msgDesc->msg_desc_attr.tx_attr.num_retries    = NUM_RETRIES;
            output_msg->msgDesc->msg_desc_attr.tx_attr.request_ack    = true;

            appMsg.payload = payloadBuff;
            appMsg.payload_size = SIDparseState.offset;
            break;

        case APP_MESSAGE_LINK_TYPE_RESPONSE:
            TL_LOG_I("[MSG] Encoding APP_MESSAGE_LINK_TYPE_RESPONSE");
            sid_parse_state_init(&SIDparseState, payloadBuff, APP_PAYLOAD_BUFF_SIZE);
            sid_demo_app_action_resp_serialize(&SIDparseState, &input_msg->data.linkTypeResponse.linkTypeResponse);

            appDescriptor.status_hdr_ind = true;
            appDescriptor.status_code    = SID_ERROR_NONE;
            appDescriptor.opc            = SID_DEMO_MSG_TYPE_RESP;
            appDescriptor.cmd_class      = SID_DEMO_APP_CLASS;
            appDescriptor.cmd_id         = SID_DEMO_APP_CLASS_CMD_ACTION;

            output_msg->msgDesc->link_type = input_msg->linkType;
            output_msg->msgDesc->type      = SID_MSG_TYPE_NOTIFY;
            output_msg->msgDesc->link_mode = SID_LINK_MODE_CLOUD;
            output_msg->msgDesc->msg_desc_attr.tx_attr.ttl_in_seconds = TTL_SECONDS;
            output_msg->msgDesc->msg_desc_attr.tx_attr.num_retries    = NUM_RETRIES;
            output_msg->msgDesc->msg_desc_attr.tx_attr.request_ack    = true;

            appMsg.payload = payloadBuff;
            appMsg.payload_size = SIDparseState.offset;
            break;
        }

    sid_parse_state_init(&SIDparseState, output_msg->msgBody->data, SID_MESSAGE_BUFF_SIZE);
    sid_demo_app_msg_serialize(&SIDparseState, &appDescriptor, &appMsg);
    output_msg->msgBody->size = SIDparseState.offset;
    return 0;
}
