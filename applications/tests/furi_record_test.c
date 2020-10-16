#include <stdio.h>
#include <string.h>
#include "flipper.h"
#include "flipper_v2.h"
#include "log.h"
#include "minunit.h"

void test_furi_create_open() {
    // 1. Create record
    uint8_t test_data = 0;
    mu_check(furi_create("test/holding", (void*)&test_data));

    // 2. Open it
    void* record = furi_open("test/holding");
    mu_assert_pointers_eq(record, &test_data);
}
