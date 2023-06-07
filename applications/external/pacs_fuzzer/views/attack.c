#include "attack.h"
#include "../fuzzer_i.h"

#include <input/input.h>
#include <gui/elements.h>

#define ATTACK_SCENE_MAX_UID_LENGTH 25
#define UID_MAX_DISPLAYED_LEN (8U)
#define LIFT_RIGHT_OFFSET (3)

struct FuzzerViewAttack {
    View* view;
    FuzzerViewAttackCallback callback;
    void* context;
};

typedef struct {
    uint8_t time_delay; // 1 = 100ms
    uint8_t time_delay_min; // 1 = 100ms
    uint8_t emu_time; // 1 = 100ms
    uint8_t emu_time_min; // 1 = 100ms
    bool td_emt_cursor; // false - time_delay, true - emu_time
    const char* attack_name;
    const char* protocol_name;
    FuzzerAttackState attack_state;
    FuriString* uid_str;
} FuzzerViewAttackModel;

void fuzzer_view_attack_reset_data(
    FuzzerViewAttack* view,
    const char* attack_name,
    const char* protocol_name) {
    furi_assert(view);

    with_view_model(
        view->view,
        FuzzerViewAttackModel * model,
        {
            model->attack_name = attack_name;
            model->protocol_name = protocol_name;
            model->attack_state = FuzzerAttackStateIdle;
            furi_string_set_str(model->uid_str, "Not_set");
        },
        true);
}

void fuzzer_view_attack_set_uid(FuzzerViewAttack* view, const FuzzerPayload* uid) {
    furi_assert(view);
    furi_assert(uid->data);

    with_view_model(
        view->view,
        FuzzerViewAttackModel * model,
        {
            furi_string_printf(model->uid_str, "%02X", uid->data[0]);
            for(uint8_t i = 1; i < uid->data_size; i++) {
                furi_string_cat_printf(model->uid_str, ":%02X", uid->data[i]);
            }
        },
        true);
}

void fuzzer_view_attack_start(FuzzerViewAttack* view) {
    furi_assert(view);

    with_view_model(
        view->view,
        FuzzerViewAttackModel * model,
        { model->attack_state = FuzzerAttackStateRunning; },
        true);
}

void fuzzer_view_attack_stop(FuzzerViewAttack* view) {
    furi_assert(view);

    with_view_model(
        view->view,
        FuzzerViewAttackModel * model,
        { model->attack_state = FuzzerAttackStateOff; },
        true);
}

void fuzzer_view_attack_pause(FuzzerViewAttack* view) {
    furi_assert(view);

    with_view_model(
        view->view,
        FuzzerViewAttackModel * model,
        { model->attack_state = FuzzerAttackStateIdle; },
        true);
}

void fuzzer_view_attack_end(FuzzerViewAttack* view) {
    furi_assert(view);

    with_view_model(
        view->view,
        FuzzerViewAttackModel * model,
        { model->attack_state = FuzzerAttackStateEnd; },
        true);
}

void fuzzer_view_attack_set_callback(
    FuzzerViewAttack* view_attack,
    FuzzerViewAttackCallback callback,
    void* context) {
    furi_assert(view_attack);

    view_attack->callback = callback;
    view_attack->context = context;
}

void fuzzer_view_attack_draw(Canvas* canvas, FuzzerViewAttackModel* model) {
    char temp_str[50];

    canvas_clear(canvas);
    canvas_set_color(canvas, ColorBlack);

    canvas_set_font(canvas, FontPrimary);
    canvas_draw_str_aligned(canvas, 64, 2, AlignCenter, AlignTop, model->attack_name);

    uint16_t crt;
    canvas_set_font(canvas, FontPrimary);

    if(!model->td_emt_cursor) {
        canvas_set_font(canvas, FontSecondary);
        snprintf(temp_str, sizeof(temp_str), "Time delay:");
        canvas_draw_str_aligned(canvas, LIFT_RIGHT_OFFSET, 21, AlignLeft, AlignBottom, temp_str);
        crt = canvas_string_width(canvas, temp_str);

        canvas_set_font(canvas, FontPrimary);
        snprintf(
            temp_str, sizeof(temp_str), "%d.%d", model->time_delay / 10, model->time_delay % 10);
        canvas_draw_str_aligned(
            canvas, crt + LIFT_RIGHT_OFFSET + 3, 21, AlignLeft, AlignBottom, temp_str);

        canvas_set_font(canvas, FontSecondary);
        snprintf(
            temp_str, sizeof(temp_str), "EmT: %d.%d", model->emu_time / 10, model->emu_time % 10);
        canvas_draw_str_aligned(
            canvas, 128 - LIFT_RIGHT_OFFSET, 21, AlignRight, AlignBottom, temp_str);
    } else {
        canvas_set_font(canvas, FontSecondary);
        snprintf(
            temp_str,
            sizeof(temp_str),
            "TD: %d.%d",
            model->time_delay / 10,
            model->time_delay % 10);

        canvas_draw_str_aligned(canvas, LIFT_RIGHT_OFFSET, 21, AlignLeft, AlignBottom, temp_str);

        canvas_set_font(canvas, FontPrimary);
        snprintf(temp_str, sizeof(temp_str), "%d.%d", model->emu_time / 10, model->emu_time % 10);
        canvas_draw_str_aligned(
            canvas, 128 - LIFT_RIGHT_OFFSET, 21, AlignRight, AlignBottom, temp_str);
        crt = canvas_string_width(canvas, temp_str);

        canvas_set_font(canvas, FontSecondary);
        snprintf(temp_str, sizeof(temp_str), "Emulation time:");
        canvas_draw_str_aligned(
            canvas, 128 - LIFT_RIGHT_OFFSET - crt - 3, 21, AlignRight, AlignBottom, temp_str);
    }

    canvas_set_font(canvas, FontSecondary);
    canvas_draw_str_aligned(canvas, 64, 26, AlignCenter, AlignTop, model->protocol_name);

    canvas_set_font(canvas, FontPrimary);
    if(128 < canvas_string_width(canvas, furi_string_get_cstr(model->uid_str))) {
        canvas_set_font(canvas, FontSecondary);
    }
    canvas_draw_str_aligned(
        canvas, 64, 38, AlignCenter, AlignTop, furi_string_get_cstr(model->uid_str));

    canvas_set_font(canvas, FontSecondary);
    if(model->attack_state == FuzzerAttackStateRunning) {
        elements_button_center(canvas, "Stop");
    } else if(model->attack_state == FuzzerAttackStateIdle) {
        if(model->td_emt_cursor) {
            elements_button_center(canvas, "Start");
            elements_button_left(canvas, "EmT -");
            elements_button_right(canvas, "+ EmT");
        } else {
            elements_button_center(canvas, "Start");
            elements_button_left(canvas, "TD -");
            elements_button_right(canvas, "+ TD");
        }

    } else if(model->attack_state == FuzzerAttackStateEnd) {
        // elements_button_center(canvas, "Restart"); // Reset
        elements_button_left(canvas, "Exit");
    }
}

bool fuzzer_view_attack_input(InputEvent* event, void* context) {
    furi_assert(context);
    FuzzerViewAttack* view_attack = context;

    if(event->key == InputKeyBack && event->type == InputTypeShort) {
        view_attack->callback(FuzzerCustomEventViewAttackBack, view_attack->context);
        return true;
    } else if(event->key == InputKeyOk && event->type == InputTypeShort) {
        view_attack->callback(FuzzerCustomEventViewAttackOk, view_attack->context);
        return true;
    } else if(event->key == InputKeyLeft) {
        with_view_model(
            view_attack->view,
            FuzzerViewAttackModel * model,
            {
                if(model->attack_state == FuzzerAttackStateIdle) {
                    if(!model->td_emt_cursor) {
                        // TimeDelay --
                        if(event->type == InputTypeShort) {
                            if(model->time_delay > model->time_delay_min) {
                                model->time_delay--;
                            }
                        } else if(event->type == InputTypeLong) {
                            if((model->time_delay - 10) >= model->time_delay_min) {
                                model->time_delay -= 10;
                            } else {
                                model->time_delay = model->time_delay_min;
                            }
                        }
                    } else {
                        // EmuTime --
                        if(event->type == InputTypeShort) {
                            if(model->emu_time > model->emu_time_min) {
                                model->emu_time--;
                            }
                        } else if(event->type == InputTypeLong) {
                            if((model->emu_time - 10) >= model->emu_time_min) {
                                model->emu_time -= 10;
                            } else {
                                model->emu_time = model->emu_time_min;
                            }
                        }
                    }
                } else if(
                    (model->attack_state == FuzzerAttackStateEnd) &&
                    (event->type == InputTypeShort)) {
                    // Exit if Ended
                    view_attack->callback(FuzzerCustomEventViewAttackBack, view_attack->context);
                }
            },
            true);
        return true;
    } else if(event->key == InputKeyRight) {
        with_view_model(
            view_attack->view,
            FuzzerViewAttackModel * model,
            {
                if(model->attack_state == FuzzerAttackStateIdle) {
                    if(!model->td_emt_cursor) {
                        // TimeDelay ++
                        if(event->type == InputTypeShort) {
                            if(model->time_delay < FUZZ_TIME_DELAY_MAX) {
                                model->time_delay++;
                            }
                        } else if(event->type == InputTypeLong) {
                            model->time_delay += 10;
                            if(model->time_delay > FUZZ_TIME_DELAY_MAX) {
                                model->time_delay = FUZZ_TIME_DELAY_MAX;
                            }
                        }
                    } else {
                        // EmuTime ++
                        if(event->type == InputTypeShort) {
                            if(model->emu_time < FUZZ_TIME_DELAY_MAX) {
                                model->emu_time++;
                            }
                        } else if(event->type == InputTypeLong) {
                            model->emu_time += 10;
                            if(model->emu_time > FUZZ_TIME_DELAY_MAX) {
                                model->emu_time = FUZZ_TIME_DELAY_MAX;
                            }
                        }
                    }
                } else {
                    // Nothing
                }
            },
            true);
        return true;
    } else if(
        (event->key == InputKeyUp || event->key == InputKeyDown) &&
        event->type == InputTypeShort) {
        with_view_model(
            view_attack->view,
            FuzzerViewAttackModel * model,
            { model->td_emt_cursor = !model->td_emt_cursor; },
            true);
        return true;
    }

    return true;
}

void fuzzer_view_attack_enter(void* context) {
    furi_assert(context);
}

void fuzzer_view_attack_exit(void* context) {
    furi_assert(context);
    FuzzerViewAttack* view_attack = context;
    with_view_model(
        view_attack->view, FuzzerViewAttackModel * model, { model->td_emt_cursor = false; }, true);
}

FuzzerViewAttack* fuzzer_view_attack_alloc() {
    if(fuzzer_proto_get_max_data_size() > UID_MAX_DISPLAYED_LEN) {
        furi_crash("Maximum of displayed bytes exceeded");
    }

    FuzzerViewAttack* view_attack = malloc(sizeof(FuzzerViewAttack));

    // View allocation and configuration
    view_attack->view = view_alloc();
    view_allocate_model(view_attack->view, ViewModelTypeLocking, sizeof(FuzzerViewAttackModel));
    view_set_context(view_attack->view, view_attack);
    view_set_draw_callback(view_attack->view, (ViewDrawCallback)fuzzer_view_attack_draw);
    view_set_input_callback(view_attack->view, fuzzer_view_attack_input);
    view_set_enter_callback(view_attack->view, fuzzer_view_attack_enter);
    view_set_exit_callback(view_attack->view, fuzzer_view_attack_exit);

    with_view_model(
        view_attack->view,
        FuzzerViewAttackModel * model,
        {
            model->time_delay = fuzzer_proto_get_def_idle_time();
            model->time_delay_min = 0; // model->time_delay;

            model->emu_time = fuzzer_proto_get_def_emu_time();

            model->emu_time_min = 2; // model->emu_time;

            model->uid_str = furi_string_alloc_set_str("Not_set");
            // malloc(ATTACK_SCENE_MAX_UID_LENGTH + 1);
            model->attack_state = FuzzerAttackStateOff;
            model->td_emt_cursor = false;

            // strcpy(model->uid_str, "Not_set");
            model->attack_name = "Not_set";
            model->protocol_name = "Not_set";
        },
        true);
    return view_attack;
}

void fuzzer_view_attack_free(FuzzerViewAttack* view_attack) {
    furi_assert(view_attack);

    with_view_model(
        view_attack->view,
        FuzzerViewAttackModel * model,
        { furi_string_free(model->uid_str); },
        true);
    view_free(view_attack->view);
    free(view_attack);
}

View* fuzzer_view_attack_get_view(FuzzerViewAttack* view_attack) {
    furi_assert(view_attack);
    return view_attack->view;
}

uint8_t fuzzer_view_attack_get_time_delay(FuzzerViewAttack* view) {
    furi_assert(view);
    uint8_t time_delay;

    with_view_model(
        view->view, FuzzerViewAttackModel * model, { time_delay = model->time_delay; }, false);

    return time_delay;
}

uint8_t fuzzer_view_attack_get_emu_time(FuzzerViewAttack* view) {
    furi_assert(view);
    uint8_t emu_time;

    with_view_model(
        view->view, FuzzerViewAttackModel * model, { emu_time = model->emu_time; }, false);

    return emu_time;
}