#pragma once
#include "main.h"
#include <cmsis_os.h>

void api_hal_spi_init();
void api_hal_spi_lock(SPI_HandleTypeDef* spi);
void api_hal_spi_unlock(SPI_HandleTypeDef* spi); 