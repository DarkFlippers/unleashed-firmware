/**
  ******************************************************************************
  * @file    usbd_req.c
  * @author  MCD Application Team
  * @brief   This file provides the standard USB requests following chapter 9.
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
#include "usbd_ctlreq.h"
#include "usbd_ioreq.h"


/** @addtogroup STM32_USBD_STATE_DEVICE_LIBRARY
  * @{
  */


/** @defgroup USBD_REQ
  * @brief USB standard requests module
  * @{
  */

/** @defgroup USBD_REQ_Private_TypesDefinitions
  * @{
  */
/**
  * @}
  */


/** @defgroup USBD_REQ_Private_Defines
  * @{
  */

/**
  * @}
  */


/** @defgroup USBD_REQ_Private_Macros
  * @{
  */
/**
  * @}
  */


/** @defgroup USBD_REQ_Private_Variables
  * @{
  */
/**
  * @}
  */


/** @defgroup USBD_REQ_Private_FunctionPrototypes
  * @{
  */
static void USBD_GetDescriptor(USBD_HandleTypeDef *pdev ,
                               USBD_SetupReqTypedef *req);

static void USBD_SetAddress(USBD_HandleTypeDef *pdev ,
                            USBD_SetupReqTypedef *req);

static void USBD_SetConfig(USBD_HandleTypeDef *pdev ,
                           USBD_SetupReqTypedef *req);

static void USBD_GetConfig(USBD_HandleTypeDef *pdev ,
                           USBD_SetupReqTypedef *req);

static void USBD_GetStatus(USBD_HandleTypeDef *pdev ,
                           USBD_SetupReqTypedef *req);

static void USBD_SetFeature(USBD_HandleTypeDef *pdev ,
                            USBD_SetupReqTypedef *req);

static void USBD_ClrFeature(USBD_HandleTypeDef *pdev ,
                            USBD_SetupReqTypedef *req);

static uint8_t USBD_GetLen(uint8_t *buf);

/**
  * @}
  */


/** @defgroup USBD_REQ_Private_Functions
  * @{
  */


/**
* @brief  USBD_StdDevReq
*         Handle standard usb device requests
* @param  pdev: device instance
* @param  req: usb request
* @retval status
*/
USBD_StatusTypeDef  USBD_StdDevReq (USBD_HandleTypeDef *pdev , USBD_SetupReqTypedef  *req)
{
  USBD_StatusTypeDef ret = USBD_OK;

  switch (req->bmRequest & USB_REQ_TYPE_MASK)
  {
  case USB_REQ_TYPE_CLASS:
  case USB_REQ_TYPE_VENDOR:
    pdev->pClass->Setup(pdev, req);
    break;

  case USB_REQ_TYPE_STANDARD:

    switch (req->bRequest)
    {
    case USB_REQ_GET_DESCRIPTOR:

      USBD_GetDescriptor (pdev, req);
      break;

    case USB_REQ_SET_ADDRESS:
      USBD_SetAddress (pdev, req);
      break;

    case USB_REQ_SET_CONFIGURATION:
      USBD_SetConfig (pdev, req);
      break;

    case USB_REQ_GET_CONFIGURATION:
      USBD_GetConfig (pdev, req);
      break;

    case USB_REQ_GET_STATUS:
      USBD_GetStatus (pdev, req);
      break;


    case USB_REQ_SET_FEATURE:
      USBD_SetFeature (pdev, req);
      break;

    case USB_REQ_CLEAR_FEATURE:
      USBD_ClrFeature (pdev, req);
      break;

    default:
      USBD_CtlError(pdev, req);
      break;
    }
    break;

  default:
    USBD_CtlError(pdev, req);
    break;
  }

  return ret;
}

/**
* @brief  USBD_StdItfReq
*         Handle standard usb interface requests
* @param  pdev: device instance
* @param  req: usb request
* @retval status
*/
USBD_StatusTypeDef  USBD_StdItfReq (USBD_HandleTypeDef *pdev , USBD_SetupReqTypedef  *req)
{
  USBD_StatusTypeDef ret = USBD_OK;

  switch (req->bmRequest & USB_REQ_TYPE_MASK)
  {
  case USB_REQ_TYPE_CLASS:
  case USB_REQ_TYPE_VENDOR:
  case USB_REQ_TYPE_STANDARD:
    switch (pdev->dev_state)
    {
    case USBD_STATE_DEFAULT:
    case USBD_STATE_ADDRESSED:
    case USBD_STATE_CONFIGURED:

      if (LOBYTE(req->wIndex) <= USBD_MAX_NUM_INTERFACES)
      {
        ret = (USBD_StatusTypeDef)pdev->pClass->Setup (pdev, req);

        if ((req->wLength == 0U) && (ret == USBD_OK))
        {
          USBD_CtlSendStatus(pdev);
        }
      }
      else
      {
        USBD_CtlError(pdev, req);
      }
      break;

    default:
      USBD_CtlError(pdev, req);
      break;
    }
    break;

  default:
    USBD_CtlError(pdev, req);
    break;
  }

  return USBD_OK;
}

/**
* @brief  USBD_StdEPReq
*         Handle standard usb endpoint requests
* @param  pdev: device instance
* @param  req: usb request
* @retval status
*/
USBD_StatusTypeDef  USBD_StdEPReq (USBD_HandleTypeDef *pdev , USBD_SetupReqTypedef  *req)
{

  uint8_t   ep_addr;
  USBD_StatusTypeDef ret = USBD_OK;
  USBD_EndpointTypeDef   *pep;
  ep_addr  = LOBYTE(req->wIndex);

  switch (req->bmRequest & USB_REQ_TYPE_MASK)
  {

  case USB_REQ_TYPE_CLASS:
  case USB_REQ_TYPE_VENDOR:
    pdev->pClass->Setup (pdev, req);
    break;

  case USB_REQ_TYPE_STANDARD:
    /* Check if it is a class request */
    if ((req->bmRequest & 0x60U) == 0x20U)
    {
      ret = (USBD_StatusTypeDef)pdev->pClass->Setup (pdev, req);

      return ret;
    }

    switch (req->bRequest)
    {

    case USB_REQ_SET_FEATURE :

      switch (pdev->dev_state)
      {
      case USBD_STATE_ADDRESSED:
        if ((ep_addr != 0x00U) && (ep_addr != 0x80U))
        {
          USBD_LL_StallEP(pdev, ep_addr);
          USBD_LL_StallEP(pdev, 0x80U);
        }
        else
        {
          USBD_CtlError(pdev, req);
        }
        break;

      case USBD_STATE_CONFIGURED:
        if (req->wValue == USB_FEATURE_EP_HALT)
        {
          if ((ep_addr != 0x00U) && (ep_addr != 0x80U) && (req->wLength == 0x00U))
          {
            USBD_LL_StallEP(pdev, ep_addr);
          }
        }
        USBD_CtlSendStatus(pdev);

        break;

      default:
        USBD_CtlError(pdev, req);
        break;
      }
      break;

    case USB_REQ_CLEAR_FEATURE :

      switch (pdev->dev_state)
      {
      case USBD_STATE_ADDRESSED:
        if ((ep_addr != 0x00U) && (ep_addr != 0x80U))
        {
          USBD_LL_StallEP(pdev, ep_addr);
          USBD_LL_StallEP(pdev, 0x80U);
        }
        else
        {
          USBD_CtlError(pdev, req);
        }
        break;

      case USBD_STATE_CONFIGURED:
        if (req->wValue == USB_FEATURE_EP_HALT)
        {
          if ((ep_addr & 0x7FU) != 0x00U)
          {
            USBD_LL_ClearStallEP(pdev, ep_addr);
          }
          USBD_CtlSendStatus(pdev);
        }
        break;

      default:
        USBD_CtlError(pdev, req);
        break;
      }
      break;

    case USB_REQ_GET_STATUS:
      switch (pdev->dev_state)
      {
      case USBD_STATE_ADDRESSED:
        if ((ep_addr != 0x00U) && (ep_addr != 0x80U))
        {
          USBD_CtlError(pdev, req);
          break;
        }
        pep = ((ep_addr & 0x80U) == 0x80U) ? &pdev->ep_in[ep_addr & 0x7FU]:\
          &pdev->ep_out[ep_addr & 0x7FU];

          pep->status = 0x0000U;

          USBD_CtlSendData (pdev, (uint8_t *)(void *)&pep->status, 2U);
          break;

      case USBD_STATE_CONFIGURED:
        if((ep_addr & 0x80U) == 0x80U)
        {
          if (pdev->ep_in[ep_addr & 0xFU].is_used == 0U)
          {
            USBD_CtlError(pdev, req);
            break;
          }
        }
        else
        {
          if (pdev->ep_out[ep_addr & 0xFU].is_used == 0U)
          {
            USBD_CtlError(pdev, req);
            break;
          }
        }

        pep = ((ep_addr & 0x80U) == 0x80U) ? &pdev->ep_in[ep_addr & 0x7FU]:\
          &pdev->ep_out[ep_addr & 0x7FU];

          if ((ep_addr == 0x00U) || (ep_addr == 0x80U))
          {
            pep->status = 0x0000U;
          }
          else if(USBD_LL_IsStallEP(pdev, ep_addr))
          {
            pep->status = 0x0001U;
          }
          else
          {
            pep->status = 0x0000U;
          }

          USBD_CtlSendData (pdev, (uint8_t *)(void *)&pep->status, 2U);
          break;

      default:
        USBD_CtlError(pdev, req);
        break;
      }
      break;

    default:
      USBD_CtlError(pdev, req);
      break;
    }
    break;

  default:
    USBD_CtlError(pdev, req);
    break;
  }

  return ret;
}
/**
* @brief  USBD_GetDescriptor
*         Handle Get Descriptor requests
* @param  pdev: device instance
* @param  req: usb request
* @retval status
*/
static void USBD_GetDescriptor(USBD_HandleTypeDef *pdev ,
                               USBD_SetupReqTypedef *req)
{
  uint16_t len;
  uint8_t *pbuf;


  switch (req->wValue >> 8)
  {
#if (USBD_LPM_ENABLED == 1U)
  case USB_DESC_TYPE_BOS:
    pbuf = pdev->pDesc->GetBOSDescriptor(pdev->dev_speed, &len);
    break;
#endif
  case USB_DESC_TYPE_DEVICE:
    pbuf = pdev->pDesc->GetDeviceDescriptor(pdev->dev_speed, &len);
    break;

  case USB_DESC_TYPE_CONFIGURATION:
    if(pdev->dev_speed == USBD_SPEED_HIGH )
    {
      pbuf   = (uint8_t *)pdev->pClass->GetHSConfigDescriptor(&len);
      pbuf[1] = USB_DESC_TYPE_CONFIGURATION;
    }
    else
    {
      pbuf   = (uint8_t *)pdev->pClass->GetFSConfigDescriptor(&len);
      pbuf[1] = USB_DESC_TYPE_CONFIGURATION;
    }
    break;

  case USB_DESC_TYPE_STRING:
    switch ((uint8_t)(req->wValue))
    {
    case USBD_IDX_LANGID_STR:
     pbuf = pdev->pDesc->GetLangIDStrDescriptor(pdev->dev_speed, &len);
      break;

    case USBD_IDX_MFC_STR:
      pbuf = pdev->pDesc->GetManufacturerStrDescriptor(pdev->dev_speed, &len);
      break;

    case USBD_IDX_PRODUCT_STR:
      pbuf = pdev->pDesc->GetProductStrDescriptor(pdev->dev_speed, &len);
      break;

    case USBD_IDX_SERIAL_STR:
      pbuf = pdev->pDesc->GetSerialStrDescriptor(pdev->dev_speed, &len);
      break;

    case USBD_IDX_CONFIG_STR:
      pbuf = pdev->pDesc->GetConfigurationStrDescriptor(pdev->dev_speed, &len);
      break;

    case USBD_IDX_INTERFACE_STR:
      pbuf = pdev->pDesc->GetInterfaceStrDescriptor(pdev->dev_speed, &len);
      break;

    default:
#if (USBD_SUPPORT_USER_STRING == 1U)
      pbuf = pdev->pClass->GetUsrStrDescriptor(pdev, (req->wValue) , &len);
      break;
#else
       USBD_CtlError(pdev , req);
      return;
#endif
    }
    break;
  case USB_DESC_TYPE_DEVICE_QUALIFIER:

    if(pdev->dev_speed == USBD_SPEED_HIGH)
    {
      pbuf = (uint8_t *)pdev->pClass->GetDeviceQualifierDescriptor(&len);
      break;
    }
    else
    {
      USBD_CtlError(pdev , req);
      return;
    }

  case USB_DESC_TYPE_OTHER_SPEED_CONFIGURATION:
    if(pdev->dev_speed == USBD_SPEED_HIGH  )
    {
      pbuf   = (uint8_t *)pdev->pClass->GetOtherSpeedConfigDescriptor(&len);
      pbuf[1] = USB_DESC_TYPE_OTHER_SPEED_CONFIGURATION;
      break;
    }
    else
    {
      USBD_CtlError(pdev , req);
      return;
    }

  default:
     USBD_CtlError(pdev , req);
    return;
  }

  if((len != 0U) && (req->wLength != 0U))
  {

    len = MIN(len, req->wLength);

    USBD_CtlSendData (pdev, pbuf, len);
  }

  if(req->wLength == 0U)
  {
   USBD_CtlSendStatus(pdev);
  }
}

/**
* @brief  USBD_SetAddress
*         Set device address
* @param  pdev: device instance
* @param  req: usb request
* @retval status
*/
static void USBD_SetAddress(USBD_HandleTypeDef *pdev ,
                            USBD_SetupReqTypedef *req)
{
  uint8_t  dev_addr;

  if ((req->wIndex == 0U) && (req->wLength == 0U) && (req->wValue < 128U))
  {
    dev_addr = (uint8_t)(req->wValue) & 0x7FU;

    if (pdev->dev_state == USBD_STATE_CONFIGURED)
    {
      USBD_CtlError(pdev , req);
    }
    else
    {
      pdev->dev_address = dev_addr;
      USBD_LL_SetUSBAddress(pdev, dev_addr);
      USBD_CtlSendStatus(pdev);

      if (dev_addr != 0U)
      {
        pdev->dev_state = USBD_STATE_ADDRESSED;
      }
      else
      {
        pdev->dev_state = USBD_STATE_DEFAULT;
      }
    }
  }
  else
  {
    USBD_CtlError(pdev, req);
  }
}

/**
* @brief  USBD_SetConfig
*         Handle Set device configuration request
* @param  pdev: device instance
* @param  req: usb request
* @retval status
*/
static void USBD_SetConfig(USBD_HandleTypeDef *pdev, USBD_SetupReqTypedef *req)
{
  static uint8_t cfgidx;

  cfgidx = (uint8_t)(req->wValue);

  if (cfgidx > USBD_MAX_NUM_CONFIGURATION)
  {
    USBD_CtlError(pdev, req);
  }
  else
  {
    switch (pdev->dev_state)
    {
    case USBD_STATE_ADDRESSED:
      if (cfgidx)
      {
        pdev->dev_config = cfgidx;
        pdev->dev_state = USBD_STATE_CONFIGURED;
        if(USBD_SetClassConfig(pdev, cfgidx) == USBD_FAIL)
        {
          USBD_CtlError(pdev, req);
          return;
        }
        USBD_CtlSendStatus(pdev);
      }
      else
      {
        USBD_CtlSendStatus(pdev);
      }
      break;

    case USBD_STATE_CONFIGURED:
      if (cfgidx == 0U)
      {
        pdev->dev_state = USBD_STATE_ADDRESSED;
        pdev->dev_config = cfgidx;
        USBD_ClrClassConfig(pdev, cfgidx);
        USBD_CtlSendStatus(pdev);
      }
      else if (cfgidx != pdev->dev_config)
      {
        /* Clear old configuration */
        USBD_ClrClassConfig(pdev, (uint8_t)pdev->dev_config);

        /* set new configuration */
        pdev->dev_config = cfgidx;
        if(USBD_SetClassConfig(pdev, cfgidx) == USBD_FAIL)
        {
          USBD_CtlError(pdev, req);
          return;
        }
        USBD_CtlSendStatus(pdev);
      }
      else
      {
        USBD_CtlSendStatus(pdev);
      }
      break;

    default:
      USBD_CtlError(pdev, req);
      USBD_ClrClassConfig(pdev, cfgidx);
      break;
    }
  }
}

/**
* @brief  USBD_GetConfig
*         Handle Get device configuration request
* @param  pdev: device instance
* @param  req: usb request
* @retval status
*/
static void USBD_GetConfig(USBD_HandleTypeDef *pdev, USBD_SetupReqTypedef *req)
{
  if (req->wLength != 1U)
  {
    USBD_CtlError(pdev , req);
  }
  else
  {
    switch (pdev->dev_state)
    {
    case USBD_STATE_DEFAULT:
    case USBD_STATE_ADDRESSED:
      pdev->dev_default_config = 0U;
      USBD_CtlSendData (pdev, (uint8_t *)(void *)&pdev->dev_default_config, 1U);
      break;

    case USBD_STATE_CONFIGURED:
      USBD_CtlSendData (pdev, (uint8_t *)(void *)&pdev->dev_config, 1U);
      break;

    default:
      USBD_CtlError(pdev , req);
      break;
    }
  }
}

/**
* @brief  USBD_GetStatus
*         Handle Get Status request
* @param  pdev: device instance
* @param  req: usb request
* @retval status
*/
static void USBD_GetStatus(USBD_HandleTypeDef *pdev, USBD_SetupReqTypedef *req)
{
  switch (pdev->dev_state)
  {
  case USBD_STATE_DEFAULT:
  case USBD_STATE_ADDRESSED:
  case USBD_STATE_CONFIGURED:
    if(req->wLength != 0x2U)
    {
      USBD_CtlError(pdev, req);
      break;
    }

#if ( USBD_SELF_POWERED == 1U)
    pdev->dev_config_status = USB_CONFIG_SELF_POWERED;
#else
    pdev->dev_config_status = 0U;
#endif

    if (pdev->dev_remote_wakeup)
    {
      pdev->dev_config_status |= USB_CONFIG_REMOTE_WAKEUP;
    }

    USBD_CtlSendData (pdev, (uint8_t *)(void *)&pdev->dev_config_status, 2U);
    break;

  default :
    USBD_CtlError(pdev , req);
    break;
  }
}


/**
* @brief  USBD_SetFeature
*         Handle Set device feature request
* @param  pdev: device instance
* @param  req: usb request
* @retval status
*/
static void USBD_SetFeature(USBD_HandleTypeDef *pdev ,
                            USBD_SetupReqTypedef *req)
{

  if (req->wValue == USB_FEATURE_REMOTE_WAKEUP)
  {
    pdev->dev_remote_wakeup = 1U;
    USBD_CtlSendStatus(pdev);
  }

}


/**
* @brief  USBD_ClrFeature
*         Handle clear device feature request
* @param  pdev: device instance
* @param  req: usb request
* @retval status
*/
static void USBD_ClrFeature(USBD_HandleTypeDef *pdev ,
                            USBD_SetupReqTypedef *req)
{
  switch (pdev->dev_state)
  {
  case USBD_STATE_DEFAULT:
  case USBD_STATE_ADDRESSED:
  case USBD_STATE_CONFIGURED:
    if (req->wValue == USB_FEATURE_REMOTE_WAKEUP)
    {
      pdev->dev_remote_wakeup = 0U;
      USBD_CtlSendStatus(pdev);
    }
    break;

  default :
     USBD_CtlError(pdev , req);
    break;
  }
}

/**
* @brief  USBD_ParseSetupRequest
*         Copy buffer into setup structure
* @param  pdev: device instance
* @param  req: usb request
* @retval None
*/

void USBD_ParseSetupRequest(USBD_SetupReqTypedef *req, uint8_t *pdata)
{
  req->bmRequest     = *(uint8_t *)  (pdata);
  req->bRequest      = *(uint8_t *)  (pdata +  1);
  req->wValue        = SWAPBYTE      (pdata +  2);
  req->wIndex        = SWAPBYTE      (pdata +  4);
  req->wLength       = SWAPBYTE      (pdata +  6);

}

/**
* @brief  USBD_CtlError
*         Handle USB low level Error
* @param  pdev: device instance
* @param  req: usb request
* @retval None
*/

void USBD_CtlError( USBD_HandleTypeDef *pdev ,
                            USBD_SetupReqTypedef *req)
{
  USBD_LL_StallEP(pdev , 0x80U);
  USBD_LL_StallEP(pdev , 0U);
}


/**
  * @brief  USBD_GetString
  *         Convert Ascii string into unicode one
  * @param  desc : descriptor buffer
  * @param  unicode : Formatted string buffer (unicode)
  * @param  len : descriptor length
  * @retval None
  */
void USBD_GetString(uint8_t *desc, uint8_t *unicode, uint16_t *len)
{
  uint8_t idx = 0U;

  if (desc != NULL)
  {
    *len = (uint16_t)USBD_GetLen(desc) * 2U + 2U;
    unicode[idx++] = *(uint8_t *)(void *)len;
    unicode[idx++] = USB_DESC_TYPE_STRING;

    while (*desc != '\0')
    {
      unicode[idx++] = *desc++;
      unicode[idx++] =  0U;
    }
  }
}

/**
  * @brief  USBD_GetLen
  *         return the string length
   * @param  buf : pointer to the ascii string buffer
  * @retval string length
  */
static uint8_t USBD_GetLen(uint8_t *buf)
{
    uint8_t  len = 0U;

    while (*buf != '\0')
    {
        len++;
        buf++;
    }

    return len;
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
