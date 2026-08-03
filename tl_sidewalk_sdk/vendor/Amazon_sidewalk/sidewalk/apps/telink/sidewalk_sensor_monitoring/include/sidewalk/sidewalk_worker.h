#pragma once
#include <sid_api.h>
#include "stdint.h"

typedef enum {
    SIDEWALK_WORKER_DO_PLATFORM_INIT,
    SIDEWALK_WORKER_DO_BOOTSTRAP,
    SIDEWALK_WORKER_DO_SID_LOOP,
    SIDEWALK_WORKER_DO_SEND_MESSAGE,
    SIDEWALK_WORKER_DO_CHANGE_LINK,
} sidewalkWorkerJobType_t;


typedef struct {
    sidewalkWorkerJobType_t jobType;
    void * jobContext;
} sidewalkWorkerJobItem_t;

typedef struct {
    struct sid_handle *handle;
    struct sid_config config;
    struct sid_status last_status;
} sidewalk_worker_instance_t;



void sidewalk_woker_start(struct sid_event_callbacks *callbackList_p);
void sidewalk_worker_take_job(sidewalkWorkerJobType_t jobType, void * jobContext);

