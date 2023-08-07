#pragma once

#include <stdint.h>
#include <stdbool.h>
#include <furi/core/mutex.h>
#include "../../types/automation_kb_layout.h"

typedef uint8_t TotpUsbTypeCodeWorkerEvent;

typedef struct TotpUsbTypeCodeWorkerContext TotpUsbTypeCodeWorkerContext;

/**
 * @brief USB token input automation worker events
 */
enum TotpUsbTypeCodeWorkerEvents {

    /**
     * @brief Reserved, should not be used anywhere
     */
    TotpUsbTypeCodeWorkerEventReserved = 0b00,

    /**
     * @brief Stop worker
     */
    TotpUsbTypeCodeWorkerEventStop = 0b01,

    /**
     * @brief Trigger token input automation
     */
    TotpUsbTypeCodeWorkerEventType = 0b10
};

/**
 * @brief Starts USB token input automation worker
 * @param code_buffer code buffer to be used to automate
 * @param code_buffer_size code buffer size
 * @param code_buffer_sync code buffer synchronization primitive
 * @param keyboard_layout keyboard layout to be used
 * @return worker context
 */
TotpUsbTypeCodeWorkerContext* totp_usb_type_code_worker_start(
    char* code_buffer,
    uint8_t code_buffer_size,
    FuriMutex* code_buffer_sync,
    AutomationKeyboardLayout keyboard_layout);

/**
 * @brief Stops USB token input automation worker
 * @param context worker context
 */
void totp_usb_type_code_worker_stop(TotpUsbTypeCodeWorkerContext* context);

/**
 * @brief Notifies USB token input automation worker with a given event
 * @param context worker context
 * @param event event to notify worker with
 * @param flags event flags
 */
void totp_usb_type_code_worker_notify(
    TotpUsbTypeCodeWorkerContext* context,
    TotpUsbTypeCodeWorkerEvent event,
    uint8_t flags);
