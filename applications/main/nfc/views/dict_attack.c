#include "dict_attack.h"

#include <gui/elements.h>

typedef enum {
    DictAttackStateRead,
    DictAttackStateCardRemoved,
} DictAttackState;

struct DictAttack {
    View* view;
    DictAttackCallback callback;
    void* context;
    bool card_present;
};

typedef struct {
    DictAttackState state;
    MfClassicType type;
    FuriString* header;
    uint8_t sectors_total;
    uint8_t sectors_read;
    uint8_t sector_current;
    uint8_t keys_total;
    uint8_t keys_found;
    uint16_t dict_keys_total;
    uint16_t dict_keys_current;
    bool is_key_attack;
    uint8_t key_attack_current_sector;
} DictAttackViewModel;

static void dict_attack_draw_callback(Canvas* canvas, void* model) {
    DictAttackViewModel* m = model;
    if(m->state == DictAttackStateCardRemoved) {
        canvas_set_font(canvas, FontPrimary);
        canvas_draw_str_aligned(canvas, 64, 4, AlignCenter, AlignTop, "Lost the tag!");
        canvas_set_font(canvas, FontSecondary);
        elements_multiline_text_aligned(
            canvas, 64, 23, AlignCenter, AlignTop, "Make sure the tag is\npositioned correctly.");
    } else if(m->state == DictAttackStateRead) {
        char draw_str[32] = {};
        canvas_set_font(canvas, FontSecondary);
        canvas_draw_str_aligned(
            canvas, 64, 0, AlignCenter, AlignTop, furi_string_get_cstr(m->header));
        if(m->is_key_attack) {
            snprintf(
                draw_str,
                sizeof(draw_str),
                "Reuse key check for sector: %d",
                m->key_attack_current_sector);
        } else {
            snprintf(draw_str, sizeof(draw_str), "Unlocking sector: %d", m->sector_current);
        }
        canvas_draw_str_aligned(canvas, 0, 10, AlignLeft, AlignTop, draw_str);
        float dict_progress = m->dict_keys_total == 0 ?
                                  0 :
                                  (float)(m->dict_keys_current) / (float)(m->dict_keys_total);
        float progress = m->sectors_total == 0 ? 0 :
                                                 ((float)(m->sector_current) + dict_progress) /
                                                     (float)(m->sectors_total);
        if(progress > 1.0) {
            progress = 1.0;
        }
        if(m->dict_keys_current == 0) {
            // Cause when people see 0 they think it's broken
            snprintf(draw_str, sizeof(draw_str), "%d/%d", 1, m->dict_keys_total);
        } else {
            snprintf(
                draw_str, sizeof(draw_str), "%d/%d", m->dict_keys_current, m->dict_keys_total);
        }
        elements_progress_bar_with_text(canvas, 0, 20, 128, dict_progress, draw_str);
        canvas_set_font(canvas, FontSecondary);
        snprintf(draw_str, sizeof(draw_str), "Keys found: %d/%d", m->keys_found, m->keys_total);
        canvas_draw_str_aligned(canvas, 0, 33, AlignLeft, AlignTop, draw_str);
        snprintf(
            draw_str, sizeof(draw_str), "Sectors Read: %d/%d", m->sectors_read, m->sectors_total);
        canvas_draw_str_aligned(canvas, 0, 43, AlignLeft, AlignTop, draw_str);
    }
    elements_button_center(canvas, "Skip");
}

static bool dict_attack_input_callback(InputEvent* event, void* context) {
    DictAttack* dict_attack = context;
    bool consumed = false;
    if(event->type == InputTypeShort && event->key == InputKeyOk) {
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
    with_view_model(
        dict_attack->view,
        DictAttackViewModel * model,
        { model->header = furi_string_alloc(); },
        false);
    return dict_attack;
}

void dict_attack_free(DictAttack* dict_attack) {
    furi_assert(dict_attack);
    with_view_model(
        dict_attack->view,
        DictAttackViewModel * model,
        { furi_string_free(model->header); },
        false);
    view_free(dict_attack->view);
    free(dict_attack);
}

void dict_attack_reset(DictAttack* dict_attack) {
    furi_assert(dict_attack);
    with_view_model(
        dict_attack->view,
        DictAttackViewModel * model,
        {
            model->state = DictAttackStateRead;
            model->type = MfClassicType1k;
            model->sectors_total = 0;
            model->sectors_read = 0;
            model->sector_current = 0;
            model->keys_total = 0;
            model->keys_found = 0;
            model->dict_keys_total = 0;
            model->dict_keys_current = 0;
            model->is_key_attack = false;
            furi_string_reset(model->header);
        },
        false);
}

View* dict_attack_get_view(DictAttack* dict_attack) {
    furi_assert(dict_attack);
    return dict_attack->view;
}

void dict_attack_set_callback(DictAttack* dict_attack, DictAttackCallback callback, void* context) {
    furi_assert(dict_attack);
    furi_assert(callback);
    dict_attack->callback = callback;
    dict_attack->context = context;
}

void dict_attack_set_header(DictAttack* dict_attack, const char* header) {
    furi_assert(dict_attack);
    furi_assert(header);

    with_view_model(
        dict_attack->view,
        DictAttackViewModel * model,
        { furi_string_set(model->header, header); },
        true);
}

void dict_attack_set_card_detected(DictAttack* dict_attack, MfClassicType type) {
    furi_assert(dict_attack);
    dict_attack->card_present = true;
    with_view_model(
        dict_attack->view,
        DictAttackViewModel * model,
        {
            model->state = DictAttackStateRead;
            model->sectors_total = mf_classic_get_total_sectors_num(type);
            model->keys_total = model->sectors_total * 2;
        },
        true);
}

void dict_attack_set_card_removed(DictAttack* dict_attack) {
    furi_assert(dict_attack);
    dict_attack->card_present = false;
    with_view_model(
        dict_attack->view,
        DictAttackViewModel * model,
        { model->state = DictAttackStateCardRemoved; },
        true);
}

bool dict_attack_get_card_state(DictAttack* dict_attack) {
    furi_assert(dict_attack);
    return dict_attack->card_present;
}

void dict_attack_set_sector_read(DictAttack* dict_attack, uint8_t sec_read) {
    furi_assert(dict_attack);
    with_view_model(
        dict_attack->view, DictAttackViewModel * model, { model->sectors_read = sec_read; }, true);
}

void dict_attack_set_keys_found(DictAttack* dict_attack, uint8_t keys_found) {
    furi_assert(dict_attack);
    with_view_model(
        dict_attack->view, DictAttackViewModel * model, { model->keys_found = keys_found; }, true);
}

void dict_attack_set_current_sector(DictAttack* dict_attack, uint8_t curr_sec) {
    furi_assert(dict_attack);
    with_view_model(
        dict_attack->view,
        DictAttackViewModel * model,
        {
            model->sector_current = curr_sec;
            model->dict_keys_current = 0;
        },
        true);
}

void dict_attack_inc_current_sector(DictAttack* dict_attack) {
    furi_assert(dict_attack);
    with_view_model(
        dict_attack->view,
        DictAttackViewModel * model,
        {
            if(model->sector_current < model->sectors_total) {
                model->sector_current++;
                model->dict_keys_current = 0;
            }
        },
        true);
}

void dict_attack_inc_keys_found(DictAttack* dict_attack) {
    furi_assert(dict_attack);
    with_view_model(
        dict_attack->view,
        DictAttackViewModel * model,
        {
            if(model->keys_found < model->keys_total) {
                model->keys_found++;
            }
        },
        true);
}

void dict_attack_set_total_dict_keys(DictAttack* dict_attack, uint16_t dict_keys_total) {
    furi_assert(dict_attack);
    with_view_model(
        dict_attack->view,
        DictAttackViewModel * model,
        { model->dict_keys_total = dict_keys_total; },
        true);
}

void dict_attack_inc_current_dict_key(DictAttack* dict_attack, uint16_t keys_tried) {
    furi_assert(dict_attack);
    with_view_model(
        dict_attack->view,
        DictAttackViewModel * model,
        {
            if(model->dict_keys_current + keys_tried < model->dict_keys_total) {
                model->dict_keys_current += keys_tried;
            }
        },
        true);
}

void dict_attack_set_key_attack(DictAttack* dict_attack, bool is_key_attack, uint8_t sector) {
    furi_assert(dict_attack);
    with_view_model(
        dict_attack->view,
        DictAttackViewModel * model,
        {
            model->is_key_attack = is_key_attack;
            model->key_attack_current_sector = sector;
        },
        true);
}

void dict_attack_inc_key_attack_current_sector(DictAttack* dict_attack) {
    furi_assert(dict_attack);
    with_view_model(
        dict_attack->view,
        DictAttackViewModel * model,
        {
            if(model->key_attack_current_sector < model->sectors_total) {
                model->key_attack_current_sector++;
            }
        },
        true);
}
