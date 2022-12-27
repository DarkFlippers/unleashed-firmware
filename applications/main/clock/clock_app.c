#include <furi.h>
#include <furi_hal.h>

#include <gui/gui.h>
#include <gui/elements.h>

#include "clock_app.h"

static void clock_input_callback(InputEvent* input_event, FuriMessageQueue* event_queue) {
    furi_assert(event_queue);
    PluginEvent event = {.type = EventTypeKey, .input = *input_event};
    furi_message_queue_put(event_queue, &event, FuriWaitForever);
}

static void clock_render_callback(Canvas* const canvas, void* ctx) {
    //canvas_clear(canvas);
    //canvas_set_color(canvas, ColorBlack);

    ClockState* state = ctx;
    if(furi_mutex_acquire(state->mutex, 200) != FuriStatusOk) {
        //FURI_LOG_D(TAG, "Can't obtain mutex, requeue render");
        PluginEvent event = {.type = EventTypeTick};
        furi_message_queue_put(state->event_queue, &event, 0);
        return;
    }

    FuriHalRtcDateTime curr_dt;
    furi_hal_rtc_get_datetime(&curr_dt);
    uint32_t curr_ts = furi_hal_rtc_datetime_to_timestamp(&curr_dt);

    char time_string[TIME_LEN];
    char date_string[DATE_LEN];
    char meridian_string[MERIDIAN_LEN];
    char timer_string[20];

    if(state->time_format == LocaleTimeFormat24h) {
        snprintf(
            time_string, TIME_LEN, CLOCK_TIME_FORMAT, curr_dt.hour, curr_dt.minute, curr_dt.second);
    } else {
        bool pm = curr_dt.hour > 12;
        bool pm12 = curr_dt.hour >= 12;
        snprintf(
            time_string,
            TIME_LEN,
            CLOCK_TIME_FORMAT,
            pm ? curr_dt.hour - 12 : curr_dt.hour,
            curr_dt.minute,
            curr_dt.second);

        snprintf(
            meridian_string,
            MERIDIAN_LEN,
            MERIDIAN_FORMAT,
            pm12 ? MERIDIAN_STRING_PM : MERIDIAN_STRING_AM);
    }

    if(state->date_format == LocaleDateFormatYMD) {
        snprintf(
            date_string, DATE_LEN, CLOCK_ISO_DATE_FORMAT, curr_dt.year, curr_dt.month, curr_dt.day);
    } else if(state->date_format == LocaleDateFormatMDY) {
        snprintf(
            date_string, DATE_LEN, CLOCK_RFC_DATE_FORMAT, curr_dt.month, curr_dt.day, curr_dt.year);
    } else {
        snprintf(
            date_string, DATE_LEN, CLOCK_RFC_DATE_FORMAT, curr_dt.day, curr_dt.month, curr_dt.year);
    }

    bool timer_running = state->timer_running;
    uint32_t timer_start_timestamp = state->timer_start_timestamp;
    uint32_t timer_stopped_seconds = state->timer_stopped_seconds;

    furi_mutex_release(state->mutex);

    canvas_set_font(canvas, FontBigNumbers);

    if(timer_start_timestamp != 0) {
        int32_t elapsed_secs = timer_running ? (curr_ts - timer_start_timestamp) :
                                               timer_stopped_seconds;
        snprintf(timer_string, 20, "%.2ld:%.2ld", elapsed_secs / 60, elapsed_secs % 60);
        canvas_draw_str_aligned(canvas, 64, 8, AlignCenter, AlignCenter, time_string); // DRAW TIME
        canvas_draw_str_aligned(canvas, 64, 32, AlignCenter, AlignTop, timer_string); // DRAW TIMER
        canvas_set_font(canvas, FontSecondary);
        canvas_draw_str_aligned(canvas, 64, 20, AlignCenter, AlignTop, date_string); // DRAW DATE
        elements_button_left(canvas, "Reset");
    } else {
        canvas_draw_str_aligned(canvas, 64, 28, AlignCenter, AlignCenter, time_string);
        canvas_set_font(canvas, FontSecondary);
        canvas_draw_str_aligned(canvas, 64, 42, AlignCenter, AlignTop, date_string);

        if(state->time_format == LocaleTimeFormat12h)
            canvas_draw_str_aligned(canvas, 65, 12, AlignCenter, AlignCenter, meridian_string);
    }
    if(timer_running) {
        elements_button_center(canvas, "Stop");
    } else if(timer_start_timestamp != 0 && !timer_running) {
        elements_button_center(canvas, "Start");
    }
}

static void clock_state_init(ClockState* const state) {
    state->time_format = locale_get_time_format();

    state->date_format = locale_get_date_format();

    //FURI_LOG_D(TAG, "Time format: %s", state->settings.time_format == H12 ? "12h" : "24h");
    //FURI_LOG_D(TAG, "Date format: %s", state->settings.date_format == Iso ? "ISO 8601" : "RFC 5322");
    //furi_hal_rtc_get_datetime(&state->datetime);
}

// Runs every 1000ms by default
static void clock_tick(void* ctx) {
    furi_assert(ctx);
    FuriMessageQueue* event_queue = ctx;
    PluginEvent event = {.type = EventTypeTick};
    // It's OK to loose this event if system overloaded
    furi_message_queue_put(event_queue, &event, 0);
}

int32_t clock_app(void* p) {
    UNUSED(p);
    ClockState* plugin_state = malloc(sizeof(ClockState));

    plugin_state->event_queue = furi_message_queue_alloc(8, sizeof(PluginEvent));
    if(plugin_state->event_queue == NULL) {
        FURI_LOG_E(TAG, "Cannot create event queue");
        free(plugin_state);
        return 255;
    }
    //FURI_LOG_D(TAG, "Event queue created");

    plugin_state->mutex = furi_mutex_alloc(FuriMutexTypeNormal);
    if(plugin_state->mutex == NULL) {
        FURI_LOG_E(TAG, "Cannot create mutex");
        furi_message_queue_free(plugin_state->event_queue);
        free(plugin_state);
        return 255;
    }
    //FURI_LOG_D(TAG, "Mutex created");

    clock_state_init(plugin_state);

    // Set system callbacks
    ViewPort* view_port = view_port_alloc();
    view_port_draw_callback_set(view_port, clock_render_callback, plugin_state);
    view_port_input_callback_set(view_port, clock_input_callback, plugin_state->event_queue);

    FuriTimer* timer =
        furi_timer_alloc(clock_tick, FuriTimerTypePeriodic, plugin_state->event_queue);

    if(timer == NULL) {
        FURI_LOG_E(TAG, "Cannot create timer");
        furi_mutex_free(plugin_state->mutex);
        furi_message_queue_free(plugin_state->event_queue);
        free(plugin_state);
        return 255;
    }
    //FURI_LOG_D(TAG, "Timer created");

    // Open GUI and register view_port
    Gui* gui = furi_record_open(RECORD_GUI);
    gui_add_view_port(gui, view_port, GuiLayerFullscreen);

    furi_timer_start(timer, furi_kernel_get_tick_frequency());
    //FURI_LOG_D(TAG, "Timer started");

    // Main loop
    PluginEvent event;
    for(bool processing = true; processing;) {
        FuriStatus event_status = furi_message_queue_get(plugin_state->event_queue, &event, 100);

        if(event_status != FuriStatusOk) continue;

        if(furi_mutex_acquire(plugin_state->mutex, FuriWaitForever) != FuriStatusOk) continue;
        // press events
        if(event.type == EventTypeKey) {
            if(event.input.type == InputTypeShort || event.input.type == InputTypeRepeat) {
                switch(event.input.key) {
                case InputKeyUp:
                case InputKeyDown:
                case InputKeyRight:
                    break;
                case InputKeyLeft:
                    if(plugin_state->timer_start_timestamp != 0) {
                        // Reset seconds
                        plugin_state->timer_running = false;
                        plugin_state->timer_start_timestamp = 0;
                        plugin_state->timer_stopped_seconds = 0;
                    }
                    break;
                case InputKeyOk:;
                    // START/STOP TIMER

                    FuriHalRtcDateTime curr_dt;
                    furi_hal_rtc_get_datetime(&curr_dt);
                    uint32_t curr_ts = furi_hal_rtc_datetime_to_timestamp(&curr_dt);

                    if(plugin_state->timer_running) {
                        // Update stopped seconds
                        plugin_state->timer_stopped_seconds =
                            curr_ts - plugin_state->timer_start_timestamp;
                    } else {
                        if(plugin_state->timer_start_timestamp == 0) {
                            // Set starting timestamp if this is first time
                            plugin_state->timer_start_timestamp = curr_ts;
                        } else {
                            // Timer was already running, need to slightly readjust so we don't
                            // count the intervening time
                            plugin_state->timer_start_timestamp =
                                curr_ts - plugin_state->timer_stopped_seconds;
                        }
                    }
                    plugin_state->timer_running = !plugin_state->timer_running;
                    break;
                case InputKeyBack:
                    // Exit the plugin
                    processing = false;
                    break;
                default:
                    break;
                }
            }
        } /*else if(event.type == EventTypeTick) {
            furi_hal_rtc_get_datetime(&plugin_state->datetime);
        }*/

        view_port_update(view_port);
        furi_mutex_release(plugin_state->mutex);
    }

    furi_timer_free(timer);
    view_port_enabled_set(view_port, false);
    gui_remove_view_port(gui, view_port);
    furi_record_close(RECORD_GUI);
    view_port_free(view_port);
    furi_message_queue_free(plugin_state->event_queue);
    furi_mutex_free(plugin_state->mutex);
    free(plugin_state);

    return 0;
}