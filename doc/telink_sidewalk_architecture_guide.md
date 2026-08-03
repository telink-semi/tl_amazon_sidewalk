# Telink Sidewalk SDK — Architecture & Getting Started Guide

This guide explains how the Telink Sidewalk SDK is structured, how the pieces fit together, and where to find things. It's intended for embedded developers who are new to the Telink platform and want to build a Sidewalk device.

---

## High-Level Architecture

```
┌─────────────────────────────────────────────────┐
│              Your Application Logic               │
│    (sidewalk_app.c — button handling, send/recv)  │
├─────────────────────────────────────────────────┤
│             Sidewalk API Layer                    │
│   sid_init() / sid_start() / sid_put_msg()       │
├─────────────────────────────────────────────────┤
│         Sidewalk Protocol Stack (library)         │
│        (session, security, framing, routing)      │
├──────────────────┬──────────────────────────────┤
│   BLE PAL        │    Sub-GHz PAL (900 MHz)      │
│ sid_ble_adapter  │    sid_pal_radio (SX126x)     │
├──────────────────┴──────────────────────────────┤
│         BLE Platform Layer (samples/app.c)       │
│   HCI events, GATT, flash, power management      │
├─────────────────────────────────────────────────┤
│        Telink BLE SDK + FreeRTOS                 │
│   blc_sdk_main_loop() + vTaskStartScheduler()    │
├─────────────────────────────────────────────────┤
│   Hardware TL3238E1(D25F/RISC‑V) + 1262x        │
└─────────────────────────────────────────────────┘
```

The SDK has two distinct layers you need to understand:
1. **BLE platform layer** (`samples/app.c`) — shared infrastructure that initializes the BLE controller, handles HCI events, manages flash protection, and provides the GATT attribute table. You typically don't modify this.
2. **Application layer** (`sidewalk/apps/telink/*/sidewalk_app.c`) — your Sidewalk logic. Runs as a FreeRTOS task with an event queue. This is where you write your code.

---

## Execution Flow

```
main() [in sid_900/main.c]
  ├── blc_app_system_init()        ← Clock, GPIO, watchdog
  ├── rf_drv_ble_init()            ← Radio driver
  ├── user_init_normal()           ← BLE stack init (from samples/app.c)
  │     ├── blc_ll_init*()         ← BLE Link Layer
  │     ├── blc_att_*()            ← GATT setup
  │     └── Register HCI/GAP callbacks
  ├── app_start() [FreeRTOS mode]  ← Sidewalk platform init
  │     ├── sid_platform_init()    ← MFG store, radio config
  │     ├── xQueueCreate()         ← Event queue
  │     └── xTaskCreate(main_thread, "sidewalk", ...)
  └── vTaskStartScheduler()        ← FreeRTOS takes over

main_thread() [in sidewalk_app.c]  ← YOUR CODE LIVES HERE
  ├── Setup sid_event_callbacks
  ├── init_and_start_link()        ← sid_init() + sid_start()
  └── while(1):
        xQueueReceive(event_queue)
        switch(event):
          SIDEWALK  → sid_process()
          SEND_HELLO → send_ping()
          FACTORY_RESET → factory_reset()
          FSK_CSS_SWITCH → deinit/reinit link
```

---

## Directory Structure

```
tl_sidewalk_sdk/
├── vendor/Amazon_sidewalk/
│   ├── samples/                        ← BLE platform layer (shared)
│   │   ├── app.c                       ← BLE HCI events, GATT, flash protection
│   │   ├── app_att.c                   ← GATT attribute table
│   │   ├── app_buffer.h                ← BLE buffer sizes
│   │   ├── app_config.h                ← Feature flags, GPIO assignments
│   │   ├── app_freertos.c              ← FreeRTOS task creation helpers
│   │   ├── app_ui.c                    ← Button/LED/keyboard handling
│   │   └── app_uart.c                  ← Debug info output
│   ├── sidewalk/apps/telink/
│   │   ├── sid_900/                    ← Primary sample app (BLE + FSK + CSS)
│   │   │   ├── main.c                  ← Entry point, system init, task start
│   │   │   ├── sidewalk_app.c          ← Application logic (events, send/recv)
│   │   │   ├── app_ble_config.c        ← BLE link configuration
│   │   │   ├── app_subGHz_config.c     ← Sub-GHz radio configuration
│   │   │   └── sid_sdk_app_config.h    ← Sidewalk feature flags
│   │   ├── sid_sbdt/                   ← Bulk Data Transfer (OTA)
│   │   ├── sid_dut/                    ← Device Under Test (qualification CLI)
│   │   └── sid_diagnostics/            ← Radio diagnostics
│   └── sidewalk/apps/common/
│       └── sidewalk_sdk/sal/telink/sid_pal/      PAL implementation
│           ├── ble_adapter/
│           │   ├── sid_ble_adapter.c           ← BLE PAL implementation
│           │   ├── sid_ble_adapter_callbacks.c
│           │   └── sid_ble_advert.c
│           └── include/                        ← PAL headers
└── 3rd-party/freertos-V5/              ← FreeRTOS kernel
```

---

## The sid_900 App — How It Works

`sid_900` is the primary getting-started app. Here's what it does:

| Button | Action |
|--------|--------|
| SW1 (short press)| Toggle FSK ↔ CSS link |
| SW2 | Switch device profile (sub-GHz) |
| SW3 | Toggle BLE connection request |
| SW4 | Send counter value to cloud |
| Any (double press) | Factory reset |

On receive: if message is "on" → turn on LED; otherwise → turn off LED.

The full app logic in `sidewalk_app.c` is ~500 lines, of which ~200 are the event loop and callbacks. The core send function is minimal:

```c
static void send_ping(app_context_t *app_context) {
    struct sid_msg msg = {
        .data = (uint8_t*)&app_context->counter,
        .size = sizeof(uint8_t)
    };
    struct sid_msg_desc desc = {
        .type = SID_MSG_TYPE_NOTIFY,
        .link_type = SID_LINK_TYPE_ANY,
        .link_mode = SID_LINK_MODE_CLOUD,
    };
    sid_put_msg(app_context->sidewalk_handle, &msg, &desc);
    app_context->counter++;
}
```

---

## Link Switching at Runtime

The app supports switching between BLE, FSK, and CSS without rebooting:

```c
static int32_t init_and_start_link(app_context_t *context, struct sid_config *config, uint32_t link_mask) {
    // Deinitialize current link
    sid_deinit(context->sidewalk_handle);
    // Reinitialize with new link mask
    config->link_mask = link_mask;
    sid_init(config, &sid_handle);
    context->sidewalk_handle = sid_handle;
    sid_start(sid_handle, link_mask);
}
```

Link types: `SID_LINK_TYPE_1` (BLE), `SID_LINK_TYPE_2` (FSK), `SID_LINK_TYPE_3` (CSS/LoRa).

---

## BLE Platform Layer (`samples/app.c`)

This ~35KB file is shared infrastructure. It handles:

- **HCI event dispatch** — connection complete, disconnect, connection update
- **GAP/GATT events** — MTU exchange, SMP pairing
- **Flash protection** — lock/unlock around writes and OTA
- **Battery monitoring** — low-voltage detection with deepsleep
- **Flash layout** — detects 1M vs 2M flash and configures MFG/NV addresses

The Sidewalk BLE PAL (`sid_ble_adapter.c`) hooks into this layer via callbacks: `ble_connect_cb()`, `ble_disconnect_cb()`, `ble_mtu_cb()`. These are called from the HCI event handlers in `app.c`.

**You typically don't need to modify `samples/app.c`** unless you're adding non-Sidewalk BLE functionality or changing power management behavior.

---

## Power Management

The SDK uses deepsleep retention mode for low power. The BLE controller manages sleep automatically, with hooks for the Sidewalk sub-GHz radio:

```c
void app_sleep_config(void) {
    blc_ll_registerTelinkControllerEventCallback(BLT_EV_FLAG_SLEEP_ENTER, &app_sid_sleep_enter);
    blc_ll_registerTelinkControllerEventCallback(BLT_EV_FLAG_GPIO_EARLY_WAKEUP, &app_sid_wakeup);
    blc_ll_registerTelinkControllerEventCallback(BLT_EV_FLAG_SUSPEND_EXIT, &app_sid_subg_wakeup);
}
```

Variables marked with `_attribute_ble_data_retention_` survive deepsleep retention cycles.

---

## Building

Using the Telink VS Code Extension:
1. Clone: `git clone https://github.com/telink-semi/tl_amazon_sidewalk`
2. Open in VS Code
3. Open Telink extension → "Project Outline" → select target (e.g., `sid_900` for TL3238E1)
4. Click build

Or via command line:
```bash
cd tl_amazon_sidewalk
mkdir build && cd build
cmake .. -DPROJECT=sid_900
make
```

# Sidewalk SDK 1M Flash Layout Introduction
## Overview
This layout applies to single-core MCUs equipped with 1MB Flash, all addresses shown in hexadecimal format.
- Firmware: `0x00000000`
- MFG blob: `0x000F5000` (changed from `0x000F9000` in SDK V1.0.0.6)

## Flash Memory Partition Table
| Start Address | Partition Name | Detailed Definition |
| ---- | ---- | ---- |
| `0xFF000` | MAC Area | - `0xFF000`: Device MAC address storage<br>- `0xFF008`: Device Serial Number (SN) storage |
| `0xFE000` | Calibration Area | - `0xFE000`: BLE RF trim calibration parameters<br>- `0xFE500`: LoRa RF trim calibration parameters |
| `0xFC000` ~ `0xFD000` | Reserved for future | Unused flash space reserved for subsequent function expansion |
| `0xF8000` ~ `0xFB000` | Run code description (16K) | 16KB dedicated area for runtime code metadata and description data |
| `0xF5000` ~ `0xF7000` | MFG.bin + other | Special manufacturing partition to store MFG blob and other factory configuration data |

## Important Note for MFG Blob
The base address of MFG blob has been adjusted since SDK V1.0.0.6:
- Old address: `0x000F9000`
- Current valid address: `0x000F5000`
This address locates inside the `0xF6000 ~ 0xF7000`  flash storage  region.
---

## Application Configuration Overview (`vendor/Amazon_sidewalk/samples/app_config.h`)

- This header is the **central switch file** for the Telink BLE SDK + Sidewalk application layer,  all features, buffers, board selection, GPIO mappings, flash layout, and log levels are toggled here.
- Before modifying, review the license header and `../common/default_config.h` for default overrides.

## Key Architectural Characteristics

| Aspect | Telink Approach |
|--------|----------------|
| RTOS | FreeRTOS (also supports bare-metal mode) |
| Sidewalk task | Dedicated FreeRTOS task with event queue |
| BLE scheduling | BLE controller runs cooperatively via `blc_sdk_main_loop()` within FreeRTOS |
| Event dispatch | `xQueueSend` / `xQueueReceive` pattern |
| Sleep management | BLE controller callbacks + deepsleep retention |
| Flash management | Platform layer handles lock/unlock |
| Link switching | Runtime `sid_deinit` → `sid_init` → `sid_start` |

---

## Common Pitfalls

1. **Wrong MFG address** — Changed to `0x000F5000` in SDK V1.0.0.6. Old address `0x000F9000` will cause provisioning failures.
2. **Not erasing flash before re-provisioning** — Residual MFG data causes conflicts. Always full-erase first.
3. **Modifying `samples/app.c` HCI callbacks** — The Sidewalk BLE PAL depends on `ble_connect_cb`/`ble_disconnect_cb`/`ble_mtu_cb` being called. Don't filter those events.
4. **Sleep during sub-GHz TX window** — If deepsleep triggers during a Sidewalk TX, the message is lost. The sleep callbacks (`app_sid_sleep_enter`) handle this, but custom modifications can break it.
5. **FreeRTOS stack size** — `main_thread` uses 6144 bytes. Deep call stacks (crypto operations) can overflow. Increase `MAIN_TASK_STACK_SIZE` if you see crashes.
