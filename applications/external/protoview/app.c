/* Copyright (C) 2022-2023 Salvatore Sanfilippo -- All Rights Reserved
 * See the LICENSE file for information about the license. */

#include "app.h"

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
    furi_mutex_acquire(app->view_updating_mutex, FuriWaitForever);

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
    case ViewBuildMessage:
        render_view_build_message(canvas, app);
        break;
    default:
        furi_crash(TAG "Invalid view selected");
        break;
    }

    /* Draw the alert box if set. */
    ui_draw_alert_if_needed(canvas, app);
    furi_mutex_release(app->view_updating_mutex);
}

/* Here all we do is putting the events into the queue that will be handled
 * in the while() loop of the app entry point function. */
static void input_callback(InputEvent* input_event, void* ctx) {
    ProtoViewApp* app = ctx;
    furi_message_queue_put(app->event_queue, input_event, FuriWaitForever);
}

/* Called to switch view (when left/right is pressed). Handles
 * changing the current view ID and calling the enter/exit view
 * callbacks if needed.
 *
 * The 'switchto' parameter can be the identifier of a view, or the
 * special views ViewGoNext and ViewGoPrev in order to move to
 * the logical next/prev view. */
static void app_switch_view(ProtoViewApp* app, ProtoViewCurrentView switchto) {
    furi_mutex_acquire(app->view_updating_mutex, FuriWaitForever);

    /* Switch to the specified view. */
    ProtoViewCurrentView old = app->current_view;
    if(switchto == ViewGoNext) {
        app->current_view++;
        if(app->current_view == ViewLast) app->current_view = 0;
    } else if(switchto == ViewGoPrev) {
        if(app->current_view == 0)
            app->current_view = ViewLast - 1;
        else
            app->current_view--;
    } else {
        app->current_view = switchto;
    }
    ProtoViewCurrentView new = app->current_view;

    /* Call the exit view callbacks. */
    if(old == ViewDirectSampling) view_exit_direct_sampling(app);
    if(old == ViewBuildMessage) view_exit_build_message(app);
    if(old == ViewInfo) view_exit_info(app);
    /* The frequency/modulation settings are actually a single view:
     * as long as the user stays between the two modes of this view we
     * don't need to call the exit-view callback. */
    if((old == ViewFrequencySettings && new != ViewModulationSettings) ||
       (old == ViewModulationSettings && new != ViewFrequencySettings))
        view_exit_settings(app);

    /* Reset the view private data each time, before calling the enter
     * callbacks that may want to setup some state. */
    memset(app->view_privdata, 0, PROTOVIEW_VIEW_PRIVDATA_LEN);

    /* Call the enter view callbacks after all the exit callback
     * of the old view was already executed. */
    if(new == ViewDirectSampling) view_enter_direct_sampling(app);
    if(new == ViewBuildMessage) view_enter_build_message(app);

    /* Set the current subview of the view we just left to zero. This is
     * the main subview of the old view. When we re-enter the view we are
     * lefting, we want to see the main thing again. */
    app->current_subview[old] = 0;

    /* If there is an alert on screen, dismiss it: if the user is
     * switching view she already read it. */
    ui_dismiss_alert(app);
    furi_mutex_release(app->view_updating_mutex);
}

/* Allocate the application state and initialize a number of stuff.
 * This is called in the entry point to create the application state. */
ProtoViewApp* protoview_app_alloc() {
    furi_hal_power_suppress_charge_enter();

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
    app->alert_dismiss_time = 0;
    app->current_view = ViewRawPulses;
    app->view_updating_mutex = furi_mutex_alloc(FuriMutexTypeNormal);
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

    /* Setup rx state. */
    app->txrx->freq_mod_changed = false;
    app->txrx->debug_timer_sampling = false;
    app->txrx->last_g0_change_time = DWT->CYCCNT;
    app->txrx->last_g0_value = false;

    app->frequency = subghz_setting_get_default_frequency(app->setting);
    app->modulation = 0; /* Defaults to ProtoViewModulations[0]. */

    // Init & set radio_device
    subghz_devices_init();
    app->radio_device =
        radio_device_loader_set(app->radio_device, SubGhzRadioDeviceTypeExternalCC1101);

    subghz_devices_reset(app->radio_device);
    subghz_devices_idle(app->radio_device);

    app->running = 1;

    return app;
}

/* Free what the application allocated. It is not clear to me if the
 * Flipper OS, once the application exits, will be able to reclaim space
 * even if we forget to free something here. */
void protoview_app_free(ProtoViewApp* app) {
    furi_assert(app);

    subghz_devices_sleep(app->radio_device);
    radio_device_loader_end(app->radio_device);

    subghz_devices_deinit();

    // View related.
    view_port_enabled_set(app->view_port, false);
    gui_remove_view_port(app->gui, app->view_port);
    view_port_free(app->view_port);
    furi_record_close(RECORD_GUI);
    furi_record_close(RECORD_NOTIFICATION);
    furi_message_queue_free(app->event_queue);
    furi_mutex_free(app->view_updating_mutex);
    app->gui = NULL;

    // Frequency setting.
    subghz_setting_free(app->setting);

    // Worker stuff.
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
    scan_for_signal(app, RawSamples, ProtoViewModulations[app->modulation].duration_filter);
}

/* This is the navigation callback we use in the view dispatcher used
 * to display the "text input" widget, that is the keyboard to get text.
 * The text input view is implemented to ignore the "back" short press,
 * so the event is not consumed and is handled by the view dispatcher.
 * However the view dispatcher implementation has the strange behavior that
 * if no navigation callback is set, it will not stop when handling back.
 *
 * We just need a dummy callback returning false. We believe the
 * implementation should be changed and if no callback is set, it should be
 * the same as returning false. */
static bool keyboard_view_dispatcher_navigation_callback(void* ctx) {
    UNUSED(ctx);
    return false;
}

/* App entry point, as specified in application.fam. */
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
                if(app->current_view != ViewRawPulses) {
                    /* If this is not the main app view, go there. */
                    app_switch_view(app, ViewRawPulses);
                } else {
                    /* If we are in the main app view, warn the user
                     * they needs to long press to really quit. */
                    ui_show_alert(app, "Long press to exit", 1000);
                }
            } else if(input.type == InputTypeLong && input.key == InputKeyBack) {
                app->running = 0;
            } else if(
                input.type == InputTypeShort && input.key == InputKeyRight &&
                ui_get_current_subview(app) == 0) {
                /* Go to the next view. */
                app_switch_view(app, ViewGoNext);
            } else if(
                input.type == InputTypeShort && input.key == InputKeyLeft &&
                ui_get_current_subview(app) == 0) {
                /* Go to the previous view. */
                app_switch_view(app, ViewGoPrev);
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
                case ViewBuildMessage:
                    process_input_build_message(app, input);
                    break;
                default:
                    furi_crash(TAG "Invalid view selected");
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
            /* We need to set a navigation callback for the view dispatcher
             * otherwise when the user presses back on the keyboard to
             * abort, the dispatcher will not stop. */
            view_dispatcher_set_navigation_event_callback(
                app->view_dispatcher, keyboard_view_dispatcher_navigation_callback);
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
