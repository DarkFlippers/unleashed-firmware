#include "main_view.h"
#include <math.h>
#include <furi.h>
#include <furi_hal.h>
#include <gui/elements.h>
#include "../../lightmeter.h"
#include "../../lightmeter_helper.h"

#define WORKER_TAG "Main View"

static const int iso_numbers[] = {
    [ISO_6] = 6,
    [ISO_12] = 12,
    [ISO_25] = 25,
    [ISO_50] = 50,
    [ISO_100] = 100,
    [ISO_200] = 200,
    [ISO_400] = 400,
    [ISO_800] = 800,
    [ISO_1600] = 1600,
    [ISO_3200] = 3200,
    [ISO_6400] = 6400,
    [ISO_12800] = 12800,
    [ISO_25600] = 25600,
    [ISO_51200] = 51200,
    [ISO_102400] = 102400,
};

static const int nd_numbers[] = {
    [ND_0] = 0,
    [ND_2] = 2,
    [ND_4] = 4,
    [ND_8] = 8,
    [ND_16] = 16,
    [ND_32] = 32,
    [ND_64] = 64,
    [ND_128] = 128,
    [ND_256] = 256,
    [ND_512] = 512,
    [ND_1024] = 1024,
    [ND_2048] = 2048,
    [ND_4096] = 4096,
};

const float aperture_numbers[] = {
    [AP_1] = 1.0,
    [AP_1_4] = 1.4,
    [AP_2] = 2.0,
    [AP_2_8] = 2.8,
    [AP_4] = 4.0,
    [AP_5_6] = 5.6,
    [AP_8] = 8,
    [AP_11] = 11,
    [AP_16] = 16,
    [AP_22] = 22,
    [AP_32] = 32,
    [AP_45] = 45,
    [AP_64] = 64,
    [AP_90] = 90,
    [AP_128] = 128,
};

const float speed_numbers[] = {
    [SPEED_8000] = 1.0 / 8000, [SPEED_4000] = 1.0 / 4000, [SPEED_2000] = 1.0 / 2000,
    [SPEED_1000] = 1.0 / 1000, [SPEED_500] = 1.0 / 500,   [SPEED_250] = 1.0 / 250,
    [SPEED_125] = 1.0 / 125,   [SPEED_60] = 1.0 / 60,     [SPEED_48] = 1.0 / 48,
    [SPEED_30] = 1.0 / 30,     [SPEED_15] = 1.0 / 15,     [SPEED_8] = 1.0 / 8,
    [SPEED_4] = 1.0 / 4,       [SPEED_2] = 1.0 / 2,       [SPEED_1S] = 1.0,
    [SPEED_2S] = 2.0,          [SPEED_4S] = 4.0,          [SPEED_8S] = 8.0,
    [SPEED_15S] = 15.0,        [SPEED_30S] = 30.0,
};

struct MainView {
    View* view;
    LightMeterMainViewButtonCallback cb_left;
    LightMeterMainViewButtonCallback cb_right;
    void* cb_context;
};

void lightmeter_main_view_set_left_callback(
    MainView* lightmeter_main_view,
    LightMeterMainViewButtonCallback callback,
    void* context) {
    with_view_model(
        lightmeter_main_view->view,
        MainViewModel * model,
        {
            UNUSED(model);
            lightmeter_main_view->cb_left = callback;
            lightmeter_main_view->cb_context = context;
        },
        true);
}

void lightmeter_main_view_set_right_callback(
    MainView* lightmeter_main_view,
    LightMeterMainViewButtonCallback callback,
    void* context) {
    with_view_model(
        lightmeter_main_view->view,
        MainViewModel * model,
        {
            UNUSED(model);
            lightmeter_main_view->cb_right = callback;
            lightmeter_main_view->cb_context = context;
        },
        true);
}

static void main_view_draw_callback(Canvas* canvas, void* context) {
    furi_assert(context);
    MainViewModel* model = context;

    canvas_clear(canvas);

    // draw button
    canvas_set_font(canvas, FontSecondary);
    elements_button_left(canvas, "Config");

    if(!model->lux_only) {
        // top row
        draw_top_row(canvas, model);

        // add f, T values
        canvas_set_font(canvas, FontBigNumbers);

        // draw f icon and number
        canvas_draw_icon(canvas, 15, 17, &I_f_10x14);
        draw_aperture(canvas, model);

        // draw T icon and number
        canvas_draw_icon(canvas, 15, 34, &I_T_10x14);
        draw_speed(canvas, model);

        // draw ND number
        draw_nd_number(canvas, model);

        // draw EV number
        canvas_set_font(canvas, FontSecondary);
        draw_EV_number(canvas, model);

        // draw mode indicator
        draw_mode_indicator(canvas, model);
    } else {
        elements_button_right(canvas, "Reset");
        draw_lux_only_mode(canvas, model);
    }
}

static void main_view_process(MainView* main_view, InputEvent* event) {
    with_view_model(
        main_view->view,
        MainViewModel * model,
        {
            if(event->type == InputTypePress) {
                if(event->key == InputKeyUp) {
                    switch(model->current_mode) {
                    case FIXED_APERTURE:
                        if(model->aperture < AP_NUM - 1) model->aperture++;
                        break;

                    case FIXED_SPEED:
                        if(model->speed < SPEED_NUM - 1) model->speed++;
                        break;

                    default:
                        break;
                    }
                } else if(event->key == InputKeyDown) {
                    switch(model->current_mode) {
                    case FIXED_APERTURE:
                        if(model->aperture > 0) model->aperture--;
                        break;

                    case FIXED_SPEED:
                        if(model->speed > 0) model->speed--;
                        break;

                    default:
                        break;
                    }
                } else if(event->key == InputKeyOk) {
                    switch(model->current_mode) {
                    case FIXED_SPEED:
                        model->current_mode = FIXED_APERTURE;
                        break;

                    case FIXED_APERTURE:
                        model->current_mode = FIXED_SPEED;
                        break;

                    default:
                        break;
                    }
                }
            }
        },
        true);
}

static bool main_view_input_callback(InputEvent* event, void* context) {
    furi_assert(context);
    MainView* main_view = context;
    bool consumed = false;

    if(event->type == InputTypeShort && event->key == InputKeyLeft) {
        if(main_view->cb_left) {
            main_view->cb_left(main_view->cb_context);
        }
        consumed = true;
    } else if(event->type == InputTypeShort && event->key == InputKeyRight) {
        if(main_view->cb_right) {
            main_view->cb_right(main_view->cb_context);
        }
        consumed = true;
    } else if(event->type == InputTypeShort && event->key == InputKeyBack) {
    } else {
        main_view_process(main_view, event);
        consumed = true;
    }

    return consumed;
}

MainView* main_view_alloc() {
    MainView* main_view = malloc(sizeof(MainView));
    main_view->view = view_alloc();
    view_set_context(main_view->view, main_view);
    view_allocate_model(main_view->view, ViewModelTypeLocking, sizeof(MainViewModel));
    view_set_draw_callback(main_view->view, main_view_draw_callback);
    view_set_input_callback(main_view->view, main_view_input_callback);

    return main_view;
}

void main_view_free(MainView* main_view) {
    furi_assert(main_view);
    view_free(main_view->view);
    free(main_view);
}

View* main_view_get_view(MainView* main_view) {
    furi_assert(main_view);
    return main_view->view;
}

void main_view_set_lux(MainView* main_view, float val) {
    furi_assert(main_view);
    with_view_model(
        main_view->view,
        MainViewModel * model,
        {
            model->lux = val;
            model->peakLux = fmax(model->peakLux, val);

            model->luxHistogram[model->luxHistogramIndex++] = val;
            model->luxHistogramIndex %= LUX_HISTORGRAM_LENGTH;
        },
        true);
}

void main_view_reset_lux(MainView* main_view) {
    furi_assert(main_view);
    with_view_model(
        main_view->view, MainViewModel * model, { model->peakLux = 0; }, true);
}

void main_view_set_EV(MainView* main_view, float val) {
    furi_assert(main_view);
    with_view_model(
        main_view->view, MainViewModel * model, { model->EV = val; }, true);
}

void main_view_set_response(MainView* main_view, bool val) {
    furi_assert(main_view);
    with_view_model(
        main_view->view, MainViewModel * model, { model->response = val; }, true);
}

void main_view_set_iso(MainView* main_view, int iso) {
    furi_assert(main_view);
    with_view_model(
        main_view->view, MainViewModel * model, { model->iso = iso; }, true);
}

void main_view_set_nd(MainView* main_view, int nd) {
    furi_assert(main_view);
    with_view_model(
        main_view->view, MainViewModel * model, { model->nd = nd; }, true);
}

void main_view_set_aperture(MainView* main_view, int aperture) {
    furi_assert(main_view);
    with_view_model(
        main_view->view, MainViewModel * model, { model->aperture = aperture; }, true);
}

void main_view_set_speed(MainView* main_view, int speed) {
    furi_assert(main_view);
    with_view_model(
        main_view->view, MainViewModel * model, { model->speed = speed; }, true);
}

void main_view_set_dome(MainView* main_view, bool dome) {
    furi_assert(main_view);
    with_view_model(
        main_view->view, MainViewModel * model, { model->dome = dome; }, true);
}

void main_view_set_lux_only(MainView* main_view, bool lux_only) {
    furi_assert(main_view);
    with_view_model(
        main_view->view, MainViewModel * model, { model->lux_only = lux_only; }, true);
}

void main_view_set_measurement_resolution(MainView* main_view, int measurement_resolution) {
    furi_assert(main_view);
    with_view_model(
        main_view->view,
        MainViewModel * model,
        { model->measurement_resolution = measurement_resolution; },
        true);
}

void main_view_set_device_addr(MainView* main_view, int device_addr) {
    furi_assert(main_view);
    with_view_model(
        main_view->view, MainViewModel * model, { model->device_addr = device_addr; }, true);
}

void main_view_set_sensor_type(MainView* main_view, int sensor_type) {
    furi_assert(main_view);
    with_view_model(
        main_view->view, MainViewModel * model, { model->sensor_type = sensor_type; }, true);
}

bool main_view_get_dome(MainView* main_view) {
    furi_assert(main_view);
    bool val = false;
    with_view_model(
        main_view->view, MainViewModel * model, { val = model->dome; }, true);
    return val;
}

void draw_top_row(Canvas* canvas, MainViewModel* context) {
    MainViewModel* model = context;

    char str[12];

    if(!model->response) {
        canvas_draw_box(canvas, 0, 0, 128, 12);
        canvas_set_color(canvas, ColorWhite);
        canvas_set_font(canvas, FontPrimary);
        canvas_draw_str(canvas, 24, 10, "No sensor found");
        canvas_set_color(canvas, ColorBlack);
    } else {
        model->iso_val = iso_numbers[model->iso];
        if(model->nd > 0) model->iso_val /= nd_numbers[model->nd];

        if(model->lux > 0) {
            if(model->current_mode == FIXED_APERTURE) {
                model->speed_val = 100 * pow(aperture_numbers[model->aperture], 2) /
                                   (double)model->iso_val / pow(2, model->EV);
            } else {
                model->aperture_val = sqrt(
                    pow(2, model->EV) * (double)model->iso_val *
                    (double)speed_numbers[model->speed] / 100);
            }
        }

        // TODO when T:30, f/0 instead of f/128

        canvas_draw_line(canvas, 0, 10, 128, 10);

        canvas_set_font(canvas, FontPrimary);
        // metering mode A – ambient, F – flash
        // canvas_draw_str_aligned(canvas, 1, 1, AlignLeft, AlignTop, "A");

        snprintf(str, sizeof(str), "ISO: %d", iso_numbers[model->iso]);
        canvas_draw_str_aligned(canvas, 19, 1, AlignLeft, AlignTop, str);

        canvas_set_font(canvas, FontSecondary);
        snprintf(str, sizeof(str), "lx: %.0f", (double)model->lux);
        canvas_draw_str_aligned(canvas, 87, 2, AlignLeft, AlignTop, str);
    }
}

void draw_aperture(Canvas* canvas, MainViewModel* context) {
    MainViewModel* model = context;

    char str[12];

    switch(model->current_mode) {
    case FIXED_APERTURE:
        if(model->response) {
            if(model->aperture < AP_8) {
                snprintf(str, sizeof(str), "/%.1f", (double)aperture_numbers[model->aperture]);
            } else {
                snprintf(str, sizeof(str), "/%.0f", (double)aperture_numbers[model->aperture]);
            }
        } else {
            snprintf(str, sizeof(str), " ---");
        }
        canvas_draw_str_aligned(canvas, 27, 15, AlignLeft, AlignTop, str);
        break;
    case FIXED_SPEED:
        if(model->aperture_val < aperture_numbers[0] || !model->response) {
            snprintf(str, sizeof(str), " ---");
        } else if(model->aperture_val < aperture_numbers[AP_8]) {
            snprintf(str, sizeof(str), "/%.1f", (double)normalizeAperture(model->aperture_val));
        } else {
            snprintf(str, sizeof(str), "/%.0f", (double)normalizeAperture(model->aperture_val));
        }
        canvas_draw_str_aligned(canvas, 27, 15, AlignLeft, AlignTop, str);
        break;
    default:
        break;
    }
}

void draw_speed(Canvas* canvas, MainViewModel* context) {
    MainViewModel* model = context;

    char str[12];

    switch(model->current_mode) {
    case FIXED_APERTURE:
        if(model->lux > 0 && model->response) {
            if(model->speed_val < 1 && model->speed_val > 0) {
                snprintf(str, sizeof(str), ":1/%.0f", 1 / (double)normalizeTime(model->speed_val));
            } else {
                snprintf(str, sizeof(str), ":%.0f", (double)normalizeTime(model->speed_val));
            }
        } else {
            snprintf(str, sizeof(str), " ---");
        }
        canvas_draw_str_aligned(canvas, 27, 34, AlignLeft, AlignTop, str);
        break;

    case FIXED_SPEED:
        if(model->response) {
            if(model->speed < SPEED_1S) {
                snprintf(str, sizeof(str), ":1/%.0f", 1 / (double)speed_numbers[model->speed]);
            } else {
                snprintf(str, sizeof(str), ":%.0f", (double)speed_numbers[model->speed]);
            }
        } else {
            snprintf(str, sizeof(str), " ---");
        }
        canvas_draw_str_aligned(canvas, 27, 34, AlignLeft, AlignTop, str);
        break;

    default:
        break;
    }
}

void draw_mode_indicator(Canvas* canvas, MainViewModel* context) {
    MainViewModel* model = context;

    switch(model->current_mode) {
    case FIXED_SPEED:
        canvas_set_font(canvas, FontBigNumbers);
        canvas_draw_str_aligned(canvas, 3, 36, AlignLeft, AlignTop, "*");
        break;

    case FIXED_APERTURE:
        canvas_set_font(canvas, FontBigNumbers);
        canvas_draw_str_aligned(canvas, 3, 17, AlignLeft, AlignTop, "*");
        break;

    default:
        break;
    }
}

void draw_nd_number(Canvas* canvas, MainViewModel* context) {
    MainViewModel* model = context;

    char str[9];

    canvas_set_font(canvas, FontSecondary);

    if(model->response) {
        snprintf(str, sizeof(str), "ND: %d", nd_numbers[model->nd]);
    } else {
        snprintf(str, sizeof(str), "ND: ---");
    }
    canvas_draw_str_aligned(canvas, 87, 20, AlignLeft, AlignBottom, str);
}

void draw_EV_number(Canvas* canvas, MainViewModel* context) {
    MainViewModel* model = context;

    char str[7];

    if(model->lux > 0 && model->response) {
        snprintf(str, sizeof(str), "EV: %1.0f", (double)model->EV);
        canvas_draw_str_aligned(canvas, 87, 29, AlignLeft, AlignBottom, str);
    } else {
        canvas_draw_str_aligned(canvas, 87, 29, AlignLeft, AlignBottom, "EV: --");
    }
}

void draw_lux_only_mode(Canvas* canvas, MainViewModel* context) {
    MainViewModel* model = context;

    if(!model->response) {
        canvas_draw_box(canvas, 0, 0, 128, 12);
        canvas_set_color(canvas, ColorWhite);
        canvas_set_font(canvas, FontPrimary);
        canvas_draw_str(canvas, 24, 10, "No sensor found");
        canvas_set_color(canvas, ColorBlack);
    } else {
        char str[12];

        canvas_set_font(canvas, FontPrimary);

        canvas_draw_line(canvas, 0, 10, 128, 10);
        canvas_draw_str_aligned(canvas, 64, 1, AlignCenter, AlignTop, "Lux meter mode");

        canvas_set_font(canvas, FontBigNumbers);
        snprintf(str, sizeof(str), "%.0f", (double)model->lux);
        canvas_draw_str_aligned(canvas, 80, 22, AlignRight, AlignCenter, str);

        canvas_set_font(canvas, FontSecondary);
        canvas_draw_str_aligned(canvas, 85, 29, AlignLeft, AlignBottom, "Lux now");

        canvas_set_font(canvas, FontPrimary);
        snprintf(str, sizeof(str), "%.0f", (double)model->peakLux);
        canvas_draw_str_aligned(canvas, 80, 39, AlignRight, AlignCenter, str);

        canvas_set_font(canvas, FontSecondary);
        canvas_draw_str_aligned(canvas, 85, 43, AlignLeft, AlignBottom, "Lux peak");

        for(int i = 0; i < LUX_HISTORGRAM_LENGTH; i++) {
            float lux =
                model->luxHistogram[(i + model->luxHistogramIndex) % LUX_HISTORGRAM_LENGTH];
            int barHeight = log10(lux) / log10(LUX_HISTORGRAM_LOGBASE);
            canvas_draw_line(
                canvas,
                LUX_HISTORGRAM_LEFT + i,
                LUX_HISTORGRAM_BOTTOM,
                LUX_HISTORGRAM_LEFT + i,
                LUX_HISTORGRAM_BOTTOM - barHeight);
        }
    }
}
