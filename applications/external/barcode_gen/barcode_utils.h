
#pragma once
#include <furi.h>
#include <furi_hal.h>

#define NUMBER_OF_BARCODE_TYPES 8

typedef enum {
    WrongNumberOfDigits, //There is too many or too few digits in the barcode
    InvalidCharacters, //The barcode contains invalid characters
    UnsupportedType, //the barcode type is not supported
    FileOpening, //A problem occurred when opening the barcode data file
    InvalidFileData, //One of the key in the file doesn't exist or there is a typo
    MissingEncodingTable, //The encoding table txt for the barcode type is missing
    EncodingTableError, //Something is wrong with the encoding table, probably missing data or typo
    OKCode
} ErrorCode;

typedef enum {
    UPCA,
    EAN8,
    EAN13,
    CODE39,
    CODE128,
    CODE128C,
    CODABAR,

    UNKNOWN
} BarcodeType;

typedef struct {
    char* name; //The name of the barcode type
    BarcodeType type; //The barcode type enum
    int min_digits; //the minimum number of digits
    int max_digits; //the maximum number of digits
    int start_pos; //where to start drawing the barcode, set to -1 to dynamically draw barcode
} BarcodeTypeObj;

typedef struct {
    BarcodeTypeObj* type_obj;
    int check_digit; //A place to store the check digit
    FuriString* raw_data; //the data directly from the file
    FuriString* correct_data; //the corrected/processed data
    bool valid; //true if the raw data is correctly formatted, such as correct num of digits, valid characters, etc.
    ErrorCode reason; //the reason why this barcode is invalid
} BarcodeData;

//All available barcode types
extern BarcodeTypeObj* barcode_type_objs[NUMBER_OF_BARCODE_TYPES];

void init_types();
void free_types();
BarcodeTypeObj* get_type(FuriString* type_string);
const char* get_error_code_name(ErrorCode error_code);
const char* get_error_code_message(ErrorCode error_code);
