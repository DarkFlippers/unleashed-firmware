#pragma once

#include "protocol.h"

#if defined(RFID_125_PROTOCOL)

#define MAX_PAYLOAD_SIZE 6

#define FUZZ_TIME_DELAY_MIN (5)
#define FUZZ_TIME_DELAY_DEFAULT (10)
#define FUZZ_TIME_DELAY_MAX (70)

#define FUZZER_APP_CUSTOM_DICT_EXTENSION ".txt"
#define FUZZER_APP_CUSTOM_DICT_FOLDER "/ext/rfidfuzzer"
#define FUZZER_APP_KEY_EXTENSION ".rfid"
#define FUZZER_APP_PATH_KEY_FOLDER "/ext/lfrfid"

#else

#define MAX_PAYLOAD_SIZE 8

#define FUZZ_TIME_DELAY_MIN (4)
#define FUZZ_TIME_DELAY_DEFAULT (8)
#define FUZZ_TIME_DELAY_MAX (80)

#define FUZZER_APP_CUSTOM_DICT_EXTENSION ".txt"
#define FUZZER_APP_CUSTOM_DICT_FOLDER "/ext/ibtnfuzzer"
#define FUZZER_APP_KEY_EXTENSION ".ibtn"
#define FUZZER_APP_PATH_KEY_FOLDER "/ext/ibutton"

#endif
