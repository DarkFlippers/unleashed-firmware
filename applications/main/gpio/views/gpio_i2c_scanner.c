#include <gui/elements.h>
#include "gpio_i2c_scanner.h"
#include "../gpio_item.h"

#include <string.h>

struct GpioI2CScanner {
    View* view;
    GpioI2CScannerOkCallback callback;
    void* context;
};

typedef struct {
    uint8_t items;
    uint8_t responding_address[AVAILABLE_NONRESVERED_I2C_ADDRESSES];
} GpioI2CScannerModel;

static bool gpio_i2c_scanner_process_ok(GpioI2CScanner* gpio_i2c_scanner, InputEvent* event);

static void gpio_i2c_scanner_draw_callback(Canvas* canvas, void* _model) {
    GpioI2CScannerModel* model = _model;

    char temp_str[25];
    elements_button_center(canvas, "Start scan");
    canvas_draw_line(canvas, 2, 10, 125, 10);
    canvas_draw_line(canvas, 2, 52, 125, 52);

    canvas_set_font(canvas, FontPrimary);
    canvas_draw_str(canvas, 2, 9, "I2C-Scanner");
    canvas_draw_str(canvas, 3, 25, "SDA:");
    canvas_draw_str(canvas, 3, 42, "SCL:");

    canvas_set_font(canvas, FontSecondary);
    snprintf(temp_str, 25, "Slaves: %u", model->items);
    canvas_draw_str_aligned(canvas, 126, 8, AlignRight, AlignBottom, temp_str);

    canvas_draw_str(canvas, 29, 25, "Pin 15");
    canvas_draw_str(canvas, 29, 42, "Pin 16");

    canvas_set_font(canvas, FontSecondary);

    char temp_str2[6];
    if(model->items > 0) {
        snprintf(temp_str, 25, "Addr: ");
        for(int i = 0; i < model->items; i++) {
            snprintf(temp_str2, 6, "0x%x ", model->responding_address[i]);
            strcat(temp_str, temp_str2);

            if(i == 1 || model->items == 1) { //Draw a maximum of two addresses in the first line
                canvas_draw_str_aligned(canvas, 127, 24, AlignRight, AlignBottom, temp_str);
                temp_str[0] = '\0';
            } else if(
                i == 4 || (model->items - 1 == i &&
                           i < 6)) { //Draw a maximum of three addresses in the second line
                canvas_draw_str_aligned(canvas, 127, 36, AlignRight, AlignBottom, temp_str);
                temp_str[0] = '\0';
            } else if(i == 7 || model->items - 1 == i) { //Draw a maximum of three addresses in the third line
                canvas_draw_str_aligned(canvas, 127, 48, AlignRight, AlignBottom, temp_str);
                break;
            }
        }
    }
}

static bool gpio_i2c_scanner_input_callback(InputEvent* event, void* context) {
    furi_assert(context);
    GpioI2CScanner* gpio_i2c_scanner = context;
    bool consumed = false;

    if(event->key == InputKeyOk) {
        consumed = gpio_i2c_scanner_process_ok(gpio_i2c_scanner, event);
    }

    return consumed;
}

static bool gpio_i2c_scanner_process_ok(GpioI2CScanner* gpio_i2c_scanner, InputEvent* event) {
    bool consumed = false;
    gpio_i2c_scanner->callback(event->type, gpio_i2c_scanner->context);

    return consumed;
}

GpioI2CScanner* gpio_i2c_scanner_alloc() {
    GpioI2CScanner* gpio_i2c_scanner = malloc(sizeof(GpioI2CScanner));

    gpio_i2c_scanner->view = view_alloc();
    view_allocate_model(gpio_i2c_scanner->view, ViewModelTypeLocking, sizeof(GpioI2CScannerModel));
    view_set_context(gpio_i2c_scanner->view, gpio_i2c_scanner);
    view_set_draw_callback(gpio_i2c_scanner->view, gpio_i2c_scanner_draw_callback);
    view_set_input_callback(gpio_i2c_scanner->view, gpio_i2c_scanner_input_callback);

    return gpio_i2c_scanner;
}

void gpio_i2c_scanner_free(GpioI2CScanner* gpio_i2c_scanner) {
    furi_assert(gpio_i2c_scanner);
    view_free(gpio_i2c_scanner->view);
    free(gpio_i2c_scanner);
}

View* gpio_i2c_scanner_get_view(GpioI2CScanner* gpio_i2c_scanner) {
    furi_assert(gpio_i2c_scanner);
    return gpio_i2c_scanner->view;
}

void gpio_i2c_scanner_set_ok_callback(
    GpioI2CScanner* gpio_i2c_scanner,
    GpioI2CScannerOkCallback callback,
    void* context) {
    furi_assert(gpio_i2c_scanner);
    furi_assert(callback);
    with_view_model(
        gpio_i2c_scanner->view,
        GpioI2CScannerModel * model,
        {
            UNUSED(model);
            gpio_i2c_scanner->callback = callback;
            gpio_i2c_scanner->context = context;
        },
        false);
}

void gpio_i2c_scanner_update_state(GpioI2CScanner* instance, I2CScannerState* st) {
    furi_assert(instance);
    furi_assert(st);

    with_view_model(
        instance->view,
        GpioI2CScannerModel * model,
        {
            model->items = st->items;

            for(int i = 0; i < model->items; i++) {
                model->responding_address[i] = st->responding_address[i];
            }
        },
        true);
}
