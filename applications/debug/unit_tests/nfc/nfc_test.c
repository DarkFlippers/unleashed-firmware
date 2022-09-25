#include <furi.h>
#include <furi_hal.h>
#include <storage/storage.h>
#include <lib/flipper_format/flipper_format.h>
#include <lib/nfc/protocols/nfca.h>
#include <lib/nfc/helpers/mf_classic_dict.h>
#include <lib/digital_signal/digital_signal.h>

#include <lib/flipper_format/flipper_format_i.h>
#include <lib/toolbox/stream/file_stream.h>

#include "../minunit.h"

#define TAG "NfcTest"

#define NFC_TEST_RESOURCES_DIR EXT_PATH("unit_tests/nfc/")
#define NFC_TEST_SIGNAL_SHORT_FILE "nfc_nfca_signal_short.nfc"
#define NFC_TEST_SIGNAL_LONG_FILE "nfc_nfca_signal_long.nfc"

static const char* nfc_test_file_type = "Flipper NFC test";
static const uint32_t nfc_test_file_version = 1;

#define NFC_TEST_DATA_MAX_LEN 18
#define NFC_TETS_TIMINGS_MAX_LEN 1350

typedef struct {
    Storage* storage;
    NfcaSignal* signal;
    uint32_t test_data_len;
    uint8_t test_data[NFC_TEST_DATA_MAX_LEN];
    uint32_t test_timings_len;
    uint32_t test_timings[NFC_TETS_TIMINGS_MAX_LEN];
} NfcTest;

static NfcTest* nfc_test = NULL;

static void nfc_test_alloc() {
    nfc_test = malloc(sizeof(NfcTest));
    nfc_test->signal = nfca_signal_alloc();
    nfc_test->storage = furi_record_open(RECORD_STORAGE);
}

static void nfc_test_free() {
    furi_assert(nfc_test);

    furi_record_close(RECORD_STORAGE);
    nfca_signal_free(nfc_test->signal);
    free(nfc_test);
    nfc_test = NULL;
}

static bool nfc_test_read_signal_from_file(const char* file_name) {
    bool success = false;

    FlipperFormat* file = flipper_format_file_alloc(nfc_test->storage);
    string_t file_type;
    string_init(file_type);
    uint32_t file_version = 0;

    do {
        if(!flipper_format_file_open_existing(file, file_name)) break;
        if(!flipper_format_read_header(file, file_type, &file_version)) break;
        if(string_cmp_str(file_type, nfc_test_file_type) || file_version != nfc_test_file_version)
            break;
        if(!flipper_format_read_uint32(file, "Data length", &nfc_test->test_data_len, 1)) break;
        if(nfc_test->test_data_len > NFC_TEST_DATA_MAX_LEN) break;
        if(!flipper_format_read_hex(
               file, "Plain data", nfc_test->test_data, nfc_test->test_data_len))
            break;
        if(!flipper_format_read_uint32(file, "Timings length", &nfc_test->test_timings_len, 1))
            break;
        if(nfc_test->test_timings_len > NFC_TETS_TIMINGS_MAX_LEN) break;
        if(!flipper_format_read_uint32(
               file, "Timings", nfc_test->test_timings, nfc_test->test_timings_len))
            break;
        success = true;
    } while(false);

    string_clear(file_type);
    flipper_format_free(file);

    return success;
}

static bool nfc_test_digital_signal_test_encode(
    const char* file_name,
    uint32_t encode_max_time,
    uint32_t timing_tolerance,
    uint32_t timings_sum_tolerance) {
    furi_assert(nfc_test);

    bool success = false;
    uint32_t time = 0;
    uint32_t dut_timings_sum = 0;
    uint32_t ref_timings_sum = 0;
    uint8_t parity[10] = {};

    do {
        // Read test data
        if(!nfc_test_read_signal_from_file(file_name)) break;

        // Encode signal
        FURI_CRITICAL_ENTER();
        time = DWT->CYCCNT;
        nfca_signal_encode(
            nfc_test->signal, nfc_test->test_data, nfc_test->test_data_len * 8, parity);
        digital_signal_prepare_arr(nfc_test->signal->tx_signal);
        time = (DWT->CYCCNT - time) / furi_hal_cortex_instructions_per_microsecond();
        FURI_CRITICAL_EXIT();

        // Check timings
        if(time > encode_max_time) {
            FURI_LOG_E(
                TAG, "Encoding time: %d us while accepted value: %d us", time, encode_max_time);
            break;
        }

        // Check data
        if(nfc_test->signal->tx_signal->edge_cnt != nfc_test->test_timings_len) {
            FURI_LOG_E(TAG, "Not equal timings buffers length");
            break;
        }

        uint32_t timings_diff = 0;
        uint32_t* ref = nfc_test->test_timings;
        uint32_t* dut = nfc_test->signal->tx_signal->reload_reg_buff;
        bool timing_check_success = true;
        for(size_t i = 0; i < nfc_test->test_timings_len; i++) {
            timings_diff = dut[i] > ref[i] ? dut[i] - ref[i] : ref[i] - dut[i];
            dut_timings_sum += dut[i];
            ref_timings_sum += ref[i];
            if(timings_diff > timing_tolerance) {
                FURI_LOG_E(
                    TAG, "Too big differece in %d timings. Ref: %d, DUT: %d", i, ref[i], dut[i]);
                timing_check_success = false;
                break;
            }
        }
        if(!timing_check_success) break;
        uint32_t sum_diff = dut_timings_sum > ref_timings_sum ? dut_timings_sum - ref_timings_sum :
                                                                ref_timings_sum - dut_timings_sum;
        if(sum_diff > timings_sum_tolerance) {
            FURI_LOG_E(
                TAG,
                "Too big difference in timings sum. Ref: %d, DUT: %d",
                ref_timings_sum,
                dut_timings_sum);
            break;
        }

        FURI_LOG_I(TAG, "Encoding time: %d us. Acceptable time: %d us", time, encode_max_time);
        FURI_LOG_I(
            TAG,
            "Timings sum difference: %d [1/64MHZ]. Acceptable difference: %d [1/64MHz]",
            sum_diff,
            timings_sum_tolerance);
        success = true;
    } while(false);

    return success;
}

MU_TEST(nfc_digital_signal_test) {
    mu_assert(
        nfc_test_digital_signal_test_encode(
            NFC_TEST_RESOURCES_DIR NFC_TEST_SIGNAL_SHORT_FILE, 500, 1, 37),
        "NFC short digital signal test failed\r\n");
    mu_assert(
        nfc_test_digital_signal_test_encode(
            NFC_TEST_RESOURCES_DIR NFC_TEST_SIGNAL_LONG_FILE, 2000, 1, 37),
        "NFC long digital signal test failed\r\n");
}

MU_TEST(mf_classic_dict_test) {
    MfClassicDict* instance = NULL;
    uint64_t key = 0;
    string_t temp_str;
    string_init(temp_str);

    instance = mf_classic_dict_alloc(MfClassicDictTypeUnitTest);
    mu_assert(instance != NULL, "mf_classic_dict_alloc\r\n");

    mu_assert(
        mf_classic_dict_get_total_keys(instance) == 0,
        "mf_classic_dict_get_total_keys == 0 assert failed\r\n");

    string_set(temp_str, "2196FAD8115B");
    mu_assert(
        mf_classic_dict_add_key_str(instance, temp_str),
        "mf_classic_dict_add_key == true assert failed\r\n");

    mu_assert(
        mf_classic_dict_get_total_keys(instance) == 1,
        "mf_classic_dict_get_total_keys == 1 assert failed\r\n");

    mu_assert(mf_classic_dict_rewind(instance), "mf_classic_dict_rewind == 1 assert failed\r\n");

    mu_assert(
        mf_classic_dict_get_key_at_index_str(instance, temp_str, 0),
        "mf_classic_dict_get_key_at_index_str == true assert failed\r\n");
    mu_assert(
        string_cmp(temp_str, "2196FAD8115B") == 0,
        "string_cmp(temp_str, \"2196FAD8115B\") == 0 assert failed\r\n");

    mu_assert(mf_classic_dict_rewind(instance), "mf_classic_dict_rewind == 1 assert failed\r\n");

    mu_assert(
        mf_classic_dict_get_key_at_index(instance, &key, 0),
        "mf_classic_dict_get_key_at_index == true assert failed\r\n");
    mu_assert(key == 0x2196FAD8115B, "key == 0x2196FAD8115B assert failed\r\n");

    mu_assert(mf_classic_dict_rewind(instance), "mf_classic_dict_rewind == 1 assert failed\r\n");

    mu_assert(
        mf_classic_dict_delete_index(instance, 0),
        "mf_classic_dict_delete_index == true assert failed\r\n");

    mf_classic_dict_free(instance);
    string_clear(temp_str);
}

MU_TEST_SUITE(nfc) {
    nfc_test_alloc();

    MU_RUN_TEST(nfc_digital_signal_test);
    MU_RUN_TEST(mf_classic_dict_test);

    nfc_test_free();
}

int run_minunit_test_nfc() {
    MU_RUN_SUITE(nfc);
    return MU_EXIT_CODE;
}
