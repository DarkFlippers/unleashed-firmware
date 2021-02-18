#pragma once

#ifdef __cplusplus
extern "C" {
#endif

typedef struct {
    SPI_HandleTypeDef* spi;
    const SPI_InitTypeDef config;
} SPIDevice;

extern const SPIDevice sd_fast_spi;
extern const SPIDevice sd_slow_spi;
extern const SPIDevice display_spi;

#ifdef __cplusplus
}
#endif