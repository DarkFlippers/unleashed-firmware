#pragma once

#include "subbrute_protocols_i.h"
#include <furi.h>
#include <m-string.h>
#include <furi_hal_subghz.h>

//typedef enum {
//    FrequencyProtocolField,
//    BitsProtocolField,
//    HasTeProtocolField,
//    RepeatProtocolField,
//    PresetProtocolField,
//    FileProtocolField,
//    TotalProtocolFields
//} ProtocolFields;

typedef enum {
    CAMEFileProtocol,
    NICEFileProtocol,
    ChamberlainFileProtocol,
    LinearFileProtocol,
    PrincetonFileProtocol,
    RAWFileProtocol,
    TotalFileProtocol,
} SubBruteFileProtocol;

typedef struct {
    uint32_t frequency;
    uint8_t bits;
    uint8_t te;
    uint8_t repeat;
    FuriHalSubGhzPreset preset;
    SubBruteFileProtocol file;
} SubBruteProtocol;

SubBruteProtocol* subbrute_protocol_alloc(void);
SubBruteProtocol* subbrute_protocol(SubBruteAttacks index);
const char* subbrute_protocol_preset(FuriHalSubGhzPreset preset);
const char* subbrute_protocol_file(SubBruteFileProtocol protocol);
FuriHalSubGhzPreset subbrute_protocol_convert_preset(string_t preset_name);
SubBruteFileProtocol subbrute_protocol_file_protocol_name(string_t name);