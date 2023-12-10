#include <furi_hal_i2c_config.h>
#include <furi_hal_resources.h>
#include <furi_hal_version.h>
#include <furi_hal_bus.h>

#include <stm32wbxx_ll_rcc.h>

/** Timing register value is computed with the STM32CubeMX Tool,
  * Standard Mode @100kHz with I2CCLK = 64 MHz,
  * rise time = 0ns, fall time = 0ns
  */
#define FURI_HAL_I2C_CONFIG_POWER_I2C_TIMINGS_100 0x10707DBC

/** Timing register value is computed with the STM32CubeMX Tool,
  * Fast Mode @400kHz with I2CCLK = 64 MHz,
  * rise time = 0ns, fall time = 0ns
  */
#define FURI_HAL_I2C_CONFIG_POWER_I2C_TIMINGS_400 0x00602173

FuriMutex* furi_hal_i2c_bus_power_mutex = NULL;

static void furi_hal_i2c_bus_power_event(FuriHalI2cBus* bus, FuriHalI2cBusEvent event) {
    if(event == FuriHalI2cBusEventInit) {
        furi_hal_i2c_bus_power_mutex = furi_mutex_alloc(FuriMutexTypeNormal);
        bus->current_handle = NULL;
    } else if(event == FuriHalI2cBusEventDeinit) {
        furi_mutex_free(furi_hal_i2c_bus_power_mutex);
    } else if(event == FuriHalI2cBusEventLock) {
        furi_check(
            furi_mutex_acquire(furi_hal_i2c_bus_power_mutex, FuriWaitForever) == FuriStatusOk);
    } else if(event == FuriHalI2cBusEventUnlock) {
        furi_check(furi_mutex_release(furi_hal_i2c_bus_power_mutex) == FuriStatusOk);
    } else if(event == FuriHalI2cBusEventActivate) {
        FURI_CRITICAL_ENTER();
        furi_hal_bus_enable(FuriHalBusI2C1);
        LL_RCC_SetI2CClockSource(LL_RCC_I2C1_CLKSOURCE_PCLK1);
        FURI_CRITICAL_EXIT();
    } else if(event == FuriHalI2cBusEventDeactivate) {
        furi_hal_bus_disable(FuriHalBusI2C1);
    }
}

FuriHalI2cBus furi_hal_i2c_bus_power = {
    .i2c = I2C1,
    .callback = furi_hal_i2c_bus_power_event,
};

FuriMutex* furi_hal_i2c_bus_external_mutex = NULL;

static void furi_hal_i2c_bus_external_event(FuriHalI2cBus* bus, FuriHalI2cBusEvent event) {
    if(event == FuriHalI2cBusEventInit) {
        furi_hal_i2c_bus_external_mutex = furi_mutex_alloc(FuriMutexTypeNormal);
        bus->current_handle = NULL;
    } else if(event == FuriHalI2cBusEventDeinit) {
        furi_mutex_free(furi_hal_i2c_bus_external_mutex);
    } else if(event == FuriHalI2cBusEventLock) {
        furi_check(
            furi_mutex_acquire(furi_hal_i2c_bus_external_mutex, FuriWaitForever) == FuriStatusOk);
    } else if(event == FuriHalI2cBusEventUnlock) {
        furi_check(furi_mutex_release(furi_hal_i2c_bus_external_mutex) == FuriStatusOk);
    } else if(event == FuriHalI2cBusEventActivate) {
        FURI_CRITICAL_ENTER();
        furi_hal_bus_enable(FuriHalBusI2C3);
        LL_RCC_SetI2CClockSource(LL_RCC_I2C3_CLKSOURCE_PCLK1);
        FURI_CRITICAL_EXIT();
    } else if(event == FuriHalI2cBusEventDeactivate) {
        furi_hal_bus_disable(FuriHalBusI2C3);
    }
}

FuriHalI2cBus furi_hal_i2c_bus_external = {
    .i2c = I2C3,
    .callback = furi_hal_i2c_bus_external_event,
};

void furi_hal_i2c_bus_handle_power_event(
    FuriHalI2cBusHandle* handle,
    FuriHalI2cBusHandleEvent event) {
    if(event == FuriHalI2cBusHandleEventActivate) {
        furi_hal_gpio_init_ex(
            &gpio_i2c_power_sda,
            GpioModeAltFunctionOpenDrain,
            GpioPullNo,
            GpioSpeedLow,
            GpioAltFn4I2C1);
        furi_hal_gpio_init_ex(
            &gpio_i2c_power_scl,
            GpioModeAltFunctionOpenDrain,
            GpioPullNo,
            GpioSpeedLow,
            GpioAltFn4I2C1);

        LL_I2C_InitTypeDef I2C_InitStruct;
        I2C_InitStruct.PeripheralMode = LL_I2C_MODE_I2C;
        I2C_InitStruct.AnalogFilter = LL_I2C_ANALOGFILTER_ENABLE;
        I2C_InitStruct.DigitalFilter = 0;
        I2C_InitStruct.OwnAddress1 = 0;
        I2C_InitStruct.TypeAcknowledge = LL_I2C_ACK;
        I2C_InitStruct.OwnAddrSize = LL_I2C_OWNADDRESS1_7BIT;
        if(furi_hal_version_get_hw_version() > 10) {
            I2C_InitStruct.Timing = FURI_HAL_I2C_CONFIG_POWER_I2C_TIMINGS_400;
        } else {
            I2C_InitStruct.Timing = FURI_HAL_I2C_CONFIG_POWER_I2C_TIMINGS_100;
        }
        LL_I2C_Init(handle->bus->i2c, &I2C_InitStruct);
        // I2C is enabled at this point
        LL_I2C_EnableAutoEndMode(handle->bus->i2c);
        LL_I2C_SetOwnAddress2(handle->bus->i2c, 0, LL_I2C_OWNADDRESS2_NOMASK);
        LL_I2C_DisableOwnAddress2(handle->bus->i2c);
        LL_I2C_DisableGeneralCall(handle->bus->i2c);
        LL_I2C_EnableClockStretching(handle->bus->i2c);
    } else if(event == FuriHalI2cBusHandleEventDeactivate) {
        LL_I2C_Disable(handle->bus->i2c);
        furi_hal_gpio_write(&gpio_i2c_power_sda, 1);
        furi_hal_gpio_write(&gpio_i2c_power_scl, 1);
        furi_hal_gpio_init_ex(
            &gpio_i2c_power_sda, GpioModeAnalog, GpioPullNo, GpioSpeedLow, GpioAltFnUnused);
        furi_hal_gpio_init_ex(
            &gpio_i2c_power_scl, GpioModeAnalog, GpioPullNo, GpioSpeedLow, GpioAltFnUnused);
    }
}

FuriHalI2cBusHandle furi_hal_i2c_handle_power = {
    .bus = &furi_hal_i2c_bus_power,
    .callback = furi_hal_i2c_bus_handle_power_event,
};

void furi_hal_i2c_bus_handle_external_event(
    FuriHalI2cBusHandle* handle,
    FuriHalI2cBusHandleEvent event) {
    if(event == FuriHalI2cBusHandleEventActivate) {
        furi_hal_gpio_init_ex(
            &gpio_ext_pc0, GpioModeAltFunctionOpenDrain, GpioPullNo, GpioSpeedLow, GpioAltFn4I2C3);
        furi_hal_gpio_init_ex(
            &gpio_ext_pc1, GpioModeAltFunctionOpenDrain, GpioPullNo, GpioSpeedLow, GpioAltFn4I2C3);

        LL_I2C_InitTypeDef I2C_InitStruct;
        I2C_InitStruct.PeripheralMode = LL_I2C_MODE_I2C;
        I2C_InitStruct.AnalogFilter = LL_I2C_ANALOGFILTER_ENABLE;
        I2C_InitStruct.DigitalFilter = 0;
        I2C_InitStruct.OwnAddress1 = 0;
        I2C_InitStruct.TypeAcknowledge = LL_I2C_ACK;
        I2C_InitStruct.OwnAddrSize = LL_I2C_OWNADDRESS1_7BIT;
        I2C_InitStruct.Timing = FURI_HAL_I2C_CONFIG_POWER_I2C_TIMINGS_100;
        LL_I2C_Init(handle->bus->i2c, &I2C_InitStruct);
        // I2C is enabled at this point
        LL_I2C_EnableAutoEndMode(handle->bus->i2c);
        LL_I2C_SetOwnAddress2(handle->bus->i2c, 0, LL_I2C_OWNADDRESS2_NOMASK);
        LL_I2C_DisableOwnAddress2(handle->bus->i2c);
        LL_I2C_DisableGeneralCall(handle->bus->i2c);
        LL_I2C_EnableClockStretching(handle->bus->i2c);
    } else if(event == FuriHalI2cBusHandleEventDeactivate) {
        LL_I2C_Disable(handle->bus->i2c);
        furi_hal_gpio_write(&gpio_ext_pc0, 1);
        furi_hal_gpio_write(&gpio_ext_pc1, 1);
        furi_hal_gpio_init_ex(
            &gpio_ext_pc0, GpioModeAnalog, GpioPullNo, GpioSpeedLow, GpioAltFnUnused);
        furi_hal_gpio_init_ex(
            &gpio_ext_pc1, GpioModeAnalog, GpioPullNo, GpioSpeedLow, GpioAltFnUnused);
    }
}

FuriHalI2cBusHandle furi_hal_i2c_handle_external = {
    .bus = &furi_hal_i2c_bus_external,
    .callback = furi_hal_i2c_bus_handle_external_event,
};
