#include <furi.h>
#include "../test.h" // IWYU pragma: keep

#define TEST_RECORD_NAME "test/holding"

void test_furi_create_open(void) {
    // Test that record does not exist
    mu_check(furi_record_exists(TEST_RECORD_NAME) == false);

    // Create record
    uint8_t test_data = 0;
    furi_record_create(TEST_RECORD_NAME, (void*)&test_data);

    // Test that record exists
    mu_check(furi_record_exists(TEST_RECORD_NAME) == true);

    // Open it
    void* record = furi_record_open(TEST_RECORD_NAME);
    mu_assert_pointers_eq(record, &test_data);

    // Close it
    furi_record_close(TEST_RECORD_NAME);

    // Clean up
    furi_record_destroy(TEST_RECORD_NAME);

    // Test that record does not exist
    mu_check(furi_record_exists(TEST_RECORD_NAME) == false);
}
