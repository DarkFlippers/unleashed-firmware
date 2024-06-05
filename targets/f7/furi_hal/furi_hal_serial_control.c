#include "furi_hal_serial_control.h"
#include "furi_hal_serial_types_i.h"
#include "furi_hal_serial.h"

#include <furi.h>
#include <toolbox/api_lock.h>

#define TAG "FuriHalSerialControl"

typedef enum {
    FuriHalSerialControlMessageTypeStop,
    FuriHalSerialControlMessageTypeAcquire,
    FuriHalSerialControlMessageTypeRelease,
    FuriHalSerialControlMessageTypeIsBusy,
    FuriHalSerialControlMessageTypeLogging,
    FuriHalSerialControlMessageTypeExpansionSetCallback,
    FuriHalSerialControlMessageTypeExpansionIrq,
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
    const FuriHalSerialId id;
    const FuriHalSerialControlExpansionCallback callback;
    void* context;
} FuriHalSerialControlMessageExpCallback;

typedef struct {
    FuriHalSerialHandle handles[FuriHalSerialIdMax];
    FuriMessageQueue* queue;
    FuriThread* thread;

    // Logging
    FuriHalSerialId log_config_serial_id;
    uint32_t log_config_serial_baud_rate;
    FuriLogHandler log_handler;
    FuriHalSerialHandle* log_serial;

    // Expansion detection
    FuriHalSerialHandle* expansion_serial;
    FuriHalSerialControlExpansionCallback expansion_cb;
    void* expansion_ctx;
} FuriHalSerialControl;

FuriHalSerialControl* furi_hal_serial_control = NULL;

static void furi_hal_serial_control_log_callback(const uint8_t* data, size_t size, void* context) {
    FuriHalSerialHandle* handle = context;
    furi_hal_serial_tx(handle, data, size);
}

static void furi_hal_serial_control_expansion_irq_callback(void* context) {
    UNUSED(context);

    FuriHalSerialControlMessage message;
    message.type = FuriHalSerialControlMessageTypeExpansionIrq;
    message.api_lock = NULL;
    furi_message_queue_put(furi_hal_serial_control->queue, &message, 0);
}

static void
    furi_hal_serial_control_enable_expansion_irq(FuriHalSerialHandle* handle, bool enable) {
    const GpioPin* gpio = furi_hal_serial_get_gpio_pin(handle, FuriHalSerialDirectionRx);

    if(enable) {
        furi_hal_serial_disable_direction(handle, FuriHalSerialDirectionRx);
        furi_hal_gpio_add_int_callback(gpio, furi_hal_serial_control_expansion_irq_callback, NULL);
        furi_hal_gpio_init(gpio, GpioModeInterruptFall, GpioPullUp, GpioSpeedLow);
    } else {
        furi_hal_gpio_remove_int_callback(gpio);
        furi_hal_serial_enable_direction(handle, FuriHalSerialDirectionRx);
    }
}

static void furi_hal_serial_control_log_set_handle(FuriHalSerialHandle* handle) {
    // Disable expansion module detection before reconfiguring UARTs
    if(furi_hal_serial_control->expansion_serial) {
        furi_hal_serial_control_enable_expansion_irq(
            furi_hal_serial_control->expansion_serial, false);
    }

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

    // Re-enable expansion module detection (if applicable)
    if(furi_hal_serial_control->expansion_serial) {
        furi_hal_serial_control_enable_expansion_irq(
            furi_hal_serial_control->expansion_serial, true);
    }
}

static bool furi_hal_serial_control_handler_stop(void* input, void* output) {
    UNUSED(input);
    UNUSED(output);
    return false;
}

static bool furi_hal_serial_control_handler_acquire(void* input, void* output) {
    FuriHalSerialId serial_id = *(FuriHalSerialId*)input;
    FuriHalSerialHandle* handle = &furi_hal_serial_control->handles[serial_id];

    if(handle->in_use) {
        *(FuriHalSerialHandle**)output = NULL;
    } else {
        // Logging
        if(furi_hal_serial_control->log_config_serial_id == serial_id) {
            furi_hal_serial_control_log_set_handle(NULL);
            // Expansion
        } else if(furi_hal_serial_control->expansion_serial == handle) {
            furi_hal_serial_control_enable_expansion_irq(handle, false);
        }
        // Return handle
        handle->in_use = true;
        *(FuriHalSerialHandle**)output = handle;
    }

    return true;
}

static bool furi_hal_serial_control_handler_release(void* input, void* output) {
    UNUSED(output);

    FuriHalSerialHandle* handle = *(FuriHalSerialHandle**)input;
    furi_assert(handle->in_use);
    furi_hal_serial_deinit(handle);
    handle->in_use = false;

    if(furi_hal_serial_control->log_config_serial_id == handle->id) {
        // Return back logging
        furi_hal_serial_control_log_set_handle(handle);
    } else if(furi_hal_serial_control->expansion_serial == handle) {
        // Re-enable expansion
        furi_hal_serial_control_enable_expansion_irq(handle, true);
    }

    return true;
}

static bool furi_hal_serial_control_handler_is_busy(void* input, void* output) {
    FuriHalSerialId serial_id = *(FuriHalSerialId*)input;
    *(bool*)output = furi_hal_serial_control->handles[serial_id].in_use;

    return true;
}

static bool furi_hal_serial_control_handler_logging(void* input, void* output) {
    UNUSED(output);

    // Set new configuration
    FuriHalSerialControlMessageInputLogging* message_input = input;
    furi_hal_serial_control->log_config_serial_id = message_input->id;
    furi_hal_serial_control->log_config_serial_baud_rate = message_input->baud_rate;
    // Apply new configuration
    FuriHalSerialHandle* handle = NULL;
    if(furi_hal_serial_control->log_config_serial_id < FuriHalSerialIdMax) {
        if(!furi_hal_serial_control->handles[furi_hal_serial_control->log_config_serial_id].in_use) {
            handle =
                &furi_hal_serial_control->handles[furi_hal_serial_control->log_config_serial_id];
        }
    }

    furi_hal_serial_control_log_set_handle(handle);

    return true;
}

static bool furi_hal_serial_control_handler_expansion_set_callback(void* input, void* output) {
    UNUSED(output);

    FuriHalSerialControlMessageExpCallback* message_input = input;
    FuriHalSerialHandle* handle = &furi_hal_serial_control->handles[message_input->id];

    const bool enable_irq = message_input->callback != NULL;

    if(enable_irq) {
        furi_check(furi_hal_serial_control->expansion_serial == NULL);
        furi_check(furi_hal_serial_control->expansion_cb == NULL);
        furi_hal_serial_control->expansion_serial = handle;
    } else {
        furi_check(furi_hal_serial_control->expansion_serial == handle);
        furi_check(furi_hal_serial_control->expansion_cb != NULL);
        furi_hal_serial_control->expansion_serial = NULL;
    }

    furi_hal_serial_control->expansion_cb = message_input->callback;
    furi_hal_serial_control->expansion_ctx = message_input->context;

    furi_hal_serial_control_enable_expansion_irq(handle, enable_irq);

    return true;
}

static bool furi_hal_serial_control_handler_expansion_irq(void* input, void* output) {
    UNUSED(input);
    UNUSED(output);

    if(furi_hal_serial_control->expansion_cb) {
        void* context = furi_hal_serial_control->expansion_ctx;
        furi_hal_serial_control->expansion_cb(context);
    }

    return true;
}

typedef bool (*FuriHalSerialControlCommandHandler)(void* input, void* output);

static const FuriHalSerialControlCommandHandler furi_hal_serial_control_handlers[] = {
    [FuriHalSerialControlMessageTypeStop] = furi_hal_serial_control_handler_stop,
    [FuriHalSerialControlMessageTypeAcquire] = furi_hal_serial_control_handler_acquire,
    [FuriHalSerialControlMessageTypeRelease] = furi_hal_serial_control_handler_release,
    [FuriHalSerialControlMessageTypeIsBusy] = furi_hal_serial_control_handler_is_busy,
    [FuriHalSerialControlMessageTypeLogging] = furi_hal_serial_control_handler_logging,
    [FuriHalSerialControlMessageTypeExpansionSetCallback] =
        furi_hal_serial_control_handler_expansion_set_callback,
    [FuriHalSerialControlMessageTypeExpansionIrq] = furi_hal_serial_control_handler_expansion_irq,
};

static int32_t furi_hal_serial_control_thread(void* args) {
    UNUSED(args);

    bool should_continue = true;
    while(should_continue || furi_message_queue_get_count(furi_hal_serial_control->queue) > 0) {
        FuriHalSerialControlMessage message = {0};
        FuriStatus status =
            furi_message_queue_get(furi_hal_serial_control->queue, &message, FuriWaitForever);
        furi_check(status == FuriStatusOk);
        furi_check(message.type < COUNT_OF(furi_hal_serial_control_handlers));

        should_continue =
            furi_hal_serial_control_handlers[message.type](message.input, message.output);

        if(message.api_lock != NULL) {
            api_lock_unlock(message.api_lock);
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
    furi_hal_serial_control->thread = furi_thread_alloc_service(
        "SerialControlDriver", 512, furi_hal_serial_control_thread, NULL);
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
    message.api_lock = NULL;
    furi_message_queue_put(furi_hal_serial_control->queue, &message, FuriWaitForever);
    furi_thread_join(furi_hal_serial_control->thread);
    // Release resources
    furi_thread_free(furi_hal_serial_control->thread);
    furi_message_queue_free(furi_hal_serial_control->queue);
    free(furi_hal_serial_control);
}

void furi_hal_serial_control_suspend(void) {
    furi_check(furi_hal_serial_control);

    for(size_t i = 0; i < FuriHalSerialIdMax; i++) {
        furi_hal_serial_tx_wait_complete(&furi_hal_serial_control->handles[i]);
        furi_hal_serial_suspend(&furi_hal_serial_control->handles[i]);
    }
}

void furi_hal_serial_control_resume(void) {
    furi_check(furi_hal_serial_control);

    for(size_t i = 0; i < FuriHalSerialIdMax; i++) {
        furi_hal_serial_resume(&furi_hal_serial_control->handles[i]);
    }
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

bool furi_hal_serial_control_is_busy(FuriHalSerialId serial_id) {
    furi_check(furi_hal_serial_control);

    bool result = false;

    FuriHalSerialControlMessage message;
    message.type = FuriHalSerialControlMessageTypeIsBusy;
    message.api_lock = api_lock_alloc_locked();
    message.input = &serial_id;
    message.output = &result;
    furi_message_queue_put(furi_hal_serial_control->queue, &message, FuriWaitForever);
    api_lock_wait_unlock_and_free(message.api_lock);

    return result;
}

void furi_hal_serial_control_set_logging_config(FuriHalSerialId serial_id, uint32_t baud_rate) {
    furi_check(serial_id <= FuriHalSerialIdMax);
    furi_check(baud_rate >= 9600 && baud_rate <= 4000000);

    // Very special case of updater, where RTC initialized before kernel start
    if(!furi_hal_serial_control) return;

    furi_check(furi_hal_serial_is_baud_rate_supported(
        &furi_hal_serial_control->handles[serial_id], baud_rate));

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

void furi_hal_serial_control_set_expansion_callback(
    FuriHalSerialId serial_id,
    FuriHalSerialControlExpansionCallback callback,
    void* context) {
    furi_check(serial_id <= FuriHalSerialIdMax);
    furi_check(furi_hal_serial_control);

    FuriHalSerialControlMessageExpCallback message_input = {
        .id = serial_id,
        .callback = callback,
        .context = context,
    };
    FuriHalSerialControlMessage message;
    message.type = FuriHalSerialControlMessageTypeExpansionSetCallback;
    message.api_lock = api_lock_alloc_locked();
    message.input = &message_input;
    furi_message_queue_put(furi_hal_serial_control->queue, &message, FuriWaitForever);
    api_lock_wait_unlock_and_free(message.api_lock);
}
