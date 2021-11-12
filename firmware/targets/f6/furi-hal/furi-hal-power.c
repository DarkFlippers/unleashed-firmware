#include <furi-hal-power.h>
#include <furi-hal-clock.h>
#include <furi-hal-bt.h>

#include <stm32wbxx_ll_rcc.h>
#include <stm32wbxx_ll_pwr.h>
#include <stm32wbxx_ll_hsem.h>
#include <stm32wbxx_ll_cortex.h>
#include <stm32wbxx_ll_gpio.h>

#include <main.h>
#include <hw_conf.h>
#include <bq27220.h>
#include <bq25896.h>

#include <furi.h>

#define TAG "FuriHalPower"

typedef struct {
    volatile uint8_t insomnia;
    volatile uint8_t deep_insomnia;
    volatile uint8_t suppress_charge;
} FuriHalPower;

static volatile FuriHalPower furi_hal_power = {
    .insomnia = 0,
    .deep_insomnia = 1,
    .suppress_charge = 0,
};

const ParamCEDV cedv = {
    .cedv_conf.gauge_conf = {
        .CCT = 1,
        .CSYNC = 0,
        .EDV_CMP = 0,
        .SC = 1,
        .FIXED_EDV0 = 1,
        .FCC_LIM = 1,
        .FC_FOR_VDQ = 1,
        .IGNORE_SD = 1,
        .SME0 = 0,
    },
    .full_charge_cap = 2100,
    .design_cap = 2100,
    .EDV0 = 3300,
    .EDV1 = 3321,
    .EDV2 = 3355,
    .EMF = 3679,
    .C0 = 430,
    .C1 = 0,
    .R1 = 408,
    .R0 = 334,
    .T0 = 4626,
    .TC = 11,
    .DOD0 = 4044,
    .DOD10 = 3905,
    .DOD20 = 3807,
    .DOD30 = 3718,
    .DOD40 = 3642,
    .DOD50 = 3585,
    .DOD60 = 3546,
    .DOD70 = 3514,
    .DOD80 = 3477,
    .DOD90 = 3411,
    .DOD100 = 3299,
};

void HAL_RCC_CSSCallback(void) {
    // TODO: notify user about issue with HSE
    furi_hal_power_reset();
}

void furi_hal_power_init() {
    LL_PWR_SetRegulVoltageScaling(LL_PWR_REGU_VOLTAGE_SCALE1);
    LL_PWR_SMPS_SetMode(LL_PWR_SMPS_STEP_DOWN);
    bq27220_init(&cedv);
    bq25896_init();
    FURI_LOG_I(TAG, "Init OK");
}

uint16_t furi_hal_power_insomnia_level() {
    return furi_hal_power.insomnia;
}

void furi_hal_power_insomnia_enter() {
    vTaskSuspendAll();
    furi_hal_power.insomnia++;
    xTaskResumeAll();
}

void furi_hal_power_insomnia_exit() {
    vTaskSuspendAll();
    furi_hal_power.insomnia--;
    xTaskResumeAll();
}

bool furi_hal_power_sleep_available() {
    return furi_hal_power.insomnia == 0;
}

bool furi_hal_power_deep_sleep_available() {
    return furi_hal_bt_is_alive() && furi_hal_power.deep_insomnia == 0;
}

void furi_hal_power_light_sleep() {
    __WFI();
}

void furi_hal_power_deep_sleep() {
  while( LL_HSEM_1StepLock(HSEM, CFG_HW_RCC_SEMID));

  if (!LL_HSEM_1StepLock(HSEM, CFG_HW_ENTRY_STOP_MODE_SEMID)) {
        if(LL_PWR_IsActiveFlag_C2DS()) {
            // Release ENTRY_STOP_MODE semaphore
            LL_HSEM_ReleaseLock(HSEM, CFG_HW_ENTRY_STOP_MODE_SEMID, 0);

            // The switch on HSI before entering Stop Mode is required 
            furi_hal_clock_switch_to_hsi();
        }
    } else {
        /**
         * The switch on HSI before entering Stop Mode is required 
         */
        furi_hal_clock_switch_to_hsi();
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
        furi_hal_clock_switch_to_pll();
    }

    LL_HSEM_ReleaseLock(HSEM, CFG_HW_RCC_SEMID, 0);
}

void furi_hal_power_sleep() {
    if(furi_hal_power_deep_sleep_available()) {
        furi_hal_power_deep_sleep();
    } else {
        furi_hal_power_light_sleep();
    }
}


uint8_t furi_hal_power_get_pct() {
    return bq27220_get_state_of_charge();
}

uint8_t furi_hal_power_get_bat_health_pct() {
    return bq27220_get_state_of_health();
}

bool furi_hal_power_is_charging() {
    return bq25896_is_charging();
}

void furi_hal_power_off() {
    bq25896_poweroff();
}

void furi_hal_power_reset() {
    NVIC_SystemReset();
}

void furi_hal_power_enable_otg() {
    bq25896_enable_otg();
}

void furi_hal_power_disable_otg() {
    bq25896_disable_otg();
}

bool furi_hal_power_is_otg_enabled() {
    return bq25896_is_otg_enabled();
}

uint32_t furi_hal_power_get_battery_remaining_capacity() {
    return bq27220_get_remaining_capacity();
}

uint32_t furi_hal_power_get_battery_full_capacity() {
    return bq27220_get_full_charge_capacity();
}

float furi_hal_power_get_battery_voltage(FuriHalPowerIC ic) {
    if (ic == FuriHalPowerICCharger) {
        return (float)bq25896_get_vbat_voltage() / 1000.0f;
    } else if (ic == FuriHalPowerICFuelGauge) {
        return (float)bq27220_get_voltage() / 1000.0f;
    } else {
        return 0.0f;
    }
}

float furi_hal_power_get_battery_current(FuriHalPowerIC ic) {
    if (ic == FuriHalPowerICCharger) {
        return (float)bq25896_get_vbat_current() / 1000.0f;
    } else if (ic == FuriHalPowerICFuelGauge) {
        return (float)bq27220_get_current() / 1000.0f;
    } else {
        return 0.0f;
    }
}

float furi_hal_power_get_battery_temperature(FuriHalPowerIC ic) {
    if (ic == FuriHalPowerICCharger) {
        // Linear approximation, +/- 5 C
        return (71.0f - (float)bq25896_get_ntc_mpct()/1000) / 0.6f;
    } else if (ic == FuriHalPowerICFuelGauge) {
        return ((float)bq27220_get_temperature() - 2731.0f) / 10.0f;
    } else {
        return 0.0f;
    }
    
}

float furi_hal_power_get_usb_voltage(){
    return (float)bq25896_get_vbus_voltage() / 1000.0f;
}

void furi_hal_power_dump_state() {
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
            bq27220_get_voltage(), bq27220_get_current(), (int)furi_hal_power_get_battery_temperature(FuriHalPowerICFuelGauge)
        );
    }

    printf(
        "bq25896: VBUS: %d, VSYS: %d, VBAT: %d, Current: %d, NTC: %ldm%%\r\n",
        bq25896_get_vbus_voltage(), bq25896_get_vsys_voltage(),
        bq25896_get_vbat_voltage(), bq25896_get_vbat_current(),
        bq25896_get_ntc_mpct()
    );
}

void furi_hal_power_enable_external_3_3v(){
    LL_GPIO_SetOutputPin(PERIPH_POWER_GPIO_Port, PERIPH_POWER_Pin);
}

void furi_hal_power_disable_external_3_3v(){
    LL_GPIO_ResetOutputPin(PERIPH_POWER_GPIO_Port, PERIPH_POWER_Pin);
}

void furi_hal_power_suppress_charge_enter() {
    vTaskSuspendAll();
    bool disable_charging = furi_hal_power.suppress_charge == 0;
    furi_hal_power.suppress_charge++;
    xTaskResumeAll();

    if (disable_charging) {
        bq25896_disable_charging();
    }
}

void furi_hal_power_suppress_charge_exit() {
    vTaskSuspendAll();
    furi_hal_power.suppress_charge--;
    bool enable_charging = furi_hal_power.suppress_charge == 0;
    xTaskResumeAll();

    if (enable_charging) {
        bq25896_enable_charging();
    }
}