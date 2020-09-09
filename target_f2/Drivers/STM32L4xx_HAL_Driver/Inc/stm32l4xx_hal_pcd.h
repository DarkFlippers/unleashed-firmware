/**
  ******************************************************************************
  * @file    stm32l4xx_hal_pcd.h
  * @author  MCD Application Team
  * @brief   Header file of PCD HAL module.
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

/* Define to prevent recursive inclusion -------------------------------------*/
#ifndef STM32L4xx_HAL_PCD_H
#define STM32L4xx_HAL_PCD_H

#ifdef __cplusplus
extern "C" {
#endif

/* Includes ------------------------------------------------------------------*/
#include "stm32l4xx_ll_usb.h"

#if defined (USB) || defined (USB_OTG_FS)

/** @addtogroup STM32L4xx_HAL_Driver
  * @{
  */

/** @addtogroup PCD
  * @{
  */

/* Exported types ------------------------------------------------------------*/
/** @defgroup PCD_Exported_Types PCD Exported Types
  * @{
  */

/**
  * @brief  PCD State structure definition
  */
typedef enum
{
  HAL_PCD_STATE_RESET   = 0x00,
  HAL_PCD_STATE_READY   = 0x01,
  HAL_PCD_STATE_ERROR   = 0x02,
  HAL_PCD_STATE_BUSY    = 0x03,
  HAL_PCD_STATE_TIMEOUT = 0x04
} PCD_StateTypeDef;

/* Device LPM suspend state */
typedef enum
{
  LPM_L0 = 0x00, /* on */
  LPM_L1 = 0x01, /* LPM L1 sleep */
  LPM_L2 = 0x02, /* suspend */
  LPM_L3 = 0x03, /* off */
} PCD_LPM_StateTypeDef;

typedef enum
{
  PCD_LPM_L0_ACTIVE = 0x00, /* on */
  PCD_LPM_L1_ACTIVE = 0x01, /* LPM L1 sleep */
} PCD_LPM_MsgTypeDef;

typedef enum
{
  PCD_BCD_ERROR                     = 0xFF,
  PCD_BCD_CONTACT_DETECTION         = 0xFE,
  PCD_BCD_STD_DOWNSTREAM_PORT       = 0xFD,
  PCD_BCD_CHARGING_DOWNSTREAM_PORT  = 0xFC,
  PCD_BCD_DEDICATED_CHARGING_PORT   = 0xFB,
  PCD_BCD_DISCOVERY_COMPLETED       = 0x00,

} PCD_BCD_MsgTypeDef;

#if defined (USB)

#endif /* defined (USB) */
#if defined (USB_OTG_FS)
typedef USB_OTG_GlobalTypeDef  PCD_TypeDef;
typedef USB_OTG_CfgTypeDef     PCD_InitTypeDef;
typedef USB_OTG_EPTypeDef      PCD_EPTypeDef;
#endif /* defined (USB_OTG_FS) */
#if defined (USB)
typedef USB_TypeDef        PCD_TypeDef;
typedef USB_CfgTypeDef     PCD_InitTypeDef;
typedef USB_EPTypeDef      PCD_EPTypeDef;
#endif /* defined (USB) */

/**
  * @brief  PCD Handle Structure definition
  */
#if (USE_HAL_PCD_REGISTER_CALLBACKS == 1U)
typedef struct __PCD_HandleTypeDef
#else
typedef struct
#endif /* USE_HAL_PCD_REGISTER_CALLBACKS */
{
  PCD_TypeDef             *Instance;   /*!< Register base address              */
  PCD_InitTypeDef         Init;        /*!< PCD required parameters            */
  __IO uint8_t            USB_Address; /*!< USB Address                        */
#if defined (USB_OTG_FS)
  PCD_EPTypeDef           IN_ep[16];   /*!< IN endpoint parameters             */
  PCD_EPTypeDef           OUT_ep[16];  /*!< OUT endpoint parameters            */
#endif /* defined (USB_OTG_FS) */
#if defined (USB)
  PCD_EPTypeDef           IN_ep[8];   /*!< IN endpoint parameters             */
  PCD_EPTypeDef           OUT_ep[8];  /*!< OUT endpoint parameters            */
#endif /* defined (USB) */
  HAL_LockTypeDef         Lock;        /*!< PCD peripheral status              */
  __IO PCD_StateTypeDef   State;       /*!< PCD communication state            */
  __IO  uint32_t          ErrorCode;   /*!< PCD Error code                     */
  uint32_t                Setup[12];   /*!< Setup packet buffer                */
  PCD_LPM_StateTypeDef    LPM_State;   /*!< LPM State                          */
  uint32_t                BESL;


  uint32_t lpm_active;                 /*!< Enable or disable the Link Power Management .
                                       This parameter can be set to ENABLE or DISABLE        */

  uint32_t battery_charging_active;    /*!< Enable or disable Battery charging.
                                       This parameter can be set to ENABLE or DISABLE        */
  void                    *pData;      /*!< Pointer to upper stack Handler */

#if (USE_HAL_PCD_REGISTER_CALLBACKS == 1U)
  void (* SOFCallback)(struct __PCD_HandleTypeDef *hpcd);                              /*!< USB OTG PCD SOF callback                */
  void (* SetupStageCallback)(struct __PCD_HandleTypeDef *hpcd);                       /*!< USB OTG PCD Setup Stage callback        */
  void (* ResetCallback)(struct __PCD_HandleTypeDef *hpcd);                            /*!< USB OTG PCD Reset callback              */
  void (* SuspendCallback)(struct __PCD_HandleTypeDef *hpcd);                          /*!< USB OTG PCD Suspend callback            */
  void (* ResumeCallback)(struct __PCD_HandleTypeDef *hpcd);                           /*!< USB OTG PCD Resume callback             */
  void (* ConnectCallback)(struct __PCD_HandleTypeDef *hpcd);                          /*!< USB OTG PCD Connect callback            */
  void (* DisconnectCallback)(struct __PCD_HandleTypeDef *hpcd);                       /*!< USB OTG PCD Disconnect callback         */

  void (* DataOutStageCallback)(struct __PCD_HandleTypeDef *hpcd, uint8_t epnum);      /*!< USB OTG PCD Data OUT Stage callback     */
  void (* DataInStageCallback)(struct __PCD_HandleTypeDef *hpcd, uint8_t epnum);       /*!< USB OTG PCD Data IN Stage callback      */
  void (* ISOOUTIncompleteCallback)(struct __PCD_HandleTypeDef *hpcd, uint8_t epnum);  /*!< USB OTG PCD ISO OUT Incomplete callback */
  void (* ISOINIncompleteCallback)(struct __PCD_HandleTypeDef *hpcd, uint8_t epnum);   /*!< USB OTG PCD ISO IN Incomplete callback  */
  void (* BCDCallback)(struct __PCD_HandleTypeDef *hpcd, PCD_BCD_MsgTypeDef msg);      /*!< USB OTG PCD BCD callback                */
  void (* LPMCallback)(struct __PCD_HandleTypeDef *hpcd, PCD_LPM_MsgTypeDef msg);      /*!< USB OTG PCD LPM callback                */

  void (* MspInitCallback)(struct __PCD_HandleTypeDef *hpcd);                          /*!< USB OTG PCD Msp Init callback           */
  void (* MspDeInitCallback)(struct __PCD_HandleTypeDef *hpcd);                        /*!< USB OTG PCD Msp DeInit callback         */
#endif /* USE_HAL_PCD_REGISTER_CALLBACKS */
} PCD_HandleTypeDef;

/**
  * @}
  */

/* Include PCD HAL Extended module */
#include "stm32l4xx_hal_pcd_ex.h"

/* Exported constants --------------------------------------------------------*/
/** @defgroup PCD_Exported_Constants PCD Exported Constants
  * @{
  */

/** @defgroup PCD_Speed PCD Speed
  * @{
  */
#define PCD_SPEED_FULL               USBD_FS_SPEED
/**
  * @}
  */

/** @defgroup PCD_PHY_Module PCD PHY Module
  * @{
  */
#define PCD_PHY_ULPI                 1U
#define PCD_PHY_EMBEDDED             2U
#define PCD_PHY_UTMI                 3U
/**
  * @}
  */

/** @defgroup PCD_Error_Code_definition PCD Error Code definition
  * @brief  PCD Error Code definition
  * @{
  */
#if (USE_HAL_PCD_REGISTER_CALLBACKS == 1U)
#define  HAL_PCD_ERROR_INVALID_CALLBACK                        (0x00000010U)    /*!< Invalid Callback error  */
#endif /* USE_HAL_PCD_REGISTER_CALLBACKS */

/**
  * @}
  */

/**
  * @}
  */

/* Exported macros -----------------------------------------------------------*/
/** @defgroup PCD_Exported_Macros PCD Exported Macros
 *  @brief macros to handle interrupts and specific clock configurations
 * @{
 */
#if defined (USB_OTG_FS)
#define __HAL_PCD_ENABLE(__HANDLE__)                       (void)USB_EnableGlobalInt ((__HANDLE__)->Instance)
#define __HAL_PCD_DISABLE(__HANDLE__)                      (void)USB_DisableGlobalInt ((__HANDLE__)->Instance)

#define __HAL_PCD_GET_FLAG(__HANDLE__, __INTERRUPT__)      ((USB_ReadInterrupts((__HANDLE__)->Instance) & (__INTERRUPT__)) == (__INTERRUPT__))
#define __HAL_PCD_CLEAR_FLAG(__HANDLE__, __INTERRUPT__)    (((__HANDLE__)->Instance->GINTSTS) &=  (__INTERRUPT__))
#define __HAL_PCD_IS_INVALID_INTERRUPT(__HANDLE__)         (USB_ReadInterrupts((__HANDLE__)->Instance) == 0U)


#define __HAL_PCD_UNGATE_PHYCLOCK(__HANDLE__)             *(__IO uint32_t *)((uint32_t)((__HANDLE__)->Instance) + USB_OTG_PCGCCTL_BASE) &= \
                                                          ~(USB_OTG_PCGCCTL_STOPCLK)

#define __HAL_PCD_GATE_PHYCLOCK(__HANDLE__)               *(__IO uint32_t *)((uint32_t)((__HANDLE__)->Instance) + USB_OTG_PCGCCTL_BASE) |= USB_OTG_PCGCCTL_STOPCLK

#define __HAL_PCD_IS_PHY_SUSPENDED(__HANDLE__)            ((*(__IO uint32_t *)((uint32_t)((__HANDLE__)->Instance) + USB_OTG_PCGCCTL_BASE)) & 0x10U)

#define __HAL_USB_OTG_FS_WAKEUP_EXTI_ENABLE_IT()    EXTI->IMR1 |= USB_OTG_FS_WAKEUP_EXTI_LINE
#define __HAL_USB_OTG_FS_WAKEUP_EXTI_DISABLE_IT()   EXTI->IMR1 &= ~(USB_OTG_FS_WAKEUP_EXTI_LINE)
#endif /* defined (USB_OTG_FS) */

#if defined (USB)
#define __HAL_PCD_ENABLE(__HANDLE__)                                  (void)USB_EnableGlobalInt ((__HANDLE__)->Instance)
#define __HAL_PCD_DISABLE(__HANDLE__)                                 (void)USB_DisableGlobalInt ((__HANDLE__)->Instance)
#define __HAL_PCD_GET_FLAG(__HANDLE__, __INTERRUPT__)                 ((USB_ReadInterrupts((__HANDLE__)->Instance) & (__INTERRUPT__)) == (__INTERRUPT__))
#define __HAL_PCD_CLEAR_FLAG(__HANDLE__, __INTERRUPT__)               (((__HANDLE__)->Instance->ISTR) &= ~(__INTERRUPT__))

#define __HAL_USB_WAKEUP_EXTI_ENABLE_IT()                             EXTI->IMR1 |= USB_WAKEUP_EXTI_LINE
#define __HAL_USB_WAKEUP_EXTI_DISABLE_IT()                            EXTI->IMR1 &= ~(USB_WAKEUP_EXTI_LINE)
#endif /* defined (USB) */

/**
  * @}
  */

/* Exported functions --------------------------------------------------------*/
/** @addtogroup PCD_Exported_Functions PCD Exported Functions
  * @{
  */

/* Initialization/de-initialization functions  ********************************/
/** @addtogroup PCD_Exported_Functions_Group1 Initialization and de-initialization functions
  * @{
  */
HAL_StatusTypeDef HAL_PCD_Init(PCD_HandleTypeDef *hpcd);
HAL_StatusTypeDef HAL_PCD_DeInit(PCD_HandleTypeDef *hpcd);
void HAL_PCD_MspInit(PCD_HandleTypeDef *hpcd);
void HAL_PCD_MspDeInit(PCD_HandleTypeDef *hpcd);

#if (USE_HAL_PCD_REGISTER_CALLBACKS == 1U)
/** @defgroup HAL_PCD_Callback_ID_enumeration_definition HAL USB OTG PCD Callback ID enumeration definition
  * @brief  HAL USB OTG PCD Callback ID enumeration definition
  * @{
  */
typedef enum
{
  HAL_PCD_SOF_CB_ID          = 0x01,      /*!< USB PCD SOF callback ID          */
  HAL_PCD_SETUPSTAGE_CB_ID   = 0x02,      /*!< USB PCD Setup Stage callback ID  */
  HAL_PCD_RESET_CB_ID        = 0x03,      /*!< USB PCD Reset callback ID        */
  HAL_PCD_SUSPEND_CB_ID      = 0x04,      /*!< USB PCD Suspend callback ID      */
  HAL_PCD_RESUME_CB_ID       = 0x05,      /*!< USB PCD Resume callback ID       */
  HAL_PCD_CONNECT_CB_ID      = 0x06,      /*!< USB PCD Connect callback ID      */
  HAL_PCD_DISCONNECT_CB_ID  = 0x07,      /*!< USB PCD Disconnect callback ID   */

  HAL_PCD_MSPINIT_CB_ID      = 0x08,      /*!< USB PCD MspInit callback ID      */
  HAL_PCD_MSPDEINIT_CB_ID    = 0x09       /*!< USB PCD MspDeInit callback ID    */

} HAL_PCD_CallbackIDTypeDef;
/**
  * @}
  */

/** @defgroup HAL_PCD_Callback_pointer_definition HAL USB OTG PCD Callback pointer definition
  * @brief  HAL USB OTG PCD Callback pointer definition
  * @{
  */

typedef void (*pPCD_CallbackTypeDef)(PCD_HandleTypeDef *hpcd);                                   /*!< pointer to a common USB OTG PCD callback function  */
typedef void (*pPCD_DataOutStageCallbackTypeDef)(PCD_HandleTypeDef *hpcd, uint8_t epnum);        /*!< pointer to USB OTG PCD Data OUT Stage callback     */
typedef void (*pPCD_DataInStageCallbackTypeDef)(PCD_HandleTypeDef *hpcd, uint8_t epnum);         /*!< pointer to USB OTG PCD Data IN Stage callback      */
typedef void (*pPCD_IsoOutIncpltCallbackTypeDef)(PCD_HandleTypeDef *hpcd, uint8_t epnum);        /*!< pointer to USB OTG PCD ISO OUT Incomplete callback */
typedef void (*pPCD_IsoInIncpltCallbackTypeDef)(PCD_HandleTypeDef *hpcd, uint8_t epnum);         /*!< pointer to USB OTG PCD ISO IN Incomplete callback  */
typedef void (*pPCD_LpmCallbackTypeDef)(PCD_HandleTypeDef *hpcd, PCD_LPM_MsgTypeDef msg);        /*!< pointer to USB OTG PCD LPM callback                */
typedef void (*pPCD_BcdCallbackTypeDef)(PCD_HandleTypeDef *hpcd, PCD_BCD_MsgTypeDef msg);        /*!< pointer to USB OTG PCD BCD callback                */

/**
  * @}
  */

HAL_StatusTypeDef HAL_PCD_RegisterCallback(PCD_HandleTypeDef *hpcd, HAL_PCD_CallbackIDTypeDef CallbackID, pPCD_CallbackTypeDef pCallback);
HAL_StatusTypeDef HAL_PCD_UnRegisterCallback(PCD_HandleTypeDef *hpcd, HAL_PCD_CallbackIDTypeDef CallbackID);

HAL_StatusTypeDef HAL_PCD_RegisterDataOutStageCallback(PCD_HandleTypeDef *hpcd, pPCD_DataOutStageCallbackTypeDef pCallback);
HAL_StatusTypeDef HAL_PCD_UnRegisterDataOutStageCallback(PCD_HandleTypeDef *hpcd);

HAL_StatusTypeDef HAL_PCD_RegisterDataInStageCallback(PCD_HandleTypeDef *hpcd, pPCD_DataInStageCallbackTypeDef pCallback);
HAL_StatusTypeDef HAL_PCD_UnRegisterDataInStageCallback(PCD_HandleTypeDef *hpcd);

HAL_StatusTypeDef HAL_PCD_RegisterIsoOutIncpltCallback(PCD_HandleTypeDef *hpcd, pPCD_IsoOutIncpltCallbackTypeDef pCallback);
HAL_StatusTypeDef HAL_PCD_UnRegisterIsoOutIncpltCallback(PCD_HandleTypeDef *hpcd);

HAL_StatusTypeDef HAL_PCD_RegisterIsoInIncpltCallback(PCD_HandleTypeDef *hpcd, pPCD_IsoInIncpltCallbackTypeDef pCallback);
HAL_StatusTypeDef HAL_PCD_UnRegisterIsoInIncpltCallback(PCD_HandleTypeDef *hpcd);

HAL_StatusTypeDef HAL_PCD_RegisterBcdCallback(PCD_HandleTypeDef *hpcd, pPCD_BcdCallbackTypeDef pCallback);
HAL_StatusTypeDef HAL_PCD_UnRegisterBcdCallback(PCD_HandleTypeDef *hpcd);

HAL_StatusTypeDef HAL_PCD_RegisterLpmCallback(PCD_HandleTypeDef *hpcd, pPCD_LpmCallbackTypeDef pCallback);
HAL_StatusTypeDef HAL_PCD_UnRegisterLpmCallback(PCD_HandleTypeDef *hpcd);
#endif /* USE_HAL_PCD_REGISTER_CALLBACKS */
/**
  * @}
  */

/* I/O operation functions  ***************************************************/
/* Non-Blocking mode: Interrupt */
/** @addtogroup PCD_Exported_Functions_Group2 Input and Output operation functions
  * @{
  */
HAL_StatusTypeDef HAL_PCD_Start(PCD_HandleTypeDef *hpcd);
HAL_StatusTypeDef HAL_PCD_Stop(PCD_HandleTypeDef *hpcd);
void HAL_PCD_IRQHandler(PCD_HandleTypeDef *hpcd);

void HAL_PCD_SOFCallback(PCD_HandleTypeDef *hpcd);
void HAL_PCD_SetupStageCallback(PCD_HandleTypeDef *hpcd);
void HAL_PCD_ResetCallback(PCD_HandleTypeDef *hpcd);
void HAL_PCD_SuspendCallback(PCD_HandleTypeDef *hpcd);
void HAL_PCD_ResumeCallback(PCD_HandleTypeDef *hpcd);
void HAL_PCD_ConnectCallback(PCD_HandleTypeDef *hpcd);
void HAL_PCD_DisconnectCallback(PCD_HandleTypeDef *hpcd);

void HAL_PCD_DataOutStageCallback(PCD_HandleTypeDef *hpcd, uint8_t epnum);
void HAL_PCD_DataInStageCallback(PCD_HandleTypeDef *hpcd, uint8_t epnum);
void HAL_PCD_ISOOUTIncompleteCallback(PCD_HandleTypeDef *hpcd, uint8_t epnum);
void HAL_PCD_ISOINIncompleteCallback(PCD_HandleTypeDef *hpcd, uint8_t epnum);
/**
  * @}
  */

/* Peripheral Control functions  **********************************************/
/** @addtogroup PCD_Exported_Functions_Group3 Peripheral Control functions
  * @{
  */
HAL_StatusTypeDef HAL_PCD_DevConnect(PCD_HandleTypeDef *hpcd);
HAL_StatusTypeDef HAL_PCD_DevDisconnect(PCD_HandleTypeDef *hpcd);
HAL_StatusTypeDef HAL_PCD_SetAddress(PCD_HandleTypeDef *hpcd, uint8_t address);
HAL_StatusTypeDef HAL_PCD_EP_Open(PCD_HandleTypeDef *hpcd, uint8_t ep_addr, uint16_t ep_mps, uint8_t ep_type);
HAL_StatusTypeDef HAL_PCD_EP_Close(PCD_HandleTypeDef *hpcd, uint8_t ep_addr);
HAL_StatusTypeDef HAL_PCD_EP_Receive(PCD_HandleTypeDef *hpcd, uint8_t ep_addr, uint8_t *pBuf, uint32_t len);
HAL_StatusTypeDef HAL_PCD_EP_Transmit(PCD_HandleTypeDef *hpcd, uint8_t ep_addr, uint8_t *pBuf, uint32_t len);
uint32_t          HAL_PCD_EP_GetRxCount(PCD_HandleTypeDef *hpcd, uint8_t ep_addr);
HAL_StatusTypeDef HAL_PCD_EP_SetStall(PCD_HandleTypeDef *hpcd, uint8_t ep_addr);
HAL_StatusTypeDef HAL_PCD_EP_ClrStall(PCD_HandleTypeDef *hpcd, uint8_t ep_addr);
HAL_StatusTypeDef HAL_PCD_EP_Flush(PCD_HandleTypeDef *hpcd, uint8_t ep_addr);
HAL_StatusTypeDef HAL_PCD_ActivateRemoteWakeup(PCD_HandleTypeDef *hpcd);
HAL_StatusTypeDef HAL_PCD_DeActivateRemoteWakeup(PCD_HandleTypeDef *hpcd);
/**
  * @}
  */

/* Peripheral State functions  ************************************************/
/** @addtogroup PCD_Exported_Functions_Group4 Peripheral State functions
  * @{
  */
PCD_StateTypeDef HAL_PCD_GetState(PCD_HandleTypeDef *hpcd);
/**
  * @}
  */

/**
  * @}
  */

/* Private constants ---------------------------------------------------------*/
/** @defgroup PCD_Private_Constants PCD Private Constants
  * @{
  */
/** @defgroup USB_EXTI_Line_Interrupt USB EXTI line interrupt
  * @{
  */
#if defined (USB_OTG_FS)
#define USB_OTG_FS_WAKEUP_EXTI_RISING_EDGE                            0x08U
#define USB_OTG_FS_WAKEUP_EXTI_FALLING_EDGE                           0x0CU
#define USB_OTG_FS_WAKEUP_EXTI_RISING_FALLING_EDGE                    0x10U

#define USB_OTG_FS_WAKEUP_EXTI_LINE                                   (0x1U << 17)  /*!< USB FS EXTI Line WakeUp Interrupt */
#endif /* defined (USB_OTG_FS) */

#if defined (USB)
#define  USB_WAKEUP_EXTI_LINE                                         (0x1U << 17)  /*!< USB FS EXTI Line WakeUp Interrupt */
#endif /* defined (USB) */

/**
  * @}
  */
#if defined (USB)
/** @defgroup PCD_EP0_MPS PCD EP0 MPS
  * @{
  */
#define PCD_EP0MPS_64                                                 DEP0CTL_MPS_64
#define PCD_EP0MPS_32                                                 DEP0CTL_MPS_32
#define PCD_EP0MPS_16                                                 DEP0CTL_MPS_16
#define PCD_EP0MPS_08                                                 DEP0CTL_MPS_8
/**
  * @}
  */

/** @defgroup PCD_ENDP PCD ENDP
  * @{
  */
#define PCD_ENDP0                                                     0U
#define PCD_ENDP1                                                     1U
#define PCD_ENDP2                                                     2U
#define PCD_ENDP3                                                     3U
#define PCD_ENDP4                                                     4U
#define PCD_ENDP5                                                     5U
#define PCD_ENDP6                                                     6U
#define PCD_ENDP7                                                     7U
/**
  * @}
  */

/** @defgroup PCD_ENDP_Kind PCD Endpoint Kind
  * @{
  */
#define PCD_SNG_BUF                                                   0U
#define PCD_DBL_BUF                                                   1U
/**
  * @}
  */
#endif /* defined (USB) */
/**
  * @}
  */

#if defined (USB_OTG_FS)
#ifndef USB_OTG_DOEPINT_OTEPSPR
#define USB_OTG_DOEPINT_OTEPSPR                (0x1UL << 5)      /*!< Status Phase Received interrupt */
#endif

#ifndef USB_OTG_DOEPMSK_OTEPSPRM
#define USB_OTG_DOEPMSK_OTEPSPRM               (0x1UL << 5)      /*!< Setup Packet Received interrupt mask */
#endif

#ifndef USB_OTG_DOEPINT_NAK
#define USB_OTG_DOEPINT_NAK                    (0x1UL << 13)      /*!< NAK interrupt */
#endif

#ifndef USB_OTG_DOEPMSK_NAKM
#define USB_OTG_DOEPMSK_NAKM                   (0x1UL << 13)      /*!< OUT Packet NAK interrupt mask */
#endif

#ifndef USB_OTG_DOEPINT_STPKTRX
#define USB_OTG_DOEPINT_STPKTRX                (0x1UL << 15)      /*!< Setup Packet Received interrupt */
#endif

#ifndef USB_OTG_DOEPMSK_NYETM
#define USB_OTG_DOEPMSK_NYETM                  (0x1UL << 14)      /*!< Setup Packet Received interrupt mask */
#endif
#endif /* defined (USB_OTG_FS) */

/* Private macros ------------------------------------------------------------*/
/** @defgroup PCD_Private_Macros PCD Private Macros
 * @{
 */
#if defined (USB)
/********************  Bit definition for USB_COUNTn_RX register  *************/
#define USB_CNTRX_NBLK_MSK                    (0x1FU << 10)
#define USB_CNTRX_BLSIZE                      (0x1U << 15)

/* SetENDPOINT */
#define PCD_SET_ENDPOINT(USBx, bEpNum, wRegValue)  (*(__IO uint16_t *)(&(USBx)->EP0R + ((bEpNum) * 2U)) = (uint16_t)(wRegValue))

/* GetENDPOINT */
#define PCD_GET_ENDPOINT(USBx, bEpNum)            (*(__IO uint16_t *)(&(USBx)->EP0R + ((bEpNum) * 2U)))

/* ENDPOINT transfer */
#define USB_EP0StartXfer                          USB_EPStartXfer

/**
  * @brief  sets the type in the endpoint register(bits EP_TYPE[1:0])
  * @param  USBx USB peripheral instance register address.
  * @param  bEpNum Endpoint Number.
  * @param  wType Endpoint Type.
  * @retval None
  */
#define PCD_SET_EPTYPE(USBx, bEpNum, wType) (PCD_SET_ENDPOINT((USBx), (bEpNum), \
                                             ((PCD_GET_ENDPOINT((USBx), (bEpNum)) & USB_EP_T_MASK) | (wType) | USB_EP_CTR_TX | USB_EP_CTR_RX)))

/**
  * @brief  gets the type in the endpoint register(bits EP_TYPE[1:0])
  * @param  USBx USB peripheral instance register address.
  * @param  bEpNum Endpoint Number.
  * @retval Endpoint Type
  */
#define PCD_GET_EPTYPE(USBx, bEpNum) (PCD_GET_ENDPOINT((USBx), (bEpNum)) & USB_EP_T_FIELD)

/**
  * @brief free buffer used from the application realizing it to the line
  *         toggles bit SW_BUF in the double buffered endpoint register
  * @param USBx USB device.
  * @param   bEpNum, bDir
  * @retval None
  */
#define PCD_FreeUserBuffer(USBx, bEpNum, bDir) do { \
  if ((bDir) == 0U) \
  { \
    /* OUT double buffered endpoint */ \
    PCD_TX_DTOG((USBx), (bEpNum)); \
  } \
  else if ((bDir) == 1U) \
  { \
    /* IN double buffered endpoint */ \
    PCD_RX_DTOG((USBx), (bEpNum)); \
  } \
} while(0)

/**
  * @brief  sets the status for tx transfer (bits STAT_TX[1:0]).
  * @param  USBx USB peripheral instance register address.
  * @param  bEpNum Endpoint Number.
  * @param  wState new state
  * @retval None
  */
#define PCD_SET_EP_TX_STATUS(USBx, bEpNum, wState) do { \
   register uint16_t _wRegVal; \
   \
    _wRegVal = PCD_GET_ENDPOINT((USBx), (bEpNum)) & USB_EPTX_DTOGMASK; \
   /* toggle first bit ? */ \
   if ((USB_EPTX_DTOG1 & (wState))!= 0U) \
   { \
      _wRegVal ^= USB_EPTX_DTOG1; \
   } \
   /* toggle second bit ?  */ \
   if ((USB_EPTX_DTOG2 & (wState))!= 0U) \
   { \
      _wRegVal ^= USB_EPTX_DTOG2; \
   } \
   PCD_SET_ENDPOINT((USBx), (bEpNum), (_wRegVal | USB_EP_CTR_RX | USB_EP_CTR_TX)); \
  } while(0) /* PCD_SET_EP_TX_STATUS */

/**
  * @brief  sets the status for rx transfer (bits STAT_TX[1:0])
  * @param  USBx USB peripheral instance register address.
  * @param  bEpNum Endpoint Number.
  * @param  wState new state
  * @retval None
  */
#define PCD_SET_EP_RX_STATUS(USBx, bEpNum,wState) do { \
    register uint16_t _wRegVal; \
    \
    _wRegVal = PCD_GET_ENDPOINT((USBx), (bEpNum)) & USB_EPRX_DTOGMASK; \
    /* toggle first bit ? */ \
    if ((USB_EPRX_DTOG1 & (wState))!= 0U) \
    { \
       _wRegVal ^= USB_EPRX_DTOG1; \
    } \
    /* toggle second bit ? */ \
    if ((USB_EPRX_DTOG2 & (wState))!= 0U) \
    { \
       _wRegVal ^= USB_EPRX_DTOG2; \
    } \
    PCD_SET_ENDPOINT((USBx), (bEpNum), (_wRegVal | USB_EP_CTR_RX | USB_EP_CTR_TX)); \
  } while(0) /* PCD_SET_EP_RX_STATUS */

/**
  * @brief  sets the status for rx & tx (bits STAT_TX[1:0] & STAT_RX[1:0])
  * @param  USBx USB peripheral instance register address.
  * @param  bEpNum Endpoint Number.
  * @param  wStaterx new state.
  * @param  wStatetx new state.
  * @retval None
  */
#define PCD_SET_EP_TXRX_STATUS(USBx, bEpNum, wStaterx, wStatetx) do { \
    register uint16_t _wRegVal; \
    \
    _wRegVal = PCD_GET_ENDPOINT((USBx), (bEpNum)) & (USB_EPRX_DTOGMASK | USB_EPTX_STAT); \
    /* toggle first bit ? */ \
    if ((USB_EPRX_DTOG1 & (wStaterx))!= 0U) \
    { \
      _wRegVal ^= USB_EPRX_DTOG1; \
    } \
    /* toggle second bit ? */ \
    if ((USB_EPRX_DTOG2 & (wStaterx))!= 0U) \
    { \
      _wRegVal ^= USB_EPRX_DTOG2; \
    } \
    /* toggle first bit ? */ \
    if ((USB_EPTX_DTOG1 & (wStatetx))!= 0U) \
    { \
      _wRegVal ^= USB_EPTX_DTOG1; \
    } \
    /* toggle second bit ?  */ \
    if ((USB_EPTX_DTOG2 & (wStatetx))!= 0U) \
    { \
      _wRegVal ^= USB_EPTX_DTOG2; \
    } \
    \
    PCD_SET_ENDPOINT((USBx), (bEpNum), (_wRegVal | USB_EP_CTR_RX | USB_EP_CTR_TX)); \
  } while(0) /* PCD_SET_EP_TXRX_STATUS */

/**
  * @brief  gets the status for tx/rx transfer (bits STAT_TX[1:0]
  *         /STAT_RX[1:0])
  * @param  USBx USB peripheral instance register address.
  * @param  bEpNum Endpoint Number.
  * @retval status
  */
#define PCD_GET_EP_TX_STATUS(USBx, bEpNum)     ((uint16_t)PCD_GET_ENDPOINT((USBx), (bEpNum)) & USB_EPTX_STAT)
#define PCD_GET_EP_RX_STATUS(USBx, bEpNum)     ((uint16_t)PCD_GET_ENDPOINT((USBx), (bEpNum)) & USB_EPRX_STAT)

/**
  * @brief  sets directly the VALID tx/rx-status into the endpoint register
  * @param  USBx USB peripheral instance register address.
  * @param  bEpNum Endpoint Number.
  * @retval None
  */
#define PCD_SET_EP_TX_VALID(USBx, bEpNum)      (PCD_SET_EP_TX_STATUS((USBx), (bEpNum), USB_EP_TX_VALID))
#define PCD_SET_EP_RX_VALID(USBx, bEpNum)      (PCD_SET_EP_RX_STATUS((USBx), (bEpNum), USB_EP_RX_VALID))

/**
  * @brief  checks stall condition in an endpoint.
  * @param  USBx USB peripheral instance register address.
  * @param  bEpNum Endpoint Number.
  * @retval TRUE = endpoint in stall condition.
  */
#define PCD_GET_EP_TX_STALL_STATUS(USBx, bEpNum) (PCD_GET_EP_TX_STATUS((USBx), (bEpNum)) \
                                   == USB_EP_TX_STALL)
#define PCD_GET_EP_RX_STALL_STATUS(USBx, bEpNum) (PCD_GET_EP_RX_STATUS((USBx), (bEpNum)) \
                                   == USB_EP_RX_STALL)

/**
  * @brief  set & clear EP_KIND bit.
  * @param  USBx USB peripheral instance register address.
  * @param  bEpNum Endpoint Number.
  * @retval None
  */
#define PCD_SET_EP_KIND(USBx, bEpNum) do { \
    register uint16_t _wRegVal; \
    \
    _wRegVal = PCD_GET_ENDPOINT((USBx), (bEpNum)) & USB_EPREG_MASK; \
    \
    PCD_SET_ENDPOINT((USBx), (bEpNum), (_wRegVal | USB_EP_CTR_RX | USB_EP_CTR_TX | USB_EP_KIND)); \
  } while(0) /* PCD_SET_EP_KIND */

#define PCD_CLEAR_EP_KIND(USBx, bEpNum) do { \
    register uint16_t _wRegVal; \
    \
    _wRegVal = PCD_GET_ENDPOINT((USBx), (bEpNum)) & USB_EPKIND_MASK; \
    \
    PCD_SET_ENDPOINT((USBx), (bEpNum), (_wRegVal | USB_EP_CTR_RX | USB_EP_CTR_TX)); \
  } while(0) /* PCD_CLEAR_EP_KIND */

/**
  * @brief  Sets/clears directly STATUS_OUT bit in the endpoint register.
  * @param  USBx USB peripheral instance register address.
  * @param  bEpNum Endpoint Number.
  * @retval None
  */
#define PCD_SET_OUT_STATUS(USBx, bEpNum)       PCD_SET_EP_KIND((USBx), (bEpNum))
#define PCD_CLEAR_OUT_STATUS(USBx, bEpNum)     PCD_CLEAR_EP_KIND((USBx), (bEpNum))

/**
  * @brief  Sets/clears directly EP_KIND bit in the endpoint register.
  * @param  USBx USB peripheral instance register address.
  * @param  bEpNum Endpoint Number.
  * @retval None
  */
#define PCD_SET_EP_DBUF(USBx, bEpNum)          PCD_SET_EP_KIND((USBx), (bEpNum))
#define PCD_CLEAR_EP_DBUF(USBx, bEpNum)        PCD_CLEAR_EP_KIND((USBx), (bEpNum))

/**
  * @brief  Clears bit CTR_RX / CTR_TX in the endpoint register.
  * @param  USBx USB peripheral instance register address.
  * @param  bEpNum Endpoint Number.
  * @retval None
  */
#define PCD_CLEAR_RX_EP_CTR(USBx, bEpNum) do { \
    register uint16_t _wRegVal; \
    \
    _wRegVal = PCD_GET_ENDPOINT((USBx), (bEpNum)) & (0x7FFFU & USB_EPREG_MASK); \
    \
    PCD_SET_ENDPOINT((USBx), (bEpNum), (_wRegVal | USB_EP_CTR_TX)); \
  } while(0) /* PCD_CLEAR_RX_EP_CTR */

#define PCD_CLEAR_TX_EP_CTR(USBx, bEpNum) do { \
    register uint16_t _wRegVal; \
    \
    _wRegVal = PCD_GET_ENDPOINT((USBx), (bEpNum)) & (0xFF7FU & USB_EPREG_MASK); \
    \
    PCD_SET_ENDPOINT((USBx), (bEpNum), (_wRegVal | USB_EP_CTR_RX)); \
  } while(0) /* PCD_CLEAR_TX_EP_CTR */

/**
  * @brief  Toggles DTOG_RX / DTOG_TX bit in the endpoint register.
  * @param  USBx USB peripheral instance register address.
  * @param  bEpNum Endpoint Number.
  * @retval None
  */
#define PCD_RX_DTOG(USBx, bEpNum) do { \
    register uint16_t _wEPVal; \
    \
    _wEPVal = PCD_GET_ENDPOINT((USBx), (bEpNum)) & USB_EPREG_MASK; \
    \
    PCD_SET_ENDPOINT((USBx), (bEpNum), (_wEPVal | USB_EP_CTR_RX | USB_EP_CTR_TX | USB_EP_DTOG_RX)); \
  } while(0) /* PCD_RX_DTOG */

#define PCD_TX_DTOG(USBx, bEpNum) do { \
    register uint16_t _wEPVal; \
    \
    _wEPVal = PCD_GET_ENDPOINT((USBx), (bEpNum)) & USB_EPREG_MASK; \
    \
    PCD_SET_ENDPOINT((USBx), (bEpNum), (_wEPVal | USB_EP_CTR_RX | USB_EP_CTR_TX | USB_EP_DTOG_TX)); \
  } while(0) /* PCD_TX_DTOG */
/**
  * @brief  Clears DTOG_RX / DTOG_TX bit in the endpoint register.
  * @param  USBx USB peripheral instance register address.
  * @param  bEpNum Endpoint Number.
  * @retval None
  */
#define PCD_CLEAR_RX_DTOG(USBx, bEpNum) do { \
    register uint16_t _wRegVal; \
    \
    _wRegVal = PCD_GET_ENDPOINT((USBx), (bEpNum)); \
    \
    if ((_wRegVal & USB_EP_DTOG_RX) != 0U)\
    { \
      PCD_RX_DTOG((USBx), (bEpNum)); \
    } \
  } while(0) /* PCD_CLEAR_RX_DTOG */

#define PCD_CLEAR_TX_DTOG(USBx, bEpNum) do { \
    register uint16_t _wRegVal; \
    \
    _wRegVal = PCD_GET_ENDPOINT((USBx), (bEpNum)); \
    \
    if ((_wRegVal & USB_EP_DTOG_TX) != 0U)\
    { \
      PCD_TX_DTOG((USBx), (bEpNum)); \
    } \
  } while(0) /* PCD_CLEAR_TX_DTOG */

/**
  * @brief  Sets address in an endpoint register.
  * @param  USBx USB peripheral instance register address.
  * @param  bEpNum Endpoint Number.
  * @param  bAddr Address.
  * @retval None
  */
#define PCD_SET_EP_ADDRESS(USBx, bEpNum, bAddr) do { \
    register uint16_t _wRegVal; \
    \
    _wRegVal = (PCD_GET_ENDPOINT((USBx), (bEpNum)) & USB_EPREG_MASK) | (bAddr); \
    \
    PCD_SET_ENDPOINT((USBx), (bEpNum), (_wRegVal | USB_EP_CTR_RX | USB_EP_CTR_TX)); \
  } while(0) /* PCD_SET_EP_ADDRESS */

/**
  * @brief  Gets address in an endpoint register.
  * @param  USBx USB peripheral instance register address.
  * @param  bEpNum Endpoint Number.
  * @retval None
  */
#define PCD_GET_EP_ADDRESS(USBx, bEpNum) ((uint8_t)(PCD_GET_ENDPOINT((USBx), (bEpNum)) & USB_EPADDR_FIELD))

#define PCD_EP_TX_CNT(USBx, bEpNum) ((uint16_t *)((((uint32_t)(USBx)->BTABLE + ((uint32_t)(bEpNum) * 8U) + 2U) * PMA_ACCESS) + ((uint32_t)(USBx) + 0x400U)))
#define PCD_EP_RX_CNT(USBx, bEpNum) ((uint16_t *)((((uint32_t)(USBx)->BTABLE + ((uint32_t)(bEpNum) * 8U) + 6U) * PMA_ACCESS) + ((uint32_t)(USBx) + 0x400U)))

/**
  * @brief  sets address of the tx/rx buffer.
  * @param  USBx USB peripheral instance register address.
  * @param  bEpNum Endpoint Number.
  * @param  wAddr address to be set (must be word aligned).
  * @retval None
  */
#define PCD_SET_EP_TX_ADDRESS(USBx, bEpNum, wAddr) do { \
  register uint16_t *_wRegVal; \
  register uint32_t _wRegBase = (uint32_t)USBx; \
  \
  _wRegBase += (uint32_t)(USBx)->BTABLE; \
  _wRegVal = (uint16_t *)(_wRegBase + 0x400U + (((uint32_t)(bEpNum) * 8U) * PMA_ACCESS)); \
  *_wRegVal = ((wAddr) >> 1) << 1; \
} while(0) /* PCD_SET_EP_TX_ADDRESS */

#define PCD_SET_EP_RX_ADDRESS(USBx, bEpNum, wAddr) do { \
  register uint16_t *_wRegVal; \
  register uint32_t _wRegBase = (uint32_t)USBx; \
  \
  _wRegBase += (uint32_t)(USBx)->BTABLE; \
  _wRegVal = (uint16_t *)(_wRegBase + 0x400U + ((((uint32_t)(bEpNum) * 8U) + 4U) * PMA_ACCESS)); \
  *_wRegVal = ((wAddr) >> 1) << 1; \
} while(0) /* PCD_SET_EP_RX_ADDRESS */

/**
  * @brief  Gets address of the tx/rx buffer.
  * @param  USBx USB peripheral instance register address.
  * @param  bEpNum Endpoint Number.
  * @retval address of the buffer.
  */
#define PCD_GET_EP_TX_ADDRESS(USBx, bEpNum) ((uint16_t)*PCD_EP_TX_ADDRESS((USBx), (bEpNum)))
#define PCD_GET_EP_RX_ADDRESS(USBx, bEpNum) ((uint16_t)*PCD_EP_RX_ADDRESS((USBx), (bEpNum)))

/**
  * @brief  Sets counter of rx buffer with no. of blocks.
  * @param  pdwReg Register pointer
  * @param  wCount Counter.
  * @param  wNBlocks no. of Blocks.
  * @retval None
  */
#define PCD_CALC_BLK32(pdwReg, wCount, wNBlocks) do { \
    (wNBlocks) = (wCount) >> 5; \
    *(pdwReg) = (uint16_t)(((wNBlocks) << 10) | USB_CNTRX_BLSIZE); \
  } while(0) /* PCD_CALC_BLK32 */

#define PCD_CALC_BLK2(pdwReg, wCount, wNBlocks) do { \
    (wNBlocks) = (wCount) >> 1; \
    if (((wCount) & 0x1U) != 0U) \
    { \
      (wNBlocks)++; \
    } \
    *(pdwReg) = (uint16_t)((wNBlocks) << 10); \
  } while(0) /* PCD_CALC_BLK2 */

#define PCD_SET_EP_CNT_RX_REG(pdwReg, wCount)  do { \
    uint32_t wNBlocks; \
    if ((wCount) == 0U) \
    { \
      *(pdwReg) &= (uint16_t)~USB_CNTRX_NBLK_MSK; \
      *(pdwReg) |= USB_CNTRX_BLSIZE; \
    } \
    else if((wCount) < 62U) \
    { \
      PCD_CALC_BLK2((pdwReg), (wCount), wNBlocks); \
    } \
    else \
    { \
      PCD_CALC_BLK32((pdwReg),(wCount), wNBlocks); \
    } \
  } while(0) /* PCD_SET_EP_CNT_RX_REG */

#define PCD_SET_EP_RX_DBUF0_CNT(USBx, bEpNum, wCount) do { \
     register uint32_t _wRegBase = (uint32_t)(USBx); \
     uint16_t *pdwReg; \
     \
    _wRegBase += (uint32_t)(USBx)->BTABLE; \
    pdwReg = (uint16_t *)(_wRegBase + 0x400U + ((((uint32_t)(bEpNum) * 8U) + 2U) * PMA_ACCESS)); \
    PCD_SET_EP_CNT_RX_REG(pdwReg, (wCount)); \
  } while(0)

/**
  * @brief  sets counter for the tx/rx buffer.
  * @param  USBx USB peripheral instance register address.
  * @param  bEpNum Endpoint Number.
  * @param  wCount Counter value.
  * @retval None
  */
#define PCD_SET_EP_TX_CNT(USBx, bEpNum, wCount) do { \
    register uint32_t _wRegBase = (uint32_t)(USBx); \
    uint16_t *_wRegVal; \
    \
    _wRegBase += (uint32_t)(USBx)->BTABLE; \
    _wRegVal = (uint16_t *)(_wRegBase + 0x400U + ((((uint32_t)(bEpNum) * 8U) + 2U) * PMA_ACCESS)); \
    *_wRegVal = (uint16_t)(wCount); \
} while(0)

#define PCD_SET_EP_RX_CNT(USBx, bEpNum, wCount) do { \
    register uint32_t _wRegBase = (uint32_t)(USBx); \
    uint16_t *_wRegVal; \
    \
    _wRegBase += (uint32_t)(USBx)->BTABLE; \
    _wRegVal = (uint16_t *)(_wRegBase + 0x400U + ((((uint32_t)(bEpNum) * 8U) + 6U) * PMA_ACCESS)); \
    PCD_SET_EP_CNT_RX_REG(_wRegVal, (wCount)); \
} while(0)

/**
  * @brief  gets counter of the tx buffer.
  * @param  USBx USB peripheral instance register address.
  * @param  bEpNum Endpoint Number.
  * @retval Counter value
  */
#define PCD_GET_EP_TX_CNT(USBx, bEpNum)        ((uint32_t)(*PCD_EP_TX_CNT((USBx), (bEpNum))) & 0x3ffU)
#define PCD_GET_EP_RX_CNT(USBx, bEpNum)        ((uint32_t)(*PCD_EP_RX_CNT((USBx), (bEpNum))) & 0x3ffU)

/**
  * @brief  Sets buffer 0/1 address in a double buffer endpoint.
  * @param  USBx USB peripheral instance register address.
  * @param  bEpNum Endpoint Number.
  * @param  wBuf0Addr buffer 0 address.
  * @retval Counter value
  */
#define PCD_SET_EP_DBUF0_ADDR(USBx, bEpNum, wBuf0Addr) do { \
    PCD_SET_EP_TX_ADDRESS((USBx), (bEpNum), (wBuf0Addr)); \
  } while(0) /* PCD_SET_EP_DBUF0_ADDR */
#define PCD_SET_EP_DBUF1_ADDR(USBx, bEpNum, wBuf1Addr) do { \
    PCD_SET_EP_RX_ADDRESS((USBx), (bEpNum), (wBuf1Addr)); \
  } while(0) /* PCD_SET_EP_DBUF1_ADDR */

/**
  * @brief  Sets addresses in a double buffer endpoint.
  * @param  USBx USB peripheral instance register address.
  * @param  bEpNum Endpoint Number.
  * @param  wBuf0Addr: buffer 0 address.
  * @param  wBuf1Addr = buffer 1 address.
  * @retval None
  */
#define PCD_SET_EP_DBUF_ADDR(USBx, bEpNum, wBuf0Addr, wBuf1Addr) do { \
    PCD_SET_EP_DBUF0_ADDR((USBx), (bEpNum), (wBuf0Addr)); \
    PCD_SET_EP_DBUF1_ADDR((USBx), (bEpNum), (wBuf1Addr)); \
  } while(0) /* PCD_SET_EP_DBUF_ADDR */

/**
  * @brief  Gets buffer 0/1 address of a double buffer endpoint.
  * @param  USBx USB peripheral instance register address.
  * @param  bEpNum Endpoint Number.
  * @retval None
  */
#define PCD_GET_EP_DBUF0_ADDR(USBx, bEpNum)    (PCD_GET_EP_TX_ADDRESS((USBx), (bEpNum)))
#define PCD_GET_EP_DBUF1_ADDR(USBx, bEpNum)    (PCD_GET_EP_RX_ADDRESS((USBx), (bEpNum)))

/**
  * @brief  Gets buffer 0/1 address of a double buffer endpoint.
  * @param  USBx USB peripheral instance register address.
  * @param  bEpNum Endpoint Number.
  * @param  bDir endpoint dir  EP_DBUF_OUT = OUT
  *         EP_DBUF_IN  = IN
  * @param  wCount: Counter value
  * @retval None
  */
#define PCD_SET_EP_DBUF0_CNT(USBx, bEpNum, bDir, wCount) do { \
    if ((bDir) == 0U) \
      /* OUT endpoint */ \
    { \
      PCD_SET_EP_RX_DBUF0_CNT((USBx), (bEpNum), (wCount)); \
    } \
    else \
    { \
      if ((bDir) == 1U) \
      { \
        /* IN endpoint */ \
        PCD_SET_EP_TX_CNT((USBx), (bEpNum), (wCount)); \
      } \
    } \
  } while(0) /* SetEPDblBuf0Count*/

#define PCD_SET_EP_DBUF1_CNT(USBx, bEpNum, bDir, wCount) do { \
    register uint32_t _wBase = (uint32_t)(USBx); \
    uint16_t *_wEPRegVal; \
    \
    if ((bDir) == 0U) \
    { \
      /* OUT endpoint */ \
      PCD_SET_EP_RX_CNT((USBx), (bEpNum), (wCount)); \
    } \
    else \
    { \
      if ((bDir) == 1U) \
      { \
        /* IN endpoint */ \
        _wBase += (uint32_t)(USBx)->BTABLE; \
        _wEPRegVal = (uint16_t *)(_wBase + 0x400U + ((((uint32_t)(bEpNum) * 8U) + 6U) * PMA_ACCESS)); \
        *_wEPRegVal = (uint16_t)(wCount); \
      } \
    } \
  } while(0) /* SetEPDblBuf1Count */

#define PCD_SET_EP_DBUF_CNT(USBx, bEpNum, bDir, wCount) do { \
    PCD_SET_EP_DBUF0_CNT((USBx), (bEpNum), (bDir), (wCount)); \
    PCD_SET_EP_DBUF1_CNT((USBx), (bEpNum), (bDir), (wCount)); \
  } while(0) /* PCD_SET_EP_DBUF_CNT  */

/**
  * @brief  Gets buffer 0/1 rx/tx counter for double buffering.
  * @param  USBx USB peripheral instance register address.
  * @param  bEpNum Endpoint Number.
  * @retval None
  */
#define PCD_GET_EP_DBUF0_CNT(USBx, bEpNum)     (PCD_GET_EP_TX_CNT((USBx), (bEpNum)))
#define PCD_GET_EP_DBUF1_CNT(USBx, bEpNum)     (PCD_GET_EP_RX_CNT((USBx), (bEpNum)))

#endif /* defined (USB) */

/**
  * @}
  */

/**
  * @}
  */

/**
  * @}
  */
#endif /* defined (USB) || defined (USB_OTG_FS) */

#ifdef __cplusplus
}
#endif

#endif /* STM32L4xx_HAL_PCD_H */

/************************ (C) COPYRIGHT STMicroelectronics *****END OF FILE****/
