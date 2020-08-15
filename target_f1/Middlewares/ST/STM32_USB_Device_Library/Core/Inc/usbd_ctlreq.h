/**
  ******************************************************************************
  * @file    usbd_req.h
  * @author  MCD Application Team
  * @brief   Header file for the usbd_req.c file
  ******************************************************************************
  * @attention
  *
  * <h2><center>&copy; Copyright (c) 2015 STMicroelectronics.
  * All rights reserved.</center></h2>
  *
  * This software component is licensed by ST under Ultimate Liberty license
  * SLA0044, the "License"; You may not use this file except in compliance with
  * the License. You may obtain a copy of the License at:
  *                      http://www.st.com/SLA0044
  *
  ******************************************************************************
  */

/* Define to prevent recursive inclusion -------------------------------------*/
#ifndef __USB_REQUEST_H
#define __USB_REQUEST_H

#ifdef __cplusplus
 extern "C" {
#endif

/* Includes ------------------------------------------------------------------*/
#include  "usbd_def.h"


/** @addtogroup STM32_USB_DEVICE_LIBRARY
  * @{
  */

/** @defgroup USBD_REQ
  * @brief header file for the usbd_req.c file
  * @{
  */

/** @defgroup USBD_REQ_Exported_Defines
  * @{
  */
/**
  * @}
  */


/** @defgroup USBD_REQ_Exported_Types
  * @{
  */
/**
  * @}
  */



/** @defgroup USBD_REQ_Exported_Macros
  * @{
  */
/**
  * @}
  */

/** @defgroup USBD_REQ_Exported_Variables
  * @{
  */
/**
  * @}
  */

/** @defgroup USBD_REQ_Exported_FunctionsPrototype
  * @{
  */

USBD_StatusTypeDef  USBD_StdDevReq (USBD_HandleTypeDef  *pdev, USBD_SetupReqTypedef  *req);
USBD_StatusTypeDef  USBD_StdItfReq (USBD_HandleTypeDef  *pdev, USBD_SetupReqTypedef  *req);
USBD_StatusTypeDef  USBD_StdEPReq  (USBD_HandleTypeDef  *pdev, USBD_SetupReqTypedef  *req);


void USBD_CtlError  (USBD_HandleTypeDef  *pdev, USBD_SetupReqTypedef *req);

void USBD_ParseSetupRequest (USBD_SetupReqTypedef *req, uint8_t *pdata);

void USBD_GetString         (uint8_t *desc, uint8_t *unicode, uint16_t *len);
/**
  * @}
  */

#ifdef __cplusplus
}
#endif

#endif /* __USB_REQUEST_H */

/**
  * @}
  */

/**
* @}
*/


/************************ (C) COPYRIGHT STMicroelectronics *****END OF FILE****/
