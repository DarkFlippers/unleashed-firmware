#pragma once

#include <stdlib.h>
#include <furi/core/thread.h>
#include <furi/core/mutex.h>
#include "../../types/token_info.h"

typedef uint8_t TotpGenerateCodeWorkerEvent;

typedef void (*TOTP_NEW_CODE_GENERATED_HANDLER)(bool time_left, void* context);
typedef void (*TOTP_CODE_LIFETIME_CHANGED_HANDLER)(float code_lifetime_percent, void* context);

typedef struct {
    char* code_buffer;
    FuriThread* thread;
    FuriMutex* code_buffer_sync;
    TokenInfo** token_info;
    float timezone_offset;
    uint8_t* iv;
    TOTP_NEW_CODE_GENERATED_HANDLER on_new_code_generated_handler;
    void* on_new_code_generated_handler_context;
    TOTP_CODE_LIFETIME_CHANGED_HANDLER on_code_lifetime_changed_handler;
    void* on_code_lifetime_changed_handler_context;
} TotpGenerateCodeWorkerContext;

enum TotGenerateCodeWorkerEvents {
    TotpGenerateCodeWorkerEventReserved = 0b00,
    TotpGenerateCodeWorkerEventStop = 0b01,
    TotpGenerateCodeWorkerEventForceUpdate = 0b10
};

TotpGenerateCodeWorkerContext* totp_generate_code_worker_start(
    char* code_buffer,
    TokenInfo** token_info,
    FuriMutex* code_buffer_sync,
    float timezone_offset,
    uint8_t* iv);
void totp_generate_code_worker_stop(TotpGenerateCodeWorkerContext* context);
void totp_generate_code_worker_notify(
    TotpGenerateCodeWorkerContext* context,
    TotpGenerateCodeWorkerEvent event);
void totp_generate_code_worker_set_code_generated_handler(
    TotpGenerateCodeWorkerContext* context,
    TOTP_NEW_CODE_GENERATED_HANDLER on_new_code_generated_handler,
    void* on_new_code_generated_handler_context);
void totp_generate_code_worker_set_lifetime_changed_handler(
    TotpGenerateCodeWorkerContext* context,
    TOTP_CODE_LIFETIME_CHANGED_HANDLER on_code_lifetime_changed_handler,
    void* on_code_lifetime_changed_handler_context);