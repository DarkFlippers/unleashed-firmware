#include "nfc_mifare_ul.h"

#include <furi.h>
#include <api-hal.h>
#include <input/input.h>

#include "../nfc_i.h"

struct NfcMifareUl {
    NfcCommon* nfc_common;
    View* view;
};

typedef struct {
    bool found;
    NfcMifareUlData nfc_mf_ul_data;
} NfcMifareUlModel;

void nfc_mifare_ul_draw(Canvas* canvas, NfcMifareUlModel* model) {
    char buffer[32];
    canvas_clear(canvas);
    canvas_set_font(canvas, FontPrimary);
    if(model->found) {
        canvas_draw_str(canvas, 0, 12, "Found Mifare Ultralight");
        canvas_set_font(canvas, FontSecondary);
        canvas_draw_str(canvas, 2, 22, "UID:");
        for(uint8_t i = 0; i < model->nfc_mf_ul_data.nfc_data.uid_len; i++) {
            snprintf(
                buffer + (i * 2),
                sizeof(buffer) - (i * 2),
                "%02X",
                model->nfc_mf_ul_data.nfc_data.uid[i]);
        }
        buffer[model->nfc_mf_ul_data.nfc_data.uid_len * 2] = 0;
        canvas_draw_str(canvas, 18, 22, buffer);

        uint8_t man_bl_size = sizeof(model->nfc_mf_ul_data.man_block);
        canvas_draw_str(canvas, 2, 32, "Manufacturer block:");
        for(uint8_t i = 0; i < man_bl_size / 2; i++) {
            snprintf(
                buffer + (i * 2),
                sizeof(buffer) - (i * 2),
                "%02X",
                model->nfc_mf_ul_data.man_block[i]);
        }
        buffer[man_bl_size] = 0;
        canvas_draw_str(canvas, 2, 42, buffer);

        for(uint8_t i = 0; i < man_bl_size / 2; i++) {
            snprintf(
                buffer + (i * 2),
                sizeof(buffer) - (i * 2),
                "%02X",
                model->nfc_mf_ul_data.man_block[man_bl_size / 2 + i]);
        }
        buffer[man_bl_size] = 0;
        canvas_draw_str(canvas, 2, 52, buffer);

        canvas_draw_str(canvas, 2, 62, "OTP: ");
        for(uint8_t i = 0; i < sizeof(model->nfc_mf_ul_data.otp); i++) {
            snprintf(
                buffer + (i * 2), sizeof(buffer) - (i * 2), "%02X", model->nfc_mf_ul_data.otp[i]);
        }
        buffer[sizeof(model->nfc_mf_ul_data.otp) * 2] = 0;
        canvas_draw_str(canvas, 22, 62, buffer);
    } else {
        canvas_draw_str(canvas, 0, 12, "Searching");
        canvas_set_font(canvas, FontSecondary);
        canvas_draw_str(canvas, 2, 22, "Place card to the back");
    }
}

bool nfc_mifare_ul_input(InputEvent* event, void* context) {
    if(event->key == InputKeyBack) {
        return false;
    }
    return true;
}

void nfc_mifare_ul_worker_callback(void* context) {
    furi_assert(context);

    NfcMifareUl* nfc_mifare_ul = (NfcMifareUl*)context;
    view_dispatcher_send_custom_event(
        nfc_mifare_ul->nfc_common->view_dispatcher, NfcEventMifareUl);
}

bool nfc_mifare_ul_custom(uint32_t event, void* context) {
    furi_assert(context);

    NfcMifareUl* nfc_mifare_ul = (NfcMifareUl*)context;
    if(event == NfcEventMifareUl) {
        NfcMifareUlData* data = (NfcMifareUlData*)&nfc_mifare_ul->nfc_common->worker_result;

        with_view_model(
            nfc_mifare_ul->view, (NfcMifareUlModel * model) {
                model->found = true;
                model->nfc_mf_ul_data = *data;
                return true;
            });
        // TODO add and configure next view model
        return true;
    }

    return false;
}

void nfc_mifare_ul_enter(void* context) {
    furi_assert(context);

    NfcMifareUl* nfc_mifare_ul = (NfcMifareUl*)context;
    with_view_model(
        nfc_mifare_ul->view, (NfcMifareUlModel * m) {
            m->found = false;
            return true;
        });
    nfc_worker_start(
        nfc_mifare_ul->nfc_common->worker,
        NfcWorkerStateReadMfUltralight,
        &nfc_mifare_ul->nfc_common->worker_result,
        nfc_mifare_ul_worker_callback,
        nfc_mifare_ul);
}

void nfc_mifare_ul_exit(void* context) {
    furi_assert(context);

    NfcMifareUl* nfc_mifare_ul = (NfcMifareUl*)context;
    nfc_worker_stop(nfc_mifare_ul->nfc_common->worker);
}

NfcMifareUl* nfc_mifare_ul_alloc(NfcCommon* nfc_common) {
    furi_assert(nfc_common);

    NfcMifareUl* nfc_mifare_ul = furi_alloc(sizeof(NfcMifareUl));
    nfc_mifare_ul->nfc_common = nfc_common;

    // View allocation and configuration
    nfc_mifare_ul->view = view_alloc();
    view_allocate_model(nfc_mifare_ul->view, ViewModelTypeLockFree, sizeof(NfcMifareUlModel));
    view_set_context(nfc_mifare_ul->view, nfc_mifare_ul);
    view_set_draw_callback(nfc_mifare_ul->view, (ViewDrawCallback)nfc_mifare_ul_draw);
    view_set_input_callback(nfc_mifare_ul->view, nfc_mifare_ul_input);
    view_set_custom_callback(nfc_mifare_ul->view, nfc_mifare_ul_custom);
    view_set_enter_callback(nfc_mifare_ul->view, nfc_mifare_ul_enter);
    view_set_exit_callback(nfc_mifare_ul->view, nfc_mifare_ul_exit);

    return nfc_mifare_ul;
}

void nfc_mifare_ul_free(NfcMifareUl* nfc_mifare_ul) {
    furi_assert(nfc_mifare_ul);

    view_free(nfc_mifare_ul->view);
    free(nfc_mifare_ul);
}

View* nfc_mifare_ul_get_view(NfcMifareUl* nfc_mifare_ul) {
    furi_assert(nfc_mifare_ul);

    return nfc_mifare_ul->view;
}