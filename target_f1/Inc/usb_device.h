/* USER CODE BEGIN Header */
/**
  ******************************************************************************
  * @file           : usb_device.h
  * @version        : v2.0_Cube
  * @brief          : Header for usb_device.c file.
  ******************************************************************************
  * @attention
  *
  * <h2><center>&copy; Copyright (c) 2020 STMicroelectronics.
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
#ifndef __USB_DEVICE__H__
#define __USB_DEVICE__H__

#ifdef __cplusplus
 extern "C" {
#endif

/* Includes ------------------------------------------------------------------*/
#include "stm32l4xx.h"
#include "stm32l4xx_hal.h"
#include "usbd_def.h"

/* USER CODE BEGIN INCLUDE */

/* USER CODE END INCLUDE */

/** @addtogroup USBD_OTG_DRIVER
  * @{
  */

/** @defgroup USBD_DEVICE USBD_DEVICE
  * @brief Device file for Usb otg low level driver.
  * @{
  */

/** @defgroup USBD_DEVICE_Exported_Variables USBD_DEVICE_Exported_Variables
  * @brief Public variables.
  * @{
  */

/* Private variables ---------------------------------------------------------*/
/* USER CODE BEGIN PV */

/* USER CODE END PV */

/* Private function prototypes -----------------------------------------------*/
/* USER CODE BEGIN PFP */

/* USER CODE END PFP */

/*
 * -- Insert your variables declaration here --
 */
/* USER CODE BEGIN VARIABLES */

/* USER CODE END VARIABLES */
/**
  * @}
  */

/** @defgroup USBD_DEVICE_Exported_FunctionsPrototype USBD_DEVICE_Exported_FunctionsPrototype
  * @brief Declaration of public functions for Usb device.
  * @{
  */

/** USB Device initialization function. */
void MX_USB_DEVICE_Init(void);

/*
 * -- Insert functions declaration here --
 */
/* USER CODE BEGIN FD */

/* USER CODE END FD */
/**
  * @}
  */

/**
  * @}
  */

/**
  * @}
  */

#ifdef __cplusplus
}
#endif

#endif /* __USB_DEVICE__H__ */

/************************ (C) COPYRIGHT STMicroelectronics *****END OF FILE****/
