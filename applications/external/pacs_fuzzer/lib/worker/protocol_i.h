#pragma once

#include "protocol.h"

#if defined(RFID_125_PROTOCOL)
#define MAX_PAYLOAD_SIZE (6)
#define PROTOCOL_MIN_IDLE_DELAY (5)
#define PROTOCOL_TIME_DELAY_MIN PROTOCOL_MIN_IDLE_DELAY + 4
#else
#define MAX_PAYLOAD_SIZE (8)
#define PROTOCOL_MIN_IDLE_DELAY (2)
#define PROTOCOL_TIME_DELAY_MIN PROTOCOL_MIN_IDLE_DELAY + 2
#endif

typedef struct ProtoDict ProtoDict;
typedef struct FuzzerProtocol FuzzerProtocol;

struct ProtoDict {
    const uint8_t* val;
    const uint8_t len; // TODO
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

// #define FUZZER_APP_CUSTOM_DICT_EXTENSION ".txt"
// #define FUZZER_APP_CUSTOM_DICT_FOLDER "/ext/rfidfuzzer"
// #define FUZZER_APP_KEY_EXTENSION ".rfid"
// #define FUZZER_APP_PATH_KEY_FOLDER "/ext/lfrfid"

// #define MAX_PAYLOAD_SIZE 8

// #define FUZZ_TIME_DELAY_MIN (4)
// #define FUZZ_TIME_DELAY_DEFAULT (8)
// #define FUZZ_TIME_DELAY_MAX (80)

// #define FUZZER_APP_CUSTOM_DICT_EXTENSION ".txt"
// #define FUZZER_APP_CUSTOM_DICT_FOLDER "/ext/ibtnfuzzer"
// #define FUZZER_APP_KEY_EXTENSION ".ibtn"
// #define FUZZER_APP_PATH_KEY_FOLDER "/ext/ibutton"

extern const FuzzerProtocol fuzzer_proto_items[];