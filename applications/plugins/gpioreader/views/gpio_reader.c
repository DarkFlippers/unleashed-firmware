#include "gpio_reader.h"
#include "../gpio_item.h"

#include <gui/elements.h>
#include <furi_hal_resources.h>

struct GpioReader {
    View* view;
    GpioReaderOkCallback callback;
    void* context;
};

typedef struct {
    uint8_t pin_idx;
    bool pullUp[GPIO_ITEM_COUNT];
} GpioReaderModel;

static bool gpio_reader_process_ok(GpioReader* gpio_reader, InputEvent* event);
static bool gpio_reader_process_left(GpioReader* gpio_reader);
static bool gpio_reader_process_right(GpioReader* gpio_reader);

static void gpio_reader_draw_callback(Canvas* canvas, void* _model) {
    GpioReaderModel* model = _model;
    canvas_set_font(canvas, FontPrimary);
    elements_multiline_text_aligned(canvas, 64, 2, AlignCenter, AlignTop, "GPIO Reader");
    canvas_set_font(canvas, FontSecondary);
    elements_multiline_text_aligned(
        canvas, 64, 16, AlignCenter, AlignTop, "A7  A6  A4  B3  B2  C3  C1  C0");
    elements_multiline_text_aligned(canvas, 64, 40, AlignCenter, AlignTop, "Pull Up");
    int charOffset = 10;
    for(uint8_t i = 0; i < GPIO_ITEM_COUNT; i++) {
        bool high = gpio_item_get_pin(i);
        if(high) {
            elements_multiline_text_aligned(canvas, charOffset, 25, AlignCenter, AlignTop, "1");
        } else {
            elements_multiline_text_aligned(canvas, charOffset, 25, AlignCenter, AlignTop, "0");
        }

        if(model->pullUp[i]) {
            elements_multiline_text_aligned(canvas, charOffset, 50, AlignCenter, AlignTop, "1");
        } else {
            elements_multiline_text_aligned(canvas, charOffset, 50, AlignCenter, AlignTop, "0");
        }
        if(i == model->pin_idx) {
            elements_multiline_text_aligned(canvas, charOffset, 53, AlignCenter, AlignTop, "_");
        }

        charOffset += 16;
    }
    //~ free(charOffset);
}

static bool gpio_reader_input_callback(InputEvent* event, void* context) {
    furi_assert(context);
    GpioReader* gpio_reader = context;
    bool consumed = false;

    if(event->type == InputTypeShort) {
        if(event->key == InputKeyRight) {
            consumed = gpio_reader_process_right(gpio_reader);
        } else if(event->key == InputKeyLeft) {
            consumed = gpio_reader_process_left(gpio_reader);
        }
    } else if(event->key == InputKeyOk) {
        consumed = gpio_reader_process_ok(gpio_reader, event);
    }

    return consumed;
}

static bool gpio_reader_process_left(GpioReader* gpio_reader) {
    with_view_model(
        gpio_reader->view,
        GpioReaderModel * model,
        {
            if(model->pin_idx) {
                model->pin_idx--;
            }
        },
        true);
    return true;
}

static bool gpio_reader_process_right(GpioReader* gpio_reader) {
    with_view_model(
        gpio_reader->view,
        GpioReaderModel * model,
        {
            if(model->pin_idx < GPIO_ITEM_COUNT - 1) {
                model->pin_idx++;
            }
        },
        true);
    return true;
}

static bool gpio_reader_process_ok(GpioReader* gpio_reader, InputEvent* event) {
    bool consumed = false;

    with_view_model(
        gpio_reader->view,
        GpioReaderModel * model,
        {
            if(event->type == InputTypePress) {
                if(model->pullUp[model->pin_idx]) {
                    gpio_item_configure_pin(model->pin_idx, GpioModeInput, GpioPullDown);
                    model->pullUp[model->pin_idx] = 0;
                    consumed = true;
                } else {
                    gpio_item_configure_pin(model->pin_idx, GpioModeInput, GpioPullUp);
                    model->pullUp[model->pin_idx] = 1;
                    consumed = true;
                }
            }
            gpio_reader->callback(event->type, gpio_reader->context);
        },
        true);

    return consumed;
}

GpioReader* gpio_reader_alloc() {
    GpioReader* gpio_reader = malloc(sizeof(GpioReader));

    gpio_reader->view = view_alloc();
    view_allocate_model(gpio_reader->view, ViewModelTypeLocking, sizeof(GpioReaderModel));
    view_set_context(gpio_reader->view, gpio_reader);
    view_set_draw_callback(gpio_reader->view, gpio_reader_draw_callback);
    view_set_input_callback(gpio_reader->view, gpio_reader_input_callback);

    return gpio_reader;
}

void gpio_reader_free(GpioReader* gpio_reader) {
    furi_assert(gpio_reader);
    view_free(gpio_reader->view);
    free(gpio_reader);
}

View* gpio_reader_get_view(GpioReader* gpio_reader) {
    furi_assert(gpio_reader);
    return gpio_reader->view;
}

void gpio_reader_set_ok_callback(
    GpioReader* gpio_reader,
    GpioReaderOkCallback callback,
    void* context) {
    furi_assert(gpio_reader);
    furi_assert(callback);
    with_view_model(
        gpio_reader->view,
        GpioReaderModel * model,
        {
            UNUSED(model);
            gpio_reader->callback = callback;
            gpio_reader->context = context;
        },
        false);
}
