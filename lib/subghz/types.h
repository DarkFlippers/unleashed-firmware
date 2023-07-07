#pragma once

#include <stdbool.h>
#include <stdint.h>
#include <stddef.h>

#include <lib/flipper_format/flipper_format.h>
#include <lib/toolbox/level_duration.h>

#include "environment.h"
#include <furi.h>
#include <furi_hal.h>

#define SUBGHZ_APP_FOLDER ANY_PATH("subghz")
#define SUBGHZ_RAW_FOLDER EXT_PATH("subghz")
#define SUBGHZ_APP_EXTENSION ".sub"

#define SUBGHZ_KEY_FILE_VERSION 1
#define SUBGHZ_KEY_FILE_TYPE "Flipper SubGhz Key File"

#define SUBGHZ_RAW_FILE_VERSION 1
#define SUBGHZ_RAW_FILE_TYPE "Flipper SubGhz RAW File"

typedef struct SubGhzProtocolRegistry SubGhzProtocolRegistry;
typedef struct SubGhzEnvironment SubGhzEnvironment;

// Radio Preset
typedef struct {
    FuriString* name;
    uint32_t frequency;
    uint8_t* data;
    size_t data_size;
} SubGhzRadioPreset;

typedef enum {
    SubGhzProtocolStatusOk = 0,
    // Errors
    SubGhzProtocolStatusError = (-1), ///< General unclassified error
    // Serialize/De-serialize
    SubGhzProtocolStatusErrorParserHeader = (-2), ///< Missing or invalid file header
    SubGhzProtocolStatusErrorParserFrequency = (-3), ///< Missing `Frequency`
    SubGhzProtocolStatusErrorParserPreset = (-4), ///< Missing `Preset`
    SubGhzProtocolStatusErrorParserCustomPreset = (-5), ///< Missing `Custom_preset_module`
    SubGhzProtocolStatusErrorParserProtocolName = (-6), ///< Missing `Protocol` name
    SubGhzProtocolStatusErrorParserBitCount = (-7), ///< Missing `Bit`
    SubGhzProtocolStatusErrorParserKey = (-8), ///< Missing `Key`
    SubGhzProtocolStatusErrorParserTe = (-9), ///< Missing `Te`
    SubGhzProtocolStatusErrorParserOthers = (-10), ///< Missing some other mandatory keys
    // Invalid data
    SubGhzProtocolStatusErrorValueBitCount = (-11), ///< Invalid bit count value
    // Encoder issue
    SubGhzProtocolStatusErrorEncoderGetUpload = (-12), ///< Payload encoder failure
    // Special Values
    SubGhzProtocolStatusReserved = 0x7FFFFFFF, ///< Prevents enum down-size compiler optimization.
} SubGhzProtocolStatus;

// Allocator and Deallocator
typedef void* (*SubGhzAlloc)(SubGhzEnvironment* environment);
typedef void (*SubGhzFree)(void* context);

// Serialize and Deserialize
typedef SubGhzProtocolStatus (
    *SubGhzSerialize)(void* context, FlipperFormat* flipper_format, SubGhzRadioPreset* preset);
typedef SubGhzProtocolStatus (*SubGhzDeserialize)(void* context, FlipperFormat* flipper_format);

// Decoder specific
typedef void (*SubGhzDecoderFeed)(void* decoder, bool level, uint32_t duration);
typedef void (*SubGhzDecoderReset)(void* decoder);
typedef uint8_t (*SubGhzGetHashData)(void* decoder);
typedef void (*SubGhzGetString)(void* decoder, FuriString* output);

// Encoder specific
typedef void (*SubGhzEncoderStop)(void* encoder);
typedef LevelDuration (*SubGhzEncoderYield)(void* context);

typedef struct {
    SubGhzAlloc alloc;
    SubGhzFree free;

    SubGhzDecoderFeed feed;
    SubGhzDecoderReset reset;

    SubGhzGetHashData get_hash_data;
    SubGhzGetString get_string;
    SubGhzSerialize serialize;
    SubGhzDeserialize deserialize;
} SubGhzProtocolDecoder;

typedef struct {
    SubGhzAlloc alloc;
    SubGhzFree free;

    SubGhzDeserialize deserialize;
    SubGhzEncoderStop stop;
    SubGhzEncoderYield yield;
} SubGhzProtocolEncoder;

typedef enum {
    SubGhzProtocolTypeUnknown = 0,
    SubGhzProtocolTypeStatic,
    SubGhzProtocolTypeDynamic,
    SubGhzProtocolTypeRAW,
    SubGhzProtocolWeatherStation,
    SubGhzProtocolCustom,
    SubGhzProtocolTypeBinRAW,
} SubGhzProtocolType;

typedef enum {
    SubGhzProtocolFlag_RAW = (1 << 0),
    SubGhzProtocolFlag_Decodable = (1 << 1),
    SubGhzProtocolFlag_315 = (1 << 2),
    SubGhzProtocolFlag_433 = (1 << 3),
    SubGhzProtocolFlag_868 = (1 << 4),
    SubGhzProtocolFlag_AM = (1 << 5),
    SubGhzProtocolFlag_FM = (1 << 6),
    SubGhzProtocolFlag_Save = (1 << 7),
    SubGhzProtocolFlag_Load = (1 << 8),
    SubGhzProtocolFlag_Send = (1 << 9),
    SubGhzProtocolFlag_BinRAW = (1 << 10),
    SubGhzProtocolFlag_StarLine = (1 << 11),
    SubGhzProtocolFlag_AutoAlarms = (1 << 12),
    SubGhzProtocolFlag_Magelan = (1 << 13),
} SubGhzProtocolFlag;

struct SubGhzProtocol {
    const char* name;
    SubGhzProtocolType type;
    SubGhzProtocolFlag flag;

    const SubGhzProtocolEncoder* encoder;
    const SubGhzProtocolDecoder* decoder;
};
