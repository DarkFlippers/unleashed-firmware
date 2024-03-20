#pragma once

#ifdef __cplusplus
extern "C" {
#endif

/* Controls for thread handling SHCI & HCI event queues. Used internally. */

void ble_event_thread_start(void);

void ble_event_thread_stop(void);

#ifdef __cplusplus
}
#endif
