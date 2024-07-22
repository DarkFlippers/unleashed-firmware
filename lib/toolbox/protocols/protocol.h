#pragma once
#include <stdint.h>
#include <stddef.h>
#include <stdbool.h>
#include <lib/toolbox/level_duration.h>
#include <furi.h>

typedef void* (*ProtocolAlloc)(void);
typedef void (*ProtocolFree)(void* protocol);
typedef uint8_t* (*ProtocolGetData)(void* protocol);

typedef void (*ProtocolDecoderStart)(void* protocol);
typedef bool (*ProtocolDecoderFeed)(void* protocol, bool level, uint32_t duration);

typedef bool (*ProtocolEncoderStart)(void* protocol);
typedef LevelDuration (*ProtocolEncoderYield)(void* protocol);

typedef void (*ProtocolRenderData)(void* protocol, FuriString* result);
typedef bool (*ProtocolWriteData)(void* protocol, void* data);

typedef struct {
    ProtocolDecoderStart start;
    ProtocolDecoderFeed feed;
} ProtocolDecoder;

typedef struct {
    ProtocolEncoderStart start;
    ProtocolEncoderYield yield;
} ProtocolEncoder;

typedef struct {
    const size_t data_size;
    const char* name;
    const char* manufacturer;
    const uint32_t features;
    const uint8_t validate_count;

    ProtocolAlloc alloc;
    ProtocolFree free;
    ProtocolGetData get_data;
    ProtocolDecoder decoder;
    ProtocolEncoder encoder;
    ProtocolRenderData render_uid;
    ProtocolRenderData render_data;
    ProtocolRenderData render_brief_data;
    ProtocolWriteData write_data;
} ProtocolBase;
