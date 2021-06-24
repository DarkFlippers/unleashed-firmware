#include "nfc_detect.h"

#include "nfc_i.h"
#include "nfc_types.h"
#include <furi.h>
#include <api-hal.h>
#include <input/input.h>

struct NfcDetect {
    NfcCommon* nfc_common;
    View* view;
};

typedef struct {
    bool found;
    NfcDeviceData data;
} NfcDetectModel;

void nfc_detect_draw(Canvas* canvas, NfcDetectModel* model) {
    char buffer[32];
    canvas_clear(canvas);
    canvas_set_font(canvas, FontPrimary);
    if(model->found) {
        canvas_draw_str(canvas, 0, 12, "Found");
        canvas_draw_str(canvas, 32, 12, nfc_get_dev_type(model->data.device));
        canvas_set_font(canvas, FontSecondary);
        if(model->data.protocol != NfcDeviceProtocolUnknown) {
            canvas_draw_str(canvas, 0, 22, nfc_get_protocol(model->data.protocol));
        }
        // Display UID
        for(uint8_t i = 0; i < model->data.uid_len; i++) {
            snprintf(buffer + (i * 2), sizeof(buffer) - (i * 2), "%02X", model->data.uid[i]);
            buffer[model->data.uid_len * 2] = 0;
        }
        canvas_draw_str(canvas, 0, 32, "UID: ");
        canvas_draw_str(canvas, 22, 32, buffer);
        // Display ATQA and SAK
        snprintf(
            buffer,
            sizeof(buffer),
            "ATQA: %02X %02X   SAK: %02X",
            model->data.atqa[1],
            model->data.atqa[0],
            model->data.sak);
        canvas_draw_str(canvas, 0, 42, buffer);
    } else {
        canvas_draw_str(canvas, 0, 12, "Searching");
        canvas_set_font(canvas, FontSecondary);
        canvas_draw_str(canvas, 2, 22, "Place card to the back");
    }
}

bool nfc_detect_input(InputEvent* event, void* context) {
    if(event->key == InputKeyBack) {
        return false;
    }
    return true;
}

void nfc_detect_worker_callback(void* context) {
    furi_assert(context);

    NfcDetect* nfc_detect = (NfcDetect*)context;
    view_dispatcher_send_custom_event(nfc_detect->nfc_common->view_dispatcher, NfcEventDetect);
}

void nfc_detect_view_dispatcher_callback(NfcDetect* nfc_detect, NfcMessage* message) {
    furi_assert(nfc_detect);
    furi_assert(message);

    if(message->found) {
        with_view_model(
            nfc_detect->view, (NfcDetectModel * model) {
                model->found = true;
                model->data = message->nfc_detect_data;
                return true;
            });
    } else {
        with_view_model(
            nfc_detect->view, (NfcDetectModel * model) {
                model->found = false;
                return true;
            });
    }
}

void nfc_detect_enter(void* context) {
    furi_assert(context);

    NfcDetect* nfc_detect = (NfcDetect*)context;
    with_view_model(
        nfc_detect->view, (NfcDetectModel * model) {
            model->found = false;
            model->data.protocol = NfcDeviceProtocolUnknown;
            return true;
        });
    nfc_worker_start(
        nfc_detect->nfc_common->worker,
        NfcWorkerStateDetect,
        nfc_detect_worker_callback,
        nfc_detect);
}

void nfc_detect_exit(void* context) {
    furi_assert(context);

    NfcDetect* nfc_detect = (NfcDetect*)context;
    nfc_worker_stop(nfc_detect->nfc_common->worker);
}

uint32_t nfc_detect_back(void* context) {
    return NfcViewMenu;
}

NfcDetect* nfc_detect_alloc(NfcCommon* nfc_common) {
    furi_assert(nfc_common);

    NfcDetect* nfc_detect = furi_alloc(sizeof(NfcDetect));
    nfc_detect->nfc_common = nfc_common;

    // View allocation and configuration
    nfc_detect->view = view_alloc();
    view_allocate_model(nfc_detect->view, ViewModelTypeLockFree, sizeof(NfcDetectModel));
    view_set_context(nfc_detect->view, nfc_detect);
    view_set_draw_callback(nfc_detect->view, (ViewDrawCallback)nfc_detect_draw);
    view_set_input_callback(nfc_detect->view, nfc_detect_input);
    view_set_enter_callback(nfc_detect->view, nfc_detect_enter);
    view_set_exit_callback(nfc_detect->view, nfc_detect_exit);
    view_set_previous_callback(nfc_detect->view, nfc_detect_back);

    return nfc_detect;
}

void nfc_detect_free(NfcDetect* nfc_detect) {
    furi_assert(nfc_detect);

    view_free(nfc_detect->view);
    free(nfc_detect);
}

View* nfc_detect_get_view(NfcDetect* nfc_detect) {
    furi_assert(nfc_detect);

    return nfc_detect->view;
}