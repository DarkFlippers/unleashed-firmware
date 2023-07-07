#include "nfc_rfid_detector_view_field_presence.h"
#include "../nfc_rfid_detector_app_i.h"
#include <nfc_rfid_detector_icons.h>

#include <input/input.h>
#include <gui/elements.h>

#define FIELD_FOUND_WEIGHT 5

typedef enum {
    NfcRfidDetectorTypeFieldPresenceNfc,
    NfcRfidDetectorTypeFieldPresenceRfid,
} NfcRfidDetectorTypeFieldPresence;

static const Icon* NfcRfidDetectorFieldPresenceIcons[] = {
    [NfcRfidDetectorTypeFieldPresenceNfc] = &I_NFC_detect_45x30,
    [NfcRfidDetectorTypeFieldPresenceRfid] = &I_Rfid_detect_45x30,
};

struct NfcRfidDetectorFieldPresence {
    View* view;
};

typedef struct {
    uint8_t nfc_field;
    uint8_t rfid_field;
    uint32_t rfid_frequency;
} NfcRfidDetectorFieldPresenceModel;

void nfc_rfid_detector_view_field_presence_update(
    NfcRfidDetectorFieldPresence* instance,
    bool nfc_field,
    bool rfid_field,
    uint32_t rfid_frequency) {
    furi_assert(instance);
    with_view_model(
        instance->view,
        NfcRfidDetectorFieldPresenceModel * model,
        {
            if(nfc_field) {
                model->nfc_field = FIELD_FOUND_WEIGHT;
            } else if(model->nfc_field) {
                model->nfc_field--;
            }
            if(rfid_field) {
                model->rfid_field = FIELD_FOUND_WEIGHT;
                model->rfid_frequency = rfid_frequency;
            } else if(model->rfid_field) {
                model->rfid_field--;
            }
        },
        true);
}

void nfc_rfid_detector_view_field_presence_draw(
    Canvas* canvas,
    NfcRfidDetectorFieldPresenceModel* model) {
    canvas_clear(canvas);
    canvas_set_color(canvas, ColorBlack);

    if(!model->nfc_field && !model->rfid_field) {
        canvas_draw_icon(canvas, 0, 16, &I_Modern_reader_18x34);
        canvas_draw_icon(canvas, 22, 12, &I_Move_flipper_26x39);
        canvas_set_font(canvas, FontSecondary);
        canvas_draw_str(canvas, 56, 36, "Touch the reader");
    } else {
        if(model->nfc_field) {
            canvas_set_font(canvas, FontPrimary);
            canvas_draw_str(canvas, 21, 10, "NFC");
            canvas_draw_icon(
                canvas,
                9,
                17,
                NfcRfidDetectorFieldPresenceIcons[NfcRfidDetectorTypeFieldPresenceNfc]);
            canvas_set_font(canvas, FontSecondary);
            canvas_draw_str(canvas, 9, 62, "13,56 MHz");
        }

        if(model->rfid_field) {
            char str[16];
            snprintf(str, sizeof(str), "%.02f KHz", (double)model->rfid_frequency / 1000);
            canvas_set_font(canvas, FontPrimary);
            canvas_draw_str(canvas, 76, 10, "LF RFID");
            canvas_draw_icon(
                canvas,
                71,
                17,
                NfcRfidDetectorFieldPresenceIcons[NfcRfidDetectorTypeFieldPresenceRfid]);
            canvas_set_font(canvas, FontSecondary);
            canvas_draw_str(canvas, 69, 62, str);
        }
    }
}

bool nfc_rfid_detector_view_field_presence_input(InputEvent* event, void* context) {
    furi_assert(context);
    NfcRfidDetectorFieldPresence* instance = context;
    UNUSED(instance);

    if(event->key == InputKeyBack) {
        return false;
    }

    return true;
}

void nfc_rfid_detector_view_field_presence_enter(void* context) {
    furi_assert(context);
    NfcRfidDetectorFieldPresence* instance = context;
    with_view_model(
        instance->view,
        NfcRfidDetectorFieldPresenceModel * model,
        {
            model->nfc_field = 0;
            model->rfid_field = 0;
            model->rfid_frequency = 0;
        },
        true);
}

void nfc_rfid_detector_view_field_presence_exit(void* context) {
    furi_assert(context);
    NfcRfidDetectorFieldPresence* instance = context;
    UNUSED(instance);
}

NfcRfidDetectorFieldPresence* nfc_rfid_detector_view_field_presence_alloc() {
    NfcRfidDetectorFieldPresence* instance = malloc(sizeof(NfcRfidDetectorFieldPresence));

    // View allocation and configuration
    instance->view = view_alloc();

    view_allocate_model(
        instance->view, ViewModelTypeLocking, sizeof(NfcRfidDetectorFieldPresenceModel));
    view_set_context(instance->view, instance);
    view_set_draw_callback(
        instance->view, (ViewDrawCallback)nfc_rfid_detector_view_field_presence_draw);
    view_set_input_callback(instance->view, nfc_rfid_detector_view_field_presence_input);
    view_set_enter_callback(instance->view, nfc_rfid_detector_view_field_presence_enter);
    view_set_exit_callback(instance->view, nfc_rfid_detector_view_field_presence_exit);

    with_view_model(
        instance->view,
        NfcRfidDetectorFieldPresenceModel * model,
        {
            model->nfc_field = 0;
            model->rfid_field = 0;
            model->rfid_frequency = 0;
        },
        true);
    return instance;
}

void nfc_rfid_detector_view_field_presence_free(NfcRfidDetectorFieldPresence* instance) {
    furi_assert(instance);

    view_free(instance->view);
    free(instance);
}

View* nfc_rfid_detector_view_field_presence_get_view(NfcRfidDetectorFieldPresence* instance) {
    furi_assert(instance);
    return instance->view;
}
