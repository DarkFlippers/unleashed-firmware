/**
  ******************************************************************************
  * @file    stm32l4xx_ll_usb.c
  * @author  MCD Application Team
  * @brief   USB Low Layer HAL module driver.
  *
  *          This file provides firmware functions to manage the following
  *          functionalities of the USB Peripheral Controller:
  *           + Initialization/de-initialization functions
  *           + I/O operation functions
  *           + Peripheral Control functions
  *           + Peripheral State functions
  *
  @verbatim
  ==============================================================================
                    ##### How to use this driver #####
  ==============================================================================
    [..]
      (#) Fill parameters of Init structure in USB_OTG_CfgTypeDef structure.

      (#) Call USB_CoreInit() API to initialize the USB Core peripheral.

      (#) The upper HAL HCD/PCD driver will call the right routines for its internal processes.

  @endverbatim
  ******************************************************************************
  * @attention
  *
  * <h2><center>&copy; Copyright (c) 2017 STMicroelectronics.
  * All rights reserved.</center></h2>
  *
  * This software component is licensed by ST under BSD 3-Clause license,
  * the "License"; You may not use this file except in compliance with the
  * License. You may obtain a copy of the License at:
  *                        opensource.org/licenses/BSD-3-Clause
  *
  ******************************************************************************
  */

/* Includes ------------------------------------------------------------------*/
#include "stm32l4xx_hal.h"

/** @addtogroup STM32L4xx_LL_USB_DRIVER
  * @{
  */

#if defined (HAL_PCD_MODULE_ENABLED) || defined (HAL_HCD_MODULE_ENABLED)
#if defined (USB) || defined (USB_OTG_FS)
/* Private typedef -----------------------------------------------------------*/
/* Private define ------------------------------------------------------------*/
/* Private macro -------------------------------------------------------------*/
/* Private variables ---------------------------------------------------------*/
/* Private function prototypes -----------------------------------------------*/
/* Private functions ---------------------------------------------------------*/
#if defined (USB_OTG_FS)
static HAL_StatusTypeDef USB_CoreReset(USB_OTG_GlobalTypeDef *USBx);

/* Exported functions --------------------------------------------------------*/
/** @defgroup USB_LL_Exported_Functions USB Low Layer Exported Functions
  * @{
  */

/** @defgroup USB_LL_Exported_Functions_Group1 Initialization/de-initialization functions
 *  @brief    Initialization and Configuration functions
 *
@verbatim
 ===============================================================================
                      ##### Initialization/de-initialization functions #####
 ===============================================================================

@endverbatim
  * @{
  */

/**
  * @brief  Initializes the USB Core
  * @param  USBx USB Instance
  * @param  cfg pointer to a USB_OTG_CfgTypeDef structure that contains
  *         the configuration information for the specified USBx peripheral.
  * @retval HAL status
  */
HAL_StatusTypeDef USB_CoreInit(USB_OTG_GlobalTypeDef *USBx, USB_OTG_CfgTypeDef cfg)
{
  HAL_StatusTypeDef ret;

  if (cfg.phy_itface == USB_OTG_ULPI_PHY)
  {
    USBx->GCCFG &= ~(USB_OTG_GCCFG_PWRDWN);

    /* Init The ULPI Interface */
    USBx->GUSBCFG &= ~(USB_OTG_GUSBCFG_TSDPS | USB_OTG_GUSBCFG_ULPIFSLS | USB_OTG_GUSBCFG_PHYSEL);

    /* Select vbus source */
    USBx->GUSBCFG &= ~(USB_OTG_GUSBCFG_ULPIEVBUSD | USB_OTG_GUSBCFG_ULPIEVBUSI);
    if (cfg.use_external_vbus == 1U)
    {
      USBx->GUSBCFG |= USB_OTG_GUSBCFG_ULPIEVBUSD;
    }
    /* Reset after a PHY select  */
    ret = USB_CoreReset(USBx);
  }
  else /* FS interface (embedded Phy) */
  {
    /* Select FS Embedded PHY */
    USBx->GUSBCFG |= USB_OTG_GUSBCFG_PHYSEL;

    /* Reset after a PHY select and set Host mode */
    ret = USB_CoreReset(USBx);

    if (cfg.battery_charging_enable == 0U)
    {
      /* Activate the USB Transceiver */
      USBx->GCCFG |= USB_OTG_GCCFG_PWRDWN;
    }
    else
    {
      /* Deactivate the USB Transceiver */
      USBx->GCCFG &= ~(USB_OTG_GCCFG_PWRDWN);
    }
  }

  return ret;
}


/**
  * @brief  Set the USB turnaround time
  * @param  USBx USB Instance
  * @param  hclk: AHB clock frequency
  * @retval USB turnaround time In PHY Clocks number
  */
HAL_StatusTypeDef USB_SetTurnaroundTime(USB_OTG_GlobalTypeDef *USBx,
                                        uint32_t hclk, uint8_t speed)
{
  uint32_t UsbTrd;

  /* The USBTRD is configured according to the tables below, depending on AHB frequency
  used by application. In the low AHB frequency range it is used to stretch enough the USB response
  time to IN tokens, the USB turnaround time, so to compensate for the longer AHB read access
  latency to the Data FIFO */
  if (speed == USBD_FS_SPEED)
  {
    if ((hclk >= 14200000U) && (hclk < 15000000U))
    {
      /* hclk Clock Range between 14.2-15 MHz */
      UsbTrd = 0xFU;
    }
    else if ((hclk >= 15000000U) && (hclk < 16000000U))
    {
      /* hclk Clock Range between 15-16 MHz */
      UsbTrd = 0xEU;
    }
    else if ((hclk >= 16000000U) && (hclk < 17200000U))
    {
      /* hclk Clock Range between 16-17.2 MHz */
      UsbTrd = 0xDU;
    }
    else if ((hclk >= 17200000U) && (hclk < 18500000U))
    {
      /* hclk Clock Range between 17.2-18.5 MHz */
      UsbTrd = 0xCU;
    }
    else if ((hclk >= 18500000U) && (hclk < 20000000U))
    {
      /* hclk Clock Range between 18.5-20 MHz */
      UsbTrd = 0xBU;
    }
    else if ((hclk >= 20000000U) && (hclk < 21800000U))
    {
      /* hclk Clock Range between 20-21.8 MHz */
      UsbTrd = 0xAU;
    }
    else if ((hclk >= 21800000U) && (hclk < 24000000U))
    {
      /* hclk Clock Range between 21.8-24 MHz */
      UsbTrd = 0x9U;
    }
    else if ((hclk >= 24000000U) && (hclk < 27700000U))
    {
      /* hclk Clock Range between 24-27.7 MHz */
      UsbTrd = 0x8U;
    }
    else if ((hclk >= 27700000U) && (hclk < 32000000U))
    {
      /* hclk Clock Range between 27.7-32 MHz */
      UsbTrd = 0x7U;
    }
    else /* if(hclk >= 32000000) */
    {
      /* hclk Clock Range between 32-200 MHz */
      UsbTrd = 0x6U;
    }
  }
  else
  {
    UsbTrd = USBD_DEFAULT_TRDT_VALUE;
  }

  USBx->GUSBCFG &= ~USB_OTG_GUSBCFG_TRDT;
  USBx->GUSBCFG |= (uint32_t)((UsbTrd << 10) & USB_OTG_GUSBCFG_TRDT);

  return HAL_OK;
}

/**
  * @brief  USB_EnableGlobalInt
  *         Enables the controller's Global Int in the AHB Config reg
  * @param  USBx  Selected device
  * @retval HAL status
  */
HAL_StatusTypeDef USB_EnableGlobalInt(USB_OTG_GlobalTypeDef *USBx)
{
  USBx->GAHBCFG |= USB_OTG_GAHBCFG_GINT;
  return HAL_OK;
}

/**
  * @brief  USB_DisableGlobalInt
  *         Disable the controller's Global Int in the AHB Config reg
  * @param  USBx  Selected device
  * @retval HAL status
*/
HAL_StatusTypeDef USB_DisableGlobalInt(USB_OTG_GlobalTypeDef *USBx)
{
  USBx->GAHBCFG &= ~USB_OTG_GAHBCFG_GINT;
  return HAL_OK;
}

/**
  * @brief  USB_SetCurrentMode : Set functional mode
  * @param  USBx  Selected device
  * @param  mode   current core mode
  *          This parameter can be one of these values:
  *            @arg USB_DEVICE_MODE: Peripheral mode
  *            @arg USB_HOST_MODE: Host mode
  *            @arg USB_DRD_MODE: Dual Role Device mode
  * @retval HAL status
  */
HAL_StatusTypeDef USB_SetCurrentMode(USB_OTG_GlobalTypeDef *USBx, USB_ModeTypeDef mode)
{
  USBx->GUSBCFG &= ~(USB_OTG_GUSBCFG_FHMOD | USB_OTG_GUSBCFG_FDMOD);

  if (mode == USB_HOST_MODE)
  {
    USBx->GUSBCFG |= USB_OTG_GUSBCFG_FHMOD;
  }
  else if (mode == USB_DEVICE_MODE)
  {
    USBx->GUSBCFG |= USB_OTG_GUSBCFG_FDMOD;
  }
  else
  {
    return HAL_ERROR;
  }
  HAL_Delay(50U);

  return HAL_OK;
}

/**
  * @brief  USB_DevInit : Initializes the USB_OTG controller registers
  *         for device mode
  * @param  USBx  Selected device
  * @param  cfg   pointer to a USB_OTG_CfgTypeDef structure that contains
  *         the configuration information for the specified USBx peripheral.
  * @retval HAL status
  */
HAL_StatusTypeDef USB_DevInit(USB_OTG_GlobalTypeDef *USBx, USB_OTG_CfgTypeDef cfg)
{
  HAL_StatusTypeDef ret = HAL_OK;
  uint32_t USBx_BASE = (uint32_t)USBx;
  uint32_t i;

  for (i = 0U; i < 15U; i++)
  {
    USBx->DIEPTXF[i] = 0U;
  }

  /* VBUS Sensing setup */
  if (cfg.vbus_sensing_enable == 0U)
  {
    /* Deactivate VBUS Sensing B */
    USBx->GCCFG &= ~USB_OTG_GCCFG_VBDEN;

    /* B-peripheral session valid override enable */
    USBx->GOTGCTL |= USB_OTG_GOTGCTL_BVALOEN;
    USBx->GOTGCTL |= USB_OTG_GOTGCTL_BVALOVAL;
  }
  else
  {
    /* Enable HW VBUS sensing */
    USBx->GCCFG |= USB_OTG_GCCFG_VBDEN;
  }

  /* Restart the Phy Clock */
  USBx_PCGCCTL = 0U;

  /* Device mode configuration */
  USBx_DEVICE->DCFG |= DCFG_FRAME_INTERVAL_80;

  /* Set Core speed to Full speed mode */
  (void)USB_SetDevSpeed(USBx, USB_OTG_SPEED_FULL);

  /* Flush the FIFOs */
  if (USB_FlushTxFifo(USBx, 0x10U) != HAL_OK) /* all Tx FIFOs */
  {
    ret = HAL_ERROR;
  }

  if (USB_FlushRxFifo(USBx) != HAL_OK)
  {
    ret = HAL_ERROR;
  }

  /* Clear all pending Device Interrupts */
  USBx_DEVICE->DIEPMSK = 0U;
  USBx_DEVICE->DOEPMSK = 0U;
  USBx_DEVICE->DAINTMSK = 0U;

  for (i = 0U; i < cfg.dev_endpoints; i++)
  {
    if ((USBx_INEP(i)->DIEPCTL & USB_OTG_DIEPCTL_EPENA) == USB_OTG_DIEPCTL_EPENA)
    {
      if (i == 0U)
      {
        USBx_INEP(i)->DIEPCTL = USB_OTG_DIEPCTL_SNAK;
      }
      else
      {
        USBx_INEP(i)->DIEPCTL = USB_OTG_DIEPCTL_EPDIS | USB_OTG_DIEPCTL_SNAK;
      }
    }
    else
    {
      USBx_INEP(i)->DIEPCTL = 0U;
    }

    USBx_INEP(i)->DIEPTSIZ = 0U;
    USBx_INEP(i)->DIEPINT  = 0xFB7FU;
  }

  for (i = 0U; i < cfg.dev_endpoints; i++)
  {
    if ((USBx_OUTEP(i)->DOEPCTL & USB_OTG_DOEPCTL_EPENA) == USB_OTG_DOEPCTL_EPENA)
    {
      if (i == 0U)
      {
        USBx_OUTEP(i)->DOEPCTL = USB_OTG_DOEPCTL_SNAK;
      }
      else
      {
        USBx_OUTEP(i)->DOEPCTL = USB_OTG_DOEPCTL_EPDIS | USB_OTG_DOEPCTL_SNAK;
      }
    }
    else
    {
      USBx_OUTEP(i)->DOEPCTL = 0U;
    }

    USBx_OUTEP(i)->DOEPTSIZ = 0U;
    USBx_OUTEP(i)->DOEPINT  = 0xFB7FU;
  }

  USBx_DEVICE->DIEPMSK &= ~(USB_OTG_DIEPMSK_TXFURM);

  /* Disable all interrupts. */
  USBx->GINTMSK = 0U;

  /* Clear any pending interrupts */
  USBx->GINTSTS = 0xBFFFFFFFU;

  /* Enable the common interrupts */
  USBx->GINTMSK |= USB_OTG_GINTMSK_RXFLVLM;

  /* Enable interrupts matching to the Device mode ONLY */
  USBx->GINTMSK |= USB_OTG_GINTMSK_USBSUSPM | USB_OTG_GINTMSK_USBRST |
                   USB_OTG_GINTMSK_ENUMDNEM | USB_OTG_GINTMSK_IEPINT |
                   USB_OTG_GINTMSK_OEPINT   | USB_OTG_GINTMSK_IISOIXFRM |
                   USB_OTG_GINTMSK_PXFRM_IISOOXFRM | USB_OTG_GINTMSK_WUIM;

  if (cfg.Sof_enable != 0U)
  {
    USBx->GINTMSK |= USB_OTG_GINTMSK_SOFM;
  }

  if (cfg.vbus_sensing_enable == 1U)
  {
    USBx->GINTMSK |= (USB_OTG_GINTMSK_SRQIM | USB_OTG_GINTMSK_OTGINT);
  }

  return ret;
}

/**
  * @brief  USB_OTG_FlushTxFifo : Flush a Tx FIFO
  * @param  USBx  Selected device
  * @param  num  FIFO number
  *         This parameter can be a value from 1 to 15
            15 means Flush all Tx FIFOs
  * @retval HAL status
  */
HAL_StatusTypeDef USB_FlushTxFifo(USB_OTG_GlobalTypeDef *USBx, uint32_t num)
{
  uint32_t count = 0U;

  USBx->GRSTCTL = (USB_OTG_GRSTCTL_TXFFLSH | (num << 6));

  do
  {
    if (++count > 200000U)
    {
      return HAL_TIMEOUT;
    }
  }
  while ((USBx->GRSTCTL & USB_OTG_GRSTCTL_TXFFLSH) == USB_OTG_GRSTCTL_TXFFLSH);

  return HAL_OK;
}

/**
  * @brief  USB_FlushRxFifo : Flush Rx FIFO
  * @param  USBx  Selected device
  * @retval HAL status
  */
HAL_StatusTypeDef USB_FlushRxFifo(USB_OTG_GlobalTypeDef *USBx)
{
  uint32_t count = 0;

  USBx->GRSTCTL = USB_OTG_GRSTCTL_RXFFLSH;

  do
  {
    if (++count > 200000U)
    {
      return HAL_TIMEOUT;
    }
  }
  while ((USBx->GRSTCTL & USB_OTG_GRSTCTL_RXFFLSH) == USB_OTG_GRSTCTL_RXFFLSH);

  return HAL_OK;
}

/**
  * @brief  USB_SetDevSpeed  Initializes the DevSpd field of DCFG register
  *         depending the PHY type and the enumeration speed of the device.
  * @param  USBx  Selected device
  * @param  speed  device speed
  *          This parameter can be one of these values:
  *            @arg USB_OTG_SPEED_FULL: Full speed mode
  * @retval  Hal status
  */
HAL_StatusTypeDef USB_SetDevSpeed(USB_OTG_GlobalTypeDef *USBx, uint8_t speed)
{
  uint32_t USBx_BASE = (uint32_t)USBx;

  USBx_DEVICE->DCFG |= speed;
  return HAL_OK;
}

/**
  * @brief  USB_GetDevSpeed  Return the Dev Speed
  * @param  USBx  Selected device
  * @retval speed  device speed
  *          This parameter can be one of these values:
  *            @arg PCD_SPEED_FULL: Full speed mode
  */
uint8_t USB_GetDevSpeed(USB_OTG_GlobalTypeDef *USBx)
{
  uint32_t USBx_BASE = (uint32_t)USBx;
  uint8_t speed;
  uint32_t DevEnumSpeed = USBx_DEVICE->DSTS & USB_OTG_DSTS_ENUMSPD;

  if ((DevEnumSpeed == DSTS_ENUMSPD_FS_PHY_30MHZ_OR_60MHZ) ||
      (DevEnumSpeed == DSTS_ENUMSPD_FS_PHY_48MHZ))
  {
    speed = USBD_FS_SPEED;
  }
  else
  {
    speed = 0xFU;
  }

  return speed;
}

/**
  * @brief  Activate and configure an endpoint
  * @param  USBx  Selected device
  * @param  ep pointer to endpoint structure
  * @retval HAL status
  */
HAL_StatusTypeDef USB_ActivateEndpoint(USB_OTG_GlobalTypeDef *USBx, USB_OTG_EPTypeDef *ep)
{
  uint32_t USBx_BASE = (uint32_t)USBx;
  uint32_t epnum = (uint32_t)ep->num;

  if (ep->is_in == 1U)
  {
    USBx_DEVICE->DAINTMSK |= USB_OTG_DAINTMSK_IEPM & (uint32_t)(1UL << (ep->num & EP_ADDR_MSK));

    if ((USBx_INEP(epnum)->DIEPCTL & USB_OTG_DIEPCTL_USBAEP) == 0U)
    {
      USBx_INEP(epnum)->DIEPCTL |= (ep->maxpacket & USB_OTG_DIEPCTL_MPSIZ) |
                                   ((uint32_t)ep->type << 18) | (epnum << 22) |
                                   USB_OTG_DIEPCTL_SD0PID_SEVNFRM |
                                   USB_OTG_DIEPCTL_USBAEP;
    }
  }
  else
  {
    USBx_DEVICE->DAINTMSK |= USB_OTG_DAINTMSK_OEPM & ((uint32_t)(1UL << (ep->num & EP_ADDR_MSK)) << 16);

    if (((USBx_OUTEP(epnum)->DOEPCTL) & USB_OTG_DOEPCTL_USBAEP) == 0U)
    {
      USBx_OUTEP(epnum)->DOEPCTL |= (ep->maxpacket & USB_OTG_DOEPCTL_MPSIZ) |
                                    ((uint32_t)ep->type << 18) |
                                    USB_OTG_DIEPCTL_SD0PID_SEVNFRM |
                                    USB_OTG_DOEPCTL_USBAEP;
    }
  }
  return HAL_OK;
}

/**
  * @brief  Activate and configure a dedicated endpoint
  * @param  USBx  Selected device
  * @param  ep pointer to endpoint structure
  * @retval HAL status
  */
HAL_StatusTypeDef USB_ActivateDedicatedEndpoint(USB_OTG_GlobalTypeDef *USBx, USB_OTG_EPTypeDef *ep)
{
  uint32_t USBx_BASE = (uint32_t)USBx;
  uint32_t epnum = (uint32_t)ep->num;

  /* Read DEPCTLn register */
  if (ep->is_in == 1U)
  {
    if (((USBx_INEP(epnum)->DIEPCTL) & USB_OTG_DIEPCTL_USBAEP) == 0U)
    {
      USBx_INEP(epnum)->DIEPCTL |= (ep->maxpacket & USB_OTG_DIEPCTL_MPSIZ) |
                                   ((uint32_t)ep->type << 18) | (epnum << 22) |
                                   USB_OTG_DIEPCTL_SD0PID_SEVNFRM |
                                   USB_OTG_DIEPCTL_USBAEP;
    }

    USBx_DEVICE->DEACHMSK |= USB_OTG_DAINTMSK_IEPM & (uint32_t)(1UL << (ep->num & EP_ADDR_MSK));
  }
  else
  {
    if (((USBx_OUTEP(epnum)->DOEPCTL) & USB_OTG_DOEPCTL_USBAEP) == 0U)
    {
      USBx_OUTEP(epnum)->DOEPCTL |= (ep->maxpacket & USB_OTG_DOEPCTL_MPSIZ) |
                                    ((uint32_t)ep->type << 18) | (epnum << 22) |
                                    USB_OTG_DOEPCTL_USBAEP;
    }

    USBx_DEVICE->DEACHMSK |= USB_OTG_DAINTMSK_OEPM & ((uint32_t)(1UL << (ep->num & EP_ADDR_MSK)) << 16);
  }

  return HAL_OK;
}

/**
  * @brief  De-activate and de-initialize an endpoint
  * @param  USBx  Selected device
  * @param  ep pointer to endpoint structure
  * @retval HAL status
  */
HAL_StatusTypeDef USB_DeactivateEndpoint(USB_OTG_GlobalTypeDef *USBx, USB_OTG_EPTypeDef *ep)
{
  uint32_t USBx_BASE = (uint32_t)USBx;
  uint32_t epnum = (uint32_t)ep->num;

  /* Read DEPCTLn register */
  if (ep->is_in == 1U)
  {
    USBx_DEVICE->DEACHMSK &= ~(USB_OTG_DAINTMSK_IEPM & (uint32_t)(1UL << (ep->num & EP_ADDR_MSK)));
    USBx_DEVICE->DAINTMSK &= ~(USB_OTG_DAINTMSK_IEPM & (uint32_t)(1UL << (ep->num & EP_ADDR_MSK)));
    USBx_INEP(epnum)->DIEPCTL &= ~(USB_OTG_DIEPCTL_USBAEP |
                                   USB_OTG_DIEPCTL_MPSIZ |
                                   USB_OTG_DIEPCTL_TXFNUM |
                                   USB_OTG_DIEPCTL_SD0PID_SEVNFRM |
                                   USB_OTG_DIEPCTL_EPTYP);
  }
  else
  {
    USBx_DEVICE->DEACHMSK &= ~(USB_OTG_DAINTMSK_OEPM & ((uint32_t)(1UL << (ep->num & EP_ADDR_MSK)) << 16));
    USBx_DEVICE->DAINTMSK &= ~(USB_OTG_DAINTMSK_OEPM & ((uint32_t)(1UL << (ep->num & EP_ADDR_MSK)) << 16));
    USBx_OUTEP(epnum)->DOEPCTL &= ~(USB_OTG_DOEPCTL_USBAEP |
                                    USB_OTG_DOEPCTL_MPSIZ |
                                    USB_OTG_DOEPCTL_SD0PID_SEVNFRM |
                                    USB_OTG_DOEPCTL_EPTYP);
  }

  return HAL_OK;
}

/**
  * @brief  De-activate and de-initialize a dedicated endpoint
  * @param  USBx  Selected device
  * @param  ep pointer to endpoint structure
  * @retval HAL status
  */
HAL_StatusTypeDef USB_DeactivateDedicatedEndpoint(USB_OTG_GlobalTypeDef *USBx, USB_OTG_EPTypeDef *ep)
{
  uint32_t USBx_BASE = (uint32_t)USBx;
  uint32_t epnum = (uint32_t)ep->num;

  /* Read DEPCTLn register */
  if (ep->is_in == 1U)
  {
    USBx_INEP(epnum)->DIEPCTL &= ~ USB_OTG_DIEPCTL_USBAEP;
    USBx_DEVICE->DAINTMSK &= ~(USB_OTG_DAINTMSK_IEPM & (uint32_t)(1UL << (ep->num & EP_ADDR_MSK)));
  }
  else
  {
    USBx_OUTEP(epnum)->DOEPCTL &= ~USB_OTG_DOEPCTL_USBAEP;
    USBx_DEVICE->DAINTMSK &= ~(USB_OTG_DAINTMSK_OEPM & ((uint32_t)(1UL << (ep->num & EP_ADDR_MSK)) << 16));
  }

  return HAL_OK;
}

/**
  * @brief  USB_EPStartXfer : setup and starts a transfer over an EP
  * @param  USBx  Selected device
  * @param  ep pointer to endpoint structure
  * @retval HAL status
  */
HAL_StatusTypeDef USB_EPStartXfer(USB_OTG_GlobalTypeDef *USBx, USB_OTG_EPTypeDef *ep)
{
  uint32_t USBx_BASE = (uint32_t)USBx;
  uint32_t epnum = (uint32_t)ep->num;
  uint16_t pktcnt;

  /* IN endpoint */
  if (ep->is_in == 1U)
  {
    /* Zero Length Packet? */
    if (ep->xfer_len == 0U)
    {
      USBx_INEP(epnum)->DIEPTSIZ &= ~(USB_OTG_DIEPTSIZ_PKTCNT);
      USBx_INEP(epnum)->DIEPTSIZ |= (USB_OTG_DIEPTSIZ_PKTCNT & (1U << 19));
      USBx_INEP(epnum)->DIEPTSIZ &= ~(USB_OTG_DIEPTSIZ_XFRSIZ);
    }
    else
    {
      /* Program the transfer size and packet count
      * as follows: xfersize = N * maxpacket +
      * short_packet pktcnt = N + (short_packet
      * exist ? 1 : 0)
      */
      USBx_INEP(epnum)->DIEPTSIZ &= ~(USB_OTG_DIEPTSIZ_XFRSIZ);
      USBx_INEP(epnum)->DIEPTSIZ &= ~(USB_OTG_DIEPTSIZ_PKTCNT);
      USBx_INEP(epnum)->DIEPTSIZ |= (USB_OTG_DIEPTSIZ_PKTCNT & (((ep->xfer_len + ep->maxpacket - 1U) / ep->maxpacket) << 19));
      USBx_INEP(epnum)->DIEPTSIZ |= (USB_OTG_DIEPTSIZ_XFRSIZ & ep->xfer_len);

      if (ep->type == EP_TYPE_ISOC)
      {
        USBx_INEP(epnum)->DIEPTSIZ &= ~(USB_OTG_DIEPTSIZ_MULCNT);
        USBx_INEP(epnum)->DIEPTSIZ |= (USB_OTG_DIEPTSIZ_MULCNT & (1U << 29));
      }
    }
    /* EP enable, IN data in FIFO */
    USBx_INEP(epnum)->DIEPCTL |= (USB_OTG_DIEPCTL_CNAK | USB_OTG_DIEPCTL_EPENA);

    if (ep->type != EP_TYPE_ISOC)
    {
      /* Enable the Tx FIFO Empty Interrupt for this EP */
      if (ep->xfer_len > 0U)
      {
        USBx_DEVICE->DIEPEMPMSK |= 1UL << (ep->num & EP_ADDR_MSK);
      }
    }
    else
    {
      if ((USBx_DEVICE->DSTS & (1U << 8)) == 0U)
      {
        USBx_INEP(epnum)->DIEPCTL |= USB_OTG_DIEPCTL_SODDFRM;
      }
      else
      {
        USBx_INEP(epnum)->DIEPCTL |= USB_OTG_DIEPCTL_SD0PID_SEVNFRM;
      }

      (void)USB_WritePacket(USBx, ep->xfer_buff, ep->num, (uint16_t)ep->xfer_len);
    }
  }
  else /* OUT endpoint */
  {
    /* Program the transfer size and packet count as follows:
    * pktcnt = N
    * xfersize = N * maxpacket
    */
    USBx_OUTEP(epnum)->DOEPTSIZ &= ~(USB_OTG_DOEPTSIZ_XFRSIZ);
    USBx_OUTEP(epnum)->DOEPTSIZ &= ~(USB_OTG_DOEPTSIZ_PKTCNT);

    if (ep->xfer_len == 0U)
    {
      USBx_OUTEP(epnum)->DOEPTSIZ |= (USB_OTG_DOEPTSIZ_XFRSIZ & ep->maxpacket);
      USBx_OUTEP(epnum)->DOEPTSIZ |= (USB_OTG_DOEPTSIZ_PKTCNT & (1U << 19));
    }
    else
    {
      pktcnt = (uint16_t)((ep->xfer_len + ep->maxpacket - 1U) / ep->maxpacket);
      USBx_OUTEP(epnum)->DOEPTSIZ |= USB_OTG_DOEPTSIZ_PKTCNT & ((uint32_t)pktcnt << 19);
      USBx_OUTEP(epnum)->DOEPTSIZ |= USB_OTG_DOEPTSIZ_XFRSIZ & (ep->maxpacket * pktcnt);
    }

    if (ep->type == EP_TYPE_ISOC)
    {
      if ((USBx_DEVICE->DSTS & (1U << 8)) == 0U)
      {
        USBx_OUTEP(epnum)->DOEPCTL |= USB_OTG_DOEPCTL_SODDFRM;
      }
      else
      {
        USBx_OUTEP(epnum)->DOEPCTL |= USB_OTG_DOEPCTL_SD0PID_SEVNFRM;
      }
    }
    /* EP enable */
    USBx_OUTEP(epnum)->DOEPCTL |= (USB_OTG_DOEPCTL_CNAK | USB_OTG_DOEPCTL_EPENA);
  }

  return HAL_OK;
}

/**
  * @brief  USB_EP0StartXfer : setup and starts a transfer over the EP  0
  * @param  USBx  Selected device
  * @param  ep pointer to endpoint structure
  * @retval HAL status
  */
HAL_StatusTypeDef USB_EP0StartXfer(USB_OTG_GlobalTypeDef *USBx, USB_OTG_EPTypeDef *ep)
{
  uint32_t USBx_BASE = (uint32_t)USBx;
  uint32_t epnum = (uint32_t)ep->num;

  /* IN endpoint */
  if (ep->is_in == 1U)
  {
    /* Zero Length Packet? */
    if (ep->xfer_len == 0U)
    {
      USBx_INEP(epnum)->DIEPTSIZ &= ~(USB_OTG_DIEPTSIZ_PKTCNT);
      USBx_INEP(epnum)->DIEPTSIZ |= (USB_OTG_DIEPTSIZ_PKTCNT & (1U << 19));
      USBx_INEP(epnum)->DIEPTSIZ &= ~(USB_OTG_DIEPTSIZ_XFRSIZ);
    }
    else
    {
      /* Program the transfer size and packet count
      * as follows: xfersize = N * maxpacket +
      * short_packet pktcnt = N + (short_packet
      * exist ? 1 : 0)
      */
      USBx_INEP(epnum)->DIEPTSIZ &= ~(USB_OTG_DIEPTSIZ_XFRSIZ);
      USBx_INEP(epnum)->DIEPTSIZ &= ~(USB_OTG_DIEPTSIZ_PKTCNT);

      if (ep->xfer_len > ep->maxpacket)
      {
        ep->xfer_len = ep->maxpacket;
      }
      USBx_INEP(epnum)->DIEPTSIZ |= (USB_OTG_DIEPTSIZ_PKTCNT & (1U << 19));
      USBx_INEP(epnum)->DIEPTSIZ |= (USB_OTG_DIEPTSIZ_XFRSIZ & ep->xfer_len);
    }

    /* EP enable, IN data in FIFO */
    USBx_INEP(epnum)->DIEPCTL |= (USB_OTG_DIEPCTL_CNAK | USB_OTG_DIEPCTL_EPENA);

    /* Enable the Tx FIFO Empty Interrupt for this EP */
    if (ep->xfer_len > 0U)
    {
      USBx_DEVICE->DIEPEMPMSK |= 1UL << (ep->num & EP_ADDR_MSK);
    }
  }
  else /* OUT endpoint */
  {
    /* Program the transfer size and packet count as follows:
    * pktcnt = N
    * xfersize = N * maxpacket
    */
    USBx_OUTEP(epnum)->DOEPTSIZ &= ~(USB_OTG_DOEPTSIZ_XFRSIZ);
    USBx_OUTEP(epnum)->DOEPTSIZ &= ~(USB_OTG_DOEPTSIZ_PKTCNT);

    if (ep->xfer_len > 0U)
    {
      ep->xfer_len = ep->maxpacket;
    }

    USBx_OUTEP(epnum)->DOEPTSIZ |= (USB_OTG_DOEPTSIZ_PKTCNT & (1U << 19));
    USBx_OUTEP(epnum)->DOEPTSIZ |= (USB_OTG_DOEPTSIZ_XFRSIZ & (ep->maxpacket));

    /* EP enable */
    USBx_OUTEP(epnum)->DOEPCTL |= (USB_OTG_DOEPCTL_CNAK | USB_OTG_DOEPCTL_EPENA);
  }

  return HAL_OK;
}

/**
  * @brief  USB_WritePacket : Writes a packet into the Tx FIFO associated
  *         with the EP/channel
  * @param  USBx  Selected device
  * @param  src   pointer to source buffer
  * @param  ch_ep_num  endpoint or host channel number
  * @param  len  Number of bytes to write
  * @retval HAL status
  */
HAL_StatusTypeDef USB_WritePacket(USB_OTG_GlobalTypeDef *USBx, uint8_t *src, uint8_t ch_ep_num, uint16_t len)
{
  uint32_t USBx_BASE = (uint32_t)USBx;
  uint32_t *pSrc = (uint32_t *)src;
  uint32_t count32b, i;

  count32b = ((uint32_t)len + 3U) / 4U;
  for (i = 0U; i < count32b; i++)
  {
    USBx_DFIFO((uint32_t)ch_ep_num) = __UNALIGNED_UINT32_READ(pSrc);
    pSrc++;
  }

  return HAL_OK;
}

/**
  * @brief  USB_ReadPacket : read a packet from the RX FIFO
  * @param  USBx  Selected device
  * @param  dest  source pointer
  * @param  len  Number of bytes to read
  * @retval pointer to destination buffer
  */
void *USB_ReadPacket(USB_OTG_GlobalTypeDef *USBx, uint8_t *dest, uint16_t len)
{
  uint32_t USBx_BASE = (uint32_t)USBx;
  uint32_t *pDest = (uint32_t *)dest;
  uint32_t i;
  uint32_t count32b = ((uint32_t)len + 3U) / 4U;

  for (i = 0U; i < count32b; i++)
  {
    __UNALIGNED_UINT32_WRITE(pDest, USBx_DFIFO(0U));
    pDest++;
  }

  return ((void *)pDest);
}

/**
  * @brief  USB_EPSetStall : set a stall condition over an EP
  * @param  USBx  Selected device
  * @param  ep pointer to endpoint structure
  * @retval HAL status
  */
HAL_StatusTypeDef USB_EPSetStall(USB_OTG_GlobalTypeDef *USBx, USB_OTG_EPTypeDef *ep)
{
  uint32_t USBx_BASE = (uint32_t)USBx;
  uint32_t epnum = (uint32_t)ep->num;

  if (ep->is_in == 1U)
  {
    if (((USBx_INEP(epnum)->DIEPCTL & USB_OTG_DIEPCTL_EPENA) == 0U) && (epnum != 0U))
    {
      USBx_INEP(epnum)->DIEPCTL &= ~(USB_OTG_DIEPCTL_EPDIS);
    }
    USBx_INEP(epnum)->DIEPCTL |= USB_OTG_DIEPCTL_STALL;
  }
  else
  {
    if (((USBx_OUTEP(epnum)->DOEPCTL & USB_OTG_DOEPCTL_EPENA) == 0U) && (epnum != 0U))
    {
      USBx_OUTEP(epnum)->DOEPCTL &= ~(USB_OTG_DOEPCTL_EPDIS);
    }
    USBx_OUTEP(epnum)->DOEPCTL |= USB_OTG_DOEPCTL_STALL;
  }

  return HAL_OK;
}

/**
  * @brief  USB_EPClearStall : Clear a stall condition over an EP
  * @param  USBx  Selected device
  * @param  ep pointer to endpoint structure
  * @retval HAL status
  */
HAL_StatusTypeDef USB_EPClearStall(USB_OTG_GlobalTypeDef *USBx, USB_OTG_EPTypeDef *ep)
{
  uint32_t USBx_BASE = (uint32_t)USBx;
  uint32_t epnum = (uint32_t)ep->num;

  if (ep->is_in == 1U)
  {
    USBx_INEP(epnum)->DIEPCTL &= ~USB_OTG_DIEPCTL_STALL;
    if ((ep->type == EP_TYPE_INTR) || (ep->type == EP_TYPE_BULK))
    {
      USBx_INEP(epnum)->DIEPCTL |= USB_OTG_DIEPCTL_SD0PID_SEVNFRM; /* DATA0 */
    }
  }
  else
  {
    USBx_OUTEP(epnum)->DOEPCTL &= ~USB_OTG_DOEPCTL_STALL;
    if ((ep->type == EP_TYPE_INTR) || (ep->type == EP_TYPE_BULK))
    {
      USBx_OUTEP(epnum)->DOEPCTL |= USB_OTG_DOEPCTL_SD0PID_SEVNFRM; /* DATA0 */
    }
  }
  return HAL_OK;
}

/**
  * @brief  USB_StopDevice : Stop the usb device mode
  * @param  USBx  Selected device
  * @retval HAL status
  */
HAL_StatusTypeDef USB_StopDevice(USB_OTG_GlobalTypeDef *USBx)
{
  HAL_StatusTypeDef ret;
  uint32_t USBx_BASE = (uint32_t)USBx;
  uint32_t i;

  /* Clear Pending interrupt */
  for (i = 0U; i < 15U; i++)
  {
    USBx_INEP(i)->DIEPINT = 0xFB7FU;
    USBx_OUTEP(i)->DOEPINT = 0xFB7FU;
  }

  /* Clear interrupt masks */
  USBx_DEVICE->DIEPMSK  = 0U;
  USBx_DEVICE->DOEPMSK  = 0U;
  USBx_DEVICE->DAINTMSK = 0U;

  /* Flush the FIFO */
  ret = USB_FlushRxFifo(USBx);
  if (ret != HAL_OK)
  {
    return ret;
  }

  ret = USB_FlushTxFifo(USBx,  0x10U);
  if (ret != HAL_OK)
  {
    return ret;
  }

  return ret;
}

/**
  * @brief  USB_SetDevAddress : Stop the usb device mode
  * @param  USBx  Selected device
  * @param  address  new device address to be assigned
  *          This parameter can be a value from 0 to 255
  * @retval HAL status
  */
HAL_StatusTypeDef  USB_SetDevAddress(USB_OTG_GlobalTypeDef *USBx, uint8_t address)
{
  uint32_t USBx_BASE = (uint32_t)USBx;

  USBx_DEVICE->DCFG &= ~(USB_OTG_DCFG_DAD);
  USBx_DEVICE->DCFG |= ((uint32_t)address << 4) & USB_OTG_DCFG_DAD;

  return HAL_OK;
}

/**
  * @brief  USB_DevConnect : Connect the USB device by enabling the pull-up/pull-down
  * @param  USBx  Selected device
  * @retval HAL status
  */
HAL_StatusTypeDef  USB_DevConnect(USB_OTG_GlobalTypeDef *USBx)
{
  uint32_t USBx_BASE = (uint32_t)USBx;

  USBx_DEVICE->DCTL &= ~USB_OTG_DCTL_SDIS;
  HAL_Delay(3U);

  return HAL_OK;
}

/**
  * @brief  USB_DevDisconnect : Disconnect the USB device by disabling the pull-up/pull-down
  * @param  USBx  Selected device
  * @retval HAL status
  */
HAL_StatusTypeDef  USB_DevDisconnect(USB_OTG_GlobalTypeDef *USBx)
{
  uint32_t USBx_BASE = (uint32_t)USBx;

  USBx_DEVICE->DCTL |= USB_OTG_DCTL_SDIS;
  HAL_Delay(3U);

  return HAL_OK;
}

/**
  * @brief  USB_ReadInterrupts: return the global USB interrupt status
  * @param  USBx  Selected device
  * @retval HAL status
  */
uint32_t  USB_ReadInterrupts(USB_OTG_GlobalTypeDef *USBx)
{
  uint32_t tmpreg;

  tmpreg = USBx->GINTSTS;
  tmpreg &= USBx->GINTMSK;

  return tmpreg;
}

/**
  * @brief  USB_ReadDevAllOutEpInterrupt: return the USB device OUT endpoints interrupt status
  * @param  USBx  Selected device
  * @retval HAL status
  */
uint32_t USB_ReadDevAllOutEpInterrupt(USB_OTG_GlobalTypeDef *USBx)
{
  uint32_t USBx_BASE = (uint32_t)USBx;
  uint32_t tmpreg;

  tmpreg  = USBx_DEVICE->DAINT;
  tmpreg &= USBx_DEVICE->DAINTMSK;

  return ((tmpreg & 0xffff0000U) >> 16);
}

/**
  * @brief  USB_ReadDevAllInEpInterrupt: return the USB device IN endpoints interrupt status
  * @param  USBx  Selected device
  * @retval HAL status
  */
uint32_t USB_ReadDevAllInEpInterrupt(USB_OTG_GlobalTypeDef *USBx)
{
  uint32_t USBx_BASE = (uint32_t)USBx;
  uint32_t tmpreg;

  tmpreg  = USBx_DEVICE->DAINT;
  tmpreg &= USBx_DEVICE->DAINTMSK;

  return ((tmpreg & 0xFFFFU));
}

/**
  * @brief  Returns Device OUT EP Interrupt register
  * @param  USBx  Selected device
  * @param  epnum  endpoint number
  *          This parameter can be a value from 0 to 15
  * @retval Device OUT EP Interrupt register
  */
uint32_t USB_ReadDevOutEPInterrupt(USB_OTG_GlobalTypeDef *USBx, uint8_t epnum)
{
  uint32_t USBx_BASE = (uint32_t)USBx;
  uint32_t tmpreg;

  tmpreg  = USBx_OUTEP((uint32_t)epnum)->DOEPINT;
  tmpreg &= USBx_DEVICE->DOEPMSK;

  return tmpreg;
}

/**
  * @brief  Returns Device IN EP Interrupt register
  * @param  USBx  Selected device
  * @param  epnum  endpoint number
  *          This parameter can be a value from 0 to 15
  * @retval Device IN EP Interrupt register
  */
uint32_t USB_ReadDevInEPInterrupt(USB_OTG_GlobalTypeDef *USBx, uint8_t epnum)
{
  uint32_t USBx_BASE = (uint32_t)USBx;
  uint32_t tmpreg, msk, emp;

  msk = USBx_DEVICE->DIEPMSK;
  emp = USBx_DEVICE->DIEPEMPMSK;
  msk |= ((emp >> (epnum & EP_ADDR_MSK)) & 0x1U) << 7;
  tmpreg = USBx_INEP((uint32_t)epnum)->DIEPINT & msk;

  return tmpreg;
}

/**
  * @brief  USB_ClearInterrupts: clear a USB interrupt
  * @param  USBx  Selected device
  * @param  interrupt  interrupt flag
  * @retval None
  */
void  USB_ClearInterrupts(USB_OTG_GlobalTypeDef *USBx, uint32_t interrupt)
{
  USBx->GINTSTS |= interrupt;
}

/**
  * @brief  Returns USB core mode
  * @param  USBx  Selected device
  * @retval return core mode : Host or Device
  *          This parameter can be one of these values:
  *           0 : Host
  *           1 : Device
  */
uint32_t USB_GetMode(USB_OTG_GlobalTypeDef *USBx)
{
  return ((USBx->GINTSTS) & 0x1U);
}

/**
  * @brief  Activate EP0 for Setup transactions
  * @param  USBx  Selected device
  * @retval HAL status
  */
HAL_StatusTypeDef  USB_ActivateSetup(USB_OTG_GlobalTypeDef *USBx)
{
  uint32_t USBx_BASE = (uint32_t)USBx;

  /* Set the MPS of the IN EP based on the enumeration speed */
  USBx_INEP(0U)->DIEPCTL &= ~USB_OTG_DIEPCTL_MPSIZ;

  if ((USBx_DEVICE->DSTS & USB_OTG_DSTS_ENUMSPD) == DSTS_ENUMSPD_LS_PHY_6MHZ)
  {
    USBx_INEP(0U)->DIEPCTL |= 3U;
  }
  USBx_DEVICE->DCTL |= USB_OTG_DCTL_CGINAK;

  return HAL_OK;
}

/**
  * @brief  Prepare the EP0 to start the first control setup
  * @param  USBx  Selected device
  * @param  psetup  pointer to setup packet
  * @retval HAL status
  */
HAL_StatusTypeDef USB_EP0_OutStart(USB_OTG_GlobalTypeDef *USBx, uint8_t *psetup)
{
  UNUSED(psetup);
  uint32_t USBx_BASE = (uint32_t)USBx;
  uint32_t gSNPSiD = *(__IO uint32_t *)(&USBx->CID + 0x1U);

  if (gSNPSiD > USB_OTG_CORE_ID_300A)
  {
    if ((USBx_OUTEP(0U)->DOEPCTL & USB_OTG_DOEPCTL_EPENA) == USB_OTG_DOEPCTL_EPENA)
    {
      return HAL_OK;
    }
  }

  USBx_OUTEP(0U)->DOEPTSIZ = 0U;
  USBx_OUTEP(0U)->DOEPTSIZ |= (USB_OTG_DOEPTSIZ_PKTCNT & (1U << 19));
  USBx_OUTEP(0U)->DOEPTSIZ |= (3U * 8U);
  USBx_OUTEP(0U)->DOEPTSIZ |=  USB_OTG_DOEPTSIZ_STUPCNT;

  return HAL_OK;
}

/**
  * @brief  Reset the USB Core (needed after USB clock settings change)
  * @param  USBx  Selected device
  * @retval HAL status
  */
static HAL_StatusTypeDef USB_CoreReset(USB_OTG_GlobalTypeDef *USBx)
{
  uint32_t count = 0U;

  /* Wait for AHB master IDLE state. */
  do
  {
    if (++count > 200000U)
    {
      return HAL_TIMEOUT;
    }
  }
  while ((USBx->GRSTCTL & USB_OTG_GRSTCTL_AHBIDL) == 0U);

  /* Core Soft Reset */
  count = 0U;
  USBx->GRSTCTL |= USB_OTG_GRSTCTL_CSRST;

  do
  {
    if (++count > 200000U)
    {
      return HAL_TIMEOUT;
    }
  }
  while ((USBx->GRSTCTL & USB_OTG_GRSTCTL_CSRST) == USB_OTG_GRSTCTL_CSRST);

  return HAL_OK;
}

/**
  * @brief  USB_HostInit : Initializes the USB OTG controller registers
  *         for Host mode
  * @param  USBx  Selected device
  * @param  cfg   pointer to a USB_OTG_CfgTypeDef structure that contains
  *         the configuration information for the specified USBx peripheral.
  * @retval HAL status
  */
HAL_StatusTypeDef USB_HostInit(USB_OTG_GlobalTypeDef *USBx, USB_OTG_CfgTypeDef cfg)
{
  uint32_t USBx_BASE = (uint32_t)USBx;
  uint32_t i;

  /* Restart the Phy Clock */
  USBx_PCGCCTL = 0U;

  /* Disable VBUS sensing */
  USBx->GCCFG &= ~(USB_OTG_GCCFG_VBDEN);

  /* Disable Battery chargin detector */
  USBx->GCCFG &= ~(USB_OTG_GCCFG_BCDEN);

  /* Set default Max speed support */
  USBx_HOST->HCFG &= ~(USB_OTG_HCFG_FSLSS);

  /* Make sure the FIFOs are flushed. */
  (void)USB_FlushTxFifo(USBx, 0x10U); /* all Tx FIFOs */
  (void)USB_FlushRxFifo(USBx);

  /* Clear all pending HC Interrupts */
  for (i = 0U; i < cfg.Host_channels; i++)
  {
    USBx_HC(i)->HCINT = 0xFFFFFFFFU;
    USBx_HC(i)->HCINTMSK = 0U;
  }

  /* Enable VBUS driving */
  (void)USB_DriveVbus(USBx, 1U);

  HAL_Delay(200U);

  /* Disable all interrupts. */
  USBx->GINTMSK = 0U;

  /* Clear any pending interrupts */
  USBx->GINTSTS = 0xFFFFFFFFU;

  /* set Rx FIFO size */
  USBx->GRXFSIZ  = 0x80U;
  USBx->DIEPTXF0_HNPTXFSIZ = (uint32_t)(((0x60U << 16) & USB_OTG_NPTXFD) | 0x80U);
  USBx->HPTXFSIZ = (uint32_t)(((0x40U << 16)& USB_OTG_HPTXFSIZ_PTXFD) | 0xE0U);
  /* Enable the common interrupts */
  USBx->GINTMSK |= USB_OTG_GINTMSK_RXFLVLM;

  /* Enable interrupts matching to the Host mode ONLY */
  USBx->GINTMSK |= (USB_OTG_GINTMSK_PRTIM            | USB_OTG_GINTMSK_HCIM | \
                    USB_OTG_GINTMSK_SOFM             | USB_OTG_GINTSTS_DISCINT | \
                    USB_OTG_GINTMSK_PXFRM_IISOOXFRM  | USB_OTG_GINTMSK_WUIM);

  return HAL_OK;
}

/**
  * @brief  USB_InitFSLSPClkSel : Initializes the FSLSPClkSel field of the
  *         HCFG register on the PHY type and set the right frame interval
  * @param  USBx  Selected device
  * @param  freq  clock frequency
  *          This parameter can be one of these values:
  *           HCFG_48_MHZ : Full Speed 48 MHz Clock
  *           HCFG_6_MHZ : Low Speed 6 MHz Clock
  * @retval HAL status
  */
HAL_StatusTypeDef USB_InitFSLSPClkSel(USB_OTG_GlobalTypeDef *USBx, uint8_t freq)
{
  uint32_t USBx_BASE = (uint32_t)USBx;

  USBx_HOST->HCFG &= ~(USB_OTG_HCFG_FSLSPCS);
  USBx_HOST->HCFG |= (uint32_t)freq & USB_OTG_HCFG_FSLSPCS;

  if (freq == HCFG_48_MHZ)
  {
    USBx_HOST->HFIR = 48000U;
  }
  else if (freq == HCFG_6_MHZ)
  {
    USBx_HOST->HFIR = 6000U;
  }
  else
  {
    /* ... */
  }

  return HAL_OK;
}

/**
* @brief  USB_OTG_ResetPort : Reset Host Port
  * @param  USBx  Selected device
  * @retval HAL status
  * @note (1)The application must wait at least 10 ms
  *   before clearing the reset bit.
  */
HAL_StatusTypeDef USB_ResetPort(USB_OTG_GlobalTypeDef *USBx)
{
  uint32_t USBx_BASE = (uint32_t)USBx;

  __IO uint32_t hprt0 = 0U;

  hprt0 = USBx_HPRT0;

  hprt0 &= ~(USB_OTG_HPRT_PENA | USB_OTG_HPRT_PCDET |
             USB_OTG_HPRT_PENCHNG | USB_OTG_HPRT_POCCHNG);

  USBx_HPRT0 = (USB_OTG_HPRT_PRST | hprt0);
  HAL_Delay(100U);                                 /* See Note #1 */
  USBx_HPRT0 = ((~USB_OTG_HPRT_PRST) & hprt0);
  HAL_Delay(10U);

  return HAL_OK;
}

/**
  * @brief  USB_DriveVbus : activate or de-activate vbus
  * @param  state  VBUS state
  *          This parameter can be one of these values:
  *           0 : VBUS Active
  *           1 : VBUS Inactive
  * @retval HAL status
*/
HAL_StatusTypeDef USB_DriveVbus(USB_OTG_GlobalTypeDef *USBx, uint8_t state)
{
  uint32_t USBx_BASE = (uint32_t)USBx;
  __IO uint32_t hprt0 = 0U;

  hprt0 = USBx_HPRT0;

  hprt0 &= ~(USB_OTG_HPRT_PENA | USB_OTG_HPRT_PCDET |
             USB_OTG_HPRT_PENCHNG | USB_OTG_HPRT_POCCHNG);

  if (((hprt0 & USB_OTG_HPRT_PPWR) == 0U) && (state == 1U))
  {
    USBx_HPRT0 = (USB_OTG_HPRT_PPWR | hprt0);
  }
  if (((hprt0 & USB_OTG_HPRT_PPWR) == USB_OTG_HPRT_PPWR) && (state == 0U))
  {
    USBx_HPRT0 = ((~USB_OTG_HPRT_PPWR) & hprt0);
  }
  return HAL_OK;
}

/**
  * @brief  Return Host Core speed
  * @param  USBx  Selected device
  * @retval speed : Host speed
  *          This parameter can be one of these values:
  *            @arg HCD_SPEED_FULL: Full speed mode
  *            @arg HCD_SPEED_LOW: Low speed mode
  */
uint32_t USB_GetHostSpeed(USB_OTG_GlobalTypeDef *USBx)
{
  uint32_t USBx_BASE = (uint32_t)USBx;
  __IO uint32_t hprt0 = 0U;

  hprt0 = USBx_HPRT0;
  return ((hprt0 & USB_OTG_HPRT_PSPD) >> 17);
}

/**
  * @brief  Return Host Current Frame number
  * @param  USBx  Selected device
  * @retval current frame number
*/
uint32_t USB_GetCurrentFrame(USB_OTG_GlobalTypeDef *USBx)
{
  uint32_t USBx_BASE = (uint32_t)USBx;

  return (USBx_HOST->HFNUM & USB_OTG_HFNUM_FRNUM);
}

/**
  * @brief  Initialize a host channel
  * @param  USBx  Selected device
  * @param  ch_num  Channel number
  *         This parameter can be a value from 1 to 15
  * @param  epnum  Endpoint number
  *          This parameter can be a value from 1 to 15
  * @param  dev_address  Current device address
  *          This parameter can be a value from 0 to 255
  * @param  speed  Current device speed
  *          This parameter can be one of these values:
  *            @arg USB_OTG_SPEED_FULL: Full speed mode
  *            @arg USB_OTG_SPEED_LOW: Low speed mode
  * @param  ep_type  Endpoint Type
  *          This parameter can be one of these values:
  *            @arg EP_TYPE_CTRL: Control type
  *            @arg EP_TYPE_ISOC: Isochronous type
  *            @arg EP_TYPE_BULK: Bulk type
  *            @arg EP_TYPE_INTR: Interrupt type
  * @param  mps  Max Packet Size
  *          This parameter can be a value from 0 to32K
  * @retval HAL state
  */
HAL_StatusTypeDef USB_HC_Init(USB_OTG_GlobalTypeDef *USBx,
                              uint8_t ch_num,
                              uint8_t epnum,
                              uint8_t dev_address,
                              uint8_t speed,
                              uint8_t ep_type,
                              uint16_t mps)
{
  HAL_StatusTypeDef ret = HAL_OK;
  uint32_t USBx_BASE = (uint32_t)USBx;
  uint32_t HCcharEpDir, HCcharLowSpeed;

  /* Clear old interrupt conditions for this host channel. */
  USBx_HC((uint32_t)ch_num)->HCINT = 0xFFFFFFFFU;

  /* Enable channel interrupts required for this transfer. */
  switch (ep_type)
  {
    case EP_TYPE_CTRL:
    case EP_TYPE_BULK:
      USBx_HC((uint32_t)ch_num)->HCINTMSK = USB_OTG_HCINTMSK_XFRCM  |
                                            USB_OTG_HCINTMSK_STALLM |
                                            USB_OTG_HCINTMSK_TXERRM |
                                            USB_OTG_HCINTMSK_DTERRM |
                                            USB_OTG_HCINTMSK_AHBERR |
                                            USB_OTG_HCINTMSK_NAKM;

      if ((epnum & 0x80U) == 0x80U)
      {
        USBx_HC((uint32_t)ch_num)->HCINTMSK |= USB_OTG_HCINTMSK_BBERRM;
      }
      break;

    case EP_TYPE_INTR:
      USBx_HC((uint32_t)ch_num)->HCINTMSK = USB_OTG_HCINTMSK_XFRCM  |
                                            USB_OTG_HCINTMSK_STALLM |
                                            USB_OTG_HCINTMSK_TXERRM |
                                            USB_OTG_HCINTMSK_DTERRM |
                                            USB_OTG_HCINTMSK_NAKM   |
                                            USB_OTG_HCINTMSK_AHBERR |
                                            USB_OTG_HCINTMSK_FRMORM;

      if ((epnum & 0x80U) == 0x80U)
      {
        USBx_HC((uint32_t)ch_num)->HCINTMSK |= USB_OTG_HCINTMSK_BBERRM;
      }

      break;

    case EP_TYPE_ISOC:
      USBx_HC((uint32_t)ch_num)->HCINTMSK = USB_OTG_HCINTMSK_XFRCM  |
                                            USB_OTG_HCINTMSK_ACKM   |
                                            USB_OTG_HCINTMSK_AHBERR |
                                            USB_OTG_HCINTMSK_FRMORM;

      if ((epnum & 0x80U) == 0x80U)
      {
        USBx_HC((uint32_t)ch_num)->HCINTMSK |= (USB_OTG_HCINTMSK_TXERRM | USB_OTG_HCINTMSK_BBERRM);
      }
      break;

    default:
      ret = HAL_ERROR;
      break;
  }

  /* Enable the top level host channel interrupt. */
  USBx_HOST->HAINTMSK |= 1UL << (ch_num & 0xFU);

  /* Make sure host channel interrupts are enabled. */
  USBx->GINTMSK |= USB_OTG_GINTMSK_HCIM;

  /* Program the HCCHAR register */
  if ((epnum & 0x80U) == 0x80U)
  {
    HCcharEpDir = (0x1U << 15) & USB_OTG_HCCHAR_EPDIR;
  }
  else
  {
    HCcharEpDir = 0U;
  }

  if (speed == HPRT0_PRTSPD_LOW_SPEED)
  {
    HCcharLowSpeed = (0x1U << 17) & USB_OTG_HCCHAR_LSDEV;
  }
  else
  {
    HCcharLowSpeed = 0U;
  }

  USBx_HC((uint32_t)ch_num)->HCCHAR = (((uint32_t)dev_address << 22) & USB_OTG_HCCHAR_DAD) |
                                      ((((uint32_t)epnum & 0x7FU) << 11) & USB_OTG_HCCHAR_EPNUM) |
                                      (((uint32_t)ep_type << 18) & USB_OTG_HCCHAR_EPTYP) |
                                      ((uint32_t)mps & USB_OTG_HCCHAR_MPSIZ) | HCcharEpDir | HCcharLowSpeed;

  if (ep_type == EP_TYPE_INTR)
  {
    USBx_HC((uint32_t)ch_num)->HCCHAR |= USB_OTG_HCCHAR_ODDFRM ;
  }

  return ret;
}

/**
  * @brief  Start a transfer over a host channel
  * @param  USBx  Selected device
  * @param  hc  pointer to host channel structure
  * @retval HAL state
  */
HAL_StatusTypeDef USB_HC_StartXfer(USB_OTG_GlobalTypeDef *USBx, USB_OTG_HCTypeDef *hc)
{
  uint32_t USBx_BASE = (uint32_t)USBx;
  uint32_t ch_num = (uint32_t)hc->ch_num;
  static __IO uint32_t tmpreg = 0U;
  uint8_t  is_oddframe;
  uint16_t len_words;
  uint16_t num_packets;
  uint16_t max_hc_pkt_count = 256U;

  /* Compute the expected number of packets associated to the transfer */
  if (hc->xfer_len > 0U)
  {
    num_packets = (uint16_t)((hc->xfer_len + hc->max_packet - 1U) / hc->max_packet);

    if (num_packets > max_hc_pkt_count)
    {
      num_packets = max_hc_pkt_count;
      hc->xfer_len = (uint32_t)num_packets * hc->max_packet;
    }
  }
  else
  {
    num_packets = 1U;
  }
  if (hc->ep_is_in != 0U)
  {
    hc->xfer_len = (uint32_t)num_packets * hc->max_packet;
  }

  /* Initialize the HCTSIZn register */
  USBx_HC(ch_num)->HCTSIZ = (hc->xfer_len & USB_OTG_HCTSIZ_XFRSIZ) |
                            (((uint32_t)num_packets << 19) & USB_OTG_HCTSIZ_PKTCNT) |
                            (((uint32_t)hc->data_pid << 29) & USB_OTG_HCTSIZ_DPID);

  is_oddframe = (((uint32_t)USBx_HOST->HFNUM & 0x01U) != 0U) ? 0U : 1U;
  USBx_HC(ch_num)->HCCHAR &= ~USB_OTG_HCCHAR_ODDFRM;
  USBx_HC(ch_num)->HCCHAR |= (uint32_t)is_oddframe << 29;

  /* Set host channel enable */
  tmpreg = USBx_HC(ch_num)->HCCHAR;
  tmpreg &= ~USB_OTG_HCCHAR_CHDIS;

  /* make sure to set the correct ep direction */
  if (hc->ep_is_in != 0U)
  {
    tmpreg |= USB_OTG_HCCHAR_EPDIR;
  }
  else
  {
    tmpreg &= ~USB_OTG_HCCHAR_EPDIR;
  }
  tmpreg |= USB_OTG_HCCHAR_CHENA;
  USBx_HC(ch_num)->HCCHAR = tmpreg;

    if ((hc->ep_is_in == 0U) && (hc->xfer_len > 0U))
    {
      switch (hc->ep_type)
      {
        /* Non periodic transfer */
        case EP_TYPE_CTRL:
        case EP_TYPE_BULK:

          len_words = (uint16_t)((hc->xfer_len + 3U) / 4U);

          /* check if there is enough space in FIFO space */
          if (len_words > (USBx->HNPTXSTS & 0xFFFFU))
          {
            /* need to process data in nptxfempty interrupt */
            USBx->GINTMSK |= USB_OTG_GINTMSK_NPTXFEM;
          }
          break;

        /* Periodic transfer */
        case EP_TYPE_INTR:
        case EP_TYPE_ISOC:
          len_words = (uint16_t)((hc->xfer_len + 3U) / 4U);
          /* check if there is enough space in FIFO space */
          if (len_words > (USBx_HOST->HPTXSTS & 0xFFFFU)) /* split the transfer */
          {
            /* need to process data in ptxfempty interrupt */
            USBx->GINTMSK |= USB_OTG_GINTMSK_PTXFEM;
          }
          break;

        default:
          break;
      }

      /* Write packet into the Tx FIFO. */
      (void)USB_WritePacket(USBx, hc->xfer_buff, hc->ch_num, (uint16_t)hc->xfer_len);
    }

  return HAL_OK;
}

/**
  * @brief Read all host channel interrupts status
  * @param  USBx  Selected device
  * @retval HAL state
  */
uint32_t USB_HC_ReadInterrupt(USB_OTG_GlobalTypeDef *USBx)
{
  uint32_t USBx_BASE = (uint32_t)USBx;

  return ((USBx_HOST->HAINT) & 0xFFFFU);
}

/**
  * @brief  Halt a host channel
  * @param  USBx  Selected device
  * @param  hc_num  Host Channel number
  *         This parameter can be a value from 1 to 15
  * @retval HAL state
  */
HAL_StatusTypeDef USB_HC_Halt(USB_OTG_GlobalTypeDef *USBx, uint8_t hc_num)
{
  uint32_t USBx_BASE = (uint32_t)USBx;
  uint32_t hcnum = (uint32_t)hc_num;
  uint32_t count = 0U;
  uint32_t HcEpType = (USBx_HC(hcnum)->HCCHAR & USB_OTG_HCCHAR_EPTYP) >> 18;

  /* Check for space in the request queue to issue the halt. */
  if ((HcEpType == HCCHAR_CTRL) || (HcEpType == HCCHAR_BULK))
  {
    USBx_HC(hcnum)->HCCHAR |= USB_OTG_HCCHAR_CHDIS;

    if ((USBx->HNPTXSTS & (0xFFU << 16)) == 0U)
    {
      USBx_HC(hcnum)->HCCHAR &= ~USB_OTG_HCCHAR_CHENA;
      USBx_HC(hcnum)->HCCHAR |= USB_OTG_HCCHAR_CHENA;
      USBx_HC(hcnum)->HCCHAR &= ~USB_OTG_HCCHAR_EPDIR;
      do
      {
        if (++count > 1000U)
        {
          break;
        }
      }
      while ((USBx_HC(hcnum)->HCCHAR & USB_OTG_HCCHAR_CHENA) == USB_OTG_HCCHAR_CHENA);
    }
    else
    {
      USBx_HC(hcnum)->HCCHAR |= USB_OTG_HCCHAR_CHENA;
    }
  }
  else
  {
    USBx_HC(hcnum)->HCCHAR |= USB_OTG_HCCHAR_CHDIS;

    if ((USBx_HOST->HPTXSTS & (0xFFU << 16)) == 0U)
    {
      USBx_HC(hcnum)->HCCHAR &= ~USB_OTG_HCCHAR_CHENA;
      USBx_HC(hcnum)->HCCHAR |= USB_OTG_HCCHAR_CHENA;
      USBx_HC(hcnum)->HCCHAR &= ~USB_OTG_HCCHAR_EPDIR;
      do
      {
        if (++count > 1000U)
        {
          break;
        }
      }
      while ((USBx_HC(hcnum)->HCCHAR & USB_OTG_HCCHAR_CHENA) == USB_OTG_HCCHAR_CHENA);
    }
    else
    {
      USBx_HC(hcnum)->HCCHAR |= USB_OTG_HCCHAR_CHENA;
    }
  }

  return HAL_OK;
}

/**
  * @brief  Initiate Do Ping protocol
  * @param  USBx  Selected device
  * @param  hc_num  Host Channel number
  *         This parameter can be a value from 1 to 15
  * @retval HAL state
  */
HAL_StatusTypeDef USB_DoPing(USB_OTG_GlobalTypeDef *USBx, uint8_t ch_num)
{
  uint32_t USBx_BASE = (uint32_t)USBx;
  uint32_t chnum = (uint32_t)ch_num;
  uint32_t num_packets = 1U;
  uint32_t tmpreg;

  USBx_HC(chnum)->HCTSIZ = ((num_packets << 19) & USB_OTG_HCTSIZ_PKTCNT) |
                           USB_OTG_HCTSIZ_DOPING;

  /* Set host channel enable */
  tmpreg = USBx_HC(chnum)->HCCHAR;
  tmpreg &= ~USB_OTG_HCCHAR_CHDIS;
  tmpreg |= USB_OTG_HCCHAR_CHENA;
  USBx_HC(chnum)->HCCHAR = tmpreg;

  return HAL_OK;
}

/**
  * @brief  Stop Host Core
  * @param  USBx  Selected device
  * @retval HAL state
  */
HAL_StatusTypeDef USB_StopHost(USB_OTG_GlobalTypeDef *USBx)
{
  uint32_t USBx_BASE = (uint32_t)USBx;
  uint32_t count = 0U;
  uint32_t value;
  uint32_t i;


  (void)USB_DisableGlobalInt(USBx);

  /* Flush FIFO */
  (void)USB_FlushTxFifo(USBx, 0x10U);
  (void)USB_FlushRxFifo(USBx);

  /* Flush out any leftover queued requests. */
  for (i = 0U; i <= 15U; i++)
  {
    value = USBx_HC(i)->HCCHAR;
    value |=  USB_OTG_HCCHAR_CHDIS;
    value &= ~USB_OTG_HCCHAR_CHENA;
    value &= ~USB_OTG_HCCHAR_EPDIR;
    USBx_HC(i)->HCCHAR = value;
  }

  /* Halt all channels to put them into a known state. */
  for (i = 0U; i <= 15U; i++)
  {
    value = USBx_HC(i)->HCCHAR;
    value |= USB_OTG_HCCHAR_CHDIS;
    value |= USB_OTG_HCCHAR_CHENA;
    value &= ~USB_OTG_HCCHAR_EPDIR;
    USBx_HC(i)->HCCHAR = value;

    do
    {
      if (++count > 1000U)
      {
        break;
      }
    }
    while ((USBx_HC(i)->HCCHAR & USB_OTG_HCCHAR_CHENA) == USB_OTG_HCCHAR_CHENA);
  }

  /* Clear any pending Host interrupts */
  USBx_HOST->HAINT = 0xFFFFFFFFU;
  USBx->GINTSTS = 0xFFFFFFFFU;
  (void)USB_EnableGlobalInt(USBx);

  return HAL_OK;
}

/**
  * @brief  USB_ActivateRemoteWakeup active remote wakeup signalling
  * @param  USBx Selected device
  * @retval HAL status
  */
HAL_StatusTypeDef USB_ActivateRemoteWakeup(USB_OTG_GlobalTypeDef *USBx)
{
  uint32_t USBx_BASE = (uint32_t)USBx;

  if ((USBx_DEVICE->DSTS & USB_OTG_DSTS_SUSPSTS) == USB_OTG_DSTS_SUSPSTS)
  {
    /* active Remote wakeup signalling */
    USBx_DEVICE->DCTL |= USB_OTG_DCTL_RWUSIG;
  }

  return HAL_OK;
}

/**
  * @brief  USB_DeActivateRemoteWakeup de-active remote wakeup signalling
  * @param  USBx Selected device
  * @retval HAL status
  */
HAL_StatusTypeDef USB_DeActivateRemoteWakeup(USB_OTG_GlobalTypeDef *USBx)
{
  uint32_t USBx_BASE = (uint32_t)USBx;

  /* active Remote wakeup signalling */
  USBx_DEVICE->DCTL &= ~(USB_OTG_DCTL_RWUSIG);

  return HAL_OK;
}
#endif /* defined (USB_OTG_FS) */

#if defined (USB)
/**
  * @brief  Initializes the USB Core
  * @param  USBx: USB Instance
  * @param  cfg : pointer to a USB_CfgTypeDef structure that contains
  *         the configuration information for the specified USBx peripheral.
  * @retval HAL status
  */
HAL_StatusTypeDef USB_CoreInit(USB_TypeDef *USBx, USB_CfgTypeDef cfg)
{
  /* Prevent unused argument(s) compilation warning */
  UNUSED(USBx);
  UNUSED(cfg);

  /* NOTE : - This function is not required by USB Device FS peripheral, it is used
              only by USB OTG FS peripheral.
            - This function is added to ensure compatibility across platforms.
   */

  return HAL_OK;
}

/**
  * @brief  USB_EnableGlobalInt
  *         Enables the controller's Global Int in the AHB Config reg
  * @param  USBx : Selected device
  * @retval HAL status
  */
HAL_StatusTypeDef USB_EnableGlobalInt(USB_TypeDef *USBx)
{
  uint16_t winterruptmask;

  /* Set winterruptmask variable */
  winterruptmask = USB_CNTR_CTRM  | USB_CNTR_WKUPM |
                   USB_CNTR_SUSPM | USB_CNTR_ERRM |
                   USB_CNTR_SOFM | USB_CNTR_ESOFM |
                   USB_CNTR_RESETM | USB_CNTR_L1REQM;

  /* Set interrupt mask */
  USBx->CNTR |= winterruptmask;

  return HAL_OK;
}

/**
  * @brief  USB_DisableGlobalInt
  *         Disable the controller's Global Int in the AHB Config reg
  * @param  USBx : Selected device
  * @retval HAL status
*/
HAL_StatusTypeDef USB_DisableGlobalInt(USB_TypeDef *USBx)
{
  uint16_t winterruptmask;

  /* Set winterruptmask variable */
  winterruptmask = USB_CNTR_CTRM  | USB_CNTR_WKUPM |
                   USB_CNTR_SUSPM | USB_CNTR_ERRM |
                   USB_CNTR_SOFM | USB_CNTR_ESOFM |
                   USB_CNTR_RESETM | USB_CNTR_L1REQM;

  /* Clear interrupt mask */
  USBx->CNTR &= ~winterruptmask;

  return HAL_OK;
}

/**
  * @brief  USB_SetCurrentMode : Set functional mode
  * @param  USBx : Selected device
  * @param  mode :  current core mode
  *          This parameter can be one of the these values:
  *            @arg USB_DEVICE_MODE: Peripheral mode mode
  * @retval HAL status
  */
HAL_StatusTypeDef USB_SetCurrentMode(USB_TypeDef *USBx, USB_ModeTypeDef mode)
{
  /* Prevent unused argument(s) compilation warning */
  UNUSED(USBx);
  UNUSED(mode);

  /* NOTE : - This function is not required by USB Device FS peripheral, it is used
              only by USB OTG FS peripheral.
            - This function is added to ensure compatibility across platforms.
   */
  return HAL_OK;
}

/**
  * @brief  USB_DevInit : Initializes the USB controller registers
  *         for device mode
  * @param  USBx : Selected device
  * @param  cfg  : pointer to a USB_CfgTypeDef structure that contains
  *         the configuration information for the specified USBx peripheral.
  * @retval HAL status
  */
HAL_StatusTypeDef USB_DevInit(USB_TypeDef *USBx, USB_CfgTypeDef cfg)
{
  /* Prevent unused argument(s) compilation warning */
  UNUSED(cfg);

  /* Init Device */
  /*CNTR_FRES = 1*/
  USBx->CNTR = USB_CNTR_FRES;

  /*CNTR_FRES = 0*/
  USBx->CNTR = 0;

  /*Clear pending interrupts*/
  USBx->ISTR = 0;

  /*Set Btable Address*/
  USBx->BTABLE = BTABLE_ADDRESS;

  /* Enable USB Device Interrupt mask */
  (void)USB_EnableGlobalInt(USBx);

  return HAL_OK;
}

/**
  * @brief  USB_SetDevSpeed :Initializes the device speed
  *         depending on the PHY type and the enumeration speed of the device.
  * @param  USBx  Selected device
  * @param  speed  device speed
  * @retval  Hal status
  */
HAL_StatusTypeDef USB_SetDevSpeed(USB_TypeDef *USBx, uint8_t speed)
{
  /* Prevent unused argument(s) compilation warning */
  UNUSED(USBx);
  UNUSED(speed);

  /* NOTE : - This function is not required by USB Device FS peripheral, it is used
              only by USB OTG FS peripheral.
            - This function is added to ensure compatibility across platforms.
   */

  return HAL_OK;
}

/**
  * @brief  USB_FlushTxFifo : Flush a Tx FIFO
  * @param  USBx : Selected device
  * @param  num : FIFO number
  *         This parameter can be a value from 1 to 15
            15 means Flush all Tx FIFOs
  * @retval HAL status
  */
HAL_StatusTypeDef USB_FlushTxFifo(USB_TypeDef *USBx, uint32_t num)
{
  /* Prevent unused argument(s) compilation warning */
  UNUSED(USBx);
  UNUSED(num);

  /* NOTE : - This function is not required by USB Device FS peripheral, it is used
              only by USB OTG FS peripheral.
            - This function is added to ensure compatibility across platforms.
   */

  return HAL_OK;
}

/**
  * @brief  USB_FlushRxFifo : Flush Rx FIFO
  * @param  USBx : Selected device
  * @retval HAL status
  */
HAL_StatusTypeDef USB_FlushRxFifo(USB_TypeDef *USBx)
{
  /* Prevent unused argument(s) compilation warning */
  UNUSED(USBx);

  /* NOTE : - This function is not required by USB Device FS peripheral, it is used
              only by USB OTG FS peripheral.
            - This function is added to ensure compatibility across platforms.
   */

  return HAL_OK;
}

/**
  * @brief  Activate and configure an endpoint
  * @param  USBx : Selected device
  * @param  ep: pointer to endpoint structure
  * @retval HAL status
  */
HAL_StatusTypeDef USB_ActivateEndpoint(USB_TypeDef *USBx, USB_EPTypeDef *ep)
{
  HAL_StatusTypeDef ret = HAL_OK;
  uint16_t wEpRegVal;

  wEpRegVal = PCD_GET_ENDPOINT(USBx, ep->num) & USB_EP_T_MASK;

  /* initialize Endpoint */
  switch (ep->type)
  {
    case EP_TYPE_CTRL:
      wEpRegVal |= USB_EP_CONTROL;
      break;

    case EP_TYPE_BULK:
      wEpRegVal |= USB_EP_BULK;
      break;

    case EP_TYPE_INTR:
      wEpRegVal |= USB_EP_INTERRUPT;
      break;

    case EP_TYPE_ISOC:
      wEpRegVal |= USB_EP_ISOCHRONOUS;
      break;

    default:
      ret = HAL_ERROR;
      break;
  }

  PCD_SET_ENDPOINT(USBx, ep->num, wEpRegVal | USB_EP_CTR_RX | USB_EP_CTR_TX);

  PCD_SET_EP_ADDRESS(USBx, ep->num, ep->num);

  if (ep->doublebuffer == 0U)
  {
    if (ep->is_in != 0U)
    {
      /*Set the endpoint Transmit buffer address */
      PCD_SET_EP_TX_ADDRESS(USBx, ep->num, ep->pmaadress);
      PCD_CLEAR_TX_DTOG(USBx, ep->num);

      if (ep->type != EP_TYPE_ISOC)
      {
        /* Configure NAK status for the Endpoint */
        PCD_SET_EP_TX_STATUS(USBx, ep->num, USB_EP_TX_NAK);
      }
      else
      {
        /* Configure TX Endpoint to disabled state */
        PCD_SET_EP_TX_STATUS(USBx, ep->num, USB_EP_TX_DIS);
      }
    }
    else
    {
      /*Set the endpoint Receive buffer address */
      PCD_SET_EP_RX_ADDRESS(USBx, ep->num, ep->pmaadress);
      /*Set the endpoint Receive buffer counter*/
      PCD_SET_EP_RX_CNT(USBx, ep->num, ep->maxpacket);
      PCD_CLEAR_RX_DTOG(USBx, ep->num);
      /* Configure VALID status for the Endpoint*/
      PCD_SET_EP_RX_STATUS(USBx, ep->num, USB_EP_RX_VALID);
    }
  }
  /*Double Buffer*/
  else
  {
    /* Set the endpoint as double buffered */
    PCD_SET_EP_DBUF(USBx, ep->num);
    /* Set buffer address for double buffered mode */
    PCD_SET_EP_DBUF_ADDR(USBx, ep->num, ep->pmaaddr0, ep->pmaaddr1);

    if (ep->is_in == 0U)
    {
      /* Clear the data toggle bits for the endpoint IN/OUT */
      PCD_CLEAR_RX_DTOG(USBx, ep->num);
      PCD_CLEAR_TX_DTOG(USBx, ep->num);

      /* Reset value of the data toggle bits for the endpoint out */
      PCD_TX_DTOG(USBx, ep->num);

      PCD_SET_EP_RX_STATUS(USBx, ep->num, USB_EP_RX_VALID);
      PCD_SET_EP_TX_STATUS(USBx, ep->num, USB_EP_TX_DIS);
    }
    else
    {
      /* Clear the data toggle bits for the endpoint IN/OUT */
      PCD_CLEAR_RX_DTOG(USBx, ep->num);
      PCD_CLEAR_TX_DTOG(USBx, ep->num);
      PCD_RX_DTOG(USBx, ep->num);

      if (ep->type != EP_TYPE_ISOC)
      {
        /* Configure NAK status for the Endpoint */
        PCD_SET_EP_TX_STATUS(USBx, ep->num, USB_EP_TX_NAK);
      }
      else
      {
        /* Configure TX Endpoint to disabled state */
        PCD_SET_EP_TX_STATUS(USBx, ep->num, USB_EP_TX_DIS);
      }

      PCD_SET_EP_RX_STATUS(USBx, ep->num, USB_EP_RX_DIS);
    }
  }

  return ret;
}

/**
  * @brief  De-activate and de-initialize an endpoint
  * @param  USBx : Selected device
  * @param  ep: pointer to endpoint structure
  * @retval HAL status
  */
HAL_StatusTypeDef USB_DeactivateEndpoint(USB_TypeDef *USBx, USB_EPTypeDef *ep)
{
  if (ep->doublebuffer == 0U)
  {
    if (ep->is_in != 0U)
    {
      PCD_CLEAR_TX_DTOG(USBx, ep->num);
      /* Configure DISABLE status for the Endpoint*/
      PCD_SET_EP_TX_STATUS(USBx, ep->num, USB_EP_TX_DIS);
    }
    else
    {
      PCD_CLEAR_RX_DTOG(USBx, ep->num);
      /* Configure DISABLE status for the Endpoint*/
      PCD_SET_EP_RX_STATUS(USBx, ep->num, USB_EP_RX_DIS);
    }
  }
  /*Double Buffer*/
  else
  {
    if (ep->is_in == 0U)
    {
      /* Clear the data toggle bits for the endpoint IN/OUT*/
      PCD_CLEAR_RX_DTOG(USBx, ep->num);
      PCD_CLEAR_TX_DTOG(USBx, ep->num);

      /* Reset value of the data toggle bits for the endpoint out*/
      PCD_TX_DTOG(USBx, ep->num);

      PCD_SET_EP_RX_STATUS(USBx, ep->num, USB_EP_RX_DIS);
      PCD_SET_EP_TX_STATUS(USBx, ep->num, USB_EP_TX_DIS);
    }
    else
    {
      /* Clear the data toggle bits for the endpoint IN/OUT*/
      PCD_CLEAR_RX_DTOG(USBx, ep->num);
      PCD_CLEAR_TX_DTOG(USBx, ep->num);
      PCD_RX_DTOG(USBx, ep->num);
      /* Configure DISABLE status for the Endpoint*/
      PCD_SET_EP_TX_STATUS(USBx, ep->num, USB_EP_TX_DIS);
      PCD_SET_EP_RX_STATUS(USBx, ep->num, USB_EP_RX_DIS);
    }
  }

  return HAL_OK;
}

/**
  * @brief  USB_EPStartXfer : setup and starts a transfer over an EP
  * @param  USBx : Selected device
  * @param  ep: pointer to endpoint structure
  * @retval HAL status
  */
HAL_StatusTypeDef USB_EPStartXfer(USB_TypeDef *USBx, USB_EPTypeDef *ep)
{
  uint16_t pmabuffer;
  uint32_t len;

  /* IN endpoint */
  if (ep->is_in == 1U)
  {
    /*Multi packet transfer*/
    if (ep->xfer_len > ep->maxpacket)
    {
      len = ep->maxpacket;
      ep->xfer_len -= len;
    }
    else
    {
      len = ep->xfer_len;
      ep->xfer_len = 0U;
    }

    /* configure and validate Tx endpoint */
    if (ep->doublebuffer == 0U)
    {
      USB_WritePMA(USBx, ep->xfer_buff, ep->pmaadress, (uint16_t)len);
      PCD_SET_EP_TX_CNT(USBx, ep->num, len);
    }
    else
    {
      /* Write the data to the USB endpoint */
      if ((PCD_GET_ENDPOINT(USBx, ep->num) & USB_EP_DTOG_TX) != 0U)
      {
        /* Set the Double buffer counter for pmabuffer1 */
        PCD_SET_EP_DBUF1_CNT(USBx, ep->num, ep->is_in, len);
        pmabuffer = ep->pmaaddr1;
      }
      else
      {
        /* Set the Double buffer counter for pmabuffer0 */
        PCD_SET_EP_DBUF0_CNT(USBx, ep->num, ep->is_in, len);
        pmabuffer = ep->pmaaddr0;
      }
      USB_WritePMA(USBx, ep->xfer_buff, pmabuffer, (uint16_t)len);
      PCD_FreeUserBuffer(USBx, ep->num, ep->is_in);
    }

    PCD_SET_EP_TX_STATUS(USBx, ep->num, USB_EP_TX_VALID);
  }
  else /* OUT endpoint */
  {
    /* Multi packet transfer*/
    if (ep->xfer_len > ep->maxpacket)
    {
      len = ep->maxpacket;
      ep->xfer_len -= len;
    }
    else
    {
      len = ep->xfer_len;
      ep->xfer_len = 0U;
    }

    /* configure and validate Rx endpoint */
    if (ep->doublebuffer == 0U)
    {
      /*Set RX buffer count*/
      PCD_SET_EP_RX_CNT(USBx, ep->num, len);
    }
    else
    {
      /*Set the Double buffer counter*/
      PCD_SET_EP_DBUF_CNT(USBx, ep->num, ep->is_in, len);
    }

    PCD_SET_EP_RX_STATUS(USBx, ep->num, USB_EP_RX_VALID);
  }

  return HAL_OK;
}

/**
  * @brief  USB_WritePacket : Writes a packet into the Tx FIFO associated
  *         with the EP/channel
  * @param  USBx : Selected device
  * @param  src :  pointer to source buffer
  * @param  ch_ep_num : endpoint or host channel number
  * @param  len : Number of bytes to write
  * @retval HAL status
  */
HAL_StatusTypeDef USB_WritePacket(USB_TypeDef *USBx, uint8_t *src, uint8_t ch_ep_num, uint16_t len)
{
  /* Prevent unused argument(s) compilation warning */
  UNUSED(USBx);
  UNUSED(src);
  UNUSED(ch_ep_num);
  UNUSED(len);
  /* NOTE : - This function is not required by USB Device FS peripheral, it is used
              only by USB OTG FS peripheral.
            - This function is added to ensure compatibility across platforms.
   */
  return HAL_OK;
}

/**
  * @brief  USB_ReadPacket : read a packet from the Tx FIFO associated
  *         with the EP/channel
  * @param  USBx : Selected device
  * @param  dest : destination pointer
  * @param  len : Number of bytes to read
  * @retval pointer to destination buffer
  */
void *USB_ReadPacket(USB_TypeDef *USBx, uint8_t *dest, uint16_t len)
{
  /* Prevent unused argument(s) compilation warning */
  UNUSED(USBx);
  UNUSED(dest);
  UNUSED(len);
  /* NOTE : - This function is not required by USB Device FS peripheral, it is used
              only by USB OTG FS peripheral.
            - This function is added to ensure compatibility across platforms.
   */
  return ((void *)NULL);
}

/**
  * @brief  USB_EPSetStall : set a stall condition over an EP
  * @param  USBx : Selected device
  * @param  ep: pointer to endpoint structure
  * @retval HAL status
  */
HAL_StatusTypeDef USB_EPSetStall(USB_TypeDef *USBx, USB_EPTypeDef *ep)
{
  if (ep->is_in != 0U)
  {
    PCD_SET_EP_TX_STATUS(USBx, ep->num, USB_EP_TX_STALL);
  }
  else
  {
    PCD_SET_EP_RX_STATUS(USBx, ep->num, USB_EP_RX_STALL);
  }

  return HAL_OK;
}

/**
  * @brief  USB_EPClearStall : Clear a stall condition over an EP
  * @param  USBx : Selected device
  * @param  ep: pointer to endpoint structure
  * @retval HAL status
  */
HAL_StatusTypeDef USB_EPClearStall(USB_TypeDef *USBx, USB_EPTypeDef *ep)
{
  if (ep->doublebuffer == 0U)
  {
    if (ep->is_in != 0U)
    {
      PCD_CLEAR_TX_DTOG(USBx, ep->num);

      if (ep->type != EP_TYPE_ISOC)
      {
        /* Configure NAK status for the Endpoint */
        PCD_SET_EP_TX_STATUS(USBx, ep->num, USB_EP_TX_NAK);
      }
    }
    else
    {
      PCD_CLEAR_RX_DTOG(USBx, ep->num);

      /* Configure VALID status for the Endpoint*/
      PCD_SET_EP_RX_STATUS(USBx, ep->num, USB_EP_RX_VALID);
    }
  }

  return HAL_OK;
}

/**
  * @brief  USB_StopDevice : Stop the usb device mode
  * @param  USBx : Selected device
  * @retval HAL status
  */
HAL_StatusTypeDef USB_StopDevice(USB_TypeDef *USBx)
{
  /* disable all interrupts and force USB reset */
  USBx->CNTR = USB_CNTR_FRES;

  /* clear interrupt status register */
  USBx->ISTR = 0;

  /* switch-off device */
  USBx->CNTR = (USB_CNTR_FRES | USB_CNTR_PDWN);

  return HAL_OK;
}

/**
  * @brief  USB_SetDevAddress : Stop the usb device mode
  * @param  USBx : Selected device
  * @param  address : new device address to be assigned
  *          This parameter can be a value from 0 to 255
  * @retval HAL status
  */
HAL_StatusTypeDef  USB_SetDevAddress(USB_TypeDef *USBx, uint8_t address)
{
  if (address == 0U)
  {
    /* set device address and enable function */
    USBx->DADDR = USB_DADDR_EF;
  }

  return HAL_OK;
}

/**
  * @brief  USB_DevConnect : Connect the USB device by enabling the pull-up/pull-down
  * @param  USBx : Selected device
  * @retval HAL status
  */
HAL_StatusTypeDef  USB_DevConnect(USB_TypeDef *USBx)
{
  /* Enabling DP Pull-UP bit to Connect internal PU resistor on USB DP line */
  USBx->BCDR |= USB_BCDR_DPPU;

  return HAL_OK;
}

/**
  * @brief  USB_DevDisconnect : Disconnect the USB device by disabling the pull-up/pull-down
  * @param  USBx : Selected device
  * @retval HAL status
  */
HAL_StatusTypeDef  USB_DevDisconnect(USB_TypeDef *USBx)
{
  /* Disable DP Pull-Up bit to disconnect the Internal PU resistor on USB DP line */
  USBx->BCDR &= (uint16_t)(~(USB_BCDR_DPPU));

  return HAL_OK;
}

/**
  * @brief  USB_ReadInterrupts: return the global USB interrupt status
  * @param  USBx : Selected device
  * @retval HAL status
  */
uint32_t  USB_ReadInterrupts(USB_TypeDef *USBx)
{
  uint32_t tmpreg;

  tmpreg = USBx->ISTR;
  return tmpreg;
}

/**
  * @brief  USB_ReadDevAllOutEpInterrupt: return the USB device OUT endpoints interrupt status
  * @param  USBx : Selected device
  * @retval HAL status
  */
uint32_t USB_ReadDevAllOutEpInterrupt(USB_TypeDef *USBx)
{
  /* Prevent unused argument(s) compilation warning */
  UNUSED(USBx);
  /* NOTE : - This function is not required by USB Device FS peripheral, it is used
              only by USB OTG FS peripheral.
            - This function is added to ensure compatibility across platforms.
   */
  return (0);
}

/**
  * @brief  USB_ReadDevAllInEpInterrupt: return the USB device IN endpoints interrupt status
  * @param  USBx : Selected device
  * @retval HAL status
  */
uint32_t USB_ReadDevAllInEpInterrupt(USB_TypeDef *USBx)
{
  /* Prevent unused argument(s) compilation warning */
  UNUSED(USBx);
  /* NOTE : - This function is not required by USB Device FS peripheral, it is used
              only by USB OTG FS peripheral.
            - This function is added to ensure compatibility across platforms.
   */
  return (0);
}

/**
  * @brief  Returns Device OUT EP Interrupt register
  * @param  USBx : Selected device
  * @param  epnum : endpoint number
  *          This parameter can be a value from 0 to 15
  * @retval Device OUT EP Interrupt register
  */
uint32_t USB_ReadDevOutEPInterrupt(USB_TypeDef *USBx, uint8_t epnum)
{
  /* Prevent unused argument(s) compilation warning */
  UNUSED(USBx);
  UNUSED(epnum);
  /* NOTE : - This function is not required by USB Device FS peripheral, it is used
              only by USB OTG FS peripheral.
            - This function is added to ensure compatibility across platforms.
   */
  return (0);
}

/**
  * @brief  Returns Device IN EP Interrupt register
  * @param  USBx : Selected device
  * @param  epnum : endpoint number
  *          This parameter can be a value from 0 to 15
  * @retval Device IN EP Interrupt register
  */
uint32_t USB_ReadDevInEPInterrupt(USB_TypeDef *USBx, uint8_t epnum)
{
  /* Prevent unused argument(s) compilation warning */
  UNUSED(USBx);
  UNUSED(epnum);
  /* NOTE : - This function is not required by USB Device FS peripheral, it is used
              only by USB OTG FS peripheral.
            - This function is added to ensure compatibility across platforms.
   */
  return (0);
}

/**
  * @brief  USB_ClearInterrupts: clear a USB interrupt
  * @param  USBx  Selected device
  * @param  interrupt  interrupt flag
  * @retval None
  */
void  USB_ClearInterrupts(USB_TypeDef *USBx, uint32_t interrupt)
{
  /* Prevent unused argument(s) compilation warning */
  UNUSED(USBx);
  UNUSED(interrupt);
  /* NOTE : - This function is not required by USB Device FS peripheral, it is used
              only by USB OTG FS peripheral.
            - This function is added to ensure compatibility across platforms.
   */
}

/**
  * @brief  Prepare the EP0 to start the first control setup
  * @param  USBx  Selected device
  * @param  psetup  pointer to setup packet
  * @retval HAL status
  */
HAL_StatusTypeDef USB_EP0_OutStart(USB_TypeDef *USBx, uint8_t *psetup)
{
  /* Prevent unused argument(s) compilation warning */
  UNUSED(USBx);
  UNUSED(psetup);
  /* NOTE : - This function is not required by USB Device FS peripheral, it is used
              only by USB OTG FS peripheral.
            - This function is added to ensure compatibility across platforms.
   */
  return HAL_OK;
}

/**
  * @brief  USB_ActivateRemoteWakeup : active remote wakeup signalling
  * @param  USBx  Selected device
  * @retval HAL status
  */
HAL_StatusTypeDef USB_ActivateRemoteWakeup(USB_TypeDef *USBx)
{
  USBx->CNTR |= USB_CNTR_RESUME;

  return HAL_OK;
}

/**
  * @brief  USB_DeActivateRemoteWakeup : de-active remote wakeup signalling
  * @param  USBx  Selected device
  * @retval HAL status
  */
HAL_StatusTypeDef USB_DeActivateRemoteWakeup(USB_TypeDef *USBx)
{
  USBx->CNTR &= ~(USB_CNTR_RESUME);
  return HAL_OK;
}

/**
  * @brief Copy a buffer from user memory area to packet memory area (PMA)
  * @param   USBx USB peripheral instance register address.
  * @param   pbUsrBuf pointer to user memory area.
  * @param   wPMABufAddr address into PMA.
  * @param   wNBytes: no. of bytes to be copied.
  * @retval None
  */
void USB_WritePMA(USB_TypeDef *USBx, uint8_t *pbUsrBuf, uint16_t wPMABufAddr, uint16_t wNBytes)
{
  uint32_t n = ((uint32_t)wNBytes + 1U) >> 1;
  uint32_t BaseAddr = (uint32_t)USBx;
  uint32_t i, temp1, temp2;
  uint16_t *pdwVal;
  uint8_t *pBuf = pbUsrBuf;

  pdwVal = (uint16_t *)(BaseAddr + 0x400U + ((uint32_t)wPMABufAddr * PMA_ACCESS));

  for (i = n; i != 0U; i--)
  {
    temp1 = (uint16_t) * pBuf;
    pBuf++;
    temp2 = temp1 | ((uint16_t)((uint16_t) * pBuf << 8));
    *pdwVal = (uint16_t)temp2;
    pdwVal++;

#if PMA_ACCESS > 1U
    pdwVal++;
#endif

    pBuf++;
  }
}

/**
  * @brief Copy a buffer from user memory area to packet memory area (PMA)
  * @param   USBx: USB peripheral instance register address.
  * @param   pbUsrBuf pointer to user memory area.
  * @param   wPMABufAddr address into PMA.
  * @param   wNBytes: no. of bytes to be copied.
  * @retval None
  */
void USB_ReadPMA(USB_TypeDef *USBx, uint8_t *pbUsrBuf, uint16_t wPMABufAddr, uint16_t wNBytes)
{
  uint32_t n = (uint32_t)wNBytes >> 1;
  uint32_t BaseAddr = (uint32_t)USBx;
  uint32_t i, temp;
  uint16_t *pdwVal;
  uint8_t *pBuf = pbUsrBuf;

  pdwVal = (uint16_t *)(BaseAddr + 0x400U + ((uint32_t)wPMABufAddr * PMA_ACCESS));

  for (i = n; i != 0U; i--)
  {
    temp = *pdwVal;
    pdwVal++;
    *pBuf = (uint8_t)((temp >> 0) & 0xFFU);
    pBuf++;
    *pBuf = (uint8_t)((temp >> 8) & 0xFFU);
    pBuf++;

#if PMA_ACCESS > 1U
    pdwVal++;
#endif
  }

  if ((wNBytes % 2U) != 0U)
  {
    temp = *pdwVal;
    *pBuf = (uint8_t)((temp >> 0) & 0xFFU);
  }
}
#endif /* defined (USB) */

/**
  * @}
  */

/**
  * @}
  */
#endif /* defined (USB) || defined (USB_OTG_FS) */
#endif /* defined (HAL_PCD_MODULE_ENABLED) || defined (HAL_HCD_MODULE_ENABLED) */

/**
  * @}
  */

/************************ (C) COPYRIGHT STMicroelectronics *****END OF FILE****/
