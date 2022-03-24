#include "dict_attack.h"
#include <m-string.h>

#include <gui/elements.h>

typedef enum {
    DictAttackStateSearchCard,
    DictAttackStateSearchKeys,
    DictAttackStateCardRemoved,
    DictAttackStateSuccess,
    DictAttackStateFail,
} DictAttackState;

struct DictAttack {
    View* view;
    DictAttackResultCallback callback;
    void* context;
};

typedef struct {
    DictAttackState state;
    MfClassicType type;
    uint8_t current_sector;
    uint8_t total_sectors;
    uint8_t keys_a_found;
    uint8_t keys_a_total;
    uint8_t keys_b_found;
    uint8_t keys_b_total;
} DictAttackViewModel;

static void dict_attack_draw_callback(Canvas* canvas, void* model) {
    DictAttackViewModel* m = model;
    if(m->state == DictAttackStateSearchCard) {
        canvas_set_font(canvas, FontPrimary);
        canvas_draw_str_aligned(
            canvas, 64, 32, AlignCenter, AlignCenter, "Detecting Mifare Classic");
    } else if(m->state == DictAttackStateCardRemoved) {
        canvas_set_font(canvas, FontPrimary);
        canvas_draw_str_aligned(
            canvas, 64, 32, AlignCenter, AlignTop, "Place card back to flipper");
    } else {
        char draw_str[32];
        if(m->state == DictAttackStateSearchKeys) {
            snprintf(
                draw_str, sizeof(draw_str), "Searching keys for sector %d", m->current_sector);
            canvas_draw_str_aligned(canvas, 64, 2, AlignCenter, AlignTop, draw_str);
        } else if(m->state == DictAttackStateSuccess) {
            canvas_draw_str_aligned(canvas, 64, 2, AlignCenter, AlignTop, "Complete!");
            elements_button_right(canvas, "Save");
        } else if(m->state == DictAttackStateFail) {
            canvas_draw_str_aligned(
                canvas, 64, 2, AlignCenter, AlignTop, "Failed to read any sector");
        }
        uint16_t keys_found = m->keys_a_found + m->keys_b_found;
        uint16_t keys_total = m->keys_a_total + m->keys_b_total;
        float progress = (float)(m->current_sector) / (float)(m->total_sectors);
        elements_progress_bar(canvas, 5, 12, 120, progress);
        canvas_set_font(canvas, FontSecondary);
        snprintf(draw_str, sizeof(draw_str), "Total keys found: %d/%d", keys_found, keys_total);
        canvas_draw_str_aligned(canvas, 1, 23, AlignLeft, AlignTop, draw_str);
        snprintf(
            draw_str, sizeof(draw_str), "A keys found: %d/%d", m->keys_a_found, m->keys_a_total);
        canvas_draw_str_aligned(canvas, 1, 34, AlignLeft, AlignTop, draw_str);
        snprintf(
            draw_str, sizeof(draw_str), "B keys found: %d/%d", m->keys_b_found, m->keys_b_total);
        canvas_draw_str_aligned(canvas, 1, 45, AlignLeft, AlignTop, draw_str);
    }
}

static bool dict_attack_input_callback(InputEvent* event, void* context) {
    DictAttack* dict_attack = context;
    bool consumed = false;
    DictAttackState state;
    with_view_model(
        dict_attack->view, (DictAttackViewModel * model) {
            state = model->state;
            return false;
        });
    if(state == DictAttackStateSuccess && event->type == InputTypeShort &&
       event->key == InputKeyRight) {
        if(dict_attack->callback) {
            dict_attack->callback(dict_attack->context);
        }
        consumed = true;
    }
    return consumed;
}

DictAttack* dict_attack_alloc() {
    DictAttack* dict_attack = malloc(sizeof(DictAttack));
    dict_attack->view = view_alloc();
    view_allocate_model(dict_attack->view, ViewModelTypeLocking, sizeof(DictAttackViewModel));
    view_set_draw_callback(dict_attack->view, dict_attack_draw_callback);
    view_set_input_callback(dict_attack->view, dict_attack_input_callback);
    view_set_context(dict_attack->view, dict_attack);
    return dict_attack;
}

void dict_attack_free(DictAttack* dict_attack) {
    furi_assert(dict_attack);
    view_free(dict_attack->view);
    free(dict_attack);
}

void dict_attack_reset(DictAttack* dict_attack) {
    furi_assert(dict_attack);
    with_view_model(
        dict_attack->view, (DictAttackViewModel * model) {
            memset(model, 0, sizeof(DictAttackViewModel));
            return true;
        });
}

View* dict_attack_get_view(DictAttack* dict_attack) {
    furi_assert(dict_attack);
    return dict_attack->view;
}

void dict_attack_set_result_callback(
    DictAttack* dict_attack,
    DictAttackResultCallback callback,
    void* context) {
    furi_assert(dict_attack);
    furi_assert(callback);
    dict_attack->callback = callback;
    dict_attack->context = context;
}

void dict_attack_card_detected(DictAttack* dict_attack, MfClassicType type) {
    furi_assert(dict_attack);
    with_view_model(
        dict_attack->view, (DictAttackViewModel * model) {
            model->state = DictAttackStateSearchKeys;
            if(type == MfClassicType1k) {
                model->total_sectors = 16;
                model->keys_a_total = 16;
                model->keys_b_total = 16;
            } else if(type == MfClassicType4k) {
                model->total_sectors = 40;
                model->keys_a_total = 40;
                model->keys_b_total = 40;
            }
            return true;
        });
}

void dict_attack_card_removed(DictAttack* dict_attack) {
    furi_assert(dict_attack);
    with_view_model(
        dict_attack->view, (DictAttackViewModel * model) {
            if(model->state == DictAttackStateSearchKeys) {
                model->state = DictAttackStateCardRemoved;
            } else {
                model->state = DictAttackStateSearchCard;
            }
            return true;
        });
}

void dict_attack_inc_curr_sector(DictAttack* dict_attack) {
    furi_assert(dict_attack);
    with_view_model(
        dict_attack->view, (DictAttackViewModel * model) {
            model->current_sector++;
            return true;
        });
}

void dict_attack_inc_found_key(DictAttack* dict_attack, MfClassicKey key) {
    furi_assert(dict_attack);
    with_view_model(
        dict_attack->view, (DictAttackViewModel * model) {
            model->state = DictAttackStateSearchKeys;
            if(key == MfClassicKeyA) {
                model->keys_a_found++;
            } else if(key == MfClassicKeyB) {
                model->keys_b_found++;
            }
            return true;
        });
}

void dict_attack_set_result(DictAttack* dict_attack, bool success) {
    furi_assert(dict_attack);
    with_view_model(
        dict_attack->view, (DictAttackViewModel * model) {
            if(success) {
                model->state = DictAttackStateSuccess;
            } else {
                model->state = DictAttackStateFail;
            }
            return true;
        });
}
