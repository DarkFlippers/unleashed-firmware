#pragma once

#include "protocol.h"

#if defined(RFID_125_PROTOCOL)
#define MAX_PAYLOAD_SIZE (6)
#define PROTOCOL_DEF_IDLE_TIME (4)
#define PROTOCOL_DEF_EMU_TIME (5)
#define PROTOCOL_TIME_DELAY_MIN PROTOCOL_DEF_IDLE_TIME + PROTOCOL_DEF_EMU_TIME
#else
#define MAX_PAYLOAD_SIZE (8)
#define PROTOCOL_DEF_IDLE_TIME (2)
#define PROTOCOL_DEF_EMU_TIME (2)
#define PROTOCOL_TIME_DELAY_MIN PROTOCOL_DEF_IDLE_TIME + PROTOCOL_DEF_EMU_TIME
#endif

typedef struct ProtoDict ProtoDict;
typedef struct FuzzerProtocol FuzzerProtocol;

struct ProtoDict {
    const uint8_t* val;
    const uint8_t len;
};

struct FuzzerProtocol {
    const char* name;
    const uint8_t data_size;
    const ProtoDict dict;
};

// #define MAX_PAYLOAD_SIZE 6

// #define FUZZ_TIME_DELAY_MIN (5)
// #define FUZZ_TIME_DELAY_DEFAULT (10)
// #define FUZZ_TIME_DELAY_MAX (70)

// #define MAX_PAYLOAD_SIZE 8

// #define FUZZ_TIME_DELAY_MIN (4)
// #define FUZZ_TIME_DELAY_DEFAULT (8)
// #define FUZZ_TIME_DELAY_MAX (80)

extern const FuzzerProtocol fuzzer_proto_items[];