#include <furi.h>
#include <flipper_format/flipper_format.h>
#include <flipper_format/flipper_format_i.h>
#include <toolbox/stream/stream.h>
#include <storage/storage.h>
#include "../minunit.h"

static const char* test_filetype = "Flipper Format test";
static const uint32_t test_version = 666;

static const char* test_string_key = "String data";
static const char* test_string_data = "String";
static const char* test_string_updated_data = "New string";
static const char* test_string_updated_2_data = "And some more";

static const char* test_int_key = "Int32 data";
static const int32_t test_int_data[] = {1234, -6345, 7813, 0};
static const int32_t test_int_updated_data[] = {-1337, 69};
static const int32_t test_int_updated_2_data[] = {-3, -2, -1, 0, 1, 2, 3};

static const char* test_uint_key = "Uint32 data";
static const uint32_t test_uint_data[] = {1234, 0, 5678, 9098, 7654321};
static const uint32_t test_uint_updated_data[] = {8, 800, 555, 35, 35};
static const uint32_t test_uint_updated_2_data[] = {20, 21};

static const char* test_float_key = "Float data";
static const float test_float_data[] = {1.5f, 1000.0f};
static const float test_float_updated_data[] = {1.2f};
static const float test_float_updated_2_data[] = {0.01f, 0.0f, -51.6f};

static const char* test_hex_key = "Hex data";
static const uint8_t test_hex_data[] = {0xDE, 0xAD, 0xBE};
static const uint8_t test_hex_updated_data[] = {0xFE, 0xCA};
static const uint8_t test_hex_updated_2_data[] = {0xCA, 0xCA, 0x05};

static const char* test_hex_new_key = "New Hex data";
static const uint8_t test_hex_new_data[] = {0xFF, 0x6A, 0x91};

static const char* test_data_nix = "Filetype: Flipper Format test\n"
                                   "Version: 666\n"
                                   "# This is comment\n"
                                   "String data: String\n"
                                   "Int32 data: 1234 -6345 7813 0\n"
                                   "Uint32 data: 1234 0 5678 9098 7654321\n"
                                   "Float data: 1.5 1000.0\n"
                                   "Hex data: DE AD BE";

static const char* test_data_win = "Filetype: Flipper Format test\r\n"
                                   "Version: 666\r\n"
                                   "# This is comment\r\n"
                                   "String data: String\r\n"
                                   "Int32 data: 1234 -6345 7813 0\r\n"
                                   "Uint32 data: 1234 0 5678 9098 7654321\r\n"
                                   "Float data: 1.5 1000.0\r\n"
                                   "Hex data: DE AD BE";

#define ARRAY_W_COUNT(x) (x), (COUNT_OF(x))
#define ARRAY_W_BSIZE(x) (x), (sizeof(x))

MU_TEST_1(flipper_format_read_and_update_test, FlipperFormat* flipper_format) {
    string_t tmpstr;
    uint32_t version;
    uint32_t uint32_data[COUNT_OF(test_uint_data)];
    int32_t int32_data[COUNT_OF(test_int_data)];
    float float_data[COUNT_OF(test_float_data)];
    uint8_t hex_data[COUNT_OF(test_hex_data)];

    uint32_t count;

    // key exist test
    size_t position_before = stream_tell(flipper_format_get_raw_stream(flipper_format));
    mu_check(flipper_format_key_exist(flipper_format, test_hex_key));
    mu_assert_int_eq(position_before, stream_tell(flipper_format_get_raw_stream(flipper_format)));

    mu_check(!flipper_format_key_exist(flipper_format, "invalid key"));
    mu_assert_int_eq(position_before, stream_tell(flipper_format_get_raw_stream(flipper_format)));

    // stream seek to end test
    mu_check(flipper_format_seek_to_end(flipper_format));
    mu_assert_int_eq(
        stream_size(flipper_format_get_raw_stream(flipper_format)),
        stream_tell(flipper_format_get_raw_stream(flipper_format)));

    // key exist test
    position_before = stream_tell(flipper_format_get_raw_stream(flipper_format));
    mu_check(flipper_format_key_exist(flipper_format, test_hex_key));
    mu_assert_int_eq(position_before, stream_tell(flipper_format_get_raw_stream(flipper_format)));

    mu_check(!flipper_format_key_exist(flipper_format, "invalid key"));
    mu_assert_int_eq(position_before, stream_tell(flipper_format_get_raw_stream(flipper_format)));

    // rewind
    mu_check(flipper_format_rewind(flipper_format));

    // key exist test
    position_before = stream_tell(flipper_format_get_raw_stream(flipper_format));
    mu_check(flipper_format_key_exist(flipper_format, test_hex_key));
    mu_assert_int_eq(position_before, stream_tell(flipper_format_get_raw_stream(flipper_format)));

    mu_check(!flipper_format_key_exist(flipper_format, "invalid key"));
    mu_assert_int_eq(position_before, stream_tell(flipper_format_get_raw_stream(flipper_format)));

    // read test
    string_init(tmpstr);

    mu_check(flipper_format_read_header(flipper_format, tmpstr, &version));
    mu_assert_string_eq(test_filetype, string_get_cstr(tmpstr));
    mu_assert_int_eq(test_version, version);

    mu_check(flipper_format_read_string(flipper_format, test_string_key, tmpstr));
    mu_assert_string_eq(test_string_data, string_get_cstr(tmpstr));

    mu_check(flipper_format_get_value_count(flipper_format, test_int_key, &count));
    mu_assert_int_eq(COUNT_OF(test_int_data), count);
    mu_check(flipper_format_read_int32(flipper_format, test_int_key, ARRAY_W_COUNT(int32_data)));
    mu_check(memcmp(test_int_data, ARRAY_W_BSIZE(int32_data)) == 0);

    mu_check(flipper_format_get_value_count(flipper_format, test_uint_key, &count));
    mu_assert_int_eq(COUNT_OF(test_uint_data), count);
    mu_check(
        flipper_format_read_uint32(flipper_format, test_uint_key, ARRAY_W_COUNT(uint32_data)));
    mu_check(memcmp(test_uint_data, ARRAY_W_BSIZE(uint32_data)) == 0);

    mu_check(flipper_format_get_value_count(flipper_format, test_float_key, &count));
    mu_assert_int_eq(COUNT_OF(test_float_data), count);
    mu_check(flipper_format_read_float(flipper_format, test_float_key, ARRAY_W_COUNT(float_data)));
    mu_check(memcmp(test_float_data, ARRAY_W_BSIZE(float_data)) == 0);

    mu_check(flipper_format_get_value_count(flipper_format, test_hex_key, &count));
    mu_assert_int_eq(COUNT_OF(test_hex_data), count);
    mu_check(flipper_format_read_hex(flipper_format, test_hex_key, ARRAY_W_COUNT(hex_data)));
    mu_check(memcmp(test_hex_data, ARRAY_W_BSIZE(hex_data)) == 0);

    mu_check(!flipper_format_read_string(flipper_format, "Key that doesn't exist", tmpstr));

    string_clear(tmpstr);

    // update data
    mu_check(flipper_format_rewind(flipper_format));
    mu_check(flipper_format_update_string_cstr(
        flipper_format, test_string_key, test_string_updated_data));
    mu_check(flipper_format_update_int32(
        flipper_format, test_int_key, ARRAY_W_COUNT(test_int_updated_data)));
    mu_check(flipper_format_update_uint32(
        flipper_format, test_uint_key, ARRAY_W_COUNT(test_uint_updated_data)));
    mu_check(flipper_format_update_float(
        flipper_format, test_float_key, ARRAY_W_COUNT(test_float_updated_data)));
    mu_check(flipper_format_update_hex(
        flipper_format, test_hex_key, ARRAY_W_COUNT(test_hex_updated_data)));

    // read updated data test
    uint32_t uint32_updated_data[COUNT_OF(test_uint_updated_data)];
    int32_t int32_updated_data[COUNT_OF(test_int_updated_data)];
    float float_updated_data[COUNT_OF(test_float_updated_data)];
    uint8_t hex_updated_data[COUNT_OF(test_hex_updated_data)];

    mu_check(flipper_format_rewind(flipper_format));
    string_init(tmpstr);

    mu_check(flipper_format_read_header(flipper_format, tmpstr, &version));
    mu_assert_string_eq(test_filetype, string_get_cstr(tmpstr));
    mu_assert_int_eq(test_version, version);

    mu_check(flipper_format_read_string(flipper_format, test_string_key, tmpstr));
    mu_assert_string_eq(test_string_updated_data, string_get_cstr(tmpstr));

    mu_check(flipper_format_get_value_count(flipper_format, test_int_key, &count));
    mu_assert_int_eq(COUNT_OF(test_int_updated_data), count);
    mu_check(flipper_format_read_int32(
        flipper_format, test_int_key, ARRAY_W_COUNT(int32_updated_data)));
    mu_check(memcmp(test_int_updated_data, ARRAY_W_BSIZE(int32_updated_data)) == 0);

    mu_check(flipper_format_get_value_count(flipper_format, test_uint_key, &count));
    mu_assert_int_eq(COUNT_OF(test_uint_updated_data), count);
    mu_check(flipper_format_read_uint32(
        flipper_format, test_uint_key, ARRAY_W_COUNT(uint32_updated_data)));
    mu_check(memcmp(test_uint_updated_data, ARRAY_W_BSIZE(uint32_updated_data)) == 0);

    mu_check(flipper_format_get_value_count(flipper_format, test_float_key, &count));
    mu_assert_int_eq(COUNT_OF(test_float_updated_data), count);
    mu_check(flipper_format_read_float(
        flipper_format, test_float_key, ARRAY_W_COUNT(float_updated_data)));
    mu_check(memcmp(test_float_updated_data, ARRAY_W_BSIZE(float_updated_data)) == 0);

    mu_check(flipper_format_get_value_count(flipper_format, test_hex_key, &count));
    mu_assert_int_eq(COUNT_OF(test_hex_updated_data), count);
    mu_check(
        flipper_format_read_hex(flipper_format, test_hex_key, ARRAY_W_COUNT(hex_updated_data)));
    mu_check(memcmp(test_hex_updated_data, ARRAY_W_BSIZE(hex_updated_data)) == 0);

    mu_check(!flipper_format_read_string(flipper_format, "Key that doesn't exist", tmpstr));

    string_clear(tmpstr);

    // update data
    mu_check(flipper_format_rewind(flipper_format));
    mu_check(flipper_format_insert_or_update_string_cstr(
        flipper_format, test_string_key, test_string_updated_2_data));
    mu_check(flipper_format_insert_or_update_int32(
        flipper_format, test_int_key, ARRAY_W_COUNT(test_int_updated_2_data)));
    mu_check(flipper_format_insert_or_update_uint32(
        flipper_format, test_uint_key, ARRAY_W_COUNT(test_uint_updated_2_data)));
    mu_check(flipper_format_insert_or_update_float(
        flipper_format, test_float_key, ARRAY_W_COUNT(test_float_updated_2_data)));
    mu_check(flipper_format_insert_or_update_hex(
        flipper_format, test_hex_key, ARRAY_W_COUNT(test_hex_updated_2_data)));
    mu_check(flipper_format_insert_or_update_hex(
        flipper_format, test_hex_new_key, ARRAY_W_COUNT(test_hex_new_data)));

    uint32_t uint32_updated_2_data[COUNT_OF(test_uint_updated_2_data)];
    int32_t int32_updated_2_data[COUNT_OF(test_int_updated_2_data)];
    float float_updated_2_data[COUNT_OF(test_float_updated_2_data)];
    uint8_t hex_updated_2_data[COUNT_OF(test_hex_updated_2_data)];
    uint8_t hex_new_data[COUNT_OF(test_hex_new_data)];

    mu_check(flipper_format_rewind(flipper_format));
    string_init(tmpstr);

    mu_check(flipper_format_read_header(flipper_format, tmpstr, &version));
    mu_assert_string_eq(test_filetype, string_get_cstr(tmpstr));
    mu_assert_int_eq(test_version, version);

    mu_check(flipper_format_read_string(flipper_format, test_string_key, tmpstr));
    mu_assert_string_eq(test_string_updated_2_data, string_get_cstr(tmpstr));

    mu_check(flipper_format_get_value_count(flipper_format, test_int_key, &count));
    mu_assert_int_eq(COUNT_OF(test_int_updated_2_data), count);
    mu_check(flipper_format_read_int32(
        flipper_format, test_int_key, ARRAY_W_COUNT(int32_updated_2_data)));
    mu_check(memcmp(test_int_updated_2_data, ARRAY_W_BSIZE(int32_updated_2_data)) == 0);

    mu_check(flipper_format_get_value_count(flipper_format, test_uint_key, &count));
    mu_assert_int_eq(COUNT_OF(test_uint_updated_2_data), count);
    mu_check(flipper_format_read_uint32(
        flipper_format, test_uint_key, ARRAY_W_COUNT(uint32_updated_2_data)));
    mu_check(memcmp(test_uint_updated_2_data, ARRAY_W_BSIZE(uint32_updated_2_data)) == 0);

    mu_check(flipper_format_get_value_count(flipper_format, test_float_key, &count));
    mu_assert_int_eq(COUNT_OF(test_float_updated_2_data), count);
    mu_check(flipper_format_read_float(
        flipper_format, test_float_key, ARRAY_W_COUNT(float_updated_2_data)));
    mu_check(memcmp(test_float_updated_2_data, ARRAY_W_BSIZE(float_updated_2_data)) == 0);

    mu_check(flipper_format_get_value_count(flipper_format, test_hex_key, &count));
    mu_assert_int_eq(COUNT_OF(test_hex_updated_2_data), count);
    mu_check(
        flipper_format_read_hex(flipper_format, test_hex_key, ARRAY_W_COUNT(hex_updated_2_data)));
    mu_check(memcmp(test_hex_updated_2_data, ARRAY_W_BSIZE(hex_updated_2_data)) == 0);

    mu_check(flipper_format_get_value_count(flipper_format, test_hex_new_key, &count));
    mu_assert_int_eq(COUNT_OF(test_hex_new_data), count);
    mu_check(
        flipper_format_read_hex(flipper_format, test_hex_new_key, ARRAY_W_COUNT(hex_new_data)));
    mu_check(memcmp(test_hex_new_data, ARRAY_W_BSIZE(hex_new_data)) == 0);

    mu_check(!flipper_format_read_string(flipper_format, "Key that doesn't exist", tmpstr));

    string_clear(tmpstr);

    // delete key test
    mu_check(flipper_format_rewind(flipper_format));
    mu_check(flipper_format_delete_key(flipper_format, test_uint_key));

    // deleted key read test
    mu_check(flipper_format_rewind(flipper_format));
    mu_check(!flipper_format_read_uint32(
        flipper_format, test_uint_key, ARRAY_W_COUNT(uint32_updated_data)));
}

MU_TEST(flipper_format_string_test) {
    FlipperFormat* flipper_format = flipper_format_string_alloc();
    Stream* stream = flipper_format_get_raw_stream(flipper_format);

    mu_check(flipper_format_write_header_cstr(flipper_format, test_filetype, test_version));
    mu_check(flipper_format_write_comment_cstr(flipper_format, "This is comment"));
    mu_check(flipper_format_write_string_cstr(flipper_format, test_string_key, test_string_data));
    mu_check(
        flipper_format_write_int32(flipper_format, test_int_key, ARRAY_W_COUNT(test_int_data)));
    mu_check(
        flipper_format_write_uint32(flipper_format, test_uint_key, ARRAY_W_COUNT(test_uint_data)));
    mu_check(flipper_format_write_float(
        flipper_format, test_float_key, ARRAY_W_COUNT(test_float_data)));
    mu_check(flipper_format_write_hex(flipper_format, test_hex_key, ARRAY_W_COUNT(test_hex_data)));

    MU_RUN_TEST_1(flipper_format_read_and_update_test, flipper_format);

    stream_clean(stream);
    stream_write_cstring(stream, test_data_nix);
    MU_RUN_TEST_1(flipper_format_read_and_update_test, flipper_format);

    stream_clean(stream);
    stream_write_cstring(stream, test_data_win);
    MU_RUN_TEST_1(flipper_format_read_and_update_test, flipper_format);

    flipper_format_free(flipper_format);
}

MU_TEST(flipper_format_file_test) {
    Storage* storage = furi_record_open(RECORD_STORAGE);
    FlipperFormat* flipper_format = flipper_format_file_alloc(storage);
    mu_check(flipper_format_file_open_always(flipper_format, EXT_PATH("flipper.fff")));
    Stream* stream = flipper_format_get_raw_stream(flipper_format);

    mu_check(flipper_format_write_header_cstr(flipper_format, test_filetype, test_version));
    mu_check(flipper_format_write_comment_cstr(flipper_format, "This is comment"));
    mu_check(flipper_format_write_string_cstr(flipper_format, test_string_key, test_string_data));
    mu_check(
        flipper_format_write_int32(flipper_format, test_int_key, ARRAY_W_COUNT(test_int_data)));
    mu_check(
        flipper_format_write_uint32(flipper_format, test_uint_key, ARRAY_W_COUNT(test_uint_data)));
    mu_check(flipper_format_write_float(
        flipper_format, test_float_key, ARRAY_W_COUNT(test_float_data)));
    mu_check(flipper_format_write_hex(flipper_format, test_hex_key, ARRAY_W_COUNT(test_hex_data)));

    MU_RUN_TEST_1(flipper_format_read_and_update_test, flipper_format);

    stream_clean(stream);
    stream_write_cstring(stream, test_data_nix);
    MU_RUN_TEST_1(flipper_format_read_and_update_test, flipper_format);

    stream_clean(stream);
    stream_write_cstring(stream, test_data_win);
    MU_RUN_TEST_1(flipper_format_read_and_update_test, flipper_format);

    flipper_format_free(flipper_format);
    furi_record_close(RECORD_STORAGE);
}

MU_TEST_SUITE(flipper_format_string_suite) {
    MU_RUN_TEST(flipper_format_string_test);
    MU_RUN_TEST(flipper_format_file_test);
}

int run_minunit_test_flipper_format_string() {
    MU_RUN_SUITE(flipper_format_string_suite);
    return MU_EXIT_CODE;
}
