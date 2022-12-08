#include <furi.h>
#include <furi_hal.h>
#include "../minunit.h"

static void power_test_deinit(void) {
    // Try to reset to default charging voltage
    furi_hal_power_set_battery_charging_voltage(4.208f);
}

MU_TEST(test_power_charge_voltage_exact) {
    // Power of 16mV charge voltages get applied exactly
    // (bq25896 charge controller works in 16mV increments)
    //
    // This test may need adapted if other charge controllers are used in the future.
    for(uint16_t charge_mv = 3840; charge_mv <= 4208; charge_mv += 16) {
        float charge_volt = (float)charge_mv / 1000.0f;
        furi_hal_power_set_battery_charging_voltage(charge_volt);
        mu_assert_double_eq(charge_volt, furi_hal_power_get_battery_charging_voltage());
    }
}

MU_TEST(test_power_charge_voltage_floating_imprecision) {
    // 4.016f should act as 4.016 V, even with floating point imprecision
    furi_hal_power_set_battery_charging_voltage(4.016f);
    mu_assert_double_eq(4.016f, furi_hal_power_get_battery_charging_voltage());
}

MU_TEST(test_power_charge_voltage_inexact) {
    // Charge voltages that are not power of 16mV get truncated down
    furi_hal_power_set_battery_charging_voltage(3.841f);
    mu_assert_double_eq(3.840, furi_hal_power_get_battery_charging_voltage());

    furi_hal_power_set_battery_charging_voltage(3.900f);
    mu_assert_double_eq(3.888, furi_hal_power_get_battery_charging_voltage());

    furi_hal_power_set_battery_charging_voltage(4.200f);
    mu_assert_double_eq(4.192, furi_hal_power_get_battery_charging_voltage());
}

MU_TEST(test_power_charge_voltage_invalid_clamped) {
    // Out-of-range charge voltages get clamped to 3.840 V and 4.208 V
    furi_hal_power_set_battery_charging_voltage(3.808f);
    mu_assert_double_eq(3.840, furi_hal_power_get_battery_charging_voltage());

    // NOTE: Intentionally picking a small increment above 4.208 V to reduce the risk of an
    // unhappy battery if this fails.
    furi_hal_power_set_battery_charging_voltage(4.240f);
    mu_assert_double_eq(4.208, furi_hal_power_get_battery_charging_voltage());
}

MU_TEST_SUITE(test_power_suite) {
    MU_RUN_TEST(test_power_charge_voltage_exact);
    MU_RUN_TEST(test_power_charge_voltage_floating_imprecision);
    MU_RUN_TEST(test_power_charge_voltage_inexact);
    MU_RUN_TEST(test_power_charge_voltage_invalid_clamped);
    power_test_deinit();
}

int run_minunit_test_power() {
    MU_RUN_SUITE(test_power_suite);
    return MU_EXIT_CODE;
}
