#include "gpio.h"

void MX_GPIO_Init(void) {
    GPIO_InitTypeDef GPIO_InitStruct = {0};

    /*Configure GPIO pin : PtPin */
    GPIO_InitStruct.Mode = GPIO_MODE_IT_RISING_FALLING;
    GPIO_InitStruct.Pull = GPIO_PULLUP;
    GPIO_InitStruct.Pin = BUTTON_BACK_Pin;
    HAL_GPIO_Init(BUTTON_BACK_GPIO_Port, &GPIO_InitStruct);

    /*Configure GPIO pin : PtPin */
    GPIO_InitStruct.Mode = GPIO_MODE_IT_RISING_FALLING;
    GPIO_InitStruct.Pull = GPIO_NOPULL;
    GPIO_InitStruct.Pin = BUTTON_OK_Pin;
    HAL_GPIO_Init(BUTTON_OK_GPIO_Port, &GPIO_InitStruct);

    /*Configure GPIO pins : PCPin PCPin PCPin PCPin */
    GPIO_InitStruct.Mode = GPIO_MODE_ANALOG;
    GPIO_InitStruct.Pull = GPIO_NOPULL;
    GPIO_InitStruct.Pin = PC0_Pin;
    HAL_GPIO_Init(PC0_GPIO_Port, &GPIO_InitStruct);
    GPIO_InitStruct.Pin = PC1_Pin;
    HAL_GPIO_Init(PC1_GPIO_Port, &GPIO_InitStruct);
    GPIO_InitStruct.Pin = PC3_Pin;
    HAL_GPIO_Init(PC3_GPIO_Port, &GPIO_InitStruct);
    GPIO_InitStruct.Pin = VIBRO_Pin;
    HAL_GPIO_Init(VIBRO_GPIO_Port, &GPIO_InitStruct);

    /* RF_SW_0 */
    HAL_GPIO_WritePin(RF_SW_0_GPIO_Port, RF_SW_0_Pin, GPIO_PIN_RESET);
    GPIO_InitStruct.Mode = GPIO_MODE_OUTPUT_PP;
    GPIO_InitStruct.Pull = GPIO_NOPULL;
    GPIO_InitStruct.Speed = GPIO_SPEED_FREQ_LOW;
    GPIO_InitStruct.Pin = RF_SW_0_Pin;
    HAL_GPIO_Init(RF_SW_0_GPIO_Port, &GPIO_InitStruct);

    /* PERIPH_POWER */
    HAL_GPIO_WritePin(PERIPH_POWER_GPIO_Port, PERIPH_POWER_Pin, GPIO_PIN_SET);
    GPIO_InitStruct.Mode = GPIO_MODE_OUTPUT_OD;
    GPIO_InitStruct.Pull = GPIO_NOPULL;
    GPIO_InitStruct.Speed = GPIO_SPEED_FREQ_LOW;
    GPIO_InitStruct.Pin = PERIPH_POWER_Pin;
    HAL_GPIO_Init(PERIPH_POWER_GPIO_Port, &GPIO_InitStruct);

    /*Configure GPIO pins : PAPin PAPin PAPin */
    GPIO_InitStruct.Mode = GPIO_MODE_ANALOG;
    GPIO_InitStruct.Pull = GPIO_NOPULL;
    GPIO_InitStruct.Pin = PA4_Pin;
    HAL_GPIO_Init(PA4_GPIO_Port, &GPIO_InitStruct);
    GPIO_InitStruct.Pin = PA6_Pin;
    HAL_GPIO_Init(PA6_GPIO_Port, &GPIO_InitStruct);
    GPIO_InitStruct.Pin = PA7_Pin;
    HAL_GPIO_Init(PA7_GPIO_Port, &GPIO_InitStruct);

    /*Configure GPIO pin : PtPin */
    GPIO_InitStruct.Pin = RFID_PULL_Pin;
    GPIO_InitStruct.Mode = GPIO_MODE_IT_RISING;
    GPIO_InitStruct.Pull = GPIO_NOPULL;
    HAL_GPIO_Init(RFID_PULL_GPIO_Port, &GPIO_InitStruct);

    /*Configure GPIO pins : PBPin PBPin PBPin */
    GPIO_InitStruct.Mode = GPIO_MODE_ANALOG;
    GPIO_InitStruct.Pull = GPIO_NOPULL;
    GPIO_InitStruct.Pin = PB2_Pin;
    HAL_GPIO_Init(PB2_GPIO_Port, &GPIO_InitStruct);
    GPIO_InitStruct.Pin = iBTN_Pin;
    HAL_GPIO_Init(iBTN_GPIO_Port, &GPIO_InitStruct);
    GPIO_InitStruct.Pin = PB3_Pin;
    HAL_GPIO_Init(PB3_GPIO_Port, &GPIO_InitStruct);

    /*Configure GPIO pins : PBPin PBPin PBPin PBPin */
    GPIO_InitStruct.Mode = GPIO_MODE_IT_RISING_FALLING;
    GPIO_InitStruct.Pull = GPIO_PULLUP;
    GPIO_InitStruct.Pin = BUTTON_UP_Pin;
    HAL_GPIO_Init(BUTTON_UP_GPIO_Port, &GPIO_InitStruct);
    GPIO_InitStruct.Pin = BUTTON_LEFT_Pin;
    HAL_GPIO_Init(BUTTON_LEFT_GPIO_Port, &GPIO_InitStruct);
    GPIO_InitStruct.Pin = BUTTON_RIGHT_Pin;
    HAL_GPIO_Init(BUTTON_RIGHT_GPIO_Port, &GPIO_InitStruct);

    /*Configure GPIO pins : PBPin PBPin PBPin PBPin */
    GPIO_InitStruct.Pin = BUTTON_DOWN_Pin;
    GPIO_InitStruct.Mode = GPIO_MODE_IT_RISING_FALLING;
    GPIO_InitStruct.Pull = GPIO_PULLUP;
    HAL_GPIO_Init(BUTTON_DOWN_GPIO_Port, &GPIO_InitStruct);

    /* DISPLAY_RST */
    HAL_GPIO_WritePin(DISPLAY_RST_GPIO_Port, DISPLAY_RST_Pin, GPIO_PIN_RESET);
    GPIO_InitStruct.Mode = GPIO_MODE_OUTPUT_PP;
    GPIO_InitStruct.Pull = GPIO_NOPULL;
    GPIO_InitStruct.Speed = GPIO_SPEED_FREQ_LOW;
    GPIO_InitStruct.Pin = DISPLAY_RST_Pin;
    HAL_GPIO_Init(DISPLAY_RST_GPIO_Port, &GPIO_InitStruct);

    /* DISPLAY_DI */
    HAL_GPIO_WritePin(DISPLAY_DI_GPIO_Port, DISPLAY_DI_Pin, GPIO_PIN_RESET);
    GPIO_InitStruct.Mode = GPIO_MODE_OUTPUT_PP;
    GPIO_InitStruct.Pull = GPIO_NOPULL;
    GPIO_InitStruct.Speed = GPIO_SPEED_FREQ_LOW;
    GPIO_InitStruct.Pin = DISPLAY_DI_Pin;
    HAL_GPIO_Init(DISPLAY_DI_GPIO_Port, &GPIO_InitStruct);

    /* SD_CD */
    GPIO_InitStruct.Pin = SD_CD_Pin;
    GPIO_InitStruct.Mode = GPIO_MODE_INPUT;
    GPIO_InitStruct.Pull = GPIO_NOPULL;
    HAL_GPIO_Init(SD_CD_GPIO_Port, &GPIO_InitStruct);

    /* Enable all NVIC lines related to gpio */
    HAL_NVIC_SetPriority(EXTI0_IRQn, 5, 0);
    HAL_NVIC_EnableIRQ(EXTI0_IRQn);

    HAL_NVIC_SetPriority(EXTI1_IRQn, 5, 0);
    HAL_NVIC_EnableIRQ(EXTI1_IRQn);

    HAL_NVIC_SetPriority(EXTI2_IRQn, 5, 0);
    HAL_NVIC_EnableIRQ(EXTI2_IRQn);

    HAL_NVIC_SetPriority(EXTI3_IRQn, 5, 0);
    HAL_NVIC_EnableIRQ(EXTI3_IRQn);

    HAL_NVIC_SetPriority(EXTI4_IRQn, 5, 0);
    HAL_NVIC_EnableIRQ(EXTI4_IRQn);

    HAL_NVIC_SetPriority(EXTI9_5_IRQn, 5, 0);
    HAL_NVIC_EnableIRQ(EXTI9_5_IRQn);

    HAL_NVIC_SetPriority(EXTI15_10_IRQn, 5, 0);
    HAL_NVIC_EnableIRQ(EXTI15_10_IRQn);
}
