#include <furi.h>
#include <flipper_format/flipper_format.h>
#include <flipper_format/flipper_format_i.h>
#include <toolbox/stream/stream.h>
#include "../minunit.h"

#define TEST_DIR TEST_DIR_NAME "/"
#define TEST_DIR_NAME EXT_PATH("unit_tests_tmp")

static const char* test_filetype = "Flipper File test";
static const uint32_t test_version = 666;

static const char* test_string_key = "String data";
static const char* test_string_data = "String";
static const char* test_string_updated_data = "New string";

static const char* test_int_key = "Int32 data";
static const int32_t test_int_data[] = {1234, -6345, 7813, 0};
static const int32_t test_int_updated_data[] = {-1337, 69};

static const char* test_uint_key = "Uint32 data";
static const uint32_t test_uint_data[] = {1234, 0, 5678, 9098, 7654321};
static const uint32_t test_uint_updated_data[] = {8, 800, 555, 35, 35};

static const char* test_float_key = "Float data";
static const float test_float_data[] = {1.5f, 1000.0f};
static const float test_float_updated_data[] = {1.2f};

static const char* test_bool_key = "Bool data";
static const bool test_bool_data[] = {true, false};
static const bool test_bool_updated_data[] = {false, true, true};

static const char* test_hex_key = "Hex data";
static const uint8_t test_hex_data[] = {0xDE, 0xAD, 0xBE};
static const uint8_t test_hex_updated_data[] = {0xFE, 0xCA};

#define READ_TEST_NIX "ff_nix.test"
static const char* test_data_nix = "Filetype: Flipper File test\n"
                                   "Version: 666\n"
                                   "# This is comment\n"
                                   "String data: String\n"
                                   "Int32 data: 1234 -6345 7813 0\n"
                                   "Uint32 data: 1234 0 5678 9098 7654321\n"
                                   "Float data: 1.5 1000.0\n"
                                   "Bool data: true false\n"
                                   "Hex data: DE AD BE";

#define READ_TEST_WIN "ff_win.test"
static const char* test_data_win = "Filetype: Flipper File test\r\n"
                                   "Version: 666\r\n"
                                   "# This is comment\r\n"
                                   "String data: String\r\n"
                                   "Int32 data: 1234 -6345 7813 0\r\n"
                                   "Uint32 data: 1234 0 5678 9098 7654321\r\n"
                                   "Float data: 1.5 1000.0\r\n"
                                   "Bool data: true false\r\n"
                                   "Hex data: DE AD BE";

#define READ_TEST_FLP "ff_flp.test"

// data created by user on linux machine
static const char* test_file_linux = TEST_DIR READ_TEST_NIX;
// data created by user on windows machine
static const char* test_file_windows = TEST_DIR READ_TEST_WIN;
// data created by flipper itself
static const char* test_file_flipper = TEST_DIR READ_TEST_FLP;

static bool storage_write_string(const char* path, const char* data) {
    Storage* storage = furi_record_open(RECORD_STORAGE);
    File* file = storage_file_alloc(storage);
    bool result = false;

    do {
        if(!storage_file_open(file, path, FSAM_WRITE, FSOM_CREATE_ALWAYS)) break;
        if(storage_file_write(file, data, strlen(data)) != strlen(data)) break;

        result = true;
    } while(false);

    storage_file_close(file);
    storage_file_free(file);
    furi_record_close(RECORD_STORAGE);

    return result;
}

static void tests_setup() {
    Storage* storage = furi_record_open(RECORD_STORAGE);
    mu_assert(storage_simply_remove_recursive(storage, TEST_DIR_NAME), "Cannot clean data");
    mu_assert(storage_simply_mkdir(storage, TEST_DIR_NAME), "Cannot create dir");
    furi_record_close(RECORD_STORAGE);
}

static void tests_teardown() {
    Storage* storage = furi_record_open(RECORD_STORAGE);
    mu_assert(storage_simply_remove_recursive(storage, TEST_DIR_NAME), "Cannot clean data");
    furi_record_close(RECORD_STORAGE);
}

static bool test_read(const char* file_name) {
    Storage* storage = furi_record_open(RECORD_STORAGE);
    bool result = false;

    FlipperFormat* file = flipper_format_file_alloc(storage);
    string_t string_value;
    string_init(string_value);
    uint32_t uint32_value;
    void* scratchpad = malloc(512);

    do {
        if(!flipper_format_file_open_existing(file, file_name)) break;

        if(!flipper_format_read_header(file, string_value, &uint32_value)) break;
        if(string_cmp_str(string_value, test_filetype) != 0) break;
        if(uint32_value != test_version) break;

        if(!flipper_format_read_string(file, test_string_key, string_value)) break;
        if(string_cmp_str(string_value, test_string_data) != 0) break;

        if(!flipper_format_get_value_count(file, test_int_key, &uint32_value)) break;
        if(uint32_value != COUNT_OF(test_int_data)) break;
        if(!flipper_format_read_int32(file, test_int_key, scratchpad, uint32_value)) break;
        if(memcmp(scratchpad, test_int_data, sizeof(int32_t) * COUNT_OF(test_int_data)) != 0)
            break;

        if(!flipper_format_get_value_count(file, test_uint_key, &uint32_value)) break;
        if(uint32_value != COUNT_OF(test_uint_data)) break;
        if(!flipper_format_read_uint32(file, test_uint_key, scratchpad, uint32_value)) break;
        if(memcmp(scratchpad, test_uint_data, sizeof(uint32_t) * COUNT_OF(test_uint_data)) != 0)
            break;

        if(!flipper_format_get_value_count(file, test_float_key, &uint32_value)) break;
        if(uint32_value != COUNT_OF(test_float_data)) break;
        if(!flipper_format_read_float(file, test_float_key, scratchpad, uint32_value)) break;
        if(memcmp(scratchpad, test_float_data, sizeof(float) * COUNT_OF(test_float_data)) != 0)
            break;

        if(!flipper_format_get_value_count(file, test_bool_key, &uint32_value)) break;
        if(uint32_value != COUNT_OF(test_bool_data)) break;
        if(!flipper_format_read_bool(file, test_bool_key, scratchpad, uint32_value)) break;
        if(memcmp(scratchpad, test_bool_data, sizeof(bool) * COUNT_OF(test_bool_data)) != 0) break;

        if(!flipper_format_get_value_count(file, test_hex_key, &uint32_value)) break;
        if(uint32_value != COUNT_OF(test_hex_data)) break;
        if(!flipper_format_read_hex(file, test_hex_key, scratchpad, uint32_value)) break;
        if(memcmp(scratchpad, test_hex_data, sizeof(uint8_t) * COUNT_OF(test_hex_data)) != 0)
            break;

        result = true;
    } while(false);

    free(scratchpad);
    string_clear(string_value);

    flipper_format_free(file);

    furi_record_close(RECORD_STORAGE);

    return result;
}

static bool test_read_updated(const char* file_name) {
    Storage* storage = furi_record_open(RECORD_STORAGE);
    bool result = false;

    FlipperFormat* file = flipper_format_file_alloc(storage);
    string_t string_value;
    string_init(string_value);
    uint32_t uint32_value;
    void* scratchpad = malloc(512);

    do {
        if(!flipper_format_file_open_existing(file, file_name)) break;

        if(!flipper_format_read_header(file, string_value, &uint32_value)) break;
        if(string_cmp_str(string_value, test_filetype) != 0) break;
        if(uint32_value != test_version) break;

        if(!flipper_format_read_string(file, test_string_key, string_value)) break;
        if(string_cmp_str(string_value, test_string_updated_data) != 0) break;

        if(!flipper_format_get_value_count(file, test_int_key, &uint32_value)) break;
        if(uint32_value != COUNT_OF(test_int_updated_data)) break;
        if(!flipper_format_read_int32(file, test_int_key, scratchpad, uint32_value)) break;
        if(memcmp(
               scratchpad,
               test_int_updated_data,
               sizeof(int32_t) * COUNT_OF(test_int_updated_data)) != 0)
            break;

        if(!flipper_format_get_value_count(file, test_uint_key, &uint32_value)) break;
        if(uint32_value != COUNT_OF(test_uint_updated_data)) break;
        if(!flipper_format_read_uint32(file, test_uint_key, scratchpad, uint32_value)) break;
        if(memcmp(
               scratchpad,
               test_uint_updated_data,
               sizeof(uint32_t) * COUNT_OF(test_uint_updated_data)) != 0)
            break;

        if(!flipper_format_get_value_count(file, test_float_key, &uint32_value)) break;
        if(uint32_value != COUNT_OF(test_float_updated_data)) break;
        if(!flipper_format_read_float(file, test_float_key, scratchpad, uint32_value)) break;
        if(memcmp(
               scratchpad,
               test_float_updated_data,
               sizeof(float) * COUNT_OF(test_float_updated_data)) != 0)
            break;

        if(!flipper_format_get_value_count(file, test_bool_key, &uint32_value)) break;
        if(uint32_value != COUNT_OF(test_bool_updated_data)) break;
        if(!flipper_format_read_bool(file, test_bool_key, scratchpad, uint32_value)) break;
        if(memcmp(
               scratchpad,
               test_bool_updated_data,
               sizeof(bool) * COUNT_OF(test_bool_updated_data)) != 0)
            break;

        if(!flipper_format_get_value_count(file, test_hex_key, &uint32_value)) break;
        if(uint32_value != COUNT_OF(test_hex_updated_data)) break;
        if(!flipper_format_read_hex(file, test_hex_key, scratchpad, uint32_value)) break;
        if(memcmp(
               scratchpad,
               test_hex_updated_data,
               sizeof(uint8_t) * COUNT_OF(test_hex_updated_data)) != 0)
            break;

        result = true;
    } while(false);

    free(scratchpad);
    string_clear(string_value);

    flipper_format_free(file);

    furi_record_close(RECORD_STORAGE);

    return result;
}

static bool test_write(const char* file_name) {
    Storage* storage = furi_record_open(RECORD_STORAGE);
    bool result = false;
    FlipperFormat* file = flipper_format_file_alloc(storage);

    do {
        if(!flipper_format_file_open_always(file, file_name)) break;
        if(!flipper_format_write_header_cstr(file, test_filetype, test_version)) break;
        if(!flipper_format_write_comment_cstr(file, "This is comment")) break;
        if(!flipper_format_write_string_cstr(file, test_string_key, test_string_data)) break;
        if(!flipper_format_write_int32(file, test_int_key, test_int_data, COUNT_OF(test_int_data)))
            break;
        if(!flipper_format_write_uint32(
               file, test_uint_key, test_uint_data, COUNT_OF(test_uint_data)))
            break;
        if(!flipper_format_write_float(
               file, test_float_key, test_float_data, COUNT_OF(test_float_data)))
            break;
        if(!flipper_format_write_bool(
               file, test_bool_key, test_bool_data, COUNT_OF(test_bool_data)))
            break;
        if(!flipper_format_write_hex(file, test_hex_key, test_hex_data, COUNT_OF(test_hex_data)))
            break;
        result = true;
    } while(false);

    flipper_format_free(file);
    furi_record_close(RECORD_STORAGE);

    return result;
}

static bool test_delete_last_key(const char* file_name) {
    Storage* storage = furi_record_open(RECORD_STORAGE);
    bool result = false;
    FlipperFormat* file = flipper_format_file_alloc(storage);

    do {
        if(!flipper_format_file_open_existing(file, file_name)) break;
        if(!flipper_format_delete_key(file, test_hex_key)) break;
        result = true;
    } while(false);

    flipper_format_free(file);
    furi_record_close(RECORD_STORAGE);

    return result;
}

static bool test_append_key(const char* file_name) {
    Storage* storage = furi_record_open(RECORD_STORAGE);
    bool result = false;
    FlipperFormat* file = flipper_format_file_alloc(storage);

    do {
        if(!flipper_format_file_open_append(file, file_name)) break;
        if(!flipper_format_write_hex(file, test_hex_key, test_hex_data, COUNT_OF(test_hex_data)))
            break;
        result = true;
    } while(false);

    flipper_format_free(file);
    furi_record_close(RECORD_STORAGE);

    return result;
}

static bool test_update(const char* file_name) {
    Storage* storage = furi_record_open(RECORD_STORAGE);
    bool result = false;
    FlipperFormat* file = flipper_format_file_alloc(storage);

    do {
        if(!flipper_format_file_open_existing(file, file_name)) break;
        if(!flipper_format_update_string_cstr(file, test_string_key, test_string_updated_data))
            break;
        if(!flipper_format_update_int32(
               file, test_int_key, test_int_updated_data, COUNT_OF(test_int_updated_data)))
            break;
        if(!flipper_format_update_uint32(
               file, test_uint_key, test_uint_updated_data, COUNT_OF(test_uint_updated_data)))
            break;
        if(!flipper_format_update_float(
               file, test_float_key, test_float_updated_data, COUNT_OF(test_float_updated_data)))
            break;
        if(!flipper_format_update_bool(
               file, test_bool_key, test_bool_updated_data, COUNT_OF(test_bool_updated_data)))
            break;
        if(!flipper_format_update_hex(
               file, test_hex_key, test_hex_updated_data, COUNT_OF(test_hex_updated_data)))
            break;

        result = true;
    } while(false);

    flipper_format_free(file);
    furi_record_close(RECORD_STORAGE);

    return result;
}

static bool test_update_backward(const char* file_name) {
    Storage* storage = furi_record_open(RECORD_STORAGE);
    bool result = false;
    FlipperFormat* file = flipper_format_file_alloc(storage);

    do {
        if(!flipper_format_file_open_existing(file, file_name)) break;
        if(!flipper_format_update_string_cstr(file, test_string_key, test_string_data)) break;
        if(!flipper_format_update_int32(file, test_int_key, test_int_data, COUNT_OF(test_int_data)))
            break;
        if(!flipper_format_update_uint32(
               file, test_uint_key, test_uint_data, COUNT_OF(test_uint_data)))
            break;
        if(!flipper_format_update_float(
               file, test_float_key, test_float_data, COUNT_OF(test_float_data)))
            break;
        if(!flipper_format_update_bool(
               file, test_bool_key, test_bool_data, COUNT_OF(test_bool_data)))
            break;
        if(!flipper_format_update_hex(file, test_hex_key, test_hex_data, COUNT_OF(test_hex_data)))
            break;

        result = true;
    } while(false);

    flipper_format_free(file);
    furi_record_close(RECORD_STORAGE);

    return result;
}

static bool test_write_multikey(const char* file_name) {
    Storage* storage = furi_record_open(RECORD_STORAGE);
    bool result = false;
    FlipperFormat* file = flipper_format_file_alloc(storage);

    do {
        if(!flipper_format_file_open_always(file, file_name)) break;
        if(!flipper_format_write_header_cstr(file, test_filetype, test_version)) break;

        bool error = false;
        for(uint8_t index = 0; index < 100; index++) {
            if(!flipper_format_write_hex(file, test_hex_key, &index, 1)) {
                error = true;
                break;
            }
        }
        if(error) break;

        result = true;
    } while(false);

    flipper_format_free(file);
    furi_record_close(RECORD_STORAGE);

    return result;
}

static bool test_read_multikey(const char* file_name) {
    Storage* storage = furi_record_open(RECORD_STORAGE);
    bool result = false;
    FlipperFormat* file = flipper_format_file_alloc(storage);

    string_t string_value;
    string_init(string_value);
    uint32_t uint32_value;

    do {
        if(!flipper_format_file_open_existing(file, file_name)) break;
        if(!flipper_format_read_header(file, string_value, &uint32_value)) break;
        if(string_cmp_str(string_value, test_filetype) != 0) break;
        if(uint32_value != test_version) break;

        bool error = false;
        uint8_t uint8_value;
        for(uint8_t index = 0; index < 100; index++) {
            if(!flipper_format_read_hex(file, test_hex_key, &uint8_value, 1)) {
                error = true;
                break;
            }

            if(uint8_value != index) {
                error = true;
                break;
            }
        }
        if(error) break;

        result = true;
    } while(false);

    string_clear(string_value);

    flipper_format_free(file);
    furi_record_close(RECORD_STORAGE);

    return result;
}

MU_TEST(flipper_format_write_test) {
    mu_assert(storage_write_string(test_file_linux, test_data_nix), "Write test error [Linux]");
    mu_assert(
        storage_write_string(test_file_windows, test_data_win), "Write test error [Windows]");
    mu_assert(test_write(test_file_flipper), "Write test error [Flipper]");
}

MU_TEST(flipper_format_read_test) {
    mu_assert(test_read(test_file_linux), "Read test error [Linux]");
    mu_assert(test_read(test_file_windows), "Read test error [Windows]");
    mu_assert(test_read(test_file_flipper), "Read test error [Flipper]");
}

MU_TEST(flipper_format_delete_test) {
    mu_assert(test_delete_last_key(test_file_linux), "Cannot delete key [Linux]");
    mu_assert(test_delete_last_key(test_file_windows), "Cannot delete key [Windows]");
    mu_assert(test_delete_last_key(test_file_flipper), "Cannot delete key [Flipper]");
}

MU_TEST(flipper_format_delete_result_test) {
    mu_assert(!test_read(test_file_linux), "Key deleted incorrectly [Linux]");
    mu_assert(!test_read(test_file_windows), "Key deleted incorrectly [Windows]");
    mu_assert(!test_read(test_file_flipper), "Key deleted incorrectly [Flipper]");
}

MU_TEST(flipper_format_append_test) {
    mu_assert(test_append_key(test_file_linux), "Cannot append data [Linux]");
    mu_assert(test_append_key(test_file_windows), "Cannot append data [Windows]");
    mu_assert(test_append_key(test_file_flipper), "Cannot append data [Flipper]");
}

MU_TEST(flipper_format_append_result_test) {
    mu_assert(test_read(test_file_linux), "Data appended incorrectly [Linux]");
    mu_assert(test_read(test_file_windows), "Data appended incorrectly [Windows]");
    mu_assert(test_read(test_file_flipper), "Data appended incorrectly [Flipper]");
}

MU_TEST(flipper_format_update_1_test) {
    mu_assert(test_update(test_file_linux), "Cannot update data #1 [Linux]");
    mu_assert(test_update(test_file_windows), "Cannot update data #1 [Windows]");
    mu_assert(test_update(test_file_flipper), "Cannot update data #1 [Flipper]");
}

MU_TEST(flipper_format_update_1_result_test) {
    mu_assert(test_read_updated(test_file_linux), "Data #1 updated incorrectly [Linux]");
    mu_assert(test_read_updated(test_file_windows), "Data #1 updated incorrectly [Windows]");
    mu_assert(test_read_updated(test_file_flipper), "Data #1 updated incorrectly [Flipper]");
}

MU_TEST(flipper_format_update_2_test) {
    mu_assert(test_update_backward(test_file_linux), "Cannot update data #2 [Linux]");
    mu_assert(test_update_backward(test_file_windows), "Cannot update data #2 [Windows]");
    mu_assert(test_update_backward(test_file_flipper), "Cannot update data #2 [Flipper]");
}

MU_TEST(flipper_format_update_2_result_test) {
    mu_assert(test_read(test_file_linux), "Data #2 updated incorrectly [Linux]");
    mu_assert(test_read(test_file_windows), "Data #2 updated incorrectly [Windows]");
    mu_assert(test_read(test_file_flipper), "Data #2 updated incorrectly [Flipper]");
}

MU_TEST(flipper_format_multikey_test) {
    mu_assert(test_write_multikey(TEST_DIR "ff_multiline.test"), "Multikey write test error");
    mu_assert(test_read_multikey(TEST_DIR "ff_multiline.test"), "Multikey read test error");
}

MU_TEST_SUITE(flipper_format) {
    tests_setup();
    MU_RUN_TEST(flipper_format_write_test);
    MU_RUN_TEST(flipper_format_read_test);
    MU_RUN_TEST(flipper_format_delete_test);
    MU_RUN_TEST(flipper_format_delete_result_test);
    MU_RUN_TEST(flipper_format_append_test);
    MU_RUN_TEST(flipper_format_append_result_test);
    MU_RUN_TEST(flipper_format_update_1_test);
    MU_RUN_TEST(flipper_format_update_1_result_test);
    MU_RUN_TEST(flipper_format_update_2_test);
    MU_RUN_TEST(flipper_format_update_2_result_test);
    MU_RUN_TEST(flipper_format_multikey_test);
    tests_teardown();
}

int run_minunit_test_flipper_format() {
    MU_RUN_SUITE(flipper_format);
    return MU_EXIT_CODE;
}
