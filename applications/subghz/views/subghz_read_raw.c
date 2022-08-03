#include "subghz_read_raw.h"
#include "../subghz_i.h"

#include <math.h>
#include <furi.h>
#include <furi_hal.h>
#include <input/input.h>
#include <gui/elements.h>

#include <assets_icons.h>
#define SUBGHZ_READ_RAW_RSSI_HISTORY_SIZE 100
#define TAG "SubGhzReadRAW"

struct SubGhzReadRAW {
    View* view;
    SubGhzReadRAWCallback callback;
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
    uint8_t ind_sin;
    SubGhzReadRAWStatus status;
} SubGhzReadRAWModel;

void subghz_read_raw_set_callback(
    SubGhzReadRAW* subghz_read_raw,
    SubGhzReadRAWCallback callback,
    void* context) {
    furi_assert(subghz_read_raw);
    furi_assert(callback);
    subghz_read_raw->callback = callback;
    subghz_read_raw->context = context;
}

void subghz_read_raw_add_data_statusbar(
    SubGhzReadRAW* instance,
    const char* frequency_str,
    const char* preset_str) {
    furi_assert(instance);
    with_view_model(
        instance->view, (SubGhzReadRAWModel * model) {
            string_set_str(model->frequency_str, frequency_str);
            string_set_str(model->preset_str, preset_str);
            return true;
        });
}

void subghz_read_raw_add_data_rssi(SubGhzReadRAW* instance, float rssi) {
    furi_assert(instance);
    uint8_t u_rssi = 0;

    if(rssi < -90) {
        u_rssi = 0;
    } else {
        u_rssi = (uint8_t)((rssi + 90) / 2.7);
    }

    with_view_model(
        instance->view, (SubGhzReadRAWModel * model) {
            model->rssi_history[model->ind_write++] = u_rssi;
            if(model->ind_write > SUBGHZ_READ_RAW_RSSI_HISTORY_SIZE) {
                model->rssi_history_end = true;
                model->ind_write = 0;
            }
            return true;
        });
}

void subghz_read_raw_update_sample_write(SubGhzReadRAW* instance, size_t sample) {
    furi_assert(instance);

    with_view_model(
        instance->view, (SubGhzReadRAWModel * model) {
            string_printf(model->sample_write, "%d spl.", sample);
            return false;
        });
}

void subghz_read_raw_stop_send(SubGhzReadRAW* instance) {
    furi_assert(instance);

    with_view_model(
        instance->view, (SubGhzReadRAWModel * model) {
            switch(model->status) {
            case SubGhzReadRAWStatusTXRepeat:
            case SubGhzReadRAWStatusLoadKeyTXRepeat:
                instance->callback(SubGhzCustomEventViewReadRAWSendStart, instance->context);
                break;
            case SubGhzReadRAWStatusTX:
                model->status = SubGhzReadRAWStatusIDLE;
                break;
            case SubGhzReadRAWStatusLoadKeyTX:
                model->status = SubGhzReadRAWStatusLoadKeyIDLE;
                break;

            default:
                FURI_LOG_W(TAG, "unknown status");
                model->status = SubGhzReadRAWStatusIDLE;
                break;
            }
            return true;
        });
}

void subghz_read_raw_update_sin(SubGhzReadRAW* instance) {
    furi_assert(instance);
    with_view_model(
        instance->view, (SubGhzReadRAWModel * model) {
            if(model->ind_sin++ > 62) {
                model->ind_sin = 0;
            }
            return true;
        });
}

static int8_t subghz_read_raw_tab_sin(uint8_t x) {
    const uint8_t tab_sin[64] = {0,   3,   6,   9,   12,  16,  19,  22,  25,  28,  31,  34,  37,
                                 40,  43,  46,  49,  51,  54,  57,  60,  63,  65,  68,  71,  73,
                                 76,  78,  81,  83,  85,  88,  90,  92,  94,  96,  98,  100, 102,
                                 104, 106, 107, 109, 111, 112, 113, 115, 116, 117, 118, 120, 121,
                                 122, 122, 123, 124, 125, 125, 126, 126, 126, 127, 127, 127};

    int8_t r = tab_sin[((x & 0x40) ? -x - 1 : x) & 0x3f];
    if(x & 0x80) return -r;
    return r;
}

void subghz_read_raw_draw_sin(Canvas* canvas, SubGhzReadRAWModel* model) {
#define SUBGHZ_RAW_SIN_AMPLITUDE 11
    for(int i = 113; i > 0; i--) {
        canvas_draw_line(
            canvas,
            i,
            32 - subghz_read_raw_tab_sin(i + model->ind_sin * 16) / SUBGHZ_RAW_SIN_AMPLITUDE,
            i + 1,
            32 + subghz_read_raw_tab_sin((i + model->ind_sin * 16 + 1) * 2) /
                     SUBGHZ_RAW_SIN_AMPLITUDE);
        canvas_draw_line(
            canvas,
            i + 1,
            32 - subghz_read_raw_tab_sin((i + model->ind_sin * 16)) / SUBGHZ_RAW_SIN_AMPLITUDE,
            i + 2,
            32 + subghz_read_raw_tab_sin((i + model->ind_sin * 16 + 1) * 2) /
                     SUBGHZ_RAW_SIN_AMPLITUDE);
    }
}

void subghz_read_raw_draw_scale(Canvas* canvas, SubGhzReadRAWModel* model) {
#define SUBGHZ_RAW_TOP_SCALE 14
#define SUBGHZ_RAW_END_SCALE 115

    if(model->rssi_history_end == false) {
        for(int i = SUBGHZ_RAW_END_SCALE; i > 0; i -= 15) {
            canvas_draw_line(canvas, i, SUBGHZ_RAW_TOP_SCALE, i, SUBGHZ_RAW_TOP_SCALE + 4);
            canvas_draw_line(canvas, i - 5, SUBGHZ_RAW_TOP_SCALE, i - 5, SUBGHZ_RAW_TOP_SCALE + 2);
            canvas_draw_line(
                canvas, i - 10, SUBGHZ_RAW_TOP_SCALE, i - 10, SUBGHZ_RAW_TOP_SCALE + 2);
        }
    } else {
        for(int i = SUBGHZ_RAW_END_SCALE - model->ind_write % 15; i > -15; i -= 15) {
            canvas_draw_line(canvas, i, SUBGHZ_RAW_TOP_SCALE, i, SUBGHZ_RAW_TOP_SCALE + 4);
            if(SUBGHZ_RAW_END_SCALE > i + 5)
                canvas_draw_line(
                    canvas, i + 5, SUBGHZ_RAW_TOP_SCALE, i + 5, SUBGHZ_RAW_TOP_SCALE + 2);
            if(SUBGHZ_RAW_END_SCALE > i + 10)
                canvas_draw_line(
                    canvas, i + 10, SUBGHZ_RAW_TOP_SCALE, i + 10, SUBGHZ_RAW_TOP_SCALE + 2);
        }
    }
}

void subghz_read_raw_draw_rssi(Canvas* canvas, SubGhzReadRAWModel* model) {
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
        base = SUBGHZ_READ_RAW_RSSI_HISTORY_SIZE - model->ind_write;
        for(int i = SUBGHZ_READ_RAW_RSSI_HISTORY_SIZE; i >= 0; i--) {
            ind = i - base;
            if(ind < 0) ind += SUBGHZ_READ_RAW_RSSI_HISTORY_SIZE;
            canvas_draw_line(canvas, i, 47, i, 47 - model->rssi_history[ind]);
        }
        canvas_draw_line(
            canvas, SUBGHZ_READ_RAW_RSSI_HISTORY_SIZE, 47, SUBGHZ_READ_RAW_RSSI_HISTORY_SIZE, 13);
        canvas_draw_line(
            canvas,
            SUBGHZ_READ_RAW_RSSI_HISTORY_SIZE - 2,
            12,
            SUBGHZ_READ_RAW_RSSI_HISTORY_SIZE + 2,
            12);
        canvas_draw_line(
            canvas,
            SUBGHZ_READ_RAW_RSSI_HISTORY_SIZE - 1,
            13,
            SUBGHZ_READ_RAW_RSSI_HISTORY_SIZE + 1,
            13);
    }
}

void subghz_read_raw_draw(Canvas* canvas, SubGhzReadRAWModel* model) {
    uint8_t graphics_mode = 1;
    canvas_set_color(canvas, ColorBlack);
    canvas_set_font(canvas, FontSecondary);
    canvas_draw_str(canvas, 5, 7, string_get_cstr(model->frequency_str));
    canvas_draw_str(canvas, 40, 7, string_get_cstr(model->preset_str));
    canvas_draw_str_aligned(
        canvas, 126, 0, AlignRight, AlignTop, string_get_cstr(model->sample_write));

    canvas_draw_line(canvas, 0, 14, 115, 14);
    canvas_draw_line(canvas, 0, 48, 115, 48);
    canvas_draw_line(canvas, 115, 14, 115, 48);

    switch(model->status) {
    case SubGhzReadRAWStatusIDLE:
        elements_button_left(canvas, "Erase");
        elements_button_center(canvas, "Send");
        elements_button_right(canvas, "Save");
        break;
    case SubGhzReadRAWStatusLoadKeyIDLE:
        elements_button_left(canvas, "New");
        elements_button_center(canvas, "Send");
        elements_button_right(canvas, "More");
        elements_text_box(
            canvas,
            4,
            20,
            110,
            30,
            AlignCenter,
            AlignCenter,
            string_get_cstr(model->file_name),
            true);
        break;

    case SubGhzReadRAWStatusTX:
    case SubGhzReadRAWStatusTXRepeat:
    case SubGhzReadRAWStatusLoadKeyTX:
    case SubGhzReadRAWStatusLoadKeyTXRepeat:
        graphics_mode = 0;
        elements_button_center(canvas, "Send");
        break;

    case SubGhzReadRAWStatusStart:
        elements_button_left(canvas, "Config");
        elements_button_center(canvas, "REC");
        break;

    default:
        elements_button_center(canvas, "Stop");
        break;
    }

    if(graphics_mode == 0) {
        subghz_read_raw_draw_sin(canvas, model);
    } else {
        subghz_read_raw_draw_rssi(canvas, model);
        subghz_read_raw_draw_scale(canvas, model);
        canvas_set_font_direction(canvas, CanvasDirectionBottomToTop);
        canvas_draw_str(canvas, 126, 40, "RSSI");
        canvas_set_font_direction(canvas, CanvasDirectionLeftToRight);
    }
}

bool subghz_read_raw_input(InputEvent* event, void* context) {
    furi_assert(context);
    SubGhzReadRAW* instance = context;

    if((event->key == InputKeyOk) &&
       (event->type == InputTypeLong || event->type == InputTypeRepeat)) {
        //we check that if we hold the transfer button,
        //further check of events is not needed, we exit
        return false;
    } else if(event->key == InputKeyOk && event->type == InputTypePress) {
        with_view_model(
            instance->view, (SubGhzReadRAWModel * model) {
                uint8_t ret = false;
                switch(model->status) {
                case SubGhzReadRAWStatusIDLE:
                    // Start TX
                    instance->callback(SubGhzCustomEventViewReadRAWSendStart, instance->context);
                    model->status = SubGhzReadRAWStatusTXRepeat;
                    ret = true;
                    break;
                case SubGhzReadRAWStatusTX:
                    // Start TXRepeat
                    model->status = SubGhzReadRAWStatusTXRepeat;
                    break;
                case SubGhzReadRAWStatusLoadKeyIDLE:
                    // Start Load Key TX
                    instance->callback(SubGhzCustomEventViewReadRAWSendStart, instance->context);
                    model->status = SubGhzReadRAWStatusLoadKeyTXRepeat;
                    ret = true;
                    break;
                case SubGhzReadRAWStatusLoadKeyTX:
                    // Start Load Key TXRepeat
                    model->status = SubGhzReadRAWStatusLoadKeyTXRepeat;
                    break;

                default:
                    break;
                }
                return ret;
            });
    } else if(event->key == InputKeyOk && event->type == InputTypeRelease) {
        with_view_model(
            instance->view, (SubGhzReadRAWModel * model) {
                if(model->status == SubGhzReadRAWStatusTXRepeat) {
                    // Stop repeat TX
                    model->status = SubGhzReadRAWStatusTX;
                } else if(model->status == SubGhzReadRAWStatusLoadKeyTXRepeat) {
                    // Stop repeat TX
                    model->status = SubGhzReadRAWStatusLoadKeyTX;
                }
                return false;
            });
    } else if(event->key == InputKeyBack && event->type == InputTypeShort) {
        with_view_model(
            instance->view, (SubGhzReadRAWModel * model) {
                switch(model->status) {
                case SubGhzReadRAWStatusREC:
                    //Stop REC
                    instance->callback(SubGhzCustomEventViewReadRAWIDLE, instance->context);
                    model->status = SubGhzReadRAWStatusIDLE;
                    break;
                case SubGhzReadRAWStatusLoadKeyTX:
                    //Stop TxRx
                    instance->callback(SubGhzCustomEventViewReadRAWTXRXStop, instance->context);
                    model->status = SubGhzReadRAWStatusLoadKeyIDLE;
                    break;
                case SubGhzReadRAWStatusTX:
                    //Stop TxRx
                    instance->callback(SubGhzCustomEventViewReadRAWTXRXStop, instance->context);
                    model->status = SubGhzReadRAWStatusIDLE;
                    break;
                case SubGhzReadRAWStatusLoadKeyIDLE:
                    //Exit
                    instance->callback(SubGhzCustomEventViewReadRAWBack, instance->context);
                    break;

                default:
                    //Exit
                    instance->callback(SubGhzCustomEventViewReadRAWBack, instance->context);
                    break;
                }
                return true;
            });
    } else if(event->key == InputKeyLeft && event->type == InputTypeShort) {
        with_view_model(
            instance->view, (SubGhzReadRAWModel * model) {
                if(model->status == SubGhzReadRAWStatusStart) {
                    //Config
                    instance->callback(SubGhzCustomEventViewReadRAWConfig, instance->context);
                } else if(
                    (model->status == SubGhzReadRAWStatusIDLE) ||
                    (model->status == SubGhzReadRAWStatusLoadKeyIDLE)) {
                    //Erase
                    model->status = SubGhzReadRAWStatusStart;
                    model->rssi_history_end = false;
                    model->ind_write = 0;
                    string_set_str(model->sample_write, "0 spl.");
                    string_reset(model->file_name);
                    instance->callback(SubGhzCustomEventViewReadRAWErase, instance->context);
                }
                return true;
            });
    } else if(event->key == InputKeyRight && event->type == InputTypeShort) {
        with_view_model(
            instance->view, (SubGhzReadRAWModel * model) {
                if(model->status == SubGhzReadRAWStatusIDLE) {
                    //Save
                    instance->callback(SubGhzCustomEventViewReadRAWSave, instance->context);
                } else if(model->status == SubGhzReadRAWStatusLoadKeyIDLE) {
                    //More
                    instance->callback(SubGhzCustomEventViewReadRAWMore, instance->context);
                }
                return true;
            });
    } else if(event->key == InputKeyOk && event->type == InputTypeShort) {
        with_view_model(
            instance->view, (SubGhzReadRAWModel * model) {
                if(model->status == SubGhzReadRAWStatusStart) {
                    //Record
                    instance->callback(SubGhzCustomEventViewReadRAWREC, instance->context);
                    model->status = SubGhzReadRAWStatusREC;
                    model->ind_write = 0;
                    model->rssi_history_end = false;
                } else if(model->status == SubGhzReadRAWStatusREC) {
                    //Stop
                    instance->callback(SubGhzCustomEventViewReadRAWIDLE, instance->context);
                    model->status = SubGhzReadRAWStatusIDLE;
                }
                return true;
            });
    }
    return true;
}

void subghz_read_raw_set_status(
    SubGhzReadRAW* instance,
    SubGhzReadRAWStatus status,
    const char* file_name) {
    furi_assert(instance);

    switch(status) {
    case SubGhzReadRAWStatusStart:
        with_view_model(
            instance->view, (SubGhzReadRAWModel * model) {
                model->status = SubGhzReadRAWStatusStart;
                model->rssi_history_end = false;
                model->ind_write = 0;
                string_reset(model->file_name);
                string_set_str(model->sample_write, "0 spl.");
                return true;
            });
        break;
    case SubGhzReadRAWStatusIDLE:
        with_view_model(
            instance->view, (SubGhzReadRAWModel * model) {
                model->status = SubGhzReadRAWStatusIDLE;
                return true;
            });
        break;
    case SubGhzReadRAWStatusLoadKeyTX:
        with_view_model(
            instance->view, (SubGhzReadRAWModel * model) {
                model->status = SubGhzReadRAWStatusLoadKeyIDLE;
                model->rssi_history_end = false;
                model->ind_write = 0;
                string_set_str(model->file_name, file_name);
                string_set_str(model->sample_write, "RAW");
                return true;
            });
        break;
    case SubGhzReadRAWStatusSaveKey:
        with_view_model(
            instance->view, (SubGhzReadRAWModel * model) {
                model->status = SubGhzReadRAWStatusLoadKeyIDLE;
                if(!model->ind_write) {
                    string_set_str(model->file_name, file_name);
                    string_set_str(model->sample_write, "RAW");
                } else {
                    string_reset(model->file_name);
                }
                return true;
            });
        break;

    default:
        FURI_LOG_W(TAG, "unknown status");
        break;
    }
}

void subghz_read_raw_enter(void* context) {
    furi_assert(context);
    //SubGhzReadRAW* instance = context;
}

void subghz_read_raw_exit(void* context) {
    furi_assert(context);
    SubGhzReadRAW* instance = context;

    with_view_model(
        instance->view, (SubGhzReadRAWModel * model) {
            if(model->status != SubGhzReadRAWStatusIDLE &&
               model->status != SubGhzReadRAWStatusStart &&
               model->status != SubGhzReadRAWStatusLoadKeyIDLE) {
                instance->callback(SubGhzCustomEventViewReadRAWIDLE, instance->context);
                model->status = SubGhzReadRAWStatusStart;
            }
            return true;
        });
}

SubGhzReadRAW* subghz_read_raw_alloc() {
    SubGhzReadRAW* instance = malloc(sizeof(SubGhzReadRAW));

    // View allocation and configuration
    instance->view = view_alloc();
    view_allocate_model(instance->view, ViewModelTypeLocking, sizeof(SubGhzReadRAWModel));
    view_set_context(instance->view, instance);
    view_set_draw_callback(instance->view, (ViewDrawCallback)subghz_read_raw_draw);
    view_set_input_callback(instance->view, subghz_read_raw_input);
    view_set_enter_callback(instance->view, subghz_read_raw_enter);
    view_set_exit_callback(instance->view, subghz_read_raw_exit);

    with_view_model(
        instance->view, (SubGhzReadRAWModel * model) {
            string_init(model->frequency_str);
            string_init(model->preset_str);
            string_init(model->sample_write);
            string_init(model->file_name);
            model->rssi_history = malloc(SUBGHZ_READ_RAW_RSSI_HISTORY_SIZE * sizeof(uint8_t));
            return true;
        });

    return instance;
}

void subghz_read_raw_free(SubGhzReadRAW* instance) {
    furi_assert(instance);

    with_view_model(
        instance->view, (SubGhzReadRAWModel * model) {
            string_clear(model->frequency_str);
            string_clear(model->preset_str);
            string_clear(model->sample_write);
            string_clear(model->file_name);
            free(model->rssi_history);
            return true;
        });
    view_free(instance->view);
    free(instance);
}

View* subghz_read_raw_get_view(SubGhzReadRAW* instance) {
    furi_assert(instance);
    return instance->view;
}
