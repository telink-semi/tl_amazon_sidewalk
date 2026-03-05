/*
 * Copyright 2020-2023 Amazon.com, Inc. or its affiliates. All rights reserved.
 *
 * AMAZON PROPRIETARY/CONFIDENTIAL
 *
 * You may not use this file except in compliance with the terms and
 * conditions set forth in the accompanying LICENSE.txt file.
 *
 * THESE MATERIALS ARE PROVIDED ON AN "AS IS" BASIS. AMAZON SPECIFICALLY
 * DISCLAIMS, WITH RESPECT TO THESE MATERIALS, ALL WARRANTIES, EXPRESS,
 * IMPLIED, OR STATUTORY, INCLUDING THE IMPLIED WARRANTIES OF MERCHANTABILITY,
 * FITNESS FOR A PARTICULAR PURPOSE, AND NON-INFRINGEMENT.
 */

#ifndef SID_LPTSN_API_H
#define SID_LPTSN_API_H

/// @cond sid_ifc_ep_en

/** @file
 *
 * @defgroup SIDEWALK_LPTSN_API Sidewalk Low Power Time Synchronized Network API
 * @brief API for communicating with the Sidewalk Low Power Time Synchronized Network network
 * @{
 * @ingroup  SIDEWALK_LPTSN_API
 */

#ifdef __cplusplus
extern "C" {
#endif

#ifdef __cplusplus
}
#endif

/**
 * The Sidewalk LPTSN library message types.
 * The messages from cloud services to the End device are designated as "Downlink Messages"
 * The messages from End device to Cloud services are designated as "Uplink Messages"
 */
enum sid_lptsn_msg_type {
    /** #SID_LPTSN_MSG_TYPE_GET is used by the sender to retrieve information from the receiver, the sender
     * expects a mandatory response from the receiver. On reception of #SID_LPTSN_MSG_TYPE_GET, the receiver
     * is expected to send a message with type #SID_LPTSN_MSG_TYPE_GET with the same message id it received
     * from the message type #SID_LPTSN_MSG_TYPE_GET.
     * This is to ensure the sender can map message type #SID_LPTSN_MSG_TYPE_GET with the received #SID_LPTSN_MSG_TYPE_GET.
     * Both uplink and downlink messages use this message type.
     * @see sid_lptsn_put_msg().
     * @see on_lptsn_msg_received in #sid_lptsn_event_callbacks.
     */
    SID_LPTSN_MSG_TYPE_GET = 0,
    /** #SID_LPTSN_MSG_TYPE_SET indicates that the sender is expecting the receiver to take an action on receiving the
     * message and the sender does not expect a response.
     * #SID_LPTSN_MSG_TYPE_SET type is used typically by Cloud services to trigger an action to be preformed by the End device.
     * Typical users for this message type are downlink messages.
     */
    SID_LPTSN_MSG_TYPE_SET = 1,
    /** #SID_LPTSN_MSG_TYPE_NOTIFY is used to notify cloud services of any periodic events or events
     * triggered/originated from the device. Cloud services do not typically use #SID_LPTSN_MSG_TYPE_NOTIFY as the
     * nature of messages from cloud services to the devices are explicit commands instead of notifications.
     * Typical users for this message type are uplink messages.
     */
    SID_LPTSN_MSG_TYPE_NOTIFY = 2,
    /** #SID_LPTSN_MSG_TYPE_RESPONSE is sent as a response to the message of type #SID_LPTSN_MSG_TYPE_GET.
     * The sender of #SID_LPTSN_MSG_TYPE_RESPONSE is required to the copy the message id received in
     * the message of type #SID_LPTSN_MSG_TYPE_GET.
     * Both uplink and downlink messages use this message type.
     * @see sid_lptsn_put_msg().
     * @see on_lptsn__msg_received in #sid_lptsn_event_callbacks.
     */
    SID_LPTSN_MSG_TYPE_RESPONSE = 3,
};

/**
 * Describes the state of the Sidewalk LPTSN library.
 */
enum sid_lptsn_state {
    /** Used when the Sidewalk LPTSN library is ready to send and receive messages */
    SID_LPTSN_STATE_READY = 0,
    /** Used when the Sidewalk LPTSN library is unable to send or receive messages, such as when the device
     * is not registered or link gets disconnected or time is not synced */
    SID_LPTSN_STATE_NOT_READY = 1,
    /** Used when the Sidewalk LPTSN library encountered an error. Use sid_lptsn_get_error() for a diagnostic
     * error code */
    SID_LPTSN_STATE_ERROR = 2,
    /** Used when the Sidewalk LPTSN library is ready to send and receive messages only with secure channel
     * establishment completed but device is not registered and time is not synced */
    SID_LPTSN_STATE_SECURE_CHANNEL_READY = 3,
};

/**
 * Describes the status of the Sidewalk LPTSN library.
 */
struct sid_lptsn_status {
    /** The current state */
    enum sid_lptsn_state state;
    /** Details of Sidewalk LPTSN library status */
    uint32_t status_details;
};

/**
 * Describes a message payload.
 */
struct sid_lptsn_msg {
    /** Pointer to buffer to be sent or received */
    void *data;
    /** Size (in bytes) of the buffer pointed to by data */
    size_t size;
};

// /** Attributes applied to the message descriptor on tx */
// struct sid_lptsn_msg_desc_tx_attributes {
//     /** Whether this message requests an ack from the AWS IOT service */
//     bool request_ack;
//     /** Number of retries the Sidewalk LPTSN stack needs to preform in case the
//      * ack is not received. Setting not applicable if request_ack is set to false
//      */
//     uint8_t num_retries;
//     /** Total time the sidewalk LPTSN stack holds the message in its queue in case
//      * the ack is not received. Setting not applicable if request_ack is set to false
//      */
//     uint16_t ttl_in_seconds;
// };

// /** Attributes with which the message is received */
// struct sid_lptsn_msg_desc_rx_attributes {
//     /** Whether the message received is an acknowledgement. Acknowledgements have the same
//      * message id as that of message sent but with zero payload size . See #sid_lptsn_msg_desc_tx_attributes
//      */
//     bool is_msg_ack;
//     /** Whether the message received is a duplicate. If a message arrives at the sidewalk LPTSN stack
//      * with message id and payload size equal to an already reported message, this message is marked as a duplicate
//      * See #SID_OPTION_SET_MSG_POLICY_FILTER_DUPLICATES
//      */
//     bool is_msg_duplicate;
//     /** Whether the message received has requested an acknowledgement to be sent, Acknowledgements have the
//      * same message id as that of the received message. Sidewalk LPTSN stack immediately queues an acknowledgement
//      * to the sender before propagating this message to the user*/
//     bool ack_requested;
//     /** rssi of the received message */
//     int8_t rssi;
//     /** snr of the received message */
//     int8_t snr;
// };

// union sid_lptsn_msg_desc_attributes {
//     /** Attributes that are applied only when message is transmitted. See #sid_lptsn_put_msg */
//     struct sid_lptsn_msg_desc_tx_attributes tx_attr;
//     /** Attributes reported per message when the message is reported to the user. See #on_lptsn__msg_received */
//     struct sid_lptsn_msg_desc_rx_attributes rx_attr;
// };

/**
 * A message descriptor given by the Sidewalk library to identify a message.
 */
struct sid_lptsn_msg_desc {
    /** The message type */
    enum sid_lptsn_msg_type type;
    // /** The link mode on which message is sent or received */
    // enum sid_lptsn_link_mode link_mode;
    /** The id associated with a message, generated by the Sidewalk LPTSN library
     * The maximum value the id can take is 0x3FFF after which the id resets to 1
     */
    uint16_t id;
    // /** Attributes applied to the message */
    // union sid_lptsn_msg_desc_attributes msg_desc_attr;
};

/**
 * Opaque handle returned by sid_lptsn_init().
 */
struct sid_lptsn_handle;

/**
 * The set of callbacks a user can register through sid_init().
 */
struct sid_lptsn_event_callbacks {
    /** A place where you can store user data */
    void *context;

    /**
     * Callback to invoke when any Sidewalk LPTSN event occurs.
     *
     * The Sidewalk LPTSN library invokes this callback when there is at least one event to process,
     * including internal events. Upon receiving this callback you are required to schedule a call
     * to sid_lptsn_process() within your main loop or running context.
     *
     * @warning sid_lptsn_process() MUST NOT be called from within the #on_event callback to avoid
     * re-entrancy and recursion problems.
     *
     * @see sid_lptsn_process
     *
     * @param[in] in_isr  true if invoked from within an ISR context, false otherwise.
     * @param[in] context The context pointer given in sid_lptsn_event_callbacks.context
     */
    void (*on_lptsn_event)(bool in_isr, void *context);

    /**
     * Callback to invoke when a LPTSN message from the Sidewalk LPTSN network is received.
     *
     * @warning sid_lptsn_put_msg() MUST NOT be called from within the #on_lptsn_msg_received callback
     * to avoid re-entrancy and recursion problems.
     *
     * @param[in] msg_desc A pointer to the received message descriptor, which is never NULL.
     * @param[in] msg      A pointer to the received message payload, which is never NULL.
     * @param[in] context  The context pointer given in sid_lptsn_event_callbacks.context
     */
    void (*on_lptsn_msg_received)(const struct sid_lptsn_msg_desc *msg_desc, const struct sid_lptsn_msg *msg, void *context);

    /**
     * Callback to invoke when a message was successfully delivered to the Sidewalk LPTSN network.
     *
     * @param[in] msg_desc A pointer to the sent message descriptor, which is never NULL.
     * @param[in] context  The context pointer given in sid_lptsn_event_callbacks.context
     */
    void (*on_lptsn_msg_sent)(const struct sid_lptsn_msg_desc *msg_desc, void *context);

    /**
     * Callback to invoke when a queued message failed to be delivered to the Sidewalk LPTSN network.
     *
     * A user can use this notification to schedule retrying sending a message or invoke other error
     * handling.
     *
     * @see sid_lptsn_put_msg
     *
     * @warning sid_lptsn_put_msg() MUST NOT be called from within the #on_send_error callback to
     * avoid re-entrancy and recursion problems.
     *
     * @param[in] error    The error code associated with the failure
     * @param[in] msg_desc A pointer to the unsent message descriptor, which is never NULL.
     * @param[in] context  The context pointer given in sid_lptsn_event_callbacks.context
     */
    void (*on_send_error)(sid_error_t error, const struct sid_lptsn_msg_desc *msg_desc, void *context);

    /**
     * Callback to invoke when the Sidewalk LPTSN library status changes.
     *
     * Once sid_lptsn_start() is called, a #SID_LPTSN_STATE_READY status indicates the library is ready to
     * accept messages sid_lptsn_put_msg().
     *
     * When receiving #SID_LPTSN_STATE_ERROR, you can call sid_lptsn_get_error() from within the
     * #on_status_changed callback context to obtain more detail about the error condition.
     * Receiving this status means the LPTSN library encountered a fatal condition and won't be
     * able to proceed. Hence, this notification is mostly for diagnostic purposes.
     *
     * @param[in] status  The current status, valid until the next invocation of this callback.
     * @param[in] context The context pointer given in sid_lptsn_event_callbacks.context
     */
    void (*on_status_changed)(const struct sid_lptsn_status *status, void *context);

    // /**
    //  * Callback to invoke when the Sidewalk LPTSN library receives a factory reset from the cloud service.
    //  *
    //  * On receiving the factory reset from the cloud service, the sidewalk library clears its
    //  * configuration from the non volatile storage and reset its state accordingly.
    //  * The sidewalk link status resets to #SID_STATE_DISABLED.
    //  * This callback is then called by the sidewalk library to notify you to handle the factory reset command
    //  *
    //  * The device needs to successfully complete device registration with the cloud services for the sidewalk
    //  * library to send and receive messages
    //  *
    //  * @param[in] context The context pointer given in sid_lptsn_event_callbacks.context
    //  */
    // void (*on_factory_reset)(void *context);
};

/**
 * Describes the configuration associated with the chosen link.
 */
struct sid_lptsn_config {
    /** The event callbacks associated with the chosen link. */
    struct sid_lptsn_event_callbacks *event_callbacks;
    /** sid_pltsn configuration. */
    const void *config;
};

/**
 * Initializes the Sidewalk LPTSN library for the chosen link type.
 *
 * sid_lptsn_init() can only be called once for the given sid_lptsn_config.link_type unless sid_lptsn_deinit() is
 * called first.
 *
 * @see sid_lptsn_deinit
 *
 * @param[in] config  The required configuration in order to properly initialize sidewalk lptsn.
 * @param[out] handle A pointer where the the opaque handle type will be stored. `handle` is set to
 *                    NULL on error.
 *
 * @returns #SID_ERROR_NONE                on success.
 * @returns #SID_ERROR_ALREADY_INITIALIZED if Sidewalk LPTSN was already initialized.
 */
sid_error_t sid_lptsn_init(const struct sid_config *config, struct sid_lptsn_handle **handle);

/**
 * De-initialize the portions of the Sidewalk library associated with the given handle.
 *
 * @see sid_lptsn_init
 *
 * @param[in] handle A pointer to the handle returned by sid_lptsn_init()
 *
 * @returns #SID_ERROR_NONE in case of success
 */
sid_error_t sid_lptsn_deinit(struct sid_lptsn_handle *handle);

/**
 * Makes the Sidewalk LPTSN library start operating.
 *
 * The notifications registered during sid_lptsn_init() are invoked once sid_lptsn_start() is called.
 *
 * TBD
 *
 * @note User can only start a link_type that was initialized in sid_lptsn_init()
 *
 * @see sid_lptsn_stop
 *
 * @param[in] handle A pointer to the handle returned by sid_lptsn_init()
 * @param[in] link_mask The links that need to started.
 *
 * @returns #SID_ERROR_NONE in case of success.
 */
sid_error_t sid_lptsn_start(struct sid_lptsn_handle *handle, uint32_t link_mask);

/**
 * Makes the Sidewalk LPTSN library stop operating.
 *
 * No messages will be sent or received and no notifications will occur after sid_lptsn_stop() is called.
 * Link status will be changed to disconnected and time sync status is cached after sid_lptsn_stop() is
 * called.
 *
 * TBD
 *
 * @note User can only stop a link_type that was initialized in sid_lptsn_init()
 *
 * @see sid_lptsn_start
 *
 * @warning sid_lptsn_stop() should be called in the same caller context as sid_lptsn_process().
 *
 * @warning sid_lptsn_stop() must not be called from within the caller context of any of the
 * sid_lptsn_event_callbacks registered during sid_lptsn_init() to avoid re-entrancy and recursion problems.
 *
 * @param[in] handle A pointer to the handle returned by sid_lptsn_init()
 *
 * @returns #SID_ERROR_NONE in case of success.
 */
sid_error_t sid_lptsn_stop(struct sid_lptsn_handle *handle);

/**
 * Process Sidewalk LPTSN events.
 *
 * When there are no events to process, the function returns immediately.
 * When events are present, sid_lptsn_process() invokes the sid_lptsn_event_callbacks registered during
 * sid_lptsn_init() within its calling context. You may not receive any callbacks for internal events.
 *
 * You are required to schedule sid_lptsn_process() to run within your main-loop or running context when
 * the sid_lptsn_event_callbacks.on_event callback is received.
 *
 * Although not recommended for efficiency and power usage reasons, sid_lptsn_process() can also be called
 * even if sid_lptsn_event_callbacks.on_event has not been received, to support main loops that operate in
 * a polling manner.
 *
 * @warning sid_lptsn_process() must not be called from within the caller context of any of the
 * sid_lptsn_event_callbacks registered during sid_lptsn_init() to avoid re-entrancy and recursion problems.
 *
 * @see sid_lptsn_init
 * @see sid_lptsn_start
 * @see sid_lptsn_event_callbacks
 *
 * @param[in] handle A pointer to the handle returned by sid_lptsn_init()
 *
 * @returns #SID_ERROR_NONE in case of success.
 * @returns #SID_ERROR_STOPPED if sid_lptsn_start() has not been called.
 */
sid_error_t sid_lptsn_process(struct sid_lptsn_handle *handle);

/**
 * Queues a message.
 *
 * @note msg_desc can be used to correlate this message with the sid_lptsn_event_callbacks.on_msg_sent
 * and sid_lptsn_event_callbacks.on_send_error callbacks.
 *
 * @note When sending #SID_LPTSN_MSG_TYPE_RESPONSE in response to #SID_LPTSN_MSG_TYPE_GET, the user is expected to fill
 * the id field of message descriptor with id from the corresponding #SID_LPTSN_MSG_TYPE_GET message descriptor.
 * This allows the sid_lptsn_api to match each unique #SID_LPTSN_MSG_TYPE_RESPONSE with #SID_LPTSN_MSG_TYPE_GET.
 *
 * @param[in]  handle   A pointer to the handle returned by sid_lptsn_init()
 * @param[in]  msg      The message data to send
 * @param[out] msg_desc The message descriptor this function fills which identifies this message.
 *                      Only valid when #SID_ERROR_NONE is returned.
 *
 * @returns #SID_ERROR_NONE when the message is successfully placed in the transmit queue.
 * @returns #SID_ERROR_TRY_AGAIN when there is no space in the transmit queue.
 */
sid_error_t sid_lptsn_put_msg(struct sid_lptsn_handle *handle, const struct sid_lptsn_msg *msg, struct sid_lptsn_msg_desc *msg_desc);

/**
 * Get the current error code.
 *
 * When the sid_lptsn_event_callbacks.on_status_changed callback is called with a #SID_LPTSN_STATE_ERROR
 * you can use this function to retrieve the detailed error code. The error code will only be valid
 * in the calling context of the sid_lptsn_event_callbacks.on_status_changed callback.
 *
 * @param[in] handle A pointer to the handle returned by sid_lptsn_init()
 *
 * @returns The current error code
 */
sid_error_t sid_lptsn_get_error(struct sid_lptsn_handle *handle);


/**
 * Set destination ID for messages.
 *
 * By default, the destination ID is set to #SID_MSG_DESTINATION_AWS_IOT_CORE unless
 * changed by sid_lptsn_set_msg_dest_id(). The destination ID is retained until the device
 * resets or its changed by another invocation of sid_lptsn_set_msg_dest_id().
 *
 * @see sid_lptsn_put_msg().
 *
 * @param[in] handle A pointer to the handle returned by sid_lptsn_init().
 * @param[in] id     The new destination ID.
 *
 * @returns #SID_ERROR_NONE on success.
 */
sid_error_t sid_lptsn_set_msg_dest_id(struct sid_lptsn_handle *handle, uint32_t id);

/**
 * Get current status from Sidewalk LPTSN library.
 *
 * @warning sid_lptsn_get_status() should be called in the same caller context as sid_lptsn_process().
 *
 * @warning sid_lptsn_get_status() must not be called from within the caller context of any of the
 * sid_lptsn_event_callbacks registered during sid_lptsn_init() to avoid re-entrancy and recursion problems.
 *
 * @param[in] handle A pointer to the handle returned by sid_lptsn_init()
 * @param[out] current_status A pointer to store the sdk current status
 *
 * @returns #SID_ERROR_NONE in case of success.
 * @returns #SID_ERROR_INVALID_ARGS when Sidewalk LPTSN library is not initialized.
 */
sid_error_t sid_lptsn_get_status(struct sid_lptsn_handle *handle, struct sid_lptsn_status *current_status);


/** @} */

/// @endcond

#endif // SID_API_LPTSN_H
