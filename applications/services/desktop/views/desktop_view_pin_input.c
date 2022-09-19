#include <gui/canvas.h>
#include <furi.h>
#include <gui/view.h>
#include <gui/elements.h>
#include <stdint.h>
#include <portmacro.h>

#include "desktop_view_pin_input.h"
#include <desktop/desktop_settings.h>

#define NO_ACTIVITY_TIMEOUT 15000

#define PIN_CELL_WIDTH 13
#define DEFAULT_PIN_X 64
#define DEFAULT_PIN_Y 32

struct DesktopViewPinInput {
    View* view;
    DesktopViewPinInputCallback back_callback;
    DesktopViewPinInputCallback timeout_callback;
    DesktopViewPinInputDoneCallback done_callback;
    void* context;
    TimerHandle_t timer;
};

typedef struct {
    PinCode pin;
    bool pin_hidden;
    bool locked_input;
    uint8_t pin_x;
    uint8_t pin_y;
    const char* primary_str;
    uint8_t primary_str_x;
    uint8_t primary_str_y;
    const char* secondary_str;
    uint8_t secondary_str_x;
    uint8_t secondary_str_y;
    const char* button_label;
} DesktopViewPinInputModel;

static bool desktop_view_pin_input_input(InputEvent* event, void* context) {
    furi_assert(event);
    furi_assert(context);

    DesktopViewPinInput* pin_input = context;
    DesktopViewPinInputModel* model = view_get_model(pin_input->view);

    bool call_back_callback = false;
    bool call_done_callback = false;
    PinCode pin_code = {0};

    if(event->type == InputTypeShort) {
        switch(event->key) {
        case InputKeyRight:
        case InputKeyLeft:
        case InputKeyDown:
        case InputKeyUp:
            if(!model->locked_input) {
                if(model->pin.length < MAX_PIN_SIZE) {
                    model->pin.data[model->pin.length++] = event->key;
                }
            }
            break;
        case InputKeyOk:
            if(model->pin.length >= MIN_PIN_SIZE) {
                call_done_callback = true;
                pin_code = model->pin;
            }
            break;
        case InputKeyBack:
            if(!model->locked_input) {
                if(model->pin.length > 0) {
                    model->pin.length = 0;
                } else {
                    call_back_callback = true;
                }
            }
            break;
        default:
            furi_assert(0);
            break;
        }
    }
    view_commit_model(pin_input->view, true);

    if(call_done_callback && pin_input->done_callback) {
        pin_input->done_callback(&pin_code, pin_input->context);
    } else if(call_back_callback && pin_input->back_callback) {
        pin_input->back_callback(pin_input->context);
    }

    xTimerStart(pin_input->timer, 0);

    return true;
}

static void desktop_view_pin_input_draw_cells(Canvas* canvas, DesktopViewPinInputModel* model) {
    furi_assert(canvas);
    furi_assert(model);

    uint8_t draw_pin_size = MAX(4, model->pin.length + 1);
    if(model->locked_input || (model->pin.length == MAX_PIN_SIZE)) {
        draw_pin_size = model->pin.length;
    }

    uint8_t x = model->pin_x - (draw_pin_size * (PIN_CELL_WIDTH - 1)) / 2;
    uint8_t y = model->pin_y - (PIN_CELL_WIDTH / 2);

    for(int i = 0; i < draw_pin_size; ++i) {
        canvas_draw_frame(canvas, x, y, PIN_CELL_WIDTH, PIN_CELL_WIDTH);
        if(i < model->pin.length) {
            if(model->pin_hidden) {
                canvas_draw_icon(canvas, x + 3, y + 3, &I_Pin_star_7x7);
            } else {
                switch(model->pin.data[i]) {
                case InputKeyDown:
                    canvas_draw_icon(canvas, x + 3, y + 2, &I_Pin_arrow_down_7x9);
                    break;
                case InputKeyUp:
                    canvas_draw_icon(canvas, x + 3, y + 2, &I_Pin_arrow_up_7x9);
                    break;
                case InputKeyLeft:
                    canvas_draw_icon(canvas, x + 2, y + 3, &I_Pin_arrow_left_9x7);
                    break;
                case InputKeyRight:
                    canvas_draw_icon(canvas, x + 2, y + 3, &I_Pin_arrow_right_9x7);
                    break;
                default:
                    furi_assert(0);
                    break;
                }
            }
        } else if(i == model->pin.length) {
            canvas_draw_icon(canvas, x + 4, y + PIN_CELL_WIDTH + 1, &I_Pin_pointer_5x3);
        }
        x += PIN_CELL_WIDTH - 1;
    }
}

static void desktop_view_pin_input_draw(Canvas* canvas, void* context) {
    furi_assert(canvas);
    furi_assert(context);

    canvas_set_font(canvas, FontSecondary);
    DesktopViewPinInputModel* model = context;
    desktop_view_pin_input_draw_cells(canvas, model);

    if((model->pin.length > 0) && !model->locked_input) {
        canvas_draw_icon(canvas, 4, 53, &I_Pin_back_full_40x8);
    }

    if(model->button_label && ((model->pin.length >= MIN_PIN_SIZE) || model->locked_input)) {
        elements_button_center(canvas, model->button_label);
    }

    if(model->primary_str) {
        canvas_set_font(canvas, FontPrimary);
        canvas_draw_str(canvas, model->primary_str_x, model->primary_str_y, model->primary_str);
        canvas_set_font(canvas, FontSecondary);
    }

    if(model->secondary_str) {
        canvas_set_font(canvas, FontSecondary);
        canvas_draw_str(
            canvas, model->secondary_str_x, model->secondary_str_y, model->secondary_str);
    }
}

void desktop_view_pin_input_timer_callback(TimerHandle_t timer) {
    DesktopViewPinInput* pin_input = pvTimerGetTimerID(timer);

    if(pin_input->timeout_callback) {
        pin_input->timeout_callback(pin_input->context);
    }
}

static void desktop_view_pin_input_enter(void* context) {
    DesktopViewPinInput* pin_input = context;
    xTimerStart(pin_input->timer, portMAX_DELAY);
}

static void desktop_view_pin_input_exit(void* context) {
    DesktopViewPinInput* pin_input = context;
    xTimerStop(pin_input->timer, portMAX_DELAY);
}

DesktopViewPinInput* desktop_view_pin_input_alloc(void) {
    DesktopViewPinInput* pin_input = malloc(sizeof(DesktopViewPinInput));
    pin_input->view = view_alloc();
    view_allocate_model(pin_input->view, ViewModelTypeLocking, sizeof(DesktopViewPinInputModel));
    view_set_context(pin_input->view, pin_input);
    view_set_draw_callback(pin_input->view, desktop_view_pin_input_draw);
    view_set_input_callback(pin_input->view, desktop_view_pin_input_input);
    pin_input->timer = xTimerCreate(
        NULL,
        pdMS_TO_TICKS(NO_ACTIVITY_TIMEOUT),
        pdFALSE,
        pin_input,
        desktop_view_pin_input_timer_callback);
    view_set_enter_callback(pin_input->view, desktop_view_pin_input_enter);
    view_set_exit_callback(pin_input->view, desktop_view_pin_input_exit);

    DesktopViewPinInputModel* model = view_get_model(pin_input->view);
    model->pin_x = DEFAULT_PIN_X;
    model->pin_y = DEFAULT_PIN_Y;
    model->pin.length = 0;
    view_commit_model(pin_input->view, false);

    return pin_input;
}

void desktop_view_pin_input_free(DesktopViewPinInput* pin_input) {
    furi_assert(pin_input);

    xTimerStop(pin_input->timer, portMAX_DELAY);
    while(xTimerIsTimerActive(pin_input->timer)) {
        furi_delay_tick(1);
    }
    xTimerDelete(pin_input->timer, portMAX_DELAY);

    view_free(pin_input->view);
    free(pin_input);
}

void desktop_view_pin_input_lock_input(DesktopViewPinInput* pin_input) {
    furi_assert(pin_input);

    DesktopViewPinInputModel* model = view_get_model(pin_input->view);
    model->locked_input = true;
    view_commit_model(pin_input->view, true);
}

void desktop_view_pin_input_unlock_input(DesktopViewPinInput* pin_input) {
    furi_assert(pin_input);

    DesktopViewPinInputModel* model = view_get_model(pin_input->view);
    model->locked_input = false;
    view_commit_model(pin_input->view, true);
}

void desktop_view_pin_input_set_pin(DesktopViewPinInput* pin_input, const PinCode* pin) {
    furi_assert(pin_input);
    furi_assert(pin);

    DesktopViewPinInputModel* model = view_get_model(pin_input->view);
    model->pin = *pin;
    view_commit_model(pin_input->view, true);
}

void desktop_view_pin_input_reset_pin(DesktopViewPinInput* pin_input) {
    furi_assert(pin_input);

    DesktopViewPinInputModel* model = view_get_model(pin_input->view);
    model->pin.length = 0;
    view_commit_model(pin_input->view, true);
}

void desktop_view_pin_input_hide_pin(DesktopViewPinInput* pin_input, bool pin_hidden) {
    furi_assert(pin_input);

    DesktopViewPinInputModel* model = view_get_model(pin_input->view);
    model->pin_hidden = pin_hidden;
    view_commit_model(pin_input->view, true);
}

void desktop_view_pin_input_set_label_button(DesktopViewPinInput* pin_input, const char* label) {
    furi_assert(pin_input);

    DesktopViewPinInputModel* model = view_get_model(pin_input->view);
    model->button_label = label;
    view_commit_model(pin_input->view, true);
}

void desktop_view_pin_input_set_label_primary(
    DesktopViewPinInput* pin_input,
    uint8_t x,
    uint8_t y,
    const char* label) {
    furi_assert(pin_input);

    DesktopViewPinInputModel* model = view_get_model(pin_input->view);
    model->primary_str = label;
    model->primary_str_x = x;
    model->primary_str_y = y;
    view_commit_model(pin_input->view, true);
}

void desktop_view_pin_input_set_label_secondary(
    DesktopViewPinInput* pin_input,
    uint8_t x,
    uint8_t y,
    const char* label) {
    furi_assert(pin_input);

    DesktopViewPinInputModel* model = view_get_model(pin_input->view);
    model->secondary_str = label;
    model->secondary_str_x = x;
    model->secondary_str_y = y;
    view_commit_model(pin_input->view, true);
}

void desktop_view_pin_input_set_pin_position(DesktopViewPinInput* pin_input, uint8_t x, uint8_t y) {
    furi_assert(pin_input);

    DesktopViewPinInputModel* model = view_get_model(pin_input->view);
    model->pin_x = x;
    model->pin_y = y;
    view_commit_model(pin_input->view, true);
}

void desktop_view_pin_input_set_context(DesktopViewPinInput* pin_input, void* context) {
    furi_assert(pin_input);
    pin_input->context = context;
}

void desktop_view_pin_input_set_timeout_callback(
    DesktopViewPinInput* pin_input,
    DesktopViewPinInputCallback callback) {
    furi_assert(pin_input);
    pin_input->timeout_callback = callback;
}

void desktop_view_pin_input_set_back_callback(
    DesktopViewPinInput* pin_input,
    DesktopViewPinInputCallback callback) {
    furi_assert(pin_input);
    pin_input->back_callback = callback;
}

void desktop_view_pin_input_set_done_callback(
    DesktopViewPinInput* pin_input,
    DesktopViewPinInputDoneCallback callback) {
    furi_assert(pin_input);
    pin_input->done_callback = callback;
}

View* desktop_view_pin_input_get_view(DesktopViewPinInput* pin_input) {
    furi_assert(pin_input);
    return pin_input->view;
}
