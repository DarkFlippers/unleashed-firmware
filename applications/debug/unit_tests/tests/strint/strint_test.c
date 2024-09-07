#include <furi.h>
#include <furi_hal.h>

#include "../test.h" // IWYU pragma: keep

#include <toolbox/strint.h>

MU_TEST(strint_test_basic) {
    uint32_t result = 0;
    mu_assert_int_eq(StrintParseNoError, strint_to_uint32("123456", NULL, &result, 10));
    mu_assert_int_eq(123456, result);
}

MU_TEST(strint_test_junk) {
    uint32_t result = 0;
    mu_assert_int_eq(StrintParseNoError, strint_to_uint32("   123456   ", NULL, &result, 10));
    mu_assert_int_eq(123456, result);
    mu_assert_int_eq(
        StrintParseNoError, strint_to_uint32("   \r\n\r\n   123456     ", NULL, &result, 10));
    mu_assert_int_eq(123456, result);
}

MU_TEST(strint_test_tail) {
    uint32_t result = 0;
    char* tail;
    mu_assert_int_eq(StrintParseNoError, strint_to_uint32("123456tail", &tail, &result, 10));
    mu_assert_int_eq(123456, result);
    mu_assert_string_eq("tail", tail);
    mu_assert_int_eq(
        StrintParseNoError, strint_to_uint32("   \r\n  123456tail", &tail, &result, 10));
    mu_assert_int_eq(123456, result);
    mu_assert_string_eq("tail", tail);
}

MU_TEST(strint_test_errors) {
    uint32_t result = 123;
    mu_assert_int_eq(StrintParseAbsentError, strint_to_uint32("", NULL, &result, 10));
    mu_assert_int_eq(123, result);
    mu_assert_int_eq(StrintParseAbsentError, strint_to_uint32("   asd\r\n", NULL, &result, 10));
    mu_assert_int_eq(123, result);
    mu_assert_int_eq(StrintParseSignError, strint_to_uint32("+++123456", NULL, &result, 10));
    mu_assert_int_eq(123, result);
    mu_assert_int_eq(StrintParseSignError, strint_to_uint32("-1", NULL, &result, 10));
    mu_assert_int_eq(123, result);
    mu_assert_int_eq(
        StrintParseOverflowError,
        strint_to_uint32("0xAAAAAAAAAAAAAAAADEADBEEF!!!!!!", NULL, &result, 0));
    mu_assert_int_eq(123, result);
    mu_assert_int_eq(StrintParseOverflowError, strint_to_uint32("4294967296", NULL, &result, 0));
    mu_assert_int_eq(123, result);

    int32_t result_i32 = 123;
    mu_assert_int_eq(
        StrintParseOverflowError, strint_to_int32("-2147483649", NULL, &result_i32, 0));
    mu_assert_int_eq(123, result_i32);
}

MU_TEST(strint_test_bases) {
    uint32_t result = 0;

    mu_assert_int_eq(StrintParseNoError, strint_to_uint32("0x123", NULL, &result, 0));
    mu_assert_int_eq(0x123, result);
    mu_assert_int_eq(StrintParseNoError, strint_to_uint32("0X123", NULL, &result, 0));
    mu_assert_int_eq(0x123, result);
    mu_assert_int_eq(StrintParseNoError, strint_to_uint32("0xDEADBEEF", NULL, &result, 0));
    mu_assert_int_eq(0xDEADBEEF, result);
    mu_assert_int_eq(StrintParseNoError, strint_to_uint32("0xDEADBEEF", NULL, &result, 16));
    mu_assert_int_eq(0xDEADBEEF, result);
    mu_assert_int_eq(StrintParseNoError, strint_to_uint32("123", NULL, &result, 16));
    mu_assert_int_eq(0x123, result);

    mu_assert_int_eq(StrintParseNoError, strint_to_uint32("123", NULL, &result, 0));
    mu_assert_int_eq(123, result);

    mu_assert_int_eq(StrintParseNoError, strint_to_uint32("0123", NULL, &result, 0));
    mu_assert_int_eq(0123, result);
    mu_assert_int_eq(StrintParseNoError, strint_to_uint32("0123", NULL, &result, 8));
    mu_assert_int_eq(0123, result);
    mu_assert_int_eq(StrintParseNoError, strint_to_uint32("123", NULL, &result, 8));
    mu_assert_int_eq(0123, result);

    mu_assert_int_eq(StrintParseNoError, strint_to_uint32("0b101", NULL, &result, 0));
    mu_assert_int_eq(0b101, result);
    mu_assert_int_eq(StrintParseNoError, strint_to_uint32("0b101", NULL, &result, 2));
    mu_assert_int_eq(0b101, result);
    mu_assert_int_eq(StrintParseNoError, strint_to_uint32("0B101", NULL, &result, 0));
    mu_assert_int_eq(0b101, result);
    mu_assert_int_eq(StrintParseNoError, strint_to_uint32("101", NULL, &result, 2));
    mu_assert_int_eq(0b101, result);
}

MU_TEST_SUITE(strint_test_limits) {
    uint64_t result_u64 = 0;
    mu_assert_int_eq(
        StrintParseNoError, strint_to_uint64("18446744073709551615", NULL, &result_u64, 0));
    // `mu_assert_int_eq' does not support longs :(
    mu_assert(UINT64_MAX == result_u64, "result does not equal UINT64_MAX");

    int64_t result_i64 = 0;
    mu_assert_int_eq(
        StrintParseNoError, strint_to_int64("9223372036854775807", NULL, &result_i64, 0));
    mu_assert(INT64_MAX == result_i64, "result does not equal INT64_MAX");
    mu_assert_int_eq(
        StrintParseNoError, strint_to_int64("-9223372036854775808", NULL, &result_i64, 0));
    mu_assert(INT64_MIN == result_i64, "result does not equal INT64_MIN");

    uint32_t result_u32 = 0;
    mu_assert_int_eq(StrintParseNoError, strint_to_uint32("4294967295", NULL, &result_u32, 0));
    mu_assert_int_eq(UINT32_MAX, result_u32);

    int32_t result_i32 = 0;
    mu_assert_int_eq(StrintParseNoError, strint_to_int32("2147483647", NULL, &result_i32, 0));
    mu_assert_int_eq(INT32_MAX, result_i32);
    mu_assert_int_eq(StrintParseNoError, strint_to_int32("-2147483648", NULL, &result_i32, 0));
    mu_assert_int_eq(INT32_MIN, result_i32);

    uint16_t result_u16 = 0;
    mu_assert_int_eq(StrintParseNoError, strint_to_uint16("65535", NULL, &result_u16, 0));
    mu_assert_int_eq(UINT16_MAX, result_u16);

    int16_t result_i16 = 0;
    mu_assert_int_eq(StrintParseNoError, strint_to_int16("32767", NULL, &result_i16, 0));
    mu_assert_int_eq(INT16_MAX, result_i16);
    mu_assert_int_eq(StrintParseNoError, strint_to_int16("-32768", NULL, &result_i16, 0));
    mu_assert_int_eq(INT16_MIN, result_i16);
}

MU_TEST_SUITE(test_strint_suite) {
    MU_RUN_TEST(strint_test_basic);
    MU_RUN_TEST(strint_test_junk);
    MU_RUN_TEST(strint_test_tail);
    MU_RUN_TEST(strint_test_errors);
    MU_RUN_TEST(strint_test_bases);
    MU_RUN_TEST(strint_test_limits);
}

int run_minunit_test_strint(void) {
    MU_RUN_SUITE(test_strint_suite);
    return MU_EXIT_CODE;
}

TEST_API_DEFINE(run_minunit_test_strint)
