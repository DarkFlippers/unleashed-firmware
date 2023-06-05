#pragma once

#include <stdint.h>

// #define RFID_125_PROTOCOL

typedef struct FuzzerPayload FuzzerPayload;

typedef enum {
#if defined(RFID_125_PROTOCOL)
    EM4100,
    HIDProx,
    PAC,
    H10301,
#else
    DS1990,
    Metakom,
    Cyfral,
#endif
} FuzzerProtocolsID;

typedef enum {
    FuzzerMainMenuIndexDefaultValues = 0,
    FuzzerMainMenuIndexLoadFile,
    FuzzerMainMenuIndexLoadFileCustomUids,
} FuzzerMainMenuIndex;

struct FuzzerPayload {
    uint8_t* data;
    uint8_t data_size;
};

uint8_t fuzzer_proto_get_max_data_size();

const char* fuzzer_proto_get_name(FuzzerProtocolsID index);

uint8_t fuzzer_proto_get_count_of_protocols();

const char* fuzzer_proto_get_menu_label(FuzzerMainMenuIndex index);

uint8_t fuzzer_proto_get_count_of_menu_items();