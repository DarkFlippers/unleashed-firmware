#include "gpio_test.h"
#include "../gpio_items.h"

#include <gui/elements.h>

struct GpioTest {
    View* view;
    GpioTestOkCallback callback;
    void* context;
};

typedef struct {
    uint8_t pin_idx;
    GPIOItems* gpio_items;
} GpioTestModel;

static bool gpio_test_process_left(GpioTest* gpio_test);
static bool gpio_test_process_right(GpioTest* gpio_test);
static bool gpio_test_process_ok(GpioTest* gpio_test, InputEvent* event);

static void gpio_test_draw_callback(Canvas* canvas, void* _model) {
    GpioTestModel* model = _model;
    canvas_set_font(canvas, FontPrimary);
    elements_multiline_text_aligned(canvas, 64, 2, AlignCenter, AlignTop, "GPIO Output Mode Test");
    canvas_set_font(canvas, FontSecondary);
    elements_multiline_text_aligned(
        canvas, 64, 16, AlignCenter, AlignTop, "Press < or > to change pin");
    elements_multiline_text_aligned(
        canvas,
        64,
        32,
        AlignCenter,
        AlignTop,
        gpio_items_get_pin_name(model->gpio_items, model->pin_idx));
}

static bool gpio_test_input_callback(InputEvent* event, void* context) {
    furi_assert(context);
    GpioTest* gpio_test = context;
    bool consumed = false;

    if(event->type == InputTypeShort) {
        if(event->key == InputKeyRight) {
            consumed = gpio_test_process_right(gpio_test);
        } else if(event->key == InputKeyLeft) {
            consumed = gpio_test_process_left(gpio_test);
        }
    } else if(event->key == InputKeyOk) {
        consumed = gpio_test_process_ok(gpio_test, event);
    }

    return consumed;
}

static bool gpio_test_process_left(GpioTest* gpio_test) {
    with_view_model(
        gpio_test->view,
        GpioTestModel * model,
        {
            if(model->pin_idx) {
                model->pin_idx--;
            }
        },
        true);
    return true;
}

static bool gpio_test_process_right(GpioTest* gpio_test) {
    with_view_model(
        gpio_test->view,
        GpioTestModel * model,
        {
            if(model->pin_idx < gpio_items_get_count(model->gpio_items)) {
                model->pin_idx++;
            }
        },
        true);
    return true;
}

static bool gpio_test_process_ok(GpioTest* gpio_test, InputEvent* event) {
    bool consumed = false;

    with_view_model(
        gpio_test->view,
        GpioTestModel * model,
        {
            if(event->type == InputTypePress) {
                if(model->pin_idx < gpio_items_get_count(model->gpio_items)) {
                    gpio_items_set_pin(model->gpio_items, model->pin_idx, true);
                } else {
                    gpio_items_set_all_pins(model->gpio_items, true);
                }
                consumed = true;
            } else if(event->type == InputTypeRelease) {
                if(model->pin_idx < gpio_items_get_count(model->gpio_items)) {
                    gpio_items_set_pin(model->gpio_items, model->pin_idx, false);
                } else {
                    gpio_items_set_all_pins(model->gpio_items, false);
                }
                consumed = true;
            }
            gpio_test->callback(event->type, gpio_test->context);
        },
        true);

    return consumed;
}

GpioTest* gpio_test_alloc(GPIOItems* gpio_items) {
    GpioTest* gpio_test = malloc(sizeof(GpioTest));

    gpio_test->view = view_alloc();
    view_allocate_model(gpio_test->view, ViewModelTypeLocking, sizeof(GpioTestModel));

    with_view_model(
        gpio_test->view, GpioTestModel * model, { model->gpio_items = gpio_items; }, false);

    view_set_context(gpio_test->view, gpio_test);
    view_set_draw_callback(gpio_test->view, gpio_test_draw_callback);
    view_set_input_callback(gpio_test->view, gpio_test_input_callback);

    return gpio_test;
}

void gpio_test_free(GpioTest* gpio_test) {
    furi_assert(gpio_test);
    view_free(gpio_test->view);
    free(gpio_test);
}

View* gpio_test_get_view(GpioTest* gpio_test) {
    furi_assert(gpio_test);
    return gpio_test->view;
}

void gpio_test_set_ok_callback(GpioTest* gpio_test, GpioTestOkCallback callback, void* context) {
    furi_assert(gpio_test);
    furi_assert(callback);
    with_view_model(
        gpio_test->view,
        GpioTestModel * model,
        {
            UNUSED(model);
            gpio_test->callback = callback;
            gpio_test->context = context;
        },
        false);
}
