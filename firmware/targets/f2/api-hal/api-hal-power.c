#include <api-hal-power.h>
#include <adc.h>
#include <math.h>

#define BATTERY_MIN_VOLTAGE 3.4f
#define BATTERY_MAX_VOLTAGE 4.1f

void api_hal_power_init() {}

uint8_t api_hal_power_get_pct() {
    float value = api_hal_power_get_battery_voltage();

    if (value == NAN || value < BATTERY_MIN_VOLTAGE) {
        return 0;
    }

    value = (value - BATTERY_MIN_VOLTAGE) / (BATTERY_MAX_VOLTAGE - BATTERY_MIN_VOLTAGE) * 100;

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

float api_hal_power_get_battery_voltage() {
    ADC_ChannelConfTypeDef sConfig = {0};
    sConfig.Channel = ADC_CHANNEL_4;
    sConfig.Rank = ADC_REGULAR_RANK_1;
    sConfig.SamplingTime = ADC_SAMPLETIME_2CYCLES_5;
    sConfig.SingleDiff = ADC_SINGLE_ENDED;
    sConfig.OffsetNumber = ADC_OFFSET_NONE;
    sConfig.Offset = 0;
    if (HAL_ADC_ConfigChannel(&hadc1, &sConfig) != HAL_OK) {
        Error_Handler();
    }

    float value = NAN;
    HAL_ADC_Start(&hadc1);
    if(HAL_ADC_PollForConversion(&hadc1, 1000) != HAL_TIMEOUT) {
        // adc range / 12 bits * adc_value * divider ratio * sampling drag correction
        value = 3.3f / 4096.0f * HAL_ADC_GetValue(&hadc1) * 2 * 1.3;
    }

    return value;
}

float api_hal_power_get_battery_current() {
    return NAN;
}

void api_hal_power_dump_state(string_t buffer) {
    string_cat_printf(buffer, "Not supported");
}
