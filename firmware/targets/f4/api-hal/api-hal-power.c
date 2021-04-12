#include <api-hal-power.h>
#include <api-hal-clock.h>
#include <api-hal-bt.h>

#include <stm32wbxx_ll_rcc.h>
#include <stm32wbxx_ll_pwr.h>
#include <stm32wbxx_ll_hsem.h>
#include <stm32wbxx_ll_cortex.h>

#include <main.h>
#include <hw_conf.h>
#include <bq27220.h>
#include <bq25896.h>

typedef struct {
    volatile uint32_t insomnia;
    volatile uint32_t deep_insomnia;
} ApiHalPower;

static volatile ApiHalPower api_hal_power = {
    .insomnia = 0,
    .deep_insomnia = 1,
};

const ParamCEDV cedv = {
    .full_charge_cap = 2100,
    .design_cap = 2100,
    .EMF = 3739,
    .C0 = 776,
    .C1 = 0,
    .R1 = 193,
    .R0 = 1,
    .T0 = 1,
    .TC = 11,
    .DOD0 = 4044,
    .DOD10 = 3899,
    .DOD20 = 3796,
    .DOD30 = 3704,
    .DOD40 = 3627,
    .DOD50 = 3573,
    .DOD60 = 3535,
    .DOD70 = 3501,
    .DOD80 = 3453,
    .DOD90 = 3366,
    .DOD100 = 2419,
};

void HAL_RCC_CSSCallback(void) {
    // TODO: notify user about issue with HSE
    NVIC_SystemReset();
}

void api_hal_power_init() {
    LL_PWR_SMPS_SetMode(LL_PWR_SMPS_STEP_DOWN);
    bq27220_init(&cedv);
    bq25896_init();
}

uint16_t api_hal_power_insomnia_level() {
    return api_hal_power.insomnia;
}

void api_hal_power_insomnia_enter() {
    api_hal_power.insomnia++;
}

void api_hal_power_insomnia_exit() {
    api_hal_power.insomnia--;
}

bool api_hal_power_sleep_available() {
    return api_hal_power.insomnia == 0;
}

bool api_hal_power_deep_sleep_available() {
    return api_hal_bt_is_alive() && api_hal_power.deep_insomnia == 0;
}

void api_hal_power_light_sleep() {
    __WFI();
}

void api_hal_power_deep_sleep() {
  while( LL_HSEM_1StepLock(HSEM, CFG_HW_RCC_SEMID));

  if (!LL_HSEM_1StepLock(HSEM, CFG_HW_ENTRY_STOP_MODE_SEMID)) {
        if(LL_PWR_IsActiveFlag_C2DS()) {
            // Release ENTRY_STOP_MODE semaphore
            LL_HSEM_ReleaseLock(HSEM, CFG_HW_ENTRY_STOP_MODE_SEMID, 0);

            // The switch on HSI before entering Stop Mode is required 
            api_hal_clock_switch_to_hsi();
        }
    } else {
        /**
         * The switch on HSI before entering Stop Mode is required 
         */
        api_hal_clock_switch_to_hsi();
    }

    /* Release RCC semaphore */
    LL_HSEM_ReleaseLock(HSEM, CFG_HW_RCC_SEMID, 0);

    // Prepare deep sleep
    LL_PWR_SetPowerMode(LL_PWR_MODE_STOP1);
    LL_LPM_EnableDeepSleep();

#if defined ( __CC_ARM)
    // Force store operations
    __force_stores();
#endif

    __WFI();

    /* Release ENTRY_STOP_MODE semaphore */
    LL_HSEM_ReleaseLock(HSEM, CFG_HW_ENTRY_STOP_MODE_SEMID, 0);

    while(LL_HSEM_1StepLock(HSEM, CFG_HW_RCC_SEMID));

    if(LL_RCC_GetSysClkSource() != LL_RCC_SYS_CLKSOURCE_STATUS_PLL) {
        api_hal_clock_switch_to_pll();
    }

    LL_HSEM_ReleaseLock(HSEM, CFG_HW_RCC_SEMID, 0);
}

void api_hal_power_sleep() {
    if(api_hal_power_deep_sleep_available()) {
        api_hal_power_deep_sleep();
    } else {
        api_hal_power_light_sleep();
    }
}


uint8_t api_hal_power_get_pct() {
    return bq27220_get_state_of_charge();
}

uint8_t api_hal_power_get_bat_health_pct() {
    return bq27220_get_state_of_health();
}

bool api_hal_power_is_charging() {
    return bq25896_is_charging();
}

void api_hal_power_off() {
    bq25896_poweroff();
}

void api_hal_power_enable_otg() {
    bq25896_enable_otg();
}

void api_hal_power_disable_otg() {
    bq25896_disable_otg();
}

uint32_t api_hal_power_get_battery_remaining_capacity() {
    return bq27220_get_remaining_capacity();
}

uint32_t api_hal_power_get_battery_full_capacity() {
    return bq27220_get_full_charge_capacity();
}

float api_hal_power_get_battery_voltage(ApiHalPowerIC ic) {
    if (ic == ApiHalPowerICCharger) {
        return (float)bq25896_get_vbat_voltage() / 1000.0f;
    } else if (ic == ApiHalPowerICFuelGauge) {
        return (float)bq27220_get_voltage() / 1000.0f;
    } else {
        return 0.0f;
    }
}

float api_hal_power_get_battery_current(ApiHalPowerIC ic) {
    if (ic == ApiHalPowerICCharger) {
        return (float)bq25896_get_vbat_current() / 1000.0f;
    } else if (ic == ApiHalPowerICFuelGauge) {
        return (float)bq27220_get_current() / 1000.0f;
    } else {
        return 0.0f;
    }
}

float api_hal_power_get_battery_temperature(ApiHalPowerIC ic) {
    if (ic == ApiHalPowerICCharger) {
        // Linear approximation, +/- 5 C
        return (71.0f - (float)bq25896_get_ntc_mpct()/1000) / 0.6f;
    } else if (ic == ApiHalPowerICFuelGauge) {
        return ((float)bq27220_get_temperature() - 2731.0f) / 10.0f;
    } else {
        return 0.0f;
    }
    
}

float api_hal_power_get_usb_voltage(){
    return (float)bq25896_get_vbus_voltage() / 1000.0f;
}

void api_hal_power_dump_state() {
    BatteryStatus battery_status;
    OperationStatus operation_status;
    if (bq27220_get_battery_status(&battery_status) == BQ27220_ERROR
        || bq27220_get_operation_status(&operation_status) == BQ27220_ERROR) {
        printf("Failed to get bq27220 status. Communication error.\r\n");
    } else {
        printf(
           "bq27220: CALMD: %d, SEC0: %d, SEC1: %d, EDV2: %d, VDQ: %d, INITCOMP: %d, SMTH: %d, BTPINT: %d, CFGUPDATE: %d\r\n",
            operation_status.CALMD, operation_status.SEC0, operation_status.SEC1,
            operation_status.EDV2, operation_status.VDQ, operation_status.INITCOMP,
            operation_status.SMTH, operation_status.BTPINT, operation_status.CFGUPDATE
        );
        // Battery status register, part 1
        printf(
           "bq27220: CHGINH: %d, FC: %d, OTD: %d, OTC: %d, SLEEP: %d, OCVFAIL: %d, OCVCOMP: %d, FD: %d\r\n",
            battery_status.CHGINH, battery_status.FC, battery_status.OTD,
            battery_status.OTC, battery_status.SLEEP, battery_status.OCVFAIL,
            battery_status.OCVCOMP, battery_status.FD
        );
        // Battery status register, part 2
        printf(
           "bq27220: DSG: %d, SYSDWN: %d, TDA: %d, BATTPRES: %d, AUTH_GD: %d, OCVGD: %d, TCA: %d, RSVD: %d\r\n",
            battery_status.DSG, battery_status.SYSDWN, battery_status.TDA,
            battery_status.BATTPRES, battery_status.AUTH_GD, battery_status.OCVGD,
            battery_status.TCA, battery_status.RSVD
        );
        // Voltage and current info
        printf(
            "bq27220: Full capacity: %dmAh, Design capacity: %dmAh, Remaining capacity: %dmAh, State of Charge: %d%%, State of health: %d%%\r\n",
            bq27220_get_full_charge_capacity(), bq27220_get_design_capacity(), bq27220_get_remaining_capacity(),
            bq27220_get_state_of_charge(), bq27220_get_state_of_health()
        );
        printf(
            "bq27220: Voltage: %dmV, Current: %dmA, Temperature: %dC\r\n",
            bq27220_get_voltage(), bq27220_get_current(), (int)api_hal_power_get_battery_temperature(ApiHalPowerICFuelGauge)
        );
    }

    printf(
        "bq25896: VBUS: %d, VSYS: %d, VBAT: %d, Current: %d, NTC: %ldm%%\r\n",
        bq25896_get_vbus_voltage(), bq25896_get_vsys_voltage(),
        bq25896_get_vbat_voltage(), bq25896_get_vbat_current(),
        bq25896_get_ntc_mpct()
    );
}

void api_hal_power_enable_external_3_3v(){
}

void api_hal_power_disable_external_3_3v(){
}
