#pragma once

#include <stdbool.h>
#include <stdint.h>
#include <stddef.h>

#include <lib/flipper_format/flipper_format.h>
#include <lib/toolbox/level_duration.h>

#include "environment.h"
#include <furi.h>
#include <furi_hal.h>
#include <subghz/helpers/subghz_types.h>

#define SUBGHZ_APP_FOLDER ANY_PATH("subghz")
#define SUBGHZ_RAW_FOLDER EXT_PATH("subghz")
#define SUBGHZ_APP_EXTENSION ".sub"

#define SUBGHZ_KEY_FILE_VERSION 1
#define SUBGHZ_KEY_FILE_TYPE "Flipper SubGhz Key File"

#define SUBGHZ_RAW_FILE_VERSION 1
#define SUBGHZ_RAW_FILE_TYPE "Flipper SubGhz RAW File"

//
// Abstract method types
//

// Allocator and Deallocator
typedef void* (*SubGhzAlloc)(SubGhzEnvironment* environment);
typedef void (*SubGhzFree)(void* context);

// Serialize and Deserialize
typedef bool (*SubGhzSerialize)(
    void* context,
    FlipperFormat* flipper_format,
    SubGhzPresetDefinition* preset);
typedef bool (*SubGhzDeserialize)(void* context, FlipperFormat* flipper_format);

// Decoder specific
typedef void (*SubGhzDecoderFeed)(void* decoder, bool level, uint32_t duration);
typedef void (*SubGhzDecoderReset)(void* decoder);
typedef uint8_t (*SubGhzGetHashData)(void* decoder);
typedef void (*SubGhzGetString)(void* decoder, string_t output);

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
} SubGhzProtocolFlag;

typedef struct {
    const char* name;
    SubGhzProtocolType type;
    SubGhzProtocolFlag flag;

    const SubGhzProtocolEncoder* encoder;
    const SubGhzProtocolDecoder* decoder;
} SubGhzProtocol;
