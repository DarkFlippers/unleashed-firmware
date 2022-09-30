#include "../minunit.h"
#include <furi.h>
#include <storage/storage.h>

#define STORAGE_LOCKED_FILE EXT_PATH("locked_file.test")
#define STORAGE_LOCKED_DIR STORAGE_INT_PATH_PREFIX

static void storage_file_open_lock_setup() {
    Storage* storage = furi_record_open(RECORD_STORAGE);
    File* file = storage_file_alloc(storage);
    storage_simply_remove(storage, STORAGE_LOCKED_FILE);
    mu_check(storage_file_open(file, STORAGE_LOCKED_FILE, FSAM_WRITE, FSOM_CREATE_NEW));
    mu_check(storage_file_write(file, "0123", 4) == 4);
    mu_check(storage_file_close(file));
    storage_file_free(file);
    furi_record_close(RECORD_STORAGE);
}

static void storage_file_open_lock_teardown() {
    Storage* storage = furi_record_open(RECORD_STORAGE);
    mu_check(storage_simply_remove(storage, STORAGE_LOCKED_FILE));
    furi_record_close(RECORD_STORAGE);
}

static int32_t storage_file_locker(void* ctx) {
    Storage* storage = furi_record_open(RECORD_STORAGE);
    FuriSemaphore* semaphore = ctx;
    File* file = storage_file_alloc(storage);
    furi_check(storage_file_open(file, STORAGE_LOCKED_FILE, FSAM_READ_WRITE, FSOM_OPEN_EXISTING));
    furi_semaphore_release(semaphore);
    furi_delay_ms(1000);

    furi_check(storage_file_close(file));
    furi_record_close(RECORD_STORAGE);
    storage_file_free(file);
    return 0;
}

MU_TEST(storage_file_open_lock) {
    Storage* storage = furi_record_open(RECORD_STORAGE);
    bool result = false;
    FuriSemaphore* semaphore = furi_semaphore_alloc(1, 0);
    File* file = storage_file_alloc(storage);

    // file_locker thread start
    FuriThread* locker_thread = furi_thread_alloc();
    furi_thread_set_name(locker_thread, "StorageFileLocker");
    furi_thread_set_stack_size(locker_thread, 2048);
    furi_thread_set_context(locker_thread, semaphore);
    furi_thread_set_callback(locker_thread, storage_file_locker);
    furi_thread_start(locker_thread);

    // wait for file lock
    furi_semaphore_acquire(semaphore, FuriWaitForever);
    furi_semaphore_free(semaphore);

    result = storage_file_open(file, STORAGE_LOCKED_FILE, FSAM_READ_WRITE, FSOM_OPEN_EXISTING);
    storage_file_close(file);

    // file_locker thread stop
    mu_check(furi_thread_join(locker_thread));
    furi_thread_free(locker_thread);

    // clean data
    storage_file_free(file);
    furi_record_close(RECORD_STORAGE);

    mu_assert(result, "cannot open locked file");
}

MU_TEST(storage_file_open_close) {
    Storage* storage = furi_record_open(RECORD_STORAGE);
    File* file;

    file = storage_file_alloc(storage);
    mu_check(storage_file_open(file, STORAGE_LOCKED_FILE, FSAM_READ_WRITE, FSOM_OPEN_EXISTING));
    storage_file_close(file);
    storage_file_free(file);

    for(size_t i = 0; i < 10; i++) {
        file = storage_file_alloc(storage);
        mu_check(
            storage_file_open(file, STORAGE_LOCKED_FILE, FSAM_READ_WRITE, FSOM_OPEN_EXISTING));
        storage_file_free(file);
    }

    furi_record_close(RECORD_STORAGE);
}

MU_TEST_SUITE(storage_file) {
    storage_file_open_lock_setup();
    MU_RUN_TEST(storage_file_open_close);
    MU_RUN_TEST(storage_file_open_lock);
    storage_file_open_lock_teardown();
}

MU_TEST(storage_dir_open_close) {
    Storage* storage = furi_record_open(RECORD_STORAGE);
    File* file;

    file = storage_file_alloc(storage);
    mu_check(storage_dir_open(file, STORAGE_LOCKED_DIR));
    storage_dir_close(file);
    storage_file_free(file);

    for(size_t i = 0; i < 10; i++) {
        file = storage_file_alloc(storage);
        mu_check(storage_dir_open(file, STORAGE_LOCKED_DIR));
        storage_file_free(file);
    }

    furi_record_close(RECORD_STORAGE);
}

static int32_t storage_dir_locker(void* ctx) {
    Storage* storage = furi_record_open(RECORD_STORAGE);
    FuriSemaphore* semaphore = ctx;
    File* file = storage_file_alloc(storage);
    furi_check(storage_dir_open(file, STORAGE_LOCKED_DIR));
    furi_semaphore_release(semaphore);
    furi_delay_ms(1000);

    furi_check(storage_dir_close(file));
    furi_record_close(RECORD_STORAGE);
    storage_file_free(file);
    return 0;
}

MU_TEST(storage_dir_open_lock) {
    Storage* storage = furi_record_open(RECORD_STORAGE);
    bool result = false;
    FuriSemaphore* semaphore = furi_semaphore_alloc(1, 0);
    File* file = storage_file_alloc(storage);

    // file_locker thread start
    FuriThread* locker_thread = furi_thread_alloc();
    furi_thread_set_name(locker_thread, "StorageDirLocker");
    furi_thread_set_stack_size(locker_thread, 2048);
    furi_thread_set_context(locker_thread, semaphore);
    furi_thread_set_callback(locker_thread, storage_dir_locker);
    furi_thread_start(locker_thread);

    // wait for dir lock
    furi_semaphore_acquire(semaphore, FuriWaitForever);
    furi_semaphore_free(semaphore);

    result = storage_dir_open(file, STORAGE_LOCKED_DIR);
    storage_dir_close(file);

    // file_locker thread stop
    mu_check(furi_thread_join(locker_thread));
    furi_thread_free(locker_thread);

    // clean data
    storage_file_free(file);
    furi_record_close(RECORD_STORAGE);

    mu_assert(result, "cannot open locked dir");
}

MU_TEST_SUITE(storage_dir) {
    MU_RUN_TEST(storage_dir_open_close);
    MU_RUN_TEST(storage_dir_open_lock);
}

static const char* const storage_copy_test_paths[] = {
    "1",
    "11",
    "111",
    "1/2",
    "1/22",
    "1/222",
    "11/1",
    "111/2",
    "111/22",
    "111/22/33",
};

static const char* const storage_copy_test_files[] = {
    "file.test",
    "1/file.test",
    "111/22/33/file.test",
};

static bool write_file_13DA(Storage* storage, const char* path) {
    File* file = storage_file_alloc(storage);
    bool result = false;
    if(storage_file_open(file, path, FSAM_WRITE, FSOM_CREATE_ALWAYS)) {
        result = storage_file_write(file, "13DA", 4) == 4;
    }
    storage_file_close(file);
    storage_file_free(file);

    return result;
}

static bool check_file_13DA(Storage* storage, const char* path) {
    File* file = storage_file_alloc(storage);
    bool result = false;
    if(storage_file_open(file, path, FSAM_READ, FSOM_OPEN_EXISTING)) {
        char data[10] = {0};
        result = storage_file_read(file, data, 4) == 4;
        if(result) {
            result = memcmp(data, "13DA", 4) == 0;
        }
    }
    storage_file_close(file);
    storage_file_free(file);

    return result;
}

static void storage_dir_create(Storage* storage, const char* base) {
    string_t path;
    string_init(path);

    storage_common_mkdir(storage, base);

    for(size_t i = 0; i < COUNT_OF(storage_copy_test_paths); i++) {
        string_printf(path, "%s/%s", base, storage_copy_test_paths[i]);
        storage_common_mkdir(storage, string_get_cstr(path));
    }

    for(size_t i = 0; i < COUNT_OF(storage_copy_test_files); i++) {
        string_printf(path, "%s/%s", base, storage_copy_test_files[i]);
        write_file_13DA(storage, string_get_cstr(path));
    }

    string_clear(path);
}

static void storage_dir_remove(Storage* storage, const char* base) {
    storage_simply_remove_recursive(storage, base);
}

static bool storage_dir_rename_check(Storage* storage, const char* base) {
    bool result = false;
    string_t path;
    string_init(path);

    result = (storage_common_stat(storage, base, NULL) == FSE_OK);

    if(result) {
        for(size_t i = 0; i < COUNT_OF(storage_copy_test_paths); i++) {
            string_printf(path, "%s/%s", base, storage_copy_test_paths[i]);
            result = (storage_common_stat(storage, string_get_cstr(path), NULL) == FSE_OK);
            if(!result) {
                break;
            }
        }
    }

    if(result) {
        for(size_t i = 0; i < COUNT_OF(storage_copy_test_files); i++) {
            string_printf(path, "%s/%s", base, storage_copy_test_files[i]);
            result = check_file_13DA(storage, string_get_cstr(path));
            if(!result) {
                break;
            }
        }
    }

    string_clear(path);
    return result;
}

MU_TEST(storage_file_rename) {
    Storage* storage = furi_record_open(RECORD_STORAGE);
    File* file = storage_file_alloc(storage);

    mu_check(write_file_13DA(storage, EXT_PATH("file.old")));
    mu_check(check_file_13DA(storage, EXT_PATH("file.old")));
    mu_assert_int_eq(
        FSE_OK, storage_common_rename(storage, EXT_PATH("file.old"), EXT_PATH("file.new")));
    mu_assert_int_eq(FSE_NOT_EXIST, storage_common_stat(storage, EXT_PATH("file.old"), NULL));
    mu_assert_int_eq(FSE_OK, storage_common_stat(storage, EXT_PATH("file.new"), NULL));
    mu_check(check_file_13DA(storage, EXT_PATH("file.new")));
    mu_assert_int_eq(FSE_OK, storage_common_remove(storage, EXT_PATH("file.new")));

    storage_file_free(file);
    furi_record_close(RECORD_STORAGE);
}

MU_TEST(storage_dir_rename) {
    Storage* storage = furi_record_open(RECORD_STORAGE);

    storage_dir_create(storage, EXT_PATH("dir.old"));

    mu_check(storage_dir_rename_check(storage, EXT_PATH("dir.old")));

    mu_assert_int_eq(
        FSE_OK, storage_common_rename(storage, EXT_PATH("dir.old"), EXT_PATH("dir.new")));
    mu_assert_int_eq(FSE_NOT_EXIST, storage_common_stat(storage, EXT_PATH("dir.old"), NULL));
    mu_check(storage_dir_rename_check(storage, EXT_PATH("dir.new")));

    storage_dir_remove(storage, EXT_PATH("dir.new"));
    mu_assert_int_eq(FSE_NOT_EXIST, storage_common_stat(storage, EXT_PATH("dir.new"), NULL));

    furi_record_close(RECORD_STORAGE);
}

MU_TEST_SUITE(storage_rename) {
    MU_RUN_TEST(storage_file_rename);
    MU_RUN_TEST(storage_dir_rename);

    Storage* storage = furi_record_open(RECORD_STORAGE);
    storage_dir_remove(storage, EXT_PATH("dir.old"));
    storage_dir_remove(storage, EXT_PATH("dir.new"));
    furi_record_close(RECORD_STORAGE);
}

int run_minunit_test_storage() {
    MU_RUN_SUITE(storage_file);
    MU_RUN_SUITE(storage_dir);
    MU_RUN_SUITE(storage_rename);
    return MU_EXIT_CODE;
}
