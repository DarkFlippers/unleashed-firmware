#include "barcode_utils.h"

BarcodeTypeObj* barcode_type_objs[NUMBER_OF_BARCODE_TYPES] = {NULL};

void init_types() {
    BarcodeTypeObj* upc_a = malloc(sizeof(BarcodeTypeObj));
    upc_a->name = "UPC-A";
    upc_a->type = UPCA;
    upc_a->min_digits = 11;
    upc_a->max_digits = 12;
    upc_a->start_pos = 16;
    barcode_type_objs[UPCA] = upc_a;

    BarcodeTypeObj* ean_8 = malloc(sizeof(BarcodeTypeObj));
    ean_8->name = "EAN-8";
    ean_8->type = EAN8;
    ean_8->min_digits = 7;
    ean_8->max_digits = 8;
    ean_8->start_pos = 32;
    barcode_type_objs[EAN8] = ean_8;

    BarcodeTypeObj* ean_13 = malloc(sizeof(BarcodeTypeObj));
    ean_13->name = "EAN-13";
    ean_13->type = EAN13;
    ean_13->min_digits = 12;
    ean_13->max_digits = 13;
    ean_13->start_pos = 16;
    barcode_type_objs[EAN13] = ean_13;

    BarcodeTypeObj* code_39 = malloc(sizeof(BarcodeTypeObj));
    code_39->name = "CODE-39";
    code_39->type = CODE39;
    code_39->min_digits = 1;
    code_39->max_digits = -1;
    code_39->start_pos = 0;
    barcode_type_objs[CODE39] = code_39;

    BarcodeTypeObj* code_128 = malloc(sizeof(BarcodeTypeObj));
    code_128->name = "CODE-128";
    code_128->type = CODE128;
    code_128->min_digits = 1;
    code_128->max_digits = -1;
    code_128->start_pos = 0;
    barcode_type_objs[CODE128] = code_128;

    BarcodeTypeObj* code_128c = malloc(sizeof(BarcodeTypeObj));
    code_128c->name = "CODE-128C";
    code_128c->type = CODE128C;
    code_128c->min_digits = 2;
    code_128c->max_digits = -1;
    code_128c->start_pos = 0;
    barcode_type_objs[CODE128C] = code_128c;

    BarcodeTypeObj* codabar = malloc(sizeof(BarcodeTypeObj));
    codabar->name = "Codabar";
    codabar->type = CODABAR;
    codabar->min_digits = 1;
    codabar->max_digits = -1;
    codabar->start_pos = 0;
    barcode_type_objs[CODABAR] = codabar;

    BarcodeTypeObj* unknown = malloc(sizeof(BarcodeTypeObj));
    unknown->name = "Unknown";
    unknown->type = UNKNOWN;
    unknown->min_digits = 0;
    unknown->max_digits = 0;
    unknown->start_pos = 0;
    barcode_type_objs[UNKNOWN] = unknown;
}

void free_types() {
    for(int i = 0; i < NUMBER_OF_BARCODE_TYPES; i++) {
        free(barcode_type_objs[i]);
    }
}

BarcodeTypeObj* get_type(FuriString* type_string) {
    if(furi_string_cmp_str(type_string, "UPC-A") == 0) {
        return barcode_type_objs[UPCA];
    }
    if(furi_string_cmp_str(type_string, "EAN-8") == 0) {
        return barcode_type_objs[EAN8];
    }
    if(furi_string_cmp_str(type_string, "EAN-13") == 0) {
        return barcode_type_objs[EAN13];
    }
    if(furi_string_cmp_str(type_string, "CODE-39") == 0) {
        return barcode_type_objs[CODE39];
    }
    if(furi_string_cmp_str(type_string, "CODE-128") == 0) {
        return barcode_type_objs[CODE128];
    }
    if(furi_string_cmp_str(type_string, "CODE-128C") == 0) {
        return barcode_type_objs[CODE128C];
    }
    if(furi_string_cmp_str(type_string, "Codabar") == 0) {
        return barcode_type_objs[CODABAR];
    }

    return barcode_type_objs[UNKNOWN];
}

const char* get_error_code_name(ErrorCode error_code) {
    switch(error_code) {
    case WrongNumberOfDigits:
        return "Wrong Number Of Digits";
    case InvalidCharacters:
        return "Invalid Characters";
    case UnsupportedType:
        return "Unsupported Type";
    case FileOpening:
        return "File Opening Error";
    case InvalidFileData:
        return "Invalid File Data";
    case MissingEncodingTable:
        return "Missing Encoding Table";
    case EncodingTableError:
        return "Encoding Table Error";
    case OKCode:
        return "OK";
    default:
        return "Unknown Code";
    };
}

const char* get_error_code_message(ErrorCode error_code) {
    switch(error_code) {
    case WrongNumberOfDigits:
        return "Wrong # of characters";
    case InvalidCharacters:
        return "Invalid characters";
    case UnsupportedType:
        return "Unsupported barcode type";
    case FileOpening:
        return "Could not open file";
    case InvalidFileData:
        return "Invalid file data";
    case MissingEncodingTable:
        return "Missing encoding table";
    case EncodingTableError:
        return "Encoding table error";
    case OKCode:
        return "OK";
    default:
        return "Could not read barcode data";
    };
}
