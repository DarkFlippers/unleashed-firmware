#include "avr_isp_spi_sw.h"

#include <furi.h>

#define AVR_ISP_SPI_SW_MISO &gpio_ext_pa6
#define AVR_ISP_SPI_SW_MOSI &gpio_ext_pa7
#define AVR_ISP_SPI_SW_SCK &gpio_ext_pb3
#define AVR_ISP_RESET &gpio_ext_pb2

struct AvrIspSpiSw {
    AvrIspSpiSwSpeed speed_wait_time;
    const GpioPin* miso;
    const GpioPin* mosi;
    const GpioPin* sck;
    const GpioPin* res;
};

AvrIspSpiSw* avr_isp_spi_sw_init(AvrIspSpiSwSpeed speed) {
    AvrIspSpiSw* instance = malloc(sizeof(AvrIspSpiSw));
    instance->speed_wait_time = speed;
    instance->miso = AVR_ISP_SPI_SW_MISO;
    instance->mosi = AVR_ISP_SPI_SW_MOSI;
    instance->sck = AVR_ISP_SPI_SW_SCK;
    instance->res = AVR_ISP_RESET;

    furi_hal_gpio_init(instance->miso, GpioModeInput, GpioPullNo, GpioSpeedVeryHigh);
    furi_hal_gpio_write(instance->mosi, false);
    furi_hal_gpio_init(instance->mosi, GpioModeOutputPushPull, GpioPullNo, GpioSpeedVeryHigh);
    furi_hal_gpio_write(instance->sck, false);
    furi_hal_gpio_init(instance->sck, GpioModeOutputPushPull, GpioPullNo, GpioSpeedVeryHigh);
    furi_hal_gpio_init(instance->res, GpioModeOutputPushPull, GpioPullNo, GpioSpeedVeryHigh);

    return instance;
}

void avr_isp_spi_sw_free(AvrIspSpiSw* instance) {
    furi_assert(instance);
    furi_hal_gpio_init(instance->res, GpioModeAnalog, GpioPullNo, GpioSpeedLow);
    furi_hal_gpio_init(instance->miso, GpioModeAnalog, GpioPullNo, GpioSpeedLow);
    furi_hal_gpio_init(instance->mosi, GpioModeAnalog, GpioPullNo, GpioSpeedLow);
    furi_hal_gpio_init(instance->sck, GpioModeAnalog, GpioPullNo, GpioSpeedLow);
    free(instance);
}

uint8_t avr_isp_spi_sw_txrx(AvrIspSpiSw* instance, uint8_t data) {
    furi_assert(instance);
    for(uint8_t i = 0; i < 8; ++i) {
        furi_hal_gpio_write(instance->mosi, (data & 0x80) ? true : false);

        furi_hal_gpio_write(instance->sck, true);
        if(instance->speed_wait_time != AvrIspSpiSwSpeed1Mhz)
            furi_delay_us(instance->speed_wait_time - 1);

        data = (data << 1) | furi_hal_gpio_read(instance->miso); //-V792

        furi_hal_gpio_write(instance->sck, false);
        if(instance->speed_wait_time != AvrIspSpiSwSpeed1Mhz)
            furi_delay_us(instance->speed_wait_time - 1);
    }
    return data;
}

void avr_isp_spi_sw_res_set(AvrIspSpiSw* instance, bool state) {
    furi_assert(instance);
    furi_hal_gpio_write(instance->res, state);
}

void avr_isp_spi_sw_sck_set(AvrIspSpiSw* instance, bool state) {
    furi_assert(instance);
    furi_hal_gpio_write(instance->sck, state);
}