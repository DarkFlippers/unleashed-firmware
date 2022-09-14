#include <furi.h>
#include "../minunit.h"
#include <toolbox/protocols/protocol_dict.h>
#include <lfrfid/protocols/lfrfid_protocols.h>
#include <toolbox/pulse_protocols/pulse_glue.h>

#define LF_RFID_READ_TIMING_MULTIPLIER 8

#define EM_TEST_DATA \
    { 0x58, 0x00, 0x85, 0x64, 0x02 }
#define EM_TEST_DATA_SIZE 5
#define EM_TEST_EMULATION_TIMINGS_COUNT (64 * 2)

const int8_t em_test_timings[EM_TEST_EMULATION_TIMINGS_COUNT] = {
    32,  -32, 32,  -32, 32,  -32, 32,  -32, 32,  -32, 32,  -32, 32,  -32, 32,  -32, 32,  -32, -32,
    32,  32,  -32, -32, 32,  32,  -32, -32, 32,  32,  -32, -32, 32,  -32, 32,  -32, 32,  32,  -32,
    -32, 32,  -32, 32,  -32, 32,  -32, 32,  -32, 32,  -32, 32,  -32, 32,  -32, 32,  -32, 32,  -32,
    32,  32,  -32, -32, 32,  -32, 32,  -32, 32,  32,  -32, -32, 32,  32,  -32, -32, 32,  32,  -32,
    -32, 32,  -32, 32,  32,  -32, 32,  -32, -32, 32,  -32, 32,  -32, 32,  32,  -32, -32, 32,  -32,
    32,  32,  -32, -32, 32,  -32, 32,  -32, 32,  -32, 32,  -32, 32,  -32, 32,  -32, 32,  32,  -32,
    -32, 32,  32,  -32, -32, 32,  -32, 32,  -32, 32,  -32, 32,  -32, 32,
};

#define HID10301_TEST_DATA \
    { 0x8D, 0x48, 0xA8 }
#define HID10301_TEST_DATA_SIZE 3
#define HID10301_TEST_EMULATION_TIMINGS_COUNT (541 * 2)

const int8_t hid10301_test_timings[HID10301_TEST_EMULATION_TIMINGS_COUNT] = {
    4, -4, 4, -4, 4, -4, 4, -4, 4, -4, 4, -4, 4, -4, 4, -4, 4, -4, 4, -4, 4, -4, 4, -4, 4, -4,
    4, -4, 4, -4, 4, -4, 4, -4, 4, -4, 4, -4, 5, -5, 5, -5, 5, -5, 5, -5, 5, -5, 5, -5, 5, -5,
    5, -5, 5, -5, 5, -5, 5, -5, 5, -5, 5, -5, 5, -5, 5, -5, 4, -4, 4, -4, 4, -4, 4, -4, 4, -4,
    4, -4, 4, -4, 5, -5, 5, -5, 5, -5, 5, -5, 5, -5, 4, -4, 4, -4, 4, -4, 4, -4, 4, -4, 4, -4,
    5, -5, 5, -5, 5, -5, 5, -5, 5, -5, 4, -4, 4, -4, 4, -4, 4, -4, 4, -4, 4, -4, 5, -5, 5, -5,
    5, -5, 5, -5, 5, -5, 4, -4, 4, -4, 4, -4, 4, -4, 4, -4, 4, -4, 5, -5, 5, -5, 5, -5, 5, -5,
    5, -5, 4, -4, 4, -4, 4, -4, 4, -4, 4, -4, 4, -4, 4, -4, 5, -5, 5, -5, 5, -5, 5, -5, 5, -5,
    4, -4, 4, -4, 4, -4, 4, -4, 4, -4, 4, -4, 5, -5, 5, -5, 5, -5, 5, -5, 5, -5, 4, -4, 4, -4,
    4, -4, 4, -4, 4, -4, 4, -4, 5, -5, 5, -5, 5, -5, 5, -5, 5, -5, 5, -5, 5, -5, 5, -5, 5, -5,
    5, -5, 4, -4, 4, -4, 4, -4, 4, -4, 4, -4, 4, -4, 4, -4, 4, -4, 4, -4, 4, -4, 4, -4, 4, -4,
    4, -4, 5, -5, 5, -5, 5, -5, 5, -5, 5, -5, 4, -4, 4, -4, 4, -4, 4, -4, 4, -4, 4, -4, 5, -5,
    5, -5, 5, -5, 5, -5, 5, -5, 4, -4, 4, -4, 4, -4, 4, -4, 4, -4, 4, -4, 5, -5, 5, -5, 5, -5,
    5, -5, 5, -5, 4, -4, 4, -4, 4, -4, 4, -4, 4, -4, 4, -4, 5, -5, 5, -5, 5, -5, 5, -5, 5, -5,
    4, -4, 4, -4, 4, -4, 4, -4, 4, -4, 4, -4, 4, -4, 5, -5, 5, -5, 5, -5, 5, -5, 5, -5, 4, -4,
    4, -4, 4, -4, 4, -4, 4, -4, 4, -4, 5, -5, 5, -5, 5, -5, 5, -5, 5, -5, 4, -4, 4, -4, 4, -4,
    4, -4, 4, -4, 4, -4, 5, -5, 5, -5, 5, -5, 5, -5, 5, -5, 4, -4, 4, -4, 4, -4, 4, -4, 4, -4,
    4, -4, 5, -5, 5, -5, 5, -5, 5, -5, 5, -5, 4, -4, 4, -4, 4, -4, 4, -4, 4, -4, 4, -4, 4, -4,
    5, -5, 5, -5, 5, -5, 5, -5, 5, -5, 4, -4, 4, -4, 4, -4, 4, -4, 4, -4, 4, -4, 5, -5, 5, -5,
    5, -5, 5, -5, 5, -5, 5, -5, 5, -5, 5, -5, 5, -5, 5, -5, 4, -4, 4, -4, 4, -4, 4, -4, 4, -4,
    4, -4, 5, -5, 5, -5, 5, -5, 5, -5, 5, -5, 4, -4, 4, -4, 4, -4, 4, -4, 4, -4, 4, -4, 5, -5,
    5, -5, 5, -5, 5, -5, 5, -5, 4, -4, 4, -4, 4, -4, 4, -4, 4, -4, 4, -4, 4, -4, 4, -4, 4, -4,
    4, -4, 4, -4, 4, -4, 4, -4, 5, -5, 5, -5, 5, -5, 5, -5, 5, -5, 4, -4, 4, -4, 4, -4, 4, -4,
    4, -4, 4, -4, 5, -5, 5, -5, 5, -5, 5, -5, 5, -5, 4, -4, 4, -4, 4, -4, 4, -4, 4, -4, 4, -4,
    5, -5, 5, -5, 5, -5, 5, -5, 5, -5, 5, -5, 5, -5, 5, -5, 5, -5, 5, -5, 4, -4, 4, -4, 4, -4,
    4, -4, 4, -4, 4, -4, 4, -4, 5, -5, 5, -5, 5, -5, 5, -5, 5, -5, 4, -4, 4, -4, 4, -4, 4, -4,
    4, -4, 4, -4, 4, -4, 4, -4, 4, -4, 4, -4, 4, -4, 4, -4, 5, -5, 5, -5, 5, -5, 5, -5, 5, -5,
    5, -5, 5, -5, 5, -5, 5, -5, 5, -5, 4, -4, 4, -4, 4, -4, 4, -4, 4, -4, 4, -4, 4, -4, 4, -4,
    4, -4, 4, -4, 4, -4, 4, -4, 4, -4, 5, -5, 5, -5, 5, -5, 5, -5, 5, -5, 5, -5, 5, -5, 5, -5,
    5, -5, 5, -5, 4, -4, 4, -4, 4, -4, 4, -4, 4, -4, 4, -4, 4, -4, 4, -4, 4, -4, 4, -4, 4, -4,
    4, -4, 5, -5, 5, -5, 5, -5, 5, -5, 5, -5, 4, -4, 4, -4, 4, -4, 4, -4, 4, -4, 4, -4, 5, -5,
    5, -5, 5, -5, 5, -5, 5, -5, 5, -5, 5, -5, 5, -5, 5, -5, 5, -5, 4, -4, 4, -4, 4, -4, 4, -4,
    4, -4, 4, -4, 4, -4, 4, -4, 4, -4, 4, -4, 4, -4, 4, -4, 4, -4, 5, -5, 5, -5, 5, -5, 5, -5,
    5, -5, 4, -4, 4, -4, 4, -4, 4, -4, 4, -4, 4, -4, 5, -5, 5, -5, 5, -5, 5, -5, 5, -5, 4, -4,
    4, -4, 4, -4, 4, -4, 4, -4, 4, -4, 5, -5, 5, -5, 5, -5, 5, -5, 5, -5, 5, -5, 5, -5, 5, -5,
    5, -5, 5, -5, 4, -4, 4, -4, 4, -4, 4, -4, 4, -4, 4, -4, 4, -4, 4, -4, 4, -4, 4, -4, 4, -4,
    4, -4, 4, -4, 5, -5, 5, -5, 5, -5, 5, -5, 5, -5, 5, -5, 5, -5, 5, -5, 5, -5, 5, -5, 4, -4,
    4, -4, 4, -4, 4, -4, 4, -4, 4, -4, 4, -4, 4, -4, 4, -4, 4, -4, 4, -4, 4, -4, 5, -5, 5, -5,
    5, -5, 5, -5, 5, -5, 5, -5, 5, -5, 5, -5, 5, -5, 5, -5, 4, -4, 4, -4, 4, -4, 4, -4, 4, -4,
    4, -4, 4, -4, 4, -4, 4, -4, 4, -4, 4, -4, 4, -4, 4, -4, 5, -5, 5, -5, 5, -5, 5, -5, 5, -5,
    4, -4, 4, -4, 4, -4, 4, -4, 4, -4, 4, -4, 5, -5, 5, -5, 5, -5, 5, -5, 5, -5, 4, -4, 4, -4,
    4, -4, 4, -4, 4, -4, 4, -4, 5, -5, 5, -5, 5, -5, 5, -5, 5, -5, 5, -5, 5, -5, 5, -5, 5, -5,
    5, -5, 4, -4, 4, -4, 4, -4, 4, -4, 4, -4, 4, -4, 4, -4,
};

#define IOPROX_XSF_TEST_DATA \
    { 0x65, 0x01, 0x05, 0x39 }
#define IOPROX_XSF_TEST_DATA_SIZE 4
#define IOPROX_XSF_TEST_EMULATION_TIMINGS_COUNT (468 * 2)

const int8_t ioprox_xsf_test_timings[IOPROX_XSF_TEST_EMULATION_TIMINGS_COUNT] = {
    4, -4, 4, -4, 4, -4, 4, -4, 4, -4, 4, -4, 4, -4, 4, -4, 4, -4, 4, -4, 4, -4, 4, -4, 4, -4,
    4, -4, 4, -4, 4, -4, 4, -4, 4, -4, 4, -4, 4, -4, 4, -4, 4, -4, 4, -4, 4, -4, 4, -4, 4, -4,
    4, -4, 4, -4, 4, -4, 4, -4, 4, -4, 4, -4, 4, -4, 4, -4, 4, -4, 4, -4, 4, -4, 4, -4, 4, -4,
    4, -4, 4, -4, 4, -4, 4, -4, 4, -4, 4, -4, 4, -4, 4, -4, 4, -4, 4, -4, 4, -4, 4, -4, 4, -4,
    4, -4, 4, -4, 4, -4, 4, -4, 4, -4, 4, -4, 4, -4, 4, -4, 4, -4, 4, -4, 4, -4, 4, -4, 4, -4,
    4, -4, 4, -4, 4, -4, 4, -4, 4, -4, 4, -4, 4, -4, 4, -4, 5, -5, 5, -5, 5, -5, 5, -5, 5, -5,
    5, -5, 5, -5, 5, -5, 5, -5, 5, -5, 5, -5, 5, -5, 5, -5, 5, -5, 5, -5, 5, -5, 5, -5, 5, -5,
    5, -5, 5, -5, 5, -5, 5, -5, 5, -5, 5, -5, 5, -5, 4, -4, 4, -4, 4, -4, 4, -4, 4, -4, 4, -4,
    4, -4, 4, -4, 4, -4, 4, -4, 4, -4, 4, -4, 4, -4, 4, -4, 4, -4, 4, -4, 4, -4, 4, -4, 4, -4,
    4, -4, 4, -4, 4, -4, 4, -4, 4, -4, 4, -4, 4, -4, 4, -4, 4, -4, 4, -4, 4, -4, 4, -4, 4, -4,
    5, -5, 5, -5, 5, -5, 5, -5, 5, -5, 5, -5, 5, -5, 4, -4, 4, -4, 4, -4, 4, -4, 4, -4, 4, -4,
    4, -4, 4, -4, 5, -5, 5, -5, 5, -5, 5, -5, 5, -5, 5, -5, 5, -5, 5, -5, 5, -5, 5, -5, 5, -5,
    5, -5, 5, -5, 4, -4, 4, -4, 4, -4, 4, -4, 4, -4, 4, -4, 4, -4, 4, -4, 4, -4, 4, -4, 4, -4,
    4, -4, 4, -4, 4, -4, 4, -4, 5, -5, 5, -5, 5, -5, 5, -5, 5, -5, 5, -5, 5, -5, 4, -4, 4, -4,
    4, -4, 4, -4, 4, -4, 4, -4, 4, -4, 4, -4, 5, -5, 5, -5, 5, -5, 5, -5, 5, -5, 5, -5, 5, -5,
    5, -5, 5, -5, 5, -5, 5, -5, 5, -5, 5, -5, 4, -4, 4, -4, 4, -4, 4, -4, 4, -4, 4, -4, 4, -4,
    4, -4, 4, -4, 4, -4, 4, -4, 4, -4, 4, -4, 4, -4, 4, -4, 4, -4, 4, -4, 4, -4, 4, -4, 4, -4,
    4, -4, 4, -4, 4, -4, 4, -4, 4, -4, 4, -4, 4, -4, 4, -4, 4, -4, 4, -4, 4, -4, 4, -4, 4, -4,
    4, -4, 4, -4, 4, -4, 4, -4, 4, -4, 4, -4, 4, -4, 4, -4, 4, -4, 4, -4, 4, -4, 4, -4, 4, -4,
    4, -4, 4, -4, 4, -4, 4, -4, 4, -4, 4, -4, 4, -4, 4, -4, 4, -4, 5, -5, 5, -5, 5, -5, 5, -5,
    5, -5, 5, -5, 5, -5, 5, -5, 5, -5, 5, -5, 5, -5, 5, -5, 5, -5, 4, -4, 4, -4, 4, -4, 4, -4,
    4, -4, 4, -4, 4, -4, 4, -4, 4, -4, 4, -4, 4, -4, 4, -4, 4, -4, 4, -4, 4, -4, 4, -4, 4, -4,
    4, -4, 4, -4, 4, -4, 4, -4, 4, -4, 4, -4, 4, -4, 4, -4, 4, -4, 4, -4, 4, -4, 4, -4, 4, -4,
    4, -4, 4, -4, 4, -4, 4, -4, 4, -4, 4, -4, 4, -4, 4, -4, 4, -4, 4, -4, 5, -5, 5, -5, 5, -5,
    5, -5, 5, -5, 5, -5, 5, -5, 4, -4, 4, -4, 4, -4, 4, -4, 4, -4, 4, -4, 4, -4, 5, -5, 5, -5,
    5, -5, 5, -5, 5, -5, 5, -5, 5, -5, 5, -5, 5, -5, 5, -5, 5, -5, 5, -5, 5, -5, 4, -4, 4, -4,
    4, -4, 4, -4, 4, -4, 4, -4, 4, -4, 4, -4, 4, -4, 4, -4, 4, -4, 4, -4, 4, -4, 4, -4, 4, -4,
    4, -4, 5, -5, 5, -5, 5, -5, 5, -5, 5, -5, 5, -5, 5, -5, 5, -5, 5, -5, 5, -5, 5, -5, 5, -5,
    5, -5, 5, -5, 5, -5, 5, -5, 5, -5, 5, -5, 5, -5, 4, -4, 4, -4, 4, -4, 4, -4, 4, -4, 4, -4,
    4, -4, 4, -4, 4, -4, 4, -4, 4, -4, 4, -4, 4, -4, 4, -4, 4, -4, 4, -4, 5, -5, 5, -5, 5, -5,
    5, -5, 5, -5, 5, -5, 5, -5, 5, -5, 5, -5, 5, -5, 5, -5, 5, -5, 5, -5, 4, -4, 4, -4, 4, -4,
    4, -4, 4, -4, 4, -4, 4, -4, 4, -4, 5, -5, 5, -5, 5, -5, 5, -5, 5, -5, 5, -5, 5, -5, 5, -5,
    5, -5, 5, -5, 5, -5, 5, -5, 5, -5, 4, -4, 4, -4, 4, -4, 4, -4, 4, -4, 4, -4, 4, -4, 4, -4,
    5, -5, 5, -5, 5, -5, 5, -5, 5, -5, 5, -5, 4, -4, 4, -4, 4, -4, 4, -4, 4, -4, 4, -4, 4, -4,
    4, -4, 5, -5, 5, -5, 5, -5, 5, -5, 5, -5, 5, -5, 5, -5, 5, -5, 5, -5, 5, -5, 5, -5, 5, -5,
    5, -5, 5, -5, 5, -5, 5, -5, 5, -5, 5, -5, 5, -5, 5, -5, 5, -5, 5, -5, 5, -5, 5, -5, 5, -5,
};

#define INDALA26_EMULATION_TIMINGS_COUNT (1024 * 2)
#define INDALA26_TEST_DATA \
    { 0x3B, 0x73, 0x64, 0xA8 }
#define INDALA26_TEST_DATA_SIZE 4

const int8_t indala26_test_timings[INDALA26_EMULATION_TIMINGS_COUNT] = {
    1,  -1, 1,  -1, 1,  -1, 1,  -1, 1,  -1, 1,  -1, 1,  -1, 1,  -1, 1,  -1, 1,  -1, 1,  -1, 1,  -1,
    1,  -1, 1,  -1, 1,  -1, 1,  -1, 1,  -1, 1,  -1, 1,  -1, 1,  -1, 1,  -1, 1,  -1, 1,  -1, 1,  -1,
    1,  -1, 1,  -1, 1,  -1, 1,  -1, 1,  -1, 1,  -1, 1,  -1, 1,  -1, -1, 1,  -1, 1,  -1, 1,  -1, 1,
    -1, 1,  -1, 1,  -1, 1,  -1, 1,  -1, 1,  -1, 1,  -1, 1,  -1, 1,  -1, 1,  -1, 1,  -1, 1,  -1, 1,
    1,  -1, 1,  -1, 1,  -1, 1,  -1, 1,  -1, 1,  -1, 1,  -1, 1,  -1, 1,  -1, 1,  -1, 1,  -1, 1,  -1,
    1,  -1, 1,  -1, 1,  -1, 1,  -1, -1, 1,  -1, 1,  -1, 1,  -1, 1,  -1, 1,  -1, 1,  -1, 1,  -1, 1,
    -1, 1,  -1, 1,  -1, 1,  -1, 1,  -1, 1,  -1, 1,  -1, 1,  -1, 1,  -1, 1,  -1, 1,  -1, 1,  -1, 1,
    -1, 1,  -1, 1,  -1, 1,  -1, 1,  -1, 1,  -1, 1,  -1, 1,  -1, 1,  -1, 1,  -1, 1,  -1, 1,  -1, 1,
    -1, 1,  -1, 1,  -1, 1,  -1, 1,  -1, 1,  -1, 1,  -1, 1,  -1, 1,  -1, 1,  -1, 1,  -1, 1,  -1, 1,
    -1, 1,  -1, 1,  -1, 1,  -1, 1,  -1, 1,  -1, 1,  -1, 1,  -1, 1,  -1, 1,  -1, 1,  -1, 1,  -1, 1,
    -1, 1,  -1, 1,  -1, 1,  -1, 1,  -1, 1,  -1, 1,  -1, 1,  -1, 1,  -1, 1,  -1, 1,  -1, 1,  -1, 1,
    -1, 1,  -1, 1,  -1, 1,  -1, 1,  -1, 1,  -1, 1,  -1, 1,  -1, 1,  -1, 1,  -1, 1,  -1, 1,  -1, 1,
    -1, 1,  -1, 1,  -1, 1,  -1, 1,  -1, 1,  -1, 1,  -1, 1,  -1, 1,  -1, 1,  -1, 1,  -1, 1,  -1, 1,
    -1, 1,  -1, 1,  -1, 1,  -1, 1,  -1, 1,  -1, 1,  -1, 1,  -1, 1,  -1, 1,  -1, 1,  -1, 1,  -1, 1,
    -1, 1,  -1, 1,  -1, 1,  -1, 1,  -1, 1,  -1, 1,  -1, 1,  -1, 1,  -1, 1,  -1, 1,  -1, 1,  -1, 1,
    -1, 1,  -1, 1,  -1, 1,  -1, 1,  -1, 1,  -1, 1,  -1, 1,  -1, 1,  -1, 1,  -1, 1,  -1, 1,  -1, 1,
    -1, 1,  -1, 1,  -1, 1,  -1, 1,  -1, 1,  -1, 1,  -1, 1,  -1, 1,  -1, 1,  -1, 1,  -1, 1,  -1, 1,
    -1, 1,  -1, 1,  -1, 1,  -1, 1,  -1, 1,  -1, 1,  -1, 1,  -1, 1,  -1, 1,  -1, 1,  -1, 1,  -1, 1,
    -1, 1,  -1, 1,  -1, 1,  -1, 1,  -1, 1,  -1, 1,  -1, 1,  -1, 1,  -1, 1,  -1, 1,  -1, 1,  -1, 1,
    -1, 1,  -1, 1,  -1, 1,  -1, 1,  -1, 1,  -1, 1,  -1, 1,  -1, 1,  -1, 1,  -1, 1,  -1, 1,  -1, 1,
    -1, 1,  -1, 1,  -1, 1,  -1, 1,  -1, 1,  -1, 1,  -1, 1,  -1, 1,  -1, 1,  -1, 1,  -1, 1,  -1, 1,
    -1, 1,  -1, 1,  -1, 1,  -1, 1,  -1, 1,  -1, 1,  -1, 1,  -1, 1,  -1, 1,  -1, 1,  -1, 1,  -1, 1,
    -1, 1,  -1, 1,  -1, 1,  -1, 1,  -1, 1,  -1, 1,  -1, 1,  -1, 1,  -1, 1,  -1, 1,  -1, 1,  -1, 1,
    -1, 1,  -1, 1,  -1, 1,  -1, 1,  -1, 1,  -1, 1,  -1, 1,  -1, 1,  -1, 1,  -1, 1,  -1, 1,  -1, 1,
    -1, 1,  -1, 1,  -1, 1,  -1, 1,  -1, 1,  -1, 1,  -1, 1,  -1, 1,  -1, 1,  -1, 1,  -1, 1,  -1, 1,
    -1, 1,  -1, 1,  -1, 1,  -1, 1,  -1, 1,  -1, 1,  -1, 1,  -1, 1,  -1, 1,  -1, 1,  -1, 1,  -1, 1,
    -1, 1,  -1, 1,  -1, 1,  -1, 1,  -1, 1,  -1, 1,  -1, 1,  -1, 1,  -1, 1,  -1, 1,  -1, 1,  -1, 1,
    -1, 1,  -1, 1,  -1, 1,  -1, 1,  -1, 1,  -1, 1,  -1, 1,  -1, 1,  -1, 1,  -1, 1,  -1, 1,  -1, 1,
    -1, 1,  -1, 1,  -1, 1,  -1, 1,  -1, 1,  -1, 1,  -1, 1,  -1, 1,  -1, 1,  -1, 1,  -1, 1,  -1, 1,
    -1, 1,  -1, 1,  -1, 1,  -1, 1,  -1, 1,  -1, 1,  -1, 1,  -1, 1,  -1, 1,  -1, 1,  -1, 1,  -1, 1,
    -1, 1,  -1, 1,  -1, 1,  -1, 1,  -1, 1,  -1, 1,  -1, 1,  -1, 1,  -1, 1,  -1, 1,  -1, 1,  -1, 1,
    -1, 1,  -1, 1,  -1, 1,  -1, 1,  -1, 1,  -1, 1,  -1, 1,  -1, 1,  -1, 1,  -1, 1,  -1, 1,  -1, 1,
    -1, 1,  -1, 1,  -1, 1,  -1, 1,  -1, 1,  -1, 1,  -1, 1,  -1, 1,  -1, 1,  -1, 1,  -1, 1,  -1, 1,
    -1, 1,  -1, 1,  -1, 1,  -1, 1,  -1, 1,  -1, 1,  -1, 1,  -1, 1,  -1, 1,  -1, 1,  -1, 1,  -1, 1,
    -1, 1,  -1, 1,  -1, 1,  -1, 1,  -1, 1,  -1, 1,  -1, 1,  -1, 1,  -1, 1,  -1, 1,  -1, 1,  -1, 1,
    -1, 1,  -1, 1,  -1, 1,  -1, 1,  -1, 1,  -1, 1,  -1, 1,  -1, 1,  -1, 1,  -1, 1,  -1, 1,  -1, 1,
    -1, 1,  -1, 1,  -1, 1,  -1, 1,  -1, 1,  -1, 1,  -1, 1,  -1, 1,  -1, 1,  -1, 1,  -1, 1,  -1, 1,
    -1, 1,  -1, 1,  -1, 1,  -1, 1,  -1, 1,  -1, 1,  -1, 1,  -1, 1,  -1, 1,  -1, 1,  -1, 1,  -1, 1,
    -1, 1,  -1, 1,  -1, 1,  -1, 1,  -1, 1,  -1, 1,  -1, 1,  -1, 1,  -1, 1,  -1, 1,  -1, 1,  -1, 1,
    -1, 1,  -1, 1,  -1, 1,  -1, 1,  -1, 1,  -1, 1,  -1, 1,  -1, 1,  -1, 1,  -1, 1,  -1, 1,  -1, 1,
    -1, 1,  -1, 1,  -1, 1,  -1, 1,  -1, 1,  -1, 1,  -1, 1,  -1, 1,  -1, 1,  -1, 1,  -1, 1,  -1, 1,
    -1, 1,  -1, 1,  -1, 1,  -1, 1,  -1, 1,  -1, 1,  -1, 1,  -1, 1,  -1, 1,  -1, 1,  -1, 1,  -1, 1,
    -1, 1,  -1, 1,  -1, 1,  -1, 1,  -1, 1,  -1, 1,  -1, 1,  -1, 1,  -1, 1,  -1, 1,  -1, 1,  -1, 1,
    -1, 1,  -1, 1,  -1, 1,  -1, 1,  -1, 1,  -1, 1,  -1, 1,  -1, 1,  -1, 1,  -1, 1,  -1, 1,  -1, 1,
    1,  -1, 1,  -1, 1,  -1, 1,  -1, 1,  -1, 1,  -1, 1,  -1, 1,  -1, 1,  -1, 1,  -1, 1,  -1, 1,  -1,
    1,  -1, 1,  -1, 1,  -1, 1,  -1, -1, 1,  -1, 1,  -1, 1,  -1, 1,  -1, 1,  -1, 1,  -1, 1,  -1, 1,
    -1, 1,  -1, 1,  -1, 1,  -1, 1,  -1, 1,  -1, 1,  -1, 1,  -1, 1,  -1, 1,  -1, 1,  -1, 1,  -1, 1,
    -1, 1,  -1, 1,  -1, 1,  -1, 1,  -1, 1,  -1, 1,  -1, 1,  -1, 1,  -1, 1,  -1, 1,  -1, 1,  -1, 1,
    1,  -1, 1,  -1, 1,  -1, 1,  -1, 1,  -1, 1,  -1, 1,  -1, 1,  -1, 1,  -1, 1,  -1, 1,  -1, 1,  -1,
    1,  -1, 1,  -1, 1,  -1, 1,  -1, 1,  -1, 1,  -1, 1,  -1, 1,  -1, 1,  -1, 1,  -1, 1,  -1, 1,  -1,
    1,  -1, 1,  -1, 1,  -1, 1,  -1, 1,  -1, 1,  -1, 1,  -1, 1,  -1, 1,  -1, 1,  -1, 1,  -1, 1,  -1,
    1,  -1, 1,  -1, 1,  -1, 1,  -1, 1,  -1, 1,  -1, 1,  -1, 1,  -1, 1,  -1, 1,  -1, 1,  -1, 1,  -1,
    -1, 1,  -1, 1,  -1, 1,  -1, 1,  -1, 1,  -1, 1,  -1, 1,  -1, 1,  -1, 1,  -1, 1,  -1, 1,  -1, 1,
    -1, 1,  -1, 1,  -1, 1,  -1, 1,  1,  -1, 1,  -1, 1,  -1, 1,  -1, 1,  -1, 1,  -1, 1,  -1, 1,  -1,
    1,  -1, 1,  -1, 1,  -1, 1,  -1, 1,  -1, 1,  -1, 1,  -1, 1,  -1, 1,  -1, 1,  -1, 1,  -1, 1,  -1,
    1,  -1, 1,  -1, 1,  -1, 1,  -1, 1,  -1, 1,  -1, 1,  -1, 1,  -1, 1,  -1, 1,  -1, 1,  -1, 1,  -1,
    -1, 1,  -1, 1,  -1, 1,  -1, 1,  -1, 1,  -1, 1,  -1, 1,  -1, 1,  -1, 1,  -1, 1,  -1, 1,  -1, 1,
    -1, 1,  -1, 1,  -1, 1,  -1, 1,  1,  -1, 1,  -1, 1,  -1, 1,  -1, 1,  -1, 1,  -1, 1,  -1, 1,  -1,
    1,  -1, 1,  -1, 1,  -1, 1,  -1, 1,  -1, 1,  -1, 1,  -1, 1,  -1, 1,  -1, 1,  -1, 1,  -1, 1,  -1,
    1,  -1, 1,  -1, 1,  -1, 1,  -1, 1,  -1, 1,  -1, 1,  -1, 1,  -1, 1,  -1, 1,  -1, 1,  -1, 1,  -1,
    1,  -1, 1,  -1, 1,  -1, 1,  -1, 1,  -1, 1,  -1, 1,  -1, 1,  -1, 1,  -1, 1,  -1, 1,  -1, 1,  -1,
    1,  -1, 1,  -1, 1,  -1, 1,  -1, -1, 1,  -1, 1,  -1, 1,  -1, 1,  -1, 1,  -1, 1,  -1, 1,  -1, 1,
    -1, 1,  -1, 1,  -1, 1,  -1, 1,  -1, 1,  -1, 1,  -1, 1,  -1, 1,  -1, 1,  -1, 1,  -1, 1,  -1, 1,
    -1, 1,  -1, 1,  -1, 1,  -1, 1,  -1, 1,  -1, 1,  -1, 1,  -1, 1,  -1, 1,  -1, 1,  -1, 1,  -1, 1,
    1,  -1, 1,  -1, 1,  -1, 1,  -1, 1,  -1, 1,  -1, 1,  -1, 1,  -1, 1,  -1, 1,  -1, 1,  -1, 1,  -1,
    1,  -1, 1,  -1, 1,  -1, 1,  -1, 1,  -1, 1,  -1, 1,  -1, 1,  -1, 1,  -1, 1,  -1, 1,  -1, 1,  -1,
    1,  -1, 1,  -1, 1,  -1, 1,  -1, 1,  -1, 1,  -1, 1,  -1, 1,  -1, -1, 1,  -1, 1,  -1, 1,  -1, 1,
    -1, 1,  -1, 1,  -1, 1,  -1, 1,  -1, 1,  -1, 1,  -1, 1,  -1, 1,  -1, 1,  -1, 1,  -1, 1,  -1, 1,
    1,  -1, 1,  -1, 1,  -1, 1,  -1, 1,  -1, 1,  -1, 1,  -1, 1,  -1, 1,  -1, 1,  -1, 1,  -1, 1,  -1,
    1,  -1, 1,  -1, 1,  -1, 1,  -1, 1,  -1, 1,  -1, 1,  -1, 1,  -1, 1,  -1, 1,  -1, 1,  -1, 1,  -1,
    1,  -1, 1,  -1, 1,  -1, 1,  -1, 1,  -1, 1,  -1, 1,  -1, 1,  -1, -1, 1,  -1, 1,  -1, 1,  -1, 1,
    -1, 1,  -1, 1,  -1, 1,  -1, 1,  -1, 1,  -1, 1,  -1, 1,  -1, 1,  -1, 1,  -1, 1,  -1, 1,  -1, 1,
    -1, 1,  -1, 1,  -1, 1,  -1, 1,  -1, 1,  -1, 1,  -1, 1,  -1, 1,  -1, 1,  -1, 1,  -1, 1,  -1, 1,
    -1, 1,  -1, 1,  -1, 1,  -1, 1,  1,  -1, 1,  -1, 1,  -1, 1,  -1, 1,  -1, 1,  -1, 1,  -1, 1,  -1,
    1,  -1, 1,  -1, 1,  -1, 1,  -1, 1,  -1, 1,  -1, 1,  -1, 1,  -1, -1, 1,  -1, 1,  -1, 1,  -1, 1,
    -1, 1,  -1, 1,  -1, 1,  -1, 1,  -1, 1,  -1, 1,  -1, 1,  -1, 1,  -1, 1,  -1, 1,  -1, 1,  -1, 1,
    -1, 1,  -1, 1,  -1, 1,  -1, 1,  -1, 1,  -1, 1,  -1, 1,  -1, 1,  -1, 1,  -1, 1,  -1, 1,  -1, 1,
    -1, 1,  -1, 1,  -1, 1,  -1, 1,  1,  -1, 1,  -1, 1,  -1, 1,  -1, 1,  -1, 1,  -1, 1,  -1, 1,  -1,
    1,  -1, 1,  -1, 1,  -1, 1,  -1, 1,  -1, 1,  -1, 1,  -1, 1,  -1, -1, 1,  -1, 1,  -1, 1,  -1, 1,
    -1, 1,  -1, 1,  -1, 1,  -1, 1,  -1, 1,  -1, 1,  -1, 1,  -1, 1,  -1, 1,  -1, 1,  -1, 1,  -1, 1,
    1,  -1, 1,  -1, 1,  -1, 1,  -1, 1,  -1, 1,  -1, 1,  -1, 1,  -1, 1,  -1, 1,  -1, 1,  -1, 1,  -1,
    1,  -1, 1,  -1, 1,  -1, 1,  -1, -1, 1,  -1, 1,  -1, 1,  -1, 1,  -1, 1,  -1, 1,  -1, 1,  -1, 1,
    -1, 1,  -1, 1,  -1, 1,  -1, 1,  -1, 1,  -1, 1,  -1, 1,  -1, 1,  -1, 1,  -1, 1,  -1, 1,  -1, 1,
    -1, 1,  -1, 1,  -1, 1,  -1, 1,  -1, 1,  -1, 1,  -1, 1,  -1, 1,  -1, 1,  -1, 1,  -1, 1,  -1, 1,
    -1, 1,  -1, 1,  -1, 1,  -1, 1,  -1, 1,  -1, 1,  -1, 1,  -1, 1,  -1, 1,  -1, 1,  -1, 1,  -1, 1,
    -1, 1,  -1, 1,  -1, 1,  -1, 1,
};

MU_TEST(test_lfrfid_protocol_em_read_simple) {
    ProtocolDict* dict = protocol_dict_alloc(lfrfid_protocols, LFRFIDProtocolMax);
    mu_assert_int_eq(EM_TEST_DATA_SIZE, protocol_dict_get_data_size(dict, LFRFIDProtocolEM4100));
    mu_assert_string_eq("EM4100", protocol_dict_get_name(dict, LFRFIDProtocolEM4100));
    mu_assert_string_eq("EM-Micro", protocol_dict_get_manufacturer(dict, LFRFIDProtocolEM4100));

    const uint8_t data[EM_TEST_DATA_SIZE] = EM_TEST_DATA;

    protocol_dict_decoders_start(dict);

    ProtocolId protocol = PROTOCOL_NO;
    PulseGlue* pulse_glue = pulse_glue_alloc();

    for(size_t i = 0; i < EM_TEST_EMULATION_TIMINGS_COUNT * 10; i++) {
        bool pulse_pop = pulse_glue_push(
            pulse_glue,
            em_test_timings[i % EM_TEST_EMULATION_TIMINGS_COUNT] >= 0,
            abs(em_test_timings[i % EM_TEST_EMULATION_TIMINGS_COUNT]) *
                LF_RFID_READ_TIMING_MULTIPLIER);

        if(pulse_pop) {
            uint32_t length, period;
            pulse_glue_pop(pulse_glue, &length, &period);

            protocol = protocol_dict_decoders_feed(dict, true, period);
            if(protocol != PROTOCOL_NO) break;

            protocol = protocol_dict_decoders_feed(dict, false, length - period);
            if(protocol != PROTOCOL_NO) break;
        }
    }

    pulse_glue_free(pulse_glue);

    mu_assert_int_eq(LFRFIDProtocolEM4100, protocol);
    uint8_t received_data[EM_TEST_DATA_SIZE] = {0};
    protocol_dict_get_data(dict, protocol, received_data, EM_TEST_DATA_SIZE);

    mu_assert_mem_eq(data, received_data, EM_TEST_DATA_SIZE);

    protocol_dict_free(dict);
}

MU_TEST(test_lfrfid_protocol_em_emulate_simple) {
    ProtocolDict* dict = protocol_dict_alloc(lfrfid_protocols, LFRFIDProtocolMax);
    mu_assert_int_eq(EM_TEST_DATA_SIZE, protocol_dict_get_data_size(dict, LFRFIDProtocolEM4100));
    mu_assert_string_eq("EM4100", protocol_dict_get_name(dict, LFRFIDProtocolEM4100));
    mu_assert_string_eq("EM-Micro", protocol_dict_get_manufacturer(dict, LFRFIDProtocolEM4100));

    const uint8_t data[EM_TEST_DATA_SIZE] = EM_TEST_DATA;

    protocol_dict_set_data(dict, LFRFIDProtocolEM4100, data, EM_TEST_DATA_SIZE);
    mu_check(protocol_dict_encoder_start(dict, LFRFIDProtocolEM4100));

    for(size_t i = 0; i < EM_TEST_EMULATION_TIMINGS_COUNT; i++) {
        LevelDuration level_duration = protocol_dict_encoder_yield(dict, LFRFIDProtocolEM4100);

        if(level_duration_get_level(level_duration)) {
            mu_assert_int_eq(em_test_timings[i], level_duration_get_duration(level_duration));
        } else {
            mu_assert_int_eq(em_test_timings[i], -level_duration_get_duration(level_duration));
        }
    }

    protocol_dict_free(dict);
}

MU_TEST(test_lfrfid_protocol_h10301_read_simple) {
    ProtocolDict* dict = protocol_dict_alloc(lfrfid_protocols, LFRFIDProtocolMax);
    mu_assert_int_eq(
        HID10301_TEST_DATA_SIZE, protocol_dict_get_data_size(dict, LFRFIDProtocolH10301));
    mu_assert_string_eq("H10301", protocol_dict_get_name(dict, LFRFIDProtocolH10301));
    mu_assert_string_eq("HID", protocol_dict_get_manufacturer(dict, LFRFIDProtocolH10301));

    const uint8_t data[HID10301_TEST_DATA_SIZE] = HID10301_TEST_DATA;

    protocol_dict_decoders_start(dict);

    ProtocolId protocol = PROTOCOL_NO;
    PulseGlue* pulse_glue = pulse_glue_alloc();

    for(size_t i = 0; i < HID10301_TEST_EMULATION_TIMINGS_COUNT * 10; i++) {
        bool pulse_pop = pulse_glue_push(
            pulse_glue,
            hid10301_test_timings[i % HID10301_TEST_EMULATION_TIMINGS_COUNT] >= 0,
            abs(hid10301_test_timings[i % HID10301_TEST_EMULATION_TIMINGS_COUNT]) *
                LF_RFID_READ_TIMING_MULTIPLIER);

        if(pulse_pop) {
            uint32_t length, period;
            pulse_glue_pop(pulse_glue, &length, &period);

            protocol = protocol_dict_decoders_feed(dict, true, period);
            if(protocol != PROTOCOL_NO) break;

            protocol = protocol_dict_decoders_feed(dict, false, length - period);
            if(protocol != PROTOCOL_NO) break;
        }
    }

    pulse_glue_free(pulse_glue);

    mu_assert_int_eq(LFRFIDProtocolH10301, protocol);
    uint8_t received_data[HID10301_TEST_DATA_SIZE] = {0};
    protocol_dict_get_data(dict, protocol, received_data, HID10301_TEST_DATA_SIZE);

    mu_assert_mem_eq(data, received_data, HID10301_TEST_DATA_SIZE);

    protocol_dict_free(dict);
}

MU_TEST(test_lfrfid_protocol_h10301_emulate_simple) {
    ProtocolDict* dict = protocol_dict_alloc(lfrfid_protocols, LFRFIDProtocolMax);
    mu_assert_int_eq(
        HID10301_TEST_DATA_SIZE, protocol_dict_get_data_size(dict, LFRFIDProtocolH10301));
    mu_assert_string_eq("H10301", protocol_dict_get_name(dict, LFRFIDProtocolH10301));
    mu_assert_string_eq("HID", protocol_dict_get_manufacturer(dict, LFRFIDProtocolH10301));

    const uint8_t data[HID10301_TEST_DATA_SIZE] = HID10301_TEST_DATA;

    protocol_dict_set_data(dict, LFRFIDProtocolH10301, data, HID10301_TEST_DATA_SIZE);
    mu_check(protocol_dict_encoder_start(dict, LFRFIDProtocolH10301));

    for(size_t i = 0; i < HID10301_TEST_EMULATION_TIMINGS_COUNT; i++) {
        LevelDuration level_duration = protocol_dict_encoder_yield(dict, LFRFIDProtocolH10301);

        if(level_duration_get_level(level_duration)) {
            mu_assert_int_eq(
                hid10301_test_timings[i], level_duration_get_duration(level_duration));
        } else {
            mu_assert_int_eq(
                hid10301_test_timings[i], -level_duration_get_duration(level_duration));
        }
    }

    protocol_dict_free(dict);
}

MU_TEST(test_lfrfid_protocol_ioprox_xsf_read_simple) {
    ProtocolDict* dict = protocol_dict_alloc(lfrfid_protocols, LFRFIDProtocolMax);
    mu_assert_int_eq(
        IOPROX_XSF_TEST_DATA_SIZE, protocol_dict_get_data_size(dict, LFRFIDProtocolIOProxXSF));
    mu_assert_string_eq("IoProxXSF", protocol_dict_get_name(dict, LFRFIDProtocolIOProxXSF));
    mu_assert_string_eq("Kantech", protocol_dict_get_manufacturer(dict, LFRFIDProtocolIOProxXSF));

    const uint8_t data[IOPROX_XSF_TEST_DATA_SIZE] = IOPROX_XSF_TEST_DATA;

    protocol_dict_decoders_start(dict);

    ProtocolId protocol = PROTOCOL_NO;
    PulseGlue* pulse_glue = pulse_glue_alloc();

    for(size_t i = 0; i < IOPROX_XSF_TEST_EMULATION_TIMINGS_COUNT * 10; i++) {
        bool pulse_pop = pulse_glue_push(
            pulse_glue,
            ioprox_xsf_test_timings[i % IOPROX_XSF_TEST_EMULATION_TIMINGS_COUNT] >= 0,
            abs(ioprox_xsf_test_timings[i % IOPROX_XSF_TEST_EMULATION_TIMINGS_COUNT]) *
                LF_RFID_READ_TIMING_MULTIPLIER);

        if(pulse_pop) {
            uint32_t length, period;
            pulse_glue_pop(pulse_glue, &length, &period);

            protocol = protocol_dict_decoders_feed(dict, true, period);
            if(protocol != PROTOCOL_NO) break;

            protocol = protocol_dict_decoders_feed(dict, false, length - period);
            if(protocol != PROTOCOL_NO) break;
        }
    }

    pulse_glue_free(pulse_glue);

    mu_assert_int_eq(LFRFIDProtocolIOProxXSF, protocol);
    uint8_t received_data[IOPROX_XSF_TEST_DATA_SIZE] = {0};
    protocol_dict_get_data(dict, protocol, received_data, IOPROX_XSF_TEST_DATA_SIZE);

    mu_assert_mem_eq(data, received_data, IOPROX_XSF_TEST_DATA_SIZE);

    protocol_dict_free(dict);
}

MU_TEST(test_lfrfid_protocol_ioprox_xsf_emulate_simple) {
    ProtocolDict* dict = protocol_dict_alloc(lfrfid_protocols, LFRFIDProtocolMax);
    mu_assert_int_eq(
        IOPROX_XSF_TEST_DATA_SIZE, protocol_dict_get_data_size(dict, LFRFIDProtocolIOProxXSF));
    mu_assert_string_eq("IoProxXSF", protocol_dict_get_name(dict, LFRFIDProtocolIOProxXSF));
    mu_assert_string_eq("Kantech", protocol_dict_get_manufacturer(dict, LFRFIDProtocolIOProxXSF));

    const uint8_t data[IOPROX_XSF_TEST_DATA_SIZE] = IOPROX_XSF_TEST_DATA;

    protocol_dict_set_data(dict, LFRFIDProtocolIOProxXSF, data, IOPROX_XSF_TEST_DATA_SIZE);
    mu_check(protocol_dict_encoder_start(dict, LFRFIDProtocolIOProxXSF));

    for(size_t i = 0; i < IOPROX_XSF_TEST_EMULATION_TIMINGS_COUNT; i++) {
        LevelDuration level_duration = protocol_dict_encoder_yield(dict, LFRFIDProtocolIOProxXSF);

        if(level_duration_get_level(level_duration)) {
            mu_assert_int_eq(
                ioprox_xsf_test_timings[i], level_duration_get_duration(level_duration));
        } else {
            mu_assert_int_eq(
                ioprox_xsf_test_timings[i], -level_duration_get_duration(level_duration));
        }
    }

    protocol_dict_free(dict);
}

MU_TEST(test_lfrfid_protocol_inadala26_emulate_simple) {
    ProtocolDict* dict = protocol_dict_alloc(lfrfid_protocols, LFRFIDProtocolMax);
    mu_assert_int_eq(
        INDALA26_TEST_DATA_SIZE, protocol_dict_get_data_size(dict, LFRFIDProtocolIndala26));
    mu_assert_string_eq("Indala26", protocol_dict_get_name(dict, LFRFIDProtocolIndala26));
    mu_assert_string_eq("Motorola", protocol_dict_get_manufacturer(dict, LFRFIDProtocolIndala26));

    const uint8_t data[INDALA26_TEST_DATA_SIZE] = INDALA26_TEST_DATA;

    protocol_dict_set_data(dict, LFRFIDProtocolIndala26, data, INDALA26_TEST_DATA_SIZE);
    mu_check(protocol_dict_encoder_start(dict, LFRFIDProtocolIndala26));

    for(size_t i = 0; i < INDALA26_EMULATION_TIMINGS_COUNT; i++) {
        LevelDuration level_duration = protocol_dict_encoder_yield(dict, LFRFIDProtocolIndala26);

        if(level_duration_get_level(level_duration)) {
            mu_assert_int_eq(
                indala26_test_timings[i], level_duration_get_duration(level_duration));
        } else {
            mu_assert_int_eq(
                indala26_test_timings[i], -level_duration_get_duration(level_duration));
        }
    }

    protocol_dict_free(dict);
}

MU_TEST_SUITE(test_lfrfid_protocols_suite) {
    MU_RUN_TEST(test_lfrfid_protocol_em_read_simple);
    MU_RUN_TEST(test_lfrfid_protocol_em_emulate_simple);

    MU_RUN_TEST(test_lfrfid_protocol_h10301_read_simple);
    MU_RUN_TEST(test_lfrfid_protocol_h10301_emulate_simple);

    MU_RUN_TEST(test_lfrfid_protocol_ioprox_xsf_read_simple);
    MU_RUN_TEST(test_lfrfid_protocol_ioprox_xsf_emulate_simple);

    MU_RUN_TEST(test_lfrfid_protocol_inadala26_emulate_simple);
}

int run_minunit_test_lfrfid_protocols() {
    MU_RUN_SUITE(test_lfrfid_protocols_suite);
    return MU_EXIT_CODE;
}