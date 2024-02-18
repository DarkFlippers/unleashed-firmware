#pragma once

#include <stdint.h>
#include <core/common_defines.h>

#ifdef __cplusplus
extern "C" {
#endif

typedef enum {
    BleEventNotAck,
    BleEventAckFlowEnable,
    BleEventAckFlowDisable,
} BleEventAckStatus;

typedef enum {
    BleEventFlowDisable,
    BleEventFlowEnable,
} BleEventFlowStatus;

/* Using other types so not to leak all the BLE stack headers 
    (we don't have a wrapper for them yet)
 * Event data is hci_uart_pckt*
 * Context is user-defined 
 */
typedef BleEventAckStatus (*BleSvcEventHandlerCb)(void* event, void* context);

typedef struct GapEventHandler GapSvcEventHandler;

/* To be called once at BLE system startup */
void ble_event_dispatcher_init(void);

/* To be called at stack reset - ensures that all handlers are unregistered */
void ble_event_dispatcher_reset(void);

BleEventFlowStatus ble_event_dispatcher_process_event(void* payload);

/* Final handler for event not ack'd by services - to be implemented by app */
BleEventFlowStatus ble_event_app_notification(void* pckt);

/* Add a handler to the list of handlers */
FURI_WARN_UNUSED GapSvcEventHandler*
    ble_event_dispatcher_register_svc_handler(BleSvcEventHandlerCb handler, void* context);

/* Remove a handler from the list of handlers */
void ble_event_dispatcher_unregister_svc_handler(GapSvcEventHandler* handler);

#ifdef __cplusplus
}
#endif
