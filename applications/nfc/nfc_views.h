#pragma once

#include <stdint.h>
#include <stdbool.h>
#include <gui/canvas.h>
#include <furi.h>

#include "nfc_types.h"

typedef enum {
    NfcViewMenu,
    NfcViewRead,
    NfcViewReadEmv,
    NfcViewEmulateEMV,
    NfcViewEmulate,
    NfcViewField,
    NfcViewReadMfUltralight,
    NfcViewError,
} NfcView;

typedef struct {
    bool found;
    NfcDevice device;
} NfcViewReadModel;

void nfc_view_read_draw(Canvas* canvas, void* model);
void nfc_view_read_nfca_draw(Canvas* canvas, NfcViewReadModel* model);
void nfc_view_read_nfcb_draw(Canvas* canvas, NfcViewReadModel* model);
void nfc_view_read_nfcf_draw(Canvas* canvas, NfcViewReadModel* model);
void nfc_view_read_nfcv_draw(Canvas* canvas, NfcViewReadModel* model);
void nfc_view_read_emv_draw(Canvas* canvas, void* model);

void nfc_view_emulate_emv_draw(Canvas* canvas, void* model);
void nfc_view_emulate_draw(Canvas* canvas, void* model);
void nfc_view_read_mf_ultralight_draw(Canvas* canvas, void* model);

void nfc_view_field_draw(Canvas* canvas, void* model);

typedef struct {
    ReturnCode error;
} NfcViewErrorModel;

void nfc_view_error_draw(Canvas* canvas, void* model);
