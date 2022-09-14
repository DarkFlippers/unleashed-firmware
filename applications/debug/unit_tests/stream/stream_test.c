#include <furi.h>
#include <toolbox/stream/stream.h>
#include <toolbox/stream/string_stream.h>
#include <toolbox/stream/file_stream.h>
#include <toolbox/stream/buffered_file_stream.h>
#include <storage/storage.h>
#include "../minunit.h"

static const char* stream_test_data = "I write differently from what I speak, "
                                      "I speak differently from what I think, "
                                      "I think differently from the way I ought to think, "
                                      "and so it all proceeds into deepest darkness.";

static const char* stream_test_left_data = "There are two cardinal human sins ";
static const char* stream_test_right_data =
    "from which all others derive: impatience and indolence.";

MU_TEST_1(stream_composite_subtest, Stream* stream) {
    const size_t data_size = 128;
    uint8_t data[data_size];
    string_t string_lee;
    string_init_set(string_lee, "lee");

    // test that stream is empty
    // "" -> ""
    mu_check(stream_size(stream) == 0);
    mu_check(stream_eof(stream));
    mu_check(stream_tell(stream) == 0);
    mu_check(stream_read(stream, data, data_size) == 0);
    mu_check(stream_eof(stream));
    mu_check(stream_tell(stream) == 0);

    // write char
    // "" -> "2"
    mu_check(stream_write_char(stream, '2') == 1);
    mu_check(stream_size(stream) == 1);
    mu_check(stream_tell(stream) == 1);
    mu_check(stream_eof(stream));

    // test rewind and eof
    stream_rewind(stream);
    mu_check(stream_size(stream) == 1);
    mu_check(stream_tell(stream) == 0);
    mu_check(!stream_eof(stream));

    // add another char with replacement
    // "2" -> "1"
    mu_check(stream_write_char(stream, '1') == 1);
    mu_check(stream_size(stream) == 1);
    mu_check(stream_tell(stream) == 1);
    mu_check(stream_eof(stream));

    // write string
    // "1" -> "1337_69"
    mu_check(stream_write_cstring(stream, "337_69") == 6);
    mu_check(stream_size(stream) == 7);
    mu_check(stream_tell(stream) == 7);
    mu_check(stream_eof(stream));

    // read data
    memset(data, 0, data_size);
    stream_rewind(stream);
    mu_check(stream_read(stream, data, data_size) == 7);
    mu_check(strcmp((char*)data, "1337_69") == 0);

    // test misc seeks
    mu_check(stream_seek(stream, 2, StreamOffsetFromStart));
    mu_check(stream_tell(stream) == 2);
    mu_check(!stream_seek(stream, 9000, StreamOffsetFromStart));
    mu_check(stream_tell(stream) == 7);
    mu_check(stream_eof(stream));
    mu_check(stream_seek(stream, -3, StreamOffsetFromEnd));
    mu_check(stream_tell(stream) == 4);

    // write string with replacemet
    // "1337_69" -> "1337lee"
    mu_check(stream_write_string(stream, string_lee) == 3);
    mu_check(stream_size(stream) == 7);
    mu_check(stream_tell(stream) == 7);
    mu_check(stream_eof(stream));

    // append char
    // "1337lee" -> "1337leet"
    mu_check(stream_write(stream, (uint8_t*)"t", 1) == 1);
    mu_check(stream_size(stream) == 8);
    mu_check(stream_tell(stream) == 8);
    mu_check(stream_eof(stream));

    // read data
    memset(data, 0, data_size);
    stream_rewind(stream);
    mu_check(stream_read(stream, data, data_size) == 8);
    mu_check(strcmp((char*)data, "1337leet") == 0);
    mu_check(stream_tell(stream) == 8);
    mu_check(stream_eof(stream));

    // negative seek from current position -> clamp to 0
    mu_check(!stream_seek(stream, -9000, StreamOffsetFromCurrent));
    mu_check(stream_tell(stream) == 0);

    // negative seek from start position -> clamp to 0
    stream_rewind(stream);
    mu_check(!stream_seek(stream, -3, StreamOffsetFromStart));
    mu_check(stream_tell(stream) == 0);

    // zero seek from current position -> clamp to stream size
    mu_check(stream_seek(stream, 0, StreamOffsetFromEnd));
    mu_check(stream_tell(stream) == 8);

    // negative seek from end position -> clamp to 0
    mu_check(!stream_seek(stream, -9000, StreamOffsetFromEnd));
    mu_check(stream_tell(stream) == 0);

    // clean stream
    stream_clean(stream);
    mu_check(stream_size(stream) == 0);
    mu_check(stream_eof(stream));
    mu_check(stream_tell(stream) == 0);

    // write format
    // "" -> "dio666"
    mu_check(stream_write_format(stream, "%s%d", "dio", 666) == 6);
    mu_check(stream_size(stream) == 6);
    mu_check(stream_eof(stream));
    mu_check(stream_tell(stream) == 6);

    // read data
    memset(data, 0, data_size);
    stream_rewind(stream);
    mu_check(stream_read(stream, data, data_size) == 6);
    mu_check(strcmp((char*)data, "dio666") == 0);

    // clean and write cstring
    // "dio666" -> "" -> "1234567890"
    stream_clean(stream);
    mu_check(stream_write_cstring(stream, "1234567890") == 10);

    // delete 4 bytes from 1 pos
    // "1xxxx67890" -> "167890"
    mu_check(stream_seek(stream, 1, StreamOffsetFromStart));
    mu_check(stream_delete(stream, 4));
    mu_assert_int_eq(6, stream_size(stream));

    // read data
    memset(data, 0, data_size);
    stream_rewind(stream);
    mu_assert_int_eq(6, stream_read(stream, data, data_size));
    mu_check(strcmp((char*)data, "167890") == 0);

    // write cstring
    // "167890" -> "167890It Was Me, Dio!"
    mu_check(stream_write_cstring(stream, "It Was Me, Dio!") == 15);

    // delete 1337 bytes from 1 pos
    // and check that we can delete only 20 bytes
    // "1xxxxxxxxxxxxxxxxxxxx" -> "1"
    mu_check(stream_seek(stream, 1, StreamOffsetFromStart));
    mu_check(stream_delete(stream, 1337));
    mu_assert_int_eq(1, stream_size(stream));

    // read data
    memset(data, 0, data_size);
    stream_rewind(stream);
    mu_check(stream_read(stream, data, data_size) == 1);
    mu_check(strcmp((char*)data, "1") == 0);

    // write cstring from 0 pos, replacing 1 byte
    // "1" -> "Oh? You're roaching me?"
    mu_check(stream_rewind(stream));
    mu_assert_int_eq(23, stream_write_cstring(stream, "Oh? You're roaching me?"));

    // insert 11 bytes to 0 pos
    // "Oh? You're roaching me?" -> "Za Warudo! Oh? You're roaching me?"
    mu_check(stream_rewind(stream));
    mu_check(stream_insert(stream, (uint8_t*)"Za Warudo! ", 11));
    mu_assert_int_eq(34, stream_size(stream));

    // read data
    memset(data, 0, data_size);
    stream_rewind(stream);
    mu_assert_int_eq(34, stream_read(stream, data, data_size));
    mu_assert_string_eq("Za Warudo! Oh? You're roaching me?", (char*)data);

    // insert cstring to 22 pos
    // "Za Warudo! Oh? You're roaching me?" -> "Za Warudo! Oh? You're approaching me?"
    mu_check(stream_seek(stream, 22, StreamOffsetFromStart));
    mu_check(stream_insert_cstring(stream, "app"));
    mu_assert_int_eq(37, stream_size(stream));

    // read data
    memset(data, 0, data_size);
    stream_rewind(stream);
    mu_assert_int_eq(37, stream_read(stream, data, data_size));
    mu_assert_string_eq("Za Warudo! Oh? You're approaching me?", (char*)data);

    // insert cstring to the end of the stream
    // "Za Warudo! Oh? You're approaching me?" -> "Za Warudo! Oh? You're approaching me? It was me, Dio!"
    mu_check(stream_seek(stream, 0, StreamOffsetFromEnd));
    mu_check(stream_insert_cstring(stream, " It was me, Dio!"));
    mu_assert_int_eq(53, stream_size(stream));

    // read data
    memset(data, 0, data_size);
    stream_rewind(stream);
    mu_assert_int_eq(53, stream_read(stream, data, data_size));
    mu_assert_string_eq("Za Warudo! Oh? You're approaching me? It was me, Dio!", (char*)data);

    // delete 168430090 bytes from stream
    // and test that we can delete only 53
    mu_check(stream_rewind(stream));
    mu_check(stream_delete(stream, 0x0A0A0A0A));
    mu_assert_int_eq(0, stream_size(stream));
    mu_check(stream_eof(stream));
    mu_assert_int_eq(0, stream_tell(stream));

    // clean stream
    stream_clean(stream);
    mu_assert_int_eq(0, stream_size(stream));
    mu_check(stream_eof(stream));
    mu_assert_int_eq(0, stream_tell(stream));

    // insert formated string at the end of stream
    // "" -> "dio666"
    mu_check(stream_insert_format(stream, "%s%d", "dio", 666));
    mu_assert_int_eq(6, stream_size(stream));
    mu_check(stream_eof(stream));
    mu_assert_int_eq(6, stream_tell(stream));

    // insert formated string at the end of stream
    // "dio666" -> "dio666zlo555"
    mu_check(stream_insert_format(stream, "%s%d", "zlo", 555));
    mu_assert_int_eq(12, stream_size(stream));
    mu_check(stream_eof(stream));
    mu_assert_int_eq(12, stream_tell(stream));

    // insert formated string at the 6 pos
    // "dio666" -> "dio666baba13zlo555"
    mu_check(stream_seek(stream, 6, StreamOffsetFromStart));
    mu_check(stream_insert_format(stream, "%s%d", "baba", 13));
    mu_assert_int_eq(18, stream_size(stream));
    mu_assert_int_eq(12, stream_tell(stream));

    // read data
    memset(data, 0, data_size);
    stream_rewind(stream);
    mu_assert_int_eq(18, stream_read(stream, data, data_size));
    mu_assert_string_eq("dio666baba13zlo555", (char*)data);

    // delete 6 chars from pos 6 and insert 1 chars
    // "dio666baba13zlo555" -> "dio666xzlo555"
    mu_check(stream_seek(stream, 6, StreamOffsetFromStart));
    mu_check(stream_delete_and_insert_char(stream, 6, 'x'));
    mu_assert_int_eq(13, stream_size(stream));
    mu_assert_int_eq(7, stream_tell(stream));

    // read data
    memset(data, 0, data_size);
    stream_rewind(stream);
    mu_check(stream_read(stream, data, data_size) == 13);
    mu_assert_string_eq("dio666xzlo555", (char*)data);

    // delete 9000 chars from pos 6 and insert 3 chars from string
    // "dio666xzlo555" -> "dio666777"
    mu_check(stream_seek(stream, 6, StreamOffsetFromStart));
    mu_check(stream_delete_and_insert_cstring(stream, 9000, "777"));
    mu_assert_int_eq(9, stream_size(stream));
    mu_assert_int_eq(9, stream_tell(stream));
    mu_check(stream_eof(stream));

    string_clear(string_lee);
}

MU_TEST(stream_composite_test) {
    // test string stream
    Stream* stream;
    stream = string_stream_alloc();
    MU_RUN_TEST_1(stream_composite_subtest, stream);
    stream_free(stream);

    // test file stream
    Storage* storage = furi_record_open(RECORD_STORAGE);
    stream = file_stream_alloc(storage);
    mu_check(
        file_stream_open(stream, EXT_PATH("filestream.str"), FSAM_READ_WRITE, FSOM_CREATE_ALWAYS));
    MU_RUN_TEST_1(stream_composite_subtest, stream);
    stream_free(stream);

    // test buffered file stream
    stream = buffered_file_stream_alloc(storage);
    mu_check(buffered_file_stream_open(
        stream, EXT_PATH("filestream.str"), FSAM_READ_WRITE, FSOM_CREATE_ALWAYS));
    MU_RUN_TEST_1(stream_composite_subtest, stream);
    stream_free(stream);
    furi_record_close(RECORD_STORAGE);
}

MU_TEST_1(stream_write_subtest, Stream* stream) {
    mu_assert_int_eq(strlen(stream_test_data), stream_write_cstring(stream, stream_test_data));
}

MU_TEST_1(stream_read_subtest, Stream* stream) {
    uint8_t data[256] = {0};
    mu_check(stream_rewind(stream));
    mu_assert_int_eq(strlen(stream_test_data), stream_read(stream, data, 256));
    mu_assert_string_eq(stream_test_data, (const char*)data);
}

MU_TEST(stream_write_read_save_load_test) {
    Stream* stream_orig = string_stream_alloc();
    Stream* stream_copy = string_stream_alloc();
    Storage* storage = furi_record_open(RECORD_STORAGE);

    // write, read
    MU_RUN_TEST_1(stream_write_subtest, stream_orig);
    MU_RUN_TEST_1(stream_read_subtest, stream_orig);

    // copy, read
    mu_assert_int_eq(strlen(stream_test_data), stream_copy_full(stream_orig, stream_copy));
    MU_RUN_TEST_1(stream_read_subtest, stream_orig);

    // save to file
    mu_check(stream_seek(stream_orig, 0, StreamOffsetFromStart));
    mu_assert_int_eq(
        strlen(stream_test_data),
        stream_save_to_file(stream_orig, storage, EXT_PATH("filestream.str"), FSOM_CREATE_ALWAYS));

    stream_free(stream_copy);
    stream_free(stream_orig);

    // load from file, read
    Stream* stream_new = string_stream_alloc();
    mu_assert_int_eq(
        strlen(stream_test_data),
        stream_load_from_file(stream_new, storage, EXT_PATH("filestream.str")));
    MU_RUN_TEST_1(stream_read_subtest, stream_new);
    stream_free(stream_new);

    furi_record_close(RECORD_STORAGE);
}

MU_TEST_1(stream_split_subtest, Stream* stream) {
    stream_clean(stream);
    stream_write_cstring(stream, stream_test_left_data);
    stream_write_cstring(stream, stream_test_right_data);

    Stream* stream_left = string_stream_alloc();
    Stream* stream_right = string_stream_alloc();

    mu_check(stream_seek(stream, strlen(stream_test_left_data), StreamOffsetFromStart));
    mu_check(stream_split(stream, stream_left, stream_right));

    uint8_t data[256] = {0};
    mu_check(stream_rewind(stream_left));
    mu_assert_int_eq(strlen(stream_test_left_data), stream_read(stream_left, data, 256));
    mu_assert_string_eq(stream_test_left_data, (const char*)data);

    mu_check(stream_rewind(stream_right));
    mu_assert_int_eq(strlen(stream_test_right_data), stream_read(stream_right, data, 256));
    mu_assert_string_eq(stream_test_right_data, (const char*)data);

    stream_free(stream_right);
    stream_free(stream_left);
}

MU_TEST(stream_split_test) {
    // test string stream
    Stream* stream;
    stream = string_stream_alloc();
    MU_RUN_TEST_1(stream_split_subtest, stream);
    stream_free(stream);

    // test file stream
    Storage* storage = furi_record_open(RECORD_STORAGE);
    stream = file_stream_alloc(storage);
    mu_check(
        file_stream_open(stream, EXT_PATH("filestream.str"), FSAM_READ_WRITE, FSOM_CREATE_ALWAYS));
    MU_RUN_TEST_1(stream_split_subtest, stream);
    stream_free(stream);

    // test buffered stream
    stream = buffered_file_stream_alloc(storage);
    mu_check(buffered_file_stream_open(
        stream, EXT_PATH("filestream.str"), FSAM_READ_WRITE, FSOM_CREATE_ALWAYS));
    MU_RUN_TEST_1(stream_split_subtest, stream);
    stream_free(stream);

    furi_record_close(RECORD_STORAGE);
}

MU_TEST(stream_buffered_write_after_read_test) {
    const char* prefix = "I write ";
    const char* substr = "Hello there";

    const size_t substr_len = strlen(substr);
    const size_t prefix_len = strlen(prefix);
    const size_t buf_size = substr_len + 1;

    char buf[buf_size];
    memset(buf, 0, buf_size);

    Storage* storage = furi_record_open(RECORD_STORAGE);
    Stream* stream = buffered_file_stream_alloc(storage);
    mu_check(buffered_file_stream_open(
        stream, EXT_PATH("filestream.str"), FSAM_READ_WRITE, FSOM_CREATE_ALWAYS));
    mu_assert_int_eq(strlen(stream_test_data), stream_write_cstring(stream, stream_test_data));
    mu_check(stream_rewind(stream));
    mu_assert_int_eq(prefix_len, stream_read(stream, (uint8_t*)buf, prefix_len));
    mu_assert_string_eq(prefix, buf);
    mu_assert_int_eq(substr_len, stream_write(stream, (uint8_t*)substr, substr_len));
    mu_check(stream_seek(stream, prefix_len, StreamOffsetFromStart));
    mu_assert_int_eq(substr_len, stream_read(stream, (uint8_t*)buf, substr_len));
    mu_assert_string_eq(substr, buf);

    stream_free(stream);
    furi_record_close(RECORD_STORAGE);
}

MU_TEST(stream_buffered_large_file_test) {
    string_t input_data;
    string_t output_data;
    string_init(input_data);
    string_init(output_data);

    Storage* storage = furi_record_open(RECORD_STORAGE);

    // generate test data consisting of several identical lines
    const size_t data_size = 4096;
    const size_t line_size = strlen(stream_test_data);
    const size_t rep_count = data_size / line_size + 1;

    for(size_t i = 0; i < rep_count; ++i) {
        string_cat_printf(input_data, "%s\n", stream_test_data);
    }

    // write test data to file
    Stream* stream = buffered_file_stream_alloc(storage);
    mu_check(buffered_file_stream_open(
        stream, EXT_PATH("filestream.str"), FSAM_READ_WRITE, FSOM_CREATE_ALWAYS));
    mu_assert_int_eq(0, stream_size(stream));
    mu_assert_int_eq(string_size(input_data), stream_write_string(stream, input_data));
    mu_assert_int_eq(string_size(input_data), stream_size(stream));

    const size_t substr_start = 8;
    const size_t substr_len = 11;

    mu_check(stream_seek(stream, substr_start, StreamOffsetFromStart));
    mu_assert_int_eq(substr_start, stream_tell(stream));

    // copy one substring from test data
    char test_substr[substr_len + 1];
    memset(test_substr, 0, substr_len + 1);
    memcpy(test_substr, stream_test_data + substr_start, substr_len);

    char buf[substr_len + 1];
    memset(buf, 0, substr_len + 1);

    // read substring
    mu_assert_int_eq(substr_len, stream_read(stream, (uint8_t*)buf, substr_len));
    mu_assert_string_eq(test_substr, buf);
    memset(buf, 0, substr_len + 1);

    // forward seek to cause a cache miss
    mu_check(stream_seek(
        stream, (line_size + 1) * (rep_count - 1) - substr_len, StreamOffsetFromCurrent));
    // read same substring from a different line
    mu_assert_int_eq(substr_len, stream_read(stream, (uint8_t*)buf, substr_len));
    mu_assert_string_eq(test_substr, buf);
    memset(buf, 0, substr_len + 1);

    // backward seek to cause a cache miss
    mu_check(stream_seek(
        stream, -((line_size + 1) * (rep_count - 1) + substr_len), StreamOffsetFromCurrent));
    mu_assert_int_eq(substr_len, stream_read(stream, (uint8_t*)buf, substr_len));
    mu_assert_string_eq(test_substr, buf);

    // read the whole file
    mu_check(stream_rewind(stream));
    string_t tmp;
    string_init(tmp);
    while(stream_read_line(stream, tmp)) {
        string_cat(output_data, tmp);
    }
    string_clear(tmp);

    // check against generated data
    mu_assert_int_eq(string_size(input_data), string_size(output_data));
    mu_check(string_equal_p(input_data, output_data));
    mu_check(stream_eof(stream));

    stream_free(stream);

    furi_record_close(RECORD_STORAGE);
    string_clear(input_data);
    string_clear(output_data);
}

MU_TEST_SUITE(stream_suite) {
    MU_RUN_TEST(stream_write_read_save_load_test);
    MU_RUN_TEST(stream_composite_test);
    MU_RUN_TEST(stream_split_test);
    MU_RUN_TEST(stream_buffered_write_after_read_test);
    MU_RUN_TEST(stream_buffered_large_file_test);
}

int run_minunit_test_stream() {
    MU_RUN_SUITE(stream_suite);
    return MU_EXIT_CODE;
}
