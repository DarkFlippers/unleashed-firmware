#include "nfc_views.h"

void nfc_view_read_draw(Canvas* canvas, void* model) {
    NfcViewReadModel* m = model;
    canvas_clear(canvas);
    canvas_set_font(canvas, FontPrimary);
    if(m->found) {
        if(m->device.type == NfcDeviceTypeNfca) {
            nfc_view_read_nfca_draw(canvas, m);
        } else if(m->device.type == NfcDeviceTypeNfcb) {
            nfc_view_read_nfcb_draw(canvas, m);
        } else if(m->device.type == NfcDeviceTypeNfcv) {
            nfc_view_read_nfcv_draw(canvas, m);
        } else if(m->device.type == NfcDeviceTypeNfcf) {
            nfc_view_read_nfcf_draw(canvas, m);
        }
    } else {
        canvas_draw_str(canvas, 0, 12, "Searching");
        canvas_set_font(canvas, FontSecondary);
        canvas_draw_str(canvas, 2, 22, "Place card to the back");
    }
}

void nfc_view_read_nfca_draw(Canvas* canvas, NfcViewReadModel* model) {
    char buffer[32];
    canvas_draw_str(canvas, 0, 12, "Found NFC-A");
    canvas_set_font(canvas, FontSecondary);
    snprintf(buffer, sizeof(buffer), "Type: %s", nfc_get_nfca_type(model->device.nfca.type));
    canvas_draw_str(canvas, 2, 22, buffer);
    snprintf(buffer, sizeof(buffer), "UID length: %d", model->device.nfca.nfcId1Len);
    canvas_draw_str(canvas, 2, 32, buffer);

    canvas_draw_str(canvas, 2, 42, "UID:");
    for(uint8_t i = 0; i < model->device.nfca.nfcId1Len; i++) {
        snprintf(buffer + (i * 2), sizeof(buffer) - (i * 2), "%02X", model->device.nfca.nfcId1[i]);
    }
    buffer[model->device.nfca.nfcId1Len * 2] = 0;
    canvas_draw_str(canvas, 18, 42, buffer);

    snprintf(
        buffer,
        sizeof(buffer),
        "SAK: %02X ATQA: %02X/%02X",
        model->device.nfca.selRes.sak,
        model->device.nfca.sensRes.anticollisionInfo,
        model->device.nfca.sensRes.platformInfo);
    canvas_draw_str(canvas, 2, 52, buffer);
}

void nfc_view_read_nfcb_draw(Canvas* canvas, NfcViewReadModel* model) {
    char buffer[32];
    canvas_draw_str(canvas, 0, 12, "Found NFC-B");
    canvas_set_font(canvas, FontSecondary);

    snprintf(buffer, sizeof(buffer), "UID length: %d", RFAL_NFCB_NFCID0_LEN);
    canvas_draw_str(canvas, 2, 32, buffer);

    canvas_draw_str(canvas, 2, 42, "UID:");
    for(uint8_t i = 0; i < RFAL_NFCB_NFCID0_LEN; i++) {
        snprintf(
            buffer + (i * 2),
            sizeof(buffer) - (i * 2),
            "%02X",
            model->device.nfcb.sensbRes.nfcid0[i]);
    }
    buffer[RFAL_NFCB_NFCID0_LEN * 2] = 0;
    canvas_draw_str(canvas, 18, 42, buffer);
}

void nfc_view_read_nfcf_draw(Canvas* canvas, NfcViewReadModel* model) {
    char buffer[32];
    canvas_draw_str(canvas, 0, 12, "Found NFC-F");
    canvas_set_font(canvas, FontSecondary);

    snprintf(buffer, sizeof(buffer), "UID length: %d", RFAL_NFCF_NFCID2_LEN);
    canvas_draw_str(canvas, 2, 32, buffer);

    canvas_draw_str(canvas, 2, 42, "UID:");
    for(uint8_t i = 0; i < RFAL_NFCF_NFCID2_LEN; i++) {
        snprintf(
            buffer + (i * 2),
            sizeof(buffer) - (i * 2),
            "%02X",
            model->device.nfcf.sensfRes.NFCID2[i]);
    }
    buffer[RFAL_NFCF_NFCID2_LEN * 2] = 0;
    canvas_draw_str(canvas, 18, 42, buffer);
}

void nfc_view_read_nfcv_draw(Canvas* canvas, NfcViewReadModel* model) {
    char buffer[32];
    canvas_draw_str(canvas, 0, 12, "Found NFC-V");
    canvas_set_font(canvas, FontSecondary);

    snprintf(buffer, sizeof(buffer), "UID length: %d", RFAL_NFCV_UID_LEN);
    canvas_draw_str(canvas, 2, 32, buffer);

    canvas_draw_str(canvas, 2, 42, "UID:");
    for(uint8_t i = 0; i < RFAL_NFCV_UID_LEN; i++) {
        snprintf(
            buffer + (i * 2), sizeof(buffer) - (i * 2), "%02X", model->device.nfcv.InvRes.UID[i]);
    }
    buffer[RFAL_NFCV_UID_LEN * 2] = 0;
    canvas_draw_str(canvas, 18, 42, buffer);
}

void nfc_view_emulate_draw(Canvas* canvas, void* model) {
    canvas_clear(canvas);
    canvas_set_font(canvas, FontPrimary);
    canvas_draw_str(canvas, 0, 12, "Emulating NFC-A");
    canvas_set_font(canvas, FontSecondary);
    canvas_draw_str(canvas, 2, 22, "Type: T2T");
    canvas_draw_str(canvas, 2, 32, "UID length: 7");
    canvas_draw_str(canvas, 2, 42, "UID: 00010203040506");
    canvas_draw_str(canvas, 2, 52, "SAK: 00 ATQA: 44/00");
}

void nfc_view_field_draw(Canvas* canvas, void* model) {
    canvas_clear(canvas);
    canvas_set_font(canvas, FontPrimary);
    canvas_draw_str(canvas, 0, 12, "Field ON");
    canvas_set_font(canvas, FontSecondary);
    canvas_draw_str(canvas, 2, 22, "TX/RX is disabled");
}

void nfc_view_cli_draw(Canvas* canvas, void* model) {
    canvas_clear(canvas);
    canvas_set_font(canvas, FontPrimary);
    canvas_draw_str(canvas, 0, 12, "USB connected");
    canvas_set_font(canvas, FontSecondary);
    canvas_draw_str(canvas, 2, 22, "Cli command in process...");
}

void nfc_view_error_draw(Canvas* canvas, void* model) {
    NfcViewErrorModel* m = model;
    char buffer[32];

    canvas_clear(canvas);
    canvas_set_font(canvas, FontPrimary);
    snprintf(buffer, sizeof(buffer), "Error: %d", m->error);
    canvas_draw_str(canvas, 0, 12, buffer);

    canvas_set_font(canvas, FontSecondary);
    if(m->error == ERR_WRONG_STATE) {
        canvas_draw_str(canvas, 2, 22, "Wrong State");
    } else if(m->error == ERR_PARAM) {
        canvas_draw_str(canvas, 2, 22, "Wrong Param");
    } else if(m->error == ERR_HW_MISMATCH) {
        canvas_draw_str(canvas, 2, 22, "HW mismatch");
    } else if(m->error == ERR_IO) {
        canvas_draw_str(canvas, 2, 22, "IO Error");
    } else {
        canvas_draw_str(canvas, 2, 22, "Details in st_errno.h");
    }
}
