#include "subghz_save_raw.h"
#include "../subghz_i.h"

#include <math.h>
#include <furi.h>
#include <furi-hal.h>
#include <input/input.h>
#include <gui/elements.h>
#include <lib/subghz/protocols/subghz_protocol_princeton.h>

#include <assets_icons.h>
#define SUBGHZ_SAVE_RAW_RSSI_HISTORY_SIZE 100

typedef enum {
    SubghzSaveRAWStatusStart,
    SubghzSaveRAWStatusIDLE,
    SubghzSaveRAWStatusREC,
    SubghzSaveRAWStatusShowName,
} SubghzSaveRAWStatus;

struct SubghzSaveRAW {
    View* view;
    osTimerId timer;
    SubghzSaveRAWCallback callback;
    void* context;
};

typedef struct {
    string_t frequency_str;
    string_t preset_str;
    string_t sample_write;
    string_t file_name;
    uint8_t* rssi_history;
    bool rssi_history_end;
    uint8_t ind_write;
    SubghzSaveRAWStatus satus;
} SubghzSaveRAWModel;

void subghz_save_raw_set_callback(
    SubghzSaveRAW* subghz_save_raw,
    SubghzSaveRAWCallback callback,
    void* context) {
    furi_assert(subghz_save_raw);
    furi_assert(callback);
    subghz_save_raw->callback = callback;
    subghz_save_raw->context = context;
}

void subghz_save_raw_add_data_statusbar(
    SubghzSaveRAW* instance,
    const char* frequency_str,
    const char* preset_str) {
    furi_assert(instance);
    with_view_model(
        instance->view, (SubghzSaveRAWModel * model) {
            string_set(model->frequency_str, frequency_str);
            string_set(model->preset_str, preset_str);
            return true;
        });
}

void subghz_save_raw_set_file_name(SubghzSaveRAW* instance, const char* file_name) {
    furi_assert(instance);
    with_view_model(
        instance->view, (SubghzSaveRAWModel * model) {
            string_set(model->file_name, file_name);
            return true;
        });
}

static void subghz_save_raw_timer_callback(void* context) {
    furi_assert(context);
    SubghzSaveRAW* instance = context;

    with_view_model(
        instance->view, (SubghzSaveRAWModel * model) {
            model->satus = SubghzSaveRAWStatusIDLE;
            return true;
        });
}

void subghz_save_raw_add_data_rssi(SubghzSaveRAW* instance, float rssi) {
    furi_assert(instance);
    uint8_t u_rssi = 0;

    if(rssi < -90) {
        u_rssi = 0;
    } else {
        u_rssi = (uint8_t)((rssi + 90) / 2.7);
    }
    //if(u_rssi > 34) u_rssi = 34;

    with_view_model(
        instance->view, (SubghzSaveRAWModel * model) {
            model->rssi_history[model->ind_write++] = u_rssi;
            if(model->ind_write > SUBGHZ_SAVE_RAW_RSSI_HISTORY_SIZE) {
                model->rssi_history_end = true;
                model->ind_write = 0;
            }
            return true;
        });
}

void subghz_save_raw_update_sample_write(SubghzSaveRAW* instance, size_t sample) {
    furi_assert(instance);

    with_view_model(
        instance->view, (SubghzSaveRAWModel * model) {
            string_printf(model->sample_write, "%d spl.", sample);
            return false;
        });
}

void subghz_save_raw_draw_rssi(Canvas* canvas, SubghzSaveRAWModel* model) {
    int ind = 0;
    int base = 0;
    if(model->rssi_history_end == false) {
        for(int i = model->ind_write; i >= 0; i--) {
            canvas_draw_line(canvas, i, 47, i, 47 - model->rssi_history[i]);
        }
        if(model->ind_write > 3) {
            canvas_draw_line(canvas, model->ind_write, 47, model->ind_write, 13);
            canvas_draw_line(canvas, model->ind_write - 2, 12, model->ind_write + 2, 12);
            canvas_draw_line(canvas, model->ind_write - 1, 13, model->ind_write + 1, 13);
        }
    } else {
        base = SUBGHZ_SAVE_RAW_RSSI_HISTORY_SIZE - model->ind_write;
        for(int i = SUBGHZ_SAVE_RAW_RSSI_HISTORY_SIZE; i >= 0; i--) {
            ind = i - base;
            if(ind < 0) ind += SUBGHZ_SAVE_RAW_RSSI_HISTORY_SIZE;
            canvas_draw_line(canvas, i, 47, i, 47 - model->rssi_history[ind]);
        }
        canvas_draw_line(
            canvas, SUBGHZ_SAVE_RAW_RSSI_HISTORY_SIZE, 47, SUBGHZ_SAVE_RAW_RSSI_HISTORY_SIZE, 13);
        canvas_draw_line(
            canvas,
            SUBGHZ_SAVE_RAW_RSSI_HISTORY_SIZE - 2,
            12,
            SUBGHZ_SAVE_RAW_RSSI_HISTORY_SIZE + 2,
            12);
        canvas_draw_line(
            canvas,
            SUBGHZ_SAVE_RAW_RSSI_HISTORY_SIZE - 1,
            13,
            SUBGHZ_SAVE_RAW_RSSI_HISTORY_SIZE + 1,
            13);
    }
}

void subghz_save_raw_draw(Canvas* canvas, SubghzSaveRAWModel* model) {
    canvas_set_color(canvas, ColorBlack);
    canvas_set_font(canvas, FontSecondary);
    if(model->satus != SubghzSaveRAWStatusShowName) {
        canvas_draw_str(canvas, 5, 8, string_get_cstr(model->frequency_str));
        canvas_draw_str(canvas, 40, 8, string_get_cstr(model->preset_str));
        canvas_draw_str_aligned(
            canvas, 126, 0, AlignRight, AlignTop, string_get_cstr(model->sample_write));
    } else {
        canvas_draw_str_aligned(
            canvas, 61, 1, AlignRight, AlignTop, string_get_cstr(model->file_name));
        canvas_draw_str(canvas, 65, 8, "Saved!");
    }

    canvas_draw_line(canvas, 0, 14, 115, 14);
    subghz_save_raw_draw_rssi(canvas, model);
    canvas_draw_line(canvas, 0, 48, 115, 48);
    canvas_draw_line(canvas, 115, 14, 115, 48);

    if(model->satus == SubghzSaveRAWStatusIDLE) {
        elements_button_left(canvas, "Config");
        elements_button_center(canvas, "REC");
        elements_button_right(canvas, "More");
    } else if(model->satus == SubghzSaveRAWStatusStart) {
        elements_button_left(canvas, "Config");
        elements_button_center(canvas, "REC");
    } else {
        elements_button_center(canvas, "Stop");
    }

    canvas_set_font_direction(canvas, 3);
    canvas_draw_str(canvas, 126, 40, "RSSI");
    canvas_set_font_direction(canvas, 0);
}

bool subghz_save_raw_input(InputEvent* event, void* context) {
    furi_assert(context);
    SubghzSaveRAW* instance = context;

    if(event->key == InputKeyBack && event->type == InputTypeShort) {
        instance->callback(SubghzCustomEventViewSaveRAWBack, instance->context);
    } else if(event->key == InputKeyLeft && event->type == InputTypeShort) {
        with_view_model(
            instance->view, (SubghzSaveRAWModel * model) {
                if(model->satus == SubghzSaveRAWStatusIDLE ||
                   model->satus == SubghzSaveRAWStatusStart) {
                    instance->callback(SubghzCustomEventViewSaveRAWConfig, instance->context);
                }
                return true;
            });
    } else if(event->key == InputKeyRight && event->type == InputTypeShort) {
        with_view_model(
            instance->view, (SubghzSaveRAWModel * model) {
                if(model->satus == SubghzSaveRAWStatusIDLE) {
                    instance->callback(SubghzCustomEventViewSaveRAWMore, instance->context);
                }
                return true;
            });
    } else if(event->key == InputKeyOk && event->type == InputTypeShort) {
        with_view_model(
            instance->view, (SubghzSaveRAWModel * model) {
                if(model->satus == SubghzSaveRAWStatusIDLE ||
                   model->satus == SubghzSaveRAWStatusStart) {
                    instance->callback(SubghzCustomEventViewSaveRAWREC, instance->context);
                    model->satus = SubghzSaveRAWStatusREC;
                    model->ind_write = 0;
                    model->rssi_history_end = false;
                } else {
                    instance->callback(SubghzCustomEventViewSaveRAWIDLE, instance->context);
                    model->satus = SubghzSaveRAWStatusShowName;
                    osTimerStart(instance->timer, 1024);
                }
                return true;
            });
    }

    if(event->key == InputKeyBack) {
        return false;
    }

    return true;
}

void subghz_save_raw_enter(void* context) {
    furi_assert(context);
    SubghzSaveRAW* instance = context;

    with_view_model(
        instance->view, (SubghzSaveRAWModel * model) {
            model->satus = SubghzSaveRAWStatusStart;
            model->rssi_history = furi_alloc(SUBGHZ_SAVE_RAW_RSSI_HISTORY_SIZE * sizeof(uint8_t));
            model->rssi_history_end = false;
            model->ind_write = 0;
            string_set(model->sample_write, "0 spl.");
            return true;
        });
}

void subghz_save_raw_exit(void* context) {
    furi_assert(context);
    SubghzSaveRAW* instance = context;

    with_view_model(
        instance->view, (SubghzSaveRAWModel * model) {
            if(model->satus != SubghzSaveRAWStatusIDLE &&
               model->satus != SubghzSaveRAWStatusStart) {
                instance->callback(SubghzCustomEventViewSaveRAWIDLE, instance->context);
                model->satus = SubghzSaveRAWStatusStart;
            }
            string_clean(model->frequency_str);
            string_clean(model->preset_str);
            string_clean(model->sample_write);
            string_clean(model->file_name);
            free(model->rssi_history);
            return true;
        });
}

SubghzSaveRAW* subghz_save_raw_alloc() {
    SubghzSaveRAW* instance = furi_alloc(sizeof(SubghzSaveRAW));

    // View allocation and configuration
    instance->view = view_alloc();
    view_allocate_model(instance->view, ViewModelTypeLocking, sizeof(SubghzSaveRAWModel));
    view_set_context(instance->view, instance);
    view_set_draw_callback(instance->view, (ViewDrawCallback)subghz_save_raw_draw);
    view_set_input_callback(instance->view, subghz_save_raw_input);
    view_set_enter_callback(instance->view, subghz_save_raw_enter);
    view_set_exit_callback(instance->view, subghz_save_raw_exit);

    instance->timer = osTimerNew(subghz_save_raw_timer_callback, osTimerOnce, instance, NULL);

    with_view_model(
        instance->view, (SubghzSaveRAWModel * model) {
            string_init(model->frequency_str);
            string_init(model->preset_str);
            string_init(model->sample_write);
            string_init(model->file_name);
            return true;
        });

    return instance;
}

void subghz_save_raw_free(SubghzSaveRAW* instance) {
    furi_assert(instance);

    with_view_model(
        instance->view, (SubghzSaveRAWModel * model) {
            string_clear(model->frequency_str);
            string_clear(model->preset_str);
            string_clear(model->sample_write);
            string_clear(model->file_name);
            return true;
        });
    osTimerDelete(instance->timer);
    view_free(instance->view);
    free(instance);
}

View* subghz_save_raw_get_view(SubghzSaveRAW* instance) {
    furi_assert(instance);
    return instance->view;
}