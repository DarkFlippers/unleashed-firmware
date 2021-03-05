#include "subghz-view-spectrum-settings.h"
#include <callback-connector.h>

struct SpectrumSettingsModel {
    uint32_t start_freq;
};

/***************************************************************************************/

static void draw_callback(Canvas* canvas, void* _model) {
    SpectrumSettingsModel* model = static_cast<SpectrumSettingsModel*>(_model);
    const uint8_t str_size = 64;
    char str_buffer[str_size];

    canvas_clear(canvas);
    snprintf(str_buffer, str_size, "Start freq < %ld > MHz", model->start_freq);
    canvas_draw_str_aligned(canvas, 64, 32, AlignCenter, AlignCenter, str_buffer);
}

static bool input_callback(InputEvent* event, void* context) {
    SubghzViewSpectrumSettings* _this = static_cast<SubghzViewSpectrumSettings*>(context);

    bool consumed = false;

    // Process key presses only
    if(event->type == InputTypeShort) {
        if(event->key == InputKeyOk) {
            _this->call_ok_callback();
            consumed = true;
        } else if(event->key == InputKeyLeft) {
            with_view_model_cpp(_this->get_view(), SpectrumSettingsModel, model, {
                model->start_freq--;
                return true;
            });
            consumed = true;
        } else if(event->key == InputKeyRight) {
            with_view_model_cpp(_this->get_view(), SpectrumSettingsModel, model, {
                model->start_freq++;
                return true;
            });
            consumed = true;
        }
    }

    return consumed;
}

/***************************************************************************************/

View* SubghzViewSpectrumSettings::get_view() {
    return view;
}

void SubghzViewSpectrumSettings::set_ok_callback(OkCallback callback, void* context) {
    ok_callback = callback;
    ok_callback_context = context;
}

void SubghzViewSpectrumSettings::call_ok_callback() {
    if(ok_callback != nullptr) {
        ok_callback(ok_callback_context);
    }
}

void SubghzViewSpectrumSettings::set_start_freq(uint32_t start_freq) {
    with_view_model_cpp(view, SpectrumSettingsModel, model, {
        model->start_freq = start_freq;
        return true;
    });
}

uint32_t SubghzViewSpectrumSettings::get_start_freq() {
    uint32_t result;

    with_view_model_cpp(view, SpectrumSettingsModel, model, {
        result = model->start_freq;
        return false;
    });

    return result;
}

SubghzViewSpectrumSettings::SubghzViewSpectrumSettings() {
    view = view_alloc();
    view_set_context(view, this);
    view_allocate_model(view, ViewModelTypeLocking, sizeof(SpectrumSettingsModel));

    view_set_draw_callback(view, draw_callback);

    view_set_input_callback(view, input_callback);
}

SubghzViewSpectrumSettings::~SubghzViewSpectrumSettings() {
    view_free(view);
}