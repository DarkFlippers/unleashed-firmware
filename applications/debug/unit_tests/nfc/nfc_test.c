#include <furi.h>
#include <furi_hal.h>
#include <storage/storage.h>
#include <lib/flipper_format/flipper_format.h>
#include <lib/nfc/protocols/nfca.h>
#include <lib/nfc/helpers/mf_classic_dict.h>
#include <lib/digital_signal/digital_signal.h>
#include <lib/nfc/nfc_device.h>
#include <lib/nfc/helpers/nfc_generators.h>

#include <lib/flipper_format/flipper_format_i.h>
#include <lib/toolbox/stream/file_stream.h>

#include "../minunit.h"

#define TAG "NfcTest"

#define NFC_TEST_RESOURCES_DIR EXT_PATH("unit_tests/nfc/")
#define NFC_TEST_SIGNAL_SHORT_FILE "nfc_nfca_signal_short.nfc"
#define NFC_TEST_SIGNAL_LONG_FILE "nfc_nfca_signal_long.nfc"
#define NFC_TEST_DICT_PATH EXT_PATH("unit_tests/mf_classic_dict.nfc")
#define NFC_TEST_NFC_DEV_PATH EXT_PATH("unit_tests/nfc/nfc_dev_test.nfc")

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
    FuriString* file_type;
    file_type = furi_string_alloc();
    uint32_t file_version = 0;

    do {
        if(!flipper_format_file_open_existing(file, file_name)) break;
        if(!flipper_format_read_header(file, file_type, &file_version)) break;
        if(furi_string_cmp_str(file_type, nfc_test_file_type) ||
           file_version != nfc_test_file_version)
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

    furi_string_free(file_type);
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
        if(!nfc_test_read_signal_from_file(file_name)) {
            FURI_LOG_E(TAG, "Failed to read signal from file");
            break;
        }

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
                TAG, "Encoding time: %ld us while accepted value: %ld us", time, encode_max_time);
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
                    TAG, "Too big difference in %d timings. Ref: %ld, DUT: %ld", i, ref[i], dut[i]);
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
                "Too big difference in timings sum. Ref: %ld, DUT: %ld",
                ref_timings_sum,
                dut_timings_sum);
            break;
        }

        FURI_LOG_I(TAG, "Encoding time: %ld us. Acceptable time: %ld us", time, encode_max_time);
        FURI_LOG_I(
            TAG,
            "Timings sum difference: %ld [1/64MHZ]. Acceptable difference: %ld [1/64MHz]",
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
    FuriString* temp_str;
    temp_str = furi_string_alloc();

    instance = mf_classic_dict_alloc(MfClassicDictTypeUnitTest);
    mu_assert(instance != NULL, "mf_classic_dict_alloc\r\n");

    mu_assert(
        mf_classic_dict_get_total_keys(instance) == 0,
        "mf_classic_dict_get_total_keys == 0 assert failed\r\n");

    furi_string_set(temp_str, "2196FAD8115B");
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
        furi_string_cmp(temp_str, "2196FAD8115B") == 0,
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
    furi_string_free(temp_str);
}

MU_TEST(mf_classic_dict_load_test) {
    Storage* storage = furi_record_open(RECORD_STORAGE);
    mu_assert(storage != NULL, "storage != NULL assert failed\r\n");

    // Delete unit test dict file if exists
    if(storage_file_exists(storage, NFC_TEST_DICT_PATH)) {
        mu_assert(
            storage_simply_remove(storage, NFC_TEST_DICT_PATH),
            "remove == true assert failed\r\n");
    }

    // Create unit test dict file
    Stream* file_stream = file_stream_alloc(storage);
    mu_assert(file_stream != NULL, "file_stream != NULL assert failed\r\n");
    mu_assert(
        file_stream_open(file_stream, NFC_TEST_DICT_PATH, FSAM_WRITE, FSOM_OPEN_ALWAYS),
        "file_stream_open == true assert failed\r\n");

    // Write unit test dict file
    char key_str[] = "a0a1a2a3a4a5";
    mu_assert(
        stream_write_cstring(file_stream, key_str) == strlen(key_str),
        "write == true assert failed\r\n");
    // Close unit test dict file
    mu_assert(file_stream_close(file_stream), "file_stream_close == true assert failed\r\n");

    // Load unit test dict file
    MfClassicDict* instance = NULL;
    instance = mf_classic_dict_alloc(MfClassicDictTypeUnitTest);
    mu_assert(instance != NULL, "mf_classic_dict_alloc\r\n");
    uint32_t total_keys = mf_classic_dict_get_total_keys(instance);
    mu_assert(total_keys == 1, "total_keys == 1 assert failed\r\n");

    // Read key
    uint64_t key_ref = 0xa0a1a2a3a4a5;
    uint64_t key_dut = 0;
    FuriString* temp_str = furi_string_alloc();
    mu_assert(
        mf_classic_dict_get_next_key_str(instance, temp_str),
        "get_next_key_str == true assert failed\r\n");
    mu_assert(furi_string_cmp_str(temp_str, key_str) == 0, "invalid key loaded\r\n");
    mu_assert(mf_classic_dict_rewind(instance), "mf_classic_dict_rewind == 1 assert failed\r\n");
    mu_assert(
        mf_classic_dict_get_next_key(instance, &key_dut),
        "get_next_key == true assert failed\r\n");
    mu_assert(key_dut == key_ref, "invalid key loaded\r\n");
    furi_string_free(temp_str);
    mf_classic_dict_free(instance);

    // Check that MfClassicDict added new line to the end of the file
    mu_assert(
        file_stream_open(file_stream, NFC_TEST_DICT_PATH, FSAM_READ, FSOM_OPEN_EXISTING),
        "file_stream_open == true assert failed\r\n");
    mu_assert(stream_seek(file_stream, -1, StreamOffsetFromEnd), "seek == true assert failed\r\n");
    uint8_t last_char = 0;
    mu_assert(stream_read(file_stream, &last_char, 1) == 1, "read == true assert failed\r\n");
    mu_assert(last_char == '\n', "last_char == '\\n' assert failed\r\n");
    mu_assert(file_stream_close(file_stream), "file_stream_close == true assert failed\r\n");

    // Delete unit test dict file
    mu_assert(
        storage_simply_remove(storage, NFC_TEST_DICT_PATH), "remove == true assert failed\r\n");
    stream_free(file_stream);
    furi_record_close(RECORD_STORAGE);
}

MU_TEST(nfca_file_test) {
    NfcDevice* nfc = nfc_device_alloc();
    mu_assert(nfc != NULL, "nfc_device_data != NULL assert failed\r\n");
    nfc->format = NfcDeviceSaveFormatUid;

    // Fill the UID, sak, ATQA and type
    uint8_t uid[7] = {0x04, 0x01, 0x23, 0x45, 0x67, 0x89, 0x00};
    memcpy(nfc->dev_data.nfc_data.uid, uid, 7);
    nfc->dev_data.nfc_data.uid_len = 7;

    nfc->dev_data.nfc_data.sak = 0x08;
    nfc->dev_data.nfc_data.atqa[0] = 0x00;
    nfc->dev_data.nfc_data.atqa[1] = 0x04;
    nfc->dev_data.nfc_data.type = FuriHalNfcTypeA;

    // Save the NFC device data to the file
    mu_assert(
        nfc_device_save(nfc, NFC_TEST_NFC_DEV_PATH), "nfc_device_save == true assert failed\r\n");
    nfc_device_free(nfc);

    // Load the NFC device data from the file
    NfcDevice* nfc_validate = nfc_device_alloc();
    mu_assert(
        nfc_device_load(nfc_validate, NFC_TEST_NFC_DEV_PATH, true),
        "nfc_device_load == true assert failed\r\n");

    // Check the UID, sak, ATQA and type
    mu_assert(memcmp(nfc_validate->dev_data.nfc_data.uid, uid, 7) == 0, "uid assert failed\r\n");
    mu_assert(nfc_validate->dev_data.nfc_data.sak == 0x08, "sak == 0x08 assert failed\r\n");
    mu_assert(
        nfc_validate->dev_data.nfc_data.atqa[0] == 0x00, "atqa[0] == 0x00 assert failed\r\n");
    mu_assert(
        nfc_validate->dev_data.nfc_data.atqa[1] == 0x04, "atqa[1] == 0x04 assert failed\r\n");
    mu_assert(
        nfc_validate->dev_data.nfc_data.type == FuriHalNfcTypeA,
        "type == FuriHalNfcTypeA assert failed\r\n");
    nfc_device_free(nfc_validate);
}

static void mf_classic_generator_test(uint8_t uid_len, MfClassicType type) {
    NfcDevice* nfc_dev = nfc_device_alloc();
    mu_assert(nfc_dev != NULL, "nfc_device_data != NULL assert failed\r\n");
    nfc_dev->format = NfcDeviceSaveFormatMifareClassic;

    // Create a test file
    nfc_generate_mf_classic(&nfc_dev->dev_data, uid_len, type);

    // Get the uid from generated MFC
    uint8_t uid[7] = {0};
    memcpy(uid, nfc_dev->dev_data.nfc_data.uid, uid_len);
    uint8_t sak = nfc_dev->dev_data.nfc_data.sak;
    uint8_t atqa[2] = {};
    memcpy(atqa, nfc_dev->dev_data.nfc_data.atqa, 2);

    MfClassicData* mf_data = &nfc_dev->dev_data.mf_classic_data;
    // Check the manufacturer block (should be uid[uid_len] + 0xFF[rest])
    uint8_t manufacturer_block[16] = {0};
    memcpy(manufacturer_block, nfc_dev->dev_data.mf_classic_data.block[0].value, 16);
    mu_assert(
        memcmp(manufacturer_block, uid, uid_len) == 0,
        "manufacturer_block uid doesn't match the file\r\n");
    for(uint8_t i = uid_len; i < 16; i++) {
        mu_assert(
            manufacturer_block[i] == 0xFF, "manufacturer_block[i] == 0xFF assert failed\r\n");
    }

    // Reference sector trailers (should be 0xFF[6] + 0xFF + 0x07 + 0x80 + 0x69 + 0xFF[6])
    uint8_t sector_trailer[16] = {
        0xFF,
        0xFF,
        0xFF,
        0xFF,
        0xFF,
        0xFF,
        0xFF,
        0x07,
        0x80,
        0x69,
        0xFF,
        0xFF,
        0xFF,
        0xFF,
        0xFF,
        0xFF};
    // Reference block data
    uint8_t block_data[16] = {};
    memset(block_data, 0xff, sizeof(block_data));
    uint16_t total_blocks = mf_classic_get_total_block_num(type);
    for(size_t i = 1; i < total_blocks; i++) {
        if(mf_classic_is_sector_trailer(i)) {
            mu_assert(
                memcmp(mf_data->block[i].value, sector_trailer, 16) == 0,
                "Failed sector trailer compare");
        } else {
            mu_assert(memcmp(mf_data->block[i].value, block_data, 16) == 0, "Failed data compare");
        }
    }
    // Save the NFC device data to the file
    mu_assert(
        nfc_device_save(nfc_dev, NFC_TEST_NFC_DEV_PATH),
        "nfc_device_save == true assert failed\r\n");
    // Verify that key cache is saved
    FuriString* key_cache_name = furi_string_alloc();
    furi_string_set_str(key_cache_name, "/ext/nfc/.cache/");
    for(size_t i = 0; i < uid_len; i++) {
        furi_string_cat_printf(key_cache_name, "%02X", uid[i]);
    }
    furi_string_cat_printf(key_cache_name, ".keys");
    mu_assert(
        storage_common_stat(nfc_dev->storage, furi_string_get_cstr(key_cache_name), NULL) ==
            FSE_OK,
        "Key cache file save failed");
    nfc_device_free(nfc_dev);

    // Load the NFC device data from the file
    NfcDevice* nfc_validate = nfc_device_alloc();
    mu_assert(nfc_validate, "Nfc device alloc assert");
    mu_assert(
        nfc_device_load(nfc_validate, NFC_TEST_NFC_DEV_PATH, false),
        "nfc_device_load == true assert failed\r\n");

    // Check the UID, sak, ATQA and type
    mu_assert(
        memcmp(nfc_validate->dev_data.nfc_data.uid, uid, uid_len) == 0,
        "uid compare assert failed\r\n");
    mu_assert(nfc_validate->dev_data.nfc_data.sak == sak, "sak compare assert failed\r\n");
    mu_assert(
        memcmp(nfc_validate->dev_data.nfc_data.atqa, atqa, 2) == 0,
        "atqa compare assert failed\r\n");
    mu_assert(
        nfc_validate->dev_data.nfc_data.type == FuriHalNfcTypeA,
        "type == FuriHalNfcTypeA assert failed\r\n");

    // Check the manufacturer block
    mu_assert(
        memcmp(nfc_validate->dev_data.mf_classic_data.block[0].value, manufacturer_block, 16) == 0,
        "manufacturer_block assert failed\r\n");
    // Check other blocks
    for(size_t i = 1; i < total_blocks; i++) {
        if(mf_classic_is_sector_trailer(i)) {
            mu_assert(
                memcmp(mf_data->block[i].value, sector_trailer, 16) == 0,
                "Failed sector trailer compare");
        } else {
            mu_assert(memcmp(mf_data->block[i].value, block_data, 16) == 0, "Failed data compare");
        }
    }
    nfc_device_free(nfc_validate);

    // Check saved key cache
    NfcDevice* nfc_keys = nfc_device_alloc();
    mu_assert(nfc_validate, "Nfc device alloc assert");
    nfc_keys->dev_data.nfc_data.uid_len = uid_len;
    memcpy(nfc_keys->dev_data.nfc_data.uid, uid, uid_len);
    mu_assert(nfc_device_load_key_cache(nfc_keys), "Failed to load key cache");
    uint8_t total_sec = mf_classic_get_total_sectors_num(type);
    uint8_t default_key[6] = {};
    memset(default_key, 0xff, 6);
    for(size_t i = 0; i < total_sec; i++) {
        MfClassicSectorTrailer* sec_tr =
            mf_classic_get_sector_trailer_by_sector(&nfc_keys->dev_data.mf_classic_data, i);
        mu_assert(memcmp(sec_tr->key_a, default_key, 6) == 0, "Failed key compare");
        mu_assert(memcmp(sec_tr->key_b, default_key, 6) == 0, "Failed key compare");
    }

    // Delete key cache file
    mu_assert(
        storage_common_remove(nfc_keys->storage, furi_string_get_cstr(key_cache_name)) == FSE_OK,
        "Failed to remove key cache file");
    furi_string_free(key_cache_name);
    nfc_device_free(nfc_keys);
}

MU_TEST(mf_mini_file_test) {
    mf_classic_generator_test(4, MfClassicTypeMini);
}

MU_TEST(mf_classic_1k_4b_file_test) {
    mf_classic_generator_test(4, MfClassicType1k);
}

MU_TEST(mf_classic_4k_4b_file_test) {
    mf_classic_generator_test(4, MfClassicType4k);
}

MU_TEST(mf_classic_1k_7b_file_test) {
    mf_classic_generator_test(7, MfClassicType1k);
}

MU_TEST(mf_classic_4k_7b_file_test) {
    mf_classic_generator_test(7, MfClassicType4k);
}

MU_TEST_SUITE(nfc) {
    nfc_test_alloc();

    MU_RUN_TEST(nfca_file_test);
    MU_RUN_TEST(mf_mini_file_test);
    MU_RUN_TEST(mf_classic_1k_4b_file_test);
    MU_RUN_TEST(mf_classic_4k_4b_file_test);
    MU_RUN_TEST(mf_classic_1k_7b_file_test);
    MU_RUN_TEST(mf_classic_4k_7b_file_test);
    MU_RUN_TEST(nfc_digital_signal_test);
    MU_RUN_TEST(mf_classic_dict_test);
    MU_RUN_TEST(mf_classic_dict_load_test);

    nfc_test_free();
}

int run_minunit_test_nfc() {
    MU_RUN_SUITE(nfc);
    return MU_EXIT_CODE;
}
