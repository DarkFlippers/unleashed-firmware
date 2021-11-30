#include "furi-hal-sd.h"
#include <stm32wbxx_ll_gpio.h>
#include <furi.h>

void hal_sd_detect_init(void) {
    // low speed input with pullup
    LL_GPIO_SetPinMode(SD_CD_GPIO_Port, SD_CD_Pin, LL_GPIO_MODE_INPUT);
    LL_GPIO_SetPinSpeed(SD_CD_GPIO_Port, SD_CD_Pin, LL_GPIO_SPEED_FREQ_LOW);
    LL_GPIO_SetPinPull(SD_CD_GPIO_Port, SD_CD_Pin, LL_GPIO_PULL_UP);
}

void hal_sd_detect_set_low(void) {
    // low speed input with pullup
    LL_GPIO_SetPinMode(SD_CD_GPIO_Port, SD_CD_Pin, LL_GPIO_MODE_OUTPUT);
    LL_GPIO_SetPinOutputType(SD_CD_GPIO_Port, SD_CD_Pin, LL_GPIO_OUTPUT_OPENDRAIN);
    LL_GPIO_ResetOutputPin(SD_CD_GPIO_Port, SD_CD_Pin);
}

bool hal_sd_detect(void) {
    bool result = !(LL_GPIO_IsInputPinSet(SD_CD_GPIO_Port, SD_CD_Pin));
    return result;
}

FuriHalSpiBusHandle* furi_hal_sd_spi_handle = NULL;
