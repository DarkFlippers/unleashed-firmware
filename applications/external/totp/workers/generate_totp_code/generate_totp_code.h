#pragma once

#include <stdlib.h>
#include <furi/core/mutex.h>
#include "../../types/token_info.h"

typedef uint8_t TotpGenerateCodeWorkerEvent;

typedef void (*TOTP_NEW_CODE_GENERATED_HANDLER)(bool time_left, void* context);
typedef void (*TOTP_CODE_LIFETIME_CHANGED_HANDLER)(float code_lifetime_percent, void* context);

typedef struct TotpGenerateCodeWorkerContext TotpGenerateCodeWorkerContext;

/**
 * @brief Generate token worker events
 */
enum TotGenerateCodeWorkerEvents {

    /**
     * @brief Reserved, should not be used anywhere
     */
    TotpGenerateCodeWorkerEventReserved = 0b00,

    /**
     * @brief Stop worker
     */
    TotpGenerateCodeWorkerEventStop = 0b01,

    /**
     * @brief Trigger token input automation
     */
    TotpGenerateCodeWorkerEventForceUpdate = 0b10
};

/**
 * @brief Starts generate code worker
 * @param code_buffer code buffer to generate code to
 * @param token_info token info to be used to generate code
 * @param code_buffer_sync code buffer synchronization primitive
 * @param timezone_offset timezone offset to be used to generate code
 * @param crypto_settings crypto settings
 * @return worker context
 */
TotpGenerateCodeWorkerContext* totp_generate_code_worker_start(
    char* code_buffer,
    const TokenInfo* token_info,
    FuriMutex* code_buffer_sync,
    float timezone_offset,
    const CryptoSettings* crypto_settings);

/**
 * @brief Stops generate code worker
 * @param context worker context
 */
void totp_generate_code_worker_stop(TotpGenerateCodeWorkerContext* context);

/**
 * @brief Notifies generate code worker with a given event
 * @param context worker context
 * @param event event to notify worker with
 */
void totp_generate_code_worker_notify(
    TotpGenerateCodeWorkerContext* context,
    TotpGenerateCodeWorkerEvent event);

/**
 * @brief Sets new handler for "on new code generated" event
 * @param context worker context
 * @param on_new_code_generated_handler handler
 * @param on_new_code_generated_handler_context handler context 
 */
void totp_generate_code_worker_set_code_generated_handler(
    TotpGenerateCodeWorkerContext* context,
    TOTP_NEW_CODE_GENERATED_HANDLER on_new_code_generated_handler,
    void* on_new_code_generated_handler_context);

/**
 * @brief Sets new handler for "on code lifetime changed" event
 * @param context worker context
 * @param on_code_lifetime_changed_handler handler
 * @param on_code_lifetime_changed_handler_context handler context 
 */
void totp_generate_code_worker_set_lifetime_changed_handler(
    TotpGenerateCodeWorkerContext* context,
    TOTP_CODE_LIFETIME_CHANGED_HANDLER on_code_lifetime_changed_handler,
    void* on_code_lifetime_changed_handler_context);