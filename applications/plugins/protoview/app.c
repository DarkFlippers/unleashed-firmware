/* Copyright (C) 2022-2023 Salvatore Sanfilippo -- All Rights Reserved
 * See the LICENSE file for information about the license. */

#include "app.h"

/* If this define is enabled, ProtoView is going to mess with the
 * otherwise opaque SubGhzWorker structure in order to disable
 * its filter for samples shorter than a given amount (30us at the
 * time I'm writing this comment).
 *
 * This structure must be taken in sync with the one of the firmware. */
#define PROTOVIEW_DISABLE_SUBGHZ_FILTER 0

#ifdef PROTOVIEW_DISABLE_SUBGHZ_FILTER
struct SubGhzWorker {
    FuriThread* thread;
    FuriStreamBuffer* stream;

    volatile bool running;
    volatile bool overrun;

    LevelDuration filter_level_duration;
    bool filter_running;
    uint16_t filter_duration;

    SubGhzWorkerOverrunCallback overrun_callback;
    SubGhzWorkerPairCallback pair_callback;
    void* context;
};
#endif

RawSamplesBuffer *RawSamples, *DetectedSamples;
extern const SubGhzProtocolRegistry protoview_protocol_registry;

/* Draw some text with a border. If the outside color is black and the inside
 * color is white, it just writes the border of the text, but the function can
 * also be used to write a bold variation of the font setting both the
 * colors to black, or alternatively to write a black text with a white
 * border so that it is visible if there are black stuff on the background. */
/* The callback actually just passes the control to the actual active
 * view callback, after setting up basic stuff like cleaning the screen
 * and setting color to black. */
static void render_callback(Canvas* const canvas, void* ctx) {
    ProtoViewApp* app = ctx;

    /* Clear screen. */
    canvas_set_color(canvas, ColorWhite);
    canvas_draw_box(canvas, 0, 0, 127, 63);
    canvas_set_color(canvas, ColorBlack);
    canvas_set_font(canvas, FontPrimary);

    /* Call who is in charge right now. */
    switch(app->current_view) {
    case ViewRawPulses:
        render_view_raw_pulses(canvas, app);
        break;
    case ViewInfo:
        render_view_info(canvas, app);
        break;
    case ViewFrequencySettings:
    case ViewModulationSettings:
        render_view_settings(canvas, app);
        break;
    case ViewDirectSampling:
        render_view_direct_sampling(canvas, app);
        break;
    case ViewLast:
        furi_crash(TAG " ViewLast selected");
        break;
    }
}

/* Here all we do is putting the events into the queue that will be handled
 * in the while() loop of the app entry point function. */
static void input_callback(InputEvent* input_event, void* ctx) {
    ProtoViewApp* app = ctx;
    furi_message_queue_put(app->event_queue, input_event, FuriWaitForever);
}

/* Called to switch view (when left/right is pressed). Handles
 * changing the current view ID and calling the enter/exit view
 * callbacks if needed. */
static void app_switch_view(ProtoViewApp* app, SwitchViewDirection dir) {
    ProtoViewCurrentView old = app->current_view;
    if(dir == AppNextView) {
        app->current_view++;
        if(app->current_view == ViewLast) app->current_view = 0;
    } else if(dir == AppPrevView) {
        if(app->current_view == 0)
            app->current_view = ViewLast - 1;
        else
            app->current_view--;
    }
    ProtoViewCurrentView new = app->current_view;

    /* Call the enter/exit view callbacks if needed. */
    if(old == ViewDirectSampling) view_exit_direct_sampling(app);
    if(new == ViewDirectSampling) view_enter_direct_sampling(app);
    /* The frequency/modulation settings are actually a single view:
     * as long as the user stays between the two modes of this view we
     * don't need to call the exit-view callback. */
    if((old == ViewFrequencySettings && new != ViewModulationSettings) ||
       (old == ViewModulationSettings && new != ViewFrequencySettings))
        view_exit_settings(app);

    /* Set the current subview of the view we just left to zero, that is
     * the main subview of the view. When re re-enter it we want to see
     * the main thing. */
    app->current_subview[old] = 0;
    memset(app->view_privdata, 0, PROTOVIEW_VIEW_PRIVDATA_LEN);
}

/* Allocate the application state and initialize a number of stuff.
 * This is called in the entry point to create the application state. */
ProtoViewApp* protoview_app_alloc() {
    ProtoViewApp* app = malloc(sizeof(ProtoViewApp));

    // Init shared data structures
    RawSamples = raw_samples_alloc();
    DetectedSamples = raw_samples_alloc();

    //init setting
    app->setting = subghz_setting_alloc();
    subghz_setting_load(app->setting, EXT_PATH("subghz/assets/setting_user"));

    // GUI
    app->gui = furi_record_open(RECORD_GUI);
    app->notification = furi_record_open(RECORD_NOTIFICATION);
    app->view_port = view_port_alloc();
    view_port_draw_callback_set(app->view_port, render_callback, app);
    view_port_input_callback_set(app->view_port, input_callback, app);
    gui_add_view_port(app->gui, app->view_port, GuiLayerFullscreen);
    app->event_queue = furi_message_queue_alloc(8, sizeof(InputEvent));
    app->view_dispatcher = NULL;
    app->text_input = NULL;
    app->show_text_input = false;
    app->current_view = ViewRawPulses;
    for(int j = 0; j < ViewLast; j++) app->current_subview[j] = 0;
    app->direct_sampling_enabled = false;
    app->view_privdata = malloc(PROTOVIEW_VIEW_PRIVDATA_LEN);
    memset(app->view_privdata, 0, PROTOVIEW_VIEW_PRIVDATA_LEN);

    // Signal found and visualization defaults
    app->signal_bestlen = 0;
    app->signal_last_scan_idx = 0;
    app->signal_decoded = false;
    app->us_scale = PROTOVIEW_RAW_VIEW_DEFAULT_SCALE;
    app->signal_offset = 0;
    app->msg_info = NULL;

    // Init Worker & Protocol
    app->txrx = malloc(sizeof(ProtoViewTxRx));

    /* Setup rx worker and environment. */
    app->txrx->freq_mod_changed = false;
    app->txrx->debug_timer_sampling = false;
    app->txrx->last_g0_change_time = DWT->CYCCNT;
    app->txrx->last_g0_value = false;
    app->txrx->worker = subghz_worker_alloc();
#ifdef PROTOVIEW_DISABLE_SUBGHZ_FILTER
    app->txrx->worker->filter_running = 0;
#endif
    app->txrx->environment = subghz_environment_alloc();
    subghz_environment_set_protocol_registry(
        app->txrx->environment, (void*)&protoview_protocol_registry);
    app->txrx->receiver = subghz_receiver_alloc_init(app->txrx->environment);
    subghz_receiver_set_filter(app->txrx->receiver, SubGhzProtocolFlag_Decodable);
    subghz_worker_set_overrun_callback(
        app->txrx->worker, (SubGhzWorkerOverrunCallback)subghz_receiver_reset);
    subghz_worker_set_pair_callback(
        app->txrx->worker, (SubGhzWorkerPairCallback)subghz_receiver_decode);
    subghz_worker_set_context(app->txrx->worker, app->txrx->receiver);

    app->frequency = subghz_setting_get_default_frequency(app->setting);
    app->modulation = 0; /* Defaults to ProtoViewModulations[0]. */

    furi_hal_power_suppress_charge_enter();
    app->running = 1;

    return app;
}

/* Free what the application allocated. It is not clear to me if the
 * Flipper OS, once the application exits, will be able to reclaim space
 * even if we forget to free something here. */
void protoview_app_free(ProtoViewApp* app) {
    furi_assert(app);

    // Put CC1101 on sleep, this also restores charging.
    radio_sleep(app);

    // View related.
    view_port_enabled_set(app->view_port, false);
    gui_remove_view_port(app->gui, app->view_port);
    view_port_free(app->view_port);
    furi_record_close(RECORD_GUI);
    furi_record_close(RECORD_NOTIFICATION);
    furi_message_queue_free(app->event_queue);
    app->gui = NULL;

    // Frequency setting.
    subghz_setting_free(app->setting);

    // Worker stuff.
    if(!app->txrx->debug_timer_sampling) {
        subghz_receiver_free(app->txrx->receiver);
        subghz_environment_free(app->txrx->environment);
        subghz_worker_free(app->txrx->worker);
    }
    free(app->txrx);

    // Raw samples buffers.
    raw_samples_free(RawSamples);
    raw_samples_free(DetectedSamples);
    furi_hal_power_suppress_charge_exit();

    free(app);
}

/* Called periodically. Do signal processing here. Data we process here
 * will be later displayed by the render callback. The side effect of this
 * function is to scan for signals and set DetectedSamples. */
static void timer_callback(void* ctx) {
    ProtoViewApp* app = ctx;
    uint32_t delta, lastidx = app->signal_last_scan_idx;

    /* scan_for_signal(), called by this function, deals with a
     * circular buffer. To never miss anything, even if a signal spawns
     * cross-boundaries, it is enough if we scan each time the buffer fills
     * for 50% more compared to the last scan. Thanks to this check we
     * can avoid scanning too many times to just find the same data. */
    if(lastidx < RawSamples->idx) {
        delta = RawSamples->idx - lastidx;
    } else {
        delta = RawSamples->total - lastidx + RawSamples->idx;
    }
    if(delta < RawSamples->total / 2) return;
    app->signal_last_scan_idx = RawSamples->idx;
    scan_for_signal(app);
}

int32_t protoview_app_entry(void* p) {
    UNUSED(p);
    ProtoViewApp* app = protoview_app_alloc();

    /* Create a timer. We do data analysis in the callback. */
    FuriTimer* timer = furi_timer_alloc(timer_callback, FuriTimerTypePeriodic, app);
    furi_timer_start(timer, furi_kernel_get_tick_frequency() / 8);

    /* Start listening to signals immediately. */
    radio_begin(app);
    radio_rx(app);

    /* This is the main event loop: here we get the events that are pushed
     * in the queue by input_callback(), and process them one after the
     * other. The timeout is 100 milliseconds, so if not input is received
     * before such time, we exit the queue_get() function and call
     * view_port_update() in order to refresh our screen content. */
    InputEvent input;
    while(app->running) {
        FuriStatus qstat = furi_message_queue_get(app->event_queue, &input, 100);
        if(qstat == FuriStatusOk) {
            if(DEBUG_MSG)
                FURI_LOG_E(TAG, "Main Loop - Input: type %d key %u", input.type, input.key);

            /* Handle navigation here. Then handle view-specific inputs
             * in the view specific handling function. */
            if(input.type == InputTypeShort && input.key == InputKeyBack) {
                /* Exit the app. */
                app->running = 0;
            } else if(
                input.type == InputTypeShort && input.key == InputKeyRight &&
                get_current_subview(app) == 0) {
                /* Go to the next view. */
                app_switch_view(app, AppNextView);
            } else if(
                input.type == InputTypeShort && input.key == InputKeyLeft &&
                get_current_subview(app) == 0) {
                /* Go to the previous view. */
                app_switch_view(app, AppPrevView);
            } else {
                /* This is where we pass the control to the currently
                 * active view input processing. */
                switch(app->current_view) {
                case ViewRawPulses:
                    process_input_raw_pulses(app, input);
                    break;
                case ViewInfo:
                    process_input_info(app, input);
                    break;
                case ViewFrequencySettings:
                case ViewModulationSettings:
                    process_input_settings(app, input);
                    break;
                case ViewDirectSampling:
                    process_input_direct_sampling(app, input);
                    break;
                case ViewLast:
                    furi_crash(TAG " ViewLast selected");
                    break;
                }
            }
        } else {
            /* Useful to understand if the app is still alive when it
             * does not respond because of bugs. */
            if(DEBUG_MSG) {
                static int c = 0;
                c++;
                if(!(c % 20)) FURI_LOG_E(TAG, "Loop timeout");
            }
        }
        if(app->show_text_input) {
            /* Remove our viewport: we need to use a view dispatcher
             * in order to show the standard Flipper keyboard. */
            gui_remove_view_port(app->gui, app->view_port);

            /* Allocate a view dispatcher, add a text input view to it,
             * and activate it. */
            app->view_dispatcher = view_dispatcher_alloc();
            view_dispatcher_enable_queue(app->view_dispatcher);
            app->text_input = text_input_alloc();
            view_dispatcher_set_event_callback_context(app->view_dispatcher, app);
            view_dispatcher_add_view(
                app->view_dispatcher, 0, text_input_get_view(app->text_input));
            view_dispatcher_switch_to_view(app->view_dispatcher, 0);

            /* Setup the text input view. The different parameters are set
             * in the app structure by the view that wanted to show the
             * input text. The callback, buffer and buffer len must be set.  */
            text_input_set_header_text(app->text_input, "Save signal filename");
            text_input_set_result_callback(
                app->text_input,
                app->text_input_done_callback,
                app,
                app->text_input_buffer,
                app->text_input_buffer_len,
                false);

            /* Run the dispatcher with the keyboard. */
            view_dispatcher_attach_to_gui(
                app->view_dispatcher, app->gui, ViewDispatcherTypeFullscreen);
            view_dispatcher_run(app->view_dispatcher);

            /* Undo all it: remove the view from the dispatcher, free it
             * so that it removes itself from the current gui, finally
             * restore our viewport. */
            view_dispatcher_remove_view(app->view_dispatcher, 0);
            text_input_free(app->text_input);
            view_dispatcher_free(app->view_dispatcher);
            app->view_dispatcher = NULL;
            gui_add_view_port(app->gui, app->view_port, GuiLayerFullscreen);
            app->show_text_input = false;
        } else {
            view_port_update(app->view_port);
        }
    }

    /* App no longer running. Shut down and free. */
    if(app->txrx->txrx_state == TxRxStateRx) {
        FURI_LOG_E(TAG, "Putting CC1101 to sleep before exiting.");
        radio_rx_end(app);
        radio_sleep(app);
    }

    furi_timer_free(timer);
    protoview_app_free(app);
    return 0;
}
