#include "../signal_gen_app_i.h"
#include "furi_hal.h"
#include <gui/elements.h>

typedef enum {
    LineIndexChannel,
    LineIndexFrequency,
    LineIndexDuty,
    LineIndexTotalCount
} LineIndex;

static const char* const pwm_ch_names[] = {"TIM1(2)", "LPTIM2(4)"};

struct SignalGenPwm {
    View* view;
    SignalGenPwmViewCallback callback;
    void* context;
};

typedef struct {
    LineIndex line_sel;
    bool edit_mode;
    uint8_t edit_digit;

    uint8_t channel_id;
    uint32_t freq;
    uint8_t duty;

} SignalGenPwmViewModel;

#define ITEM_H 64 / 3
#define ITEM_W 128

#define VALUE_X 95
#define VALUE_W 55

#define FREQ_VALUE_X 62
#define FREQ_MAX 1000000UL
#define FREQ_DIGITS_NB 7

static void pwm_set_config(SignalGenPwm* pwm) {
    FuriHalPwmOutputId channel;
    uint32_t freq;
    uint8_t duty;

    with_view_model(
        pwm->view, (SignalGenPwmViewModel * model) {
            channel = model->channel_id;
            freq = model->freq;
            duty = model->duty;
            return false;
        });

    furi_assert(pwm->callback);
    pwm->callback(channel, freq, duty, pwm->context);
}

static void pwm_channel_change(SignalGenPwmViewModel* model, InputEvent* event) {
    if(event->key == InputKeyLeft) {
        if(model->channel_id > 0) {
            model->channel_id--;
        }
    } else if(event->key == InputKeyRight) {
        if(model->channel_id < (COUNT_OF(pwm_ch_names) - 1)) {
            model->channel_id++;
        }
    }
}

static void pwm_duty_change(SignalGenPwmViewModel* model, InputEvent* event) {
    if(event->key == InputKeyLeft) {
        if(model->duty > 0) {
            model->duty--;
        }
    } else if(event->key == InputKeyRight) {
        if(model->duty < 100) {
            model->duty++;
        }
    }
}

static bool pwm_freq_edit(SignalGenPwmViewModel* model, InputEvent* event) {
    bool consumed = false;
    if((event->type == InputTypeShort) || (event->type == InputTypeRepeat)) {
        if(event->key == InputKeyRight) {
            if(model->edit_digit > 0) {
                model->edit_digit--;
            }
            consumed = true;
        } else if(event->key == InputKeyLeft) {
            if(model->edit_digit < (FREQ_DIGITS_NB - 1)) {
                model->edit_digit++;
            }
            consumed = true;
        } else if(event->key == InputKeyUp) {
            uint32_t step = 1;
            for(uint8_t i = 0; i < model->edit_digit; i++) {
                step *= 10;
            }
            if((model->freq + step) < FREQ_MAX) {
                model->freq += step;
            } else {
                model->freq = FREQ_MAX;
            }
            consumed = true;
        } else if(event->key == InputKeyDown) {
            uint32_t step = 1;
            for(uint8_t i = 0; i < model->edit_digit; i++) {
                step *= 10;
            }
            if(model->freq > (step + 1)) {
                model->freq -= step;
            } else {
                model->freq = 1;
            }
            consumed = true;
        }
    }
    return consumed;
}

static void signal_gen_pwm_draw_callback(Canvas* canvas, void* _model) {
    SignalGenPwmViewModel* model = _model;
    char* line_label = NULL;
    char val_text[16];

    for(uint8_t line = 0; line < LineIndexTotalCount; line++) {
        if(line == LineIndexChannel) {
            line_label = "PWM Channel";
        } else if(line == LineIndexFrequency) {
            line_label = "Frequency";
        } else if(line == LineIndexDuty) {
            line_label = "Duty Cycle";
        }

        canvas_set_color(canvas, ColorBlack);
        if(line == model->line_sel) {
            elements_slightly_rounded_box(canvas, 0, ITEM_H * line + 1, ITEM_W, ITEM_H - 1);
            canvas_set_color(canvas, ColorWhite);
        }

        uint8_t text_y = ITEM_H * line + ITEM_H / 2 + 2;

        canvas_draw_str_aligned(canvas, 6, text_y, AlignLeft, AlignCenter, line_label);

        if(line == LineIndexChannel) {
            snprintf(val_text, sizeof(val_text), "%s", pwm_ch_names[model->channel_id]);
            canvas_draw_str_aligned(canvas, VALUE_X, text_y, AlignCenter, AlignCenter, val_text);
            if(model->channel_id != 0) {
                canvas_draw_str_aligned(
                    canvas, VALUE_X - VALUE_W / 2, text_y, AlignCenter, AlignCenter, "<");
            }
            if(model->channel_id != (COUNT_OF(pwm_ch_names) - 1)) {
                canvas_draw_str_aligned(
                    canvas, VALUE_X + VALUE_W / 2, text_y, AlignCenter, AlignCenter, ">");
            }
        } else if(line == LineIndexFrequency) {
            snprintf(val_text, sizeof(val_text), "%7lu Hz", model->freq);
            canvas_set_font(canvas, FontKeyboard);
            canvas_draw_str_aligned(
                canvas, FREQ_VALUE_X, text_y, AlignLeft, AlignCenter, val_text);
            canvas_set_font(canvas, FontSecondary);

            if(model->edit_mode) {
                uint8_t icon_x = (FREQ_VALUE_X - 1) + (FREQ_DIGITS_NB - model->edit_digit - 1) * 6;
                canvas_draw_icon(canvas, icon_x, text_y - 9, &I_SmallArrowUp_4x7);
                canvas_draw_icon(canvas, icon_x, text_y + 4, &I_SmallArrowDown_4x7);
            }
        } else if(line == LineIndexDuty) {
            snprintf(val_text, sizeof(val_text), "%d%%", model->duty);
            canvas_draw_str_aligned(canvas, VALUE_X, text_y, AlignCenter, AlignCenter, val_text);
            if(model->duty != 0) {
                canvas_draw_str_aligned(
                    canvas, VALUE_X - VALUE_W / 2, text_y, AlignCenter, AlignCenter, "<");
            }
            if(model->duty != 100) {
                canvas_draw_str_aligned(
                    canvas, VALUE_X + VALUE_W / 2, text_y, AlignCenter, AlignCenter, ">");
            }
        }
    }
}

static bool signal_gen_pwm_input_callback(InputEvent* event, void* context) {
    furi_assert(context);
    SignalGenPwm* pwm = context;
    bool consumed = false;
    bool need_update = false;

    with_view_model(
        pwm->view, (SignalGenPwmViewModel * model) {
            if(model->edit_mode == false) {
                if((event->type == InputTypeShort) || (event->type == InputTypeRepeat)) {
                    if(event->key == InputKeyUp) {
                        if(model->line_sel == 0) {
                            model->line_sel = LineIndexTotalCount - 1;
                        } else {
                            model->line_sel =
                                CLAMP(model->line_sel - 1, LineIndexTotalCount - 1, 0);
                        }
                        consumed = true;
                    } else if(event->key == InputKeyDown) {
                        if(model->line_sel == LineIndexTotalCount - 1) {
                            model->line_sel = 0;
                        } else {
                            model->line_sel =
                                CLAMP(model->line_sel + 1, LineIndexTotalCount - 1, 0);
                        }
                        consumed = true;
                    } else if((event->key == InputKeyLeft) || (event->key == InputKeyRight)) {
                        if(model->line_sel == LineIndexChannel) {
                            pwm_channel_change(model, event);
                            need_update = true;
                        } else if(model->line_sel == LineIndexDuty) {
                            pwm_duty_change(model, event);
                            need_update = true;
                        } else if(model->line_sel == LineIndexFrequency) {
                            model->edit_mode = true;
                        }
                        consumed = true;
                    } else if(event->key == InputKeyOk) {
                        if(model->line_sel == LineIndexFrequency) {
                            model->edit_mode = true;
                        }
                        consumed = true;
                    }
                }
            } else {
                if((event->key == InputKeyOk) || (event->key == InputKeyBack)) {
                    if(event->type == InputTypeShort) {
                        model->edit_mode = false;
                        consumed = true;
                    }
                } else {
                    if(model->line_sel == LineIndexFrequency) {
                        consumed = pwm_freq_edit(model, event);
                        need_update = consumed;
                    }
                }
            }
            return true;
        });

    if(need_update) {
        pwm_set_config(pwm);
    }

    return consumed;
}

SignalGenPwm* signal_gen_pwm_alloc() {
    SignalGenPwm* pwm = malloc(sizeof(SignalGenPwm));

    pwm->view = view_alloc();
    view_allocate_model(pwm->view, ViewModelTypeLocking, sizeof(SignalGenPwmViewModel));
    view_set_context(pwm->view, pwm);
    view_set_draw_callback(pwm->view, signal_gen_pwm_draw_callback);
    view_set_input_callback(pwm->view, signal_gen_pwm_input_callback);

    return pwm;
}

void signal_gen_pwm_free(SignalGenPwm* pwm) {
    furi_assert(pwm);
    view_free(pwm->view);
    free(pwm);
}

View* signal_gen_pwm_get_view(SignalGenPwm* pwm) {
    furi_assert(pwm);
    return pwm->view;
}

void signal_gen_pwm_set_callback(
    SignalGenPwm* pwm,
    SignalGenPwmViewCallback callback,
    void* context) {
    furi_assert(pwm);
    furi_assert(callback);

    with_view_model(
        pwm->view, (SignalGenPwmViewModel * model) {
            UNUSED(model);
            pwm->callback = callback;
            pwm->context = context;
            return false;
        });
}

void signal_gen_pwm_set_params(SignalGenPwm* pwm, uint8_t channel_id, uint32_t freq, uint8_t duty) {
    with_view_model(
        pwm->view, (SignalGenPwmViewModel * model) {
            model->channel_id = channel_id;
            model->freq = freq;
            model->duty = duty;
            return true;
        });

    furi_assert(pwm->callback);
    pwm->callback(channel_id, freq, duty, pwm->context);
}
