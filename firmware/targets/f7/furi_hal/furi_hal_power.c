#include <furi_hal_power.h>
#include <furi_hal_clock.h>
#include <furi_hal_bt.h>
#include <furi_hal_vibro.h>
#include <furi_hal_resources.h>
#include <furi_hal_uart.h>
#include <furi_hal_rtc.h>
#include <furi_hal_debug.h>

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

#ifndef FURI_HAL_POWER_DEBUG_WFI_GPIO
#define FURI_HAL_POWER_DEBUG_WFI_GPIO (&gpio_ext_pb2)
#endif

#ifndef FURI_HAL_POWER_DEBUG_STOP_GPIO
#define FURI_HAL_POWER_DEBUG_STOP_GPIO (&gpio_ext_pc3)
#endif

#ifndef FURI_HAL_POWER_STOP_MODE
#define FURI_HAL_POWER_STOP_MODE (LL_PWR_MODE_STOP2)
#endif

typedef struct {
    volatile uint8_t insomnia;
    volatile uint8_t suppress_charge;

    uint8_t gauge_initialized;
    uint8_t charger_initialized;
} FuriHalPower;

static volatile FuriHalPower furi_hal_power = {
    .insomnia = 0,
    .suppress_charge = 0,
};

#include <furi_hal_power_calibration.h>

void furi_hal_power_init() {
#ifdef FURI_HAL_POWER_DEBUG
    furi_hal_gpio_init_simple(FURI_HAL_POWER_DEBUG_WFI_GPIO, GpioModeOutputPushPull);
    furi_hal_gpio_init_simple(FURI_HAL_POWER_DEBUG_STOP_GPIO, GpioModeOutputPushPull);
    furi_hal_gpio_write(FURI_HAL_POWER_DEBUG_WFI_GPIO, 0);
    furi_hal_gpio_write(FURI_HAL_POWER_DEBUG_STOP_GPIO, 0);
#endif

    LL_PWR_SetRegulVoltageScaling(LL_PWR_REGU_VOLTAGE_SCALE1);
    LL_PWR_SMPS_SetMode(LL_PWR_SMPS_STEP_DOWN);

    LL_PWR_SetPowerMode(FURI_HAL_POWER_STOP_MODE);
    LL_C2_PWR_SetPowerMode(FURI_HAL_POWER_STOP_MODE);

    furi_hal_i2c_acquire(&furi_hal_i2c_handle_power);
    bq27220_init(&furi_hal_i2c_handle_power, &cedv);
    bq25896_init(&furi_hal_i2c_handle_power);
    furi_hal_i2c_release(&furi_hal_i2c_handle_power);

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

static inline bool furi_hal_power_deep_sleep_available() {
    return furi_hal_bt_is_alive() && !furi_hal_rtc_is_flag_set(FuriHalRtcFlagLegacySleep) &&
           !furi_hal_debug_is_gdb_session_active();
}

static inline void furi_hal_power_light_sleep() {
    __WFI();
}

static inline void furi_hal_power_suspend_aux_periphs() {
    // Disable USART
    furi_hal_uart_suspend(FuriHalUartIdUSART1);
    furi_hal_uart_suspend(FuriHalUartIdLPUART1);
}

static inline void furi_hal_power_resume_aux_periphs() {
    // Re-enable USART
    furi_hal_uart_resume(FuriHalUartIdUSART1);
    furi_hal_uart_resume(FuriHalUartIdLPUART1);
}

static inline void furi_hal_power_deep_sleep() {
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
    LL_LPM_EnableDeepSleep();

#if defined(__CC_ARM)
    // Force store operations
    __force_stores();
#endif

    __WFI();

    LL_LPM_EnableSleep();

    /* Release ENTRY_STOP_MODE semaphore */
    LL_HSEM_ReleaseLock(HSEM, CFG_HW_ENTRY_STOP_MODE_SEMID, 0);

    while(LL_HSEM_1StepLock(HSEM, CFG_HW_RCC_SEMID))
        ;

    if(LL_RCC_GetSysClkSource() != LL_RCC_SYS_CLKSOURCE_STATUS_PLL) {
        furi_hal_clock_switch_to_pll();
    }

    LL_HSEM_ReleaseLock(HSEM, CFG_HW_RCC_SEMID, 0);

    furi_hal_power_resume_aux_periphs();
    furi_hal_rtc_sync_shadow();
}

void furi_hal_power_sleep() {
    if(furi_hal_power_deep_sleep_available()) {
#ifdef FURI_HAL_POWER_DEBUG
        furi_hal_gpio_write(FURI_HAL_POWER_DEBUG_STOP_GPIO, 1);
#endif
        furi_hal_power_deep_sleep();
#ifdef FURI_HAL_POWER_DEBUG
        furi_hal_gpio_write(FURI_HAL_POWER_DEBUG_STOP_GPIO, 0);
#endif
    } else {
#ifdef FURI_HAL_POWER_DEBUG
        furi_hal_gpio_write(FURI_HAL_POWER_DEBUG_WFI_GPIO, 1);
#endif
        furi_hal_power_light_sleep();
#ifdef FURI_HAL_POWER_DEBUG
        furi_hal_gpio_write(FURI_HAL_POWER_DEBUG_WFI_GPIO, 0);
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
    furi_hal_vibro_on(true);
    furi_delay_us(50000);
    // Send poweroff to charger
    furi_hal_i2c_acquire(&furi_hal_i2c_handle_power);
    bq25896_poweroff(&furi_hal_i2c_handle_power);
    furi_hal_i2c_release(&furi_hal_i2c_handle_power);
    furi_hal_vibro_on(false);
}

void furi_hal_power_reset() {
    NVIC_SystemReset();
}

bool furi_hal_power_enable_otg() {
    furi_hal_i2c_acquire(&furi_hal_i2c_handle_power);
    bq25896_set_boost_lim(&furi_hal_i2c_handle_power, BoostLim_2150);
    bq25896_enable_otg(&furi_hal_i2c_handle_power);
    furi_delay_ms(30);
    bool ret = bq25896_is_otg_enabled(&furi_hal_i2c_handle_power);
    bq25896_set_boost_lim(&furi_hal_i2c_handle_power, BoostLim_1400);
    furi_hal_i2c_release(&furi_hal_i2c_handle_power);
    return ret;
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

float furi_hal_power_get_battery_charge_voltage_limit() {
    furi_hal_i2c_acquire(&furi_hal_i2c_handle_power);
    float ret = (float)bq25896_get_vreg_voltage(&furi_hal_i2c_handle_power) / 1000.0f;
    furi_hal_i2c_release(&furi_hal_i2c_handle_power);
    return ret;
}

void furi_hal_power_set_battery_charge_voltage_limit(float voltage) {
    furi_hal_i2c_acquire(&furi_hal_i2c_handle_power);
    // Adding 0.0005 is necessary because 4.016f is 4.015999794000, which gets truncated
    bq25896_set_vreg_voltage(&furi_hal_i2c_handle_power, (uint16_t)(voltage * 1000.0f + 0.0005f));
    furi_hal_i2c_release(&furi_hal_i2c_handle_power);
}

bool furi_hal_power_check_otg_fault() {
    furi_hal_i2c_acquire(&furi_hal_i2c_handle_power);
    bool ret = bq25896_check_otg_fault(&furi_hal_i2c_handle_power);
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

void furi_hal_power_enable_external_3_3v() {
    furi_hal_gpio_write(&gpio_periph_power, 1);
}

void furi_hal_power_disable_external_3_3v() {
    furi_hal_gpio_write(&gpio_periph_power, 0);
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

void furi_hal_power_info_get(PropertyValueCallback out, char sep, void* context) {
    furi_assert(out);

    FuriString* value = furi_string_alloc();
    FuriString* key = furi_string_alloc();

    PropertyValueContext property_context = {
        .key = key, .value = value, .out = out, .sep = sep, .last = false, .context = context};

    if(sep == '.') {
        property_value_out(&property_context, NULL, 2, "format", "major", "2");
        property_value_out(&property_context, NULL, 2, "format", "minor", "1");
    } else {
        property_value_out(&property_context, NULL, 3, "power", "info", "major", "2");
        property_value_out(&property_context, NULL, 3, "power", "info", "minor", "1");
    }

    uint8_t charge = furi_hal_power_get_pct();
    property_value_out(&property_context, "%u", 2, "charge", "level", charge);

    const char* charge_state;
    if(furi_hal_power_is_charging()) {
        if((charge < 100) && (!furi_hal_power_is_charging_done())) {
            charge_state = "charging";
        } else {
            charge_state = "charged";
        }
    } else {
        charge_state = "discharging";
    }

    property_value_out(&property_context, NULL, 2, "charge", "state", charge_state);
    uint16_t charge_voltage_limit =
        (uint16_t)(furi_hal_power_get_battery_charge_voltage_limit() * 1000.f);
    property_value_out(
        &property_context, "%u", 3, "charge", "voltage", "limit", charge_voltage_limit);
    uint16_t voltage =
        (uint16_t)(furi_hal_power_get_battery_voltage(FuriHalPowerICFuelGauge) * 1000.f);
    property_value_out(&property_context, "%u", 2, "battery", "voltage", voltage);
    int16_t current =
        (int16_t)(furi_hal_power_get_battery_current(FuriHalPowerICFuelGauge) * 1000.f);
    property_value_out(&property_context, "%d", 2, "battery", "current", current);
    int16_t temperature = (int16_t)furi_hal_power_get_battery_temperature(FuriHalPowerICFuelGauge);
    property_value_out(&property_context, "%d", 2, "battery", "temp", temperature);
    property_value_out(
        &property_context, "%u", 2, "battery", "health", furi_hal_power_get_bat_health_pct());
    property_value_out(
        &property_context,
        "%lu",
        2,
        "capacity",
        "remain",
        furi_hal_power_get_battery_remaining_capacity());
    property_value_out(
        &property_context,
        "%lu",
        2,
        "capacity",
        "full",
        furi_hal_power_get_battery_full_capacity());
    property_context.last = true;
    property_value_out(
        &property_context,
        "%lu",
        2,
        "capacity",
        "design",
        furi_hal_power_get_battery_design_capacity());

    furi_string_free(key);
    furi_string_free(value);
}

void furi_hal_power_debug_get(PropertyValueCallback out, void* context) {
    furi_assert(out);

    FuriString* value = furi_string_alloc();
    FuriString* key = furi_string_alloc();

    PropertyValueContext property_context = {
        .key = key, .value = value, .out = out, .sep = '.', .last = false, .context = context};

    BatteryStatus battery_status;
    OperationStatus operation_status;

    furi_hal_i2c_acquire(&furi_hal_i2c_handle_power);

    // Power Debug version
    property_value_out(&property_context, NULL, 2, "format", "major", "1");
    property_value_out(&property_context, NULL, 2, "format", "minor", "0");

    property_value_out(
        &property_context,
        "%d",
        2,
        "charger",
        "vbus",
        bq25896_get_vbus_voltage(&furi_hal_i2c_handle_power));
    property_value_out(
        &property_context,
        "%d",
        2,
        "charger",
        "vsys",
        bq25896_get_vsys_voltage(&furi_hal_i2c_handle_power));
    property_value_out(
        &property_context,
        "%d",
        2,
        "charger",
        "vbat",
        bq25896_get_vbat_voltage(&furi_hal_i2c_handle_power));
    property_value_out(
        &property_context,
        "%d",
        2,
        "charger",
        "vreg",
        bq25896_get_vreg_voltage(&furi_hal_i2c_handle_power));
    property_value_out(
        &property_context,
        "%d",
        2,
        "charger",
        "current",
        bq25896_get_vbat_current(&furi_hal_i2c_handle_power));

    const uint32_t ntc_mpct = bq25896_get_ntc_mpct(&furi_hal_i2c_handle_power);

    if(bq27220_get_battery_status(&furi_hal_i2c_handle_power, &battery_status) != BQ27220_ERROR &&
       bq27220_get_operation_status(&furi_hal_i2c_handle_power, &operation_status) !=
           BQ27220_ERROR) {
        property_value_out(&property_context, "%lu", 2, "charger", "ntc", ntc_mpct);
        property_value_out(&property_context, "%d", 2, "gauge", "calmd", operation_status.CALMD);
        property_value_out(&property_context, "%d", 2, "gauge", "sec", operation_status.SEC);
        property_value_out(&property_context, "%d", 2, "gauge", "edv2", operation_status.EDV2);
        property_value_out(&property_context, "%d", 2, "gauge", "vdq", operation_status.VDQ);
        property_value_out(
            &property_context, "%d", 2, "gauge", "initcomp", operation_status.INITCOMP);
        property_value_out(&property_context, "%d", 2, "gauge", "smth", operation_status.SMTH);
        property_value_out(&property_context, "%d", 2, "gauge", "btpint", operation_status.BTPINT);
        property_value_out(
            &property_context, "%d", 2, "gauge", "cfgupdate", operation_status.CFGUPDATE);

        // Battery status register, part 1
        property_value_out(&property_context, "%d", 2, "gauge", "chginh", battery_status.CHGINH);
        property_value_out(&property_context, "%d", 2, "gauge", "fc", battery_status.FC);
        property_value_out(&property_context, "%d", 2, "gauge", "otd", battery_status.OTD);
        property_value_out(&property_context, "%d", 2, "gauge", "otc", battery_status.OTC);
        property_value_out(&property_context, "%d", 2, "gauge", "sleep", battery_status.SLEEP);
        property_value_out(&property_context, "%d", 2, "gauge", "ocvfail", battery_status.OCVFAIL);
        property_value_out(&property_context, "%d", 2, "gauge", "ocvcomp", battery_status.OCVCOMP);
        property_value_out(&property_context, "%d", 2, "gauge", "fd", battery_status.FD);

        // Battery status register, part 2
        property_value_out(&property_context, "%d", 2, "gauge", "dsg", battery_status.DSG);
        property_value_out(&property_context, "%d", 2, "gauge", "sysdwn", battery_status.SYSDWN);
        property_value_out(&property_context, "%d", 2, "gauge", "tda", battery_status.TDA);
        property_value_out(
            &property_context, "%d", 2, "gauge", "battpres", battery_status.BATTPRES);
        property_value_out(&property_context, "%d", 2, "gauge", "authgd", battery_status.AUTH_GD);
        property_value_out(&property_context, "%d", 2, "gauge", "ocvgd", battery_status.OCVGD);
        property_value_out(&property_context, "%d", 2, "gauge", "tca", battery_status.TCA);
        property_value_out(&property_context, "%d", 2, "gauge", "rsvd", battery_status.RSVD);

        // Voltage and current info
        property_value_out(
            &property_context,
            "%d",
            3,
            "gauge",
            "capacity",
            "full",
            bq27220_get_full_charge_capacity(&furi_hal_i2c_handle_power));
        property_value_out(
            &property_context,
            "%d",
            3,
            "gauge",
            "capacity",
            "design",
            bq27220_get_design_capacity(&furi_hal_i2c_handle_power));
        property_value_out(
            &property_context,
            "%d",
            3,
            "gauge",
            "capacity",
            "remain",
            bq27220_get_remaining_capacity(&furi_hal_i2c_handle_power));
        property_value_out(
            &property_context,
            "%d",
            3,
            "gauge",
            "state",
            "charge",
            bq27220_get_state_of_charge(&furi_hal_i2c_handle_power));
        property_value_out(
            &property_context,
            "%d",
            3,
            "gauge",
            "state",
            "health",
            bq27220_get_state_of_health(&furi_hal_i2c_handle_power));
        property_value_out(
            &property_context,
            "%d",
            2,
            "gauge",
            "voltage",
            bq27220_get_voltage(&furi_hal_i2c_handle_power));
        property_value_out(
            &property_context,
            "%d",
            2,
            "gauge",
            "current",
            bq27220_get_current(&furi_hal_i2c_handle_power));

        property_context.last = true;
        const int battery_temp =
            (int)furi_hal_power_get_battery_temperature_internal(FuriHalPowerICFuelGauge);
        property_value_out(&property_context, "%d", 2, "gauge", "temperature", battery_temp);
    } else {
        property_context.last = true;
        property_value_out(&property_context, "%lu", 2, "charger", "ntc", ntc_mpct);
    }

    furi_string_free(key);
    furi_string_free(value);

    furi_hal_i2c_release(&furi_hal_i2c_handle_power);
}
