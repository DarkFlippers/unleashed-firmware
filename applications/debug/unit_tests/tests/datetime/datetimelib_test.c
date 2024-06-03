#include <furi.h>
#include "../test.h" // IWYU pragma: keep

#include <datetime/datetime.h>

MU_TEST(test_datetime_validate_datetime_correct_min) {
    DateTime correct_min = {0, 0, 0, 1, 1, 2000, 1};
    bool result = datetime_validate_datetime(&correct_min);

    mu_assert_int_eq(true, result);
}

MU_TEST(test_datetime_validate_datetime_correct_max) {
    DateTime correct_max = {23, 59, 59, 31, 12, 2099, 7};
    bool result = datetime_validate_datetime(&correct_max);

    mu_assert_int_eq(true, result);
}

MU_TEST(test_datetime_validate_datetime_incorrect_second) {
    DateTime incorrect_sec = {0, 0, 60, 1, 1, 2000, 1};
    bool result = datetime_validate_datetime(&incorrect_sec);

    mu_assert_int_eq(false, result);
}

MU_TEST(test_datetime_validate_datetime_incorrect_minute) {
    DateTime incorrect_min = {0, 60, 0, 1, 1, 2000, 1};
    bool result = datetime_validate_datetime(&incorrect_min);

    mu_assert_int_eq(false, result);
}

MU_TEST(test_datetime_validate_datetime_incorrect_hour) {
    DateTime incorrect_hour = {24, 0, 0, 1, 1, 2000, 1};
    bool result = datetime_validate_datetime(&incorrect_hour);

    mu_assert_int_eq(false, result);
}

MU_TEST(test_datetime_validate_datetime_incorrect_day_min) {
    DateTime incorrect_day_min = {0, 0, 0, 0, 1, 2000, 1};
    bool result = datetime_validate_datetime(&incorrect_day_min);

    mu_assert_int_eq(false, result);
}

MU_TEST(test_datetime_validate_datetime_incorrect_day_max) {
    DateTime incorrect_day_max = {0, 0, 0, 32, 1, 2000, 1};
    bool result = datetime_validate_datetime(&incorrect_day_max);

    mu_assert_int_eq(false, result);
}

MU_TEST(test_datetime_validate_datetime_incorrect_month_min) {
    DateTime incorrect_month_min = {0, 0, 0, 1, 0, 2000, 1};
    bool result = datetime_validate_datetime(&incorrect_month_min);

    mu_assert_int_eq(false, result);
}

MU_TEST(test_datetime_validate_datetime_incorrect_month_max) {
    DateTime incorrect_month_max = {0, 0, 0, 1, 13, 2000, 1};
    bool result = datetime_validate_datetime(&incorrect_month_max);

    mu_assert_int_eq(false, result);
}

MU_TEST(test_datetime_validate_datetime_incorrect_year_min) {
    DateTime incorrect_year_min = {0, 0, 0, 1, 1, 1999, 1};
    bool result = datetime_validate_datetime(&incorrect_year_min);

    mu_assert_int_eq(false, result);
}

MU_TEST(test_datetime_validate_datetime_incorrect_year_max) {
    DateTime incorrect_year_max = {0, 0, 0, 1, 1, 2100, 1};
    bool result = datetime_validate_datetime(&incorrect_year_max);

    mu_assert_int_eq(false, result);
}

MU_TEST(test_datetime_validate_datetime_incorrect_weekday_min) {
    DateTime incorrect_weekday_min = {0, 0, 0, 1, 1, 2000, 0};
    bool result = datetime_validate_datetime(&incorrect_weekday_min);

    mu_assert_int_eq(false, result);
}

MU_TEST(test_datetime_validate_datetime_incorrect_weekday_max) {
    DateTime incorrect_weekday_max = {0, 0, 0, 1, 1, 2000, 8};
    bool result = datetime_validate_datetime(&incorrect_weekday_max);

    mu_assert_int_eq(false, result);
}

MU_TEST_SUITE(test_datetime_validate_datetime) {
    MU_RUN_TEST(test_datetime_validate_datetime_correct_min);
    MU_RUN_TEST(test_datetime_validate_datetime_correct_max);
    MU_RUN_TEST(test_datetime_validate_datetime_incorrect_second);
    MU_RUN_TEST(test_datetime_validate_datetime_incorrect_minute);
    MU_RUN_TEST(test_datetime_validate_datetime_incorrect_hour);
    MU_RUN_TEST(test_datetime_validate_datetime_incorrect_day_min);
    MU_RUN_TEST(test_datetime_validate_datetime_incorrect_day_max);
    MU_RUN_TEST(test_datetime_validate_datetime_incorrect_month_min);
    MU_RUN_TEST(test_datetime_validate_datetime_incorrect_month_max);
    MU_RUN_TEST(test_datetime_validate_datetime_incorrect_year_min);
    MU_RUN_TEST(test_datetime_validate_datetime_incorrect_year_max);
    MU_RUN_TEST(test_datetime_validate_datetime_incorrect_weekday_min);
    MU_RUN_TEST(test_datetime_validate_datetime_incorrect_weekday_max);
}

MU_TEST(test_datetime_timestamp_to_datetime_min) {
    uint32_t test_value = 0;
    DateTime min_datetime_expected = {0, 0, 0, 1, 1, 1970, 4};

    DateTime result = {0};
    datetime_timestamp_to_datetime(test_value, &result);

    mu_assert_mem_eq(&min_datetime_expected, &result, sizeof(result));
}

MU_TEST(test_datetime_timestamp_to_datetime_max) {
    uint32_t test_value = UINT32_MAX;
    DateTime max_datetime_expected = {6, 28, 15, 7, 2, 2106, 7};

    DateTime result = {0};
    datetime_timestamp_to_datetime(test_value, &result);

    mu_assert_mem_eq(&max_datetime_expected, &result, sizeof(result));
}

MU_TEST(test_datetime_timestamp_to_datetime_to_timestamp) {
    uint32_t test_value = random();

    DateTime datetime = {0};
    datetime_timestamp_to_datetime(test_value, &datetime);

    uint32_t result = datetime_datetime_to_timestamp(&datetime);

    mu_assert_int_eq(test_value, result);
}

MU_TEST(test_datetime_timestamp_to_datetime_weekday) {
    uint32_t test_value = 1709748421; // Wed Mar 06 18:07:01 2024 UTC

    DateTime datetime = {0};
    datetime_timestamp_to_datetime(test_value, &datetime);

    mu_assert_int_eq(datetime.hour, 18);
    mu_assert_int_eq(datetime.minute, 7);
    mu_assert_int_eq(datetime.second, 1);
    mu_assert_int_eq(datetime.day, 6);
    mu_assert_int_eq(datetime.month, 3);
    mu_assert_int_eq(datetime.weekday, 3);
    mu_assert_int_eq(datetime.year, 2024);
}

MU_TEST_SUITE(test_datetime_timestamp_to_datetime_suite) {
    MU_RUN_TEST(test_datetime_timestamp_to_datetime_min);
    MU_RUN_TEST(test_datetime_timestamp_to_datetime_max);
    MU_RUN_TEST(test_datetime_timestamp_to_datetime_to_timestamp);
    MU_RUN_TEST(test_datetime_timestamp_to_datetime_weekday);
}

MU_TEST(test_datetime_datetime_to_timestamp_min) {
    DateTime min_datetime = {0, 0, 0, 1, 1, 1970, 0};

    uint32_t result = datetime_datetime_to_timestamp(&min_datetime);
    mu_assert_int_eq(0, result);
}

MU_TEST(test_datetime_datetime_to_timestamp_max) {
    DateTime max_datetime = {6, 28, 15, 7, 2, 2106, 0};

    uint32_t result = datetime_datetime_to_timestamp(&max_datetime);
    mu_assert_int_eq(UINT32_MAX, result);
}

MU_TEST_SUITE(test_datetime_datetime_to_timestamp_suite) {
    MU_RUN_TEST(test_datetime_datetime_to_timestamp_min);
    MU_RUN_TEST(test_datetime_datetime_to_timestamp_max);
}

int run_minunit_test_datetime(void) {
    MU_RUN_SUITE(test_datetime_timestamp_to_datetime_suite);
    MU_RUN_SUITE(test_datetime_datetime_to_timestamp_suite);
    MU_RUN_SUITE(test_datetime_validate_datetime);

    return MU_EXIT_CODE;
}

TEST_API_DEFINE(run_minunit_test_datetime)
