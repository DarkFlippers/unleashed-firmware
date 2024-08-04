#include "expansion.h"

#include <furi_hal_serial_control.h>

#include <furi.h>
#include <storage/storage.h>
#include <toolbox/api_lock.h>

#include "expansion_worker.h"
#include "expansion_settings.h"

#define TAG "ExpansionSrv"

#define EXPANSION_CONTROL_QUEUE_SIZE (8UL)
#define EXPANSION_CONTROL_STACK_SIZE (768UL)

typedef enum {
    ExpansionStateDisabled,
    ExpansionStateEnabled,
    ExpansionStateRunning,
} ExpansionState;

typedef enum {
    ExpansionMessageTypeEnable,
    ExpansionMessageTypeDisable,
    ExpansionMessageTypeSetListenSerial,
    ExpansionMessageTypeReloadSettings,
    ExpansionMessageTypeModuleConnected,
    ExpansionMessageTypeModuleDisconnected,
} ExpansionMessageType;

typedef union {
    FuriHalSerialId serial_id;
} ExpansionMessageData;

typedef struct {
    ExpansionMessageType type;
    ExpansionMessageData data;
    FuriApiLock api_lock;
} ExpansionMessage;

struct Expansion {
    FuriThread* thread;
    FuriMessageQueue* queue;
    FuriHalSerialId serial_id;
    ExpansionWorker* worker;
    ExpansionState state;
};

static const char* const expansion_uart_names[] = {
    "USART",
    "LPUART",
};

// Called from the serial control thread
static void expansion_detect_callback(void* context) {
    furi_assert(context);
    Expansion* instance = context;

    ExpansionMessage message = {
        .type = ExpansionMessageTypeModuleConnected,
        .api_lock = NULL, // Not locking the API here to avoid a deadlock
    };

    // Not waiting for available queue space, discarding message if there is none
    const FuriStatus status = furi_message_queue_put(instance->queue, &message, 0);
    UNUSED(status);
}

static void expansion_worker_callback(void* context) {
    furi_assert(context);
    Expansion* instance = context;

    ExpansionMessage message = {
        .type = ExpansionMessageTypeModuleDisconnected,
        .api_lock = NULL, // Not locking the API here to avoid a deadlock
    };

    const FuriStatus status = furi_message_queue_put(instance->queue, &message, FuriWaitForever);
    furi_check(status == FuriStatusOk);
}

static void
    expansion_control_handler_enable(Expansion* instance, const ExpansionMessageData* data) {
    UNUSED(data);

    if(instance->state != ExpansionStateDisabled) {
        return;
    }

    ExpansionSettings settings;
    expansion_settings_load(&settings);

    if(settings.uart_index < FuriHalSerialIdMax) {
        instance->state = ExpansionStateEnabled;
        instance->serial_id = settings.uart_index;
        furi_hal_serial_control_set_expansion_callback(
            instance->serial_id, expansion_detect_callback, instance);

        FURI_LOG_D(TAG, "Detection enabled on %s", expansion_uart_names[instance->serial_id]);
    }
}

static void
    expansion_control_handler_disable(Expansion* instance, const ExpansionMessageData* data) {
    UNUSED(data);
    if(instance->state == ExpansionStateDisabled) {
        return;
    } else if(instance->state == ExpansionStateRunning) {
        expansion_worker_stop(instance->worker);
        expansion_worker_free(instance->worker);
    } else {
        furi_hal_serial_control_set_expansion_callback(instance->serial_id, NULL, NULL);
    }

    instance->state = ExpansionStateDisabled;

    FURI_LOG_D(TAG, "Detection disabled");
}

static void expansion_control_handler_set_listen_serial(
    Expansion* instance,
    const ExpansionMessageData* data) {
    if(instance->state != ExpansionStateDisabled && instance->serial_id == data->serial_id) {
        return;

    } else if(instance->state == ExpansionStateRunning) {
        expansion_worker_stop(instance->worker);
        expansion_worker_free(instance->worker);

    } else if(instance->state == ExpansionStateEnabled) {
        furi_hal_serial_control_set_expansion_callback(instance->serial_id, NULL, NULL);
    }

    instance->state = ExpansionStateEnabled;
    instance->serial_id = data->serial_id;

    furi_hal_serial_control_set_expansion_callback(
        instance->serial_id, expansion_detect_callback, instance);

    FURI_LOG_D(TAG, "Listen serial changed to %s", expansion_uart_names[instance->serial_id]);
}

static void expansion_control_handler_reload_settings(
    Expansion* instance,
    const ExpansionMessageData* data) {
    UNUSED(data);

    ExpansionSettings settings;
    expansion_settings_load(&settings);

    if(settings.uart_index < FuriHalSerialIdMax) {
        const ExpansionMessageData data = {
            .serial_id = settings.uart_index,
        };

        expansion_control_handler_set_listen_serial(instance, &data);

    } else {
        expansion_control_handler_disable(instance, NULL);
    }
}

static void expansion_control_handler_module_connected(
    Expansion* instance,
    const ExpansionMessageData* data) {
    UNUSED(data);
    if(instance->state != ExpansionStateEnabled) {
        return;
    }

    furi_hal_serial_control_set_expansion_callback(instance->serial_id, NULL, NULL);

    instance->state = ExpansionStateRunning;
    instance->worker = expansion_worker_alloc(instance->serial_id);

    expansion_worker_set_callback(instance->worker, expansion_worker_callback, instance);
    expansion_worker_start(instance->worker);
}

static void expansion_control_handler_module_disconnected(
    Expansion* instance,
    const ExpansionMessageData* data) {
    UNUSED(data);
    if(instance->state != ExpansionStateRunning) {
        return;
    }

    instance->state = ExpansionStateEnabled;
    expansion_worker_free(instance->worker);
    furi_hal_serial_control_set_expansion_callback(
        instance->serial_id, expansion_detect_callback, instance);
}

typedef void (*ExpansionControlHandler)(Expansion*, const ExpansionMessageData*);

static const ExpansionControlHandler expansion_control_handlers[] = {
    [ExpansionMessageTypeEnable] = expansion_control_handler_enable,
    [ExpansionMessageTypeDisable] = expansion_control_handler_disable,
    [ExpansionMessageTypeSetListenSerial] = expansion_control_handler_set_listen_serial,
    [ExpansionMessageTypeReloadSettings] = expansion_control_handler_reload_settings,
    [ExpansionMessageTypeModuleConnected] = expansion_control_handler_module_connected,
    [ExpansionMessageTypeModuleDisconnected] = expansion_control_handler_module_disconnected,
};

static int32_t expansion_control(void* context) {
    furi_assert(context);
    Expansion* instance = context;

    for(;;) {
        ExpansionMessage message;

        FuriStatus status = furi_message_queue_get(instance->queue, &message, FuriWaitForever);
        furi_check(status == FuriStatusOk);

        furi_check(message.type < COUNT_OF(expansion_control_handlers));
        expansion_control_handlers[message.type](instance, &message.data);

        if(message.api_lock != NULL) {
            api_lock_unlock(message.api_lock);
        }
    }

    return 0;
}

static Expansion* expansion_alloc(void) {
    Expansion* instance = malloc(sizeof(Expansion));

    instance->queue =
        furi_message_queue_alloc(EXPANSION_CONTROL_QUEUE_SIZE, sizeof(ExpansionMessage));
    instance->thread =
        furi_thread_alloc_ex(TAG, EXPANSION_CONTROL_STACK_SIZE, expansion_control, instance);

    return instance;
}

static void expansion_storage_callback(const void* message, void* context) {
    furi_assert(context);

    const StorageEvent* event = message;
    Expansion* instance = context;

    if(event->type == StorageEventTypeCardMount) {
        ExpansionMessage em = {
            .type = ExpansionMessageTypeReloadSettings,
            .api_lock = NULL,
        };

        furi_check(furi_message_queue_put(instance->queue, &em, FuriWaitForever) == FuriStatusOk);
    }
}

void expansion_on_system_start(void* arg) {
    UNUSED(arg);

    Expansion* instance = expansion_alloc();
    furi_record_create(RECORD_EXPANSION, instance);
    furi_thread_start(instance->thread);

    Storage* storage = furi_record_open(RECORD_STORAGE);
    furi_pubsub_subscribe(storage_get_pubsub(storage), expansion_storage_callback, instance);

    if(storage_sd_status(storage) != FSE_OK) {
        FURI_LOG_D(TAG, "SD Card not ready, skipping settings");
        return;
    }

    expansion_enable(instance);
}

// Public API functions

void expansion_enable(Expansion* instance) {
    furi_check(instance);

    ExpansionMessage message = {
        .type = ExpansionMessageTypeEnable,
        .api_lock = api_lock_alloc_locked(),
    };

    furi_message_queue_put(instance->queue, &message, FuriWaitForever);
    api_lock_wait_unlock_and_free(message.api_lock);
}

void expansion_disable(Expansion* instance) {
    furi_check(instance);

    ExpansionMessage message = {
        .type = ExpansionMessageTypeDisable,
        .api_lock = api_lock_alloc_locked(),
    };

    furi_message_queue_put(instance->queue, &message, FuriWaitForever);
    api_lock_wait_unlock_and_free(message.api_lock);
}

void expansion_set_listen_serial(Expansion* instance, FuriHalSerialId serial_id) {
    furi_check(instance);
    furi_check(serial_id < FuriHalSerialIdMax);

    ExpansionMessage message = {
        .type = ExpansionMessageTypeSetListenSerial,
        .data.serial_id = serial_id,
        .api_lock = api_lock_alloc_locked(),
    };

    furi_message_queue_put(instance->queue, &message, FuriWaitForever);
    api_lock_wait_unlock_and_free(message.api_lock);
}
