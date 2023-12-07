#include "dict_attack.h"

#include <gui/elements.h>

#define NFC_CLASSIC_KEYS_PER_SECTOR 2

struct DictAttack {
    View* view;
    DictAttackCallback callback;
    void* context;
};

typedef struct {
    FuriString* header;
    bool card_detected;
    uint8_t sectors_total;
    uint8_t sectors_read;
    uint8_t current_sector;
    uint8_t keys_found;
    size_t dict_keys_total;
    size_t dict_keys_current;
    bool is_key_attack;
    uint8_t key_attack_current_sector;
} DictAttackViewModel;

static void dict_attack_draw_callback(Canvas* canvas, void* model) {
    DictAttackViewModel* m = model;
    if(!m->card_detected) {
        canvas_set_font(canvas, FontPrimary);
        canvas_draw_str_aligned(canvas, 64, 4, AlignCenter, AlignTop, "Lost the tag!");
        canvas_set_font(canvas, FontSecondary);
        elements_multiline_text_aligned(
            canvas, 64, 23, AlignCenter, AlignTop, "Make sure the tag is\npositioned correctly.");
    } else {
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
            snprintf(draw_str, sizeof(draw_str), "Unlocking sector: %d", m->current_sector);
        }
        canvas_draw_str_aligned(canvas, 0, 10, AlignLeft, AlignTop, draw_str);
        float dict_progress = m->dict_keys_total == 0 ?
                                  0 :
                                  (float)(m->dict_keys_current) / (float)(m->dict_keys_total);
        float progress = m->sectors_total == 0 ? 0 :
                                                 ((float)(m->current_sector) + dict_progress) /
                                                     (float)(m->sectors_total);
        if(progress > 1.0f) {
            progress = 1.0f;
        }
        if(m->dict_keys_current == 0) {
            // Cause when people see 0 they think it's broken
            snprintf(draw_str, sizeof(draw_str), "%d/%zu", 1, m->dict_keys_total);
        } else {
            snprintf(
                draw_str, sizeof(draw_str), "%zu/%zu", m->dict_keys_current, m->dict_keys_total);
        }
        elements_progress_bar_with_text(canvas, 0, 20, 128, dict_progress, draw_str);
        canvas_set_font(canvas, FontSecondary);
        snprintf(
            draw_str,
            sizeof(draw_str),
            "Keys found: %d/%d",
            m->keys_found,
            m->sectors_total * NFC_CLASSIC_KEYS_PER_SECTOR);
        canvas_draw_str_aligned(canvas, 0, 33, AlignLeft, AlignTop, draw_str);
        snprintf(
            draw_str, sizeof(draw_str), "Sectors Read: %d/%d", m->sectors_read, m->sectors_total);
        canvas_draw_str_aligned(canvas, 0, 43, AlignLeft, AlignTop, draw_str);
    }
    elements_button_center(canvas, "Skip");
}

static bool dict_attack_input_callback(InputEvent* event, void* context) {
    DictAttack* instance = context;
    bool consumed = false;

    if(event->type == InputTypeShort && event->key == InputKeyOk) {
        if(instance->callback) {
            instance->callback(DictAttackEventSkipPressed, instance->context);
        }
        consumed = true;
    }

    return consumed;
}

DictAttack* dict_attack_alloc() {
    DictAttack* instance = malloc(sizeof(DictAttack));
    instance->view = view_alloc();
    view_allocate_model(instance->view, ViewModelTypeLocking, sizeof(DictAttackViewModel));
    view_set_draw_callback(instance->view, dict_attack_draw_callback);
    view_set_input_callback(instance->view, dict_attack_input_callback);
    view_set_context(instance->view, instance);
    with_view_model(
        instance->view,
        DictAttackViewModel * model,
        { model->header = furi_string_alloc(); },
        false);

    return instance;
}

void dict_attack_free(DictAttack* instance) {
    furi_assert(instance);

    with_view_model(
        instance->view, DictAttackViewModel * model, { furi_string_free(model->header); }, false);

    view_free(instance->view);
    free(instance);
}

void dict_attack_reset(DictAttack* instance) {
    furi_assert(instance);

    with_view_model(
        instance->view,
        DictAttackViewModel * model,
        {
            model->sectors_total = 0;
            model->sectors_read = 0;
            model->current_sector = 0;
            model->keys_found = 0;
            model->dict_keys_total = 0;
            model->dict_keys_current = 0;
            model->is_key_attack = false;
            furi_string_reset(model->header);
        },
        false);
}

View* dict_attack_get_view(DictAttack* instance) {
    furi_assert(instance);

    return instance->view;
}

void dict_attack_set_callback(DictAttack* instance, DictAttackCallback callback, void* context) {
    furi_assert(instance);
    furi_assert(callback);

    instance->callback = callback;
    instance->context = context;
}

void dict_attack_set_header(DictAttack* instance, const char* header) {
    furi_assert(instance);
    furi_assert(header);

    with_view_model(
        instance->view,
        DictAttackViewModel * model,
        { furi_string_set(model->header, header); },
        true);
}

void dict_attack_set_card_state(DictAttack* instance, bool detected) {
    furi_assert(instance);

    with_view_model(
        instance->view, DictAttackViewModel * model, { model->card_detected = detected; }, true);
}

void dict_attack_set_sectors_total(DictAttack* instance, uint8_t sectors_total) {
    furi_assert(instance);

    with_view_model(
        instance->view,
        DictAttackViewModel * model,
        { model->sectors_total = sectors_total; },
        true);
}

void dict_attack_set_sectors_read(DictAttack* instance, uint8_t sectors_read) {
    furi_assert(instance);

    with_view_model(
        instance->view, DictAttackViewModel * model, { model->sectors_read = sectors_read; }, true);
}

void dict_attack_set_keys_found(DictAttack* instance, uint8_t keys_found) {
    furi_assert(instance);

    with_view_model(
        instance->view, DictAttackViewModel * model, { model->keys_found = keys_found; }, true);
}

void dict_attack_set_current_sector(DictAttack* instance, uint8_t current_sector) {
    furi_assert(instance);

    with_view_model(
        instance->view,
        DictAttackViewModel * model,
        { model->current_sector = current_sector; },
        true);
}

void dict_attack_set_total_dict_keys(DictAttack* instance, size_t dict_keys_total) {
    furi_assert(instance);

    with_view_model(
        instance->view,
        DictAttackViewModel * model,
        { model->dict_keys_total = dict_keys_total; },
        true);
}

void dict_attack_set_current_dict_key(DictAttack* instance, size_t cur_key_num) {
    furi_assert(instance);

    with_view_model(
        instance->view,
        DictAttackViewModel * model,
        { model->dict_keys_current = cur_key_num; },
        true);
}

void dict_attack_set_key_attack(DictAttack* instance, uint8_t sector) {
    furi_assert(instance);

    with_view_model(
        instance->view,
        DictAttackViewModel * model,
        {
            model->is_key_attack = true;
            model->key_attack_current_sector = sector;
        },
        true);
}

void dict_attack_reset_key_attack(DictAttack* instance) {
    furi_assert(instance);

    with_view_model(
        instance->view, DictAttackViewModel * model, { model->is_key_attack = false; }, true);
}
