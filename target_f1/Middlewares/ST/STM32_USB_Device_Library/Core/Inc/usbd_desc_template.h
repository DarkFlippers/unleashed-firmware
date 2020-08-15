/**
  ******************************************************************************
  * @file    usbd_desc_template.h
  * @author  MCD Application Team
  * @brief   Header for usbd_desc_template.c module
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
#ifndef __USBD_DESC_TEMPLATE_H
#define __USBD_DESC_TEMPLATE_H

/* Includes ------------------------------------------------------------------*/
#include "usbd_def.h"

/* Exported types ------------------------------------------------------------*/
/* Exported constants --------------------------------------------------------*/
#define         DEVICE_ID1          (0x1FFF7A10)
#define         DEVICE_ID2          (0x1FFF7A14)
#define         DEVICE_ID3          (0x1FFF7A18)

#define  USB_SIZ_STRING_SERIAL       0x1A

/* Exported macro ------------------------------------------------------------*/
/* Exported functions ------------------------------------------------------- */
extern USBD_DescriptorsTypeDef XXX_Desc; /* Replace 'XXX_Desc' with your active USB device class, ex: HID_Desc */

#endif /* __USBD_DESC_TEMPLATE_H*/

/************************ (C) COPYRIGHT STMicroelectronics *****END OF FILE****/
