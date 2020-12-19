#include "api-hal-spi.h"

osMutexId_t spi_mutex_r;
osMutexId_t spi_mutex_d;

extern SPI_HandleTypeDef SPI_R;
extern SPI_HandleTypeDef SPI_D;

void api_hal_spi_init() {
    spi_mutex_r = osMutexNew(NULL);
    spi_mutex_d = osMutexNew(NULL);
}

void api_hal_spi_lock(SPI_HandleTypeDef* spi) {
    if(spi == &SPI_D) {
        osMutexAcquire(spi_mutex_d, osWaitForever);
    } else if(spi == &SPI_R) {
        osMutexAcquire(spi_mutex_r, osWaitForever);
    } else {
        Error_Handler();
    }
}

void api_hal_spi_unlock(SPI_HandleTypeDef* spi) {
    if(spi == &SPI_D) {
        osMutexRelease(spi_mutex_d);
    } else if(spi == &SPI_R) {
        osMutexRelease(spi_mutex_r);
    } else {
        Error_Handler();
    }
}