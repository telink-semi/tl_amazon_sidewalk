#include "tl_common.h"
#include "tl_wrappers.h"
#include "sid_ble_adapter.h"
#include "utility.h"


void debugwait(void)
{
#if(TLKAPI_DEBUG_ENABLE)
    while (tlkapi_debug_isBusy()) {
        tlkapi_debug_handler();
    }
#endif
}

void log_buffer(const void *buff, uint16_t len)
{
    const uint8_t       *b_p  = buff;
    uint16_t            to_go = len;

    while (to_go != 0) {
        uint16_t chunk_len;

        chunk_len = min(32, to_go);
        TL_LOG_I("%s", hex_to_str(b_p, chunk_len));
        debugwait();
        b_p += chunk_len;
        to_go -= chunk_len;
    }
}