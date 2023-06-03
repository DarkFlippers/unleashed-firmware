#pragma once

#include <stdint.h>

// #define RFID_125_PROTOCOL

#if defined(RFID_125_PROTOCOL)

#define MAX_PAYLOAD_SIZE 6

#define FUZZ_TIME_DELAY_MIN (5)
#define FUZZ_TIME_DELAY_DEFAULT (10)
#define FUZZ_TIME_DELAY_MAX (70)

#else

#define MAX_PAYLOAD_SIZE 8

#define FUZZ_TIME_DELAY_MIN (4)
#define FUZZ_TIME_DELAY_DEFAULT (8)
#define FUZZ_TIME_DELAY_MAX (80)

#endif

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