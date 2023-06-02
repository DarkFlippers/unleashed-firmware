#include "attack.h"
#include "../fuzzer_i.h"

#include <input/input.h>
#include <gui/elements.h>

#include "../helpers/protocol.h"

#define ATTACK_SCENE_MAX_UID_LENGTH 25

struct FuzzerViewAttack {
    View* view;
    FuzzerViewAttackCallback callback;
    void* context;
};

typedef struct {
    uint8_t time_delay;
    const char* attack_name;
    const char* protocol_name;
    bool attack_enabled;
    char* uid;
    uint8_t uid_size;
} FuzzerViewAttackModel;

void fuzzer_view_attack_reset_data(
    FuzzerViewAttack* view,
    const char* attack_name,
    const char* protocol_name,
    uint8_t uid_size) {
    furi_assert(view);

    with_view_model(
        view->view,
        FuzzerViewAttackModel * model,
        {
            model->attack_name = attack_name;
            model->protocol_name = protocol_name;
            model->attack_enabled = false;
            model->uid_size = uid_size;
        },
        true);
}

void fuzzer_view_attack_set_uid(FuzzerViewAttack* view, const uint8_t* uid, bool attack) {
    furi_assert(view);

    with_view_model(
        view->view,
        FuzzerViewAttackModel * model,
        {
            snprintf(
                model->uid,
                model->uid_size * 3,
                "%02X:%02X:%02X:%02X:%02X:%02X:%02X:%02X",
                uid[0],
                uid[1],
                uid[2],
                uid[3],
                uid[4],
                uid[5],
                uid[6],
                uid[7]);
            model->attack_enabled = attack;
        },
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
    char time_delay[16];
    snprintf(time_delay, sizeof(time_delay), "Time delay: %d", model->time_delay);

    canvas_clear(canvas);
    canvas_set_color(canvas, ColorBlack);

    canvas_set_font(canvas, FontPrimary);
    canvas_draw_str_aligned(canvas, 64, 2, AlignCenter, AlignTop, model->attack_name);

    canvas_set_font(canvas, FontSecondary);
    canvas_draw_str_aligned(canvas, 64, 14, AlignCenter, AlignTop, time_delay);
    canvas_draw_str_aligned(canvas, 64, 26, AlignCenter, AlignTop, model->protocol_name);

    canvas_set_font(canvas, FontPrimary);
    if(128 < canvas_string_width(canvas, model->uid)) {
        canvas_set_font(canvas, FontSecondary);
    }
    canvas_draw_str_aligned(canvas, 64, 38, AlignCenter, AlignTop, model->uid);

    if(model->attack_enabled) {
        elements_button_center(canvas, "Stop");
    } else {
        elements_button_center(canvas, "Start");
        elements_button_left(canvas, "TD -");
        elements_button_right(canvas, "+ TD");
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
                if(!model->attack_enabled) {
                    if(event->type == InputTypeShort) {
                        if(model->time_delay > FUZZ_TIME_DELAY_MIN) {
                            model->time_delay--;
                        }
                    } else if(event->type == InputTypeLong) {
                        if((model->time_delay - 10) >= FUZZ_TIME_DELAY_MIN) {
                            model->time_delay -= 10;
                        } else {
                            model->time_delay = FUZZ_TIME_DELAY_MIN;
                        }
                    }
                }
            },
            true);
        return true;
    } else if(event->key == InputKeyRight) {
        with_view_model(
            view_attack->view,
            FuzzerViewAttackModel * model,
            {
                if(!model->attack_enabled) {
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
                }
            },
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
}

FuzzerViewAttack* fuzzer_view_attack_alloc() {
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
            model->time_delay = FUZZ_TIME_DELAY_MIN;
            model->uid = malloc(ATTACK_SCENE_MAX_UID_LENGTH + 1);
            model->attack_enabled = false;

            strcpy(model->uid, "Not_set");
            model->attack_name = "Not_set";
            model->protocol_name = "Not_set";
        },
        true);
    return view_attack;
}

void fuzzer_view_attack_free(FuzzerViewAttack* view_attack) {
    furi_assert(view_attack);

    with_view_model(
        view_attack->view, FuzzerViewAttackModel * model, { free(model->uid); }, true);
    view_free(view_attack->view);
    free(view_attack);
}

View* fuzzer_view_attack_get_view(FuzzerViewAttack* view_attack) {
    furi_assert(view_attack);
    return view_attack->view;
}