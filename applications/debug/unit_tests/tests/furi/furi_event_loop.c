#include "../test.h"
#include <furi.h>
#include <furi_hal.h>

#define TAG "TestFuriEventLoop"

#define EVENT_LOOP_EVENT_COUNT (256u)

typedef struct {
    FuriMessageQueue* mq;

    FuriEventLoop* producer_event_loop;
    uint32_t producer_counter;

    FuriEventLoop* consumer_event_loop;
    uint32_t consumer_counter;
} TestFuriData;

bool test_furi_event_loop_producer_mq_callback(FuriMessageQueue* queue, void* context) {
    furi_check(context);

    TestFuriData* data = context;
    furi_check(data->mq == queue, "Invalid queue");

    FURI_LOG_I(
        TAG, "producer_mq_callback: %lu %lu", data->producer_counter, data->consumer_counter);

    // Remove and add should not cause crash
    // if(data->producer_counter == EVENT_LOOP_EVENT_COUNT/2) {
    //     furi_event_loop_message_queue_remove(data->producer_event_loop, data->mq);
    //     furi_event_loop_message_queue_add(
    //     data->producer_event_loop,
    //     data->mq,
    //     FuriEventLoopEventOut,
    //     test_furi_event_loop_producer_mq_callback,
    //     data);
    // }

    if(data->producer_counter == EVENT_LOOP_EVENT_COUNT) {
        furi_event_loop_stop(data->producer_event_loop);
        return false;
    }

    data->producer_counter++;
    furi_check(
        furi_message_queue_put(data->mq, &data->producer_counter, 0) == FuriStatusOk,
        "furi_message_queue_put failed");
    furi_delay_us(furi_hal_random_get() % 1000);

    return true;
}

int32_t test_furi_event_loop_producer(void* p) {
    furi_check(p);

    FURI_LOG_I(TAG, "producer start");

    TestFuriData* data = p;

    data->producer_event_loop = furi_event_loop_alloc();
    furi_event_loop_message_queue_subscribe(
        data->producer_event_loop,
        data->mq,
        FuriEventLoopEventOut,
        test_furi_event_loop_producer_mq_callback,
        data);

    furi_event_loop_run(data->producer_event_loop);

    furi_event_loop_message_queue_unsubscribe(data->producer_event_loop, data->mq);
    furi_event_loop_free(data->producer_event_loop);

    FURI_LOG_I(TAG, "producer end");

    return 0;
}

bool test_furi_event_loop_consumer_mq_callback(FuriMessageQueue* queue, void* context) {
    furi_check(context);

    TestFuriData* data = context;
    furi_check(data->mq == queue);

    furi_delay_us(furi_hal_random_get() % 1000);
    furi_check(furi_message_queue_get(data->mq, &data->consumer_counter, 0) == FuriStatusOk);

    FURI_LOG_I(
        TAG, "consumer_mq_callback: %lu %lu", data->producer_counter, data->consumer_counter);

    // Remove and add should not cause crash
    // if(data->producer_counter == EVENT_LOOP_EVENT_COUNT/2) {
    //     furi_event_loop_message_queue_remove(data->consumer_event_loop, data->mq);
    //     furi_event_loop_message_queue_add(
    //     data->consumer_event_loop,
    //     data->mq,
    //     FuriEventLoopEventIn,
    //     test_furi_event_loop_producer_mq_callback,
    //     data);
    // }

    if(data->consumer_counter == EVENT_LOOP_EVENT_COUNT) {
        furi_event_loop_stop(data->consumer_event_loop);
        return false;
    }

    return true;
}

int32_t test_furi_event_loop_consumer(void* p) {
    furi_check(p);

    FURI_LOG_I(TAG, "consumer start");

    TestFuriData* data = p;

    data->consumer_event_loop = furi_event_loop_alloc();
    furi_event_loop_message_queue_subscribe(
        data->consumer_event_loop,
        data->mq,
        FuriEventLoopEventIn,
        test_furi_event_loop_consumer_mq_callback,
        data);

    furi_event_loop_run(data->consumer_event_loop);

    furi_event_loop_message_queue_unsubscribe(data->consumer_event_loop, data->mq);
    furi_event_loop_free(data->consumer_event_loop);

    FURI_LOG_I(TAG, "consumer end");

    return 0;
}

void test_furi_event_loop(void) {
    TestFuriData data = {};

    data.mq = furi_message_queue_alloc(16, sizeof(uint32_t));

    FuriThread* producer_thread = furi_thread_alloc();
    furi_thread_set_name(producer_thread, "producer_thread");
    furi_thread_set_stack_size(producer_thread, 1 * 1024);
    furi_thread_set_callback(producer_thread, test_furi_event_loop_producer);
    furi_thread_set_context(producer_thread, &data);
    furi_thread_start(producer_thread);

    FuriThread* consumer_thread = furi_thread_alloc();
    furi_thread_set_name(consumer_thread, "consumer_thread");
    furi_thread_set_stack_size(consumer_thread, 1 * 1024);
    furi_thread_set_callback(consumer_thread, test_furi_event_loop_consumer);
    furi_thread_set_context(consumer_thread, &data);
    furi_thread_start(consumer_thread);

    // Wait for thread to complete their tasks
    furi_thread_join(producer_thread);
    furi_thread_join(consumer_thread);

    // The test itself
    mu_assert_int_eq(data.producer_counter, data.consumer_counter);

    // Release memory
    furi_thread_free(consumer_thread);
    furi_thread_free(producer_thread);
    furi_message_queue_free(data.mq);
}
