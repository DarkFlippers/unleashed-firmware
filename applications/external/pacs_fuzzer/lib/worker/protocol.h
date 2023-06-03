#pragma once

#include <stdint.h>

// #define RFID_125_PROTOCOL

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

    // Reserved
    FuzzerProtoMax,
} FuzzerProtos;

struct ProtoDict {
    const uint8_t* val;
    const uint8_t len;
};

typedef struct ProtoDict ProtoDict;

struct FuzzerProtocol {
    const char* name;
    const uint8_t data_size;
    const ProtoDict dict;
};

typedef struct FuzzerProtocol FuzzerProtocol;

extern const FuzzerProtocol fuzzer_proto_items[];