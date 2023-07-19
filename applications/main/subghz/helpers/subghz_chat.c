#include "subghz_chat.h"
#include <lib/subghz/subghz_tx_rx_worker.h>

#define TAG "SubGhzChat"
#define SUBGHZ_CHAT_WORKER_TIMEOUT_BETWEEN_MESSAGES 500

struct SubGhzChatWorker {
    FuriThread* thread;
    SubGhzTxRxWorker* subghz_txrx;

    volatile bool worker_running;
    volatile bool worker_stopping;
    FuriMessageQueue* event_queue;
    uint32_t last_time_rx_data;

    Cli* cli;
};

/** Worker thread
 * 
 * @param context 
 * @return exit code 
 */
static int32_t subghz_chat_worker_thread(void* context) {
    SubGhzChatWorker* instance = context;
    FURI_LOG_I(TAG, "Worker start");
    char c;
    SubGhzChatEvent event;
    event.event = SubGhzChatEventUserEntrance;
    furi_message_queue_put(instance->event_queue, &event, 0);
    while(instance->worker_running) {
        if(cli_read_timeout(instance->cli, (uint8_t*)&c, 1, 1000) == 1) {
            event.event = SubGhzChatEventInputData;
            event.c = c;
            furi_message_queue_put(instance->event_queue, &event, FuriWaitForever);
        }
    }

    FURI_LOG_I(TAG, "Worker stop");
    return 0;
}

static void subghz_chat_worker_update_rx_event_chat(void* context) {
    furi_assert(context);
    SubGhzChatWorker* instance = context;
    SubGhzChatEvent event;
    if((furi_get_tick() - instance->last_time_rx_data) >
       SUBGHZ_CHAT_WORKER_TIMEOUT_BETWEEN_MESSAGES) {
        event.event = SubGhzChatEventNewMessage;
        furi_message_queue_put(instance->event_queue, &event, FuriWaitForever);
    }
    instance->last_time_rx_data = furi_get_tick();
    event.event = SubGhzChatEventRXData;
    furi_message_queue_put(instance->event_queue, &event, FuriWaitForever);
}

SubGhzChatWorker* subghz_chat_worker_alloc(Cli* cli) {
    SubGhzChatWorker* instance = malloc(sizeof(SubGhzChatWorker));

    instance->cli = cli;

    instance->thread =
        furi_thread_alloc_ex("SubGhzChat", 2048, subghz_chat_worker_thread, instance);
    instance->subghz_txrx = subghz_tx_rx_worker_alloc();
    instance->event_queue = furi_message_queue_alloc(80, sizeof(SubGhzChatEvent));
    return instance;
}

void subghz_chat_worker_free(SubGhzChatWorker* instance) {
    furi_assert(instance);
    furi_assert(!instance->worker_running);
    furi_message_queue_free(instance->event_queue);
    subghz_tx_rx_worker_free(instance->subghz_txrx);
    furi_thread_free(instance->thread);

    free(instance);
}

bool subghz_chat_worker_start(
    SubGhzChatWorker* instance,
    const SubGhzDevice* device,
    uint32_t frequency) {
    furi_assert(instance);
    furi_assert(!instance->worker_running);
    bool res = false;

    if(subghz_tx_rx_worker_start(instance->subghz_txrx, device, frequency)) {
        furi_message_queue_reset(instance->event_queue);
        subghz_tx_rx_worker_set_callback_have_read(
            instance->subghz_txrx, subghz_chat_worker_update_rx_event_chat, instance);

        instance->worker_running = true;
        instance->last_time_rx_data = 0;

        furi_thread_start(instance->thread);

        res = true;
    }
    return res;
}

void subghz_chat_worker_stop(SubGhzChatWorker* instance) {
    furi_assert(instance);
    furi_assert(instance->worker_running);
    if(subghz_tx_rx_worker_is_running(instance->subghz_txrx)) {
        subghz_tx_rx_worker_stop(instance->subghz_txrx);
    }

    instance->worker_running = false;

    furi_thread_join(instance->thread);
}

bool subghz_chat_worker_is_running(SubGhzChatWorker* instance) {
    furi_assert(instance);
    return instance->worker_running;
}

SubGhzChatEvent subghz_chat_worker_get_event_chat(SubGhzChatWorker* instance) {
    furi_assert(instance);
    SubGhzChatEvent event;
    if(furi_message_queue_get(instance->event_queue, &event, FuriWaitForever) == FuriStatusOk) {
        return event;
    } else {
        event.event = SubGhzChatEventNoEvent;
        return event;
    }
}

void subghz_chat_worker_put_event_chat(SubGhzChatWorker* instance, SubGhzChatEvent* event) {
    furi_assert(instance);
    furi_message_queue_put(instance->event_queue, event, FuriWaitForever);
}

size_t subghz_chat_worker_available(SubGhzChatWorker* instance) {
    furi_assert(instance);
    return subghz_tx_rx_worker_available(instance->subghz_txrx);
}

size_t subghz_chat_worker_read(SubGhzChatWorker* instance, uint8_t* data, size_t size) {
    furi_assert(instance);
    return subghz_tx_rx_worker_read(instance->subghz_txrx, data, size);
}

bool subghz_chat_worker_write(SubGhzChatWorker* instance, uint8_t* data, size_t size) {
    furi_assert(instance);
    return subghz_tx_rx_worker_write(instance->subghz_txrx, data, size);
}
