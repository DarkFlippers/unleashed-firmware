/**
  ******************************************************************************
  * @file    usbd_ioreq.c
  * @author  MCD Application Team
  * @brief   This file provides the IO requests APIs for control endpoints.
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

/* Includes ------------------------------------------------------------------*/
#include "usbd_ioreq.h"

/** @addtogroup STM32_USB_DEVICE_LIBRARY
  * @{
  */


/** @defgroup USBD_IOREQ
  * @brief control I/O requests module
  * @{
  */

/** @defgroup USBD_IOREQ_Private_TypesDefinitions
  * @{
  */
/**
  * @}
  */


/** @defgroup USBD_IOREQ_Private_Defines
  * @{
  */

/**
  * @}
  */


/** @defgroup USBD_IOREQ_Private_Macros
  * @{
  */
/**
  * @}
  */


/** @defgroup USBD_IOREQ_Private_Variables
  * @{
  */

/**
  * @}
  */


/** @defgroup USBD_IOREQ_Private_FunctionPrototypes
  * @{
  */
/**
  * @}
  */


/** @defgroup USBD_IOREQ_Private_Functions
  * @{
  */

/**
* @brief  USBD_CtlSendData
*         send data on the ctl pipe
* @param  pdev: device instance
* @param  buff: pointer to data buffer
* @param  len: length of data to be sent
* @retval status
*/
USBD_StatusTypeDef USBD_CtlSendData (USBD_HandleTypeDef *pdev, uint8_t *pbuf,
                                     uint16_t len)
{
  /* Set EP0 State */
  pdev->ep0_state = USBD_EP0_DATA_IN;
  pdev->ep_in[0].total_length = len;
  pdev->ep_in[0].rem_length   = len;

 /* Start the transfer */
  USBD_LL_Transmit (pdev, 0x00U, pbuf, len);

  return USBD_OK;
}

/**
* @brief  USBD_CtlContinueSendData
*         continue sending data on the ctl pipe
* @param  pdev: device instance
* @param  buff: pointer to data buffer
* @param  len: length of data to be sent
* @retval status
*/
USBD_StatusTypeDef USBD_CtlContinueSendData (USBD_HandleTypeDef *pdev,
                                             uint8_t *pbuf, uint16_t len)
{
 /* Start the next transfer */
  USBD_LL_Transmit (pdev, 0x00U, pbuf, len);

  return USBD_OK;
}

/**
* @brief  USBD_CtlPrepareRx
*         receive data on the ctl pipe
* @param  pdev: device instance
* @param  buff: pointer to data buffer
* @param  len: length of data to be received
* @retval status
*/
USBD_StatusTypeDef USBD_CtlPrepareRx (USBD_HandleTypeDef *pdev, uint8_t *pbuf,
                                      uint16_t len)
{
  /* Set EP0 State */
  pdev->ep0_state = USBD_EP0_DATA_OUT;
  pdev->ep_out[0].total_length = len;
  pdev->ep_out[0].rem_length   = len;

  /* Start the transfer */
  USBD_LL_PrepareReceive (pdev, 0U, pbuf, len);

  return USBD_OK;
}

/**
* @brief  USBD_CtlContinueRx
*         continue receive data on the ctl pipe
* @param  pdev: device instance
* @param  buff: pointer to data buffer
* @param  len: length of data to be received
* @retval status
*/
USBD_StatusTypeDef USBD_CtlContinueRx (USBD_HandleTypeDef *pdev, uint8_t *pbuf,
                                       uint16_t len)
{
  USBD_LL_PrepareReceive(pdev, 0U, pbuf, len);

  return USBD_OK;
}

/**
* @brief  USBD_CtlSendStatus
*         send zero lzngth packet on the ctl pipe
* @param  pdev: device instance
* @retval status
*/
USBD_StatusTypeDef USBD_CtlSendStatus (USBD_HandleTypeDef *pdev)
{
  /* Set EP0 State */
  pdev->ep0_state = USBD_EP0_STATUS_IN;

  /* Start the transfer */
  USBD_LL_Transmit(pdev, 0x00U, NULL, 0U);

  return USBD_OK;
}

/**
* @brief  USBD_CtlReceiveStatus
*         receive zero lzngth packet on the ctl pipe
* @param  pdev: device instance
* @retval status
*/
USBD_StatusTypeDef USBD_CtlReceiveStatus (USBD_HandleTypeDef *pdev)
{
  /* Set EP0 State */
  pdev->ep0_state = USBD_EP0_STATUS_OUT;

 /* Start the transfer */
  USBD_LL_PrepareReceive (pdev, 0U, NULL, 0U);

  return USBD_OK;
}

/**
* @brief  USBD_GetRxCount
*         returns the received data length
* @param  pdev: device instance
* @param  ep_addr: endpoint address
* @retval Rx Data blength
*/
uint32_t USBD_GetRxCount (USBD_HandleTypeDef *pdev, uint8_t ep_addr)
{
  return USBD_LL_GetRxDataSize(pdev, ep_addr);
}

/**
  * @}
  */


/**
  * @}
  */


/**
  * @}
  */

/************************ (C) COPYRIGHT STMicroelectronics *****END OF FILE****/
