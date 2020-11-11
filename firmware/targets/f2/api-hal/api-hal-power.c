#include <api-hal-power.h>
#include <adc.h>

#define BATTERY_MIN_VOLTAGE 3.2f
#define BATTERY_MAX_VOLTAGE 4.0f

void api_hal_power_init() {}

uint8_t api_hal_power_get_pct() {
    float value;
    HAL_ADC_Start(&hadc1);
    if(HAL_ADC_PollForConversion(&hadc1, 1000) != HAL_TIMEOUT) {
        value = HAL_ADC_GetValue(&hadc1);
    }

    value = ((float)value / 10 * 2 - BATTERY_MIN_VOLTAGE) /
                       (BATTERY_MAX_VOLTAGE - BATTERY_MIN_VOLTAGE);

    if(value > 100) {
        value = 100;
    }

    return value;
}

bool api_hal_power_is_charging() {
    return false;
}

void api_hal_power_off() {}

void api_hal_power_enable_otg() {}

void api_hal_power_disable_otg() {}
