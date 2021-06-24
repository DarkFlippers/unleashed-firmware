#include "nfc_emv.h"

#include "nfc_i.h"
#include "nfc_types.h"
#include <furi.h>
#include <api-hal.h>
#include <input/input.h>

struct NfcEmv {
    NfcCommon* nfc_common;
    View* view;
};

typedef struct {
    bool found;
    NfcEmvData emv_data;
} NfcEmvModel;

void nfc_emv_draw(Canvas* canvas, NfcEmvModel* model) {
    char buffer[32];
    canvas_clear(canvas);
    canvas_set_font(canvas, FontPrimary);
    if(model->found) {
        canvas_draw_str(canvas, 0, 12, "Found EMV card");
        canvas_set_font(canvas, FontSecondary);
        canvas_draw_str(canvas, 2, 22, "Type:");
        snprintf(buffer, sizeof(buffer), "%s", model->emv_data.name);
        canvas_draw_str(canvas, 2, 32, buffer);
        snprintf(buffer, sizeof(buffer), "Number:\n");
        canvas_draw_str(canvas, 2, 42, buffer);
        uint8_t card_num_len = sizeof(model->emv_data.number);
        for(uint8_t i = 0; i < card_num_len; i++) {
            snprintf(
                buffer + (i * 2), sizeof(buffer) - (i * 2), "%02X", model->emv_data.number[i]);
        }
        buffer[card_num_len * 2] = 0;
        canvas_draw_str(canvas, 2, 52, buffer);
    } else {
        canvas_draw_str(canvas, 0, 12, "Searching");
        canvas_set_font(canvas, FontSecondary);
        canvas_draw_str(canvas, 2, 22, "Place card to the back");
    }
}

bool nfc_emv_input(InputEvent* event, void* context) {
    if(event->key == InputKeyBack) {
        return false;
    }
    return true;
}

void nfc_emv_worker_callback(void* context) {
    furi_assert(context);

    NfcEmv* nfc_emv = (NfcEmv*)context;
    view_dispatcher_send_custom_event(nfc_emv->nfc_common->view_dispatcher, NfcEventEmv);
}

void nfc_emv_view_dispatcher_callback(NfcEmv* nfc_emv, NfcMessage* message) {
    furi_assert(nfc_emv);
    furi_assert(message);

    if(message->found) {
        with_view_model(
            nfc_emv->view, (NfcEmvModel * model) {
                model->found = true;
                model->emv_data = message->nfc_emv_data;
                return true;
            });
    } else {
        with_view_model(
            nfc_emv->view, (NfcEmvModel * model) {
                model->found = false;
                return true;
            });
    }
}

void nfc_emv_enter(void* context) {
    furi_assert(context);

    NfcEmv* nfc_emv = (NfcEmv*)context;
    with_view_model(
        nfc_emv->view, (NfcEmvModel * model) {
            model->found = false;
            return true;
        });
    nfc_worker_start(
        nfc_emv->nfc_common->worker, NfcWorkerStateReadEMV, nfc_emv_worker_callback, nfc_emv);
}

void nfc_emv_exit(void* context) {
    furi_assert(context);

    NfcEmv* nfc_emv = (NfcEmv*)context;
    nfc_worker_stop(nfc_emv->nfc_common->worker);
}

uint32_t nfc_emv_back(void* context) {
    return NfcViewMenu;
}

NfcEmv* nfc_emv_alloc(NfcCommon* nfc_common) {
    furi_assert(nfc_common);

    NfcEmv* nfc_emv = furi_alloc(sizeof(NfcEmv));
    nfc_emv->nfc_common = nfc_common;

    // View allocation and configuration
    nfc_emv->view = view_alloc();
    view_allocate_model(nfc_emv->view, ViewModelTypeLockFree, sizeof(NfcEmvModel));
    view_set_context(nfc_emv->view, nfc_emv);
    view_set_draw_callback(nfc_emv->view, (ViewDrawCallback)nfc_emv_draw);
    view_set_input_callback(nfc_emv->view, nfc_emv_input);
    view_set_enter_callback(nfc_emv->view, nfc_emv_enter);
    view_set_exit_callback(nfc_emv->view, nfc_emv_exit);
    view_set_previous_callback(nfc_emv->view, nfc_emv_back);

    return nfc_emv;
}

void nfc_emv_free(NfcEmv* nfc_emv) {
    furi_assert(nfc_emv);

    view_free(nfc_emv->view);
    free(nfc_emv);
}

View* nfc_emv_get_view(NfcEmv* nfc_emv) {
    furi_assert(nfc_emv);

    return nfc_emv->view;
}