#include "irda_worker.h"
#include <irda.h>
#include <api-hal-irda.h>
#include <limits.h>
#include <stdint.h>
#include <stream_buffer.h>
#include <furi.h>
#include <notification/notification-messages.h>

#define MAX_TIMINGS_AMOUNT                  500
#define IRDA_WORKER_RX_TIMEOUT              150 // ms
#define IRDA_WORKER_RX_RECEIVED             0x01
#define IRDA_WORKER_RX_TIMEOUT_RECEIVED     0x02
#define IRDA_WORKER_OVERRUN                 0x04
#define IRDA_WORKER_EXIT                    0x08

struct IrdaWorkerSignal {
    bool decoded;
    size_t timings_cnt;
    union {
        IrdaMessage message;
        uint32_t timings[MAX_TIMINGS_AMOUNT];
    } data;
};

struct IrdaWorker {
    FuriThread* thread;
    IrdaDecoderHandler* irda_decoder;
    StreamBufferHandle_t stream;

    TaskHandle_t worker_handle;
    IrdaWorkerSignal signal;

    IrdaWorkerReceivedSignalCallback received_signal_callback;
    void* context;
    bool blink_enable;
    bool overrun;
    NotificationApp* notification;
};

static void irda_worker_rx_timeout_callback(void* context) {
    IrdaWorker* instance = context;
    BaseType_t xHigherPriorityTaskWoken = pdFALSE;
    xTaskNotifyFromISR(instance->worker_handle, IRDA_WORKER_RX_TIMEOUT_RECEIVED, eSetBits,  &xHigherPriorityTaskWoken);
    portYIELD_FROM_ISR(xHigherPriorityTaskWoken);
}

static void irda_worker_rx_callback(void* context, bool level, uint32_t duration) {
    IrdaWorker* instance = context;

    BaseType_t xHigherPriorityTaskWoken = pdFALSE;
    LevelDuration level_duration = level_duration_make(level, duration);

    size_t ret =
        xStreamBufferSendFromISR(instance->stream, &level_duration, sizeof(LevelDuration), &xHigherPriorityTaskWoken);
    uint32_t notify_value = (ret == sizeof(LevelDuration)) ? IRDA_WORKER_RX_RECEIVED : IRDA_WORKER_OVERRUN;
    xTaskNotifyFromISR(instance->worker_handle, notify_value, eSetBits,  &xHigherPriorityTaskWoken);
    portYIELD_FROM_ISR(xHigherPriorityTaskWoken);
}

static void irda_worker_process_timeout(IrdaWorker* instance) {
    if (instance->signal.timings_cnt < 2)
        return;

    instance->signal.decoded = false;
    if (instance->received_signal_callback)
        instance->received_signal_callback(instance->context, &instance->signal);
}

static void irda_worker_process_timings(IrdaWorker* instance, uint32_t duration, bool level) {
    const IrdaMessage* message_decoded = irda_decode(instance->irda_decoder, level, duration);
    if (message_decoded) {
        instance->signal.data.message = *message_decoded;
        instance->signal.timings_cnt = 0;
        instance->signal.decoded = true;
        if (instance->received_signal_callback)
            instance->received_signal_callback(instance->context, &instance->signal);
    } else {
        /* Skip first timing if it's starts from Space */
        if ((instance->signal.timings_cnt == 0) && !level) {
            return;
        }

        if (instance->signal.timings_cnt < MAX_TIMINGS_AMOUNT) {
            instance->signal.data.timings[instance->signal.timings_cnt] = duration;
            ++instance->signal.timings_cnt;
        } else {
            xTaskNotify(instance->worker_handle, IRDA_WORKER_OVERRUN, eSetBits);
            instance->overrun = true;
        }
    }
}

static int32_t irda_worker_thread_callback(void* context) {
    IrdaWorker* instance = context;
    uint32_t notify_value = 0;
    LevelDuration level_duration;
    TickType_t last_blink_time = 0;

    while(1) {
        BaseType_t result;
        result = xTaskNotifyWait(pdFALSE, ULONG_MAX, &notify_value, 1000);
        if (result != pdPASS)
            continue;

        if (notify_value & IRDA_WORKER_RX_RECEIVED) {
            if (!instance->overrun && instance->blink_enable && ((xTaskGetTickCount() - last_blink_time) > 80)) {
                last_blink_time = xTaskGetTickCount();
                notification_message(instance->notification, &sequence_blink_blue_10);
            }
            if (instance->signal.timings_cnt == 0)
                notification_message(instance->notification, &sequence_display_on);
            while (sizeof(LevelDuration) == xStreamBufferReceive(instance->stream, &level_duration, sizeof(LevelDuration), 0)) {
                if (!instance->overrun) {
                    bool level = level_duration_get_level(level_duration);
                    uint32_t duration = level_duration_get_duration(level_duration);
                    irda_worker_process_timings(instance, duration, level);
                }
            }
        }
        if (notify_value & IRDA_WORKER_OVERRUN) {
            printf("#");
            irda_reset_decoder(instance->irda_decoder);
            instance->signal.timings_cnt = 0;
            if (instance->blink_enable)
                notification_message(instance->notification, &sequence_set_red_255);
        }
        if (notify_value & IRDA_WORKER_RX_TIMEOUT_RECEIVED) {
            if (instance->overrun) {
                printf("\nOVERRUN, max samples: %d\n", MAX_TIMINGS_AMOUNT);
                instance->overrun = false;
                if (instance->blink_enable)
                    notification_message(instance->notification, &sequence_reset_red);
            } else {
                irda_worker_process_timeout(instance);
            }
            instance->signal.timings_cnt = 0;
        }
        if (notify_value & IRDA_WORKER_EXIT)
            break;
    }

    return 0;
}

void irda_worker_set_received_signal_callback(IrdaWorker* instance, IrdaWorkerReceivedSignalCallback callback) {
    furi_assert(instance);
    instance->received_signal_callback = callback;
}

IrdaWorker* irda_worker_alloc() {
    IrdaWorker* instance = furi_alloc(sizeof(IrdaWorker));

    instance->thread = furi_thread_alloc();
    furi_thread_set_name(instance->thread, "irda_worker");
    furi_thread_set_stack_size(instance->thread, 2048);
    furi_thread_set_context(instance->thread, instance);
    furi_thread_set_callback(instance->thread, irda_worker_thread_callback);

    instance->stream = xStreamBufferCreate(sizeof(LevelDuration) * 512, sizeof(LevelDuration));

    instance->irda_decoder = irda_alloc_decoder();
    instance->blink_enable = false;
    instance->notification = furi_record_open("notification");

    return instance;
}

void irda_worker_free(IrdaWorker* instance) {
    furi_assert(instance);
    furi_assert(!instance->worker_handle);

    furi_record_close("notification");
    irda_free_decoder(instance->irda_decoder);
    vStreamBufferDelete(instance->stream);
    furi_thread_free(instance->thread);

    free(instance);
}

void irda_worker_set_context(IrdaWorker* instance, void* context) {
    furi_assert(instance);
    instance->context = context;
}

void irda_worker_start(IrdaWorker* instance) {
    furi_assert(instance);
    furi_assert(!instance->worker_handle);

    furi_thread_start(instance->thread);

    instance->worker_handle = furi_thread_get_thread_id(instance->thread);
    api_hal_irda_async_rx_start();
    api_hal_irda_async_rx_set_timeout(IRDA_WORKER_RX_TIMEOUT);
    api_hal_irda_async_rx_set_capture_isr_callback(irda_worker_rx_callback, instance);
    api_hal_irda_async_rx_set_timeout_isr_callback(irda_worker_rx_timeout_callback, instance);
}

void irda_worker_stop(IrdaWorker* instance) {
    furi_assert(instance);
    furi_assert(instance->worker_handle);

    api_hal_irda_async_rx_set_timeout_isr_callback(NULL, NULL);
    api_hal_irda_async_rx_set_capture_isr_callback(NULL, NULL);
    api_hal_irda_async_rx_stop();

    xTaskNotify(instance->worker_handle, IRDA_WORKER_EXIT, eSetBits);

    instance->worker_handle = NULL;

    furi_thread_join(instance->thread);
}

bool irda_worker_signal_is_decoded(const IrdaWorkerSignal* signal) {
    furi_assert(signal);
    return signal->decoded;
}

void irda_worker_get_raw_signal(const IrdaWorkerSignal* signal, const uint32_t** timings, size_t* timings_cnt) {
    furi_assert(signal);
    furi_assert(timings);
    furi_assert(timings_cnt);

    *timings = signal->data.timings;
    *timings_cnt = signal->timings_cnt;
}

const IrdaMessage* irda_worker_get_decoded_message(const IrdaWorkerSignal* signal) {
    furi_assert(signal);
    return &signal->data.message;
}

void irda_worker_enable_blink_on_receiving(IrdaWorker* instance, bool enable) {
    furi_assert(instance);
    instance->blink_enable = enable;
}

