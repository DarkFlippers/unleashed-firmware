#pragma once

#include <gui/view.h>

typedef struct BarcodeApp BarcodeApp;

typedef struct {
    View* view;
    BarcodeApp* barcode_app;
} MessageView;

typedef struct {
    const char* message;
} MessageViewModel;

MessageView* message_view_allocate(BarcodeApp* barcode_app);

void message_view_free_model(MessageView* message_view_object);

void message_view_free(MessageView* message_view_object);

View* message_get_view(MessageView* message_view_object);
