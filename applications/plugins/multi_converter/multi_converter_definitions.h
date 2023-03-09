#pragma once

#define MULTI_CONVERTER_NUMBER_DIGITS 9

typedef enum {
    EventTypeKey,
} EventType;

typedef struct {
    InputEvent input;
    EventType type;
} MultiConverterEvent;

typedef enum {
    ModeDisplay,
    ModeSelector,
} MultiConverterMode;

typedef enum {
    None,
    Reset,
    Convert,
} MultiConverterModeTrigger;

// new units goes here, used as index to the main multi_converter_available_units array (multi_converter_units.h)
typedef enum {
    UnitTypeDec,
    UnitTypeHex,
    UnitTypeBin,

    UnitTypeCelsius,
    UnitTypeFahernheit,
    UnitTypeKelvin,

    UnitTypeKilometers,
    UnitTypeMeters,
    UnitTypeCentimeters,
    UnitTypeMiles,
    UnitTypeFeet,
    UnitTypeInches,

    UnitTypeDegree,
    UnitTypeRadian,
} MultiConverterUnitType;

typedef struct {
    MultiConverterUnitType selected_unit_type_orig;
    MultiConverterUnitType selected_unit_type_dest;
    uint8_t select_orig;
} MultiConverterModeSelect;

typedef struct {
    uint8_t cursor; // cursor position when typing
    int8_t key; // hover key
    uint8_t comma; // comma already added? (only one comma allowed)
    uint8_t negative; // is negative?
} MultiConverterModeDisplay;

typedef struct MultiConverterUnit MultiConverterUnit;
typedef struct MultiConverterState MultiConverterState;

struct MultiConverterUnit {
    uint8_t allow_comma;
    uint8_t allow_negative;
    uint8_t max_number_keys;
    char mini_name[4];
    char name[12];
    void (*convert_function)(MultiConverterState* const);
    uint8_t (*allowed_function)(MultiConverterUnitType);
};

struct MultiConverterState {
    FuriMutex* mutex;
    char buffer_orig[MULTI_CONVERTER_NUMBER_DIGITS + 1];
    char buffer_dest[MULTI_CONVERTER_NUMBER_DIGITS + 1];
    MultiConverterUnitType unit_type_orig;
    MultiConverterUnitType unit_type_dest;
    MultiConverterMode mode;
    MultiConverterModeDisplay display;
    MultiConverterModeSelect select;
    uint8_t keyboard_lock; // used to create a small lock when switching from SELECT to DISPLAY modes
        // (debouncing, basically; otherwise it switch modes twice 'cause it's too fast!)
};
