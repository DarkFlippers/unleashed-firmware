#include "../minunit.h"
#include <furi.h>
#include <furi_hal_delay.h>
#include <storage/storage.h>

#define STORAGE_LOCKED_FILE "/ext/locked_file.test"
#define STORAGE_LOCKED_DIR "/int"

static void storage_file_open_lock_setup() {
    Storage* storage = furi_record_open("storage");
    File* file = storage_file_alloc(storage);
    storage_simply_remove(storage, STORAGE_LOCKED_FILE);
    mu_check(storage_file_open(file, STORAGE_LOCKED_FILE, FSAM_WRITE, FSOM_CREATE_NEW));
    mu_check(storage_file_write(file, "0123", 4) == 4);
    mu_check(storage_file_close(file));
    storage_file_free(file);
    furi_record_close("storage");
}

static void storage_file_open_lock_teardown() {
    Storage* storage = furi_record_open("storage");
    mu_check(storage_simply_remove(storage, STORAGE_LOCKED_FILE));
    furi_record_close("storage");
}

static int32_t storage_file_locker(void* ctx) {
    Storage* storage = furi_record_open("storage");
    osSemaphoreId_t semaphore = ctx;
    File* file = storage_file_alloc(storage);
    furi_check(storage_file_open(file, STORAGE_LOCKED_FILE, FSAM_READ_WRITE, FSOM_OPEN_EXISTING));
    osSemaphoreRelease(semaphore);
    furi_hal_delay_ms(1000);

    furi_check(storage_file_close(file));
    furi_record_close("storage");
    storage_file_free(file);
    return 0;
}

MU_TEST(storage_file_open_lock) {
    Storage* storage = furi_record_open("storage");
    bool result = false;
    osSemaphoreId_t semaphore = osSemaphoreNew(1, 0, NULL);
    File* file = storage_file_alloc(storage);

    // file_locker thread start
    FuriThread* locker_thread = furi_thread_alloc();
    furi_thread_set_name(locker_thread, "StorageFileLocker");
    furi_thread_set_stack_size(locker_thread, 2048);
    furi_thread_set_context(locker_thread, semaphore);
    furi_thread_set_callback(locker_thread, storage_file_locker);
    mu_check(furi_thread_start(locker_thread));

    // wait for file lock
    osSemaphoreAcquire(semaphore, osWaitForever);
    osSemaphoreDelete(semaphore);

    result = storage_file_open(file, STORAGE_LOCKED_FILE, FSAM_READ_WRITE, FSOM_OPEN_EXISTING);
    storage_file_close(file);

    // file_locker thread stop
    mu_check(furi_thread_join(locker_thread) == osOK);
    furi_thread_free(locker_thread);

    // clean data
    storage_file_free(file);
    furi_record_close("storage");

    mu_assert(result, "cannot open locked file");
}

MU_TEST(storage_file_open_close) {
    Storage* storage = furi_record_open("storage");
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

    furi_record_close("storage");
}

MU_TEST_SUITE(storage_file) {
    storage_file_open_lock_setup();
    MU_RUN_TEST(storage_file_open_close);
    MU_RUN_TEST(storage_file_open_lock);
    storage_file_open_lock_teardown();
}

MU_TEST(storage_dir_open_close) {
    Storage* storage = furi_record_open("storage");
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

    furi_record_close("storage");
}

static int32_t storage_dir_locker(void* ctx) {
    Storage* storage = furi_record_open("storage");
    osSemaphoreId_t semaphore = ctx;
    File* file = storage_file_alloc(storage);
    furi_check(storage_dir_open(file, STORAGE_LOCKED_DIR));
    osSemaphoreRelease(semaphore);
    furi_hal_delay_ms(1000);

    furi_check(storage_dir_close(file));
    furi_record_close("storage");
    storage_file_free(file);
    return 0;
}

MU_TEST(storage_dir_open_lock) {
    Storage* storage = furi_record_open("storage");
    bool result = false;
    osSemaphoreId_t semaphore = osSemaphoreNew(1, 0, NULL);
    File* file = storage_file_alloc(storage);

    // file_locker thread start
    FuriThread* locker_thread = furi_thread_alloc();
    furi_thread_set_name(locker_thread, "StorageDirLocker");
    furi_thread_set_stack_size(locker_thread, 2048);
    furi_thread_set_context(locker_thread, semaphore);
    furi_thread_set_callback(locker_thread, storage_dir_locker);
    mu_check(furi_thread_start(locker_thread));

    // wait for dir lock
    osSemaphoreAcquire(semaphore, osWaitForever);
    osSemaphoreDelete(semaphore);

    result = storage_dir_open(file, STORAGE_LOCKED_DIR);
    storage_dir_close(file);

    // file_locker thread stop
    mu_check(furi_thread_join(locker_thread) == osOK);
    furi_thread_free(locker_thread);

    // clean data
    storage_file_free(file);
    furi_record_close("storage");

    mu_assert(result, "cannot open locked dir");
}

MU_TEST_SUITE(storage_dir) {
    MU_RUN_TEST(storage_dir_open_close);
    MU_RUN_TEST(storage_dir_open_lock);
}

int run_minunit_test_storage() {
    MU_RUN_SUITE(storage_file);
    MU_RUN_SUITE(storage_dir);
    return MU_EXIT_CODE;
}