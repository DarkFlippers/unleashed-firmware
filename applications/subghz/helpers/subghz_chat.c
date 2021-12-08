#include "subghz_chat.h"
#include <lib/subghz/subghz_tx_rx_worker.h>

#define TAG "SubGhzChat"
#define SUBGHZ_CHAT_WORKER_TIMEOUT_BETWEEN_MESSAGES 500

struct SubGhzChatWorker {
    FuriThread* thread;
    SubGhzTxRxWorker* subghz_txrx;

    volatile bool worker_running;
    volatile bool worker_stoping;
    osMessageQueueId_t event_queue;
    uint32_t last_time_rx_data;
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
    SubghzChatEvent event;
    event.event = SubghzChatEventUserEntrance;
    osMessageQueuePut(instance->event_queue, &event, 0, 0);
    while(instance->worker_running) {
        if(furi_hal_vcp_rx_with_timeout((uint8_t*)&c, 1, osWaitForever) == 1) {
            event.event = SubghzChatEventInputData;
            event.c = c;
            osMessageQueuePut(instance->event_queue, &event, 0, osWaitForever);
        }
    }

    FURI_LOG_I(TAG, "Worker stop");
    return 0;
}

static void subghz_chat_worker_update_rx_event_chat(void* context) {
    furi_assert(context);
    SubGhzChatWorker* instance = context;
    SubghzChatEvent event;
    if((millis() - instance->last_time_rx_data) > SUBGHZ_CHAT_WORKER_TIMEOUT_BETWEEN_MESSAGES) {
        event.event = SubghzChatEventNewMessage;
        osMessageQueuePut(instance->event_queue, &event, 0, osWaitForever);
    }
    instance->last_time_rx_data = millis();
    event.event = SubghzChatEventRXData;
    osMessageQueuePut(instance->event_queue, &event, 0, osWaitForever);
}

SubGhzChatWorker* subghz_chat_worker_alloc() {
    SubGhzChatWorker* instance = furi_alloc(sizeof(SubGhzChatWorker));

    instance->thread = furi_thread_alloc();
    furi_thread_set_name(instance->thread, "SubghzChat");
    furi_thread_set_stack_size(instance->thread, 2048);
    furi_thread_set_context(instance->thread, instance);
    furi_thread_set_callback(instance->thread, subghz_chat_worker_thread);
    instance->subghz_txrx = subghz_tx_rx_worker_alloc();
    instance->event_queue = osMessageQueueNew(80, sizeof(SubghzChatEvent), NULL);
    return instance;
}

void subghz_chat_worker_free(SubGhzChatWorker* instance) {
    furi_assert(instance);
    furi_assert(!instance->worker_running);
    osMessageQueueDelete(instance->event_queue);
    subghz_tx_rx_worker_free(instance->subghz_txrx);
    furi_thread_free(instance->thread);

    free(instance);
}

bool subghz_chat_worker_start(SubGhzChatWorker* instance, uint32_t frequency) {
    furi_assert(instance);
    furi_assert(!instance->worker_running);
    bool res = false;

    if(subghz_tx_rx_worker_start(instance->subghz_txrx, frequency)) {
        osMessageQueueReset(instance->event_queue);
        subghz_tx_rx_worker_set_callback_have_read(
            instance->subghz_txrx, subghz_chat_worker_update_rx_event_chat, instance);

        instance->worker_running = true;
        instance->last_time_rx_data = 0;

        res = furi_thread_start(instance->thread);
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

SubghzChatEvent subghz_chat_worker_get_event_chat(SubGhzChatWorker* instance) {
    furi_assert(instance);
    SubghzChatEvent event;
    if(osMessageQueueGet(instance->event_queue, &event, NULL, osWaitForever) == osOK) {
        return event;
    } else {
        event.event = SubghzChatEventNoEvent;
        return event;
    }
}

void subghz_chat_worker_put_event_chat(SubGhzChatWorker* instance, SubghzChatEvent* event) {
    furi_assert(instance);
    osMessageQueuePut(instance->event_queue, event, 0, osWaitForever);
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