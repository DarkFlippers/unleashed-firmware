#pragma once

#include <furi.h>
#include <furi_hal_subghz.h>
#include <core/string.h>

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

typedef enum {
    SubBruteAttackCAME12bit303,
    SubBruteAttackCAME12bit307,
    SubBruteAttackCAME12bit433,
    SubBruteAttackCAME12bit868,
    SubBruteAttackNICE12bit433,
    SubBruteAttackNICE12bit868,
    SubBruteAttackChamberlain9bit300,
    SubBruteAttackChamberlain9bit315,
    SubBruteAttackChamberlain9bit390,
    SubBruteAttackLinear10bit300,
    SubBruteAttackLinear10bit310,
    SubBruteAttackLoadFile,
    SubBruteAttackTotalCount,
} SubBruteAttacks;

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
FuriHalSubGhzPreset subbrute_protocol_convert_preset(FuriString* preset_name);
SubBruteFileProtocol subbrute_protocol_file_protocol_name(FuriString* name);
const char* subbrute_protocol_name(SubBruteAttacks index);