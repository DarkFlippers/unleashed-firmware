#include <float.h>
#include <float_tools.h>

#include "../test.h" // IWYU pragma: keep

MU_TEST(float_tools_equal_test) {
    mu_check(float_is_equal(FLT_MAX, FLT_MAX));
    mu_check(float_is_equal(FLT_MIN, FLT_MIN));
    mu_check(float_is_equal(-FLT_MAX, -FLT_MAX));
    mu_check(float_is_equal(-FLT_MIN, -FLT_MIN));

    mu_check(!float_is_equal(FLT_MIN, FLT_MAX));
    mu_check(!float_is_equal(-FLT_MIN, FLT_MAX));
    mu_check(!float_is_equal(FLT_MIN, -FLT_MAX));
    mu_check(!float_is_equal(-FLT_MIN, -FLT_MAX));

    const float pi = 3.14159f;
    mu_check(float_is_equal(pi, pi));
    mu_check(float_is_equal(-pi, -pi));
    mu_check(!float_is_equal(pi, -pi));
    mu_check(!float_is_equal(-pi, pi));

    const float one_third = 1.f / 3.f;
    const float one_third_dec = 0.3333333f;
    mu_check(one_third != one_third_dec);
    mu_check(float_is_equal(one_third, one_third_dec));

    const float big_num = 1.e12f;
    const float med_num = 95.389f;
    const float smol_num = 1.e-12f;
    mu_check(float_is_equal(big_num, big_num));
    mu_check(float_is_equal(med_num, med_num));
    mu_check(float_is_equal(smol_num, smol_num));
    mu_check(!float_is_equal(smol_num, big_num));
    mu_check(!float_is_equal(med_num, smol_num));
    mu_check(!float_is_equal(big_num, med_num));

    const float more_than_one = 1.f + FLT_EPSILON;
    const float less_than_one = 1.f - FLT_EPSILON;
    mu_check(!float_is_equal(more_than_one, less_than_one));
    mu_check(!float_is_equal(more_than_one, -less_than_one));
    mu_check(!float_is_equal(-more_than_one, less_than_one));
    mu_check(!float_is_equal(-more_than_one, -less_than_one));

    const float slightly_more_than_one = 1.f + FLT_EPSILON / 2.f;
    const float slightly_less_than_one = 1.f - FLT_EPSILON / 2.f;
    mu_check(float_is_equal(slightly_more_than_one, slightly_less_than_one));
    mu_check(float_is_equal(-slightly_more_than_one, -slightly_less_than_one));
    mu_check(!float_is_equal(slightly_more_than_one, -slightly_less_than_one));
    mu_check(!float_is_equal(-slightly_more_than_one, slightly_less_than_one));
}

MU_TEST_SUITE(float_tools_suite) {
    MU_RUN_TEST(float_tools_equal_test);
}

int run_minunit_test_float_tools(void) {
    MU_RUN_SUITE(float_tools_suite);
    return MU_EXIT_CODE;
}

TEST_API_DEFINE(run_minunit_test_float_tools)
