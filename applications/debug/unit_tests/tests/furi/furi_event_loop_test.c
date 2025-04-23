#include "../test.h"
#include <furi.h>
#include <furi_hal.h>

#include <FreeRTOS.h>
#include <task.h>

#define TAG "TestFuriEventLoop"

#define MESSAGE_COUNT    (256UL)
#define EVENT_FLAG_COUNT (23UL)
#define PRIMITIVE_COUNT  (4UL)
#define RUN_COUNT        (2UL)

typedef struct {
    FuriEventLoop* event_loop;
    uint32_t message_queue_count;
    uint32_t stream_buffer_count;
    uint32_t event_flag_count;
    uint32_t semaphore_count;
    uint32_t primitives_tested;
} TestFuriEventLoopThread;

typedef struct {
    FuriMessageQueue* message_queue;
    FuriStreamBuffer* stream_buffer;
    FuriEventFlag* event_flag;
    FuriSemaphore* semaphore;

    TestFuriEventLoopThread producer;
    TestFuriEventLoopThread consumer;
} TestFuriEventLoopData;

static void test_furi_event_loop_pending_callback(void* context) {
    furi_check(context);

    TestFuriEventLoopThread* test_thread = context;
    furi_check(test_thread->primitives_tested < PRIMITIVE_COUNT);

    test_thread->primitives_tested++;
    FURI_LOG_I(TAG, "primitives tested: %lu", test_thread->primitives_tested);

    if(test_thread->primitives_tested == PRIMITIVE_COUNT) {
        furi_event_loop_stop(test_thread->event_loop);
    }
}

static void test_furi_event_loop_thread_init(TestFuriEventLoopThread* test_thread) {
    memset(test_thread, 0, sizeof(TestFuriEventLoopThread));
    test_thread->event_loop = furi_event_loop_alloc();
}

static void test_furi_event_loop_thread_run_and_cleanup(TestFuriEventLoopThread* test_thread) {
    furi_event_loop_run(test_thread->event_loop);
    // 2 EventLoop index, 0xFFFFFFFF - all possible flags, emulate uncleared flags
    xTaskNotifyIndexed(xTaskGetCurrentTaskHandle(), 2, 0xFFFFFFFF, eSetBits);
    furi_event_loop_free(test_thread->event_loop);
}

static void test_furi_event_loop_producer_message_queue_callback(
    FuriEventLoopObject* object,
    void* context) {
    furi_check(context);

    TestFuriEventLoopData* data = context;
    furi_check(data->message_queue == object);

    FURI_LOG_I(
        TAG,
        "producer MessageQueue: %lu %lu",
        data->producer.message_queue_count,
        data->consumer.message_queue_count);

    if(data->producer.message_queue_count == MESSAGE_COUNT / 2) {
        furi_event_loop_unsubscribe(data->producer.event_loop, data->message_queue);
        furi_event_loop_subscribe_message_queue(
            data->producer.event_loop,
            data->message_queue,
            FuriEventLoopEventOut,
            test_furi_event_loop_producer_message_queue_callback,
            data);

    } else if(data->producer.message_queue_count == MESSAGE_COUNT) {
        furi_event_loop_unsubscribe(data->producer.event_loop, data->message_queue);
        furi_event_loop_pend_callback(
            data->producer.event_loop, test_furi_event_loop_pending_callback, &data->producer);
        return;
    }

    data->producer.message_queue_count++;

    furi_check(
        furi_message_queue_put(data->message_queue, &data->producer.message_queue_count, 0) ==
        FuriStatusOk);

    furi_delay_us(furi_hal_random_get() % 100);
}

static void test_furi_event_loop_producer_stream_buffer_callback(
    FuriEventLoopObject* object,
    void* context) {
    furi_check(context);

    TestFuriEventLoopData* data = context;
    furi_check(data->stream_buffer == object);

    TestFuriEventLoopThread* producer = &data->producer;
    TestFuriEventLoopThread* consumer = &data->consumer;

    FURI_LOG_I(
        TAG,
        "producer StreamBuffer: %lu %lu",
        producer->stream_buffer_count,
        consumer->stream_buffer_count);

    if(producer->stream_buffer_count == MESSAGE_COUNT / 2) {
        furi_event_loop_unsubscribe(producer->event_loop, data->stream_buffer);
        furi_event_loop_subscribe_stream_buffer(
            producer->event_loop,
            data->stream_buffer,
            FuriEventLoopEventOut,
            test_furi_event_loop_producer_stream_buffer_callback,
            data);

    } else if(producer->stream_buffer_count == MESSAGE_COUNT) {
        furi_event_loop_unsubscribe(producer->event_loop, data->stream_buffer);
        furi_event_loop_pend_callback(
            producer->event_loop, test_furi_event_loop_pending_callback, producer);
        return;
    }

    producer->stream_buffer_count++;

    furi_check(
        furi_stream_buffer_send(
            data->stream_buffer, &producer->stream_buffer_count, sizeof(uint32_t), 0) ==
        sizeof(uint32_t));

    furi_delay_us(furi_hal_random_get() % 100);
}

static void
    test_furi_event_loop_producer_event_flag_callback(FuriEventLoopObject* object, void* context) {
    furi_check(context);

    TestFuriEventLoopData* data = context;
    furi_check(data->event_flag == object);

    const uint32_t producer_flags = (1UL << data->producer.event_flag_count);
    const uint32_t consumer_flags = (1UL << data->consumer.event_flag_count);

    FURI_LOG_I(TAG, "producer EventFlag: 0x%06lX 0x%06lX", producer_flags, consumer_flags);

    furi_check(furi_event_flag_set(data->event_flag, producer_flags) & producer_flags);

    if(data->producer.event_flag_count == EVENT_FLAG_COUNT / 2) {
        furi_event_loop_unsubscribe(data->producer.event_loop, data->event_flag);
        furi_event_loop_subscribe_event_flag(
            data->producer.event_loop,
            data->event_flag,
            FuriEventLoopEventOut,
            test_furi_event_loop_producer_event_flag_callback,
            data);

    } else if(data->producer.event_flag_count == EVENT_FLAG_COUNT) {
        furi_event_loop_unsubscribe(data->producer.event_loop, data->event_flag);
        furi_event_loop_pend_callback(
            data->producer.event_loop, test_furi_event_loop_pending_callback, &data->producer);
        return;
    }

    data->producer.event_flag_count++;

    furi_delay_us(furi_hal_random_get() % 100);
}

static void
    test_furi_event_loop_producer_semaphore_callback(FuriEventLoopObject* object, void* context) {
    furi_check(context);

    TestFuriEventLoopData* data = context;
    furi_check(data->semaphore == object);

    TestFuriEventLoopThread* producer = &data->producer;
    TestFuriEventLoopThread* consumer = &data->consumer;

    FURI_LOG_I(
        TAG, "producer Semaphore: %lu %lu", producer->semaphore_count, consumer->semaphore_count);
    furi_check(furi_semaphore_release(data->semaphore) == FuriStatusOk);

    if(producer->semaphore_count == MESSAGE_COUNT / 2) {
        furi_event_loop_unsubscribe(producer->event_loop, data->semaphore);
        furi_event_loop_subscribe_semaphore(
            producer->event_loop,
            data->semaphore,
            FuriEventLoopEventOut,
            test_furi_event_loop_producer_semaphore_callback,
            data);

    } else if(producer->semaphore_count == MESSAGE_COUNT) {
        furi_event_loop_unsubscribe(producer->event_loop, data->semaphore);
        furi_event_loop_pend_callback(
            producer->event_loop, test_furi_event_loop_pending_callback, producer);
        return;
    }

    data->producer.semaphore_count++;

    furi_delay_us(furi_hal_random_get() % 100);
}

static int32_t test_furi_event_loop_producer(void* p) {
    furi_check(p);

    TestFuriEventLoopData* data = p;
    TestFuriEventLoopThread* producer = &data->producer;

    for(uint32_t i = 0; i < RUN_COUNT; ++i) {
        FURI_LOG_I(TAG, "producer start run %lu", i);

        test_furi_event_loop_thread_init(producer);

        furi_event_loop_subscribe_message_queue(
            producer->event_loop,
            data->message_queue,
            FuriEventLoopEventOut,
            test_furi_event_loop_producer_message_queue_callback,
            data);
        furi_event_loop_subscribe_stream_buffer(
            producer->event_loop,
            data->stream_buffer,
            FuriEventLoopEventOut,
            test_furi_event_loop_producer_stream_buffer_callback,
            data);
        furi_event_loop_subscribe_event_flag(
            producer->event_loop,
            data->event_flag,
            FuriEventLoopEventOut,
            test_furi_event_loop_producer_event_flag_callback,
            data);
        furi_event_loop_subscribe_semaphore(
            producer->event_loop,
            data->semaphore,
            FuriEventLoopEventOut,
            test_furi_event_loop_producer_semaphore_callback,
            data);

        test_furi_event_loop_thread_run_and_cleanup(producer);
    }

    FURI_LOG_I(TAG, "producer end");

    return 0;
}

static void test_furi_event_loop_consumer_message_queue_callback(
    FuriEventLoopObject* object,
    void* context) {
    furi_check(context);

    TestFuriEventLoopData* data = context;
    furi_check(data->message_queue == object);

    furi_delay_us(furi_hal_random_get() % 100);

    furi_check(
        furi_message_queue_get(data->message_queue, &data->consumer.message_queue_count, 0) ==
        FuriStatusOk);

    FURI_LOG_I(
        TAG,
        "consumer MessageQueue: %lu %lu",
        data->producer.message_queue_count,
        data->consumer.message_queue_count);

    if(data->consumer.message_queue_count == MESSAGE_COUNT / 2) {
        furi_event_loop_unsubscribe(data->consumer.event_loop, data->message_queue);
        furi_event_loop_subscribe_message_queue(
            data->consumer.event_loop,
            data->message_queue,
            FuriEventLoopEventIn,
            test_furi_event_loop_consumer_message_queue_callback,
            data);

    } else if(data->consumer.message_queue_count == MESSAGE_COUNT) {
        furi_event_loop_unsubscribe(data->consumer.event_loop, data->message_queue);
        furi_event_loop_pend_callback(
            data->consumer.event_loop, test_furi_event_loop_pending_callback, &data->consumer);
    }
}

static void test_furi_event_loop_consumer_stream_buffer_callback(
    FuriEventLoopObject* object,
    void* context) {
    furi_check(context);

    TestFuriEventLoopData* data = context;
    furi_check(data->stream_buffer == object);

    TestFuriEventLoopThread* producer = &data->producer;
    TestFuriEventLoopThread* consumer = &data->consumer;

    furi_delay_us(furi_hal_random_get() % 100);

    furi_check(
        furi_stream_buffer_receive(
            data->stream_buffer, &consumer->stream_buffer_count, sizeof(uint32_t), 0) ==
        sizeof(uint32_t));

    FURI_LOG_I(
        TAG,
        "consumer StreamBuffer: %lu %lu",
        producer->stream_buffer_count,
        consumer->stream_buffer_count);

    if(consumer->stream_buffer_count == MESSAGE_COUNT / 2) {
        furi_event_loop_unsubscribe(consumer->event_loop, data->stream_buffer);
        furi_event_loop_subscribe_stream_buffer(
            consumer->event_loop,
            data->stream_buffer,
            FuriEventLoopEventIn,
            test_furi_event_loop_consumer_stream_buffer_callback,
            data);

    } else if(consumer->stream_buffer_count == MESSAGE_COUNT) {
        furi_event_loop_unsubscribe(data->consumer.event_loop, data->stream_buffer);
        furi_event_loop_pend_callback(
            consumer->event_loop, test_furi_event_loop_pending_callback, consumer);
    }
}

static void
    test_furi_event_loop_consumer_event_flag_callback(FuriEventLoopObject* object, void* context) {
    furi_check(context);

    TestFuriEventLoopData* data = context;
    furi_check(data->event_flag == object);

    furi_delay_us(furi_hal_random_get() % 100);

    const uint32_t producer_flags = (1UL << data->producer.event_flag_count);
    const uint32_t consumer_flags = (1UL << data->consumer.event_flag_count);

    furi_check(
        furi_event_flag_wait(data->event_flag, consumer_flags, FuriFlagWaitAny, 0) &
        consumer_flags);

    FURI_LOG_I(TAG, "consumer EventFlag: 0x%06lX 0x%06lX", producer_flags, consumer_flags);

    if(data->consumer.event_flag_count == EVENT_FLAG_COUNT / 2) {
        furi_event_loop_unsubscribe(data->consumer.event_loop, data->event_flag);
        furi_event_loop_subscribe_event_flag(
            data->consumer.event_loop,
            data->event_flag,
            FuriEventLoopEventIn,
            test_furi_event_loop_consumer_event_flag_callback,
            data);

    } else if(data->consumer.event_flag_count == EVENT_FLAG_COUNT) {
        furi_event_loop_unsubscribe(data->consumer.event_loop, data->event_flag);
        furi_event_loop_pend_callback(
            data->consumer.event_loop, test_furi_event_loop_pending_callback, &data->consumer);
        return;
    }

    data->consumer.event_flag_count++;
}

static void
    test_furi_event_loop_consumer_semaphore_callback(FuriEventLoopObject* object, void* context) {
    furi_check(context);

    TestFuriEventLoopData* data = context;
    furi_check(data->semaphore == object);

    furi_delay_us(furi_hal_random_get() % 100);

    TestFuriEventLoopThread* producer = &data->producer;
    TestFuriEventLoopThread* consumer = &data->consumer;

    furi_check(furi_semaphore_acquire(data->semaphore, 0) == FuriStatusOk);

    FURI_LOG_I(
        TAG, "consumer Semaphore: %lu %lu", producer->semaphore_count, consumer->semaphore_count);

    if(consumer->semaphore_count == MESSAGE_COUNT / 2) {
        furi_event_loop_unsubscribe(consumer->event_loop, data->semaphore);
        furi_event_loop_subscribe_semaphore(
            consumer->event_loop,
            data->semaphore,
            FuriEventLoopEventIn,
            test_furi_event_loop_consumer_semaphore_callback,
            data);

    } else if(consumer->semaphore_count == MESSAGE_COUNT) {
        furi_event_loop_unsubscribe(consumer->event_loop, data->semaphore);
        furi_event_loop_pend_callback(
            consumer->event_loop, test_furi_event_loop_pending_callback, consumer);
        return;
    }

    data->consumer.semaphore_count++;
}

static int32_t test_furi_event_loop_consumer(void* p) {
    furi_check(p);

    TestFuriEventLoopData* data = p;
    TestFuriEventLoopThread* consumer = &data->consumer;

    for(uint32_t i = 0; i < RUN_COUNT; ++i) {
        FURI_LOG_I(TAG, "consumer start run %lu", i);

        test_furi_event_loop_thread_init(consumer);

        furi_event_loop_subscribe_message_queue(
            consumer->event_loop,
            data->message_queue,
            FuriEventLoopEventIn,
            test_furi_event_loop_consumer_message_queue_callback,
            data);
        furi_event_loop_subscribe_stream_buffer(
            consumer->event_loop,
            data->stream_buffer,
            FuriEventLoopEventIn,
            test_furi_event_loop_consumer_stream_buffer_callback,
            data);
        furi_event_loop_subscribe_event_flag(
            consumer->event_loop,
            data->event_flag,
            FuriEventLoopEventIn,
            test_furi_event_loop_consumer_event_flag_callback,
            data);
        furi_event_loop_subscribe_semaphore(
            consumer->event_loop,
            data->semaphore,
            FuriEventLoopEventIn,
            test_furi_event_loop_consumer_semaphore_callback,
            data);

        test_furi_event_loop_thread_run_and_cleanup(consumer);
    }

    FURI_LOG_I(TAG, "consumer end");

    return 0;
}

typedef struct {
    FuriEventLoop* event_loop;
    FuriSemaphore* semaphore;
    size_t counter;
} SelfUnsubTestTimerContext;

static void test_self_unsub_semaphore_callback(FuriEventLoopObject* object, void* context) {
    furi_event_loop_unsubscribe(context, object); // shouldn't crash here
}

static void test_self_unsub_timer_callback(void* arg) {
    SelfUnsubTestTimerContext* context = arg;

    if(context->counter == 0) {
        furi_semaphore_release(context->semaphore);
    } else if(context->counter == 1) {
        furi_event_loop_stop(context->event_loop);
    }

    context->counter++;
}

void test_furi_event_loop_self_unsubscribe(void) {
    FuriEventLoop* event_loop = furi_event_loop_alloc();

    FuriSemaphore* semaphore = furi_semaphore_alloc(1, 0);
    furi_event_loop_subscribe_semaphore(
        event_loop,
        semaphore,
        FuriEventLoopEventIn,
        test_self_unsub_semaphore_callback,
        event_loop);

    SelfUnsubTestTimerContext timer_context = {
        .event_loop = event_loop,
        .semaphore = semaphore,
        .counter = 0,
    };
    FuriEventLoopTimer* timer = furi_event_loop_timer_alloc(
        event_loop, test_self_unsub_timer_callback, FuriEventLoopTimerTypePeriodic, &timer_context);
    furi_event_loop_timer_start(timer, furi_ms_to_ticks(20));

    furi_event_loop_run(event_loop);

    furi_event_loop_timer_free(timer);
    furi_semaphore_free(semaphore);
    furi_event_loop_free(event_loop);
}

void test_furi_event_loop(void) {
    TestFuriEventLoopData data = {};

    data.message_queue = furi_message_queue_alloc(16, sizeof(uint32_t));
    data.stream_buffer = furi_stream_buffer_alloc(16, sizeof(uint32_t));
    data.event_flag = furi_event_flag_alloc();
    data.semaphore = furi_semaphore_alloc(8, 0);

    FuriThread* producer_thread =
        furi_thread_alloc_ex("producer_thread", 1 * 1024, test_furi_event_loop_producer, &data);
    furi_thread_start(producer_thread);

    FuriThread* consumer_thread =
        furi_thread_alloc_ex("consumer_thread", 1 * 1024, test_furi_event_loop_consumer, &data);
    furi_thread_start(consumer_thread);

    // Wait for thread to complete their tasks
    furi_thread_join(producer_thread);
    furi_thread_join(consumer_thread);

    TestFuriEventLoopThread* producer = &data.producer;
    TestFuriEventLoopThread* consumer = &data.consumer;

    // The test itself
    mu_assert_int_eq(producer->message_queue_count, consumer->message_queue_count);
    mu_assert_int_eq(producer->message_queue_count, MESSAGE_COUNT);
    mu_assert_int_eq(producer->stream_buffer_count, consumer->stream_buffer_count);
    mu_assert_int_eq(producer->stream_buffer_count, MESSAGE_COUNT);
    mu_assert_int_eq(producer->event_flag_count, consumer->event_flag_count);
    mu_assert_int_eq(producer->event_flag_count, EVENT_FLAG_COUNT);
    mu_assert_int_eq(producer->semaphore_count, consumer->semaphore_count);
    mu_assert_int_eq(producer->semaphore_count, MESSAGE_COUNT);

    // Release memory
    furi_thread_free(consumer_thread);
    furi_thread_free(producer_thread);

    furi_message_queue_free(data.message_queue);
    furi_stream_buffer_free(data.stream_buffer);
    furi_event_flag_free(data.event_flag);
    furi_semaphore_free(data.semaphore);
}
