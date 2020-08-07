/**
  ******************************************************************************
  * @file    usbd_core.c
  * @author  MCD Application Team
  * @brief   This file provides all the USBD core functions.
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
#include "usbd_core.h"

/** @addtogroup STM32_USBD_DEVICE_LIBRARY
* @{
*/


/** @defgroup USBD_CORE
* @brief usbd core module
* @{
*/

/** @defgroup USBD_CORE_Private_TypesDefinitions
* @{
*/
/**
* @}
*/


/** @defgroup USBD_CORE_Private_Defines
* @{
*/

/**
* @}
*/


/** @defgroup USBD_CORE_Private_Macros
* @{
*/
/**
* @}
*/




/** @defgroup USBD_CORE_Private_FunctionPrototypes
* @{
*/

/**
* @}
*/

/** @defgroup USBD_CORE_Private_Variables
* @{
*/

/**
* @}
*/

/** @defgroup USBD_CORE_Private_Functions
* @{
*/

/**
* @brief  USBD_Init
*         Initializes the device stack and load the class driver
* @param  pdev: device instance
* @param  pdesc: Descriptor structure address
* @param  id: Low level core index
* @retval None
*/
USBD_StatusTypeDef USBD_Init(USBD_HandleTypeDef *pdev, USBD_DescriptorsTypeDef *pdesc, uint8_t id)
{
  /* Check whether the USB Host handle is valid */
  if(pdev == NULL)
  {
#if (USBD_DEBUG_LEVEL > 1U)
    USBD_ErrLog("Invalid Device handle");
#endif
    return USBD_FAIL;
  }

  /* Unlink previous class*/
  if(pdev->pClass != NULL)
  {
    pdev->pClass = NULL;
  }

  /* Assign USBD Descriptors */
  if(pdesc != NULL)
  {
    pdev->pDesc = pdesc;
  }

  /* Set Device initial State */
  pdev->dev_state  = USBD_STATE_DEFAULT;
  pdev->id = id;
  /* Initialize low level driver */
  USBD_LL_Init(pdev);

  return USBD_OK;
}

/**
* @brief  USBD_DeInit
*         Re-Initialize th device library
* @param  pdev: device instance
* @retval status: status
*/
USBD_StatusTypeDef USBD_DeInit(USBD_HandleTypeDef *pdev)
{
  /* Set Default State */
  pdev->dev_state  = USBD_STATE_DEFAULT;

  /* Free Class Resources */
  pdev->pClass->DeInit(pdev, (uint8_t)pdev->dev_config);

    /* Stop the low level driver  */
  USBD_LL_Stop(pdev);

  /* Initialize low level driver */
  USBD_LL_DeInit(pdev);

  return USBD_OK;
}

/**
  * @brief  USBD_RegisterClass
  *         Link class driver to Device Core.
  * @param  pDevice : Device Handle
  * @param  pclass: Class handle
  * @retval USBD Status
  */
USBD_StatusTypeDef  USBD_RegisterClass(USBD_HandleTypeDef *pdev, USBD_ClassTypeDef *pclass)
{
  USBD_StatusTypeDef   status = USBD_OK;
  if(pclass != 0)
  {
    /* link the class to the USB Device handle */
    pdev->pClass = pclass;
    status = USBD_OK;
  }
  else
  {
#if (USBD_DEBUG_LEVEL > 1U)
    USBD_ErrLog("Invalid Class handle");
#endif
    status = USBD_FAIL;
  }

  return status;
}

/**
  * @brief  USBD_Start
  *         Start the USB Device Core.
  * @param  pdev: Device Handle
  * @retval USBD Status
  */
USBD_StatusTypeDef  USBD_Start  (USBD_HandleTypeDef *pdev)
{

  /* Start the low level driver  */
  USBD_LL_Start(pdev);

  return USBD_OK;
}

/**
  * @brief  USBD_Stop
  *         Stop the USB Device Core.
  * @param  pdev: Device Handle
  * @retval USBD Status
  */
USBD_StatusTypeDef  USBD_Stop   (USBD_HandleTypeDef *pdev)
{
  /* Free Class Resources */
  pdev->pClass->DeInit(pdev, (uint8_t)pdev->dev_config);

  /* Stop the low level driver  */
  USBD_LL_Stop(pdev);

  return USBD_OK;
}

/**
* @brief  USBD_RunTestMode
*         Launch test mode process
* @param  pdev: device instance
* @retval status
*/
USBD_StatusTypeDef  USBD_RunTestMode (USBD_HandleTypeDef  *pdev)
{
  /* Prevent unused argument compilation warning */
  UNUSED(pdev);

  return USBD_OK;
}

/**
* @brief  USBD_SetClassConfig
*        Configure device and start the interface
* @param  pdev: device instance
* @param  cfgidx: configuration index
* @retval status
*/

USBD_StatusTypeDef USBD_SetClassConfig(USBD_HandleTypeDef  *pdev, uint8_t cfgidx)
{
  USBD_StatusTypeDef   ret = USBD_FAIL;

  if(pdev->pClass != NULL)
  {
    /* Set configuration  and Start the Class*/
    if(pdev->pClass->Init(pdev, cfgidx) == 0U)
    {
      ret = USBD_OK;
    }
  }

  return ret;
}

/**
* @brief  USBD_ClrClassConfig
*         Clear current configuration
* @param  pdev: device instance
* @param  cfgidx: configuration index
* @retval status: USBD_StatusTypeDef
*/
USBD_StatusTypeDef USBD_ClrClassConfig(USBD_HandleTypeDef  *pdev, uint8_t cfgidx)
{
  /* Clear configuration  and De-initialize the Class process*/
  pdev->pClass->DeInit(pdev, cfgidx);
  return USBD_OK;
}


/**
* @brief  USBD_SetupStage
*         Handle the setup stage
* @param  pdev: device instance
* @retval status
*/
USBD_StatusTypeDef USBD_LL_SetupStage(USBD_HandleTypeDef *pdev, uint8_t *psetup)
{
  USBD_ParseSetupRequest(&pdev->request, psetup);

  pdev->ep0_state = USBD_EP0_SETUP;

  pdev->ep0_data_len = pdev->request.wLength;

  switch (pdev->request.bmRequest & 0x1FU)
  {
  case USB_REQ_RECIPIENT_DEVICE:
    USBD_StdDevReq (pdev, &pdev->request);
    break;

  case USB_REQ_RECIPIENT_INTERFACE:
    USBD_StdItfReq(pdev, &pdev->request);
    break;

  case USB_REQ_RECIPIENT_ENDPOINT:
    USBD_StdEPReq(pdev, &pdev->request);
    break;

  default:
    USBD_LL_StallEP(pdev, (pdev->request.bmRequest & 0x80U));
    break;
  }

  return USBD_OK;
}

/**
* @brief  USBD_DataOutStage
*         Handle data OUT stage
* @param  pdev: device instance
* @param  epnum: endpoint index
* @retval status
*/
USBD_StatusTypeDef USBD_LL_DataOutStage(USBD_HandleTypeDef *pdev,
                                        uint8_t epnum, uint8_t *pdata)
{
  USBD_EndpointTypeDef    *pep;

  if(epnum == 0U)
  {
    pep = &pdev->ep_out[0];

    if ( pdev->ep0_state == USBD_EP0_DATA_OUT)
    {
      if(pep->rem_length > pep->maxpacket)
      {
        pep->rem_length -=  pep->maxpacket;

        USBD_CtlContinueRx (pdev,
                            pdata,
                            (uint16_t)MIN(pep->rem_length, pep->maxpacket));
      }
      else
      {
        if((pdev->pClass->EP0_RxReady != NULL)&&
           (pdev->dev_state == USBD_STATE_CONFIGURED))
        {
          pdev->pClass->EP0_RxReady(pdev);
        }
        USBD_CtlSendStatus(pdev);
      }
    }
    else
    {
      if (pdev->ep0_state == USBD_EP0_STATUS_OUT)
      {
        /*
         * STATUS PHASE completed, update ep0_state to idle
         */
        pdev->ep0_state = USBD_EP0_IDLE;
        USBD_LL_StallEP(pdev, 0U);
      }
    }
  }
  else if((pdev->pClass->DataOut != NULL) &&
          (pdev->dev_state == USBD_STATE_CONFIGURED))
  {
    pdev->pClass->DataOut(pdev, epnum);
  }
  else
  {
    /* should never be in this condition */
    return USBD_FAIL;
  }

  return USBD_OK;
}

/**
* @brief  USBD_DataInStage
*         Handle data in stage
* @param  pdev: device instance
* @param  epnum: endpoint index
* @retval status
*/
USBD_StatusTypeDef USBD_LL_DataInStage(USBD_HandleTypeDef *pdev, uint8_t epnum,
                                       uint8_t *pdata)
{
  USBD_EndpointTypeDef *pep;

  if(epnum == 0U)
  {
    pep = &pdev->ep_in[0];

    if ( pdev->ep0_state == USBD_EP0_DATA_IN)
    {
      if(pep->rem_length > pep->maxpacket)
      {
        pep->rem_length -= pep->maxpacket;

        USBD_CtlContinueSendData (pdev, pdata, (uint16_t)pep->rem_length);

        /* Prepare endpoint for premature end of transfer */
        USBD_LL_PrepareReceive (pdev, 0U, NULL, 0U);
      }
      else
      { /* last packet is MPS multiple, so send ZLP packet */
        if((pep->total_length % pep->maxpacket == 0U) &&
           (pep->total_length >= pep->maxpacket) &&
           (pep->total_length < pdev->ep0_data_len))
        {
          USBD_CtlContinueSendData(pdev, NULL, 0U);
          pdev->ep0_data_len = 0U;

          /* Prepare endpoint for premature end of transfer */
          USBD_LL_PrepareReceive (pdev, 0U, NULL, 0U);
        }
        else
        {
          if((pdev->pClass->EP0_TxSent != NULL)&&
             (pdev->dev_state == USBD_STATE_CONFIGURED))
          {
            pdev->pClass->EP0_TxSent(pdev);
          }
          USBD_LL_StallEP(pdev, 0x80U);
          USBD_CtlReceiveStatus(pdev);
        }
      }
    }
    else
    {
      if ((pdev->ep0_state == USBD_EP0_STATUS_IN) ||
          (pdev->ep0_state == USBD_EP0_IDLE))
      {
        USBD_LL_StallEP(pdev, 0x80U);
      }
    }

    if (pdev->dev_test_mode == 1U)
    {
      USBD_RunTestMode(pdev);
      pdev->dev_test_mode = 0U;
    }
  }
  else if((pdev->pClass->DataIn != NULL) &&
          (pdev->dev_state == USBD_STATE_CONFIGURED))
  {
    pdev->pClass->DataIn(pdev, epnum);
  }
  else
  {
    /* should never be in this condition */
    return USBD_FAIL;
  }

  return USBD_OK;
}

/**
* @brief  USBD_LL_Reset
*         Handle Reset event
* @param  pdev: device instance
* @retval status
*/

USBD_StatusTypeDef USBD_LL_Reset(USBD_HandleTypeDef  *pdev)
{
  /* Open EP0 OUT */
  USBD_LL_OpenEP(pdev, 0x00U, USBD_EP_TYPE_CTRL, USB_MAX_EP0_SIZE);
  pdev->ep_out[0x00U & 0xFU].is_used = 1U;

  pdev->ep_out[0].maxpacket = USB_MAX_EP0_SIZE;

  /* Open EP0 IN */
  USBD_LL_OpenEP(pdev, 0x80U, USBD_EP_TYPE_CTRL, USB_MAX_EP0_SIZE);
  pdev->ep_in[0x80U & 0xFU].is_used = 1U;

  pdev->ep_in[0].maxpacket = USB_MAX_EP0_SIZE;
  /* Upon Reset call user call back */
  pdev->dev_state = USBD_STATE_DEFAULT;
  pdev->ep0_state = USBD_EP0_IDLE;
  pdev->dev_config= 0U;
  pdev->dev_remote_wakeup = 0U;

  if (pdev->pClassData)
  {
    pdev->pClass->DeInit(pdev, (uint8_t)pdev->dev_config);
  }

  return USBD_OK;
}

/**
* @brief  USBD_LL_Reset
*         Handle Reset event
* @param  pdev: device instance
* @retval status
*/
USBD_StatusTypeDef USBD_LL_SetSpeed(USBD_HandleTypeDef  *pdev, USBD_SpeedTypeDef speed)
{
  pdev->dev_speed = speed;
  return USBD_OK;
}

/**
* @brief  USBD_Suspend
*         Handle Suspend event
* @param  pdev: device instance
* @retval status
*/

USBD_StatusTypeDef USBD_LL_Suspend(USBD_HandleTypeDef  *pdev)
{
  pdev->dev_old_state =  pdev->dev_state;
  pdev->dev_state  = USBD_STATE_SUSPENDED;
  return USBD_OK;
}

/**
* @brief  USBD_Resume
*         Handle Resume event
* @param  pdev: device instance
* @retval status
*/

USBD_StatusTypeDef USBD_LL_Resume(USBD_HandleTypeDef  *pdev)
{
  pdev->dev_state = pdev->dev_old_state;
  return USBD_OK;
}

/**
* @brief  USBD_SOF
*         Handle SOF event
* @param  pdev: device instance
* @retval status
*/

USBD_StatusTypeDef USBD_LL_SOF(USBD_HandleTypeDef  *pdev)
{
  if(pdev->dev_state == USBD_STATE_CONFIGURED)
  {
    if(pdev->pClass->SOF != NULL)
    {
      pdev->pClass->SOF(pdev);
    }
  }
  return USBD_OK;
}

/**
* @brief  USBD_IsoINIncomplete
*         Handle iso in incomplete event
* @param  pdev: device instance
* @retval status
*/
USBD_StatusTypeDef USBD_LL_IsoINIncomplete(USBD_HandleTypeDef  *pdev, uint8_t epnum)
{
  /* Prevent unused arguments compilation warning */
  UNUSED(pdev);
  UNUSED(epnum);

  return USBD_OK;
}

/**
* @brief  USBD_IsoOUTIncomplete
*         Handle iso out incomplete event
* @param  pdev: device instance
* @retval status
*/
USBD_StatusTypeDef USBD_LL_IsoOUTIncomplete(USBD_HandleTypeDef  *pdev, uint8_t epnum)
{
  /* Prevent unused arguments compilation warning */
  UNUSED(pdev);
  UNUSED(epnum);

  return USBD_OK;
}

/**
* @brief  USBD_DevConnected
*         Handle device connection event
* @param  pdev: device instance
* @retval status
*/
USBD_StatusTypeDef USBD_LL_DevConnected(USBD_HandleTypeDef  *pdev)
{
  /* Prevent unused argument compilation warning */
  UNUSED(pdev);

  return USBD_OK;
}

/**
* @brief  USBD_DevDisconnected
*         Handle device disconnection event
* @param  pdev: device instance
* @retval status
*/
USBD_StatusTypeDef USBD_LL_DevDisconnected(USBD_HandleTypeDef  *pdev)
{
  /* Free Class Resources */
  pdev->dev_state = USBD_STATE_DEFAULT;
  pdev->pClass->DeInit(pdev, (uint8_t)pdev->dev_config);

  return USBD_OK;
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

