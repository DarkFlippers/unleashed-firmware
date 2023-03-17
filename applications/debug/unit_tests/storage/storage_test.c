#include "../minunit.h"
#include <furi.h>
#include <storage/storage.h>

// DO NOT USE THIS IN PRODUCTION CODE
// This is a hack to access internal storage functions and definitions
#include <storage/storage_i.h>

#define UNIT_TESTS_PATH(path) EXT_PATH("unit_tests/" path)

#define STORAGE_LOCKED_FILE EXT_PATH("locked_file.test")
#define STORAGE_LOCKED_DIR STORAGE_INT_PATH_PREFIX

#define STORAGE_TEST_DIR UNIT_TESTS_PATH("test_dir")

static bool storage_file_create(Storage* storage, const char* path, const char* data) {
    File* file = storage_file_alloc(storage);
    bool result = false;
    do {
        if(!storage_file_open(file, path, FSAM_WRITE, FSOM_CREATE_NEW)) {
            break;
        }

        if(storage_file_write(file, data, strlen(data)) != strlen(data)) {
            break;
        }

        if(!storage_file_close(file)) {
            break;
        }

        result = true;
    } while(0);

    storage_file_free(file);
    return result;
}

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
    FuriThread* locker_thread =
        furi_thread_alloc_ex("StorageFileLocker", 2048, storage_file_locker, semaphore);
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
    furi_delay_ms(100);

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
    FuriThread* locker_thread =
        furi_thread_alloc_ex("StorageDirLocker", 2048, storage_dir_locker, semaphore);
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

MU_TEST(storage_dir_exists_test) {
    Storage* storage = furi_record_open(RECORD_STORAGE);

    mu_check(!storage_dir_exists(storage, STORAGE_TEST_DIR));
    mu_assert_int_eq(FSE_OK, storage_common_mkdir(storage, STORAGE_TEST_DIR));
    mu_check(storage_dir_exists(storage, STORAGE_TEST_DIR));
    mu_assert_int_eq(FSE_OK, storage_common_remove(storage, STORAGE_TEST_DIR));

    furi_record_close(RECORD_STORAGE);
}

MU_TEST_SUITE(storage_dir) {
    MU_RUN_TEST(storage_dir_open_close);
    MU_RUN_TEST(storage_dir_open_lock);
    MU_RUN_TEST(storage_dir_exists_test);
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
    FuriString* path;
    path = furi_string_alloc();

    storage_common_mkdir(storage, base);

    for(size_t i = 0; i < COUNT_OF(storage_copy_test_paths); i++) {
        furi_string_printf(path, "%s/%s", base, storage_copy_test_paths[i]);
        storage_common_mkdir(storage, furi_string_get_cstr(path));
    }

    for(size_t i = 0; i < COUNT_OF(storage_copy_test_files); i++) {
        furi_string_printf(path, "%s/%s", base, storage_copy_test_files[i]);
        write_file_13DA(storage, furi_string_get_cstr(path));
    }

    furi_string_free(path);
}

static void storage_dir_remove(Storage* storage, const char* base) {
    storage_simply_remove_recursive(storage, base);
}

static bool storage_dir_rename_check(Storage* storage, const char* base) {
    bool result = false;
    FuriString* path;
    path = furi_string_alloc();

    result = (storage_common_stat(storage, base, NULL) == FSE_OK);

    if(result) {
        for(size_t i = 0; i < COUNT_OF(storage_copy_test_paths); i++) {
            furi_string_printf(path, "%s/%s", base, storage_copy_test_paths[i]);
            result = (storage_common_stat(storage, furi_string_get_cstr(path), NULL) == FSE_OK);
            if(!result) {
                break;
            }
        }
    }

    if(result) {
        for(size_t i = 0; i < COUNT_OF(storage_copy_test_files); i++) {
            furi_string_printf(path, "%s/%s", base, storage_copy_test_files[i]);
            result = check_file_13DA(storage, furi_string_get_cstr(path));
            if(!result) {
                break;
            }
        }
    }

    furi_string_free(path);
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

#define APPSDATA_APP_PATH(path) APPS_DATA_PATH "/" path

static const char* storage_test_apps[] = {
    "-_twilight_-",
    "-_rainbow_-",
    "-_pinkie_-",
    "-_apple_-",
    "-_flutter_-",
    "-_rare_-",
};

static size_t storage_test_apps_count = COUNT_OF(storage_test_apps);

static int32_t storage_test_app(void* arg) {
    UNUSED(arg);
    Storage* storage = furi_record_open(RECORD_STORAGE);
    storage_common_remove(storage, "/data/test");
    int32_t ret = storage_file_create(storage, "/data/test", "test");
    furi_record_close(RECORD_STORAGE);
    return ret;
}

MU_TEST(test_storage_data_path_apps) {
    for(size_t i = 0; i < storage_test_apps_count; i++) {
        FuriThread* thread =
            furi_thread_alloc_ex(storage_test_apps[i], 1024, storage_test_app, NULL);
        furi_thread_set_appid(thread, storage_test_apps[i]);
        furi_thread_start(thread);
        furi_thread_join(thread);

        mu_assert_int_eq(true, furi_thread_get_return_code(thread));

        // Check if app data dir and file exists
        Storage* storage = furi_record_open(RECORD_STORAGE);
        FuriString* expected = furi_string_alloc();
        furi_string_printf(expected, APPSDATA_APP_PATH("%s"), storage_test_apps[i]);

        mu_check(storage_dir_exists(storage, furi_string_get_cstr(expected)));
        furi_string_cat(expected, "/test");
        mu_check(storage_file_exists(storage, furi_string_get_cstr(expected)));

        furi_string_printf(expected, APPSDATA_APP_PATH("%s"), storage_test_apps[i]);
        storage_simply_remove_recursive(storage, furi_string_get_cstr(expected));

        furi_record_close(RECORD_STORAGE);

        furi_string_free(expected);
        furi_thread_free(thread);
    }
}

MU_TEST(test_storage_data_path) {
    Storage* storage = furi_record_open(RECORD_STORAGE);

    File* file = storage_file_alloc(storage);
    mu_check(storage_dir_open(file, "/data"));
    mu_check(storage_dir_close(file));
    storage_file_free(file);

    // check that appsdata folder exists
    mu_check(storage_dir_exists(storage, APPS_DATA_PATH));

    // check that cli folder exists
    mu_check(storage_dir_exists(storage, APPSDATA_APP_PATH("cli")));

    storage_simply_remove(storage, APPSDATA_APP_PATH("cli"));

    furi_record_close(RECORD_STORAGE);
}

MU_TEST(test_storage_common_migrate) {
    Storage* storage = furi_record_open(RECORD_STORAGE);

    // Setup test folders
    storage_simply_remove_recursive(storage, UNIT_TESTS_PATH("migrate_old"));
    storage_simply_remove_recursive(storage, UNIT_TESTS_PATH("migrate_new"));

    // Test migration from non existing
    mu_assert_int_eq(
        FSE_OK,
        storage_common_migrate(
            storage, UNIT_TESTS_PATH("migrate_old"), UNIT_TESTS_PATH("migrate_new")));

    // Test migration from existing folder to non existing
    mu_assert_int_eq(FSE_OK, storage_common_mkdir(storage, UNIT_TESTS_PATH("migrate_old")));
    mu_check(storage_file_create(storage, UNIT_TESTS_PATH("migrate_old/file1"), "test1"));
    mu_check(storage_file_create(storage, UNIT_TESTS_PATH("migrate_old/file2.ext"), "test2"));
    mu_check(storage_file_create(storage, UNIT_TESTS_PATH("migrate_old/file3.ext.ext"), "test3"));
    mu_assert_int_eq(
        FSE_OK,
        storage_common_migrate(
            storage, UNIT_TESTS_PATH("migrate_old"), UNIT_TESTS_PATH("migrate_new")));

    mu_check(storage_file_exists(storage, UNIT_TESTS_PATH("migrate_new/file1")));
    mu_check(storage_file_exists(storage, UNIT_TESTS_PATH("migrate_new/file2.ext")));
    mu_check(storage_file_exists(storage, UNIT_TESTS_PATH("migrate_new/file3.ext.ext")));
    mu_check(storage_dir_exists(storage, UNIT_TESTS_PATH("migrate_new")));
    mu_check(!storage_dir_exists(storage, UNIT_TESTS_PATH("migrate_old")));

    // Test migration from existing folder to existing folder
    mu_assert_int_eq(FSE_OK, storage_common_mkdir(storage, UNIT_TESTS_PATH("migrate_old")));
    mu_check(storage_file_create(storage, UNIT_TESTS_PATH("migrate_old/file1"), "test1"));
    mu_check(storage_file_create(storage, UNIT_TESTS_PATH("migrate_old/file2.ext"), "test2"));
    mu_check(storage_file_create(storage, UNIT_TESTS_PATH("migrate_old/file3.ext.ext"), "test3"));

    mu_assert_int_eq(
        FSE_OK,
        storage_common_migrate(
            storage, UNIT_TESTS_PATH("migrate_old"), UNIT_TESTS_PATH("migrate_new")));

    mu_check(storage_file_exists(storage, UNIT_TESTS_PATH("migrate_new/file1")));
    mu_check(storage_file_exists(storage, UNIT_TESTS_PATH("migrate_new/file2.ext")));
    mu_check(storage_file_exists(storage, UNIT_TESTS_PATH("migrate_new/file3.ext.ext")));
    mu_check(storage_file_exists(storage, UNIT_TESTS_PATH("migrate_new/file11")));
    mu_check(storage_file_exists(storage, UNIT_TESTS_PATH("migrate_new/file21.ext")));
    mu_check(storage_file_exists(storage, UNIT_TESTS_PATH("migrate_new/file3.ext1.ext")));
    mu_check(storage_dir_exists(storage, UNIT_TESTS_PATH("migrate_new")));
    mu_check(!storage_dir_exists(storage, UNIT_TESTS_PATH("migrate_old")));

    storage_simply_remove_recursive(storage, UNIT_TESTS_PATH("migrate_old"));
    storage_simply_remove_recursive(storage, UNIT_TESTS_PATH("migrate_new"));

    // Test migration from empty folder to existing file
    // Expected result: FSE_OK, folder removed, file untouched
    mu_assert_int_eq(FSE_OK, storage_common_mkdir(storage, UNIT_TESTS_PATH("migrate_old")));
    mu_check(storage_file_create(storage, UNIT_TESTS_PATH("migrate_new"), "test1"));

    mu_assert_int_eq(
        FSE_OK,
        storage_common_migrate(
            storage, UNIT_TESTS_PATH("migrate_old"), UNIT_TESTS_PATH("migrate_new")));

    mu_check(storage_file_exists(storage, UNIT_TESTS_PATH("migrate_new")));
    mu_check(!storage_dir_exists(storage, UNIT_TESTS_PATH("migrate_old")));

    storage_simply_remove_recursive(storage, UNIT_TESTS_PATH("migrate_old"));
    storage_simply_remove_recursive(storage, UNIT_TESTS_PATH("migrate_new"));

    // Test migration from empty folder to existing folder
    // Expected result: FSE_OK, old folder removed, new folder untouched
    mu_assert_int_eq(FSE_OK, storage_common_mkdir(storage, UNIT_TESTS_PATH("migrate_old")));
    mu_assert_int_eq(FSE_OK, storage_common_mkdir(storage, UNIT_TESTS_PATH("migrate_new")));

    mu_assert_int_eq(
        FSE_OK,
        storage_common_migrate(
            storage, UNIT_TESTS_PATH("migrate_old"), UNIT_TESTS_PATH("migrate_new")));

    mu_check(storage_dir_exists(storage, UNIT_TESTS_PATH("migrate_new")));
    mu_check(!storage_dir_exists(storage, UNIT_TESTS_PATH("migrate_old")));

    storage_simply_remove_recursive(storage, UNIT_TESTS_PATH("migrate_old"));
    storage_simply_remove_recursive(storage, UNIT_TESTS_PATH("migrate_new"));

    // Test migration from existing file to non existing, no extension
    mu_check(storage_file_create(storage, UNIT_TESTS_PATH("migrate_old"), "test1"));

    mu_assert_int_eq(
        FSE_OK,
        storage_common_migrate(
            storage, UNIT_TESTS_PATH("migrate_old"), UNIT_TESTS_PATH("migrate_new")));

    mu_check(storage_file_exists(storage, UNIT_TESTS_PATH("migrate_new")));
    mu_check(!storage_file_exists(storage, UNIT_TESTS_PATH("migrate_old")));

    storage_simply_remove_recursive(storage, UNIT_TESTS_PATH("migrate_old"));
    storage_simply_remove_recursive(storage, UNIT_TESTS_PATH("migrate_new"));

    // Test migration from existing file to non existing, with extension
    mu_check(storage_file_create(storage, UNIT_TESTS_PATH("migrate_old.file"), "test1"));

    mu_assert_int_eq(
        FSE_OK,
        storage_common_migrate(
            storage, UNIT_TESTS_PATH("migrate_old.file"), UNIT_TESTS_PATH("migrate_new.file")));

    mu_check(storage_file_exists(storage, UNIT_TESTS_PATH("migrate_new.file")));
    mu_check(!storage_file_exists(storage, UNIT_TESTS_PATH("migrate_old.file")));

    storage_simply_remove_recursive(storage, UNIT_TESTS_PATH("migrate_old.file"));
    storage_simply_remove_recursive(storage, UNIT_TESTS_PATH("migrate_new.file"));

    // Test migration from existing file to existing file, no extension
    mu_check(storage_file_create(storage, UNIT_TESTS_PATH("migrate_old"), "test1"));
    mu_check(storage_file_create(storage, UNIT_TESTS_PATH("migrate_new"), "test2"));

    mu_assert_int_eq(
        FSE_OK,
        storage_common_migrate(
            storage, UNIT_TESTS_PATH("migrate_old"), UNIT_TESTS_PATH("migrate_new")));

    mu_check(storage_file_exists(storage, UNIT_TESTS_PATH("migrate_new")));
    mu_check(!storage_file_exists(storage, UNIT_TESTS_PATH("migrate_old")));
    mu_check(storage_file_exists(storage, UNIT_TESTS_PATH("migrate_new1")));

    storage_simply_remove_recursive(storage, UNIT_TESTS_PATH("migrate_old"));
    storage_simply_remove_recursive(storage, UNIT_TESTS_PATH("migrate_new"));
    storage_simply_remove_recursive(storage, UNIT_TESTS_PATH("migrate_new1"));

    // Test migration from existing file to existing file, with extension
    mu_check(storage_file_create(storage, UNIT_TESTS_PATH("migrate_old.file"), "test1"));
    mu_check(storage_file_create(storage, UNIT_TESTS_PATH("migrate_new.file"), "test2"));

    mu_assert_int_eq(
        FSE_OK,
        storage_common_migrate(
            storage, UNIT_TESTS_PATH("migrate_old.file"), UNIT_TESTS_PATH("migrate_new.file")));

    mu_check(storage_file_exists(storage, UNIT_TESTS_PATH("migrate_new.file")));
    mu_check(!storage_file_exists(storage, UNIT_TESTS_PATH("migrate_old.file")));
    mu_check(storage_file_exists(storage, UNIT_TESTS_PATH("migrate_new1.file")));

    storage_simply_remove_recursive(storage, UNIT_TESTS_PATH("migrate_old.file"));
    storage_simply_remove_recursive(storage, UNIT_TESTS_PATH("migrate_new.file"));
    storage_simply_remove_recursive(storage, UNIT_TESTS_PATH("migrate_new1.file"));

    // Test migration from existing file to existing folder
    mu_check(storage_file_create(storage, UNIT_TESTS_PATH("migrate_old"), "test1"));
    mu_assert_int_eq(FSE_OK, storage_common_mkdir(storage, UNIT_TESTS_PATH("migrate_new")));

    mu_assert_int_eq(
        FSE_OK,
        storage_common_migrate(
            storage, UNIT_TESTS_PATH("migrate_old"), UNIT_TESTS_PATH("migrate_new")));

    mu_check(storage_dir_exists(storage, UNIT_TESTS_PATH("migrate_new")));
    mu_check(!storage_file_exists(storage, UNIT_TESTS_PATH("migrate_old")));
    mu_check(storage_file_exists(storage, UNIT_TESTS_PATH("migrate_new1")));

    storage_simply_remove_recursive(storage, UNIT_TESTS_PATH("migrate_old"));
    storage_simply_remove_recursive(storage, UNIT_TESTS_PATH("migrate_new"));
    storage_simply_remove_recursive(storage, UNIT_TESTS_PATH("migrate_new1"));

    furi_record_close(RECORD_STORAGE);
}

MU_TEST_SUITE(test_data_path) {
    MU_RUN_TEST(test_storage_data_path);
    MU_RUN_TEST(test_storage_data_path_apps);
}

MU_TEST_SUITE(test_storage_common) {
    MU_RUN_TEST(test_storage_common_migrate);
}

int run_minunit_test_storage() {
    MU_RUN_SUITE(storage_file);
    MU_RUN_SUITE(storage_dir);
    MU_RUN_SUITE(storage_rename);
    MU_RUN_SUITE(test_data_path);
    MU_RUN_SUITE(test_storage_common);
    return MU_EXIT_CODE;
}
