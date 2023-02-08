#include <furi_hal_spi_config.h>
#include <furi_hal_resources.h>
#include <furi_hal_spi.h>
#include <furi.h>

#define TAG "FuriHalSpiConfig"

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

FuriMutex* furi_hal_spi_bus_r_mutex = NULL;

void furi_hal_spi_config_init_early() {
    furi_hal_spi_bus_init(&furi_hal_spi_bus_d);
    furi_hal_spi_bus_handle_init(&furi_hal_spi_bus_handle_display);
}

void furi_hal_spi_config_deinit_early() {
    furi_hal_spi_bus_handle_deinit(&furi_hal_spi_bus_handle_display);
    furi_hal_spi_bus_deinit(&furi_hal_spi_bus_d);
}

void furi_hal_spi_config_init() {
    furi_hal_spi_bus_handle_init(&furi_hal_spi_bus_handle_sd_fast);
    furi_hal_spi_bus_handle_init(&furi_hal_spi_bus_handle_sd_slow);

    FURI_LOG_I(TAG, "Init OK");
}

static void furi_hal_spi_bus_r_event_callback(FuriHalSpiBus* bus, FuriHalSpiBusEvent event) {
    if(event == FuriHalSpiBusEventInit) {
        furi_hal_spi_bus_r_mutex = furi_mutex_alloc(FuriMutexTypeNormal);
        FURI_CRITICAL_ENTER();
        LL_APB2_GRP1_ForceReset(LL_APB2_GRP1_PERIPH_SPI1);
        FURI_CRITICAL_EXIT();
        bus->current_handle = NULL;
    } else if(event == FuriHalSpiBusEventDeinit) {
        furi_mutex_free(furi_hal_spi_bus_r_mutex);
        FURI_CRITICAL_ENTER();
        LL_APB2_GRP1_ForceReset(LL_APB2_GRP1_PERIPH_SPI1);
        LL_APB2_GRP1_ReleaseReset(LL_APB2_GRP1_PERIPH_SPI1);
        FURI_CRITICAL_EXIT();
    } else if(event == FuriHalSpiBusEventLock) {
        furi_check(furi_mutex_acquire(furi_hal_spi_bus_r_mutex, FuriWaitForever) == FuriStatusOk);
    } else if(event == FuriHalSpiBusEventUnlock) {
        furi_check(furi_mutex_release(furi_hal_spi_bus_r_mutex) == FuriStatusOk);
    } else if(event == FuriHalSpiBusEventActivate) {
        FURI_CRITICAL_ENTER();
        LL_APB2_GRP1_ReleaseReset(LL_APB2_GRP1_PERIPH_SPI1);
        FURI_CRITICAL_EXIT();
    } else if(event == FuriHalSpiBusEventDeactivate) {
        FURI_CRITICAL_ENTER();
        LL_APB2_GRP1_ForceReset(LL_APB2_GRP1_PERIPH_SPI1);
        FURI_CRITICAL_EXIT();
    }
}

FuriHalSpiBus furi_hal_spi_bus_r = {
    .spi = SPI1,
    .callback = furi_hal_spi_bus_r_event_callback,
};

FuriMutex* furi_hal_spi_bus_d_mutex = NULL;

static void furi_hal_spi_bus_d_event_callback(FuriHalSpiBus* bus, FuriHalSpiBusEvent event) {
    if(event == FuriHalSpiBusEventInit) {
        furi_hal_spi_bus_d_mutex = furi_mutex_alloc(FuriMutexTypeNormal);
        FURI_CRITICAL_ENTER();
        LL_APB1_GRP1_ForceReset(LL_APB1_GRP1_PERIPH_SPI2);
        FURI_CRITICAL_EXIT();
        bus->current_handle = NULL;
    } else if(event == FuriHalSpiBusEventDeinit) {
        furi_mutex_free(furi_hal_spi_bus_d_mutex);
        FURI_CRITICAL_ENTER();
        LL_APB1_GRP1_ForceReset(LL_APB1_GRP1_PERIPH_SPI2);
        LL_APB1_GRP1_ReleaseReset(LL_APB1_GRP1_PERIPH_SPI2);
        FURI_CRITICAL_EXIT();
    } else if(event == FuriHalSpiBusEventLock) {
        furi_check(furi_mutex_acquire(furi_hal_spi_bus_d_mutex, FuriWaitForever) == FuriStatusOk);
    } else if(event == FuriHalSpiBusEventUnlock) {
        furi_check(furi_mutex_release(furi_hal_spi_bus_d_mutex) == FuriStatusOk);
    } else if(event == FuriHalSpiBusEventActivate) {
        FURI_CRITICAL_ENTER();
        LL_APB1_GRP1_ReleaseReset(LL_APB1_GRP1_PERIPH_SPI2);
        FURI_CRITICAL_EXIT();
    } else if(event == FuriHalSpiBusEventDeactivate) {
        FURI_CRITICAL_ENTER();
        LL_APB1_GRP1_ForceReset(LL_APB1_GRP1_PERIPH_SPI2);
        FURI_CRITICAL_EXIT();
    }
}

FuriHalSpiBus furi_hal_spi_bus_d = {
    .spi = SPI2,
    .callback = furi_hal_spi_bus_d_event_callback,
};

/* SPI Bus Handles */

inline static void furi_hal_spi_bus_r_handle_event_callback(
    FuriHalSpiBusHandle* handle,
    FuriHalSpiBusHandleEvent event,
    const LL_SPI_InitTypeDef* preset) {
    if(event == FuriHalSpiBusHandleEventInit) {
        furi_hal_gpio_write(handle->cs, true);
        furi_hal_gpio_init(handle->cs, GpioModeOutputPushPull, GpioPullNo, GpioSpeedVeryHigh);
    } else if(event == FuriHalSpiBusHandleEventDeinit) {
        furi_hal_gpio_write(handle->cs, true);
        furi_hal_gpio_init(handle->cs, GpioModeAnalog, GpioPullNo, GpioSpeedLow);
    } else if(event == FuriHalSpiBusHandleEventActivate) {
        LL_SPI_Init(handle->bus->spi, (LL_SPI_InitTypeDef*)preset);
        LL_SPI_SetRxFIFOThreshold(handle->bus->spi, LL_SPI_RX_FIFO_TH_QUARTER);
        LL_SPI_Enable(handle->bus->spi);

        furi_hal_gpio_init_ex(
            handle->miso,
            GpioModeAltFunctionPushPull,
            GpioPullNo,
            GpioSpeedVeryHigh,
            GpioAltFn5SPI1);
        furi_hal_gpio_init_ex(
            handle->mosi,
            GpioModeAltFunctionPushPull,
            GpioPullNo,
            GpioSpeedVeryHigh,
            GpioAltFn5SPI1);
        furi_hal_gpio_init_ex(
            handle->sck,
            GpioModeAltFunctionPushPull,
            GpioPullNo,
            GpioSpeedVeryHigh,
            GpioAltFn5SPI1);

        furi_hal_gpio_write(handle->cs, false);
    } else if(event == FuriHalSpiBusHandleEventDeactivate) {
        furi_hal_gpio_write(handle->cs, true);

        furi_hal_gpio_init(handle->miso, GpioModeAnalog, GpioPullNo, GpioSpeedLow);
        furi_hal_gpio_init(handle->mosi, GpioModeAnalog, GpioPullNo, GpioSpeedLow);
        furi_hal_gpio_init(handle->sck, GpioModeAnalog, GpioPullNo, GpioSpeedLow);

        LL_SPI_Disable(handle->bus->spi);
    }
}

inline static void furi_hal_spi_bus_nfc_handle_event_callback(
    FuriHalSpiBusHandle* handle,
    FuriHalSpiBusHandleEvent event,
    const LL_SPI_InitTypeDef* preset) {
    if(event == FuriHalSpiBusHandleEventInit) {
        // Configure GPIOs in normal SPI mode
        furi_hal_gpio_init_ex(
            handle->miso,
            GpioModeAltFunctionPushPull,
            GpioPullNo,
            GpioSpeedVeryHigh,
            GpioAltFn5SPI1);
        furi_hal_gpio_init_ex(
            handle->mosi,
            GpioModeAltFunctionPushPull,
            GpioPullNo,
            GpioSpeedVeryHigh,
            GpioAltFn5SPI1);
        furi_hal_gpio_init_ex(
            handle->sck,
            GpioModeAltFunctionPushPull,
            GpioPullNo,
            GpioSpeedVeryHigh,
            GpioAltFn5SPI1);
        furi_hal_gpio_write(handle->cs, true);
        furi_hal_gpio_init(handle->cs, GpioModeOutputPushPull, GpioPullNo, GpioSpeedVeryHigh);
    } else if(event == FuriHalSpiBusHandleEventDeinit) {
        // Configure GPIOs for st25r3916 Transparent mode
        furi_hal_gpio_init(handle->sck, GpioModeInput, GpioPullUp, GpioSpeedLow);
        furi_hal_gpio_init(handle->miso, GpioModeInput, GpioPullUp, GpioSpeedLow);
        furi_hal_gpio_init(handle->cs, GpioModeInput, GpioPullUp, GpioSpeedLow);
        furi_hal_gpio_write(handle->mosi, false);
        furi_hal_gpio_init(handle->mosi, GpioModeOutputPushPull, GpioPullNo, GpioSpeedVeryHigh);
    } else if(event == FuriHalSpiBusHandleEventActivate) {
        LL_SPI_Init(handle->bus->spi, (LL_SPI_InitTypeDef*)preset);
        LL_SPI_SetRxFIFOThreshold(handle->bus->spi, LL_SPI_RX_FIFO_TH_QUARTER);
        LL_SPI_Enable(handle->bus->spi);

        furi_hal_gpio_init_ex(
            handle->miso,
            GpioModeAltFunctionPushPull,
            GpioPullNo,
            GpioSpeedVeryHigh,
            GpioAltFn5SPI1);
        furi_hal_gpio_init_ex(
            handle->mosi,
            GpioModeAltFunctionPushPull,
            GpioPullNo,
            GpioSpeedVeryHigh,
            GpioAltFn5SPI1);
        furi_hal_gpio_init_ex(
            handle->sck,
            GpioModeAltFunctionPushPull,
            GpioPullNo,
            GpioSpeedVeryHigh,
            GpioAltFn5SPI1);

    } else if(event == FuriHalSpiBusHandleEventDeactivate) {
        furi_hal_gpio_init(handle->miso, GpioModeAnalog, GpioPullNo, GpioSpeedLow);
        furi_hal_gpio_init(handle->mosi, GpioModeAnalog, GpioPullNo, GpioSpeedLow);
        furi_hal_gpio_init(handle->sck, GpioModeAnalog, GpioPullNo, GpioSpeedLow);

        LL_SPI_Disable(handle->bus->spi);
    }
}

static void furi_hal_spi_bus_handle_external_event_callback(
    FuriHalSpiBusHandle* handle,
    FuriHalSpiBusHandleEvent event) {
    furi_hal_spi_bus_r_handle_event_callback(handle, event, &furi_hal_spi_preset_1edge_low_2m);
}

FuriHalSpiBusHandle furi_hal_spi_bus_handle_external = {
    .bus = &furi_hal_spi_bus_r,
    .callback = furi_hal_spi_bus_handle_external_event_callback,
    .miso = &gpio_ext_pa6,
    .mosi = &gpio_ext_pa7,
    .sck = &gpio_ext_pb3,
    .cs = &gpio_ext_pa4,
};

inline static void furi_hal_spi_bus_d_handle_event_callback(
    FuriHalSpiBusHandle* handle,
    FuriHalSpiBusHandleEvent event,
    const LL_SPI_InitTypeDef* preset) {
    if(event == FuriHalSpiBusHandleEventInit) {
        furi_hal_gpio_write(handle->cs, true);
        furi_hal_gpio_init(handle->cs, GpioModeOutputPushPull, GpioPullUp, GpioSpeedVeryHigh);

        furi_hal_gpio_init_ex(
            handle->miso,
            GpioModeAltFunctionPushPull,
            GpioPullNo,
            GpioSpeedVeryHigh,
            GpioAltFn5SPI2);
        furi_hal_gpio_init_ex(
            handle->mosi,
            GpioModeAltFunctionPushPull,
            GpioPullNo,
            GpioSpeedVeryHigh,
            GpioAltFn5SPI2);
        furi_hal_gpio_init_ex(
            handle->sck,
            GpioModeAltFunctionPushPull,
            GpioPullNo,
            GpioSpeedVeryHigh,
            GpioAltFn5SPI2);

    } else if(event == FuriHalSpiBusHandleEventDeinit) {
        furi_hal_gpio_write(handle->cs, true);
        furi_hal_gpio_init(handle->cs, GpioModeAnalog, GpioPullUp, GpioSpeedLow);
    } else if(event == FuriHalSpiBusHandleEventActivate) {
        LL_SPI_Init(handle->bus->spi, (LL_SPI_InitTypeDef*)preset);
        LL_SPI_SetRxFIFOThreshold(handle->bus->spi, LL_SPI_RX_FIFO_TH_QUARTER);
        LL_SPI_Enable(handle->bus->spi);
        furi_hal_gpio_write(handle->cs, false);
    } else if(event == FuriHalSpiBusHandleEventDeactivate) {
        furi_hal_gpio_write(handle->cs, true);
        LL_SPI_Disable(handle->bus->spi);
    }
}

static void furi_hal_spi_bus_handle_display_event_callback(
    FuriHalSpiBusHandle* handle,
    FuriHalSpiBusHandleEvent event) {
    furi_hal_spi_bus_d_handle_event_callback(handle, event, &furi_hal_spi_preset_1edge_low_4m);
}

FuriHalSpiBusHandle furi_hal_spi_bus_handle_display = {
    .bus = &furi_hal_spi_bus_d,
    .callback = furi_hal_spi_bus_handle_display_event_callback,
    .miso = &gpio_spi_d_miso,
    .mosi = &gpio_spi_d_mosi,
    .sck = &gpio_spi_d_sck,
    .cs = &gpio_display_cs,
};

static void furi_hal_spi_bus_handle_sd_fast_event_callback(
    FuriHalSpiBusHandle* handle,
    FuriHalSpiBusHandleEvent event) {
    furi_hal_spi_bus_d_handle_event_callback(handle, event, &furi_hal_spi_preset_1edge_low_16m);
}

FuriHalSpiBusHandle furi_hal_spi_bus_handle_sd_fast = {
    .bus = &furi_hal_spi_bus_d,
    .callback = furi_hal_spi_bus_handle_sd_fast_event_callback,
    .miso = &gpio_spi_d_miso,
    .mosi = &gpio_spi_d_mosi,
    .sck = &gpio_spi_d_sck,
    .cs = &gpio_sdcard_cs,
};

static void furi_hal_spi_bus_handle_sd_slow_event_callback(
    FuriHalSpiBusHandle* handle,
    FuriHalSpiBusHandleEvent event) {
    furi_hal_spi_bus_d_handle_event_callback(handle, event, &furi_hal_spi_preset_1edge_low_2m);
}

FuriHalSpiBusHandle furi_hal_spi_bus_handle_sd_slow = {
    .bus = &furi_hal_spi_bus_d,
    .callback = furi_hal_spi_bus_handle_sd_slow_event_callback,
    .miso = &gpio_spi_d_miso,
    .mosi = &gpio_spi_d_mosi,
    .sck = &gpio_spi_d_sck,
    .cs = &gpio_sdcard_cs,
};
