
#include "../test.h" // IWYU pragma: keep
#include <toolbox/args.h>

const uint32_t one_ms = 1;
const uint32_t one_s = 1000 * one_ms;
const uint32_t one_m = 60 * one_s;
const uint32_t one_h = 60 * one_m;

MU_TEST(args_read_duration_default_values_test) {
    FuriString* args_string;
    uint32_t value = 0;

    // Check default == NULL (ms)
    args_string = furi_string_alloc_set_str("1");
    mu_check(args_read_duration(args_string, &value, NULL));
    mu_assert_int_eq(value, one_ms);
    furi_string_free(args_string);
    value = 0;

    // Check default == ms
    args_string = furi_string_alloc_set_str("1");
    mu_check(args_read_duration(args_string, &value, "ms"));
    mu_assert_int_eq(value, one_ms);
    furi_string_free(args_string);
    value = 0;

    // Check default == s
    args_string = furi_string_alloc_set_str("1");
    mu_check(args_read_duration(args_string, &value, "s"));
    mu_assert_int_eq(value, one_s);
    furi_string_free(args_string);
    value = 0;

    // Check default == m
    args_string = furi_string_alloc_set_str("1");
    mu_check(args_read_duration(args_string, &value, "m"));
    mu_assert_int_eq(value, one_m);
    furi_string_free(args_string);
    value = 0;

    // Check default == h
    args_string = furi_string_alloc_set_str("1");
    mu_check(args_read_duration(args_string, &value, "h"));
    mu_assert_int_eq(value, one_h);
    furi_string_free(args_string);
    value = 0;
}

MU_TEST(args_read_duration_suffix_values_test) {
    FuriString* args_string;
    uint32_t value = 0;

    // Check ms
    args_string = furi_string_alloc_set_str("1ms");
    mu_check(args_read_duration(args_string, &value, NULL));
    mu_assert_int_eq(value, one_ms);
    furi_string_free(args_string);
    value = 0;

    // Check s
    args_string = furi_string_alloc_set_str("1s");
    mu_check(args_read_duration(args_string, &value, NULL));
    mu_assert_int_eq(value, one_s);
    furi_string_free(args_string);
    value = 0;

    // Check m
    args_string = furi_string_alloc_set_str("1m");
    mu_check(args_read_duration(args_string, &value, NULL));
    mu_assert_int_eq(value, one_m);
    furi_string_free(args_string);
    value = 0;

    // Check h
    args_string = furi_string_alloc_set_str("1h");
    mu_check(args_read_duration(args_string, &value, NULL));
    mu_assert_int_eq(value, one_h);
    furi_string_free(args_string);
    value = 0;
}

MU_TEST(args_read_duration_values_test) {
    FuriString* args_string;
    uint32_t value = 0;

    // Check for ms
    args_string = furi_string_alloc_set_str("4294967295ms");
    mu_check(args_read_duration(args_string, &value, NULL));
    mu_assert_int_eq(value, 4294967295U);
    furi_string_free(args_string);

    // Check for s
    args_string = furi_string_alloc_set_str("4294967s");
    mu_check(args_read_duration(args_string, &value, NULL));
    mu_assert_int_eq(value, 4294967U * one_s);
    furi_string_free(args_string);

    // Check for m
    args_string = furi_string_alloc_set_str("71582m");
    mu_check(args_read_duration(args_string, &value, NULL));
    mu_assert_int_eq(value, 71582U * one_m);
    furi_string_free(args_string);

    // Check for h
    args_string = furi_string_alloc_set_str("1193h");
    mu_check(args_read_duration(args_string, &value, NULL));
    mu_assert_int_eq(value, 1193U * one_h);
    furi_string_free(args_string);

    // Check for ms in float
    args_string = furi_string_alloc_set_str("4.2ms");
    mu_check(args_read_duration(args_string, &value, NULL));
    mu_assert_int_eq(value, 4);
    furi_string_free(args_string);

    // Check for s in float
    args_string = furi_string_alloc_set_str("1.5s");
    mu_check(args_read_duration(args_string, &value, NULL));
    mu_assert_int_eq(value, (uint32_t)(1.5 * one_s));
    furi_string_free(args_string);

    // Check for m in float
    args_string = furi_string_alloc_set_str("1.5m");
    mu_check(args_read_duration(args_string, &value, NULL));
    mu_assert_int_eq(value, (uint32_t)(1.5 * one_m));
    furi_string_free(args_string);

    // Check for h in float
    args_string = furi_string_alloc_set_str("1.5h");
    mu_check(args_read_duration(args_string, &value, NULL));
    mu_assert_int_eq(value, (uint32_t)(1.5 * one_h));
    furi_string_free(args_string);
}

MU_TEST(args_read_duration_errors_test) {
    FuriString* args_string;
    uint32_t value = 0;

    // Check wrong suffix
    args_string = furi_string_alloc_set_str("1x");
    mu_check(!args_read_duration(args_string, &value, NULL));
    mu_assert_int_eq(value, 0);
    furi_string_free(args_string);

    // Check wrong suffix
    args_string = furi_string_alloc_set_str("1xs");
    mu_check(!args_read_duration(args_string, &value, NULL));
    mu_assert_int_eq(value, 0);
    furi_string_free(args_string);

    // Check negative value
    args_string = furi_string_alloc_set_str("-1s");
    mu_check(!args_read_duration(args_string, &value, NULL));
    mu_assert_int_eq(value, 0);
    furi_string_free(args_string);

    // Check wrong values

    // Check only suffix
    args_string = furi_string_alloc_set_str("s");
    mu_check(!args_read_duration(args_string, &value, NULL));
    mu_assert_int_eq(value, 0);
    furi_string_free(args_string);

    // Check doubled point
    args_string = furi_string_alloc_set_str("0.1.1");
    mu_check(!args_read_duration(args_string, &value, NULL));
    mu_assert_int_eq(value, 0);
    furi_string_free(args_string);

    // Check overflow values

    // Check for ms
    args_string = furi_string_alloc_set_str("4294967296ms");
    mu_check(!args_read_duration(args_string, &value, NULL));
    mu_assert_int_eq(value, 0);
    furi_string_free(args_string);

    // Check for s
    args_string = furi_string_alloc_set_str("4294968s");
    mu_check(!args_read_duration(args_string, &value, NULL));
    mu_assert_int_eq(value, 0);
    furi_string_free(args_string);

    // Check for m
    args_string = furi_string_alloc_set_str("71583m");
    mu_check(!args_read_duration(args_string, &value, NULL));
    mu_assert_int_eq(value, 0);
    furi_string_free(args_string);

    // Check for h
    args_string = furi_string_alloc_set_str("1194h");
    mu_check(!args_read_duration(args_string, &value, NULL));
    mu_assert_int_eq(value, 0);
    furi_string_free(args_string);
}

MU_TEST_SUITE(toolbox_args_read_duration_suite) {
    MU_RUN_TEST(args_read_duration_default_values_test);
    MU_RUN_TEST(args_read_duration_suffix_values_test);
    MU_RUN_TEST(args_read_duration_values_test);
    MU_RUN_TEST(args_read_duration_errors_test);
}

int run_minunit_test_toolbox_args(void) {
    MU_RUN_SUITE(toolbox_args_read_duration_suite);
    return MU_EXIT_CODE;
}

TEST_API_DEFINE(run_minunit_test_toolbox_args)
