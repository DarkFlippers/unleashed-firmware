#include <furi.h>
#include <furi_hal.h>

#include <gui/gui.h>
#include <locale/locale.h>

typedef enum {
    ClockEventTypeTick,
    ClockEventTypeKey,
} ClockEventType;

typedef struct {
    ClockEventType type;
    InputEvent input;
} ClockEvent;

typedef struct {
    FuriString* buffer;
    FuriHalRtcDateTime datetime;
    LocaleTimeFormat timeformat;
    LocaleDateFormat dateformat;
} ClockData;

typedef struct {
    FuriMutex* mutex;
    FuriMessageQueue* queue;
    ClockData* data;
} Clock;

static void clock_input_callback(InputEvent* input_event, FuriMessageQueue* queue) {
    furi_assert(queue);
    ClockEvent event = {.type = ClockEventTypeKey, .input = *input_event};
    furi_message_queue_put(queue, &event, FuriWaitForever);
}

static void clock_render_callback(Canvas* canvas, void* ctx) {
    Clock* clock = ctx;
    if(furi_mutex_acquire(clock->mutex, 200) != FuriStatusOk) {
        return;
    }

    ClockData* data = clock->data;

    canvas_set_font(canvas, FontBigNumbers);
    locale_format_time(data->buffer, &data->datetime, data->timeformat, true);
    canvas_draw_str_aligned(
        canvas, 64, 28, AlignCenter, AlignCenter, furi_string_get_cstr(data->buffer));

    // Special case to cover missing glyphs in FontBigNumbers
    if(data->timeformat == LocaleTimeFormat12h) {
        size_t time_width = canvas_string_width(canvas, furi_string_get_cstr(data->buffer));
        canvas_set_font(canvas, FontPrimary);
        canvas_draw_str_aligned(
            canvas,
            64 + (time_width / 2) - 10,
            31,
            AlignLeft,
            AlignCenter,
            (data->datetime.hour > 12) ? "AM" : "PM");
    }

    canvas_set_font(canvas, FontSecondary);
    locale_format_date(data->buffer, &data->datetime, data->dateformat, "/");
    canvas_draw_str_aligned(
        canvas, 64, 42, AlignCenter, AlignTop, furi_string_get_cstr(data->buffer));

    furi_mutex_release(clock->mutex);
}

static void clock_tick(void* ctx) {
    furi_assert(ctx);
    FuriMessageQueue* queue = ctx;
    ClockEvent event = {.type = ClockEventTypeTick};
    // It's OK to loose this event if system overloaded
    furi_message_queue_put(queue, &event, 0);
}

int32_t clock_app(void* p) {
    UNUSED(p);
    Clock* clock = malloc(sizeof(Clock));
    clock->data = malloc(sizeof(ClockData));
    clock->data->buffer = furi_string_alloc();

    clock->queue = furi_message_queue_alloc(8, sizeof(ClockEvent));
    clock->mutex = furi_mutex_alloc(FuriMutexTypeNormal);

    furi_hal_rtc_get_datetime(&clock->data->datetime);
    clock->data->timeformat = locale_get_time_format();
    clock->data->dateformat = locale_get_date_format();

    // Set ViewPort callbacks
    ViewPort* view_port = view_port_alloc();
    view_port_draw_callback_set(view_port, clock_render_callback, clock);
    view_port_input_callback_set(view_port, clock_input_callback, clock->queue);

    FuriTimer* timer = furi_timer_alloc(clock_tick, FuriTimerTypePeriodic, clock->queue);

    // Open GUI and register view_port
    Gui* gui = furi_record_open(RECORD_GUI);
    gui_add_view_port(gui, view_port, GuiLayerFullscreen);

    furi_timer_start(timer, 100);

    // Main loop
    ClockEvent event;
    for(bool processing = true; processing;) {
        furi_check(furi_message_queue_get(clock->queue, &event, FuriWaitForever) == FuriStatusOk);
        furi_mutex_acquire(clock->mutex, FuriWaitForever);
        if(event.type == ClockEventTypeKey) {
            if(event.input.type == InputTypeShort && event.input.key == InputKeyBack) {
                processing = false;
            }
        } else if(event.type == ClockEventTypeTick) {
            furi_hal_rtc_get_datetime(&clock->data->datetime);
        }

        furi_mutex_release(clock->mutex);
        view_port_update(view_port);
    }

    furi_timer_free(timer);
    view_port_enabled_set(view_port, false);
    gui_remove_view_port(gui, view_port);
    view_port_free(view_port);
    furi_record_close(RECORD_GUI);

    furi_message_queue_free(clock->queue);
    furi_mutex_free(clock->mutex);

    furi_string_free(clock->data->buffer);

    free(clock->data);
    free(clock);

    return 0;
}