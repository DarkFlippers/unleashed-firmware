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
    DictAttackType attack_type;

    // MIFARE Classic specific
    uint8_t sectors_total;
    uint8_t sectors_read;
    uint8_t current_sector;
    uint8_t keys_found;
    bool is_key_attack;
    uint8_t key_attack_current_sector;
    MfClassicNestedPhase nested_phase;
    MfClassicPrngType prng_type;
    MfClassicBackdoor backdoor;
    uint16_t nested_target_key;
    uint16_t msb_count;

    // Ultralight C specific
    uint8_t pages_total;
    uint8_t pages_read;
    bool key_found;

    // Common
    size_t dict_keys_total;
    size_t dict_keys_current;
} DictAttackViewModel;

static void dict_attack_draw_mf_classic(Canvas* canvas, DictAttackViewModel* m) {
    char draw_str[32] = {};
    canvas_set_font(canvas, FontSecondary);

    switch(m->nested_phase) {
    case MfClassicNestedPhaseAnalyzePRNG:
        furi_string_set(m->header, "PRNG Analysis");
        break;
    case MfClassicNestedPhaseDictAttack:
    case MfClassicNestedPhaseDictAttackVerify:
    case MfClassicNestedPhaseDictAttackResume:
        furi_string_set(m->header, "Nested Dictionary");
        break;
    case MfClassicNestedPhaseCalibrate:
    case MfClassicNestedPhaseRecalibrate:
        furi_string_set(m->header, "Calibration");
        break;
    case MfClassicNestedPhaseCollectNtEnc:
        furi_string_set(m->header, "Nonce Collection");
        break;
    default:
        break;
    }

    if(m->prng_type == MfClassicPrngTypeHard) {
        furi_string_cat(m->header, " (Hard)");
    }

    if(m->backdoor != MfClassicBackdoorNone && m->backdoor != MfClassicBackdoorUnknown) {
        if(m->nested_phase != MfClassicNestedPhaseNone) {
            furi_string_cat(m->header, " (Backdoor)");
        } else {
            furi_string_set(m->header, "Backdoor Read");
        }
    }

    canvas_draw_str_aligned(canvas, 0, 0, AlignLeft, AlignTop, furi_string_get_cstr(m->header));
    if(m->nested_phase == MfClassicNestedPhaseCollectNtEnc) {
        uint8_t nonce_sector =
            m->nested_target_key / (m->prng_type == MfClassicPrngTypeWeak ? 4 : 2);
        snprintf(draw_str, sizeof(draw_str), "Collecting from sector: %d", nonce_sector);
        canvas_draw_str_aligned(canvas, 0, 10, AlignLeft, AlignTop, draw_str);
    } else if(m->is_key_attack) {
        snprintf(
            draw_str,
            sizeof(draw_str),
            "Reuse key check for sector: %d",
            m->key_attack_current_sector);
    } else {
        snprintf(draw_str, sizeof(draw_str), "Unlocking sector: %d", m->current_sector);
    }
    canvas_draw_str_aligned(canvas, 0, 10, AlignLeft, AlignTop, draw_str);
    float dict_progress = 0;
    if(m->nested_phase == MfClassicNestedPhaseAnalyzePRNG ||
       m->nested_phase == MfClassicNestedPhaseDictAttack ||
       m->nested_phase == MfClassicNestedPhaseDictAttackVerify ||
       m->nested_phase == MfClassicNestedPhaseDictAttackResume) {
        // Phase: Nested dictionary attack
        uint8_t target_sector =
            m->nested_target_key / (m->prng_type == MfClassicPrngTypeWeak ? 2 : 16);
        dict_progress = (float)(target_sector) / (float)(m->sectors_total);
        snprintf(draw_str, sizeof(draw_str), "%d/%d", target_sector, m->sectors_total);
    } else if(
        m->nested_phase == MfClassicNestedPhaseCalibrate ||
        m->nested_phase == MfClassicNestedPhaseRecalibrate ||
        m->nested_phase == MfClassicNestedPhaseCollectNtEnc) {
        // Phase: Nonce collection
        if(m->prng_type == MfClassicPrngTypeWeak) {
            uint8_t target_sector = m->nested_target_key / 4;
            dict_progress = (float)(target_sector) / (float)(m->sectors_total);
            snprintf(draw_str, sizeof(draw_str), "%d/%d", target_sector, m->sectors_total);
        } else {
            uint16_t max_msb = UINT8_MAX + 1;
            dict_progress = (float)(m->msb_count) / (float)(max_msb);
            snprintf(draw_str, sizeof(draw_str), "%d/%d", m->msb_count, max_msb);
        }
    } else {
        dict_progress = m->dict_keys_total == 0 ?
                            0 :
                            (float)(m->dict_keys_current) / (float)(m->dict_keys_total);
        if(m->dict_keys_current == 0) {
            // Cause when people see 0 they think it's broken
            snprintf(draw_str, sizeof(draw_str), "%d/%zu", 1, m->dict_keys_total);
        } else {
            snprintf(
                draw_str, sizeof(draw_str), "%zu/%zu", m->dict_keys_current, m->dict_keys_total);
        }
    }
    if(dict_progress > 1.0f) {
        dict_progress = 1.0f;
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
    snprintf(draw_str, sizeof(draw_str), "Sectors Read: %d/%d", m->sectors_read, m->sectors_total);
    canvas_draw_str_aligned(canvas, 0, 43, AlignLeft, AlignTop, draw_str);
}

static void dict_attack_draw_mf_ultralight_c(Canvas* canvas, DictAttackViewModel* m) {
    char draw_str[32] = {};
    canvas_set_font(canvas, FontSecondary);

    canvas_draw_str_aligned(canvas, 0, 0, AlignLeft, AlignTop, furi_string_get_cstr(m->header));

    snprintf(draw_str, sizeof(draw_str), "Trying keys");
    canvas_draw_str_aligned(canvas, 0, 10, AlignLeft, AlignTop, draw_str);

    float dict_progress =
        m->dict_keys_total == 0 ? 0 : (float)(m->dict_keys_current) / (float)(m->dict_keys_total);
    if(m->dict_keys_current == 0) {
        snprintf(draw_str, sizeof(draw_str), "%d/%zu", 1, m->dict_keys_total);
    } else {
        snprintf(draw_str, sizeof(draw_str), "%zu/%zu", m->dict_keys_current, m->dict_keys_total);
    }
    if(dict_progress > 1.0f) {
        dict_progress = 1.0f;
    }
    elements_progress_bar_with_text(canvas, 0, 20, 128, dict_progress, draw_str);

    canvas_set_font(canvas, FontSecondary);
    snprintf(draw_str, sizeof(draw_str), "Key found: %s", m->key_found ? "Yes" : "No");
    canvas_draw_str_aligned(canvas, 0, 33, AlignLeft, AlignTop, draw_str);

    snprintf(draw_str, sizeof(draw_str), "Pages read: %d/%d", m->pages_read, m->pages_total);
    canvas_draw_str_aligned(canvas, 0, 43, AlignLeft, AlignTop, draw_str);
}

static void dict_attack_draw_callback(Canvas* canvas, void* model) {
    DictAttackViewModel* m = model;
    if(!m->card_detected) {
        canvas_set_font(canvas, FontPrimary);
        canvas_draw_str_aligned(canvas, 64, 4, AlignCenter, AlignTop, "Lost the tag!");
        canvas_set_font(canvas, FontSecondary);
        elements_multiline_text_aligned(
            canvas, 64, 23, AlignCenter, AlignTop, "Make sure the tag is\npositioned correctly.");
    } else {
        if(m->attack_type == DictAttackTypeMfClassic) {
            dict_attack_draw_mf_classic(canvas, m);
        } else if(m->attack_type == DictAttackTypeMfUltralightC) {
            dict_attack_draw_mf_ultralight_c(canvas, m);
        }
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

DictAttack* dict_attack_alloc(void) {
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
            model->attack_type = DictAttackTypeMfClassic;

            // MIFARE Classic fields
            model->sectors_total = 0;
            model->sectors_read = 0;
            model->current_sector = 0;
            model->keys_found = 0;
            model->is_key_attack = false;
            model->nested_phase = MfClassicNestedPhaseNone;
            model->prng_type = MfClassicPrngTypeUnknown;
            model->backdoor = MfClassicBackdoorUnknown;
            model->nested_target_key = 0;
            model->msb_count = 0;

            // Ultralight C fields
            model->pages_total = 0;
            model->pages_read = 0;
            model->key_found = false;

            // Common fields
            model->dict_keys_total = 0;
            model->dict_keys_current = 0;
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

void dict_attack_set_nested_phase(DictAttack* instance, MfClassicNestedPhase nested_phase) {
    furi_assert(instance);

    with_view_model(
        instance->view, DictAttackViewModel * model, { model->nested_phase = nested_phase; }, true);
}

void dict_attack_set_prng_type(DictAttack* instance, MfClassicPrngType prng_type) {
    furi_assert(instance);

    with_view_model(
        instance->view, DictAttackViewModel * model, { model->prng_type = prng_type; }, true);
}

void dict_attack_set_backdoor(DictAttack* instance, MfClassicBackdoor backdoor) {
    furi_assert(instance);

    with_view_model(
        instance->view, DictAttackViewModel * model, { model->backdoor = backdoor; }, true);
}

void dict_attack_set_nested_target_key(DictAttack* instance, uint16_t nested_target_key) {
    furi_assert(instance);

    with_view_model(
        instance->view,
        DictAttackViewModel * model,
        { model->nested_target_key = nested_target_key; },
        true);
}

void dict_attack_set_msb_count(DictAttack* instance, uint16_t msb_count) {
    furi_assert(instance);

    with_view_model(
        instance->view, DictAttackViewModel * model, { model->msb_count = msb_count; }, true);
}

void dict_attack_set_type(DictAttack* instance, DictAttackType type) {
    furi_assert(instance);

    with_view_model(
        instance->view, DictAttackViewModel * model, { model->attack_type = type; }, true);
}

void dict_attack_set_pages_total(DictAttack* instance, uint8_t pages_total) {
    furi_assert(instance);

    with_view_model(
        instance->view, DictAttackViewModel * model, { model->pages_total = pages_total; }, true);
}

void dict_attack_set_pages_read(DictAttack* instance, uint8_t pages_read) {
    furi_assert(instance);

    with_view_model(
        instance->view, DictAttackViewModel * model, { model->pages_read = pages_read; }, true);
}

void dict_attack_set_key_found(DictAttack* instance, bool key_found) {
    furi_assert(instance);

    with_view_model(
        instance->view, DictAttackViewModel * model, { model->key_found = key_found; }, true);
}
