#include <furi.h>
#include <furi_hal.h>
#include "../test.h" // IWYU pragma: keep

#include <bt/bt_service/bt_keys_storage.h>
#include <storage/storage.h>
#include <toolbox/saved_struct.h>

#define BT_TEST_KEY_STORAGE_FILE_PATH EXT_PATH("unit_tests/bt_test.keys")
#define BT_TEST_MIGRATION_FILE_PATH   EXT_PATH("unit_tests/bt_migration_test.keys")
#define BT_TEST_NVM_RAM_BUFF_SIZE     (507 * 4) // The same as in ble NVM storage

// Identity root key
static const uint8_t gap_legacy_irk[16] =
    {0x12, 0x34, 0x56, 0x78, 0x9a, 0xbc, 0xde, 0xf0, 0x12, 0x34, 0x56, 0x78, 0x9a, 0xbc, 0xde, 0xf0};
// Encryption root key
static const uint8_t gap_legacy_erk[16] =
    {0xfe, 0xdc, 0xba, 0x09, 0x87, 0x65, 0x43, 0x21, 0xfe, 0xdc, 0xba, 0x09, 0x87, 0x65, 0x43, 0x21};

// Test constants for migration (matching bt_keys_storage.c)
#define BT_KEYS_STORAGE_MAGIC_TEST     (0x18)
#define BT_KEYS_STORAGE_VERSION_1_TEST (1)

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

// Helper function to create a version 0 file manually
static bool
    bt_test_create_v0_file(const char* file_path, const uint8_t* nvm_data, size_t nvm_size) {
    // Version 0 files use saved_struct format with magic 0x18, version 0, containing only BLE pairing data
    return saved_struct_save(
        file_path,
        nvm_data,
        nvm_size,
        BT_KEYS_STORAGE_MAGIC_TEST,
        0); // Version 0
}

// Helper function to verify file format version
static bool bt_test_verify_file_version(const char* file_path, uint32_t expected_version) {
    uint8_t magic, version;
    size_t size;

    if(!saved_struct_get_metadata(file_path, &magic, &version, &size)) {
        return false;
    }

    return (magic == BT_KEYS_STORAGE_MAGIC_TEST && version == expected_version);
}

// Test migration from version 0 to version 1, including root key preservation
static void bt_test_migration_v0_to_v1(void) {
    // Create test NVM data
    const size_t test_nvm_size = 100;
    uint8_t test_nvm_data[test_nvm_size];
    for(size_t i = 0; i < test_nvm_size; i++) {
        test_nvm_data[i] = (uint8_t)(i & 0xFF);
    }

    // Create a version 0 file
    mu_assert(
        bt_test_create_v0_file(BT_TEST_MIGRATION_FILE_PATH, test_nvm_data, test_nvm_size),
        "Failed to create version 0 test file");

    // Create BT keys storage and load the v0 file (should trigger migration)
    BtKeysStorage* migration_storage = bt_keys_storage_alloc(BT_TEST_MIGRATION_FILE_PATH);
    uint8_t loaded_buffer[BT_TEST_NVM_RAM_BUFF_SIZE];
    memset(loaded_buffer, 0, sizeof(loaded_buffer));
    bt_keys_storage_set_ram_params(migration_storage, loaded_buffer, sizeof(loaded_buffer));

    // Load should succeed and migrate v0 to v1
    mu_assert(bt_keys_storage_load(migration_storage), "Failed to load and migrate v0 file");

    // Verify the file is now version 1
    mu_assert(
        bt_test_verify_file_version(BT_TEST_MIGRATION_FILE_PATH, BT_KEYS_STORAGE_VERSION_1_TEST),
        "File was not migrated to version 1");

    // Verify the NVM data was preserved during migration
    mu_assert(
        memcmp(test_nvm_data, loaded_buffer, test_nvm_size) == 0,
        "NVM data was corrupted during migration");

    // Verify that legacy root keys are used after migration
    const GapRootSecurityKeys* migrated_keys = bt_keys_storage_get_root_keys(migration_storage);
    mu_assert(
        memcmp(migrated_keys->irk, gap_legacy_irk, sizeof(gap_legacy_irk)) == 0,
        "IRK not set to legacy after migration");
    mu_assert(
        memcmp(migrated_keys->erk, gap_legacy_erk, sizeof(gap_legacy_erk)) == 0,
        "ERK not set to legacy after migration");

    bt_keys_storage_free(migration_storage);
    storage_simply_remove(bt_test->storage, BT_TEST_MIGRATION_FILE_PATH);
}

// Test that migration preserves existing pairing data and root keys are not changed on reload
static void bt_test_migration_preserves_pairings_and_keys(void) {
    const size_t pairing_data_size = 200;
    uint8_t pairing_data[pairing_data_size];
    for(size_t i = 0; i < pairing_data_size; i++) {
        pairing_data[i] = (uint8_t)((i * 7 + 42) & 0xFF);
    }
    mu_assert(
        bt_test_create_v0_file(BT_TEST_MIGRATION_FILE_PATH, pairing_data, pairing_data_size),
        "Failed to create v0 file with pairing data");

    GapRootSecurityKeys keys_after_first_load;
    for(int iteration = 0; iteration < 2; iteration++) {
        BtKeysStorage* storage = bt_keys_storage_alloc(BT_TEST_MIGRATION_FILE_PATH);
        uint8_t buffer[BT_TEST_NVM_RAM_BUFF_SIZE];
        memset(buffer, 0, sizeof(buffer));
        bt_keys_storage_set_ram_params(storage, buffer, sizeof(buffer));
        mu_assert(bt_keys_storage_load(storage), "Failed to load on iteration");
        mu_assert(
            memcmp(pairing_data, buffer, pairing_data_size) == 0,
            "Pairing data corrupted on iteration");
        const GapRootSecurityKeys* keys = bt_keys_storage_get_root_keys(storage);
        if(iteration == 0)
            memcpy(&keys_after_first_load, keys, sizeof(GapRootSecurityKeys));
        else
            mu_assert(
                memcmp(&keys_after_first_load, keys, sizeof(GapRootSecurityKeys)) == 0,
                "Root keys changed after reload");
        bt_keys_storage_free(storage);
    }
    storage_simply_remove(bt_test->storage, BT_TEST_MIGRATION_FILE_PATH);
}

// Test that delete operation generates new secure keys in v1 and does not match legacy
static void bt_test_delete_generates_new_keys_and_not_legacy(void) {
    BtKeysStorage* storage = bt_keys_storage_alloc(BT_TEST_MIGRATION_FILE_PATH);
    uint8_t buffer[BT_TEST_NVM_RAM_BUFF_SIZE];
    memset(buffer, 0x55, sizeof(buffer));
    bt_keys_storage_set_ram_params(storage, buffer, sizeof(buffer));
    mu_assert(bt_keys_storage_update(storage, buffer, 100), "Failed to create initial v1 file");
    const GapRootSecurityKeys* original_keys = bt_keys_storage_get_root_keys(storage);
    uint8_t original_keys_copy[sizeof(GapRootSecurityKeys)];
    memcpy(original_keys_copy, original_keys, sizeof(original_keys_copy));
    bt_keys_storage_delete(storage);
    const GapRootSecurityKeys* new_keys = bt_keys_storage_get_root_keys(storage);
    mu_assert(
        memcmp(original_keys_copy, new_keys, sizeof(original_keys_copy)) != 0,
        "Root keys were not regenerated after delete");
    mu_assert(
        memcmp(new_keys->irk, gap_legacy_irk, sizeof(gap_legacy_irk)) != 0,
        "IRK after delete should not match legacy");
    mu_assert(
        memcmp(new_keys->erk, gap_legacy_erk, sizeof(gap_legacy_erk)) != 0,
        "ERK after delete should not match legacy");
    bt_keys_storage_free(storage);
    storage_simply_remove(bt_test->storage, BT_TEST_MIGRATION_FILE_PATH);
}

MU_TEST(bt_test_keys_storage_serial_profile) {
    furi_check(bt_test);

    bt_test_keys_remove_test_file();
    bt_test_keys_storage_profile();
    bt_test_keys_remove_test_file();
}

MU_TEST(bt_test_migration_v0_to_v1_test) {
    furi_check(bt_test);
    bt_test_migration_v0_to_v1();
}

MU_TEST(bt_test_migration_preserves_pairings_and_keys_test) {
    furi_check(bt_test);
    bt_test_migration_preserves_pairings_and_keys();
}

MU_TEST(bt_test_delete_generates_new_keys_and_not_legacy_test) {
    furi_check(bt_test);
    bt_test_delete_generates_new_keys_and_not_legacy();
}

MU_TEST_SUITE(test_bt) {
    bt_test_alloc();

    MU_RUN_TEST(bt_test_keys_storage_serial_profile);
    MU_RUN_TEST(bt_test_migration_v0_to_v1_test);
    MU_RUN_TEST(bt_test_migration_preserves_pairings_and_keys_test);
    MU_RUN_TEST(bt_test_delete_generates_new_keys_and_not_legacy_test);

    bt_test_free();
}

int run_minunit_test_bt(void) {
    MU_RUN_SUITE(test_bt);
    return MU_EXIT_CODE;
}

TEST_API_DEFINE(run_minunit_test_bt)
