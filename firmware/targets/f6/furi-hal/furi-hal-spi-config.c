#include <furi-hal-spi-config.h>
#include <furi-hal-resources.h>

/* SPI Presets */

const LL_SPI_InitTypeDef furi_hal_spi_preset_2edge_low_8m = {
    .Mode = LL_SPI_MODE_MASTER,
    .TransferDirection = LL_SPI_FULL_DUPLEX,
    .DataWidth = LL_SPI_DATAWIDTH_8BIT,
    .ClockPolarity = LL_SPI_POLARITY_LOW,
    .ClockPhase = LL_SPI_PHASE_2EDGE,
    .NSS = LL_SPI_NSS_SOFT,
    .BaudRate = LL_SPI_BAUDRATEPRESCALER_DIV8,
    .BitOrder = LL_SPI_MSB_FIRST,
    .CRCCalculation = LL_SPI_CRCCALCULATION_DISABLE,
    .CRCPoly = 7,
};

const LL_SPI_InitTypeDef furi_hal_spi_preset_1edge_low_8m = {
    .Mode = LL_SPI_MODE_MASTER,
    .TransferDirection = LL_SPI_FULL_DUPLEX,
    .DataWidth = LL_SPI_DATAWIDTH_8BIT,
    .ClockPolarity = LL_SPI_POLARITY_LOW,
    .ClockPhase = LL_SPI_PHASE_1EDGE,
    .NSS = LL_SPI_NSS_SOFT,
    .BaudRate = LL_SPI_BAUDRATEPRESCALER_DIV8,
    .BitOrder = LL_SPI_MSB_FIRST,
    .CRCCalculation = LL_SPI_CRCCALCULATION_DISABLE,
    .CRCPoly = 7,
};

const LL_SPI_InitTypeDef furi_hal_spi_preset_1edge_low_4m = {
    .Mode = LL_SPI_MODE_MASTER,
    .TransferDirection = LL_SPI_FULL_DUPLEX,
    .DataWidth = LL_SPI_DATAWIDTH_8BIT,
    .ClockPolarity = LL_SPI_POLARITY_LOW,
    .ClockPhase = LL_SPI_PHASE_1EDGE,
    .NSS = LL_SPI_NSS_SOFT,
    .BaudRate = LL_SPI_BAUDRATEPRESCALER_DIV16,
    .BitOrder = LL_SPI_MSB_FIRST,
    .CRCCalculation = LL_SPI_CRCCALCULATION_DISABLE,
    .CRCPoly = 7,
};

const LL_SPI_InitTypeDef furi_hal_spi_preset_1edge_low_16m = {
    .Mode = LL_SPI_MODE_MASTER,
    .TransferDirection = LL_SPI_FULL_DUPLEX,
    .DataWidth = LL_SPI_DATAWIDTH_8BIT,
    .ClockPolarity = LL_SPI_POLARITY_LOW,
    .ClockPhase = LL_SPI_PHASE_1EDGE,
    .NSS = LL_SPI_NSS_SOFT,
    .BaudRate = LL_SPI_BAUDRATEPRESCALER_DIV2,
    .BitOrder = LL_SPI_MSB_FIRST,
    .CRCCalculation = LL_SPI_CRCCALCULATION_DISABLE,
    .CRCPoly = 7,
};

const LL_SPI_InitTypeDef furi_hal_spi_preset_1edge_low_2m = {
    .Mode = LL_SPI_MODE_MASTER,
    .TransferDirection = LL_SPI_FULL_DUPLEX,
    .DataWidth = LL_SPI_DATAWIDTH_8BIT,
    .ClockPolarity = LL_SPI_POLARITY_LOW,
    .ClockPhase = LL_SPI_PHASE_1EDGE,
    .NSS = LL_SPI_NSS_SOFT,
    .BaudRate = LL_SPI_BAUDRATEPRESCALER_DIV32,
    .BitOrder = LL_SPI_MSB_FIRST,
    .CRCCalculation = LL_SPI_CRCCALCULATION_DISABLE,
    .CRCPoly = 7,
};

/* SPI Buses */

osMutexId_t furi_hal_spi_bus_r_mutex = NULL;

static void furi_hal_spi_bus_r_event_callback(FuriHalSpiBus* bus, FuriHalSpiBusEvent event) {
    if (event == FuriHalSpiBusEventInit) {
        furi_hal_spi_bus_r_mutex = osMutexNew(NULL);
        FURI_CRITICAL_ENTER();
        LL_APB2_GRP1_EnableClock(LL_APB2_GRP1_PERIPH_SPI1);
        LL_APB2_GRP1_ForceReset(LL_APB2_GRP1_PERIPH_SPI1);
        FURI_CRITICAL_EXIT();
        bus->current_handle = NULL;
    } else if (event == FuriHalSpiBusEventDeinit) {
        furi_check(osMutexDelete(furi_hal_spi_bus_r_mutex));
    } else if (event == FuriHalSpiBusEventLock) {
        furi_check(osMutexAcquire(furi_hal_spi_bus_r_mutex, osWaitForever) == osOK);
    } else if (event == FuriHalSpiBusEventUnlock) {
        furi_check(osMutexRelease(furi_hal_spi_bus_r_mutex) == osOK);
    } else if (event == FuriHalSpiBusEventActivate) {
        FURI_CRITICAL_ENTER();
        LL_APB2_GRP1_ReleaseReset(LL_APB2_GRP1_PERIPH_SPI1);
        FURI_CRITICAL_EXIT();
    } else if (event == FuriHalSpiBusEventDeactivate) {
        FURI_CRITICAL_ENTER();
        LL_APB2_GRP1_ForceReset(LL_APB2_GRP1_PERIPH_SPI1);
        FURI_CRITICAL_EXIT();
    }
}

FuriHalSpiBus furi_hal_spi_bus_r = {
    .spi=SPI1,
    .callback = furi_hal_spi_bus_r_event_callback,
};

osMutexId_t furi_hal_spi_bus_d_mutex = NULL;

static void furi_hal_spi_bus_d_event_callback(FuriHalSpiBus* bus, FuriHalSpiBusEvent event) {
    if (event == FuriHalSpiBusEventInit) {
        furi_hal_spi_bus_d_mutex = osMutexNew(NULL);
        FURI_CRITICAL_ENTER();
        LL_APB1_GRP1_EnableClock(LL_APB1_GRP1_PERIPH_SPI2);
        LL_APB1_GRP1_ForceReset(LL_APB1_GRP1_PERIPH_SPI2);
        FURI_CRITICAL_EXIT();
        bus->current_handle = NULL;
    } else if (event == FuriHalSpiBusEventDeinit) {
        furi_check(osMutexDelete(furi_hal_spi_bus_d_mutex));
    } else if (event == FuriHalSpiBusEventLock) {
        furi_check(osMutexAcquire(furi_hal_spi_bus_d_mutex, osWaitForever) == osOK);
    } else if (event == FuriHalSpiBusEventUnlock) {
        furi_check(osMutexRelease(furi_hal_spi_bus_d_mutex) == osOK);
    } else if (event == FuriHalSpiBusEventActivate) {
        FURI_CRITICAL_ENTER();
        LL_APB1_GRP1_ReleaseReset(LL_APB1_GRP1_PERIPH_SPI2);
        FURI_CRITICAL_EXIT();
    } else if (event == FuriHalSpiBusEventDeactivate) {
        FURI_CRITICAL_ENTER();
        LL_APB1_GRP1_ForceReset(LL_APB1_GRP1_PERIPH_SPI2);
        FURI_CRITICAL_EXIT();
    }
}

FuriHalSpiBus furi_hal_spi_bus_d = {
    .spi=SPI2,
    .callback = furi_hal_spi_bus_d_event_callback,
};

/* SPI Bus Handles */

inline static void furi_hal_spi_bus_r_handle_event_callback(FuriHalSpiBusHandle* handle, FuriHalSpiBusHandleEvent event, const LL_SPI_InitTypeDef* preset) {
    if (event == FuriHalSpiBusHandleEventInit) {
        hal_gpio_write(handle->cs, true);
        hal_gpio_init(handle->cs, GpioModeOutputPushPull, GpioPullNo, GpioSpeedVeryHigh);
    } else if (event == FuriHalSpiBusHandleEventDeinit) {
        hal_gpio_write(handle->cs, true);
        hal_gpio_init(handle->cs, GpioModeAnalog, GpioPullNo, GpioSpeedLow);
    } else if (event == FuriHalSpiBusHandleEventActivate) {
        LL_SPI_Init(handle->bus->spi, (LL_SPI_InitTypeDef*)preset);
        LL_SPI_SetRxFIFOThreshold(handle->bus->spi, LL_SPI_RX_FIFO_TH_QUARTER);
        LL_SPI_Enable(handle->bus->spi);

        hal_gpio_init_ex(handle->miso, GpioModeAltFunctionPushPull, GpioPullNo, GpioSpeedVeryHigh, GpioAltFn5SPI1);
        hal_gpio_init_ex(handle->mosi, GpioModeAltFunctionPushPull, GpioPullNo, GpioSpeedVeryHigh, GpioAltFn5SPI1);
        hal_gpio_init_ex(handle->sck, GpioModeAltFunctionPushPull, GpioPullNo, GpioSpeedVeryHigh, GpioAltFn5SPI1);

        hal_gpio_write(handle->cs, false);
    } else if (event == FuriHalSpiBusHandleEventDeactivate) {
        hal_gpio_write(handle->cs, true);

        hal_gpio_init(handle->miso, GpioModeAnalog, GpioPullNo, GpioSpeedLow);
        hal_gpio_init(handle->mosi, GpioModeAnalog, GpioPullNo, GpioSpeedLow);
        hal_gpio_init(handle->sck, GpioModeAnalog, GpioPullNo, GpioSpeedLow);

        LL_SPI_Disable(handle->bus->spi);
    }
}

static void furi_hal_spi_bus_handle_subghz_event_callback(FuriHalSpiBusHandle* handle, FuriHalSpiBusHandleEvent event) {
    furi_hal_spi_bus_r_handle_event_callback(handle, event, &furi_hal_spi_preset_1edge_low_8m);
}

FuriHalSpiBusHandle furi_hal_spi_bus_handle_subghz = {
    .bus=&furi_hal_spi_bus_r,
    .callback=furi_hal_spi_bus_handle_subghz_event_callback,
    .miso=&gpio_spi_r_miso,
    .mosi=&gpio_spi_r_mosi,
    .sck=&gpio_spi_r_sck,
    .cs=&gpio_subghz_cs,
};

static void furi_hal_spi_bus_handle_nfc_event_callback(FuriHalSpiBusHandle* handle, FuriHalSpiBusHandleEvent event) {
    furi_hal_spi_bus_r_handle_event_callback(handle, event, &furi_hal_spi_preset_2edge_low_8m);
}

FuriHalSpiBusHandle furi_hal_spi_bus_handle_nfc = {
    .bus=&furi_hal_spi_bus_r,
    .callback=furi_hal_spi_bus_handle_nfc_event_callback,
    .miso=&gpio_spi_r_miso,
    .mosi=&gpio_spi_r_mosi,
    .sck=&gpio_spi_r_sck,
    .cs=&gpio_nfc_cs,
};

static void furi_hal_spi_bus_handle_external_event_callback(FuriHalSpiBusHandle* handle, FuriHalSpiBusHandleEvent event) {
    furi_hal_spi_bus_r_handle_event_callback(handle, event, &furi_hal_spi_preset_1edge_low_2m);
}

FuriHalSpiBusHandle furi_hal_spi_bus_handle_external = {
    .bus=&furi_hal_spi_bus_r,
    .callback=furi_hal_spi_bus_handle_external_event_callback,
    .miso=&gpio_ext_pa6,
    .mosi=&gpio_ext_pa7,
    .sck=&gpio_ext_pb3,
    .cs=&gpio_ext_pa4,
};

inline static void furi_hal_spi_bus_d_handle_event_callback(FuriHalSpiBusHandle* handle, FuriHalSpiBusHandleEvent event, const LL_SPI_InitTypeDef* preset) {
    if (event == FuriHalSpiBusHandleEventInit) {
        hal_gpio_write(handle->cs, true);
        hal_gpio_init(handle->cs, GpioModeOutputPushPull, GpioPullUp, GpioSpeedVeryHigh);

        hal_gpio_init_ex(handle->miso, GpioModeAltFunctionPushPull, GpioPullNo, GpioSpeedVeryHigh, GpioAltFn5SPI2);
        hal_gpio_init_ex(handle->mosi, GpioModeAltFunctionPushPull, GpioPullNo, GpioSpeedVeryHigh, GpioAltFn5SPI2);
        hal_gpio_init_ex(handle->sck, GpioModeAltFunctionPushPull, GpioPullNo, GpioSpeedVeryHigh, GpioAltFn5SPI2);

    } else if (event == FuriHalSpiBusHandleEventDeinit) {
        hal_gpio_write(handle->cs, true);
        hal_gpio_init(handle->cs, GpioModeAnalog, GpioPullUp, GpioSpeedLow);
    } else if (event == FuriHalSpiBusHandleEventActivate) {
        LL_SPI_Init(handle->bus->spi, (LL_SPI_InitTypeDef*)preset);
        LL_SPI_SetRxFIFOThreshold(handle->bus->spi, LL_SPI_RX_FIFO_TH_QUARTER);
        LL_SPI_Enable(handle->bus->spi);
        hal_gpio_write(handle->cs, false);
    } else if (event == FuriHalSpiBusHandleEventDeactivate) {
        hal_gpio_write(handle->cs, true);
        LL_SPI_Disable(handle->bus->spi);
    }
}

static void furi_hal_spi_bus_handle_display_event_callback(FuriHalSpiBusHandle* handle, FuriHalSpiBusHandleEvent event) {
    furi_hal_spi_bus_d_handle_event_callback(handle, event, &furi_hal_spi_preset_1edge_low_4m);
}

FuriHalSpiBusHandle furi_hal_spi_bus_handle_display = {
    .bus=&furi_hal_spi_bus_d,
    .callback=furi_hal_spi_bus_handle_display_event_callback,
    .miso=&gpio_spi_d_miso,
    .mosi=&gpio_spi_d_mosi,
    .sck=&gpio_spi_d_sck,
    .cs=&gpio_display_cs,
};

static void furi_hal_spi_bus_handle_sd_fast_event_callback(FuriHalSpiBusHandle* handle, FuriHalSpiBusHandleEvent event) {
    furi_hal_spi_bus_d_handle_event_callback(handle, event, &furi_hal_spi_preset_1edge_low_16m);
}

FuriHalSpiBusHandle furi_hal_spi_bus_handle_sd_fast = {
    .bus=&furi_hal_spi_bus_d,
    .callback=furi_hal_spi_bus_handle_sd_fast_event_callback,
    .miso=&gpio_spi_d_miso,
    .mosi=&gpio_spi_d_mosi,
    .sck=&gpio_spi_d_sck,
    .cs=&gpio_sdcard_cs,
};

static void furi_hal_spi_bus_handle_sd_slow_event_callback(FuriHalSpiBusHandle* handle, FuriHalSpiBusHandleEvent event) {
    furi_hal_spi_bus_d_handle_event_callback(handle, event, &furi_hal_spi_preset_1edge_low_2m);
}

FuriHalSpiBusHandle furi_hal_spi_bus_handle_sd_slow = {
    .bus=&furi_hal_spi_bus_d,
    .callback=furi_hal_spi_bus_handle_sd_slow_event_callback,
    .miso=&gpio_spi_d_miso,
    .mosi=&gpio_spi_d_mosi,
    .sck=&gpio_spi_d_sck,
    .cs=&gpio_sdcard_cs,
};
