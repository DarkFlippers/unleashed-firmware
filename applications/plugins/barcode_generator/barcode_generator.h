#pragma once

#include <furi.h>
#include <gui/gui.h>
#include <input/input.h>
#include <stdlib.h>
#include <storage/storage.h>
#include <toolbox/saved_struct.h>

#define BARCODE_SETTINGS_FILE_NAME "apps/Misc/barcodegen.save"

#define BARCODE_SETTINGS_VER (1)
#define BARCODE_SETTINGS_PATH EXT_PATH(BARCODE_SETTINGS_FILE_NAME)
#define BARCODE_SETTINGS_MAGIC (0xC2)

#define SAVE_BARCODE_SETTINGS(x) \
    saved_struct_save(           \
        BARCODE_SETTINGS_PATH,   \
        (x),                     \
        sizeof(BarcodeState),    \
        BARCODE_SETTINGS_MAGIC,  \
        BARCODE_SETTINGS_VER)

#define LOAD_BARCODE_SETTINGS(x) \
    saved_struct_load(           \
        BARCODE_SETTINGS_PATH,   \
        (x),                     \
        sizeof(BarcodeState),    \
        BARCODE_SETTINGS_MAGIC,  \
        BARCODE_SETTINGS_VER)

#define BARCODE_HEIGHT 50
#define BARCODE_Y_START 3
#define BARCODE_TEXT_OFFSET 9
#define BARCODE_MAX_LENS 13
#define NUMBER_OF_BARCODE_TYPES 3
#define MENU_INDEX_VIEW 0
#define MENU_INDEX_EDIT 1
#define MENU_INDEX_PARITY 2
#define MENU_INDEX_TYPE 3

typedef enum {
    EventTypeTick,
    EventTypeKey,
} EventType;

typedef struct {
    EventType type;
    InputEvent input;
} PluginEvent;

typedef enum {
    ViewMode,
    EditMode,
    MenuMode,
} Mode;

typedef enum {
    BarEncodingTypeLeft,
    BarEncodingTypeRight,
    BarEncodingTypeG,
} BarEncodingType;

typedef enum {
    BarTypeEAN8,
    BarTypeUPCA,
    BarTypeEAN13,
} BarType;

typedef struct {
    char* name;
    int numberOfDigits;
    int startPos;
    BarType bartype;
} BarcodeType;

typedef struct {
    int barcodeNumeral[BARCODE_MAX_LENS]; //The current barcode number
    bool doParityCalculation; //Should do parity check?
    int barcodeTypeIndex;
} BarcodeState;

typedef struct {
    BarcodeState barcode_state;
    int editingIndex; //The index of the editing symbol
    int menuIndex; //The index of the menu cursor
    Mode mode; //View, edit or menu
} PluginState;

static const int DIGITS[10][4] = {
    {3, 2, 1, 1},
    {2, 2, 2, 1},
    {2, 1, 2, 2},
    {1, 4, 1, 1},
    {1, 1, 3, 2},
    {1, 2, 3, 1},
    {1, 1, 1, 4},
    {1, 3, 1, 2},
    {1, 2, 1, 3},
    {3, 1, 1, 2},
};

static const uint8_t EAN13ENCODE[10] = {
    0b000000,
    0b110100,
    0b101100,
    0b011100,
    0b110010,
    0b100110,
    0b001110,
    0b101010,
    0b011010,
    0b010110,
};