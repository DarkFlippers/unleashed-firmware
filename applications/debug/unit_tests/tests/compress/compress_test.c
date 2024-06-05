#include "../test.h" // IWYU pragma: keep

#include <toolbox/compress.h>

#include <furi.h>
#include <furi_hal.h>
#include <furi_hal_random.h>

#include <storage/storage.h>

#include <stdint.h>

#define COMPRESS_UNIT_TESTS_PATH(path) EXT_PATH("unit_tests/compress/" path)

static void compress_test_reference_comp_decomp() {
    Storage* storage = furi_record_open(RECORD_STORAGE);

    File* compressed_file = storage_file_alloc(storage);
    File* decompressed_file = storage_file_alloc(storage);

    mu_assert(
        storage_file_open(
            compressed_file,
            COMPRESS_UNIT_TESTS_PATH("compressed.bin"),
            FSAM_READ,
            FSOM_OPEN_EXISTING),
        "Failed to open compressed file");
    mu_assert(
        storage_file_open(
            decompressed_file,
            COMPRESS_UNIT_TESTS_PATH("uncompressed.bin"),
            FSAM_READ,
            FSOM_OPEN_EXISTING),
        "Failed to open decompressed file");

    uint64_t compressed_ref_size = storage_file_size(compressed_file);
    uint64_t decompressed_ref_size = storage_file_size(decompressed_file);

    mu_assert(compressed_ref_size > 0 && decompressed_ref_size > 0, "Invalid file sizes");

    uint8_t* compressed_ref_buff = malloc(compressed_ref_size);
    uint8_t* decompressed_ref_buff = malloc(decompressed_ref_size);

    mu_assert(
        storage_file_read(compressed_file, compressed_ref_buff, compressed_ref_size) ==
            compressed_ref_size,
        "Failed to read compressed file");

    mu_assert(
        storage_file_read(decompressed_file, decompressed_ref_buff, decompressed_ref_size) ==
            decompressed_ref_size,
        "Failed to read decompressed file");

    storage_file_free(compressed_file);
    storage_file_free(decompressed_file);
    furi_record_close(RECORD_STORAGE);

    uint8_t* temp_buffer = malloc(1024);
    Compress* comp = compress_alloc(1024);

    size_t encoded_size = 0;
    mu_assert(
        compress_encode(
            comp, decompressed_ref_buff, decompressed_ref_size, temp_buffer, 1024, &encoded_size),
        "Compress failed");

    mu_assert(encoded_size == compressed_ref_size, "Encoded size is not equal to reference size");

    mu_assert(
        memcmp(temp_buffer, compressed_ref_buff, compressed_ref_size) == 0,
        "Encoded buffer is not equal to reference");

    size_t decoded_size = 0;
    mu_assert(
        compress_decode(
            comp, compressed_ref_buff, compressed_ref_size, temp_buffer, 1024, &decoded_size),
        "Decompress failed");

    mu_assert(
        decoded_size == decompressed_ref_size, "Decoded size is not equal to reference size");

    mu_assert(
        memcmp(temp_buffer, decompressed_ref_buff, decompressed_ref_size) == 0,
        "Decoded buffer is not equal to reference");

    compress_free(comp);

    free(temp_buffer);
    free(compressed_ref_buff);
    free(decompressed_ref_buff);
}

static void compress_test_random_comp_decomp() {
    static const size_t src_buffer_size = 1024;
    static const size_t encoded_buffer_size = 1024;
    static const size_t small_buffer_size = src_buffer_size / 32;

    // We only fill half of the buffer with random data, so if anything goes wrong, there's no overflow
    static const size_t src_data_size = src_buffer_size / 2;

    Compress* comp = compress_alloc(src_buffer_size);
    uint8_t* src_buff = malloc(src_buffer_size);
    uint8_t* encoded_buff = malloc(encoded_buffer_size);
    uint8_t* decoded_buff = malloc(src_buffer_size);
    uint8_t* small_buff = malloc(small_buffer_size);

    furi_hal_random_fill_buf(src_buff, src_data_size);

    size_t encoded_size = 0;

    mu_assert(
        compress_encode(
            comp, src_buff, src_data_size, encoded_buff, encoded_buffer_size, &encoded_size),
        "Compress failed");

    mu_assert(encoded_size > 0, "Encoded size is zero");

    size_t small_enc_dec_size = 0;
    mu_assert(
        compress_encode(
            comp, src_buff, src_data_size, small_buff, small_buffer_size, &small_enc_dec_size) ==
            false,
        "Compress to small buffer failed");

    size_t decoded_size = 0;
    mu_assert(
        compress_decode(
            comp, encoded_buff, encoded_size, decoded_buff, src_buffer_size, &decoded_size),
        "Decompress failed");
    mu_assert(decoded_size == src_data_size, "Decoded size is not equal to source size");

    mu_assert(
        memcmp(src_buff, decoded_buff, src_data_size) == 0,
        "Decoded buffer is not equal to source");

    mu_assert(
        compress_decode(
            comp, encoded_buff, encoded_size, small_buff, small_buffer_size, &small_enc_dec_size) ==
            false,
        "Decompress to small buffer failed");

    free(small_buff);
    free(src_buff);
    free(encoded_buff);
    free(decoded_buff);
    compress_free(comp);
}

MU_TEST_SUITE(test_compress) {
    MU_RUN_TEST(compress_test_random_comp_decomp);
    MU_RUN_TEST(compress_test_reference_comp_decomp);
}

int run_minunit_test_compress(void) {
    MU_RUN_SUITE(test_compress);
    return MU_EXIT_CODE;
}

TEST_API_DEFINE(run_minunit_test_compress)
