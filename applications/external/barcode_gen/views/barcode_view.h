#pragma once

#include <gui/view.h>

typedef struct BarcodeApp BarcodeApp;

typedef struct {
    View* view;
    BarcodeApp* barcode_app;
} Barcode;

typedef struct {
    FuriString* file_path;
    BarcodeData* data;
} BarcodeModel;

Barcode* barcode_view_allocate(BarcodeApp* barcode_app);

void barcode_free_model(Barcode* barcode);

void barcode_free(Barcode* barcode);

View* barcode_get_view(Barcode* barcode);
