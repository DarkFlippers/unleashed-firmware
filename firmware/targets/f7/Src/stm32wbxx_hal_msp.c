#include "main.h"

void HAL_MspInit(void) {
    HAL_NVIC_SetPriority(PendSV_IRQn, 15, 0);

    HAL_NVIC_SetPriority(RCC_IRQn, 5, 0);
    HAL_NVIC_EnableIRQ(RCC_IRQn);

    HAL_NVIC_SetPriority(HSEM_IRQn, 5, 0);
    HAL_NVIC_EnableIRQ(HSEM_IRQn);
}
