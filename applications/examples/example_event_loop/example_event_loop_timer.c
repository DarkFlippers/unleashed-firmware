/**
 * @file example_event_loop_timer.c
 * @brief Example application that demonstrates FuriEventLoop's software timer capability.
 *
 * This application prints a countdown from 10 to 0 to the debug output and then exits.
 * Despite only one timer being used in this example for clarity, an event loop instance can have
 * an arbitrary number of independent timers of any type (periodic or one-shot).
 *
 */
#include <furi.h>

#define TAG "ExampleEventLoopTimer"

#define COUNTDOWN_START_VALUE (10)
#define COUNTDOWN_INTERVAL_MS (1000)

typedef struct {
    FuriEventLoop* event_loop;
    FuriEventLoopTimer* timer;
    uint32_t countdown_value;
} EventLoopTimerApp;

// This function is called each time the timer expires (i.e. once per 1000 ms (1s) in this example)
static void event_loop_timer_callback(void* context) {
    furi_assert(context);
    EventLoopTimerApp* app = context;

    // Print the countdown value
    FURI_LOG_I(TAG, "T-00:00:%02lu", app->countdown_value);

    if(app->countdown_value == 0) {
        // If the countdown reached 0, print the final line and stop the event loop
        FURI_LOG_I(TAG, "Blast off to adventure!");
        // After this call, the control will be returned back to event_loop_timers_app_run()
        furi_event_loop_stop(app->event_loop);

    } else {
        // Decrement the countdown value
        app->countdown_value -= 1;
    }
}

static EventLoopTimerApp* event_loop_timer_app_alloc(void) {
    EventLoopTimerApp* app = malloc(sizeof(EventLoopTimerApp));

    // Create an event loop instance.
    app->event_loop = furi_event_loop_alloc();
    // Create a software timer instance.
    // The timer is bound to the event loop instance and will execute in its context.
    // Here, the timer type is periodic, i.e. it will restart automatically after expiring.
    app->timer = furi_event_loop_timer_alloc(
        app->event_loop, event_loop_timer_callback, FuriEventLoopTimerTypePeriodic, app);
    // The countdown value will be tracked in this variable.
    app->countdown_value = COUNTDOWN_START_VALUE;

    return app;
}

static void event_loop_timer_app_free(EventLoopTimerApp* app) {
    // IMPORTANT: All event loop timers MUST be deleted BEFORE deleting the event loop itself.
    // Failure to do so will result in a crash.
    furi_event_loop_timer_free(app->timer);
    // With all timers deleted, it's safe to delete the event loop.
    furi_event_loop_free(app->event_loop);
    free(app);
}

static void event_loop_timer_app_run(EventLoopTimerApp* app) {
    FURI_LOG_I(TAG, "All systems go! Prepare for countdown!");

    // Timers can be started either before the event loop is run, or in any
    // callback function called by a running event loop.
    furi_event_loop_timer_start(app->timer, COUNTDOWN_INTERVAL_MS);
    // This call will block until furi_event_loop_stop() is called.
    furi_event_loop_run(app->event_loop);
}

// The application's entry point - referenced in application.fam
int32_t example_event_loop_timer_app(void* arg) {
    UNUSED(arg);

    EventLoopTimerApp* app = event_loop_timer_app_alloc();
    event_loop_timer_app_run(app);
    event_loop_timer_app_free(app);

    return 0;
}
