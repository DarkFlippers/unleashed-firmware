#include <furi.h>
#include <furi_hal_resources.h>

#include <gui/gui.h>
#include <gui/elements.h>
#include <gui/view_port.h>

#include <input/input.h>

#define TAG "EventLoopBlinkTest"

#define TIMER_COUNT (6U)

typedef struct {
    FuriEventLoop* event_loop;
    FuriMessageQueue* input_queue;
    FuriEventLoopTimer* timers[TIMER_COUNT];
} EventLoopBlinkTestApp;

static const GpioPin* blink_gpio_pins[] = {
    &gpio_ext_pa7,
    &gpio_ext_pa6,
    &gpio_ext_pa4,
    &gpio_ext_pb3,
    &gpio_ext_pb2,
    &gpio_ext_pc3,
};

static_assert(COUNT_OF(blink_gpio_pins) == TIMER_COUNT);

static const uint32_t timer_intervals[] = {
    25,
    50,
    100,
    200,
    400,
    800,
};

static_assert(COUNT_OF(timer_intervals) == TIMER_COUNT);

static void blink_gpio_init(void) {
    for(size_t i = 0; i < TIMER_COUNT; ++i) {
        furi_hal_gpio_init_simple(blink_gpio_pins[i], GpioModeOutputPushPull);
        furi_hal_gpio_write(blink_gpio_pins[i], false);
    }

    furi_hal_gpio_init_simple(&gpio_ext_pc0, GpioModeOutputPushPull);
    furi_hal_gpio_write(&gpio_ext_pc0, false);
}

static void blink_gpio_deinit(void) {
    for(size_t i = 0; i < TIMER_COUNT; ++i) {
        furi_hal_gpio_write(blink_gpio_pins[i], false);
        furi_hal_gpio_init_simple(blink_gpio_pins[i], GpioModeAnalog);
    }

    furi_hal_gpio_write(&gpio_ext_pc0, false);
    furi_hal_gpio_init_simple(&gpio_ext_pc0, GpioModeAnalog);
}

static void view_port_draw_callback(Canvas* canvas, void* context) {
    UNUSED(context);
    canvas_clear(canvas);
    elements_text_box(
        canvas,
        0,
        0,
        canvas_width(canvas),
        canvas_height(canvas),
        AlignCenter,
        AlignCenter,
        "\e#Event Loop Timers Test\e#\n"
        "Press buttons\n"
        "to enable or disable timers\n"
        "\e#Exit\e# = long press \e#Back\e#",
        false);
}

static void view_port_input_callback(InputEvent* input_event, void* context) {
    EventLoopBlinkTestApp* app = context;
    furi_message_queue_put(app->input_queue, input_event, 0);
}

static bool input_queue_callback(FuriEventLoopObject* object, void* context) {
    FuriMessageQueue* queue = object;
    EventLoopBlinkTestApp* app = context;

    InputEvent event;
    FuriStatus status = furi_message_queue_get(queue, &event, 0);
    furi_assert(status == FuriStatusOk);

    if(event.type == InputTypeShort) {
        const size_t timer_idx = event.key;
        furi_assert(timer_idx < TIMER_COUNT);

        FuriEventLoopTimer* timer = app->timers[timer_idx];

        if(furi_event_loop_timer_is_running(timer)) {
            furi_event_loop_timer_stop(timer);
        } else {
            furi_event_loop_timer_restart(timer);
        }

    } else if(event.type == InputTypeLong) {
        if(event.key == InputKeyBack) {
            furi_event_loop_stop(app->event_loop);
        }
    }

    return true;
}

static void blink_timer_callback(void* context) {
    const GpioPin* gpio = blink_gpio_pins[(size_t)context];
    furi_hal_gpio_write(gpio, !furi_hal_gpio_read(gpio));
}

static void event_loop_tick_callback(void* context) {
    UNUSED(context);
    furi_hal_gpio_write(&gpio_ext_pc0, !furi_hal_gpio_read(&gpio_ext_pc0));
}

int32_t event_loop_blink_test_app(void* arg) {
    UNUSED(arg);

    blink_gpio_init();

    EventLoopBlinkTestApp app;

    app.event_loop = furi_event_loop_alloc();
    app.input_queue = furi_message_queue_alloc(3, sizeof(InputEvent));

    for(size_t i = 0; i < TIMER_COUNT; ++i) {
        app.timers[i] = furi_event_loop_timer_alloc(
            app.event_loop, blink_timer_callback, FuriEventLoopTimerTypePeriodic, (void*)i);
        furi_event_loop_timer_start(app.timers[i], timer_intervals[i]);
    }

    ViewPort* view_port = view_port_alloc();
    view_port_draw_callback_set(view_port, view_port_draw_callback, &app);
    view_port_input_callback_set(view_port, view_port_input_callback, &app);

    Gui* gui = furi_record_open(RECORD_GUI);
    gui_add_view_port(gui, view_port, GuiLayerFullscreen);

    furi_event_loop_tick_set(app.event_loop, 500, event_loop_tick_callback, &app);
    furi_event_loop_subscribe_message_queue(
        app.event_loop, app.input_queue, FuriEventLoopEventIn, input_queue_callback, &app);

    furi_event_loop_run(app.event_loop);

    gui_remove_view_port(gui, view_port);
    view_port_free(view_port);

    furi_record_close(RECORD_GUI);

    furi_event_loop_unsubscribe(app.event_loop, app.input_queue);
    furi_message_queue_free(app.input_queue);

    for(size_t i = 0; i < TIMER_COUNT; ++i) {
        furi_event_loop_timer_free(app.timers[i]);
    }

    furi_event_loop_free(app.event_loop);

    blink_gpio_deinit();

    return 0;
}
