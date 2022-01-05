/* USER CODE BEGIN Header */
/**
  ******************************************************************************
  * @file    hw_if.h
  * @author  MCD Application Team
  * @brief   Hardware Interface
  ******************************************************************************
  * @attention
  *
  * <h2><center>&copy; Copyright (c) 2019 STMicroelectronics.
  * All rights reserved.</center></h2>
  *
  * This software component is licensed by ST under Ultimate Liberty license
  * SLA0044, the "License"; You may not use this file except in compliance with
  * the License. You may obtain a copy of the License at:
  *                             www.st.com/SLA0044
  *
  ******************************************************************************
  */
/* USER CODE END Header */

/* Define to prevent recursive inclusion -------------------------------------*/
#ifndef HW_IF_H
#define HW_IF_H

#ifdef __cplusplus
extern "C" {
#endif

/* Includes ------------------------------------------------------------------*/
#include "stm32wbxx.h"
#include "stm32wbxx_ll_exti.h"
#include "stm32wbxx_ll_system.h"
#include "stm32wbxx_ll_rcc.h"
#include "stm32wbxx_ll_ipcc.h"
#include "stm32wbxx_ll_bus.h"
#include "stm32wbxx_ll_pwr.h"
#include "stm32wbxx_ll_cortex.h"
#include "stm32wbxx_ll_utils.h"
#include "stm32wbxx_ll_hsem.h"
#include "stm32wbxx_ll_gpio.h"
#include "stm32wbxx_ll_rtc.h"

#ifdef USE_STM32WBXX_USB_DONGLE
#include "stm32wbxx_usb_dongle.h"
#endif
#ifdef USE_STM32WBXX_NUCLEO
#include "stm32wbxx_nucleo.h"
#endif
#ifdef USE_X_NUCLEO_EPD
#include "x_nucleo_epd.h"
#endif

/* Private includes ----------------------------------------------------------*/
/* USER CODE BEGIN Includes */

/* USER CODE END Includes */

/******************************************************************************
   * HW UART
   ******************************************************************************/
typedef enum {
    hw_uart1,
    hw_uart2,
    hw_lpuart1,
} hw_uart_id_t;

typedef enum {
    hw_uart_ok,
    hw_uart_error,
    hw_uart_busy,
    hw_uart_to,
} hw_status_t;

void HW_UART_Init(hw_uart_id_t hw_uart_id);
void HW_UART_Receive_IT(
    hw_uart_id_t hw_uart_id,
    uint8_t* pData,
    uint16_t Size,
    void (*Callback)(void));
void HW_UART_Transmit_IT(
    hw_uart_id_t hw_uart_id,
    uint8_t* pData,
    uint16_t Size,
    void (*Callback)(void));
hw_status_t
    HW_UART_Transmit(hw_uart_id_t hw_uart_id, uint8_t* p_data, uint16_t size, uint32_t timeout);
hw_status_t HW_UART_Transmit_DMA(
    hw_uart_id_t hw_uart_id,
    uint8_t* p_data,
    uint16_t size,
    void (*Callback)(void));
void HW_UART_Interrupt_Handler(hw_uart_id_t hw_uart_id);
void HW_UART_DMA_Interrupt_Handler(hw_uart_id_t hw_uart_id);

#ifdef __cplusplus
}
#endif

#endif /*HW_IF_H */

/************************ (C) COPYRIGHT STMicroelectronics *****END OF FILE****/
