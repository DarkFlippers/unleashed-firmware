/**
 * @file example_event_loop_multi.c
 * @brief Example application that demonstrates multiple primitives used with two FuriEventLoop instances.
 *
 * This application simulates a complex use case of having two concurrent event loops (each one executing in
 * its own thread) using a stream buffer for communication and additional timers and message passing to handle
 * the keypad input. Additionally, it shows how to use thread signals to stop an event loop in another thread.
 * The GUI functionality is there only for the purpose of exclusive access to the input events.
 *
 * The application's functionality consists of the following:
 * - Print keypad key names and types when pressed,
 * - If the Back key is long-pressed, a countdown starts upon completion of which the app exits,
 * - The countdown can be cancelled by long-pressing the Ok button, it also resets the counter,
 * - Blocks of random data are periodically generated in a separate thread,
 * - When ready, the main application thread gets notified and prints the data.
 */

#include <furi.h>
#include <gui/gui.h>
#include <gui/view_port.h>

#include <furi_hal_random.h>

#define TAG "ExampleEventLoopMulti"

#define COUNTDOWN_START_VALUE   (5UL)
#define COUNTDOWN_INTERVAL_MS   (1000UL)
#define WORKER_DATA_INTERVAL_MS (1500UL)

#define INPUT_QUEUE_SIZE   (8)
#define STREAM_BUFFER_SIZE (16)

typedef struct {
    FuriEventLoop* event_loop;
    FuriEventLoopTimer* timer;
    FuriStreamBuffer* stream_buffer;
} EventLoopMultiAppWorker;

typedef struct {
    Gui* gui;
    ViewPort* view_port;
    FuriThread* worker_thread;
    FuriEventLoop* event_loop;
    FuriMessageQueue* input_queue;
    FuriEventLoopTimer* exit_timer;
    FuriStreamBuffer* stream_buffer;
    uint32_t exit_countdown_value;
} EventLoopMultiApp;

/*
 * Worker functions
 */

// This function is executed each time the data is taken out of the stream buffer. It is used to restart the worker timer.
static bool
    event_loop_multi_app_stream_buffer_worker_callback(FuriEventLoopObject* object, void* context) {
    furi_assert(context);
    EventLoopMultiAppWorker* worker = context;

    furi_assert(object == worker->stream_buffer);

    FURI_LOG_I(TAG, "Data was removed from buffer");
    // Restart the timer to generate another block of random data.
    furi_event_loop_timer_start(worker->timer, WORKER_DATA_INTERVAL_MS);

    return true;
}

// This function is executed when the worker timer expires. The timer will NOT restart automatically
// since it is of one-shot type.
static void event_loop_multi_app_worker_timer_callback(void* context) {
    furi_assert(context);
    EventLoopMultiAppWorker* worker = context;

    // Generate a block of random data.
    uint8_t data[STREAM_BUFFER_SIZE];
    furi_hal_random_fill_buf(data, sizeof(data));
    // Put the generated data in the stream buffer.
    // IMPORTANT: No waiting in the event handlers!
    furi_check(
        furi_stream_buffer_send(worker->stream_buffer, &data, sizeof(data), 0) == sizeof(data));
}

static EventLoopMultiAppWorker*
    event_loop_multi_app_worker_alloc(FuriStreamBuffer* stream_buffer) {
    EventLoopMultiAppWorker* worker = malloc(sizeof(EventLoopMultiAppWorker));
    // Create the worker event loop.
    worker->event_loop = furi_event_loop_alloc();
    // Create the timer governing the data generation.
    // It is of one-shot type, i.e. it will not restart automatically upon expiration.
    worker->timer = furi_event_loop_timer_alloc(
        worker->event_loop,
        event_loop_multi_app_worker_timer_callback,
        FuriEventLoopTimerTypeOnce,
        worker);

    // Using the same stream buffer as the main thread (it was already created beforehand).
    worker->stream_buffer = stream_buffer;
    // Notify the worker event loop about data being taken out of the stream buffer.
    furi_event_loop_subscribe_stream_buffer(
        worker->event_loop,
        worker->stream_buffer,
        FuriEventLoopEventOut | FuriEventLoopEventFlagEdge,
        event_loop_multi_app_stream_buffer_worker_callback,
        worker);

    return worker;
}

static void event_loop_multi_app_worker_free(EventLoopMultiAppWorker* worker) {
    // IMPORTANT: The user code MUST unsubscribe from all events before deleting the event loop.
    // Failure to do so will result in a crash.
    furi_event_loop_unsubscribe(worker->event_loop, worker->stream_buffer);
    // IMPORTANT: All timers MUST be deleted before deleting the associated event loop.
    // Failure to do so will result in a crash.
    furi_event_loop_timer_free(worker->timer);
    // Now it is okay to delete the event loop.
    furi_event_loop_free(worker->event_loop);

    free(worker);
}

static void event_loop_multi_app_worker_run(EventLoopMultiAppWorker* worker) {
    furi_event_loop_timer_start(worker->timer, WORKER_DATA_INTERVAL_MS);
    furi_event_loop_run(worker->event_loop);
}

// This function is the worker thread body and (obviously) is executed in the worker thread.
static int32_t event_loop_multi_app_worker_thread(void* context) {
    furi_assert(context);
    EventLoopMultiApp* app = context;

    // Because an event loop is used, it MUST be created in the thread it will be run in.
    // Therefore, the worker creation and deletion is handled in the worker thread.
    EventLoopMultiAppWorker* worker = event_loop_multi_app_worker_alloc(app->stream_buffer);
    event_loop_multi_app_worker_run(worker);
    event_loop_multi_app_worker_free(worker);

    return 0;
}

/*
 * Main application functions
 */

// This function is executed in the GUI context each time an input event occurs (e.g. the user pressed a key)
static void event_loop_multi_app_input_callback(InputEvent* event, void* context) {
    furi_assert(context);
    EventLoopMultiApp* app = context;
    // Pass the event to the the application's input queue
    furi_check(furi_message_queue_put(app->input_queue, event, FuriWaitForever) == FuriStatusOk);
}

// This function is executed each time new data is available in the stream buffer.
static bool
    event_loop_multi_app_stream_buffer_callback(FuriEventLoopObject* object, void* context) {
    furi_assert(context);
    EventLoopMultiApp* app = context;

    furi_assert(object == app->stream_buffer);
    // Get the data from the stream buffer
    uint8_t data[STREAM_BUFFER_SIZE];
    // IMPORTANT: No waiting in the event handlers!
    furi_check(
        furi_stream_buffer_receive(app->stream_buffer, &data, sizeof(data), 0) == sizeof(data));

    // Format the data for printing and print it to the debug output.
    FuriString* tmp_str = furi_string_alloc();
    for(uint32_t i = 0; i < sizeof(data); ++i) {
        furi_string_cat_printf(tmp_str, "%02X ", data[i]);
    }

    FURI_LOG_I(TAG, "Received data: %s", furi_string_get_cstr(tmp_str));
    furi_string_free(tmp_str);

    return true;
}

// This function is executed each time a new message is inserted in the input queue.
static bool event_loop_multi_app_input_queue_callback(FuriEventLoopObject* object, void* context) {
    furi_assert(context);
    EventLoopMultiApp* app = context;

    furi_assert(object == app->input_queue);

    InputEvent event;
    // IMPORTANT: No waiting in the event handlers!
    furi_check(furi_message_queue_get(app->input_queue, &event, 0) == FuriStatusOk);

    if(event.type == InputTypeLong) {
        // The user has long-pressed the Back key, try starting the countdown.
        if(event.key == InputKeyBack) {
            if(!furi_event_loop_timer_is_running(app->exit_timer)) {
                // Actually start the countdown
                FURI_LOG_I(TAG, "Starting exit countdown!");
                furi_event_loop_timer_start(app->exit_timer, COUNTDOWN_INTERVAL_MS);

            } else {
                // The countdown is already in progress, print a warning message
                FURI_LOG_W(TAG, "Countdown has already been started");
            }

            // The user has long-pressed the Ok key, try stopping the countdown.
        } else if(event.key == InputKeyOk) {
            if(furi_event_loop_timer_is_running(app->exit_timer)) {
                // Actually cancel the countdown
                FURI_LOG_I(TAG, "Exit countdown cancelled!");
                app->exit_countdown_value = COUNTDOWN_START_VALUE;
                furi_event_loop_timer_stop(app->exit_timer);

            } else {
                // The countdown is not running, print a warning message
                FURI_LOG_W(TAG, "Countdown has not been started yet");
            }

        } else {
            // Not a Back or Ok key, just print its name.
            FURI_LOG_I(TAG, "Long press: %s", input_get_key_name(event.key));
        }

    } else if(event.type == InputTypeShort) {
        // Not a long press, just print the key's name.
        FURI_LOG_I(TAG, "Short press: %s", input_get_key_name(event.key));
    }

    return true;
}

// This function is executed each time the countdown timer expires.
static void event_loop_multi_app_exit_timer_callback(void* context) {
    furi_assert(context);
    EventLoopMultiApp* app = context;

    FURI_LOG_I(TAG, "Exiting in %lu ...", app->exit_countdown_value);

    // If the coundown value has reached 0, exit the application
    if(app->exit_countdown_value == 0) {
        FURI_LOG_I(TAG, "Exiting NOW!");

        // Send a signal to the worker thread to exit.
        // A signal handler that handles FuriSignalExit is already set by default.
        furi_thread_signal(app->worker_thread, FuriSignalExit, NULL);
        // Request the application event loop to stop.
        furi_event_loop_stop(app->event_loop);

        // Otherwise just decrement it and wait for the next time the timer expires.
    } else {
        app->exit_countdown_value -= 1;
    }
}

static EventLoopMultiApp* event_loop_multi_app_alloc(void) {
    EventLoopMultiApp* app = malloc(sizeof(EventLoopMultiApp));
    // Create event loop instances.
    app->event_loop = furi_event_loop_alloc();

    // Create a worker thread instance. The worker event loop will execute inside it.
    app->worker_thread = furi_thread_alloc_ex(
        "EventLoopMultiWorker", 1024, event_loop_multi_app_worker_thread, app);
    // Create a message queue to receive the input events.
    app->input_queue = furi_message_queue_alloc(INPUT_QUEUE_SIZE, sizeof(InputEvent));
    // Create a stream buffer to receive the generated data.
    app->stream_buffer = furi_stream_buffer_alloc(STREAM_BUFFER_SIZE, STREAM_BUFFER_SIZE);
    // Create a timer to run the countdown.
    app->exit_timer = furi_event_loop_timer_alloc(
        app->event_loop,
        event_loop_multi_app_exit_timer_callback,
        FuriEventLoopTimerTypePeriodic,
        app);

    app->gui = furi_record_open(RECORD_GUI);
    app->view_port = view_port_alloc();
    // Start the countdown from this value
    app->exit_countdown_value = COUNTDOWN_START_VALUE;
    // Gain exclusive access to the input events
    view_port_input_callback_set(app->view_port, event_loop_multi_app_input_callback, app);
    gui_add_view_port(app->gui, app->view_port, GuiLayerFullscreen);
    // Notify the event loop about incoming messages in the queue
    furi_event_loop_subscribe_message_queue(
        app->event_loop,
        app->input_queue,
        FuriEventLoopEventIn,
        event_loop_multi_app_input_queue_callback,
        app);
    // Notify the event loop about new data in the stream buffer
    furi_event_loop_subscribe_stream_buffer(
        app->event_loop,
        app->stream_buffer,
        FuriEventLoopEventIn | FuriEventLoopEventFlagEdge,
        event_loop_multi_app_stream_buffer_callback,
        app);

    return app;
}

static void event_loop_multi_app_free(EventLoopMultiApp* app) {
    gui_remove_view_port(app->gui, app->view_port);
    furi_record_close(RECORD_GUI);
    // IMPORTANT: The user code MUST unsubscribe from all events before deleting the event loop.
    // Failure to do so will result in a crash.
    furi_event_loop_unsubscribe(app->event_loop, app->input_queue);
    furi_event_loop_unsubscribe(app->event_loop, app->stream_buffer);
    // Delete all instances
    view_port_free(app->view_port);
    furi_message_queue_free(app->input_queue);
    furi_stream_buffer_free(app->stream_buffer);
    // IMPORTANT: All timers MUST be deleted before deleting the associated event loop.
    // Failure to do so will result in a crash.
    furi_event_loop_timer_free(app->exit_timer);
    furi_thread_free(app->worker_thread);
    furi_event_loop_free(app->event_loop);

    free(app);
}

static void event_loop_multi_app_run(EventLoopMultiApp* app) {
    FURI_LOG_I(TAG, "Press keys to see them printed here.");
    FURI_LOG_I(TAG, "Long press \"Back\" to exit after %lu seconds.", COUNTDOWN_START_VALUE);
    FURI_LOG_I(TAG, "Long press \"Ok\" to cancel the countdown.");

    // Start the worker thread
    furi_thread_start(app->worker_thread);
    // Run the application event loop. This call will block until the application is about to exit.
    furi_event_loop_run(app->event_loop);
    // Wait for the worker thread to finish.
    furi_thread_join(app->worker_thread);
}

/*******************************************************************
 *                     vvv START HERE vvv
 *
 * The application's entry point - referenced in application.fam
 *******************************************************************/
int32_t example_event_loop_multi_app(void* arg) {
    UNUSED(arg);

    EventLoopMultiApp* app = event_loop_multi_app_alloc();
    event_loop_multi_app_run(app);
    event_loop_multi_app_free(app);

    return 0;
}
