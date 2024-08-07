/**
 * @file example_event_loop_mutex.c
 * @brief Example application that demonstrates the FuriEventLoop and FuriMutex integration.
 *
 * This application simulates a use case where a time-consuming blocking operation is executed
 * in a separate thread and a mutex is being used for synchronization. The application runs 10 iterations
 * of the above mentioned simulated work and prints the results to the debug output each time, then exits.
 */

#include <furi.h>
#include <furi_hal_random.h>

#define TAG "ExampleEventLoopMutex"

#define WORKER_ITERATION_COUNT (10)
// We are interested in IN events (for the mutex, that means that the mutex has been released),
// using edge trigger mode (reacting only to changes in mutex state) and
// employing one-shot mode to automatically unsubscribe before the event is processed.
#define MUTEX_EVENT_AND_FLAGS \
    (FuriEventLoopEventIn | FuriEventLoopEventFlagEdge | FuriEventLoopEventFlagOnce)

typedef struct {
    FuriEventLoop* event_loop;
    FuriThread* worker_thread;
    FuriMutex* worker_mutex;
    uint8_t worker_result;
} EventLoopMutexApp;

// This funciton is being run in a separate thread to simulate lenghty blocking operations
static int32_t event_loop_mutex_app_worker_thread(void* context) {
    furi_assert(context);
    EventLoopMutexApp* app = context;

    FURI_LOG_I(TAG, "Worker thread started");

    // Run 10 iterations of simulated work
    for(uint32_t i = 0; i < WORKER_ITERATION_COUNT; ++i) {
        FURI_LOG_I(TAG, "Doing work ...");
        // Take the mutex so that no-one can access the worker_result variable
        furi_check(furi_mutex_acquire(app->worker_mutex, FuriWaitForever) == FuriStatusOk);
        // Simulate a blocking operation with a random delay between 900 and 1100 ms
        const uint32_t work_time_ms = 900 + furi_hal_random_get() % 200;
        furi_delay_ms(work_time_ms);
        // Simulate a result with a random number between 0 and 255
        app->worker_result = furi_hal_random_get() % 0xFF;

        FURI_LOG_I(TAG, "Work done in %lu ms", work_time_ms);
        // Release the mutex, which will notify the event loop that the result is ready
        furi_check(furi_mutex_release(app->worker_mutex) == FuriStatusOk);
        // Return control to the scheduler so that the event loop can take the mutex in its turn
        furi_thread_yield();
    }

    FURI_LOG_I(TAG, "All work done, worker thread out!");
    // Request the event loop to stop
    furi_event_loop_stop(app->event_loop);

    return 0;
}

// This function is being run each time when the mutex gets released
static bool event_loop_mutex_app_event_callback(FuriEventLoopObject* object, void* context) {
    furi_assert(context);

    EventLoopMutexApp* app = context;
    furi_assert(object == app->worker_mutex);

    // Take the mutex so that no-one can access the worker_result variable
    // IMPORTANT: the wait time MUST be 0, i.e. the event loop event callbacks
    // must NOT ever block. If it is possible that the mutex will be taken by
    // others, then the event callback code must take it into account.
    furi_check(furi_mutex_acquire(app->worker_mutex, 0) == FuriStatusOk);
    // Access the worker_result variable and print it.
    FURI_LOG_I(TAG, "Result available! Value: %u", app->worker_result);
    // Release the mutex, enabling the worker thread to continue when it's ready
    furi_check(furi_mutex_release(app->worker_mutex) == FuriStatusOk);
    // Subscribe for the mutex release events again, since we were unsubscribed automatically
    // before processing the event.
    furi_event_loop_subscribe_mutex(
        app->event_loop,
        app->worker_mutex,
        MUTEX_EVENT_AND_FLAGS,
        event_loop_mutex_app_event_callback,
        app);

    return true;
}

static EventLoopMutexApp* event_loop_mutex_app_alloc(void) {
    EventLoopMutexApp* app = malloc(sizeof(EventLoopMutexApp));

    // Create an event loop instance.
    app->event_loop = furi_event_loop_alloc();
    // Create a worker thread instance.
    app->worker_thread = furi_thread_alloc_ex(
        "EventLoopMutexWorker", 1024, event_loop_mutex_app_worker_thread, app);
    // Create a mutex instance.
    app->worker_mutex = furi_mutex_alloc(FuriMutexTypeNormal);
    // Subscribe for the mutex release events.
    // Note that since FuriEventLoopEventFlagOneShot is used, we will be automatically unsubscribed
    // from events before entering the event processing callback. This is necessary in order to not
    // trigger on events caused by releasing the mutex in the callback.
    furi_event_loop_subscribe_mutex(
        app->event_loop,
        app->worker_mutex,
        MUTEX_EVENT_AND_FLAGS,
        event_loop_mutex_app_event_callback,
        app);

    return app;
}

static void event_loop_mutex_app_free(EventLoopMutexApp* app) {
    // IMPORTANT: The user code MUST unsubscribe from all events before deleting the event loop.
    // Failure to do so will result in a crash.
    furi_event_loop_unsubscribe(app->event_loop, app->worker_mutex);
    // Delete all instances
    furi_thread_free(app->worker_thread);
    furi_mutex_free(app->worker_mutex);
    furi_event_loop_free(app->event_loop);

    free(app);
}

static void event_loop_mutex_app_run(EventLoopMutexApp* app) {
    furi_thread_start(app->worker_thread);
    furi_event_loop_run(app->event_loop);
    furi_thread_join(app->worker_thread);
}

// The application's entry point - referenced in application.fam
int32_t example_event_loop_mutex_app(void* arg) {
    UNUSED(arg);

    EventLoopMutexApp* app = event_loop_mutex_app_alloc();
    event_loop_mutex_app_run(app);
    event_loop_mutex_app_free(app);

    return 0;
}
