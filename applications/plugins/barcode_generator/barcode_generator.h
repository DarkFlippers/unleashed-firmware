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
    int editingIndex; //The index of the editing symbol
    int menuIndex; //The index of the menu cursor
    Mode mode; //View, edit or menu
    bool doParityCalculation; //Should do parity check?
    int barcodeTypeIndex;
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