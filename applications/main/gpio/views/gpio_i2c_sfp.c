#include <gui/elements.h>
#include "gpio_i2c_sfp.h"
#include "../gpio_item.h"

#include <string.h>

struct GpioI2CSfp {
    View* view;
    GpioI2CSfpOkCallback callback;
    void* context;
};

typedef struct {
    char vendor[32];
    char oui[32];
    char rev[32];
    char pn[32];
    char sn[32];
    char dc[32];
    uint8_t type;
    char connector[32];
    int wavelength;
    int sm_reach;
    int mm_reach_om3;
    int bitrate;
} GpioI2CSfpModel;

static bool gpio_i2c_sfp_process_ok(GpioI2CSfp* gpio_i2c_sfp, InputEvent* event);

static void gpio_i2c_sfp_draw_callback(Canvas* canvas, void* _model) {
    GpioI2CSfpModel* model = _model;

    // Temp String for formatting output
    char temp_str[280];

    canvas_set_font(canvas, FontSecondary);
    elements_button_center(canvas, "Read");
    canvas_draw_str(canvas, 2, 63, "P15 SCL");
    canvas_draw_str(canvas, 92, 63, "P16 SDA");

    snprintf(temp_str, 280, "Vendor: %s", model->vendor);
    canvas_draw_str(canvas, 2, 9, temp_str);

    snprintf(temp_str, 280, "PN: %s", model->pn);
    canvas_draw_str(canvas, 2, 19, temp_str);

    snprintf(temp_str, 280, "SN: %s", model->sn);
    canvas_draw_str(canvas, 2, 29, temp_str);

    snprintf(temp_str, 280, "REV: %s", model->rev);
    canvas_draw_str(canvas, 2, 39, temp_str);

    snprintf(temp_str, 280, "CON: %s", model->connector);
    canvas_draw_str(canvas, 50, 39, temp_str);

    //Print Wavelength of Module
    snprintf(temp_str, 280, "%u nm", model->wavelength);
    canvas_draw_str(canvas, 2, 49, temp_str);

    // These values will be zero if not applicable..
    if(model->sm_reach != 0) {
        snprintf(temp_str, 280, "%u km (SM)", model->sm_reach);
        canvas_draw_str(canvas, 50, 49, temp_str);
    }
    if(model->mm_reach_om3 != 0) {
        snprintf(temp_str, 280, "%u m (MM OM3)", model->mm_reach_om3);
        canvas_draw_str(canvas, 50, 49, temp_str);
    }
}

static bool gpio_i2c_sfp_input_callback(InputEvent* event, void* context) {
    furi_assert(context);
    GpioI2CSfp* gpio_i2c_sfp = context;
    bool consumed = false;

    if(event->key == InputKeyOk) {
        consumed = gpio_i2c_sfp_process_ok(gpio_i2c_sfp, event);
    }

    return consumed;
}

static bool gpio_i2c_sfp_process_ok(GpioI2CSfp* gpio_i2c_sfp, InputEvent* event) {
    bool consumed = false;
    gpio_i2c_sfp->callback(event->type, gpio_i2c_sfp->context);

    return consumed;
}

GpioI2CSfp* gpio_i2c_sfp_alloc() {
    GpioI2CSfp* gpio_i2c_sfp = malloc(sizeof(GpioI2CSfp));

    gpio_i2c_sfp->view = view_alloc();
    view_allocate_model(gpio_i2c_sfp->view, ViewModelTypeLocking, sizeof(GpioI2CSfpModel));
    view_set_context(gpio_i2c_sfp->view, gpio_i2c_sfp);
    view_set_draw_callback(gpio_i2c_sfp->view, gpio_i2c_sfp_draw_callback);
    view_set_input_callback(gpio_i2c_sfp->view, gpio_i2c_sfp_input_callback);

    return gpio_i2c_sfp;
}

void gpio_i2c_sfp_free(GpioI2CSfp* gpio_i2c_sfp) {
    furi_assert(gpio_i2c_sfp);
    view_free(gpio_i2c_sfp->view);
    free(gpio_i2c_sfp);
}

View* gpio_i2c_sfp_get_view(GpioI2CSfp* gpio_i2c_sfp) {
    furi_assert(gpio_i2c_sfp);
    return gpio_i2c_sfp->view;
}

void gpio_i2c_sfp_set_ok_callback(
    GpioI2CSfp* gpio_i2c_sfp,
    GpioI2CSfpOkCallback callback,
    void* context) {
    furi_assert(gpio_i2c_sfp);
    furi_assert(callback);
    with_view_model(
        gpio_i2c_sfp->view,
        GpioI2CSfpModel * model,
        {
            UNUSED(model);
            gpio_i2c_sfp->callback = callback;
            gpio_i2c_sfp->context = context;
        },
        false);
}

void gpio_i2c_sfp_update_state(GpioI2CSfp* instance, I2CSfpState* st) {
    furi_assert(instance);
    furi_assert(st);

    with_view_model(
        instance->view,
        GpioI2CSfpModel * model,
        {
            // Insert values into model...
            strcpy(model->vendor, st->vendor);
            strcpy(model->pn, st->pn);
            strcpy(model->sn, st->sn);
            strcpy(model->rev, st->rev);
            strcpy(model->connector, st->connector);
            model->wavelength = st->wavelength;
            model->sm_reach = st->sm_reach;
            model->mm_reach_om3 = st->mm_reach_om3;
        },
        true);
}
