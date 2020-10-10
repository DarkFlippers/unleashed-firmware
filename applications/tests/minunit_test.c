#include <stdio.h>
#include "flipper.h"
#include "log.h"
#include "minunit_vars.h"
#include "minunit.h"

bool test_furi_ac_create_kill();
bool test_furi_ac_switch_exit();
bool test_furi_pipe_record();
bool test_furi_holding_data();
bool test_furi_concurrent_access();
bool test_furi_nonexistent_data();
bool test_furi_mute_algorithm();

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

MU_TEST(mu_test_furi_pipe_record) {
    mu_assert_int_eq(test_furi_pipe_record(), true);
}

MU_TEST(mu_test_furi_holding_data) {
    mu_assert_int_eq(test_furi_holding_data(), true);
}

MU_TEST(mu_test_furi_concurrent_access) {
    mu_assert_int_eq(test_furi_concurrent_access(), true);
}

MU_TEST(mu_test_furi_nonexistent_data) {
    mu_assert_int_eq(test_furi_nonexistent_data(), true);
}

/*
MU_TEST(mu_test_furi_mute_algorithm) {
    mu_assert_int_eq(test_furi_mute_algorithm(test_log), true);
}
*/

MU_TEST_SUITE(test_suite) {
    MU_SUITE_CONFIGURE(&test_setup, &test_teardown);

    MU_RUN_TEST(test_check);
    MU_RUN_TEST(mu_test_furi_ac_create_kill);
    MU_RUN_TEST(mu_test_furi_ac_switch_exit);
    MU_RUN_TEST(mu_test_furi_pipe_record);
    MU_RUN_TEST(mu_test_furi_holding_data);
    MU_RUN_TEST(mu_test_furi_concurrent_access);
    MU_RUN_TEST(mu_test_furi_nonexistent_data);
    // MU_RUN_TEST(mu_test_furi_mute_algorithm);
}

int run_minunit() {
    MU_RUN_SUITE(test_suite);
    MU_REPORT();

    return MU_EXIT_CODE;
}