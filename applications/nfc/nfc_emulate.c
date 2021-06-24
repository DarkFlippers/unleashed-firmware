#include "nfc_emulate.h"

#include "nfc_i.h"
#include "nfc_types.h"
#include <furi.h>
#include <api-hal.h>
#include <input/input.h>

struct NfcEmulate {
    NfcCommon* nfc_common;
    View* view;
};

void nfc_emulate_draw(Canvas* canvas, void* model) {
    canvas_clear(canvas);
    canvas_set_font(canvas, FontPrimary);
    canvas_draw_str(canvas, 0, 12, "Emulating NFC-A");
    canvas_set_font(canvas, FontSecondary);
    canvas_draw_str(canvas, 2, 22, "Type: T2T");
    canvas_draw_str(canvas, 2, 32, "UID length: 7");
    canvas_draw_str(canvas, 2, 42, "UID: 36 9C E7 B1 0A C1 34");
    canvas_draw_str(canvas, 2, 52, "SAK: 00 ATQA: 00/44");
}

bool nfc_emulate_input(InputEvent* event, void* context) {
    if(event->key == InputKeyBack) {
        return false;
    }
    return true;
}

void nfc_emulate_enter(void* context) {
    furi_assert(context);

    NfcEmulate* nfc_emulate = (NfcEmulate*)context;
    nfc_worker_start(nfc_emulate->nfc_common->worker, NfcWorkerStateEmulate, NULL, NULL);
}

void nfc_emulate_exit(void* context) {
    furi_assert(context);

    NfcEmulate* nfc_emulate = (NfcEmulate*)context;
    nfc_worker_stop(nfc_emulate->nfc_common->worker);
}

uint32_t nfc_emulate_back(void* context) {
    return NfcViewMenu;
}

NfcEmulate* nfc_emulate_alloc(NfcCommon* nfc_common) {
    furi_assert(nfc_common);

    NfcEmulate* nfc_emulate = furi_alloc(sizeof(NfcEmulate));
    nfc_emulate->nfc_common = nfc_common;

    // View allocation and configuration
    nfc_emulate->view = view_alloc();
    view_set_context(nfc_emulate->view, nfc_emulate);
    view_set_draw_callback(nfc_emulate->view, (ViewDrawCallback)nfc_emulate_draw);
    view_set_input_callback(nfc_emulate->view, nfc_emulate_input);
    view_set_enter_callback(nfc_emulate->view, nfc_emulate_enter);
    view_set_exit_callback(nfc_emulate->view, nfc_emulate_exit);
    view_set_previous_callback(nfc_emulate->view, nfc_emulate_back);

    return nfc_emulate;
}

void nfc_emulate_free(NfcEmulate* nfc_emulate) {
    furi_assert(nfc_emulate);

    view_free(nfc_emulate->view);
    free(nfc_emulate);
}

View* nfc_emulate_get_view(NfcEmulate* nfc_emulate) {
    furi_assert(nfc_emulate);

    return nfc_emulate->view;
}