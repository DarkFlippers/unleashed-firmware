#pragma once

#include <gui/view.h>

typedef struct BarcodeApp BarcodeApp;

typedef enum {
    FileNameSetter,
    BarcodeDataSetter
} InputSetter; //what value to set for the text input view

typedef enum {
    EditMode,

    NewMode
} CreateMode;

typedef struct {
    View* view;
    BarcodeApp* barcode_app;

    InputSetter setter;
    char input[TEXT_BUFFER_SIZE];
} CreateView;

typedef struct {
    int selected_menu_item;

    CreateMode mode;
    BarcodeTypeObj* barcode_type;
    FuriString* file_path; //the current file that is opened
    FuriString* file_name;
    FuriString* barcode_data;
} CreateViewModel;

CreateView* create_view_allocate(BarcodeApp* barcode_app);

void remove_barcode(CreateView* create_view_object);

void save_barcode(CreateView* create_view_object);

void create_view_free_model(CreateView* create_view_object);

void create_view_free(CreateView* create_view_object);

View* create_get_view(CreateView* create_view_object);
