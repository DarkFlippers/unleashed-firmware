#include <furi_hal_power.h>
#include <furi_hal_clock.h>
#include <furi_hal_bt.h>
#include <furi_hal_resources.h>
#include <furi_hal_uart.h>

#include <stm32wbxx_ll_rcc.h>
#include <stm32wbxx_ll_pwr.h>
#include <stm32wbxx_ll_hsem.h>
#include <stm32wbxx_ll_cortex.h>
#include <stm32wbxx_ll_gpio.h>

#include <hw_conf.h>
#include <bq27220.h>
#include <bq25896.h>

#include <furi.h>

#define TAG "FuriHalPower"

#ifdef FURI_HAL_POWER_DEEP_SLEEP_ENABLED
#define FURI_HAL_POWER_DEEP_INSOMNIA 0
#else
#define FURI_HAL_POWER_DEEP_INSOMNIA 1
#endif

typedef struct {
    volatile uint8_t insomnia;
    volatile uint8_t deep_insomnia;
    volatile uint8_t suppress_charge;

    uint8_t gauge_initialized;
    uint8_t charger_initialized;
} FuriHalPower;

static volatile FuriHalPower furi_hal_power = {
    .insomnia = 0,
    .deep_insomnia = FURI_HAL_POWER_DEEP_INSOMNIA,
    .suppress_charge = 0,
};

const ParamCEDV cedv = {
    .cedv_conf.gauge_conf =
        {
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
    .full_charge_cap = 2101,
    .design_cap = 2101,
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

void furi_hal_power_init() {
    LL_PWR_SetRegulVoltageScaling(LL_PWR_REGU_VOLTAGE_SCALE1);
    LL_PWR_SMPS_SetMode(LL_PWR_SMPS_STEP_DOWN);

    furi_hal_i2c_acquire(&furi_hal_i2c_handle_power);
    bq27220_init(&furi_hal_i2c_handle_power, &cedv);
    bq25896_init(&furi_hal_i2c_handle_power);
    furi_hal_i2c_release(&furi_hal_i2c_handle_power);

#ifdef FURI_HAL_OS_DEBUG
    furi_hal_gpio_init_simple(&gpio_ext_pb2, GpioModeOutputPushPull);
    furi_hal_gpio_init_simple(&gpio_ext_pc3, GpioModeOutputPushPull);
#endif

    FURI_LOG_I(TAG, "Init OK");
}

bool furi_hal_power_gauge_is_ok() {
    bool ret = true;

    BatteryStatus battery_status;
    OperationStatus operation_status;

    furi_hal_i2c_acquire(&furi_hal_i2c_handle_power);

    if(bq27220_get_battery_status(&furi_hal_i2c_handle_power, &battery_status) == BQ27220_ERROR ||
       bq27220_get_operation_status(&furi_hal_i2c_handle_power, &operation_status) ==
           BQ27220_ERROR) {
        ret = false;
    } else {
        ret &= battery_status.BATTPRES;
        ret &= operation_status.INITCOMP;
        ret &= (cedv.design_cap == bq27220_get_design_capacity(&furi_hal_i2c_handle_power));
    }

    furi_hal_i2c_release(&furi_hal_i2c_handle_power);

    return ret;
}

uint16_t furi_hal_power_insomnia_level() {
    return furi_hal_power.insomnia;
}

void furi_hal_power_insomnia_enter() {
    FURI_CRITICAL_ENTER();
    furi_assert(furi_hal_power.insomnia < UINT8_MAX);
    furi_hal_power.insomnia++;
    FURI_CRITICAL_EXIT();
}

void furi_hal_power_insomnia_exit() {
    FURI_CRITICAL_ENTER();
    furi_assert(furi_hal_power.insomnia > 0);
    furi_hal_power.insomnia--;
    FURI_CRITICAL_EXIT();
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

static inline void furi_hal_power_suspend_aux_periphs() {
    // Disable USART
    furi_hal_uart_suspend(FuriHalUartIdUSART1);
    furi_hal_uart_suspend(FuriHalUartIdLPUART1);
    // TODO: Disable USB
}

static inline void furi_hal_power_resume_aux_periphs() {
    // Re-enable USART
    furi_hal_uart_resume(FuriHalUartIdUSART1);
    furi_hal_uart_resume(FuriHalUartIdLPUART1);
    // TODO: Re-enable USB
}

void furi_hal_power_deep_sleep() {
    furi_hal_power_suspend_aux_periphs();

    while(LL_HSEM_1StepLock(HSEM, CFG_HW_RCC_SEMID))
        ;

    if(!LL_HSEM_1StepLock(HSEM, CFG_HW_ENTRY_STOP_MODE_SEMID)) {
        if(LL_PWR_IsActiveFlag_C2DS() || LL_PWR_IsActiveFlag_C2SB()) {
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
    LL_PWR_SetPowerMode(LL_PWR_MODE_STOP2);
    LL_C2_PWR_SetPowerMode(LL_PWR_MODE_STOP2);
    LL_LPM_EnableDeepSleep();

#if defined(__CC_ARM)
    // Force store operations
    __force_stores();
#endif

    __WFI();

    LL_LPM_EnableSleep();

    // Make sure that values differ to prevent disaster on wfi
    LL_PWR_SetPowerMode(LL_PWR_MODE_STOP0);
    LL_C2_PWR_SetPowerMode(LL_PWR_MODE_SHUTDOWN);

    LL_PWR_ClearFlag_C1STOP_C1STB();
    LL_PWR_ClearFlag_C2STOP_C2STB();

    /* Release ENTRY_STOP_MODE semaphore */
    LL_HSEM_ReleaseLock(HSEM, CFG_HW_ENTRY_STOP_MODE_SEMID, 0);

    while(LL_HSEM_1StepLock(HSEM, CFG_HW_RCC_SEMID))
        ;

    if(LL_RCC_GetSysClkSource() != LL_RCC_SYS_CLKSOURCE_STATUS_PLL) {
        furi_hal_clock_switch_to_pll();
    }

    LL_HSEM_ReleaseLock(HSEM, CFG_HW_RCC_SEMID, 0);

    furi_hal_power_resume_aux_periphs();
}

void furi_hal_power_sleep() {
    if(furi_hal_power_deep_sleep_available()) {
#ifdef FURI_HAL_OS_DEBUG
        furi_hal_gpio_write(&gpio_ext_pc3, 1);
#endif

        furi_hal_power_deep_sleep();

#ifdef FURI_HAL_OS_DEBUG
        furi_hal_gpio_write(&gpio_ext_pc3, 0);
#endif
    } else {
#ifdef FURI_HAL_OS_DEBUG
        furi_hal_gpio_write(&gpio_ext_pb2, 1);
#endif

        furi_hal_power_light_sleep();

#ifdef FURI_HAL_OS_DEBUG
        furi_hal_gpio_write(&gpio_ext_pb2, 0);
#endif
    }
}

uint8_t furi_hal_power_get_pct() {
    furi_hal_i2c_acquire(&furi_hal_i2c_handle_power);
    uint8_t ret = bq27220_get_state_of_charge(&furi_hal_i2c_handle_power);
    furi_hal_i2c_release(&furi_hal_i2c_handle_power);
    return ret;
}

uint8_t furi_hal_power_get_bat_health_pct() {
    furi_hal_i2c_acquire(&furi_hal_i2c_handle_power);
    uint8_t ret = bq27220_get_state_of_health(&furi_hal_i2c_handle_power);
    furi_hal_i2c_release(&furi_hal_i2c_handle_power);
    return ret;
}

bool furi_hal_power_is_charging() {
    furi_hal_i2c_acquire(&furi_hal_i2c_handle_power);
    bool ret = bq25896_is_charging(&furi_hal_i2c_handle_power);
    furi_hal_i2c_release(&furi_hal_i2c_handle_power);
    return ret;
}

bool furi_hal_power_is_charging_done() {
    furi_hal_i2c_acquire(&furi_hal_i2c_handle_power);
    bool ret = bq25896_is_charging_done(&furi_hal_i2c_handle_power);
    furi_hal_i2c_release(&furi_hal_i2c_handle_power);
    return ret;
}

void furi_hal_power_shutdown() {
    furi_hal_power_insomnia_enter();

    furi_hal_bt_reinit();

    while(LL_HSEM_1StepLock(HSEM, CFG_HW_RCC_SEMID))
        ;

    if(!LL_HSEM_1StepLock(HSEM, CFG_HW_ENTRY_STOP_MODE_SEMID)) {
        if(LL_PWR_IsActiveFlag_C2DS() || LL_PWR_IsActiveFlag_C2SB()) {
            // Release ENTRY_STOP_MODE semaphore
            LL_HSEM_ReleaseLock(HSEM, CFG_HW_ENTRY_STOP_MODE_SEMID, 0);
        }
    }

    // Prepare Wakeup pin
    LL_PWR_SetWakeUpPinPolarityLow(LL_PWR_WAKEUP_PIN2);
    LL_PWR_EnableWakeUpPin(LL_PWR_WAKEUP_PIN2);
    LL_C2_PWR_EnableWakeUpPin(LL_PWR_WAKEUP_PIN2);

    /* Release RCC semaphore */
    LL_HSEM_ReleaseLock(HSEM, CFG_HW_RCC_SEMID, 0);

    LL_PWR_DisableBootC2();
    LL_PWR_SetPowerMode(LL_PWR_MODE_SHUTDOWN);
    LL_C2_PWR_SetPowerMode(LL_PWR_MODE_SHUTDOWN);
    LL_LPM_EnableDeepSleep();

    __WFI();
    furi_crash("Insomniac core2");
}

void furi_hal_power_off() {
    // Crutch: shutting down with ext 3V3 off is causing LSE to stop
    furi_hal_power_enable_external_3_3v();
    furi_delay_us(1000);
    // Send poweroff to charger
    furi_hal_i2c_acquire(&furi_hal_i2c_handle_power);
    bq25896_poweroff(&furi_hal_i2c_handle_power);
    furi_hal_i2c_release(&furi_hal_i2c_handle_power);
}

void furi_hal_power_reset() {
    NVIC_SystemReset();
}

void furi_hal_power_enable_otg() {
    furi_hal_i2c_acquire(&furi_hal_i2c_handle_power);
    bq25896_enable_otg(&furi_hal_i2c_handle_power);
    furi_hal_i2c_release(&furi_hal_i2c_handle_power);
}

void furi_hal_power_disable_otg() {
    furi_hal_i2c_acquire(&furi_hal_i2c_handle_power);
    bq25896_disable_otg(&furi_hal_i2c_handle_power);
    furi_hal_i2c_release(&furi_hal_i2c_handle_power);
}

bool furi_hal_power_is_otg_enabled() {
    furi_hal_i2c_acquire(&furi_hal_i2c_handle_power);
    bool ret = bq25896_is_otg_enabled(&furi_hal_i2c_handle_power);
    furi_hal_i2c_release(&furi_hal_i2c_handle_power);
    return ret;
}

void furi_hal_power_check_otg_status() {
    furi_hal_i2c_acquire(&furi_hal_i2c_handle_power);
    if(bq25896_check_otg_fault(&furi_hal_i2c_handle_power))
        bq25896_disable_otg(&furi_hal_i2c_handle_power);
    furi_hal_i2c_release(&furi_hal_i2c_handle_power);
}

uint32_t furi_hal_power_get_battery_remaining_capacity() {
    furi_hal_i2c_acquire(&furi_hal_i2c_handle_power);
    uint32_t ret = bq27220_get_remaining_capacity(&furi_hal_i2c_handle_power);
    furi_hal_i2c_release(&furi_hal_i2c_handle_power);
    return ret;
}

uint32_t furi_hal_power_get_battery_full_capacity() {
    furi_hal_i2c_acquire(&furi_hal_i2c_handle_power);
    uint32_t ret = bq27220_get_full_charge_capacity(&furi_hal_i2c_handle_power);
    furi_hal_i2c_release(&furi_hal_i2c_handle_power);
    return ret;
}

uint32_t furi_hal_power_get_battery_design_capacity() {
    furi_hal_i2c_acquire(&furi_hal_i2c_handle_power);
    uint32_t ret = bq27220_get_design_capacity(&furi_hal_i2c_handle_power);
    furi_hal_i2c_release(&furi_hal_i2c_handle_power);
    return ret;
}

float furi_hal_power_get_battery_voltage(FuriHalPowerIC ic) {
    float ret = 0.0f;

    furi_hal_i2c_acquire(&furi_hal_i2c_handle_power);
    if(ic == FuriHalPowerICCharger) {
        ret = (float)bq25896_get_vbat_voltage(&furi_hal_i2c_handle_power) / 1000.0f;
    } else if(ic == FuriHalPowerICFuelGauge) {
        ret = (float)bq27220_get_voltage(&furi_hal_i2c_handle_power) / 1000.0f;
    }
    furi_hal_i2c_release(&furi_hal_i2c_handle_power);

    return ret;
}

float furi_hal_power_get_battery_current(FuriHalPowerIC ic) {
    float ret = 0.0f;

    furi_hal_i2c_acquire(&furi_hal_i2c_handle_power);
    if(ic == FuriHalPowerICCharger) {
        ret = (float)bq25896_get_vbat_current(&furi_hal_i2c_handle_power) / 1000.0f;
    } else if(ic == FuriHalPowerICFuelGauge) {
        ret = (float)bq27220_get_current(&furi_hal_i2c_handle_power) / 1000.0f;
    }
    furi_hal_i2c_release(&furi_hal_i2c_handle_power);

    return ret;
}

static float furi_hal_power_get_battery_temperature_internal(FuriHalPowerIC ic) {
    float ret = 0.0f;

    if(ic == FuriHalPowerICCharger) {
        // Linear approximation, +/- 5 C
        ret = (71.0f - (float)bq25896_get_ntc_mpct(&furi_hal_i2c_handle_power) / 1000) / 0.6f;
    } else if(ic == FuriHalPowerICFuelGauge) {
        ret = ((float)bq27220_get_temperature(&furi_hal_i2c_handle_power) - 2731.0f) / 10.0f;
    }

    return ret;
}

float furi_hal_power_get_battery_temperature(FuriHalPowerIC ic) {
    furi_hal_i2c_acquire(&furi_hal_i2c_handle_power);
    float ret = furi_hal_power_get_battery_temperature_internal(ic);
    furi_hal_i2c_release(&furi_hal_i2c_handle_power);

    return ret;
}

float furi_hal_power_get_usb_voltage() {
    furi_hal_i2c_acquire(&furi_hal_i2c_handle_power);
    float ret = (float)bq25896_get_vbus_voltage(&furi_hal_i2c_handle_power) / 1000.0f;
    furi_hal_i2c_release(&furi_hal_i2c_handle_power);
    return ret;
}

void furi_hal_power_dump_state() {
    BatteryStatus battery_status;
    OperationStatus operation_status;

    furi_hal_i2c_acquire(&furi_hal_i2c_handle_power);

    if(bq27220_get_battery_status(&furi_hal_i2c_handle_power, &battery_status) == BQ27220_ERROR ||
       bq27220_get_operation_status(&furi_hal_i2c_handle_power, &operation_status) ==
           BQ27220_ERROR) {
        printf("Failed to get bq27220 status. Communication error.\r\n");
    } else {
        // Operation status register
        printf(
            "bq27220: CALMD: %d, SEC: %d, EDV2: %d, VDQ: %d, INITCOMP: %d, SMTH: %d, BTPINT: %d, CFGUPDATE: %d\r\n",
            operation_status.CALMD,
            operation_status.SEC,
            operation_status.EDV2,
            operation_status.VDQ,
            operation_status.INITCOMP,
            operation_status.SMTH,
            operation_status.BTPINT,
            operation_status.CFGUPDATE);
        // Battery status register, part 1
        printf(
            "bq27220: CHGINH: %d, FC: %d, OTD: %d, OTC: %d, SLEEP: %d, OCVFAIL: %d, OCVCOMP: %d, FD: %d\r\n",
            battery_status.CHGINH,
            battery_status.FC,
            battery_status.OTD,
            battery_status.OTC,
            battery_status.SLEEP,
            battery_status.OCVFAIL,
            battery_status.OCVCOMP,
            battery_status.FD);
        // Battery status register, part 2
        printf(
            "bq27220: DSG: %d, SYSDWN: %d, TDA: %d, BATTPRES: %d, AUTH_GD: %d, OCVGD: %d, TCA: %d, RSVD: %d\r\n",
            battery_status.DSG,
            battery_status.SYSDWN,
            battery_status.TDA,
            battery_status.BATTPRES,
            battery_status.AUTH_GD,
            battery_status.OCVGD,
            battery_status.TCA,
            battery_status.RSVD);
        // Voltage and current info
        printf(
            "bq27220: Full capacity: %dmAh, Design capacity: %dmAh, Remaining capacity: %dmAh, State of Charge: %d%%, State of health: %d%%\r\n",
            bq27220_get_full_charge_capacity(&furi_hal_i2c_handle_power),
            bq27220_get_design_capacity(&furi_hal_i2c_handle_power),
            bq27220_get_remaining_capacity(&furi_hal_i2c_handle_power),
            bq27220_get_state_of_charge(&furi_hal_i2c_handle_power),
            bq27220_get_state_of_health(&furi_hal_i2c_handle_power));
        printf(
            "bq27220: Voltage: %dmV, Current: %dmA, Temperature: %dC\r\n",
            bq27220_get_voltage(&furi_hal_i2c_handle_power),
            bq27220_get_current(&furi_hal_i2c_handle_power),
            (int)furi_hal_power_get_battery_temperature_internal(FuriHalPowerICFuelGauge));
    }

    printf(
        "bq25896: VBUS: %d, VSYS: %d, VBAT: %d, Current: %d, NTC: %ldm%%\r\n",
        bq25896_get_vbus_voltage(&furi_hal_i2c_handle_power),
        bq25896_get_vsys_voltage(&furi_hal_i2c_handle_power),
        bq25896_get_vbat_voltage(&furi_hal_i2c_handle_power),
        bq25896_get_vbat_current(&furi_hal_i2c_handle_power),
        bq25896_get_ntc_mpct(&furi_hal_i2c_handle_power));

    furi_hal_i2c_release(&furi_hal_i2c_handle_power);
}

void furi_hal_power_enable_external_3_3v() {
    furi_hal_gpio_write(&periph_power, 1);
}

void furi_hal_power_disable_external_3_3v() {
    furi_hal_gpio_write(&periph_power, 0);
}

void furi_hal_power_suppress_charge_enter() {
    vTaskSuspendAll();
    bool disable_charging = furi_hal_power.suppress_charge == 0;
    furi_hal_power.suppress_charge++;
    xTaskResumeAll();

    if(disable_charging) {
        furi_hal_i2c_acquire(&furi_hal_i2c_handle_power);
        bq25896_disable_charging(&furi_hal_i2c_handle_power);
        furi_hal_i2c_release(&furi_hal_i2c_handle_power);
    }
}

void furi_hal_power_suppress_charge_exit() {
    vTaskSuspendAll();
    furi_hal_power.suppress_charge--;
    bool enable_charging = furi_hal_power.suppress_charge == 0;
    xTaskResumeAll();

    if(enable_charging) {
        furi_hal_i2c_acquire(&furi_hal_i2c_handle_power);
        bq25896_enable_charging(&furi_hal_i2c_handle_power);
        furi_hal_i2c_release(&furi_hal_i2c_handle_power);
    }
}

void furi_hal_power_info_get(FuriHalPowerInfoCallback out, void* context) {
    furi_assert(out);

    string_t value;
    string_init(value);

    // Power Info version
    out("power_info_major", "1", false, context);
    out("power_info_minor", "0", false, context);

    uint8_t charge = furi_hal_power_get_pct();

    string_printf(value, "%u", charge);
    out("charge_level", string_get_cstr(value), false, context);

    if(furi_hal_power_is_charging()) {
        if(charge < 100) {
            string_printf(value, "charging");
        } else {
            string_printf(value, "charged");
        }
    } else {
        string_printf(value, "discharging");
    }
    out("charge_state", string_get_cstr(value), false, context);

    uint16_t voltage =
        (uint16_t)(furi_hal_power_get_battery_voltage(FuriHalPowerICFuelGauge) * 1000.f);
    string_printf(value, "%u", voltage);
    out("battery_voltage", string_get_cstr(value), false, context);

    int16_t current =
        (int16_t)(furi_hal_power_get_battery_current(FuriHalPowerICFuelGauge) * 1000.f);
    string_printf(value, "%d", current);
    out("battery_current", string_get_cstr(value), false, context);

    int16_t temperature = (int16_t)furi_hal_power_get_battery_temperature(FuriHalPowerICFuelGauge);
    string_printf(value, "%d", temperature);
    out("gauge_temp", string_get_cstr(value), false, context);

    string_printf(value, "%u", furi_hal_power_get_bat_health_pct());
    out("battery_health", string_get_cstr(value), false, context);

    string_printf(value, "%u", furi_hal_power_get_battery_remaining_capacity());
    out("capacity_remain", string_get_cstr(value), false, context);

    string_printf(value, "%u", furi_hal_power_get_battery_full_capacity());
    out("capacity_full", string_get_cstr(value), false, context);

    string_printf(value, "%u", furi_hal_power_get_battery_design_capacity());
    out("capacity_design", string_get_cstr(value), true, context);

    string_clear(value);
}
