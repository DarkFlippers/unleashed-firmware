#pragma once
#include <furi.h>
#include <furi_hal.h>

#include <gui/gui.h>
#include <input/input.h>
#include <dialogs/dialogs.h>
#include <gui/view_dispatcher.h>
#include <gui/modules/submenu.h>
#include <gui/modules/text_input.h>
#include <gui/modules/text_input.h>

#include <flipper_format/flipper_format.h>

#include "barcode_utils.h"

#define TAG "BARCODE"
#define VERSION "1.1"
#define FILE_VERSION "1"

#define TEXT_BUFFER_SIZE 128

#define BARCODE_HEIGHT 50
#define BARCODE_Y_START 3

//the folder where the encodings are located
#define BARCODE_DATA_FILE_DIR_PATH EXT_PATH("apps_data/barcode_data")

//the folder where the codabar encoding table is located
#define CODABAR_DICT_FILE_PATH BARCODE_DATA_FILE_DIR_PATH "/codabar_encodings.txt"

//the folder where the code 39 encoding table is located
#define CODE39_DICT_FILE_PATH BARCODE_DATA_FILE_DIR_PATH "/code39_encodings.txt"

//the folder where the code 128 encoding table is located
#define CODE128_DICT_FILE_PATH BARCODE_DATA_FILE_DIR_PATH "/code128_encodings.txt"

//the folder where the code 128 C encoding table is located
#define CODE128C_DICT_FILE_PATH BARCODE_DATA_FILE_DIR_PATH "/code128c_encodings.txt"

//the folder where the user stores their barcodes
#define DEFAULT_USER_BARCODES EXT_PATH("apps_data/barcodes")

//The extension barcode files use
#define BARCODE_EXTENSION ".txt"
#define BARCODE_EXTENSION_LENGTH 4

#include "views/barcode_view.h"
#include "views/create_view.h"
#include "views/message_view.h"
#include "barcode_validator.h"

typedef struct BarcodeApp BarcodeApp;

struct BarcodeApp {
    Submenu* main_menu;
    ViewDispatcher* view_dispatcher;
    Gui* gui;

    FuriMessageQueue* event_queue;

    CreateView* create_view;
    Barcode* barcode_view;

    MessageView* message_view;
    TextInput* text_input;
};

enum SubmenuItems {
    SelectBarcodeItem,
    EditBarcodeItem,

    CreateBarcodeItem
};

enum Views {
    TextInputView,
    MessageErrorView,
    MainMenuView,
    CreateBarcodeView,

    BarcodeView
};

void submenu_callback(void* context, uint32_t index);

uint32_t main_menu_callback(void* context);

uint32_t exit_callback(void* context);

int32_t barcode_main(void* p);
