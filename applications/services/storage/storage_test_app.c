#include <furi.h>
#include <furi_hal.h>
#include <storage/storage.h>

#define TAG "StorageTest"
#define BYTES_COUNT 16
#define TEST_STRING "TestDataStringProvidedByDiceRoll"
#define SEEK_OFFSET_FROM_START 10
#define SEEK_OFFSET_INCREASE 12
#define SEEK_OFFSET_SUM (SEEK_OFFSET_FROM_START + SEEK_OFFSET_INCREASE)

static void do_file_test(Storage* api, const char* path) {
    File* file = storage_file_alloc(api);
    bool result;
    uint8_t bytes[BYTES_COUNT + 1];
    uint8_t bytes_count;
    uint64_t position;
    uint64_t size;

    FURI_LOG_I(TAG, "--------- FILE \"%s\" ---------", path);

    // open
    result = storage_file_open(file, path, FSAM_WRITE, FSOM_CREATE_ALWAYS);
    if(result) {
        FURI_LOG_I(TAG, "open");
    } else {
        FURI_LOG_E(TAG, "open, %s", storage_file_get_error_desc(file));
    }

    // write
    bytes_count = storage_file_write(file, TEST_STRING, strlen(TEST_STRING));
    if(bytes_count == 0) {
        FURI_LOG_E(TAG, "write, %s", storage_file_get_error_desc(file));
    } else {
        FURI_LOG_I(TAG, "write");
    }

    // sync
    result = storage_file_sync(file);
    if(result) {
        FURI_LOG_I(TAG, "sync");
    } else {
        FURI_LOG_E(TAG, "sync, %s", storage_file_get_error_desc(file));
    }

    // eof #1
    result = storage_file_eof(file);
    if(result) {
        FURI_LOG_I(TAG, "eof #1");
    } else {
        FURI_LOG_E(TAG, "eof #1, %s", storage_file_get_error_desc(file));
    }

    // seek from start and tell
    result = storage_file_seek(file, SEEK_OFFSET_FROM_START, true);
    if(result) {
        FURI_LOG_I(TAG, "seek #1");
    } else {
        FURI_LOG_E(TAG, "seek #1, %s", storage_file_get_error_desc(file));
    }
    position = storage_file_tell(file);
    if(position != SEEK_OFFSET_FROM_START) {
        FURI_LOG_E(TAG, "tell #1, %s", storage_file_get_error_desc(file));
    } else {
        FURI_LOG_I(TAG, "tell #1");
    }

    // size
    size = storage_file_size(file);
    if(size != strlen(TEST_STRING)) {
        FURI_LOG_E(TAG, "size #1, %s", storage_file_get_error_desc(file));
    } else {
        FURI_LOG_I(TAG, "size #1");
    }

    // seek and tell
    result = storage_file_seek(file, SEEK_OFFSET_INCREASE, false);
    if(result) {
        FURI_LOG_I(TAG, "seek #2");
    } else {
        FURI_LOG_E(TAG, "seek #2, %s", storage_file_get_error_desc(file));
    }
    position = storage_file_tell(file);
    if(position != SEEK_OFFSET_SUM) {
        FURI_LOG_E(TAG, "tell #2, %s", storage_file_get_error_desc(file));
    } else {
        FURI_LOG_I(TAG, "tell #2");
    }

    // eof #2
    result = storage_file_eof(file);
    if(!result) {
        FURI_LOG_I(TAG, "eof #2");
    } else {
        FURI_LOG_E(TAG, "eof #2, %s", storage_file_get_error_desc(file));
    }

    // truncate
    result = storage_file_truncate(file);
    if(result) {
        FURI_LOG_I(TAG, "truncate");
    } else {
        FURI_LOG_E(TAG, "truncate, %s", storage_file_get_error_desc(file));
    }
    size = storage_file_size(file);
    if(size != SEEK_OFFSET_SUM) {
        FURI_LOG_E(TAG, "size #2, %s", storage_file_get_error_desc(file));
    } else {
        FURI_LOG_I(TAG, "size #2");
    }

    // close
    result = storage_file_close(file);
    if(result) {
        FURI_LOG_I(TAG, "close");
    } else {
        FURI_LOG_E(TAG, "close, error");
    }

    // open
    result = storage_file_open(file, path, FSAM_READ, FSOM_OPEN_EXISTING);
    if(result) {
        FURI_LOG_I(TAG, "open");
    } else {
        FURI_LOG_E(TAG, "open, %s", storage_file_get_error_desc(file));
    }

    // read
    memset(bytes, 0, BYTES_COUNT + 1);
    bytes_count = storage_file_read(file, bytes, BYTES_COUNT);
    if(bytes_count == 0) {
        FURI_LOG_E(TAG, "read, %s", storage_file_get_error_desc(file));
    } else {
        if(memcmp(TEST_STRING, bytes, bytes_count) == 0) {
            FURI_LOG_I(TAG, "read");
        } else {
            FURI_LOG_E(TAG, "read, garbage");
        }
    }

    // close
    result = storage_file_close(file);
    if(result) {
        FURI_LOG_I(TAG, "close");
    } else {
        FURI_LOG_E(TAG, "close, error");
    }

    storage_file_free(file);
}

static void do_dir_test(Storage* api, const char* path) {
    File* file = storage_file_alloc(api);
    bool result;

    FURI_LOG_I(TAG, "--------- DIR \"%s\" ---------", path);

    // open
    result = storage_dir_open(file, path);
    if(result) {
        FURI_LOG_I(TAG, "open");
    } else {
        FURI_LOG_E(TAG, "open, %s", storage_file_get_error_desc(file));
    }

    // read
    const uint8_t filename_size = 100;
    char* filename = malloc(filename_size);
    FileInfo fileinfo;

    do {
        result = storage_dir_read(file, &fileinfo, filename, filename_size);
        if(result) {
            if(strlen(filename)) {
                FURI_LOG_I(
                    TAG,
                    "read #1, [%s]%s",
                    ((fileinfo.flags & FSF_DIRECTORY) ? "D" : "F"),
                    filename);
            }
        } else if(storage_file_get_error(file) != FSE_NOT_EXIST) {
            FURI_LOG_E(TAG, "read #1, %s", storage_file_get_error_desc(file));
            break;
        }

    } while(result);

    // rewind
    result = storage_dir_rewind(file);
    if(result) {
        FURI_LOG_I(TAG, "rewind");
    } else {
        FURI_LOG_E(TAG, "rewind, %s", storage_file_get_error_desc(file));
    }

    // read
    do {
        result = storage_dir_read(file, &fileinfo, filename, filename_size);
        if(result) {
            if(strlen(filename)) {
                FURI_LOG_I(
                    TAG,
                    "read #2, [%s]%s",
                    ((fileinfo.flags & FSF_DIRECTORY) ? "D" : "F"),
                    filename);
            }
        } else if(storage_file_get_error(file) != FSE_NOT_EXIST) {
            FURI_LOG_E(TAG, "read #2, %s", storage_file_get_error_desc(file));
            break;
        }

    } while((strlen(filename)));

    // close
    result = storage_dir_close(file);
    if(result) {
        FURI_LOG_I(TAG, "close");
    } else {
        FURI_LOG_E(TAG, "close, error");
    }

    storage_file_free(file);
    free(filename);
}

static void do_test_start(Storage* api, const char* path) {
    string_t str_path;
    string_init_printf(str_path, "%s/test-folder", path);

    FURI_LOG_I(TAG, "--------- START \"%s\" ---------", path);

    // mkdir
    FS_Error result = storage_common_mkdir(api, string_get_cstr(str_path));

    if(result == FSE_OK) {
        FURI_LOG_I(TAG, "mkdir ok");
    } else {
        FURI_LOG_E(TAG, "mkdir, %s", storage_error_get_desc(result));
    }

    // stat
    FileInfo fileinfo;
    result = storage_common_stat(api, string_get_cstr(str_path), &fileinfo);

    if(result == FSE_OK) {
        if(fileinfo.flags & FSF_DIRECTORY) {
            FURI_LOG_I(TAG, "stat #1 ok");
        } else {
            FURI_LOG_E(TAG, "stat #1, %s", storage_error_get_desc(result));
        }
    } else {
        FURI_LOG_E(TAG, "stat #1, %s", storage_error_get_desc(result));
    }

    string_clear(str_path);
}

static void do_test_end(Storage* api, const char* path) {
    uint64_t total_space;
    uint64_t free_space;
    string_t str_path_1;
    string_t str_path_2;
    string_init_printf(str_path_1, "%s/test-folder", path);
    string_init_printf(str_path_2, "%s/test-folder2", path);

    FURI_LOG_I(TAG, "--------- END \"%s\" ---------", path);

    // fs stat
    FS_Error result = storage_common_fs_info(api, path, &total_space, &free_space);

    if(result == FSE_OK) {
        uint32_t total_kb = total_space / 1024;
        uint32_t free_kb = free_space / 1024;
        FURI_LOG_I(TAG, "fs_info: total %luk, free %luk", total_kb, free_kb);
    } else {
        FURI_LOG_E(TAG, "fs_info, %s", storage_error_get_desc(result));
    }

    // rename #1
    result = storage_common_rename(api, string_get_cstr(str_path_1), string_get_cstr(str_path_2));
    if(result == FSE_OK) {
        FURI_LOG_I(TAG, "rename #1 ok");
    } else {
        FURI_LOG_E(TAG, "rename #1, %s", storage_error_get_desc(result));
    }

    // remove #1
    result = storage_common_remove(api, string_get_cstr(str_path_2));
    if(result == FSE_OK) {
        FURI_LOG_I(TAG, "remove #1 ok");
    } else {
        FURI_LOG_E(TAG, "remove #1, %s", storage_error_get_desc(result));
    }

    // rename #2
    string_printf(str_path_1, "%s/test.txt", path);
    string_printf(str_path_2, "%s/test2.txt", path);

    result = storage_common_rename(api, string_get_cstr(str_path_1), string_get_cstr(str_path_2));
    if(result == FSE_OK) {
        FURI_LOG_I(TAG, "rename #2 ok");
    } else {
        FURI_LOG_E(TAG, "rename #2, %s", storage_error_get_desc(result));
    }

    // remove #2
    result = storage_common_remove(api, string_get_cstr(str_path_2));
    if(result == FSE_OK) {
        FURI_LOG_I(TAG, "remove #2 ok");
    } else {
        FURI_LOG_E(TAG, "remove #2, %s", storage_error_get_desc(result));
    }

    string_clear(str_path_1);
    string_clear(str_path_2);
}

int32_t storage_test_app(void* p) {
    UNUSED(p);
    Storage* api = furi_record_open(RECORD_STORAGE);
    do_test_start(api, STORAGE_INT_PATH_PREFIX);
    do_test_start(api, STORAGE_ANY_PATH_PREFIX);
    do_test_start(api, STORAGE_EXT_PATH_PREFIX);

    do_file_test(api, INT_PATH("test.txt"));
    do_file_test(api, ANY_PATH("test.txt"));
    do_file_test(api, EXT_PATH("test.txt"));

    do_dir_test(api, STORAGE_INT_PATH_PREFIX);
    do_dir_test(api, STORAGE_ANY_PATH_PREFIX);
    do_dir_test(api, STORAGE_EXT_PATH_PREFIX);

    do_test_end(api, STORAGE_INT_PATH_PREFIX);
    do_test_end(api, STORAGE_ANY_PATH_PREFIX);
    do_test_end(api, STORAGE_EXT_PATH_PREFIX);

    while(true) {
        furi_delay_ms(1000);
    }

    return 0;
}
