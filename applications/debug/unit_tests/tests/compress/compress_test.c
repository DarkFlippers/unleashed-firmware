#include "../test.h" // IWYU pragma: keep

#include <toolbox/compress.h>
#include <toolbox/md5_calc.h>
#include <toolbox/tar/tar_archive.h>
#include <toolbox/dir_walk.h>

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
    Compress* comp = compress_alloc(CompressTypeHeatshrink, &compress_config_heatshrink_default);

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

    Compress* comp = compress_alloc(CompressTypeHeatshrink, &compress_config_heatshrink_default);
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

static int32_t hs_unpacker_file_read(void* context, uint8_t* buffer, size_t size) {
    File* file = (File*)context;
    return storage_file_read(file, buffer, size);
}

static int32_t hs_unpacker_file_write(void* context, uint8_t* buffer, size_t size) {
    File* file = (File*)context;
    return storage_file_write(file, buffer, size);
}
/*
Source file was generated with:
```python3
import random, string
random.seed(1337)
with open("hsstream.out.bin", "wb") as f:
    for c in random.choices(string.printable, k=1024):
        for _ in range(random.randint(1, 10)):
            f.write(c.encode())
```

It was compressed with heatshrink using the following command:
`python3 -m heatshrink2 compress -w 9 -l 4 hsstream.out.bin hsstream.in.bin`
*/

#define HSSTREAM_IN  COMPRESS_UNIT_TESTS_PATH("hsstream.in.bin")
#define HSSTREAM_OUT COMPRESS_UNIT_TESTS_PATH("hsstream.out.bin")

static void compress_test_heatshrink_stream() {
    Storage* api = furi_record_open(RECORD_STORAGE);
    File* comp_file = storage_file_alloc(api);
    File* dest_file = storage_file_alloc(api);

    CompressConfigHeatshrink config = {
        .window_sz2 = 9,
        .lookahead_sz2 = 4,
        .input_buffer_sz = 128,
    };
    Compress* compress = compress_alloc(CompressTypeHeatshrink, &config);

    do {
        storage_simply_remove(api, HSSTREAM_OUT);

        mu_assert(
            storage_file_open(comp_file, HSSTREAM_IN, FSAM_READ, FSOM_OPEN_EXISTING),
            "Failed to open compressed file");

        mu_assert(
            storage_file_open(dest_file, HSSTREAM_OUT, FSAM_WRITE, FSOM_OPEN_ALWAYS),
            "Failed to open decompressed file");

        mu_assert(
            compress_decode_streamed(
                compress, hs_unpacker_file_read, comp_file, hs_unpacker_file_write, dest_file),
            "Decompression failed");

        storage_file_close(dest_file);

        unsigned char md5[16];
        FS_Error file_error;
        mu_assert(
            md5_calc_file(dest_file, HSSTREAM_OUT, md5, &file_error), "Failed to calculate md5");

        const unsigned char expected_md5[16] = {
            0xa3,
            0x70,
            0xe8,
            0x8b,
            0xa9,
            0x42,
            0x74,
            0xf4,
            0xaa,
            0x12,
            0x8d,
            0x41,
            0xd2,
            0xb6,
            0x71,
            0xc9};
        mu_assert(memcmp(md5, expected_md5, sizeof(md5)) == 0, "MD5 mismatch after decompression");

        storage_simply_remove(api, HSSTREAM_OUT);
    } while(false);

    compress_free(compress);
    storage_file_free(comp_file);
    storage_file_free(dest_file);
    furi_record_close(RECORD_STORAGE);
}

#define HS_TAR_PATH         COMPRESS_UNIT_TESTS_PATH("test.ths")
#define HS_TAR_EXTRACT_PATH COMPRESS_UNIT_TESTS_PATH("tar_out")

static bool file_counter(const char* name, bool is_dir, void* context) {
    UNUSED(name);
    UNUSED(is_dir);
    int32_t* n_entries = (int32_t*)context;
    (*n_entries)++;
    return true;
}

/*
Heatshrink tar file contents and MD5 sums:
file1.txt:                      64295676ceed5cce2d0dcac402e4bda4
file2.txt:                      188f67f297eedd7bf3d6a4d3c2fc31c4
dir/file3.txt:                  34d98ad8135ffe502dba374690136d16
dir/big_file.txt:               ee169c1e1791a4d319dbfaefaa850e98
dir/nested_dir/file4.txt:       e099fcb2aaa0672375eaedc549247ee6
dir/nested_dir/empty_file.txt:  d41d8cd98f00b204e9800998ecf8427e 

XOR of all MD5 sums:            92ed5729786d0e1176d047e35f52d376
*/

static void compress_test_heatshrink_tar() {
    Storage* api = furi_record_open(RECORD_STORAGE);

    TarArchive* archive = tar_archive_alloc(api);
    FuriString* path = furi_string_alloc();
    FileInfo fileinfo;
    File* file = storage_file_alloc(api);

    do {
        storage_simply_remove_recursive(api, HS_TAR_EXTRACT_PATH);

        mu_assert(storage_simply_mkdir(api, HS_TAR_EXTRACT_PATH), "Failed to create extract dir");

        mu_assert(
            tar_archive_get_mode_for_path(HS_TAR_PATH) == TarOpenModeReadHeatshrink,
            "Invalid mode for heatshrink tar");

        mu_assert(
            tar_archive_open(archive, HS_TAR_PATH, TarOpenModeReadHeatshrink),
            "Failed to open heatshrink tar");

        int32_t n_entries = 0;
        tar_archive_set_file_callback(archive, file_counter, &n_entries);

        mu_assert(
            tar_archive_unpack_to(archive, HS_TAR_EXTRACT_PATH, NULL),
            "Failed to unpack heatshrink tar");

        mu_assert(n_entries == 9, "Invalid number of entries in heatshrink tar");

        uint8_t md5_total[16] = {0}, md5_file[16];

        DirWalk* dir_walk = dir_walk_alloc(api);
        mu_assert(dir_walk_open(dir_walk, HS_TAR_EXTRACT_PATH), "Failed to open dirwalk");
        while(dir_walk_read(dir_walk, path, &fileinfo) == DirWalkOK) {
            if(file_info_is_dir(&fileinfo)) {
                continue;
            }
            mu_assert(
                md5_calc_file(file, furi_string_get_cstr(path), md5_file, NULL),
                "Failed to calc md5");

            for(size_t i = 0; i < 16; i++) {
                md5_total[i] ^= md5_file[i];
            }
        }
        dir_walk_free(dir_walk);

        static const unsigned char expected_md5[16] = {
            0x92,
            0xed,
            0x57,
            0x29,
            0x78,
            0x6d,
            0x0e,
            0x11,
            0x76,
            0xd0,
            0x47,
            0xe3,
            0x5f,
            0x52,
            0xd3,
            0x76};
        mu_assert(memcmp(md5_total, expected_md5, sizeof(md5_total)) == 0, "MD5 mismatch");

        storage_simply_remove_recursive(api, HS_TAR_EXTRACT_PATH);
    } while(false);

    storage_file_free(file);
    furi_string_free(path);
    tar_archive_free(archive);
    furi_record_close(RECORD_STORAGE);
}

MU_TEST_SUITE(test_compress) {
    MU_RUN_TEST(compress_test_random_comp_decomp);
    MU_RUN_TEST(compress_test_reference_comp_decomp);
    MU_RUN_TEST(compress_test_heatshrink_stream);
    MU_RUN_TEST(compress_test_heatshrink_tar);
}

int run_minunit_test_compress(void) {
    MU_RUN_SUITE(test_compress);
    return MU_EXIT_CODE;
}

TEST_API_DEFINE(run_minunit_test_compress)
