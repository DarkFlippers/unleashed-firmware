/* USER CODE BEGIN Header */
/**
  ******************************************************************************
  * @file           : usbd_conf.c
  * @version        : v2.0_Cube
  * @brief          : This file implements the board support package for the USB device library
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

/* Includes ------------------------------------------------------------------*/
#include "stm32l4xx.h"
#include "stm32l4xx_hal.h"
#include "usbd_def.h"
#include "usbd_core.h"
#include "usbd_cdc.h"

/* USER CODE BEGIN Includes */

/* USER CODE END Includes */

/* Private typedef -----------------------------------------------------------*/
/* Private define ------------------------------------------------------------*/
/* Private macro -------------------------------------------------------------*/

/* USER CODE BEGIN PV */
/* Private variables ---------------------------------------------------------*/

/* USER CODE END PV */

PCD_HandleTypeDef hpcd_USB_OTG_FS;
void Error_Handler(void);

/* USER CODE BEGIN 0 */

/* USER CODE END 0 */

/* Exported function prototypes ----------------------------------------------*/
extern USBD_StatusTypeDef USBD_LL_BatteryCharging(USBD_HandleTypeDef* pdev);

/* USER CODE BEGIN PFP */
/* Private function prototypes -----------------------------------------------*/

/* USER CODE END PFP */

/* Private functions ---------------------------------------------------------*/

/* USER CODE BEGIN 1 */
static void SystemClockConfig_Resume(void);

/* USER CODE END 1 */
extern void SystemClock_Config(void);

/*******************************************************************************
                       LL Driver Callbacks (PCD -> USB Device Library)
*******************************************************************************/
/* MSP Init */

void HAL_PCD_MspInit(PCD_HandleTypeDef* pcdHandle) {
    GPIO_InitTypeDef GPIO_InitStruct = {0};
    if(pcdHandle->Instance == USB_OTG_FS) {
        /* USER CODE BEGIN USB_OTG_FS_MspInit 0 */

        /* USER CODE END USB_OTG_FS_MspInit 0 */

        __HAL_RCC_GPIOA_CLK_ENABLE();
        /**USB_OTG_FS GPIO Configuration    
    PA11     ------> USB_OTG_FS_DM
    PA12     ------> USB_OTG_FS_DP 
    */
        GPIO_InitStruct.Pin = GPIO_PIN_11 | GPIO_PIN_12;
        GPIO_InitStruct.Mode = GPIO_MODE_AF_PP;
        GPIO_InitStruct.Pull = GPIO_NOPULL;
        GPIO_InitStruct.Speed = GPIO_SPEED_FREQ_VERY_HIGH;
        GPIO_InitStruct.Alternate = GPIO_AF10_OTG_FS;
        HAL_GPIO_Init(GPIOA, &GPIO_InitStruct);

        /* Peripheral clock enable */
        __HAL_RCC_USB_OTG_FS_CLK_ENABLE();

        /* Enable VDDUSB */
        if(__HAL_RCC_PWR_IS_CLK_DISABLED()) {
            __HAL_RCC_PWR_CLK_ENABLE();
            HAL_PWREx_EnableVddUSB();
            __HAL_RCC_PWR_CLK_DISABLE();
        } else {
            HAL_PWREx_EnableVddUSB();
        }

        /* Peripheral interrupt init */
        HAL_NVIC_SetPriority(OTG_FS_IRQn, 5, 0);
        HAL_NVIC_EnableIRQ(OTG_FS_IRQn);
        /* USER CODE BEGIN USB_OTG_FS_MspInit 1 */

        /* USER CODE END USB_OTG_FS_MspInit 1 */
    }
}

void HAL_PCD_MspDeInit(PCD_HandleTypeDef* pcdHandle) {
    if(pcdHandle->Instance == USB_OTG_FS) {
        /* USER CODE BEGIN USB_OTG_FS_MspDeInit 0 */

        /* USER CODE END USB_OTG_FS_MspDeInit 0 */
        /* Peripheral clock disable */
        __HAL_RCC_USB_OTG_FS_CLK_DISABLE();

        /**USB_OTG_FS GPIO Configuration    
    PA11     ------> USB_OTG_FS_DM
    PA12     ------> USB_OTG_FS_DP 
    */
        HAL_GPIO_DeInit(GPIOA, GPIO_PIN_11 | GPIO_PIN_12);

        /* Disable VDDUSB */
        if(__HAL_RCC_PWR_IS_CLK_DISABLED()) {
            __HAL_RCC_PWR_CLK_ENABLE();
            HAL_PWREx_DisableVddUSB();
            __HAL_RCC_PWR_CLK_DISABLE();
        } else {
            HAL_PWREx_DisableVddUSB();
        }

        /* Peripheral interrupt Deinit*/
        HAL_NVIC_DisableIRQ(OTG_FS_IRQn);

        /* USER CODE BEGIN USB_OTG_FS_MspDeInit 1 */

        /* USER CODE END USB_OTG_FS_MspDeInit 1 */
    }
}

/**
  * @brief  Setup stage callback
  * @param  hpcd: PCD handle
  * @retval None
  */
#if(USE_HAL_PCD_REGISTER_CALLBACKS == 1U)
static void PCD_SetupStageCallback(PCD_HandleTypeDef* hpcd)
#else
void HAL_PCD_SetupStageCallback(PCD_HandleTypeDef* hpcd)
#endif /* USE_HAL_PCD_REGISTER_CALLBACKS */
{
    USBD_LL_SetupStage((USBD_HandleTypeDef*)hpcd->pData, (uint8_t*)hpcd->Setup);
}

/**
  * @brief  Data Out stage callback.
  * @param  hpcd: PCD handle
  * @param  epnum: Endpoint number
  * @retval None
  */
#if(USE_HAL_PCD_REGISTER_CALLBACKS == 1U)
static void PCD_DataOutStageCallback(PCD_HandleTypeDef* hpcd, uint8_t epnum)
#else
void HAL_PCD_DataOutStageCallback(PCD_HandleTypeDef* hpcd, uint8_t epnum)
#endif /* USE_HAL_PCD_REGISTER_CALLBACKS */
{
    USBD_LL_DataOutStage((USBD_HandleTypeDef*)hpcd->pData, epnum, hpcd->OUT_ep[epnum].xfer_buff);
}

/**
  * @brief  Data In stage callback.
  * @param  hpcd: PCD handle
  * @param  epnum: Endpoint number
  * @retval None
  */
#if(USE_HAL_PCD_REGISTER_CALLBACKS == 1U)
static void PCD_DataInStageCallback(PCD_HandleTypeDef* hpcd, uint8_t epnum)
#else
void HAL_PCD_DataInStageCallback(PCD_HandleTypeDef* hpcd, uint8_t epnum)
#endif /* USE_HAL_PCD_REGISTER_CALLBACKS */
{
    USBD_LL_DataInStage((USBD_HandleTypeDef*)hpcd->pData, epnum, hpcd->IN_ep[epnum].xfer_buff);
}

/**
  * @brief  SOF callback.
  * @param  hpcd: PCD handle
  * @retval None
  */
#if(USE_HAL_PCD_REGISTER_CALLBACKS == 1U)
static void PCD_SOFCallback(PCD_HandleTypeDef* hpcd)
#else
void HAL_PCD_SOFCallback(PCD_HandleTypeDef* hpcd)
#endif /* USE_HAL_PCD_REGISTER_CALLBACKS */
{
    USBD_LL_SOF((USBD_HandleTypeDef*)hpcd->pData);
}

/**
  * @brief  Reset callback.
  * @param  hpcd: PCD handle
  * @retval None
  */
#if(USE_HAL_PCD_REGISTER_CALLBACKS == 1U)
static void PCD_ResetCallback(PCD_HandleTypeDef* hpcd)
#else
void HAL_PCD_ResetCallback(PCD_HandleTypeDef* hpcd)
#endif /* USE_HAL_PCD_REGISTER_CALLBACKS */
{
    USBD_SpeedTypeDef speed = USBD_SPEED_FULL;

    if(hpcd->Init.speed != PCD_SPEED_FULL) {
        Error_Handler();
    }
    /* Set Speed. */
    USBD_LL_SetSpeed((USBD_HandleTypeDef*)hpcd->pData, speed);

    /* Reset Device. */
    USBD_LL_Reset((USBD_HandleTypeDef*)hpcd->pData);
}

/**
  * @brief  Suspend callback.
  * When Low power mode is enabled the debug cannot be used (IAR, Keil doesn't support it)
  * @param  hpcd: PCD handle
  * @retval None
  */
#if(USE_HAL_PCD_REGISTER_CALLBACKS == 1U)
static void PCD_SuspendCallback(PCD_HandleTypeDef* hpcd)
#else
void HAL_PCD_SuspendCallback(PCD_HandleTypeDef* hpcd)
#endif /* USE_HAL_PCD_REGISTER_CALLBACKS */
{
    __HAL_PCD_GATE_PHYCLOCK(hpcd);
    /* Inform USB library that core enters in suspend Mode. */
    USBD_LL_Suspend((USBD_HandleTypeDef*)hpcd->pData);
    /* Enter in STOP mode. */
    /* USER CODE BEGIN 2 */
    if(hpcd->Init.low_power_enable) {
        /* Set SLEEPDEEP bit and SleepOnExit of Cortex System Control Register. */
        SCB->SCR |= (uint32_t)((uint32_t)(SCB_SCR_SLEEPDEEP_Msk | SCB_SCR_SLEEPONEXIT_Msk));
    }
    /* USER CODE END 2 */
}

/**
  * @brief  Resume callback.
  * When Low power mode is enabled the debug cannot be used (IAR, Keil doesn't support it)
  * @param  hpcd: PCD handle
  * @retval None
  */
#if(USE_HAL_PCD_REGISTER_CALLBACKS == 1U)
static void PCD_ResumeCallback(PCD_HandleTypeDef* hpcd)
#else
void HAL_PCD_ResumeCallback(PCD_HandleTypeDef* hpcd)
#endif /* USE_HAL_PCD_REGISTER_CALLBACKS */
{
    __HAL_PCD_UNGATE_PHYCLOCK(hpcd);

    /* USER CODE BEGIN 3 */
    if(hpcd->Init.low_power_enable) {
        /* Reset SLEEPDEEP bit of Cortex System Control Register. */
        SCB->SCR &= (uint32_t) ~((uint32_t)(SCB_SCR_SLEEPDEEP_Msk | SCB_SCR_SLEEPONEXIT_Msk));
        SystemClockConfig_Resume();
    }
    /* USER CODE END 3 */
    USBD_LL_Resume((USBD_HandleTypeDef*)hpcd->pData);
}

/**
  * @brief  ISOOUTIncomplete callback.
  * @param  hpcd: PCD handle
  * @param  epnum: Endpoint number
  * @retval None
  */
#if(USE_HAL_PCD_REGISTER_CALLBACKS == 1U)
static void PCD_ISOOUTIncompleteCallback(PCD_HandleTypeDef* hpcd, uint8_t epnum)
#else
void HAL_PCD_ISOOUTIncompleteCallback(PCD_HandleTypeDef* hpcd, uint8_t epnum)
#endif /* USE_HAL_PCD_REGISTER_CALLBACKS */
{
    USBD_LL_IsoOUTIncomplete((USBD_HandleTypeDef*)hpcd->pData, epnum);
}

/**
  * @brief  ISOINIncomplete callback.
  * @param  hpcd: PCD handle
  * @param  epnum: Endpoint number
  * @retval None
  */
#if(USE_HAL_PCD_REGISTER_CALLBACKS == 1U)
static void PCD_ISOINIncompleteCallback(PCD_HandleTypeDef* hpcd, uint8_t epnum)
#else
void HAL_PCD_ISOINIncompleteCallback(PCD_HandleTypeDef* hpcd, uint8_t epnum)
#endif /* USE_HAL_PCD_REGISTER_CALLBACKS */
{
    USBD_LL_IsoINIncomplete((USBD_HandleTypeDef*)hpcd->pData, epnum);
}

/**
  * @brief  Connect callback.
  * @param  hpcd: PCD handle
  * @retval None
  */
#if(USE_HAL_PCD_REGISTER_CALLBACKS == 1U)
static void PCD_ConnectCallback(PCD_HandleTypeDef* hpcd)
#else
void HAL_PCD_ConnectCallback(PCD_HandleTypeDef* hpcd)
#endif /* USE_HAL_PCD_REGISTER_CALLBACKS */
{
    // TODO send UsbConnect event to FURI

    USBD_LL_DevConnected((USBD_HandleTypeDef*)hpcd->pData);
}

/**
  * @brief  Disconnect callback.
  * @param  hpcd: PCD handle
  * @retval None
  */
#if(USE_HAL_PCD_REGISTER_CALLBACKS == 1U)
static void PCD_DisconnectCallback(PCD_HandleTypeDef* hpcd)
#else
void HAL_PCD_DisconnectCallback(PCD_HandleTypeDef* hpcd)
#endif /* USE_HAL_PCD_REGISTER_CALLBACKS */
{
    // TODO send UsbDisconnect event to FURI

    USBD_LL_DevDisconnected((USBD_HandleTypeDef*)hpcd->pData);
}

/*******************************************************************************
                       LL Driver Interface (USB Device Library --> PCD)
*******************************************************************************/

/**
  * @brief  Initializes the low level portion of the device driver.
  * @param  pdev: Device handle
  * @retval USBD status
  */
USBD_StatusTypeDef USBD_LL_Init(USBD_HandleTypeDef* pdev) {
    /* Init USB Ip. */
    if(pdev->id == DEVICE_FS) {
        /* Enable USB power on Pwrctrl CR2 register. */
        /* Link the driver to the stack. */
        hpcd_USB_OTG_FS.pData = pdev;
        pdev->pData = &hpcd_USB_OTG_FS;

        hpcd_USB_OTG_FS.Instance = USB_OTG_FS;
        hpcd_USB_OTG_FS.Init.dev_endpoints = 6;
        hpcd_USB_OTG_FS.Init.speed = PCD_SPEED_FULL;
        hpcd_USB_OTG_FS.Init.phy_itface = PCD_PHY_EMBEDDED;
        hpcd_USB_OTG_FS.Init.Sof_enable = DISABLE;
        hpcd_USB_OTG_FS.Init.low_power_enable = DISABLE;
        hpcd_USB_OTG_FS.Init.lpm_enable = DISABLE;
        hpcd_USB_OTG_FS.Init.battery_charging_enable = DISABLE;
        hpcd_USB_OTG_FS.Init.use_dedicated_ep1 = DISABLE;
        hpcd_USB_OTG_FS.Init.vbus_sensing_enable = DISABLE;
        if(HAL_PCD_Init(&hpcd_USB_OTG_FS) != HAL_OK) {
            Error_Handler();
        }

#if(USE_HAL_PCD_REGISTER_CALLBACKS == 1U)
        /* Register USB PCD CallBacks */
        HAL_PCD_RegisterCallback(&hpcd_USB_OTG_FS, HAL_PCD_SOF_CB_ID, PCD_SOFCallback);
        HAL_PCD_RegisterCallback(
            &hpcd_USB_OTG_FS, HAL_PCD_SETUPSTAGE_CB_ID, PCD_SetupStageCallback);
        HAL_PCD_RegisterCallback(&hpcd_USB_OTG_FS, HAL_PCD_RESET_CB_ID, PCD_ResetCallback);
        HAL_PCD_RegisterCallback(&hpcd_USB_OTG_FS, HAL_PCD_SUSPEND_CB_ID, PCD_SuspendCallback);
        HAL_PCD_RegisterCallback(&hpcd_USB_OTG_FS, HAL_PCD_RESUME_CB_ID, PCD_ResumeCallback);
        HAL_PCD_RegisterCallback(&hpcd_USB_OTG_FS, HAL_PCD_CONNECT_CB_ID, PCD_ConnectCallback);
        HAL_PCD_RegisterCallback(
            &hpcd_USB_OTG_FS, HAL_PCD_DISCONNECT_CB_ID, PCD_DisconnectCallback);

        HAL_PCD_RegisterDataOutStageCallback(&hpcd_USB_OTG_FS, PCD_DataOutStageCallback);
        HAL_PCD_RegisterDataInStageCallback(&hpcd_USB_OTG_FS, PCD_DataInStageCallback);
        HAL_PCD_RegisterIsoOutIncpltCallback(&hpcd_USB_OTG_FS, PCD_ISOOUTIncompleteCallback);
        HAL_PCD_RegisterIsoInIncpltCallback(&hpcd_USB_OTG_FS, PCD_ISOINIncompleteCallback);
#endif /* USE_HAL_PCD_REGISTER_CALLBACKS */
        HAL_PCDEx_SetRxFiFo(&hpcd_USB_OTG_FS, 0x80);
        HAL_PCDEx_SetTxFiFo(&hpcd_USB_OTG_FS, 0, 0x40);
        HAL_PCDEx_SetTxFiFo(&hpcd_USB_OTG_FS, 1, 0x80);
    }
    return USBD_OK;
}

/**
  * @brief  De-Initializes the low level portion of the device driver.
  * @param  pdev: Device handle
  * @retval USBD status
  */
USBD_StatusTypeDef USBD_LL_DeInit(USBD_HandleTypeDef* pdev) {
    HAL_StatusTypeDef hal_status = HAL_OK;
    USBD_StatusTypeDef usb_status = USBD_OK;

    hal_status = HAL_PCD_DeInit(pdev->pData);

    switch(hal_status) {
    case HAL_OK:
        usb_status = USBD_OK;
        break;
    case HAL_ERROR:
        usb_status = USBD_FAIL;
        break;
    case HAL_BUSY:
        usb_status = USBD_BUSY;
        break;
    case HAL_TIMEOUT:
        usb_status = USBD_FAIL;
        break;
    default:
        usb_status = USBD_FAIL;
        break;
    }
    return usb_status;
}

/**
  * @brief  Starts the low level portion of the device driver.
  * @param  pdev: Device handle
  * @retval USBD status
  */
USBD_StatusTypeDef USBD_LL_Start(USBD_HandleTypeDef* pdev) {
    HAL_StatusTypeDef hal_status = HAL_OK;
    USBD_StatusTypeDef usb_status = USBD_OK;

    hal_status = HAL_PCD_Start(pdev->pData);

    switch(hal_status) {
    case HAL_OK:
        usb_status = USBD_OK;
        break;
    case HAL_ERROR:
        usb_status = USBD_FAIL;
        break;
    case HAL_BUSY:
        usb_status = USBD_BUSY;
        break;
    case HAL_TIMEOUT:
        usb_status = USBD_FAIL;
        break;
    default:
        usb_status = USBD_FAIL;
        break;
    }
    return usb_status;
}

/**
  * @brief  Stops the low level portion of the device driver.
  * @param  pdev: Device handle
  * @retval USBD status
  */
USBD_StatusTypeDef USBD_LL_Stop(USBD_HandleTypeDef* pdev) {
    HAL_StatusTypeDef hal_status = HAL_OK;
    USBD_StatusTypeDef usb_status = USBD_OK;

    hal_status = HAL_PCD_Stop(pdev->pData);

    switch(hal_status) {
    case HAL_OK:
        usb_status = USBD_OK;
        break;
    case HAL_ERROR:
        usb_status = USBD_FAIL;
        break;
    case HAL_BUSY:
        usb_status = USBD_BUSY;
        break;
    case HAL_TIMEOUT:
        usb_status = USBD_FAIL;
        break;
    default:
        usb_status = USBD_FAIL;
        break;
    }
    return usb_status;
}

/**
  * @brief  Opens an endpoint of the low level driver.
  * @param  pdev: Device handle
  * @param  ep_addr: Endpoint number
  * @param  ep_type: Endpoint type
  * @param  ep_mps: Endpoint max packet size
  * @retval USBD status
  */
USBD_StatusTypeDef
USBD_LL_OpenEP(USBD_HandleTypeDef* pdev, uint8_t ep_addr, uint8_t ep_type, uint16_t ep_mps) {
    HAL_StatusTypeDef hal_status = HAL_OK;
    USBD_StatusTypeDef usb_status = USBD_OK;

    hal_status = HAL_PCD_EP_Open(pdev->pData, ep_addr, ep_mps, ep_type);

    switch(hal_status) {
    case HAL_OK:
        usb_status = USBD_OK;
        break;
    case HAL_ERROR:
        usb_status = USBD_FAIL;
        break;
    case HAL_BUSY:
        usb_status = USBD_BUSY;
        break;
    case HAL_TIMEOUT:
        usb_status = USBD_FAIL;
        break;
    default:
        usb_status = USBD_FAIL;
        break;
    }
    return usb_status;
}

/**
  * @brief  Closes an endpoint of the low level driver.
  * @param  pdev: Device handle
  * @param  ep_addr: Endpoint number
  * @retval USBD status
  */
USBD_StatusTypeDef USBD_LL_CloseEP(USBD_HandleTypeDef* pdev, uint8_t ep_addr) {
    HAL_StatusTypeDef hal_status = HAL_OK;
    USBD_StatusTypeDef usb_status = USBD_OK;

    hal_status = HAL_PCD_EP_Close(pdev->pData, ep_addr);

    switch(hal_status) {
    case HAL_OK:
        usb_status = USBD_OK;
        break;
    case HAL_ERROR:
        usb_status = USBD_FAIL;
        break;
    case HAL_BUSY:
        usb_status = USBD_BUSY;
        break;
    case HAL_TIMEOUT:
        usb_status = USBD_FAIL;
        break;
    default:
        usb_status = USBD_FAIL;
        break;
    }
    return usb_status;
}

/**
  * @brief  Flushes an endpoint of the Low Level Driver.
  * @param  pdev: Device handle
  * @param  ep_addr: Endpoint number
  * @retval USBD status
  */
USBD_StatusTypeDef USBD_LL_FlushEP(USBD_HandleTypeDef* pdev, uint8_t ep_addr) {
    HAL_StatusTypeDef hal_status = HAL_OK;
    USBD_StatusTypeDef usb_status = USBD_OK;

    hal_status = HAL_PCD_EP_Flush(pdev->pData, ep_addr);

    switch(hal_status) {
    case HAL_OK:
        usb_status = USBD_OK;
        break;
    case HAL_ERROR:
        usb_status = USBD_FAIL;
        break;
    case HAL_BUSY:
        usb_status = USBD_BUSY;
        break;
    case HAL_TIMEOUT:
        usb_status = USBD_FAIL;
        break;
    default:
        usb_status = USBD_FAIL;
        break;
    }
    return usb_status;
}

/**
  * @brief  Sets a Stall condition on an endpoint of the Low Level Driver.
  * @param  pdev: Device handle
  * @param  ep_addr: Endpoint number
  * @retval USBD status
  */
USBD_StatusTypeDef USBD_LL_StallEP(USBD_HandleTypeDef* pdev, uint8_t ep_addr) {
    HAL_StatusTypeDef hal_status = HAL_OK;
    USBD_StatusTypeDef usb_status = USBD_OK;

    hal_status = HAL_PCD_EP_SetStall(pdev->pData, ep_addr);

    switch(hal_status) {
    case HAL_OK:
        usb_status = USBD_OK;
        break;
    case HAL_ERROR:
        usb_status = USBD_FAIL;
        break;
    case HAL_BUSY:
        usb_status = USBD_BUSY;
        break;
    case HAL_TIMEOUT:
        usb_status = USBD_FAIL;
        break;
    default:
        usb_status = USBD_FAIL;
        break;
    }
    return usb_status;
}

/**
  * @brief  Clears a Stall condition on an endpoint of the Low Level Driver.
  * @param  pdev: Device handle
  * @param  ep_addr: Endpoint number
  * @retval USBD status
  */
USBD_StatusTypeDef USBD_LL_ClearStallEP(USBD_HandleTypeDef* pdev, uint8_t ep_addr) {
    HAL_StatusTypeDef hal_status = HAL_OK;
    USBD_StatusTypeDef usb_status = USBD_OK;

    hal_status = HAL_PCD_EP_ClrStall(pdev->pData, ep_addr);

    switch(hal_status) {
    case HAL_OK:
        usb_status = USBD_OK;
        break;
    case HAL_ERROR:
        usb_status = USBD_FAIL;
        break;
    case HAL_BUSY:
        usb_status = USBD_BUSY;
        break;
    case HAL_TIMEOUT:
        usb_status = USBD_FAIL;
        break;
    default:
        usb_status = USBD_FAIL;
        break;
    }
    return usb_status;
}

/**
  * @brief  Returns Stall condition.
  * @param  pdev: Device handle
  * @param  ep_addr: Endpoint number
  * @retval Stall (1: Yes, 0: No)
  */
uint8_t USBD_LL_IsStallEP(USBD_HandleTypeDef* pdev, uint8_t ep_addr) {
    PCD_HandleTypeDef* hpcd = (PCD_HandleTypeDef*)pdev->pData;

    if((ep_addr & 0x80) == 0x80) {
        return hpcd->IN_ep[ep_addr & 0x7F].is_stall;
    } else {
        return hpcd->OUT_ep[ep_addr & 0x7F].is_stall;
    }
}

/**
  * @brief  Assigns a USB address to the device.
  * @param  pdev: Device handle
  * @param  dev_addr: Device address
  * @retval USBD status
  */
USBD_StatusTypeDef USBD_LL_SetUSBAddress(USBD_HandleTypeDef* pdev, uint8_t dev_addr) {
    HAL_StatusTypeDef hal_status = HAL_OK;
    USBD_StatusTypeDef usb_status = USBD_OK;

    hal_status = HAL_PCD_SetAddress(pdev->pData, dev_addr);

    switch(hal_status) {
    case HAL_OK:
        usb_status = USBD_OK;
        break;
    case HAL_ERROR:
        usb_status = USBD_FAIL;
        break;
    case HAL_BUSY:
        usb_status = USBD_BUSY;
        break;
    case HAL_TIMEOUT:
        usb_status = USBD_FAIL;
        break;
    default:
        usb_status = USBD_FAIL;
        break;
    }
    return usb_status;
}

/**
  * @brief  Transmits data over an endpoint.
  * @param  pdev: Device handle
  * @param  ep_addr: Endpoint number
  * @param  pbuf: Pointer to data to be sent
  * @param  size: Data size    
  * @retval USBD status
  */
USBD_StatusTypeDef
USBD_LL_Transmit(USBD_HandleTypeDef* pdev, uint8_t ep_addr, uint8_t* pbuf, uint16_t size) {
    HAL_StatusTypeDef hal_status = HAL_OK;
    USBD_StatusTypeDef usb_status = USBD_OK;

    hal_status = HAL_PCD_EP_Transmit(pdev->pData, ep_addr, pbuf, size);

    switch(hal_status) {
    case HAL_OK:
        usb_status = USBD_OK;
        break;
    case HAL_ERROR:
        usb_status = USBD_FAIL;
        break;
    case HAL_BUSY:
        usb_status = USBD_BUSY;
        break;
    case HAL_TIMEOUT:
        usb_status = USBD_FAIL;
        break;
    default:
        usb_status = USBD_FAIL;
        break;
    }
    return usb_status;
}

/**
  * @brief  Prepares an endpoint for reception.
  * @param  pdev: Device handle
  * @param  ep_addr: Endpoint number
  * @param  pbuf: Pointer to data to be received
  * @param  size: Data size
  * @retval USBD status
  */
USBD_StatusTypeDef
USBD_LL_PrepareReceive(USBD_HandleTypeDef* pdev, uint8_t ep_addr, uint8_t* pbuf, uint16_t size) {
    HAL_StatusTypeDef hal_status = HAL_OK;
    USBD_StatusTypeDef usb_status = USBD_OK;

    hal_status = HAL_PCD_EP_Receive(pdev->pData, ep_addr, pbuf, size);

    switch(hal_status) {
    case HAL_OK:
        usb_status = USBD_OK;
        break;
    case HAL_ERROR:
        usb_status = USBD_FAIL;
        break;
    case HAL_BUSY:
        usb_status = USBD_BUSY;
        break;
    case HAL_TIMEOUT:
        usb_status = USBD_FAIL;
        break;
    default:
        usb_status = USBD_FAIL;
        break;
    }
    return usb_status;
}

/**
  * @brief  Returns the last transfered packet size.
  * @param  pdev: Device handle
  * @param  ep_addr: Endpoint number
  * @retval Recived Data Size
  */
uint32_t USBD_LL_GetRxDataSize(USBD_HandleTypeDef* pdev, uint8_t ep_addr) {
    return HAL_PCD_EP_GetRxCount((PCD_HandleTypeDef*)pdev->pData, ep_addr);
}

/**
  * @brief  Send LPM message to user layer
  * @param  hpcd: PCD handle
  * @param  msg: LPM message
  * @retval None
  */
void HAL_PCDEx_LPM_Callback(PCD_HandleTypeDef* hpcd, PCD_LPM_MsgTypeDef msg) {
    switch(msg) {
    case PCD_LPM_L0_ACTIVE:
        if(hpcd->Init.low_power_enable) {
            SystemClockConfig_Resume();

            /* Reset SLEEPDEEP bit of Cortex System Control Register. */
            SCB->SCR &= (uint32_t) ~((uint32_t)(SCB_SCR_SLEEPDEEP_Msk | SCB_SCR_SLEEPONEXIT_Msk));
        }
        __HAL_PCD_UNGATE_PHYCLOCK(hpcd);
        USBD_LL_Resume(hpcd->pData);
        break;

    case PCD_LPM_L1_ACTIVE:
        __HAL_PCD_GATE_PHYCLOCK(hpcd);
        USBD_LL_Suspend(hpcd->pData);

        /* Enter in STOP mode. */
        if(hpcd->Init.low_power_enable) {
            /* Set SLEEPDEEP bit and SleepOnExit of Cortex System Control Register. */
            SCB->SCR |= (uint32_t)((uint32_t)(SCB_SCR_SLEEPDEEP_Msk | SCB_SCR_SLEEPONEXIT_Msk));
        }
        break;
    }
}

/**
  * @brief  Delays routine for the USB Device Library.
  * @param  Delay: Delay in ms
  * @retval None
  */
void USBD_LL_Delay(uint32_t Delay) {
    HAL_Delay(Delay);
}

/**
  * @brief  Static single allocation.
  * @param  size: Size of allocated memory
  * @retval None
  */
void* USBD_static_malloc(uint32_t size) {
    static uint32_t mem[(sizeof(USBD_CDC_HandleTypeDef) / 4) + 1]; /* On 32-bit boundary */
    return mem;
}

/**
  * @brief  Dummy memory free
  * @param  p: Pointer to allocated  memory address
  * @retval None
  */
void USBD_static_free(void* p) {
}

/* USER CODE BEGIN 5 */
/**
  * @brief  Configures system clock after wake-up from USB resume callBack:
  *         enable HSI, PLL and select PLL as system clock source.
  * @retval None
  */
static void SystemClockConfig_Resume(void) {
    SystemClock_Config();
}
/* USER CODE END 5 */

/************************ (C) COPYRIGHT STMicroelectronics *****END OF FILE****/
