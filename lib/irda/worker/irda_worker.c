#include "furi/check.h"
#include "furi/common_defines.h"
#include "sys/_stdint.h"
#include "irda_worker.h"
#include <irda.h>
#include <furi-hal-irda.h>
#include <limits.h>
#include <stdint.h>
#include <furi.h>
#include <notification/notification-messages.h>
#include <stream_buffer.h>

#define IRDA_WORKER_RX_TIMEOUT              IRDA_RAW_RX_TIMING_DELAY_US

#define IRDA_WORKER_RX_RECEIVED             0x01
#define IRDA_WORKER_RX_TIMEOUT_RECEIVED     0x02
#define IRDA_WORKER_OVERRUN                 0x04
#define IRDA_WORKER_EXIT                    0x08
#define IRDA_WORKER_TX_FILL_BUFFER          0x10
#define IRDA_WORKER_TX_MESSAGE_SENT         0x20

#define IRDA_WORKER_ALL_RX_EVENTS       (IRDA_WORKER_RX_RECEIVED \
                                        | IRDA_WORKER_RX_TIMEOUT_RECEIVED \
                                        | IRDA_WORKER_OVERRUN \
                                        | IRDA_WORKER_EXIT)

#define IRDA_WORKER_ALL_TX_EVENTS       (IRDA_WORKER_TX_FILL_BUFFER \
                                        | IRDA_WORKER_TX_MESSAGE_SENT \
                                        | IRDA_WORKER_EXIT)

#define IRDA_WORKER_ALL_EVENTS          (IRDA_WORKER_ALL_RX_EVENTS | IRDA_WORKER_ALL_TX_EVENTS)

typedef enum {
    IrdaWorkerStateIdle,
    IrdaWorkerStateRunRx,
    IrdaWorkerStateRunTx,
    IrdaWorkerStateWaitTxEnd,
    IrdaWorkerStateStopTx,
    IrdaWorkerStateStartTx,
} IrdaWorkerState;

struct IrdaWorkerSignal{
    bool decoded;
    size_t timings_cnt;
    union {
        IrdaMessage message;
        /* +1 is for pause we add at the beginning */
        uint32_t timings[MAX_TIMINGS_AMOUNT + 1];
    };
};

struct IrdaWorker {
    FuriThread* thread;
    StreamBufferHandle_t stream;
    osEventFlagsId_t events;

    IrdaWorkerSignal signal;
    IrdaWorkerState state;
    IrdaEncoderHandler* irda_encoder;
    IrdaDecoderHandler* irda_decoder;
    NotificationApp* notification;
    bool blink_enable;

    union {
        struct {
            IrdaWorkerGetSignalCallback get_signal_callback;
            IrdaWorkerMessageSentCallback message_sent_callback;
            void* get_signal_context;
            void* message_sent_context;
            uint32_t frequency;
            float duty_cycle;
            uint32_t tx_raw_cnt;
            bool need_reinitialization;
            bool steady_signal_sent;
        } tx;
        struct {
            IrdaWorkerReceivedSignalCallback received_signal_callback;
            void* received_signal_context;
            bool overrun;
        } rx;
    };
};

typedef struct {
    uint32_t duration;
    bool level;
    FuriHalIrdaTxGetDataState state;
} IrdaWorkerTiming;

static int32_t irda_worker_tx_thread(void* context);
static FuriHalIrdaTxGetDataState irda_worker_furi_hal_data_isr_callback(void* context, uint32_t* duration, bool* level);
static void irda_worker_furi_hal_message_sent_isr_callback(void* context);


static void irda_worker_rx_timeout_callback(void* context) {
    IrdaWorker* instance = context;
    uint32_t flags_set = osEventFlagsSet(instance->events, IRDA_WORKER_RX_TIMEOUT_RECEIVED);
    furi_check(flags_set & IRDA_WORKER_RX_TIMEOUT_RECEIVED);
}

static void irda_worker_rx_callback(void* context, bool level, uint32_t duration) {
    IrdaWorker* instance = context;

    BaseType_t xHigherPriorityTaskWoken = pdFALSE;
    furi_assert(duration != 0);
    LevelDuration level_duration = level_duration_make(level, duration);

    size_t ret =
        xStreamBufferSendFromISR(instance->stream, &level_duration, sizeof(LevelDuration), &xHigherPriorityTaskWoken);
    uint32_t events = (ret == sizeof(LevelDuration)) ? IRDA_WORKER_RX_RECEIVED : IRDA_WORKER_OVERRUN;
    portYIELD_FROM_ISR(xHigherPriorityTaskWoken);

    uint32_t flags_set = osEventFlagsSet(instance->events, events);
    furi_check(flags_set & events);
}

static void irda_worker_process_timeout(IrdaWorker* instance) {
    if (instance->signal.timings_cnt < 2)
        return;

    const IrdaMessage* message_decoded = irda_check_decoder_ready(instance->irda_decoder);
    if (message_decoded) {
        instance->signal.message = *message_decoded;
        instance->signal.timings_cnt = 0;
        instance->signal.decoded = true;
    } else {
        instance->signal.decoded = false;
    }
    if (instance->rx.received_signal_callback)
        instance->rx.received_signal_callback(instance->rx.received_signal_context, &instance->signal);
}

static void irda_worker_process_timings(IrdaWorker* instance, uint32_t duration, bool level) {
    const IrdaMessage* message_decoded = irda_decode(instance->irda_decoder, level, duration);
    if (message_decoded) {
        instance->signal.message = *message_decoded;
        instance->signal.timings_cnt = 0;
        instance->signal.decoded = true;
        if (instance->rx.received_signal_callback)
            instance->rx.received_signal_callback(instance->rx.received_signal_context, &instance->signal);
    } else {
        /* Skip first timing if it starts from Space */
        if ((instance->signal.timings_cnt == 0) && !level) {
            return;
        }

        if (instance->signal.timings_cnt < MAX_TIMINGS_AMOUNT) {
            instance->signal.timings[instance->signal.timings_cnt] = duration;
            ++instance->signal.timings_cnt;
        } else {
            uint32_t flags_set = osEventFlagsSet(instance->events, IRDA_WORKER_OVERRUN);
            furi_check(flags_set & IRDA_WORKER_OVERRUN);
            instance->rx.overrun = true;
        }
    }
}

static int32_t irda_worker_rx_thread(void* thread_context) {
    IrdaWorker* instance = thread_context;
    uint32_t events = 0;
    LevelDuration level_duration;
    TickType_t last_blink_time = 0;

    while(1) {
        events = osEventFlagsWait(instance->events, IRDA_WORKER_ALL_RX_EVENTS, 0, osWaitForever);
        furi_check(events & IRDA_WORKER_ALL_RX_EVENTS); /* at least one caught */

        if (events & IRDA_WORKER_RX_RECEIVED) {
            if (!instance->rx.overrun && instance->blink_enable && ((xTaskGetTickCount() - last_blink_time) > 80)) {
                last_blink_time = xTaskGetTickCount();
                notification_message(instance->notification, &sequence_blink_blue_10);
            }
            if (instance->signal.timings_cnt == 0)
                notification_message(instance->notification, &sequence_display_on);
            while (sizeof(LevelDuration) == xStreamBufferReceive(instance->stream, &level_duration, sizeof(LevelDuration), 0)) {
                if (!instance->rx.overrun) {
                    bool level = level_duration_get_level(level_duration);
                    uint32_t duration = level_duration_get_duration(level_duration);
                    irda_worker_process_timings(instance, duration, level);
                }
            }
        }
        if (events & IRDA_WORKER_OVERRUN) {
            printf("#");
            irda_reset_decoder(instance->irda_decoder);
            instance->signal.timings_cnt = 0;
            if (instance->blink_enable)
                notification_message(instance->notification, &sequence_set_red_255);
        }
        if (events & IRDA_WORKER_RX_TIMEOUT_RECEIVED) {
            if (instance->rx.overrun) {
                printf("\nOVERRUN, max samples: %d\n", MAX_TIMINGS_AMOUNT);
                instance->rx.overrun = false;
                if (instance->blink_enable)
                    notification_message(instance->notification, &sequence_reset_red);
            } else {
                irda_worker_process_timeout(instance);
            }
            instance->signal.timings_cnt = 0;
        }
        if (events & IRDA_WORKER_EXIT)
            break;
    }

    return 0;
}

void irda_worker_rx_set_received_signal_callback(IrdaWorker* instance, IrdaWorkerReceivedSignalCallback callback, void* context) {
    furi_assert(instance);
    instance->rx.received_signal_callback = callback;
    instance->rx.received_signal_context = context;
}

IrdaWorker* irda_worker_alloc() {
    IrdaWorker* instance = furi_alloc(sizeof(IrdaWorker));

    instance->thread = furi_thread_alloc();
    furi_thread_set_name(instance->thread, "irda_worker");
    furi_thread_set_stack_size(instance->thread, 2048);
    furi_thread_set_context(instance->thread, instance);

    size_t buffer_size = MAX(sizeof(IrdaWorkerTiming) * (MAX_TIMINGS_AMOUNT + 1), sizeof(LevelDuration) * MAX_TIMINGS_AMOUNT);
    instance->stream = xStreamBufferCreate(buffer_size, sizeof(IrdaWorkerTiming));
    instance->irda_decoder = irda_alloc_decoder();
    instance->irda_encoder = irda_alloc_encoder();
    instance->blink_enable = false;
    instance->notification = furi_record_open("notification");
    instance->state = IrdaWorkerStateIdle;
    instance->events = osEventFlagsNew(NULL);

    return instance;
}

void irda_worker_free(IrdaWorker* instance) {
    furi_assert(instance);
    furi_assert(instance->state == IrdaWorkerStateIdle);

    furi_record_close("notification");
    irda_free_decoder(instance->irda_decoder);
    irda_free_encoder(instance->irda_encoder);
    vStreamBufferDelete(instance->stream);
    furi_thread_free(instance->thread);
    osEventFlagsDelete(instance->events);

    free(instance);
}

void irda_worker_rx_start(IrdaWorker* instance) {
    furi_assert(instance);
    furi_assert(instance->state == IrdaWorkerStateIdle);

    xStreamBufferSetTriggerLevel(instance->stream, sizeof(LevelDuration));

    osEventFlagsClear(instance->events, IRDA_WORKER_ALL_EVENTS);
    furi_thread_set_callback(instance->thread, irda_worker_rx_thread);
    furi_thread_start(instance->thread);

    furi_hal_irda_async_rx_set_capture_isr_callback(irda_worker_rx_callback, instance);
    furi_hal_irda_async_rx_set_timeout_isr_callback(irda_worker_rx_timeout_callback, instance);
    furi_hal_irda_async_rx_start();
    furi_hal_irda_async_rx_set_timeout(IRDA_WORKER_RX_TIMEOUT);

    instance->rx.overrun = false;
    instance->state = IrdaWorkerStateRunRx;
}

void irda_worker_rx_stop(IrdaWorker* instance) {
    furi_assert(instance);
    furi_assert(instance->state == IrdaWorkerStateRunRx);

    furi_hal_irda_async_rx_set_timeout_isr_callback(NULL, NULL);
    furi_hal_irda_async_rx_set_capture_isr_callback(NULL, NULL);
    furi_hal_irda_async_rx_stop();

    osEventFlagsSet(instance->events, IRDA_WORKER_EXIT);
    furi_thread_join(instance->thread);

    BaseType_t xReturn = xStreamBufferReset(instance->stream);
    furi_assert(xReturn == pdPASS);
    (void)xReturn;

    instance->state = IrdaWorkerStateIdle;
}

bool irda_worker_signal_is_decoded(const IrdaWorkerSignal* signal) {
    furi_assert(signal);
    return signal->decoded;
}

void irda_worker_get_raw_signal(const IrdaWorkerSignal* signal, const uint32_t** timings, size_t* timings_cnt) {
    furi_assert(signal);
    furi_assert(timings);
    furi_assert(timings_cnt);

    *timings = signal->timings;
    *timings_cnt = signal->timings_cnt;
}

const IrdaMessage* irda_worker_get_decoded_signal(const IrdaWorkerSignal* signal) {
    furi_assert(signal);
    return &signal->message;
}

void irda_worker_rx_enable_blink_on_receiving(IrdaWorker* instance, bool enable) {
    furi_assert(instance);
    instance->blink_enable = enable;
}

void irda_worker_tx_start(IrdaWorker* instance) {
    furi_assert(instance);
    furi_assert(instance->state == IrdaWorkerStateIdle);

    // size have to be greater than api hal irda async tx buffer size
    xStreamBufferSetTriggerLevel(instance->stream, sizeof(IrdaWorkerTiming));

    osEventFlagsClear(instance->events, IRDA_WORKER_ALL_EVENTS);
    furi_thread_set_callback(instance->thread, irda_worker_tx_thread);
    furi_thread_start(instance->thread);

    instance->tx.steady_signal_sent = false;
    instance->tx.need_reinitialization = false;
    furi_hal_irda_async_tx_set_data_isr_callback(irda_worker_furi_hal_data_isr_callback, instance);
    furi_hal_irda_async_tx_set_signal_sent_isr_callback(irda_worker_furi_hal_message_sent_isr_callback, instance);

    instance->state = IrdaWorkerStateStartTx;
}

static void irda_worker_furi_hal_message_sent_isr_callback(void* context) {
    IrdaWorker* instance = context;
    uint32_t flags_set = osEventFlagsSet(instance->events, IRDA_WORKER_TX_MESSAGE_SENT);
    furi_check(flags_set & IRDA_WORKER_TX_MESSAGE_SENT);
}

static FuriHalIrdaTxGetDataState irda_worker_furi_hal_data_isr_callback(void* context, uint32_t* duration, bool* level) {
    furi_assert(context);
    furi_assert(duration);
    furi_assert(level);

    IrdaWorker* instance = context;
    IrdaWorkerTiming timing;
    FuriHalIrdaTxGetDataState state;

    if (sizeof(IrdaWorkerTiming) == xStreamBufferReceiveFromISR(instance->stream, &timing, sizeof(IrdaWorkerTiming), 0)) {
        *level = timing.level;
        *duration = timing.duration;
        state = timing.state;
    } else {
        furi_assert(0);
        *level = 0;
        *duration = 100;
        state = FuriHalIrdaTxGetDataStateDone;
    }

    uint32_t flags_set = osEventFlagsSet(instance->events, IRDA_WORKER_TX_FILL_BUFFER);
    furi_check(flags_set & IRDA_WORKER_TX_FILL_BUFFER);

    return state;
}

static bool irda_get_new_signal(IrdaWorker* instance) {
    bool new_signal_obtained = false;

    IrdaWorkerGetSignalResponse response = instance->tx.get_signal_callback(instance->tx.get_signal_context, instance);
    if (response == IrdaWorkerGetSignalResponseNew) {
        uint32_t new_tx_frequency = 0;
        float new_tx_duty_cycle = 0;
        if (instance->signal.decoded) {
            new_tx_frequency = irda_get_protocol_frequency(instance->signal.message.protocol);
            new_tx_duty_cycle = irda_get_protocol_duty_cycle(instance->signal.message.protocol);
        } else {
            furi_assert(instance->signal.timings_cnt > 1);
            new_tx_frequency = IRDA_COMMON_CARRIER_FREQUENCY;
            new_tx_duty_cycle = IRDA_COMMON_DUTY_CYCLE;
        }

        instance->tx.tx_raw_cnt = 0;
        instance->tx.need_reinitialization = (new_tx_frequency != instance->tx.frequency) || (new_tx_duty_cycle != instance->tx.duty_cycle);
        instance->tx.frequency = new_tx_frequency;
        instance->tx.duty_cycle = new_tx_duty_cycle;
        if (instance->signal.decoded) {
            irda_reset_encoder(instance->irda_encoder, &instance->signal.message);
        }
        new_signal_obtained = true;
    } else if (response == IrdaWorkerGetSignalResponseSame) {
        new_signal_obtained = true;
        /* no need to reinit */
    } else if (response == IrdaWorkerGetSignalResponseStop) {
        new_signal_obtained = false;
    } else {
        furi_assert(0);
    }

    return new_signal_obtained;
}

static bool irda_worker_tx_fill_buffer(IrdaWorker* instance) {
    bool new_data_available = true;
    IrdaWorkerTiming timing;
    IrdaStatus status = IrdaStatusError;

    while(!xStreamBufferIsFull(instance->stream) && !instance->tx.need_reinitialization && new_data_available) {
        if (instance->signal.decoded) {
            status = irda_encode(instance->irda_encoder, &timing.duration, &timing.level);
        } else {
            timing.duration = instance->signal.timings[instance->tx.tx_raw_cnt];
/* raw always starts from Mark, but we fill it with space delay at start */
            timing.level = (instance->tx.tx_raw_cnt % 2);
            ++instance->tx.tx_raw_cnt;
            if (instance->tx.tx_raw_cnt >= instance->signal.timings_cnt) {
                instance->tx.tx_raw_cnt = 0;
                status = IrdaStatusDone;
            } else {
                status = IrdaStatusOk;
            }
        }

        if (status == IrdaStatusError) {
            furi_assert(0);
            new_data_available = false;
            break;
        } else if (status == IrdaStatusOk) {
            timing.state = FuriHalIrdaTxGetDataStateOk;
        } else if (status == IrdaStatusDone) {
            timing.state = FuriHalIrdaTxGetDataStateDone;

            new_data_available = irda_get_new_signal(instance);
            if (instance->tx.need_reinitialization || !new_data_available) {
                timing.state = FuriHalIrdaTxGetDataStateLastDone;
            }
        } else {
            furi_assert(0);
        }
        uint32_t written_size = xStreamBufferSend(instance->stream, &timing, sizeof(IrdaWorkerTiming), 0);
        furi_assert(sizeof(IrdaWorkerTiming) == written_size);
        (void)written_size;
    }

    return new_data_available;
}

static int32_t irda_worker_tx_thread(void* thread_context) {
    IrdaWorker* instance = thread_context;
    furi_assert(instance->state == IrdaWorkerStateStartTx);
    furi_assert(thread_context);

    uint32_t events = 0;
    bool new_data_available = true;
    bool exit = false;

    exit = !irda_get_new_signal(instance);
    furi_assert(!exit);

    while(!exit) {
        switch (instance->state) {
        case IrdaWorkerStateStartTx:
            instance->tx.need_reinitialization = false;
            new_data_available = irda_worker_tx_fill_buffer(instance);
            furi_hal_irda_async_tx_start(instance->tx.frequency, instance->tx.duty_cycle);

            if (!new_data_available) {
                instance->state = IrdaWorkerStateStopTx;
            } else if (instance->tx.need_reinitialization) {
                instance->state = IrdaWorkerStateWaitTxEnd;
            } else {
                instance->state = IrdaWorkerStateRunTx;
            }

            break;
        case IrdaWorkerStateStopTx:
            furi_hal_irda_async_tx_stop();
            exit = true;
            break;
        case IrdaWorkerStateWaitTxEnd:
            furi_hal_irda_async_tx_wait_termination();
            instance->state = IrdaWorkerStateStartTx;

            events = osEventFlagsGet(instance->events);
            if(events & IRDA_WORKER_EXIT) {
                exit = true;
                break;
            }

            break;
        case IrdaWorkerStateRunTx:
            events = osEventFlagsWait(instance->events, IRDA_WORKER_ALL_TX_EVENTS, 0, osWaitForever);
            furi_check(events & IRDA_WORKER_ALL_TX_EVENTS); /* at least one caught */

            if (events & IRDA_WORKER_EXIT) {
                instance->state = IrdaWorkerStateStopTx;
                break;
            }

            if (events & IRDA_WORKER_TX_FILL_BUFFER) {
                irda_worker_tx_fill_buffer(instance);

                if (instance->tx.need_reinitialization) {
                    instance->state = IrdaWorkerStateWaitTxEnd;
                }
            }

            if (events & IRDA_WORKER_TX_MESSAGE_SENT) {
                if (instance->tx.message_sent_callback)
                    instance->tx.message_sent_callback(instance->tx.message_sent_context);
            }
            break;
        default:
            furi_assert(0);
            break;
        }
    }

    return 0;
}

void irda_worker_tx_set_get_signal_callback(IrdaWorker* instance, IrdaWorkerGetSignalCallback callback, void* context) {
    furi_assert(instance);
    instance->tx.get_signal_callback = callback;
    instance->tx.get_signal_context = context;
}

void irda_worker_tx_set_signal_sent_callback(IrdaWorker* instance, IrdaWorkerMessageSentCallback callback, void* context) {
    furi_assert(instance);
    instance->tx.message_sent_callback = callback;
    instance->tx.message_sent_context = context;
}

void irda_worker_tx_stop(IrdaWorker* instance) {
    furi_assert(instance);
    furi_assert(instance->state != IrdaWorkerStateRunRx);

    osEventFlagsSet(instance->events, IRDA_WORKER_EXIT);
    furi_thread_join(instance->thread);
    furi_hal_irda_async_tx_set_data_isr_callback(NULL, NULL);
    furi_hal_irda_async_tx_set_signal_sent_isr_callback(NULL, NULL);

    instance->signal.timings_cnt = 0;
    BaseType_t xReturn = pdFAIL;
    xReturn = xStreamBufferReset(instance->stream);
    furi_assert(xReturn == pdPASS);
    (void)xReturn;
    instance->state = IrdaWorkerStateIdle;
}

void irda_worker_set_decoded_signal(IrdaWorker* instance, const IrdaMessage* message) {
    furi_assert(instance);
    furi_assert(message);

    instance->signal.decoded = true;
    instance->signal.message = *message;
}

void irda_worker_set_raw_signal(IrdaWorker* instance, const uint32_t* timings, size_t timings_cnt) {
    furi_assert(instance);
    furi_assert(timings);
    furi_assert(timings_cnt > 0);
    size_t max_copy_num = COUNT_OF(instance->signal.timings) - 1;
    furi_check(timings_cnt <= max_copy_num);

    instance->signal.timings[0] = IRDA_RAW_TX_TIMING_DELAY_US;
    memcpy(&instance->signal.timings[1], timings, timings_cnt * sizeof(uint32_t));
    instance->signal.decoded = false;
    instance->signal.timings_cnt = timings_cnt + 1;
}

IrdaWorkerGetSignalResponse irda_worker_tx_get_signal_steady_callback(void* context, IrdaWorker* instance) {
    IrdaWorkerGetSignalResponse response = instance->tx.steady_signal_sent ? IrdaWorkerGetSignalResponseSame : IrdaWorkerGetSignalResponseNew;
    instance->tx.steady_signal_sent = true;
    return response;
}

