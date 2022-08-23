#include <furi.h>
#include <furi_hal.h>
#include "../minunit.h"
#include <toolbox/varint.h>
#include <toolbox/profiler.h>

MU_TEST(test_varint_basic_u) {
    mu_assert_int_eq(1, varint_uint32_length(0));
    mu_assert_int_eq(5, varint_uint32_length(UINT32_MAX));

    uint8_t data[8] = {};
    uint32_t out_value;

    mu_assert_int_eq(1, varint_uint32_pack(0, data));
    mu_assert_int_eq(1, varint_uint32_unpack(&out_value, data, 8));
    mu_assert_int_eq(0, out_value);

    mu_assert_int_eq(5, varint_uint32_pack(UINT32_MAX, data));
    mu_assert_int_eq(5, varint_uint32_unpack(&out_value, data, 8));
    mu_assert_int_eq(UINT32_MAX, out_value);
}

MU_TEST(test_varint_basic_i) {
    mu_assert_int_eq(5, varint_int32_length(INT32_MIN / 2));
    mu_assert_int_eq(1, varint_int32_length(0));
    mu_assert_int_eq(5, varint_int32_length(INT32_MAX / 2));

    mu_assert_int_eq(2, varint_int32_length(127));
    mu_assert_int_eq(2, varint_int32_length(-127));

    uint8_t data[8] = {};
    int32_t out_value;
    mu_assert_int_eq(1, varint_int32_pack(0, data));
    mu_assert_int_eq(1, varint_int32_unpack(&out_value, data, 8));
    mu_assert_int_eq(0, out_value);

    mu_assert_int_eq(2, varint_int32_pack(127, data));
    mu_assert_int_eq(2, varint_int32_unpack(&out_value, data, 8));
    mu_assert_int_eq(127, out_value);

    mu_assert_int_eq(2, varint_int32_pack(-127, data));
    mu_assert_int_eq(2, varint_int32_unpack(&out_value, data, 8));
    mu_assert_int_eq(-127, out_value);

    mu_assert_int_eq(5, varint_int32_pack(INT32_MAX, data));
    mu_assert_int_eq(5, varint_int32_unpack(&out_value, data, 8));
    mu_assert_int_eq(INT32_MAX, out_value);

    mu_assert_int_eq(5, varint_int32_pack(INT32_MIN / 2 + 1, data));
    mu_assert_int_eq(5, varint_int32_unpack(&out_value, data, 8));
    mu_assert_int_eq(INT32_MIN / 2 + 1, out_value);
}

MU_TEST(test_varint_rand_u) {
    uint8_t data[8] = {};
    uint32_t out_value;

    for(size_t i = 0; i < 200000; i++) {
        uint32_t rand_value = rand();
        mu_assert_int_eq(
            varint_uint32_pack(rand_value, data), varint_uint32_unpack(&out_value, data, 8));
        mu_assert_int_eq(rand_value, out_value);
    }
}

MU_TEST(test_varint_rand_i) {
    uint8_t data[8] = {};
    int32_t out_value;

    for(size_t i = 0; i < 200000; i++) {
        int32_t rand_value = rand() + (INT32_MIN / 2 + 1);
        mu_assert_int_eq(
            varint_int32_pack(rand_value, data), varint_int32_unpack(&out_value, data, 8));
        mu_assert_int_eq(rand_value, out_value);
    }
}

MU_TEST_SUITE(test_varint_suite) {
    MU_RUN_TEST(test_varint_basic_u);
    MU_RUN_TEST(test_varint_basic_i);
    MU_RUN_TEST(test_varint_rand_u);
    MU_RUN_TEST(test_varint_rand_i);
}

int run_minunit_test_varint() {
    MU_RUN_SUITE(test_varint_suite);
    return MU_EXIT_CODE;
}