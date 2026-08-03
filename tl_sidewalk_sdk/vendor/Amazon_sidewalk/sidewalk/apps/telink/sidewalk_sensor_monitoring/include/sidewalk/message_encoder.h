#pragma once
#include "sid_api.h"
#include "sidewalk/sid_demo_types.h"
#include "sidewalk/sid_demo_parser.h"

#define APP_PAYLOAD_BUFF_SIZE 128
#define SID_MESSAGE_BUFF_SIZE 256

#define TTL_SECONDS         (60)
#define NUM_RETRIES         (3)

typedef enum {
    APP_MESSAGE_CAPABILITIES,
    APP_MESSAGE_BUTTONS_REPORT,
    APP_MESSAGE_SENSORS_REPORT,
    APP_MESSGAE_IMU,
    APP_MESSAGE_LEDS_RESPONSE,
    APP_MESSAGE_LINK_TYPE_RESPONSE
} app_message_type_t;



typedef struct {
    struct sid_demo_capability_discovery deviceCapabilities;
} app_message_capabilities_t;


typedef struct {
    struct sid_demo_action_notification buttonsNotification;
} app_message_buttons_notification_t;

typedef struct {
    struct sid_demo_action_notification sensorsNotification;
} app_message_sensors_notification_t;

typedef struct {
    struct sid_demo_action_resp LEDresponse;
} app_message_LEDs_response_t;

typedef struct {
    struct sid_demo_action_resp linkTypeResponse;
} app_message_link_type_response_t;

typedef struct {
    app_message_type_t messageType;
    uint32_t linkType;

    union {
        app_message_capabilities_t capabilities;
        app_message_buttons_notification_t buttonsReport;
        app_message_LEDs_response_t ledsResponse;
        app_message_sensors_notification_t sensorsReport;
        app_message_link_type_response_t linkTypeResponse;

    } data;

} app_message_to_encode_t;


typedef struct {
     struct sid_msg *msgBody;
     struct sid_msg_desc *msgDesc;
} app_message_to_send_t;


int encode_message(app_message_to_encode_t *input_msg, app_message_to_send_t *output_msg);
