#include "furi_hal_serial_control.h"
#include "furi_hal_serial_types_i.h"
#include "furi_hal_serial.h"

#include <furi.h>
#include <toolbox/api_lock.h>

#define TAG "FuriHalSerialControl"

typedef enum {
    FuriHalSerialControlMessageTypeStop,
    FuriHalSerialControlMessageTypeSuspend,
    FuriHalSerialControlMessageTypeResume,
    FuriHalSerialControlMessageTypeAcquire,
    FuriHalSerialControlMessageTypeRelease,
    FuriHalSerialControlMessageTypeLogging,
} FuriHalSerialControlMessageType;

typedef struct {
    FuriHalSerialControlMessageType type;
    FuriApiLock api_lock;
    void* input;
    void* output;
} FuriHalSerialControlMessage;

typedef struct {
    const FuriHalSerialId id;
    const uint32_t baud_rate;
} FuriHalSerialControlMessageInputLogging;

typedef struct {
    FuriHalSerialHandle handles[FuriHalSerialIdMax];
    FuriMessageQueue* queue;
    FuriThread* thread;

    // Logging
    FuriHalSerialId log_config_serial_id;
    uint32_t log_config_serial_baud_rate;
    FuriLogHandler log_handler;
    FuriHalSerialHandle* log_serial;
} FuriHalSerialControl;

FuriHalSerialControl* furi_hal_serial_control = NULL;

static void furi_hal_serial_control_log_callback(const uint8_t* data, size_t size, void* context) {
    FuriHalSerialHandle* handle = context;
    furi_hal_serial_tx(handle, data, size);
}

static void furi_hal_serial_control_log_set_handle(FuriHalSerialHandle* handle) {
    if(furi_hal_serial_control->log_serial) {
        furi_log_remove_handler(furi_hal_serial_control->log_handler);
        furi_hal_serial_deinit(furi_hal_serial_control->log_serial);
        furi_hal_serial_control->log_serial = NULL;
    }

    if(handle) {
        furi_hal_serial_control->log_serial = handle;
        furi_hal_serial_init(
            furi_hal_serial_control->log_serial,
            furi_hal_serial_control->log_config_serial_baud_rate);
        furi_hal_serial_control->log_handler.callback = furi_hal_serial_control_log_callback;
        furi_hal_serial_control->log_handler.context = furi_hal_serial_control->log_serial;
        furi_log_add_handler(furi_hal_serial_control->log_handler);
    }
}

static int32_t furi_hal_serial_control_thread(void* args) {
    UNUSED(args);

    bool should_continue = true;
    while(should_continue || furi_message_queue_get_count(furi_hal_serial_control->queue) > 0) {
        FuriHalSerialControlMessage message = {0};
        FuriStatus status =
            furi_message_queue_get(furi_hal_serial_control->queue, &message, FuriWaitForever);
        furi_check(status == FuriStatusOk);

        if(message.type == FuriHalSerialControlMessageTypeStop) {
            should_continue = false;
        } else if(message.type == FuriHalSerialControlMessageTypeSuspend) {
            for(size_t i = 0; i < FuriHalSerialIdMax; i++) {
                furi_hal_serial_tx_wait_complete(&furi_hal_serial_control->handles[i]);
                furi_hal_serial_suspend(&furi_hal_serial_control->handles[i]);
            }
            api_lock_unlock(message.api_lock);
        } else if(message.type == FuriHalSerialControlMessageTypeResume) {
            for(size_t i = 0; i < FuriHalSerialIdMax; i++) {
                furi_hal_serial_resume(&furi_hal_serial_control->handles[i]);
            }
            api_lock_unlock(message.api_lock);
        } else if(message.type == FuriHalSerialControlMessageTypeAcquire) {
            FuriHalSerialId serial_id = *(FuriHalSerialId*)message.input;
            if(furi_hal_serial_control->handles[serial_id].in_use) {
                *(FuriHalSerialHandle**)message.output = NULL;
            } else {
                // Logging
                if(furi_hal_serial_control->log_config_serial_id == serial_id) {
                    furi_hal_serial_control_log_set_handle(NULL);
                }
                // Return handle
                furi_hal_serial_control->handles[serial_id].in_use = true;
                *(FuriHalSerialHandle**)message.output =
                    &furi_hal_serial_control->handles[serial_id];
            }
            api_lock_unlock(message.api_lock);
        } else if(message.type == FuriHalSerialControlMessageTypeRelease) {
            FuriHalSerialHandle* handle = *(FuriHalSerialHandle**)message.input;
            furi_assert(handle->in_use);
            furi_hal_serial_deinit(handle);
            handle->in_use = false;

            // Return back logging
            if(furi_hal_serial_control->log_config_serial_id == handle->id) {
                furi_hal_serial_control_log_set_handle(handle);
            }
            api_lock_unlock(message.api_lock);
        } else if(message.type == FuriHalSerialControlMessageTypeLogging) {
            // Set new configuration
            FuriHalSerialControlMessageInputLogging* message_input = message.input;
            furi_hal_serial_control->log_config_serial_id = message_input->id;
            furi_hal_serial_control->log_config_serial_baud_rate = message_input->baud_rate;
            // Apply new configuration
            FuriHalSerialHandle* handle = NULL;
            if(furi_hal_serial_control->log_config_serial_id < FuriHalSerialIdMax) {
                handle = &furi_hal_serial_control
                              ->handles[furi_hal_serial_control->log_config_serial_id];
            }
            furi_hal_serial_control_log_set_handle(handle);
            api_lock_unlock(message.api_lock);
        } else {
            furi_crash("Invalid parameter");
        }
    }

    return 0;
}

void furi_hal_serial_control_init(void) {
    furi_check(furi_hal_serial_control == NULL);
    // Allocate resources
    furi_hal_serial_control = malloc(sizeof(FuriHalSerialControl));
    furi_hal_serial_control->handles[FuriHalSerialIdUsart].id = FuriHalSerialIdUsart;
    furi_hal_serial_control->handles[FuriHalSerialIdLpuart].id = FuriHalSerialIdLpuart;
    furi_hal_serial_control->queue =
        furi_message_queue_alloc(8, sizeof(FuriHalSerialControlMessage));
    furi_hal_serial_control->thread =
        furi_thread_alloc_ex("SerialControlDriver", 512, furi_hal_serial_control_thread, NULL);
    furi_thread_mark_as_service(furi_hal_serial_control->thread);
    furi_thread_set_priority(furi_hal_serial_control->thread, FuriThreadPriorityHighest);
    furi_hal_serial_control->log_config_serial_id = FuriHalSerialIdMax;
    // Start control plane thread
    furi_thread_start(furi_hal_serial_control->thread);
}

void furi_hal_serial_control_deinit(void) {
    furi_check(furi_hal_serial_control);
    // Stop control plane thread
    FuriHalSerialControlMessage message;
    message.type = FuriHalSerialControlMessageTypeStop;
    furi_message_queue_put(furi_hal_serial_control->queue, &message, FuriWaitForever);
    furi_thread_join(furi_hal_serial_control->thread);
    // Release resources
    furi_thread_free(furi_hal_serial_control->thread);
    furi_message_queue_free(furi_hal_serial_control->queue);
    free(furi_hal_serial_control);
}

void furi_hal_serial_control_suspend(void) {
    furi_check(furi_hal_serial_control);

    FuriHalSerialControlMessage message;
    message.type = FuriHalSerialControlMessageTypeSuspend;
    message.api_lock = api_lock_alloc_locked();
    furi_message_queue_put(furi_hal_serial_control->queue, &message, FuriWaitForever);
    api_lock_wait_unlock_and_free(message.api_lock);
}

void furi_hal_serial_control_resume(void) {
    furi_check(furi_hal_serial_control);

    FuriHalSerialControlMessage message;
    message.type = FuriHalSerialControlMessageTypeResume;
    message.api_lock = api_lock_alloc_locked();
    furi_message_queue_put(furi_hal_serial_control->queue, &message, FuriWaitForever);
    api_lock_wait_unlock_and_free(message.api_lock);
}

FuriHalSerialHandle* furi_hal_serial_control_acquire(FuriHalSerialId serial_id) {
    furi_check(furi_hal_serial_control);

    FuriHalSerialHandle* output = NULL;

    FuriHalSerialControlMessage message;
    message.type = FuriHalSerialControlMessageTypeAcquire;
    message.api_lock = api_lock_alloc_locked();
    message.input = &serial_id;
    message.output = &output;
    furi_message_queue_put(furi_hal_serial_control->queue, &message, FuriWaitForever);
    api_lock_wait_unlock_and_free(message.api_lock);

    return output;
}

void furi_hal_serial_control_release(FuriHalSerialHandle* handle) {
    furi_check(furi_hal_serial_control);
    furi_check(handle);

    FuriHalSerialControlMessage message;
    message.type = FuriHalSerialControlMessageTypeRelease;
    message.api_lock = api_lock_alloc_locked();
    message.input = &handle;
    furi_message_queue_put(furi_hal_serial_control->queue, &message, FuriWaitForever);
    api_lock_wait_unlock_and_free(message.api_lock);
}

void furi_hal_serial_control_set_logging_config(FuriHalSerialId serial_id, uint32_t baud_rate) {
    furi_check(serial_id <= FuriHalSerialIdMax);
    furi_check(baud_rate >= 9600 && baud_rate <= 4000000);

    // Very special case of updater, where RTC initialized before kernel start
    if(!furi_hal_serial_control) return;

    FuriHalSerialControlMessageInputLogging message_input = {
        .id = serial_id,
        .baud_rate = baud_rate,
    };
    FuriHalSerialControlMessage message;
    message.type = FuriHalSerialControlMessageTypeLogging;
    message.api_lock = api_lock_alloc_locked();
    message.input = &message_input;
    furi_message_queue_put(furi_hal_serial_control->queue, &message, FuriWaitForever);
    api_lock_wait_unlock_and_free(message.api_lock);
}
