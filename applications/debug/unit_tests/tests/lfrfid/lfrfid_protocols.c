#include <furi.h>
#include "../test.h" // IWYU pragma: keep
#include <toolbox/protocols/protocol_dict.h>
#include <lfrfid/protocols/lfrfid_protocols.h>
#include <lfrfid/lfrfid_write_targets.h>
#include <toolbox/pulse_protocols/pulse_glue.h>

#define LF_RFID_READ_TIMING_MULTIPLIER 8

#define EM_TEST_DATA                    {0x58, 0x00, 0x85, 0x64, 0x02}
#define EM_TEST_DATA_SIZE               5
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

#define HID10301_TEST_DATA                    {0x8D, 0x48, 0xA8}
#define HID10301_TEST_DATA_SIZE               3
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

#define IOPROX_XSF_TEST_DATA                    {0x65, 0x01, 0x05, 0x39}
#define IOPROX_XSF_TEST_DATA_SIZE               4
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
#define INDALA26_TEST_DATA               {0x3B, 0x73, 0x64, 0xA8}
#define INDALA26_TEST_DATA_SIZE          4

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

#define FDXB_TEST_DATA                    {0x44, 0x88, 0x23, 0xF2, 0x5A, 0x6F, 0x00, 0x01, 0x00, 0x00, 0x00}
#define FDXB_TEST_DATA_SIZE               11
#define FDXB_TEST_EMULATION_TIMINGS_COUNT (206)

const int8_t fdxb_test_timings[FDXB_TEST_EMULATION_TIMINGS_COUNT] = {
    32,  -16, 16,  -16, 16,  -16, 16,  -16, 16,  -16, 16,  -16, 16,  -16, 16,  -16, 16,  -16, 16,
    -16, 16,  -32, 16,  -16, 32,  -16, 16,  -16, 16,  -16, 16,  -32, 16,  -16, 16,  -16, 32,  -32,
    16,  -16, 16,  -16, 16,  -16, 32,  -16, 16,  -16, 16,  -16, 16,  -32, 16,  -16, 16,  -16, 32,
    -16, 16,  -16, 16,  -16, 16,  -32, 32,  -32, 32,  -32, 32,  -32, 16,  -16, 16,  -16, 32,  -16,
    16,  -32, 16,  -16, 32,  -16, 16,  -32, 32,  -16, 16,  -32, 16,  -16, 32,  -16, 16,  -32, 32,
    -16, 16,  -32, 32,  -32, 32,  -32, 16,  -16, 16,  -16, 16,  -16, 16,  -16, 16,  -16, 16,  -16,
    16,  -16, 16,  -16, 32,  -16, 16,  -16, 16,  -16, 16,  -16, 16,  -16, 16,  -16, 16,  -16, 16,
    -32, 32,  -32, 32,  -32, 32,  -32, 16,  -16, 32,  -32, 32,  -16, 16,  -16, 16,  -32, 32,  -32,
    32,  -32, 32,  -32, 16,  -16, 16,  -16, 16,  -16, 16,  -16, 16,  -16, 16,  -16, 16,  -16, 16,
    -16, 32,  -16, 16,  -16, 16,  -16, 16,  -16, 16,  -16, 16,  -16, 16,  -16, 16,  -16, 16,  -32,
    16,  -16, 16,  -16, 16,  -16, 16,  -16, 16,  -16, 16,  -16, 16,  -16, 16,  -16,
};

#define KERI_TEST_DATA        {0x80, 0x00, 0x30, 0x39}
#define KERI_TEST_DATA_SIZE   4
#define KERI_TEST_RUNS_COUNT  4
#define KERI_TEST_FRAME_COUNT 40

// One frame of a real RF/32 PSK1 Keri tag (internal ID 12345, T5577 blocks
// 00000004 / 000181CF) captured with `rfid raw_read psk`: the high and low
// length of each run, in microseconds. 64 bits in runs of 4H/29L, 1H/17L, 2H/6L,
// 3H/2L totalling 16373 us, so ~255.8 us per bit, with every high run ~135 us
// long and every low run ~135 us short. Uncorrected these total 66 bits, the
// preamble never aligns, and the frame does not decode at all. Fed straight to
// the decoder rather than through PulseGlue, as the PSK read path does.
static const uint16_t keri_test_timings[KERI_TEST_RUNS_COUNT][2] = {
    {1158, 7288},
    {392, 4214},
    {647, 1396},
    {902, 376},
};

// The same frame with the 2H/6L run pair read as 3H/5L, setting encoded bit 52
// so it carries 0x80003839 rather than 0x80003039. Each decodes on its own, so
// alternating them is a tag whose two frames disagree.
static const uint16_t keri_test_timings_alt[KERI_TEST_RUNS_COUNT][2] = {
    {1158, 7288},
    {392, 4214},
    {902, 1141},
    {902, 376},
};

MU_TEST(test_lfrfid_protocol_keri_read_simple) {
    ProtocolDict* dict = protocol_dict_alloc(lfrfid_protocols, LFRFIDProtocolMax);
    mu_assert_int_eq(KERI_TEST_DATA_SIZE, protocol_dict_get_data_size(dict, LFRFIDProtocolKeri));
    mu_assert_string_eq("Keri", protocol_dict_get_name(dict, LFRFIDProtocolKeri));
    mu_assert_string_eq("Keri", protocol_dict_get_manufacturer(dict, LFRFIDProtocolKeri));

    const uint8_t data[KERI_TEST_DATA_SIZE] = KERI_TEST_DATA;

    protocol_dict_decoders_start(dict);

    ProtocolId protocol = PROTOCOL_NO;

    for(size_t i = 0; i < KERI_TEST_RUNS_COUNT * KERI_TEST_FRAME_COUNT; i++) {
        const uint16_t* run = keri_test_timings[i % KERI_TEST_RUNS_COUNT];

        protocol = protocol_dict_decoders_feed(dict, true, run[0]);
        if(protocol != PROTOCOL_NO) break;

        protocol = protocol_dict_decoders_feed(dict, false, run[1]);
        if(protocol != PROTOCOL_NO) break;
    }

    mu_assert_int_eq(LFRFIDProtocolKeri, protocol);
    uint8_t received_data[KERI_TEST_DATA_SIZE] = {0};
    protocol_dict_get_data(dict, protocol, received_data, KERI_TEST_DATA_SIZE);

    mu_assert_mem_eq(data, received_data, KERI_TEST_DATA_SIZE);

    protocol_dict_free(dict);
}

// Frames that disagree must not decode. Without the ID check the stream reads
// out as whichever frame lands first, a credential the tag never presented.
MU_TEST(test_lfrfid_protocol_keri_read_mismatched_frames) {
    ProtocolDict* dict = protocol_dict_alloc(lfrfid_protocols, LFRFIDProtocolMax);

    protocol_dict_decoders_start(dict);

    ProtocolId protocol = PROTOCOL_NO;

    for(size_t i = 0; i < KERI_TEST_RUNS_COUNT * KERI_TEST_FRAME_COUNT; i++) {
        const uint16_t* run = ((i / KERI_TEST_RUNS_COUNT) % 2 == 0) ?
                                  keri_test_timings[i % KERI_TEST_RUNS_COUNT] :
                                  keri_test_timings_alt[i % KERI_TEST_RUNS_COUNT];

        protocol = protocol_dict_decoders_feed(dict, true, run[0]);
        if(protocol != PROTOCOL_NO) break;

        protocol = protocol_dict_decoders_feed(dict, false, run[1]);
        if(protocol != PROTOCOL_NO) break;
    }

    mu_assert_int_eq(PROTOCOL_NO, protocol);

    protocol_dict_free(dict);
}

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

MU_TEST(test_lfrfid_protocol_hid_generic_render) {
    ProtocolDict* dict = protocol_dict_alloc(lfrfid_protocols, LFRFIDProtocolMax);
    FuriString* result = furi_string_alloc();

    // The size header: a 1 in the first six bits is a 38- to 43-bit frame, bit 6 clear a
    // 37-bit one, bit 6 set a shorter frame that starts right after the next 1, down to
    // 26 bits. The data is shown in hex by the caller, the render only names the length.
    static const struct {
        uint8_t data[6];
        const char* text;
    } cases[] = {
        // bit 0 set: 43-bit frame
        {{0x80, 0x00, 0x00, 0x00, 0x00, 0x00}, "43-bit HID Proximity"},
        // bit 5 set: 38-bit frame
        {{0x04, 0x00, 0x00, 0x00, 0x00, 0x00}, "38-bit HID Proximity"},
        // no header: 37-bit frame
        {{0x00, 0x90, 0x08, 0x00, 0x40, 0x00}, "37-bit HID Proximity"},
        // bit 6 then a 1 at bit 7: 36-bit frame
        {{0x03, 0x00, 0x00, 0x00, 0x00, 0x00}, "36-bit HID Proximity"},
        // bit 6 then a 1 at bit 8: 35-bit frame
        {{0x02, 0x80, 0x08, 0x00, 0x00, 0x00}, "35-bit HID Proximity"},
        // bit 6 then a 1 at bit 9: 34-bit frame
        {{0x02, 0x40, 0x00, 0x02, 0x00, 0x00}, "34-bit HID Proximity"},
        // bit 6 then a 1 at bit 17: 26-bit frame
        {{0x02, 0x00, 0x60, 0x40, 0x00, 0x80}, "26-bit HID Proximity"},
        // bit 6 and no 1 before bit 18: shorter than 26, not a frame
        {{0x02, 0x00, 0x00, 0x00, 0x00, 0x80}, "Generic HID Proximity"},
    };

    for(size_t i = 0; i < COUNT_OF(cases); i++) {
        protocol_dict_set_data(
            dict, LFRFIDProtocolHidGeneric, cases[i].data, sizeof(cases[i].data));
        protocol_dict_render_data(dict, result, LFRFIDProtocolHidGeneric);
        mu_assert_string_eq(cases[i].text, furi_string_get_cstr(result));
    }

    furi_string_free(result);
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

MU_TEST(test_lfrfid_protocol_fdxb_emulate_simple) {
    ProtocolDict* dict = protocol_dict_alloc(lfrfid_protocols, LFRFIDProtocolMax);
    mu_assert_int_eq(FDXB_TEST_DATA_SIZE, protocol_dict_get_data_size(dict, LFRFIDProtocolFDXB));
    mu_assert_string_eq("FDX-B", protocol_dict_get_name(dict, LFRFIDProtocolFDXB));
    mu_assert_string_eq("ISO", protocol_dict_get_manufacturer(dict, LFRFIDProtocolFDXB));

    const uint8_t data[FDXB_TEST_DATA_SIZE] = FDXB_TEST_DATA;

    protocol_dict_set_data(dict, LFRFIDProtocolFDXB, data, FDXB_TEST_DATA_SIZE);
    mu_check(protocol_dict_encoder_start(dict, LFRFIDProtocolFDXB));

    for(size_t i = 0; i < FDXB_TEST_EMULATION_TIMINGS_COUNT; i++) {
        LevelDuration level_duration = protocol_dict_encoder_yield(dict, LFRFIDProtocolFDXB);

        if(level_duration_get_level(level_duration)) {
            mu_assert_int_eq(fdxb_test_timings[i], level_duration_get_duration(level_duration));
        } else {
            mu_assert_int_eq(fdxb_test_timings[i], -level_duration_get_duration(level_duration));
        }
    }

    protocol_dict_free(dict);
}

MU_TEST(test_lfrfid_protocol_fdxb_read_simple) {
    ProtocolDict* dict = protocol_dict_alloc(lfrfid_protocols, LFRFIDProtocolMax);
    mu_assert_int_eq(FDXB_TEST_DATA_SIZE, protocol_dict_get_data_size(dict, LFRFIDProtocolFDXB));
    mu_assert_string_eq("FDX-B", protocol_dict_get_name(dict, LFRFIDProtocolFDXB));
    mu_assert_string_eq("ISO", protocol_dict_get_manufacturer(dict, LFRFIDProtocolFDXB));

    const uint8_t data[FDXB_TEST_DATA_SIZE] = FDXB_TEST_DATA;

    protocol_dict_decoders_start(dict);

    ProtocolId protocol = PROTOCOL_NO;
    PulseGlue* pulse_glue = pulse_glue_alloc();

    for(size_t i = 0; i < FDXB_TEST_EMULATION_TIMINGS_COUNT * 10; i++) {
        bool pulse_pop = pulse_glue_push(
            pulse_glue,
            fdxb_test_timings[i % FDXB_TEST_EMULATION_TIMINGS_COUNT] >= 0,
            abs(fdxb_test_timings[i % FDXB_TEST_EMULATION_TIMINGS_COUNT]) *
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

    mu_assert_int_eq(LFRFIDProtocolFDXB, protocol);
    uint8_t received_data[FDXB_TEST_DATA_SIZE] = {0};
    protocol_dict_get_data(dict, protocol, received_data, FDXB_TEST_DATA_SIZE);

    mu_assert_mem_eq(data, received_data, FDXB_TEST_DATA_SIZE);

    protocol_dict_free(dict);
}

// Indala224: 224-bit PSK2 frame, no known FC/CN descramble,
// test data uses the Proxmark3-verified reference (lf indala reader output)
#define INDALA224_TEST_DATA_SIZE 28
#define INDALA224_TEST_DATA                                                              \
    {0x80, 0x00, 0x00, 0x01, 0xB2, 0x35, 0x23, 0xA6, 0xC2, 0xE3, 0x1E, 0xBA, 0x3C, 0xBE, \
     0xE4, 0xAF, 0xB3, 0xC6, 0xAD, 0x1F, 0xCF, 0x64, 0x93, 0x93, 0x92, 0x8C, 0x14, 0xE5}
#define INDALA224_BITS_PER_FRAME   224
#define INDALA224_US_PER_BIT       255
#define INDALA224_FRAMES_TO_DECODE 3

MU_TEST(test_lfrfid_protocol_indala224_roundtrip) {
    ProtocolDict* dict = protocol_dict_alloc(lfrfid_protocols, LFRFIDProtocolMax);
    mu_assert_int_eq(
        INDALA224_TEST_DATA_SIZE, protocol_dict_get_data_size(dict, LFRFIDProtocolIndala224));
    mu_assert_string_eq("Indala224", protocol_dict_get_name(dict, LFRFIDProtocolIndala224));
    mu_assert_string_eq("Motorola", protocol_dict_get_manufacturer(dict, LFRFIDProtocolIndala224));

    const uint8_t data[INDALA224_TEST_DATA_SIZE] = INDALA224_TEST_DATA;

    // Encode: extract bit-level polarity from encoder output.
    // PSK encoder yields carrier-level oscillation (duration=1 per half-cycle).
    // PulseGlue produces ~16us outputs which are below the decoder's 127us threshold,
    // so we extract the bit-level polarity directly: sample the encoder's phase state
    // at the start of each bit period (every 32 yields = 16 carrier cycles).
    protocol_dict_set_data(dict, LFRFIDProtocolIndala224, data, INDALA224_TEST_DATA_SIZE);
    mu_check(protocol_dict_encoder_start(dict, LFRFIDProtocolIndala224));

    const size_t total_bits = INDALA224_BITS_PER_FRAME * INDALA224_FRAMES_TO_DECODE;
    bool* bit_levels = malloc(total_bits);
    mu_check(bit_levels != NULL);

    for(size_t i = 0; i < total_bits; i++) {
        LevelDuration ld = protocol_dict_encoder_yield(dict, LFRFIDProtocolIndala224);
        bit_levels[i] = level_duration_get_level(ld);
        // Skip remaining 31 yields for this bit period
        for(size_t skip = 0; skip < 31; skip++) {
            protocol_dict_encoder_yield(dict, LFRFIDProtocolIndala224);
        }
    }

    // Decode: convert bit-level polarities to run-length timing pairs
    // and feed directly to decoder (simulates hardware PSK demodulator output).
    protocol_dict_decoders_start(dict);
    ProtocolId protocol = PROTOCOL_NO;

    size_t i = 0;
    while(i < total_bits) {
        bool cur = bit_levels[i];
        size_t run = 1;
        while(i + run < total_bits && bit_levels[i + run] == cur) {
            run++;
        }
        uint32_t duration = (uint32_t)(run * INDALA224_US_PER_BIT);

        protocol = protocol_dict_decoders_feed(dict, cur, duration);
        if(protocol != PROTOCOL_NO) break;

        i += run;
    }

    free(bit_levels);

    mu_assert_int_eq(LFRFIDProtocolIndala224, protocol);
    uint8_t received_data[INDALA224_TEST_DATA_SIZE] = {0};
    protocol_dict_get_data(dict, protocol, received_data, INDALA224_TEST_DATA_SIZE);
    mu_assert_mem_eq(data, received_data, INDALA224_TEST_DATA_SIZE);

    protocol_dict_free(dict);
}

// Indala224 phase-alternating test: data with odd number of 1-bits
// causes PSK2 carrier phase to invert between consecutive frames.
// Decoder must accept inverted preamble at the second frame boundary.
#define INDALA224_ALT_TEST_DATA                                                          \
    {0x80, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, \
     0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00}

MU_TEST(test_lfrfid_protocol_indala224_alternating_phase) {
    ProtocolDict* dict = protocol_dict_alloc(lfrfid_protocols, LFRFIDProtocolMax);

    const uint8_t data[INDALA224_TEST_DATA_SIZE] = INDALA224_ALT_TEST_DATA;

    protocol_dict_set_data(dict, LFRFIDProtocolIndala224, data, INDALA224_TEST_DATA_SIZE);
    mu_check(protocol_dict_encoder_start(dict, LFRFIDProtocolIndala224));

    const size_t total_bits = INDALA224_BITS_PER_FRAME * INDALA224_FRAMES_TO_DECODE;
    bool* bit_levels = malloc(total_bits);
    mu_check(bit_levels != NULL);

    for(size_t i = 0; i < total_bits; i++) {
        LevelDuration ld = protocol_dict_encoder_yield(dict, LFRFIDProtocolIndala224);
        bit_levels[i] = level_duration_get_level(ld);
        for(size_t skip = 0; skip < 31; skip++) {
            protocol_dict_encoder_yield(dict, LFRFIDProtocolIndala224);
        }
    }

    // Verify phase alternation: frame 1 and frame 2 start with opposite polarity
    mu_check(bit_levels[0] != bit_levels[INDALA224_BITS_PER_FRAME]);

    protocol_dict_decoders_start(dict);
    ProtocolId protocol = PROTOCOL_NO;

    size_t i = 0;
    while(i < total_bits) {
        bool cur = bit_levels[i];
        size_t run = 1;
        while(i + run < total_bits && bit_levels[i + run] == cur) {
            run++;
        }
        uint32_t duration = (uint32_t)(run * INDALA224_US_PER_BIT);

        protocol = protocol_dict_decoders_feed(dict, cur, duration);
        if(protocol != PROTOCOL_NO) break;

        i += run;
    }

    free(bit_levels);

    mu_assert_int_eq(LFRFIDProtocolIndala224, protocol);
    uint8_t received_data[INDALA224_TEST_DATA_SIZE] = {0};
    protocol_dict_get_data(dict, protocol, received_data, INDALA224_TEST_DATA_SIZE);
    mu_assert_mem_eq(data, received_data, INDALA224_TEST_DATA_SIZE);

    protocol_dict_free(dict);
}

// The Hitag S writer's own vectors: CRC-8, every frame builder, and the anticollision decoder,
// against datasheet and Proxmark3 references. None of it is reachable from here directly - the
// module keeps it all static and behind an RF session - so it reports the first failing vector
// by name instead.
MU_TEST(test_lfrfid_hitags_frames_and_decoder) {
    const char* failure = hitags_selftest();
    mu_assert(failure == NULL, failure ? failure : "");
}

// The write targets a key can be offered, and the opt-in default that keeps the Hitag S write -
// the one that can destroy a card it was not meant for - off until the user asks for it.
MU_TEST(test_lfrfid_hitags_write_target) {
    mu_assert_string_eq("8268", lfrfid_write_target_name(LFRFIDWriteTargetHitagS8268));

    mu_check(LFRFID_WRITE_TARGET_MASK_ALL & LFRFID_WRITE_TARGET_BIT(LFRFIDWriteTargetHitagS8268));
    mu_assert_int_eq(
        LFRFID_WRITE_TARGET_MASK_ALL & ~LFRFID_WRITE_TARGET_BIT(LFRFIDWriteTargetHitagS8268),
        lfrfid_write_targets_default());

    // End to end: target -> write type -> the protocol's encoder -> the mask the worker consults.
    ProtocolDict* dict = protocol_dict_alloc(lfrfid_protocols, LFRFIDProtocolMax);
    const uint8_t data[] = EM_TEST_DATA;

    protocol_dict_set_data(dict, LFRFIDProtocolEM4100, data, EM_TEST_DATA_SIZE);
    mu_check(
        lfrfid_write_targets_supported(dict, LFRFIDProtocolEM4100) &
        LFRFID_WRITE_TARGET_BIT(LFRFIDWriteTargetHitagS8268));

    protocol_dict_set_data(dict, LFRFIDProtocolEM4100_32, data, EM_TEST_DATA_SIZE);
    mu_check(
        !(lfrfid_write_targets_supported(dict, LFRFIDProtocolEM4100_32) &
          LFRFID_WRITE_TARGET_BIT(LFRFIDWriteTargetHitagS8268)));

    protocol_dict_free(dict);
}

// The Hitag S pages are the two halves of the same 64-bit EM4100 frame the T5577 blocks carry, so
// check them against each other rather than against a hand-computed constant.
MU_TEST(test_lfrfid_protocol_em_write_hitags) {
    ProtocolDict* dict = protocol_dict_alloc(lfrfid_protocols, LFRFIDProtocolMax);
    const uint8_t data[] = EM_TEST_DATA;

    LFRFIDWriteRequest via_t5577 = {.write_type = LFRFIDWriteTypeT5577};
    protocol_dict_set_data(dict, LFRFIDProtocolEM4100, data, EM_TEST_DATA_SIZE);
    mu_check(protocol_dict_get_write_data(dict, LFRFIDProtocolEM4100, &via_t5577));

    // Encoding modifies the protocol's data, so restore it before encoding again.
    LFRFIDWriteRequest via_hitags = {.write_type = LFRFIDWriteTypeHitagS};
    protocol_dict_set_data(dict, LFRFIDProtocolEM4100, data, EM_TEST_DATA_SIZE);
    mu_check(protocol_dict_get_write_data(dict, LFRFIDProtocolEM4100, &via_hitags));

    // Against the frame itself, so a symmetric change to both byte-split loops cannot pass.
    const uint8_t expected_page4[] = {0xFF, 0xAA, 0x20, 0x04};
    const uint8_t expected_page5[] = {0x54, 0xC4, 0x80, 0xA0};
    mu_assert_mem_eq(expected_page4, via_hitags.hitags.page4, LFRFID_HITAGS_PAGE_SIZE);
    mu_assert_mem_eq(expected_page5, via_hitags.hitags.page5, LFRFID_HITAGS_PAGE_SIZE);

    // And against the T5577 encoding of the same key, which says the two writers agree.
    for(uint8_t i = 0; i < LFRFID_HITAGS_PAGE_SIZE; i++) {
        mu_assert_int_eq(
            (via_t5577.t5577.block[1] >> (24 - i * 8)) & 0xFF, via_hitags.hitags.page4[i]);
        mu_assert_int_eq(
            (via_t5577.t5577.block[2] >> (24 - i * 8)) & 0xFF, via_hitags.hitags.page5[i]);
    }

    // The chip's factory config streams those pages at 2 kBit, which is RF/64 and nothing else, so
    // the faster EM4100 variants must be refused rather than written at the wrong rate.
    LFRFIDWriteRequest wrong_clock = {.write_type = LFRFIDWriteTypeHitagS};
    protocol_dict_set_data(dict, LFRFIDProtocolEM4100_32, data, EM_TEST_DATA_SIZE);
    mu_check(!protocol_dict_get_write_data(dict, LFRFIDProtocolEM4100_32, &wrong_clock));
    protocol_dict_set_data(dict, LFRFIDProtocolEM4100_16, data, EM_TEST_DATA_SIZE);
    mu_check(!protocol_dict_get_write_data(dict, LFRFIDProtocolEM4100_16, &wrong_clock));

    // Positive control: those two protocols do encode for another chip, so the refusals above
    // are the clock gate talking and not a mis-wired protocol entry.
    LFRFIDWriteRequest other_chip = {.write_type = LFRFIDWriteTypeT5577};
    protocol_dict_set_data(dict, LFRFIDProtocolEM4100_32, data, EM_TEST_DATA_SIZE);
    mu_check(protocol_dict_get_write_data(dict, LFRFIDProtocolEM4100_32, &other_chip));
    protocol_dict_set_data(dict, LFRFIDProtocolEM4100_16, data, EM_TEST_DATA_SIZE);
    mu_check(protocol_dict_get_write_data(dict, LFRFIDProtocolEM4100_16, &other_chip));

    protocol_dict_free(dict);
}

MU_TEST_SUITE(test_lfrfid_protocols_suite) {
    MU_RUN_TEST(test_lfrfid_protocol_em_read_simple);
    MU_RUN_TEST(test_lfrfid_protocol_em_emulate_simple);
    MU_RUN_TEST(test_lfrfid_protocol_em_write_hitags);

    MU_RUN_TEST(test_lfrfid_hitags_frames_and_decoder);
    MU_RUN_TEST(test_lfrfid_hitags_write_target);

    MU_RUN_TEST(test_lfrfid_protocol_h10301_read_simple);
    MU_RUN_TEST(test_lfrfid_protocol_h10301_emulate_simple);
    MU_RUN_TEST(test_lfrfid_protocol_hid_generic_render);

    MU_RUN_TEST(test_lfrfid_protocol_ioprox_xsf_read_simple);
    MU_RUN_TEST(test_lfrfid_protocol_ioprox_xsf_emulate_simple);

    MU_RUN_TEST(test_lfrfid_protocol_inadala26_emulate_simple);

    MU_RUN_TEST(test_lfrfid_protocol_indala224_roundtrip);
    MU_RUN_TEST(test_lfrfid_protocol_indala224_alternating_phase);

    MU_RUN_TEST(test_lfrfid_protocol_keri_read_simple);
    MU_RUN_TEST(test_lfrfid_protocol_keri_read_mismatched_frames);

    MU_RUN_TEST(test_lfrfid_protocol_fdxb_read_simple);
    MU_RUN_TEST(test_lfrfid_protocol_fdxb_emulate_simple);
}

int run_minunit_test_lfrfid_protocols(void) {
    MU_RUN_SUITE(test_lfrfid_protocols_suite);
    return MU_EXIT_CODE;
}

TEST_API_DEFINE(run_minunit_test_lfrfid_protocols)
