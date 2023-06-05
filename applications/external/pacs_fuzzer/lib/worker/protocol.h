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

/**
 * Get maximum length of UID among all supported protocols
 * @return Maximum length of UID
 */
uint8_t fuzzer_proto_get_max_data_size();

/**
 * Get protocol name based on its index
 * @param index protocol index
 * @return pointer to a string containing the name
 */
const char* fuzzer_proto_get_name(FuzzerProtocolsID index);

/**
 * Get number of protocols
 * @return number of protocols
 */
uint8_t fuzzer_proto_get_count_of_protocols();

/**
 * Get menu label based on its index
 * @param index menu index
 * @return pointer to a string containing the menu label
 */
const char* fuzzer_proto_get_menu_label(FuzzerMainMenuIndex index);

/**
 * Get number of menu items
 * @return number of menu items
 */
uint8_t fuzzer_proto_get_count_of_menu_items();