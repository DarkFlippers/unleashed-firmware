#include <furi.h>
#include <furi_hal.h>
#include "../test.h" // IWYU pragma: keep

#include <bt/bt_service/bt_keys_storage.h>
#include <storage/storage.h>

#define BT_TEST_KEY_STORAGE_FILE_PATH EXT_PATH("unit_tests/bt_test.keys")
#define BT_TEST_NVM_RAM_BUFF_SIZE     (507 * 4) // The same as in ble NVM storage

typedef struct {
    Storage* storage;
    BtKeysStorage* bt_keys_storage;
    uint8_t* nvm_ram_buff_dut;
    uint8_t* nvm_ram_buff_ref;
} BtTest;

BtTest* bt_test = NULL;

void bt_test_alloc(void) {
    bt_test = malloc(sizeof(BtTest));
    bt_test->storage = furi_record_open(RECORD_STORAGE);
    bt_test->nvm_ram_buff_dut = malloc(BT_TEST_NVM_RAM_BUFF_SIZE);
    bt_test->nvm_ram_buff_ref = malloc(BT_TEST_NVM_RAM_BUFF_SIZE);
    bt_test->bt_keys_storage = bt_keys_storage_alloc(BT_TEST_KEY_STORAGE_FILE_PATH);
    bt_keys_storage_set_ram_params(
        bt_test->bt_keys_storage, bt_test->nvm_ram_buff_dut, BT_TEST_NVM_RAM_BUFF_SIZE);
}

void bt_test_free(void) {
    furi_check(bt_test);
    free(bt_test->nvm_ram_buff_ref);
    free(bt_test->nvm_ram_buff_dut);
    bt_keys_storage_free(bt_test->bt_keys_storage);
    furi_record_close(RECORD_STORAGE);
    free(bt_test);
    bt_test = NULL;
}

static void bt_test_keys_storage_profile(void) {
    // Emulate nvm change on initial connection
    const int nvm_change_size_on_connection = 88;
    for(size_t i = 0; i < nvm_change_size_on_connection; i++) {
        bt_test->nvm_ram_buff_dut[i] = rand();
        bt_test->nvm_ram_buff_ref[i] = bt_test->nvm_ram_buff_dut[i];
    }
    // Emulate update storage on initial connect
    mu_assert(
        bt_keys_storage_update(
            bt_test->bt_keys_storage, bt_test->nvm_ram_buff_dut, nvm_change_size_on_connection),
        "Failed to update key storage on initial connect");
    memset(bt_test->nvm_ram_buff_dut, 0, BT_TEST_NVM_RAM_BUFF_SIZE);
    mu_assert(bt_keys_storage_load(bt_test->bt_keys_storage), "Failed to load NVM");
    mu_assert(
        memcmp(
            bt_test->nvm_ram_buff_ref, bt_test->nvm_ram_buff_dut, nvm_change_size_on_connection) ==
            0,
        "Wrong buffer loaded");

    const int nvm_disconnect_update_offset = 84;
    const int nvm_disconnect_update_size = 324;
    const int nvm_total_size = nvm_change_size_on_connection -
                               (nvm_change_size_on_connection - nvm_disconnect_update_offset) +
                               nvm_disconnect_update_size;
    // Emulate update storage on initial disconnect
    for(size_t i = nvm_disconnect_update_offset;
        i < nvm_disconnect_update_offset + nvm_disconnect_update_size;
        i++) {
        bt_test->nvm_ram_buff_dut[i] = rand();
        bt_test->nvm_ram_buff_ref[i] = bt_test->nvm_ram_buff_dut[i];
    }
    mu_assert(
        bt_keys_storage_update(
            bt_test->bt_keys_storage,
            &bt_test->nvm_ram_buff_dut[nvm_disconnect_update_offset],
            nvm_disconnect_update_size),
        "Failed to update key storage on initial disconnect");
    memset(bt_test->nvm_ram_buff_dut, 0, BT_TEST_NVM_RAM_BUFF_SIZE);
    mu_assert(bt_keys_storage_load(bt_test->bt_keys_storage), "Failed to load NVM");
    mu_assert(
        memcmp(bt_test->nvm_ram_buff_ref, bt_test->nvm_ram_buff_dut, nvm_total_size) == 0,
        "Wrong buffer loaded");
}

static void bt_test_keys_remove_test_file(void) {
    mu_assert(
        storage_simply_remove(bt_test->storage, BT_TEST_KEY_STORAGE_FILE_PATH),
        "Can't remove test file");
}

MU_TEST(bt_test_keys_storage_serial_profile) {
    furi_check(bt_test);

    bt_test_keys_remove_test_file();
    bt_test_keys_storage_profile();
    bt_test_keys_remove_test_file();
}

MU_TEST_SUITE(test_bt) {
    bt_test_alloc();

    MU_RUN_TEST(bt_test_keys_storage_serial_profile);

    bt_test_free();
}

int run_minunit_test_bt(void) {
    MU_RUN_SUITE(test_bt);
    return MU_EXIT_CODE;
}

TEST_API_DEFINE(run_minunit_test_bt)
