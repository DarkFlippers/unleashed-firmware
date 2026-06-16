// Ported from https://github.com/uzyn/flipper-bambu (GPL-3.0, (c) U-Zyn Chua)
//
// Bambu Lab NFC Parser - Core Parsing Logic
// This header contains the portable parsing functions shared between
// the Flipper Zero plugin and the test suite.
//
// Usage:
// - For Flipper: Include after mf_classic.h (types already defined)
// - For tests: Define BAMBU_PARSER_TEST_MODE and mock types before including

#ifndef BAMBU_PARSER_H
#define BAMBU_PARSER_H

#include <stdint.h>
#include <stdbool.h>
#include <stddef.h>
#include <string.h>

// Block layout for Bambu Lab spool RFID tags (Mifare Classic 1K)
// Skip blocks 3,7,11,15,... (sector trailers with MIFARE keys)
#define BLOCK_MATERIAL_IDS      1   // Material ID (GFxxx) + Variant ID (xxx-Rx)
#define BLOCK_FILAMENT_TYPE     2   // Filament type (PLA, PETG, ABS, etc.)
#define BLOCK_DETAILED_TYPE     4   // Detailed type (PLA Basic, PLA Matte, etc.)
#define BLOCK_COLOR_WEIGHT      5   // RGBA color, weight (g), diameter (mm)
#define BLOCK_TEMPERATURES      6   // Drying temp/hours, hotend max/min temps
#define BLOCK_NOZZLE            8   // Nozzle diameter (float at bytes 12-15)
#define BLOCK_SPOOL_WIDTH      10   // Spool width (uint16 at bytes 4-5, mm*100)
#define BLOCK_PRODUCTION_DATE  12   // Production date (ASCII YYYY_MM_DD_HH_MM)
#define BLOCK_FILAMENT_LENGTH  14   // Filament length (uint16 at bytes 4-5, meters)

// Helper: Read little-endian uint16
static inline uint16_t bambu_read_le16(const uint8_t* data) {
    return (uint16_t)(data[0] | (data[1] << 8));
}

// Helper: Read little-endian uint32 as float (IEEE 754)
static inline float bambu_read_le_float(const uint8_t* data) {
    union {
        uint32_t u;
        float f;
    } val;
    val.u = (uint32_t)(data[0] | (data[1] << 8) | (data[2] << 16) | (data[3] << 24));
    return val.f;
}

// Helper: Check if block contains printable ASCII (with null padding allowed)
static inline bool bambu_is_printable_ascii(const uint8_t* data, size_t len) {
    bool found_printable = false;
    for(size_t i = 0; i < len; i++) {
        if(data[i] == 0) continue;  // Null padding is OK
        if(data[i] < 0x20 || data[i] > 0x7E) return false;  // Non-printable
        found_printable = true;
    }
    return found_printable;
}

// Helper: Copy null-terminated ASCII string from block data
static inline void bambu_copy_ascii_string(char* dest, const uint8_t* src, size_t max_len) {
    size_t i;
    for(i = 0; i < max_len && src[i] != 0 && src[i] >= 0x20 && src[i] <= 0x7E; i++) {
        dest[i] = (char)src[i];
    }
    dest[i] = '\0';
}

// Known filament types for validation
static const char* const BAMBU_KNOWN_FILAMENT_TYPES[] = {
    "PLA", "PETG", "ABS", "TPU", "PA", "PC", "ASA", "PVA", "HIPS", "PET"
};
#define BAMBU_NUM_FILAMENT_TYPES (sizeof(BAMBU_KNOWN_FILAMENT_TYPES) / sizeof(BAMBU_KNOWN_FILAMENT_TYPES[0]))

// Validation: Conservative Bambu-specific validation
// Returns true only if this looks like a Bambu Lab spool tag
// Requires MfClassicData and MfClassicType1k to be defined
static inline bool bambu_tag_is_valid(const MfClassicData* data) {
    // Must be Mifare Classic 1K
    if(data->type != MfClassicType1k) {
        return false;
    }

    // Block 1: Check Material ID starts with "GF" at bytes 8-9
    const uint8_t* block1 = data->block[BLOCK_MATERIAL_IDS].data;
    if(block1[8] != 'G' || block1[9] != 'F') {
        return false;
    }

    // Block 2: Check for known filament types
    const uint8_t* block2 = data->block[BLOCK_FILAMENT_TYPE].data;
    bool valid_type = false;
    for(size_t i = 0; i < BAMBU_NUM_FILAMENT_TYPES; i++) {
        size_t len = strlen(BAMBU_KNOWN_FILAMENT_TYPES[i]);
        if(memcmp(block2, BAMBU_KNOWN_FILAMENT_TYPES[i], len) == 0) {
            valid_type = true;
            break;
        }
    }
    if(!valid_type) {
        return false;
    }

    // Block 4: Check detailed type is printable ASCII
    const uint8_t* block4 = data->block[BLOCK_DETAILED_TYPE].data;
    if(!bambu_is_printable_ascii(block4, 16)) {
        return false;
    }

    // Block 5: Check diameter is plausible (1.6-2.0mm or 2.7-3.0mm range)
    const uint8_t* block5 = data->block[BLOCK_COLOR_WEIGHT].data;
    float diameter = bambu_read_le_float(&block5[8]);
    bool valid_diameter = (diameter >= 1.6f && diameter <= 2.0f) ||
                          (diameter >= 2.7f && diameter <= 3.0f);
    if(!valid_diameter) {
        return false;
    }

    return true;
}

#endif // BAMBU_PARSER_H
