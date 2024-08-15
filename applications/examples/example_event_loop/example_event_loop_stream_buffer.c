/**
 * @file example_event_loop_stream_buffer.c
 * @brief Example application that demonstrates the FuriEventLoop and FuriStreamBuffer integration.
 *
 * This application simulates a use case where some data data stream comes from a separate thread (or hardware)
 * and a stream buffer is used to act as an intermediate buffer. The worker thread produces 10 iterations of 32
 * bytes of simulated data, and each time when the buffer is half-filled, the data is taken out of it and printed
 * to the debug output. After completing all iterations, the application exits.
 */

#include <furi.h>
#include <furi_hal_random.h>

#define TAG "ExampleEventLoopStreamBuffer"

#define WORKER_ITERATION_COUNT (10)

#define STREAM_BUFFER_SIZE            (32)
#define STREAM_BUFFER_TRIG_LEVEL      (STREAM_BUFFER_SIZE / 2)
#define STREAM_BUFFER_EVENT_AND_FLAGS (FuriEventLoopEventIn | FuriEventLoopEventFlagEdge)

typedef struct {
    FuriEventLoop* event_loop;
    FuriThread* worker_thread;
    FuriStreamBuffer* stream_buffer;
} EventLoopStreamBufferApp;

// This funciton is being run in a separate thread to simulate data coming from a producer thread or some device.
static int32_t event_loop_stream_buffer_app_worker_thread(void* context) {
    furi_assert(context);
    EventLoopStreamBufferApp* app = context;

    FURI_LOG_I(TAG, "Worker thread started");

    for(uint32_t i = 0; i < WORKER_ITERATION_COUNT; ++i) {
        // Produce 32 bytes of simulated data.
        for(uint32_t j = 0; j < STREAM_BUFFER_SIZE; ++j) {
            // Simulate incoming data by generating a random byte.
            uint8_t data = furi_hal_random_get() % 0xFF;
            // Put the byte in the buffer. Depending on the use case, it may or may be not acceptable
            // to wait for free space to become available.
            furi_check(
                furi_stream_buffer_send(app->stream_buffer, &data, 1, FuriWaitForever) == 1);
            // Delay between 30 and 50 ms to slow down the output for clarity.
            furi_delay_ms(30 + furi_hal_random_get() % 20);
        }
    }

    FURI_LOG_I(TAG, "All work done, worker thread out!");
    // Request the event loop to stop
    furi_event_loop_stop(app->event_loop);

    return 0;
}

// This function is being run each time when the number of bytes in the buffer is above its trigger level.
static bool
    event_loop_stream_buffer_app_event_callback(FuriEventLoopObject* object, void* context) {
    furi_assert(context);
    EventLoopStreamBufferApp* app = context;

    furi_assert(object == app->stream_buffer);

    // Temporary buffer that can hold at most half of the stream buffer's capacity.
    uint8_t data[STREAM_BUFFER_TRIG_LEVEL];
    // Receive the data. It is guaranteed that the amount of data in the buffer will be equal to
    // or greater than the trigger level, therefore, no waiting delay is necessary.
    furi_check(
        furi_stream_buffer_receive(app->stream_buffer, data, sizeof(data), 0) == sizeof(data));

    // Format the data for printing and print it to the debug output.
    FuriString* tmp_str = furi_string_alloc();
    for(uint32_t i = 0; i < sizeof(data); ++i) {
        furi_string_cat_printf(tmp_str, "%02X ", data[i]);
    }

    FURI_LOG_I(TAG, "Received data: %s", furi_string_get_cstr(tmp_str));
    furi_string_free(tmp_str);

    return true;
}

static EventLoopStreamBufferApp* event_loop_stream_buffer_app_alloc(void) {
    EventLoopStreamBufferApp* app = malloc(sizeof(EventLoopStreamBufferApp));

    // Create an event loop instance.
    app->event_loop = furi_event_loop_alloc();
    // Create a worker thread instance.
    app->worker_thread = furi_thread_alloc_ex(
        "EventLoopStreamBufferWorker", 1024, event_loop_stream_buffer_app_worker_thread, app);
    // Create a stream_buffer instance.
    app->stream_buffer = furi_stream_buffer_alloc(STREAM_BUFFER_SIZE, STREAM_BUFFER_TRIG_LEVEL);
    // Subscribe for the stream buffer IN events in edge triggered mode.
    furi_event_loop_subscribe_stream_buffer(
        app->event_loop,
        app->stream_buffer,
        STREAM_BUFFER_EVENT_AND_FLAGS,
        event_loop_stream_buffer_app_event_callback,
        app);

    return app;
}

static void event_loop_stream_buffer_app_free(EventLoopStreamBufferApp* app) {
    // IMPORTANT: The user code MUST unsubscribe from all events before deleting the event loop.
    // Failure to do so will result in a crash.
    furi_event_loop_unsubscribe(app->event_loop, app->stream_buffer);
    // Delete all instances
    furi_thread_free(app->worker_thread);
    furi_stream_buffer_free(app->stream_buffer);
    furi_event_loop_free(app->event_loop);

    free(app);
}

static void event_loop_stream_buffer_app_run(EventLoopStreamBufferApp* app) {
    furi_thread_start(app->worker_thread);
    furi_event_loop_run(app->event_loop);
    furi_thread_join(app->worker_thread);
}

// The application's entry point - referenced in application.fam
int32_t example_event_loop_stream_buffer_app(void* arg) {
    UNUSED(arg);

    EventLoopStreamBufferApp* app = event_loop_stream_buffer_app_alloc();
    event_loop_stream_buffer_app_run(app);
    event_loop_stream_buffer_app_free(app);

    return 0;
}
