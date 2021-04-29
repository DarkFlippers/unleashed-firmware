#include <furi.h>
#include <api-hal.h>

#include <gui/gui.h>
#include <input/input.h>

typedef struct {
    InputEvent input;
} VibroEvent;

void vibro_draw_callback(Canvas* canvas, void* ctx) {
    canvas_clear(canvas);
    canvas_set_font(canvas, FontPrimary);
    canvas_draw_str(canvas, 2, 10, "Vibro application");
    canvas_set_font(canvas, FontSecondary);
    canvas_draw_str(canvas, 2, 22, "Press OK turns on vibro");
    canvas_set_font(canvas, FontSecondary);
    canvas_draw_str(canvas, 2, 34, "Release OK turns off vibro");
}

void vibro_input_callback(InputEvent* input_event, void* ctx) {
    furi_assert(ctx);
    osMessageQueueId_t event_queue = ctx;

    VibroEvent event = {.input = *input_event};
    osMessageQueuePut(event_queue, &event, 0, 0);
}

int32_t application_vibro(void* p) {
    GpioPin* gpio = (GpioPin*)&vibro_gpio;
    osMessageQueueId_t event_queue = osMessageQueueNew(8, sizeof(VibroEvent), NULL);

    // Configure view port
    ViewPort* view_port = view_port_alloc();
    furi_check(view_port);
    view_port_draw_callback_set(view_port, vibro_draw_callback, NULL);
    view_port_input_callback_set(view_port, vibro_input_callback, event_queue);

    // Register view port in GUI
    Gui* gui = furi_record_open("gui");
    gui_add_view_port(gui, view_port, GuiLayerFullscreen);

    hal_gpio_init(gpio, GpioModeOutputPushPull, GpioPullNo, GpioSpeedLow);
    hal_gpio_write(gpio, false);
    VibroEvent event;

    while(1) {
        furi_check(osMessageQueueGet(event_queue, &event, NULL, osWaitForever) == osOK);
        if(event.input.type == InputTypeShort && event.input.key == InputKeyBack) {
            hal_gpio_write(gpio, false);
            api_hal_light_set(LightGreen, 0);
            view_port_enabled_set(view_port, false);
            gui_remove_view_port(gui, view_port);
            view_port_free(view_port);
            osMessageQueueDelete(event_queue);

            return 0;
        }
        if(event.input.key == InputKeyOk) {
            if(event.input.type == InputTypePress) {
                hal_gpio_write(gpio, true);
                api_hal_light_set(LightGreen, 255);
            } else if(event.input.type == InputTypeRelease) {
                hal_gpio_write(gpio, false);
                api_hal_light_set(LightGreen, 0);
            }
        }
    }

    return 0;
}