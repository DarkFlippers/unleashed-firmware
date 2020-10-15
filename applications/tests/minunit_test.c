#include <stdio.h>
#include "flipper.h"
#include "log.h"
#include "minunit_vars.h"
#include "minunit.h"

bool test_furi_ac_create_kill();
bool test_furi_ac_switch_exit();

bool test_furi_nonexistent_data();
bool test_furi_mute_algorithm();

// v2 tests
void test_furi_create_open();
void test_furi_valuemutex();
void test_furi_concurrent_access();

static int foo = 0;

void test_setup(void) {
    foo = 7;
}

void test_teardown(void) {
    /* Nothing */
}

MU_TEST(test_check) {
    mu_check(foo != 6);
}

MU_TEST(mu_test_furi_ac_create_kill) {
    mu_assert_int_eq(test_furi_ac_create_kill(), true);
}

MU_TEST(mu_test_furi_ac_switch_exit) {
    mu_assert_int_eq(test_furi_ac_switch_exit(), true);
}

MU_TEST(mu_test_furi_nonexistent_data) {
    mu_assert_int_eq(test_furi_nonexistent_data(), true);
}

// v2 tests
MU_TEST(mu_test_furi_create_open) {
    test_furi_create_open();
}

MU_TEST(mu_test_furi_valuemutex) {
    test_furi_valuemutex();
}

MU_TEST(mu_test_furi_concurrent_access) {
    test_furi_concurrent_access();
}

MU_TEST_SUITE(test_suite) {
    MU_SUITE_CONFIGURE(&test_setup, &test_teardown);

    MU_RUN_TEST(test_check);
    MU_RUN_TEST(mu_test_furi_ac_create_kill);
    MU_RUN_TEST(mu_test_furi_ac_switch_exit);

    MU_RUN_TEST(mu_test_furi_nonexistent_data);

    // v2 tests
    MU_RUN_TEST(mu_test_furi_create_open);
    MU_RUN_TEST(mu_test_furi_valuemutex);
    MU_RUN_TEST(mu_test_furi_concurrent_access);
}

int run_minunit() {
    MU_RUN_SUITE(test_suite);
    MU_REPORT();

    return MU_EXIT_CODE;
}