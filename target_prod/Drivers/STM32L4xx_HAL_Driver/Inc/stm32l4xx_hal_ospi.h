/**
  ******************************************************************************
  * @file    stm32l4xx_hal_ospi.h
  * @author  MCD Application Team
  * @brief   Header file of OSPI HAL module.
  ******************************************************************************
  * @attention
  *
  * <h2><center>&copy; Copyright (c) 2018 STMicroelectronics.
  * All rights reserved.</center></h2>
  *
  * This software component is licensed by ST under BSD 3-Clause license,
  * the "License"; You may not use this file except in compliance with the
  * License. You may obtain a copy of the License at:
  *                       opensource.org/licenses/BSD-3-Clause
  *
  ******************************************************************************
  */

/* Define to prevent recursive inclusion -------------------------------------*/
#ifndef STM32L4xx_HAL_OSPI_H
#define STM32L4xx_HAL_OSPI_H

#ifdef __cplusplus
 extern "C" {
#endif

/* Includes ------------------------------------------------------------------*/
#include "stm32l4xx_hal_def.h"

#if defined(OCTOSPI) || defined(OCTOSPI1) || defined(OCTOSPI2)

/** @addtogroup STM32L4xx_HAL_Driver
  * @{
  */

/** @addtogroup OSPI
  * @{
  */

/* Exported types ------------------------------------------------------------*/
/** @defgroup OSPI_Exported_Types OSPI Exported Types
  * @{
  */

/**
  * @brief OSPI Init structure definition
  */
typedef struct
{
  uint32_t FifoThreshold;             /* This is the threshold used by the Peripheral to generate the interrupt
                                         indicating that data are available in reception or free place
                                         is available in transmission.
                                         This parameter can be a value between 1 and 32 */
  uint32_t DualQuad;                  /* It enables or not the dual-quad mode which allow to access up to
                                         quad mode on two different devices to increase the throughput.
                                         This parameter can be a value of @ref OSPI_DualQuad */
  uint32_t MemoryType;                /* It indicates the external device type connected to the OSPI.
                                         This parameter can be a value of @ref OSPI_MemoryType */
  uint32_t DeviceSize;                /* It defines the size of the external device connected to the OSPI,
                                         it corresponds to the number of address bits required to access
                                         the external device.
                                         This parameter can be a value between 1 and 32 */
  uint32_t ChipSelectHighTime;        /* It defines the minimun number of clocks which the chip select
                                         must remain high between commands.
                                         This parameter can be a value between 1 and 8 */
  uint32_t FreeRunningClock;          /* It enables or not the free running clock.
                                         This parameter can be a value of @ref OSPI_FreeRunningClock */
  uint32_t ClockMode;                 /* It indicates the level of clock when the chip select is released.
                                         This parameter can be a value of @ref OSPI_ClockMode */
  uint32_t WrapSize;                  /* It indicates the wrap-size corresponding the external device configuration.
                                         This parameter can be a value of @ref OSPI_WrapSize */
  uint32_t ClockPrescaler;            /* It specifies the prescaler factor used for generating
                                         the external clock based on the AHB clock.
                                         This parameter can be a value between 1 and 256 */
  uint32_t SampleShifting;            /* It allows to delay to 1/2 cycle the data sampling in order
                                         to take in account external signal delays.
                                         This parameter can be a value of @ref OSPI_SampleShifting */
  uint32_t DelayHoldQuarterCycle;     /* It allows to hold to 1/4 cycle the data.
                                         This parameter can be a value of @ref OSPI_DelayHoldQuarterCycle */
  uint32_t ChipSelectBoundary;        /* It enables the transaction boundary feature and
                                         defines the boundary of bytes to release the chip select.
                                         This parameter can be a value between 0 and 31 */
}OSPI_InitTypeDef;

/**
  * @brief  HAL OSPI Handle Structure definition
  */
typedef struct __OSPI_HandleTypeDef
{
  OCTOSPI_TypeDef            *Instance;     /* OSPI registers base address                      */
  OSPI_InitTypeDef           Init;          /* OSPI initialization parameters                   */
  uint8_t                    *pBuffPtr;     /* Address of the OSPI buffer for transfer          */
  __IO uint32_t              XferSize;      /* Number of data to transfer                       */
  __IO uint32_t              XferCount;     /* Counter of data transferred                      */
  DMA_HandleTypeDef     *hdma;    /* Handle of the DMA channel used for the transfer  */
  __IO uint32_t              State;         /* Internal state of the OSPI HAL driver            */
  __IO uint32_t              ErrorCode;     /* Error code in case of HAL driver internal error  */
  uint32_t                   Timeout;       /* Timeout used for the OSPI external device access */
#if defined (USE_HAL_OSPI_REGISTER_CALLBACKS) && (USE_HAL_OSPI_REGISTER_CALLBACKS == 1U)
  void (* ErrorCallback)        (struct __OSPI_HandleTypeDef *hospi);
  void (* AbortCpltCallback)    (struct __OSPI_HandleTypeDef *hospi);
  void (* FifoThresholdCallback)(struct __OSPI_HandleTypeDef *hospi);
  void (* CmdCpltCallback)      (struct __OSPI_HandleTypeDef *hospi);
  void (* RxCpltCallback)       (struct __OSPI_HandleTypeDef *hospi);
  void (* TxCpltCallback)       (struct __OSPI_HandleTypeDef *hospi);
  void (* RxHalfCpltCallback)   (struct __OSPI_HandleTypeDef *hospi);
  void (* TxHalfCpltCallback)   (struct __OSPI_HandleTypeDef *hospi);
  void (* StatusMatchCallback)  (struct __OSPI_HandleTypeDef *hospi);
  void (* TimeOutCallback)      (struct __OSPI_HandleTypeDef *hospi);

  void (* MspInitCallback)      (struct __OSPI_HandleTypeDef *hospi);
  void (* MspDeInitCallback)    (struct __OSPI_HandleTypeDef *hospi);
#endif
}OSPI_HandleTypeDef;

/**
  * @brief  HAL OSPI Regular Command Structure definition
  */
typedef struct
{
  uint32_t OperationType;             /* It indicates if the configuration applies to the common regsiters or
                                         to the registers for the write operation (these registers are only
                                         used for memory-mapped mode).
                                         This parameter can be a value of @ref OSPI_OperationType */
  uint32_t FlashId;                   /* It indicates which external device is selected for this command (it
                                         applies only if Dualquad is disabled in the initialization structure).
                                         This parameter can be a value of @ref OSPI_FlashId */
  uint32_t Instruction;               /* It contains the instruction to be sent to the device.
                                         This parameter can be a value between 0 and 0xFFFFFFFF */
  uint32_t InstructionMode;           /* It indicates the mode of the instruction.
                                         This parameter can be a value of @ref OSPI_InstructionMode */
  uint32_t InstructionSize;           /* It indicates the size of the instruction.
                                         This parameter can be a value of @ref OSPI_InstructionSize */
  uint32_t InstructionDtrMode;        /* It enables or not the DTR mode for the instruction phase.
                                         This parameter can be a value of @ref OSPI_InstructionDtrMode */
  uint32_t Address;                   /* It contains the address to be sent to the device.
                                         This parameter can be a value between 0 and 0xFFFFFFFF */
  uint32_t AddressMode;               /* It indicates the mode of the address.
                                         This parameter can be a value of @ref OSPI_AddressMode */
  uint32_t AddressSize;               /* It indicates the size of the address.
                                         This parameter can be a value of @ref OSPI_AddressSize */
  uint32_t AddressDtrMode;            /* It enables or not the DTR mode for the address phase.
                                         This parameter can be a value of @ref OSPI_AddressDtrMode */
  uint32_t AlternateBytes;            /* It contains the alternate bytes to be sent to the device.
                                         This parameter can be a value between 0 and 0xFFFFFFFF */
  uint32_t AlternateBytesMode;        /* It indicates the mode of the alternate bytes.
                                         This parameter can be a value of @ref OSPI_AlternateBytesMode */
  uint32_t AlternateBytesSize;        /* It indicates the size of the alternate bytes.
                                         This parameter can be a value of @ref OSPI_AlternateBytesSize */
  uint32_t AlternateBytesDtrMode;     /* It enables or not the DTR mode for the alternate bytes phase.
                                         This parameter can be a value of @ref OSPI_AlternateBytesDtrMode */
  uint32_t DataMode;                  /* It indicates the mode of the data.
                                         This parameter can be a value of @ref OSPI_DataMode */
  uint32_t NbData;                    /* It indicates the number of data transferred with this command.
                                         This field is only used for indirect mode.
                                         This parameter can be a value between 1 and 0xFFFFFFFF */
  uint32_t DataDtrMode;               /* It enables or not the DTR mode for the data phase.
                                         This parameter can be a value of @ref OSPI_DataDtrMode */
  uint32_t DummyCycles;               /* It indicates the number of dummy cycles inserted before data phase.
                                         This parameter can be a value between 0 and 31 */
  uint32_t DQSMode;                   /* It enables or not the data strobe management.
                                         This parameter can be a value of @ref OSPI_DQSMode */
  uint32_t SIOOMode;                  /* It enables or not the SIOO mode.
                                         This parameter can be a value of @ref OSPI_SIOOMode */
}OSPI_RegularCmdTypeDef;

/**
  * @brief  HAL OSPI Hyperbus Configuration Structure definition
  */
typedef struct
{
  uint32_t RWRecoveryTime;       /* It indicates the number of cycles for the device read write recovery time.
                                    This parameter can be a value between 0 and 255 */
  uint32_t AccessTime;           /* It indicates the number of cycles for the device acces time.
                                    This parameter can be a value between 0 and 255 */
  uint32_t WriteZeroLatency;     /* It enables or not the latency for the write access.
                                    This parameter can be a value of @ref OSPI_WriteZeroLatency */
  uint32_t LatencyMode;          /* It configures the latency mode.
                                    This parameter can be a value of @ref OSPI_LatencyMode */
}OSPI_HyperbusCfgTypeDef;

/**
  * @brief  HAL OSPI Hyperbus Command Structure definition
  */
typedef struct
{
  uint32_t AddressSpace;     /* It indicates the address space accessed by the command.
                                This parameter can be a value of @ref OSPI_AddressSpace */
  uint32_t Address;          /* It contains the address to be sent tot he device.
                                This parameter can be a value between 0 and 0xFFFFFFFF */
  uint32_t AddressSize;      /* It indicates the size of the address.
                                This parameter can be a value of @ref OSPI_AddressSize */
  uint32_t NbData;           /* It indicates the number of data transferred with this command.
                                This field is only used for indirect mode.
                                This parameter can be a value between 1 and 0xFFFFFFFF
                                In case of autopolling mode, this parameter can be any value between 1 and 4 */
  uint32_t DQSMode;          /* It enables or not the data strobe management.
                                This parameter can be a value of @ref OSPI_DQSMode */
}OSPI_HyperbusCmdTypeDef;

/**
  * @brief  HAL OSPI Auto Polling mode configuration structure definition
  */
typedef struct
{
  uint32_t Match;              /* Specifies the value to be compared with the masked status register to get a match.
                                  This parameter can be any value between 0 and 0xFFFFFFFF */
  uint32_t Mask;               /* Specifies the mask to be applied to the status bytes received.
                                  This parameter can be any value between 0 and 0xFFFFFFFF */
  uint32_t MatchMode;          /* Specifies the method used for determining a match.
                                  This parameter can be a value of @ref OSPI_MatchMode */
  uint32_t AutomaticStop;      /* Specifies if automatic polling is stopped after a match.
                                  This parameter can be a value of @ref OSPI_AutomaticStop */
  uint32_t Interval;           /* Specifies the number of clock cycles between two read during automatic polling phases.
                                  This parameter can be any value between 0 and 0xFFFF */
}OSPI_AutoPollingTypeDef;

/**
  * @brief  HAL OSPI Memory Mapped mode configuration structure definition
  */
typedef struct
{
  uint32_t TimeOutActivation;  /* Specifies if the timeout counter is enabled to release the chip select.
                                  This parameter can be a value of @ref OSPI_TimeOutActivation */
  uint32_t TimeOutPeriod;      /* Specifies the number of clock to wait when the FIFO is full before to release the chip select.
                                  This parameter can be any value between 0 and 0xFFFF */
}OSPI_MemoryMappedTypeDef;

/**
  * @brief HAL OSPI IO Manager Configuration structure definition
  */
typedef struct
{
  uint32_t ClkPort;                /* It indicates which port of the OSPI IO Manager is used for the CLK pins.
                                      This parameter can be a value between 1 and 8 */
  uint32_t DQSPort;                /* It indicates which port of the OSPI IO Manager is used for the DQS pin.
                                      This parameter can be a value between 1 and 8 */
  uint32_t NCSPort;                /* It indicates which port of the OSPI IO Manager is used for the NCS pin.
                                      This parameter can be a value between 1 and 8 */
  uint32_t IOLowPort;              /* It indicates which port of the OSPI IO Manager is used for the IO[3:0] pins.
                                      This parameter can be a value of @ref OSPIM_IOPort */
  uint32_t IOHighPort;             /* It indicates which port of the OSPI IO Manager is used for the IO[7:4] pins.
                                      This parameter can be a value of @ref OSPIM_IOPort */
}OSPIM_CfgTypeDef;

#if defined (USE_HAL_OSPI_REGISTER_CALLBACKS) && (USE_HAL_OSPI_REGISTER_CALLBACKS == 1U)
/**
  * @brief  HAL OSPI Callback ID enumeration definition
  */
typedef enum
{
  HAL_OSPI_ERROR_CB_ID          = 0x00U,  /*!< OSPI Error Callback ID            */
  HAL_OSPI_ABORT_CB_ID          = 0x01U,  /*!< OSPI Abort Callback ID            */
  HAL_OSPI_FIFO_THRESHOLD_CB_ID = 0x02U,  /*!< OSPI FIFO Threshold Callback ID   */
  HAL_OSPI_CMD_CPLT_CB_ID       = 0x03U,  /*!< OSPI Command Complete Callback ID */
  HAL_OSPI_RX_CPLT_CB_ID        = 0x04U,  /*!< OSPI Rx Complete Callback ID      */
  HAL_OSPI_TX_CPLT_CB_ID        = 0x05U,  /*!< OSPI Tx Complete Callback ID      */
  HAL_OSPI_RX_HALF_CPLT_CB_ID   = 0x06U,  /*!< OSPI Rx Half Complete Callback ID */
  HAL_OSPI_TX_HALF_CPLT_CB_ID   = 0x07U,  /*!< OSPI Tx Half Complete Callback ID */
  HAL_OSPI_STATUS_MATCH_CB_ID   = 0x08U,  /*!< OSPI Status Match Callback ID     */
  HAL_OSPI_TIMEOUT_CB_ID        = 0x09U,  /*!< OSPI Timeout Callback ID          */

  HAL_OSPI_MSP_INIT_CB_ID       = 0x0AU,  /*!< OSPI MspInit Callback ID          */
  HAL_OSPI_MSP_DEINIT_CB_ID     = 0x0BU   /*!< OSPI MspDeInit Callback ID        */
}HAL_OSPI_CallbackIDTypeDef;

/**
  * @brief  HAL OSPI Callback pointer definition
  */
typedef void (*pOSPI_CallbackTypeDef)(OSPI_HandleTypeDef *hospi);
#endif
/**
  * @}
  */

/* Exported constants --------------------------------------------------------*/
/** @defgroup OSPI_Exported_Constants OSPI Exported Constants
  * @{
  */

/** @defgroup OSPI_State OSPI State
  * @{
  */
#define HAL_OSPI_STATE_RESET                 ((uint32_t)0x00000000U)      /*!< Initial state                                                          */
#define HAL_OSPI_STATE_HYPERBUS_INIT         ((uint32_t)0x00000001U)      /*!< Initialization done in hyperbus mode but timing configuration not done */
#define HAL_OSPI_STATE_READY                 ((uint32_t)0x00000002U)      /*!< Driver ready to be used                                                */
#define HAL_OSPI_STATE_CMD_CFG               ((uint32_t)0x00000004U)      /*!< Command (regular or hyperbus) configured, ready for an action          */
#define HAL_OSPI_STATE_READ_CMD_CFG          ((uint32_t)0x00000014U)      /*!< Read command configuration done, not the write command configuration   */
#define HAL_OSPI_STATE_WRITE_CMD_CFG         ((uint32_t)0x00000024U)      /*!< Write command configuration done, not the read command configuration   */
#define HAL_OSPI_STATE_BUSY_CMD              ((uint32_t)0x00000008U)      /*!< Command without data on-going                                          */
#define HAL_OSPI_STATE_BUSY_TX               ((uint32_t)0x00000018U)      /*!< Indirect Tx on-going                                                   */
#define HAL_OSPI_STATE_BUSY_RX               ((uint32_t)0x00000028U)      /*!< Indirect Rx on-going                                                   */
#define HAL_OSPI_STATE_BUSY_AUTO_POLLING     ((uint32_t)0x00000048U)      /*!< Auto-polling on-going                                                  */
#define HAL_OSPI_STATE_BUSY_MEM_MAPPED       ((uint32_t)0x00000088U)      /*!< Memory-mapped on-going                                                 */
#define HAL_OSPI_STATE_ABORT                 ((uint32_t)0x00000100U)      /*!< Abort on-going                                                         */
#define HAL_OSPI_STATE_ERROR                 ((uint32_t)0x00000200U)      /*!< Blocking error, driver should be re-initialized                        */
/**
  * @}
  */

/** @defgroup OSPI_ErrorCode OSPI Error Code
  * @{
  */
#define HAL_OSPI_ERROR_NONE                  ((uint32_t)0x00000000U)                                         /*!< No error                                   */
#define HAL_OSPI_ERROR_TIMEOUT               ((uint32_t)0x00000001U)                                         /*!< Timeout error                              */
#define HAL_OSPI_ERROR_TRANSFER              ((uint32_t)0x00000002U)                                         /*!< Transfer error                             */
#define HAL_OSPI_ERROR_DMA                   ((uint32_t)0x00000004U)                                         /*!< DMA transfer error                         */
#define HAL_OSPI_ERROR_INVALID_PARAM         ((uint32_t)0x00000008U)                                         /*!< Invalid parameters error                   */
#define HAL_OSPI_ERROR_INVALID_SEQUENCE      ((uint32_t)0x00000010U)                                         /*!< Sequence of the state machine is incorrect */
#if defined (USE_HAL_OSPI_REGISTER_CALLBACKS) && (USE_HAL_OSPI_REGISTER_CALLBACKS == 1U)
#define HAL_OSPI_ERROR_INVALID_CALLBACK      ((uint32_t)0x00000020U)                                         /*!< Invalid callback error                     */
#endif
/**
  * @}
  */

/** @defgroup OSPI_DualQuad OSPI Dual-Quad
  * @{
  */
#define HAL_OSPI_DUALQUAD_DISABLE            ((uint32_t)0x00000000U)                                         /*!< Dual-Quad mode disabled */
#define HAL_OSPI_DUALQUAD_ENABLE             ((uint32_t)OCTOSPI_CR_DQM)                                      /*!< Dual-Quad mode enabled  */
/**
  * @}
  */

/** @defgroup OSPI_MemoryType OSPI Memory Type
  * @{
  */
#define HAL_OSPI_MEMTYPE_MICRON              ((uint32_t)0x00000000U)                                         /*!< Micron mode       */
#define HAL_OSPI_MEMTYPE_MACRONIX            ((uint32_t)OCTOSPI_DCR1_MTYP_0)                                 /*!< Macronix mode     */
#if !defined(STM32L4R5xx)&&!defined(STM32L4R7xx)&&!defined(STM32L4R9xx)&&!defined(STM32L4S5xx)&&!defined(STM32L4S7xx)&&!defined(STM32L4S9xx)
#define HAL_OSPI_MEMTYPE_APMEMORY            ((uint32_t)OCTOSPI_DCR1_MTYP_1)                                 /*!< AP Memory mode    */
#endif
#define HAL_OSPI_MEMTYPE_MACRONIX_RAM        ((uint32_t)(OCTOSPI_DCR1_MTYP_1 | OCTOSPI_DCR1_MTYP_0))         /*!< Macronix RAM mode */
#define HAL_OSPI_MEMTYPE_HYPERBUS            ((uint32_t)OCTOSPI_DCR1_MTYP_2)                                 /*!< Hyperbus mode     */
/**
  * @}
  */

/** @defgroup OSPI_FreeRunningClock OSPI Free Running Clock
  * @{
  */
#define HAL_OSPI_FREERUNCLK_DISABLE          ((uint32_t)0x00000000U)                                         /*!< CLK is not free running               */
#define HAL_OSPI_FREERUNCLK_ENABLE           ((uint32_t)OCTOSPI_DCR1_FRCK)                                   /*!< CLK is free running (always provided) */
/**
  * @}
  */

/** @defgroup OSPI_ClockMode OSPI Clock Mode
  * @{
  */
#define HAL_OSPI_CLOCK_MODE_0                ((uint32_t)0x00000000U)                                         /*!< CLK must stay low while nCS is high  */
#define HAL_OSPI_CLOCK_MODE_3                ((uint32_t)OCTOSPI_DCR1_CKMODE)                                 /*!< CLK must stay high while nCS is high */
/**
  * @}
  */

/** @defgroup OSPI_WrapSize OSPI Wrap-Size
  * @{
  */
#define HAL_OSPI_WRAP_NOT_SUPPORTED          ((uint32_t)0x00000000U)                                         /*!< wrapped reads are not supported by the memory   */
#define HAL_OSPI_WRAP_16_BYTES               ((uint32_t)OCTOSPI_DCR2_WRAPSIZE_1)                             /*!< external memory supports wrap size of 16 bytes  */
#define HAL_OSPI_WRAP_32_BYTES               ((uint32_t)(OCTOSPI_DCR2_WRAPSIZE_0 | OCTOSPI_DCR2_WRAPSIZE_1)) /*!< external memory supports wrap size of 32 bytes  */
#define HAL_OSPI_WRAP_64_BYTES               ((uint32_t)OCTOSPI_DCR2_WRAPSIZE_2)                             /*!< external memory supports wrap size of 64 bytes  */
#define HAL_OSPI_WRAP_128_BYTES              ((uint32_t)(OCTOSPI_DCR2_WRAPSIZE_0 | OCTOSPI_DCR2_WRAPSIZE_2)) /*!< external memory supports wrap size of 128 bytes */
/**
  * @}
  */

/** @defgroup OSPI_SampleShifting OSPI Sample Shifting
  * @{
  */
#define HAL_OSPI_SAMPLE_SHIFTING_NONE        ((uint32_t)0x00000000U)                                         /*!< No shift        */
#define HAL_OSPI_SAMPLE_SHIFTING_HALFCYCLE   ((uint32_t)OCTOSPI_TCR_SSHIFT)                                  /*!< 1/2 cycle shift */
/**
  * @}
  */

/** @defgroup OSPI_DelayHoldQuarterCycle OSPI Delay Hold Quarter Cycle
  * @{
  */
#define HAL_OSPI_DHQC_DISABLE                ((uint32_t)0x00000000U)                                         /*!< No Delay             */
#define HAL_OSPI_DHQC_ENABLE                 ((uint32_t)OCTOSPI_TCR_DHQC)                                    /*!< Delay Hold 1/4 cycle */
/**
  * @}
  */

/** @defgroup OSPI_OperationType OSPI Operation Type
  * @{
  */
#define HAL_OSPI_OPTYPE_COMMON_CFG           ((uint32_t)0x00000000U)                                         /*!< Common configuration (indirect or auto-polling mode) */
#define HAL_OSPI_OPTYPE_READ_CFG             ((uint32_t)0x00000001U)                                         /*!< Read configuration (memory-mapped mode)              */
#define HAL_OSPI_OPTYPE_WRITE_CFG            ((uint32_t)0x00000002U)                                         /*!< Write configuration (memory-mapped mode)             */
/**
  * @}
  */

/** @defgroup OSPI_FlashID OSPI Flash Id
  * @{
  */
#define HAL_OSPI_FLASH_ID_1                  ((uint32_t)0x00000000U)                                         /*!< FLASH 1 selected */
#define HAL_OSPI_FLASH_ID_2                  ((uint32_t)OCTOSPI_CR_FSEL)                                     /*!< FLASH 2 selected */
/**
  * @}
  */

/** @defgroup OSPI_InstructionMode OSPI Instruction Mode
  * @{
  */
#define HAL_OSPI_INSTRUCTION_NONE            ((uint32_t)0x00000000U)                                         /*!< No instruction               */
#define HAL_OSPI_INSTRUCTION_1_LINE          ((uint32_t)OCTOSPI_CCR_IMODE_0)                                 /*!< Instruction on a single line */
#define HAL_OSPI_INSTRUCTION_2_LINES         ((uint32_t)OCTOSPI_CCR_IMODE_1)                                 /*!< Instruction on two lines     */
#define HAL_OSPI_INSTRUCTION_4_LINES         ((uint32_t)(OCTOSPI_CCR_IMODE_0 | OCTOSPI_CCR_IMODE_1))         /*!< Instruction on four lines    */
#define HAL_OSPI_INSTRUCTION_8_LINES         ((uint32_t)OCTOSPI_CCR_IMODE_2)                                 /*!< Instruction on eight lines   */
/**
  * @}
  */

/** @defgroup OSPI_InstructionSize OSPI Instruction Size
  * @{
  */
#define HAL_OSPI_INSTRUCTION_8_BITS          ((uint32_t)0x00000000U)                                         /*!< 8-bit instruction  */
#define HAL_OSPI_INSTRUCTION_16_BITS         ((uint32_t)OCTOSPI_CCR_ISIZE_0)                                 /*!< 16-bit instruction */
#define HAL_OSPI_INSTRUCTION_24_BITS         ((uint32_t)OCTOSPI_CCR_ISIZE_1)                                 /*!< 24-bit instruction */
#define HAL_OSPI_INSTRUCTION_32_BITS         ((uint32_t)OCTOSPI_CCR_ISIZE)                                   /*!< 32-bit instruction */
/**
  * @}
  */

/** @defgroup OSPI_InstructionDtrMode OSPI Instruction DTR Mode
  * @{
  */
#define HAL_OSPI_INSTRUCTION_DTR_DISABLE     ((uint32_t)0x00000000U)                                         /*!< DTR mode disabled for instruction phase */
#define HAL_OSPI_INSTRUCTION_DTR_ENABLE      ((uint32_t)OCTOSPI_CCR_IDTR)                                    /*!< DTR mode enabled for instruction phase  */
/**
  * @}
  */

/** @defgroup OSPI_AddressMode OSPI Address Mode
  * @{
  */
#define HAL_OSPI_ADDRESS_NONE                ((uint32_t)0x00000000U)                                         /*!< No address               */
#define HAL_OSPI_ADDRESS_1_LINE              ((uint32_t)OCTOSPI_CCR_ADMODE_0)                                /*!< Address on a single line */
#define HAL_OSPI_ADDRESS_2_LINES             ((uint32_t)OCTOSPI_CCR_ADMODE_1)                                /*!< Address on two lines     */
#define HAL_OSPI_ADDRESS_4_LINES             ((uint32_t)(OCTOSPI_CCR_ADMODE_0 | OCTOSPI_CCR_ADMODE_1))       /*!< Address on four lines    */
#define HAL_OSPI_ADDRESS_8_LINES             ((uint32_t)OCTOSPI_CCR_ADMODE_2)                                /*!< Address on eight lines   */
/**
  * @}
  */

/** @defgroup OSPI_AddressSize OSPI Address Size
  * @{
  */
#define HAL_OSPI_ADDRESS_8_BITS              ((uint32_t)0x00000000U)                                         /*!< 8-bit address  */
#define HAL_OSPI_ADDRESS_16_BITS             ((uint32_t)OCTOSPI_CCR_ADSIZE_0)                                /*!< 16-bit address */
#define HAL_OSPI_ADDRESS_24_BITS             ((uint32_t)OCTOSPI_CCR_ADSIZE_1)                                /*!< 24-bit address */
#define HAL_OSPI_ADDRESS_32_BITS             ((uint32_t)OCTOSPI_CCR_ADSIZE)                                  /*!< 32-bit address */
/**
  * @}
  */

/** @defgroup OSPI_AddressDtrMode OSPI Address DTR Mode
  * @{
  */
#define HAL_OSPI_ADDRESS_DTR_DISABLE         ((uint32_t)0x00000000U)                                         /*!< DTR mode disabled for address phase */
#define HAL_OSPI_ADDRESS_DTR_ENABLE          ((uint32_t)OCTOSPI_CCR_ADDTR)                                   /*!< DTR mode enabled for address phase  */
/**
  * @}
  */

/** @defgroup OSPI_AlternateBytesMode OSPI Alternate Bytes Mode
  * @{
  */
#define HAL_OSPI_ALTERNATE_BYTES_NONE        ((uint32_t)0x00000000U)                                         /*!< No alternate bytes               */
#define HAL_OSPI_ALTERNATE_BYTES_1_LINE      ((uint32_t)OCTOSPI_CCR_ABMODE_0)                                /*!< Alternate bytes on a single line */
#define HAL_OSPI_ALTERNATE_BYTES_2_LINES     ((uint32_t)OCTOSPI_CCR_ABMODE_1)                                /*!< Alternate bytes on two lines     */
#define HAL_OSPI_ALTERNATE_BYTES_4_LINES     ((uint32_t)(OCTOSPI_CCR_ABMODE_0 | OCTOSPI_CCR_ABMODE_1))       /*!< Alternate bytes on four lines    */
#define HAL_OSPI_ALTERNATE_BYTES_8_LINES     ((uint32_t)OCTOSPI_CCR_ABMODE_2)                                /*!< Alternate bytes on eight lines   */
/**
  * @}
  */

/** @defgroup OSPI_AlternateBytesSize OSPI Alternate Bytes Size
  * @{
  */
#define HAL_OSPI_ALTERNATE_BYTES_8_BITS      ((uint32_t)0x00000000U)                                         /*!< 8-bit alternate bytes  */
#define HAL_OSPI_ALTERNATE_BYTES_16_BITS     ((uint32_t)OCTOSPI_CCR_ABSIZE_0)                                /*!< 16-bit alternate bytes */
#define HAL_OSPI_ALTERNATE_BYTES_24_BITS     ((uint32_t)OCTOSPI_CCR_ABSIZE_1)                                /*!< 24-bit alternate bytes */
#define HAL_OSPI_ALTERNATE_BYTES_32_BITS     ((uint32_t)OCTOSPI_CCR_ABSIZE)                                  /*!< 32-bit alternate bytes */
/**
  * @}
  */

/** @defgroup OSPI_AlternateBytesDtrMode OSPI Alternate Bytes DTR Mode
  * @{
  */
#define HAL_OSPI_ALTERNATE_BYTES_DTR_DISABLE ((uint32_t)0x00000000U)                                         /*!< DTR mode disabled for alternate bytes phase */
#define HAL_OSPI_ALTERNATE_BYTES_DTR_ENABLE  ((uint32_t)OCTOSPI_CCR_ABDTR)                                   /*!< DTR mode enabled for alternate bytes phase  */
/**
  * @}
  */

/** @defgroup OSPI_DataMode OSPI Data Mode
  * @{
  */
#define HAL_OSPI_DATA_NONE                   ((uint32_t)0x00000000U)                                         /*!< No data               */
#define HAL_OSPI_DATA_1_LINE                 ((uint32_t)OCTOSPI_CCR_DMODE_0)                                 /*!< Data on a single line */
#define HAL_OSPI_DATA_2_LINES                ((uint32_t)OCTOSPI_CCR_DMODE_1)                                 /*!< Data on two lines     */
#define HAL_OSPI_DATA_4_LINES                ((uint32_t)(OCTOSPI_CCR_DMODE_0 | OCTOSPI_CCR_DMODE_1))         /*!< Data on four lines    */
#define HAL_OSPI_DATA_8_LINES                ((uint32_t)OCTOSPI_CCR_DMODE_2)                                 /*!< Data on eight lines   */
/**
  * @}
  */

/** @defgroup OSPI_DataDtrMode OSPI Data DTR Mode
  * @{
  */
#define HAL_OSPI_DATA_DTR_DISABLE            ((uint32_t)0x00000000U)                                         /*!< DTR mode disabled for data phase */
#define HAL_OSPI_DATA_DTR_ENABLE             ((uint32_t)OCTOSPI_CCR_DDTR)                                    /*!< DTR mode enabled for data phase  */
/**
  * @}
  */

/** @defgroup OSPI_DQSMode OSPI DQS Mode
  * @{
  */
#define HAL_OSPI_DQS_DISABLE                 ((uint32_t)0x00000000U)                                         /*!< DQS disabled */
#define HAL_OSPI_DQS_ENABLE                  ((uint32_t)OCTOSPI_CCR_DQSE)                                    /*!< DQS enabled  */
/**
  * @}
  */

/** @defgroup OSPI_SIOOMode OSPI SIOO Mode
  * @{
  */
#define HAL_OSPI_SIOO_INST_EVERY_CMD         ((uint32_t)0x00000000U)                                         /*!< Send instruction on every transaction       */
#define HAL_OSPI_SIOO_INST_ONLY_FIRST_CMD    ((uint32_t)OCTOSPI_CCR_SIOO)                                    /*!< Send instruction only for the first command */
/**
  * @}
  */

/** @defgroup OSPI_WriteZeroLatency OSPI Hyperbus Write Zero Latency Activation
  * @{
  */
#define HAL_OSPI_LATENCY_ON_WRITE            ((uint32_t)0x00000000U)                                         /*!< Latency on write accesses    */
#define HAL_OSPI_NO_LATENCY_ON_WRITE         ((uint32_t)OCTOSPI_HLCR_WZL)                                    /*!< No latency on write accesses */
/**
  * @}
  */

/** @defgroup OSPI_LatencyMode OSPI Hyperbus Latency Mode
  * @{
  */
#define HAL_OSPI_VARIABLE_LATENCY            ((uint32_t)0x00000000U)                                         /*!< Variable initial latency */
#define HAL_OSPI_FIXED_LATENCY               ((uint32_t)OCTOSPI_HLCR_LM)                                     /*!< Fixed latency            */
/**
  * @}
  */

/** @defgroup OSPI_AddressSpace OSPI Hyperbus Address Space
  * @{
  */
#define HAL_OSPI_MEMORY_ADDRESS_SPACE        ((uint32_t)0x00000000U)                                         /*!< HyperBus memory mode   */
#define HAL_OSPI_REGISTER_ADDRESS_SPACE      ((uint32_t)OCTOSPI_DCR1_MTYP_0)                                 /*!< HyperBus register mode */
/**
  * @}
  */

/** @defgroup OSPI_MatchMode OSPI Match Mode
  * @{
  */
#define HAL_OSPI_MATCH_MODE_AND              ((uint32_t)0x00000000U)                                         /*!< AND match mode between unmasked bits */
#define HAL_OSPI_MATCH_MODE_OR               ((uint32_t)OCTOSPI_CR_PMM)                                      /*!< OR match mode between unmasked bits  */
/**
  * @}
  */

/** @defgroup OSPI_AutomaticStop OSPI Automatic Stop
  * @{
  */
#define HAL_OSPI_AUTOMATIC_STOP_DISABLE      ((uint32_t)0x00000000U)                                         /*!< AutoPolling stops only with abort or OSPI disabling */
#define HAL_OSPI_AUTOMATIC_STOP_ENABLE       ((uint32_t)OCTOSPI_CR_APMS)                                     /*!< AutoPolling stops as soon as there is a match       */
/**
  * @}
  */

/** @defgroup OSPI_TimeOutActivation OSPI Timeout Activation
  * @{
  */
#define HAL_OSPI_TIMEOUT_COUNTER_DISABLE     ((uint32_t)0x00000000U)                                         /*!< Timeout counter disabled, nCS remains active               */
#define HAL_OSPI_TIMEOUT_COUNTER_ENABLE      ((uint32_t)OCTOSPI_CR_TCEN)                                     /*!< Timeout counter enabled, nCS released when timeout expires */
/**
  * @}
  */

/** @defgroup OSPI_Flags OSPI Flags
  * @{
  */
#define HAL_OSPI_FLAG_BUSY                   OCTOSPI_SR_BUSY                                                 /*!< Busy flag: operation is ongoing                                                                          */
#define HAL_OSPI_FLAG_TO                     OCTOSPI_SR_TOF                                                  /*!< Timeout flag: timeout occurs in memory-mapped mode                                                       */
#define HAL_OSPI_FLAG_SM                     OCTOSPI_SR_SMF                                                  /*!< Status match flag: received data matches in autopolling mode                                             */
#define HAL_OSPI_FLAG_FT                     OCTOSPI_SR_FTF                                                  /*!< Fifo threshold flag: Fifo threshold reached or data left after read from memory is complete              */
#define HAL_OSPI_FLAG_TC                     OCTOSPI_SR_TCF                                                  /*!< Transfer complete flag: programmed number of data have been transferred or the transfer has been aborted */
#define HAL_OSPI_FLAG_TE                     OCTOSPI_SR_TEF                                                  /*!< Transfer error flag: invalid address is being accessed                                                   */
/**
  * @}
  */

/** @defgroup OSPI_Interrupts OSPI Interrupts
  * @{
  */
#define HAL_OSPI_IT_TO                       OCTOSPI_CR_TOIE                                                 /*!< Interrupt on the timeout flag           */
#define HAL_OSPI_IT_SM                       OCTOSPI_CR_SMIE                                                 /*!< Interrupt on the status match flag      */
#define HAL_OSPI_IT_FT                       OCTOSPI_CR_FTIE                                                 /*!< Interrupt on the fifo threshold flag    */
#define HAL_OSPI_IT_TC                       OCTOSPI_CR_TCIE                                                 /*!< Interrupt on the transfer complete flag */
#define HAL_OSPI_IT_TE                       OCTOSPI_CR_TEIE                                                 /*!< Interrupt on the transfer error flag    */
/**
  * @}
  */

/** @defgroup OSPI_Timeout_definition OSPI Timeout definition
  * @{
  */
#define HAL_OSPI_TIMEOUT_DEFAULT_VALUE       ((uint32_t)5000U)                                               /* 5 s */
/**
  * @}
  */

/** @defgroup OSPIM_IOPort OSPI IO Manager IO Port
  * @{
  */
#define HAL_OSPIM_IOPORT_1_LOW             ((uint32_t)(OCTOSPIM_PCR_IOLEN | 0x1U))                          /*!< Port 1 - IO[3:0] */
#define HAL_OSPIM_IOPORT_1_HIGH            ((uint32_t)(OCTOSPIM_PCR_IOHEN | 0x1U))                          /*!< Port 1 - IO[7:4] */
#define HAL_OSPIM_IOPORT_2_LOW             ((uint32_t)(OCTOSPIM_PCR_IOLEN | 0x2U))                          /*!< Port 2 - IO[3:0] */
#define HAL_OSPIM_IOPORT_2_HIGH            ((uint32_t)(OCTOSPIM_PCR_IOHEN | 0x2U))                          /*!< Port 2 - IO[7:4] */
/**
  * @}
  */
/**
  * @}
  */

/* Exported macros -----------------------------------------------------------*/
/** @defgroup OSPI_Exported_Macros OSPI Exported Macros
  * @{
  */
/** @brief Reset OSPI handle state.
  * @param  __HANDLE__: OSPI handle.
  * @retval None
  */
#if defined (USE_HAL_OSPI_REGISTER_CALLBACKS) && (USE_HAL_OSPI_REGISTER_CALLBACKS == 1U)
#define __HAL_OSPI_RESET_HANDLE_STATE(__HANDLE__)           do {                                              \
                                                                  (__HANDLE__)->State = HAL_OSPI_STATE_RESET; \
                                                                  (__HANDLE__)->MspInitCallback = NULL;       \
                                                                  (__HANDLE__)->MspDeInitCallback = NULL;     \
                                                               } while(0)
#else
#define __HAL_OSPI_RESET_HANDLE_STATE(__HANDLE__)           ((__HANDLE__)->State = HAL_OSPI_STATE_RESET)
#endif

/** @brief  Enable the OSPI peripheral.
  * @param  __HANDLE__: specifies the OSPI Handle.
  * @retval None
  */
#define __HAL_OSPI_ENABLE(__HANDLE__)                       SET_BIT((__HANDLE__)->Instance->CR, OCTOSPI_CR_EN)

/** @brief  Disable the OSPI peripheral.
  * @param  __HANDLE__: specifies the OSPI Handle.
  * @retval None
  */
#define __HAL_OSPI_DISABLE(__HANDLE__)                      CLEAR_BIT((__HANDLE__)->Instance->CR, OCTOSPI_CR_EN)

/** @brief  Enable the specified OSPI interrupt.
  * @param  __HANDLE__: specifies the OSPI Handle.
  * @param  __INTERRUPT__: specifies the OSPI interrupt source to enable.
  *          This parameter can be one of the following values:
  *            @arg HAL_OSPI_IT_TO: OSPI Timeout interrupt
  *            @arg HAL_OSPI_IT_SM: OSPI Status match interrupt
  *            @arg HAL_OSPI_IT_FT: OSPI FIFO threshold interrupt
  *            @arg HAL_OSPI_IT_TC: OSPI Transfer complete interrupt
  *            @arg HAL_OSPI_IT_TE: OSPI Transfer error interrupt
  * @retval None
  */
#define __HAL_OSPI_ENABLE_IT(__HANDLE__, __INTERRUPT__)     SET_BIT((__HANDLE__)->Instance->CR, (__INTERRUPT__))


/** @brief  Disable the specified OSPI interrupt.
  * @param  __HANDLE__: specifies the OSPI Handle.
  * @param  __INTERRUPT__: specifies the OSPI interrupt source to disable.
  *          This parameter can be one of the following values:
  *            @arg HAL_OSPI_IT_TO: OSPI Timeout interrupt
  *            @arg HAL_OSPI_IT_SM: OSPI Status match interrupt
  *            @arg HAL_OSPI_IT_FT: OSPI FIFO threshold interrupt
  *            @arg HAL_OSPI_IT_TC: OSPI Transfer complete interrupt
  *            @arg HAL_OSPI_IT_TE: OSPI Transfer error interrupt
  * @retval None
  */
#define __HAL_OSPI_DISABLE_IT(__HANDLE__, __INTERRUPT__)    CLEAR_BIT((__HANDLE__)->Instance->CR, (__INTERRUPT__))

/** @brief  Check whether the specified OSPI interrupt source is enabled or not.
  * @param  __HANDLE__: specifies the OSPI Handle.
  * @param  __INTERRUPT__: specifies the OSPI interrupt source to check.
  *          This parameter can be one of the following values:
  *            @arg HAL_OSPI_IT_TO: OSPI Timeout interrupt
  *            @arg HAL_OSPI_IT_SM: OSPI Status match interrupt
  *            @arg HAL_OSPI_IT_FT: OSPI FIFO threshold interrupt
  *            @arg HAL_OSPI_IT_TC: OSPI Transfer complete interrupt
  *            @arg HAL_OSPI_IT_TE: OSPI Transfer error interrupt
  * @retval The new state of __INTERRUPT__ (TRUE or FALSE).
  */
#define __HAL_OSPI_GET_IT_SOURCE(__HANDLE__, __INTERRUPT__) (READ_BIT((__HANDLE__)->Instance->CR, (__INTERRUPT__)) == (__INTERRUPT__))

/**
  * @brief  Check whether the selected OSPI flag is set or not.
  * @param  __HANDLE__: specifies the OSPI Handle.
  * @param  __FLAG__: specifies the OSPI flag to check.
  *          This parameter can be one of the following values:
  *            @arg HAL_OSPI_FLAG_BUSY: OSPI Busy flag
  *            @arg HAL_OSPI_FLAG_TO:   OSPI Timeout flag
  *            @arg HAL_OSPI_FLAG_SM:   OSPI Status match flag
  *            @arg HAL_OSPI_FLAG_FT:   OSPI FIFO threshold flag
  *            @arg HAL_OSPI_FLAG_TC:   OSPI Transfer complete flag
  *            @arg HAL_OSPI_FLAG_TE:   OSPI Transfer error flag
  * @retval None
  */
#define __HAL_OSPI_GET_FLAG(__HANDLE__, __FLAG__)           ((READ_BIT((__HANDLE__)->Instance->SR, (__FLAG__)) != 0U) ? SET : RESET)

/** @brief  Clears the specified OSPI's flag status.
  * @param  __HANDLE__: specifies the OSPI Handle.
  * @param  __FLAG__: specifies the OSPI clear register flag that needs to be set
  *          This parameter can be one of the following values:
  *            @arg HAL_OSPI_FLAG_TO:   OSPI Timeout flag
  *            @arg HAL_OSPI_FLAG_SM:   OSPI Status match flag
  *            @arg HAL_OSPI_FLAG_TC:   OSPI Transfer complete flag
  *            @arg HAL_OSPI_FLAG_TE:   OSPI Transfer error flag
  * @retval None
  */
#define __HAL_OSPI_CLEAR_FLAG(__HANDLE__, __FLAG__)         WRITE_REG((__HANDLE__)->Instance->FCR, (__FLAG__))

/**
  * @}
  */

/* Exported functions --------------------------------------------------------*/
/** @addtogroup OSPI_Exported_Functions
  * @{
  */

/* Initialization/de-initialization functions  ********************************/
/** @addtogroup OSPI_Exported_Functions_Group1
  * @{
  */
HAL_StatusTypeDef     HAL_OSPI_Init                 (OSPI_HandleTypeDef *hospi);
void                  HAL_OSPI_MspInit              (OSPI_HandleTypeDef *hospi);
HAL_StatusTypeDef     HAL_OSPI_DeInit               (OSPI_HandleTypeDef *hospi);
void                  HAL_OSPI_MspDeInit            (OSPI_HandleTypeDef *hospi);

/**
  * @}
  */

/* IO operation functions *****************************************************/
/** @addtogroup OSPI_Exported_Functions_Group2
  * @{
  */
/* OSPI IRQ handler function */
void                  HAL_OSPI_IRQHandler           (OSPI_HandleTypeDef *hospi);

/* OSPI command configuration functions */
HAL_StatusTypeDef     HAL_OSPI_Command              (OSPI_HandleTypeDef *hospi, OSPI_RegularCmdTypeDef *cmd, uint32_t Timeout);
HAL_StatusTypeDef     HAL_OSPI_Command_IT           (OSPI_HandleTypeDef *hospi, OSPI_RegularCmdTypeDef *cmd);
HAL_StatusTypeDef     HAL_OSPI_HyperbusCfg          (OSPI_HandleTypeDef *hospi, OSPI_HyperbusCfgTypeDef *cfg, uint32_t Timeout);
HAL_StatusTypeDef     HAL_OSPI_HyperbusCmd          (OSPI_HandleTypeDef *hospi, OSPI_HyperbusCmdTypeDef *cmd, uint32_t Timeout);

/* OSPI indirect mode functions */
HAL_StatusTypeDef     HAL_OSPI_Transmit             (OSPI_HandleTypeDef *hospi, uint8_t *pData, uint32_t Timeout);
HAL_StatusTypeDef     HAL_OSPI_Receive              (OSPI_HandleTypeDef *hospi, uint8_t *pData, uint32_t Timeout);
HAL_StatusTypeDef     HAL_OSPI_Transmit_IT          (OSPI_HandleTypeDef *hospi, uint8_t *pData);
HAL_StatusTypeDef     HAL_OSPI_Receive_IT           (OSPI_HandleTypeDef *hospi, uint8_t *pData);
HAL_StatusTypeDef     HAL_OSPI_Transmit_DMA         (OSPI_HandleTypeDef *hospi, uint8_t *pData);
HAL_StatusTypeDef     HAL_OSPI_Receive_DMA          (OSPI_HandleTypeDef *hospi, uint8_t *pData);

/* OSPI status flag polling mode functions */
HAL_StatusTypeDef     HAL_OSPI_AutoPolling          (OSPI_HandleTypeDef *hospi, OSPI_AutoPollingTypeDef *cfg, uint32_t Timeout);
HAL_StatusTypeDef     HAL_OSPI_AutoPolling_IT       (OSPI_HandleTypeDef *hospi, OSPI_AutoPollingTypeDef *cfg);

/* OSPI memory-mapped mode functions */
HAL_StatusTypeDef     HAL_OSPI_MemoryMapped         (OSPI_HandleTypeDef *hospi, OSPI_MemoryMappedTypeDef *cfg);

/* Callback functions in non-blocking modes ***********************************/
void                  HAL_OSPI_ErrorCallback        (OSPI_HandleTypeDef *hospi);
void                  HAL_OSPI_AbortCpltCallback    (OSPI_HandleTypeDef *hospi);
void                  HAL_OSPI_FifoThresholdCallback(OSPI_HandleTypeDef *hospi);

/* OSPI indirect mode functions */
void                  HAL_OSPI_CmdCpltCallback      (OSPI_HandleTypeDef *hospi);
void                  HAL_OSPI_RxCpltCallback       (OSPI_HandleTypeDef *hospi);
void                  HAL_OSPI_TxCpltCallback       (OSPI_HandleTypeDef *hospi);
void                  HAL_OSPI_RxHalfCpltCallback   (OSPI_HandleTypeDef *hospi);
void                  HAL_OSPI_TxHalfCpltCallback   (OSPI_HandleTypeDef *hospi);

/* OSPI status flag polling mode functions */
void                  HAL_OSPI_StatusMatchCallback  (OSPI_HandleTypeDef *hospi);

/* OSPI memory-mapped mode functions */
void                  HAL_OSPI_TimeOutCallback      (OSPI_HandleTypeDef *hospi);

#if defined (USE_HAL_OSPI_REGISTER_CALLBACKS) && (USE_HAL_OSPI_REGISTER_CALLBACKS == 1U)
/* OSPI callback registering/unregistering */
HAL_StatusTypeDef     HAL_OSPI_RegisterCallback     (OSPI_HandleTypeDef *hospi, HAL_OSPI_CallbackIDTypeDef CallbackID, pOSPI_CallbackTypeDef pCallback);
HAL_StatusTypeDef     HAL_OSPI_UnRegisterCallback   (OSPI_HandleTypeDef *hospi, HAL_OSPI_CallbackIDTypeDef CallbackID);
#endif
/**
  * @}
  */

/* Peripheral Control and State functions  ************************************/
/** @addtogroup OSPI_Exported_Functions_Group3
  * @{
  */
HAL_StatusTypeDef     HAL_OSPI_Abort                (OSPI_HandleTypeDef *hospi);
HAL_StatusTypeDef     HAL_OSPI_Abort_IT             (OSPI_HandleTypeDef *hospi);
HAL_StatusTypeDef     HAL_OSPI_SetFifoThreshold     (OSPI_HandleTypeDef *hospi, uint32_t Threshold);
uint32_t              HAL_OSPI_GetFifoThreshold     (OSPI_HandleTypeDef *hospi);
HAL_StatusTypeDef     HAL_OSPI_SetTimeout           (OSPI_HandleTypeDef *hospi, uint32_t Timeout);
uint32_t              HAL_OSPI_GetError             (OSPI_HandleTypeDef *hospi);
uint32_t              HAL_OSPI_GetState             (OSPI_HandleTypeDef *hospi);

/**
  * @}
  */

/* OSPI IO Manager configuration function  ************************************/
/** @addtogroup OSPI_Exported_Functions_Group4
  * @{
  */
HAL_StatusTypeDef     HAL_OSPIM_Config              (OSPI_HandleTypeDef *hospi, OSPIM_CfgTypeDef *cfg, uint32_t Timeout);

/**
  * @}
  */

/**
  * @}
  */
/* End of exported functions -------------------------------------------------*/

/* Private macros ------------------------------------------------------------*/
/**
  @cond 0
  */
#define IS_OSPI_FIFO_THRESHOLD(THRESHOLD)  (((THRESHOLD) >= 1U) && ((THRESHOLD) <= 32U))

#define IS_OSPI_DUALQUAD_MODE(MODE)        (((MODE) == HAL_OSPI_DUALQUAD_DISABLE) || \
                                            ((MODE) == HAL_OSPI_DUALQUAD_ENABLE))

#if defined(STM32L4R5xx) || defined(STM32L4R7xx) || defined(STM32L4R9xx) || defined(STM32L4S5xx) || defined(STM32L4S7xx) || defined(STM32L4S9xx)
#define IS_OSPI_MEMORY_TYPE(TYPE)          (((TYPE) == HAL_OSPI_MEMTYPE_MICRON)       || \
                                            ((TYPE) == HAL_OSPI_MEMTYPE_MACRONIX)     || \
                                            ((TYPE) == HAL_OSPI_MEMTYPE_MACRONIX_RAM) || \
                                            ((TYPE) == HAL_OSPI_MEMTYPE_HYPERBUS))
#else
#define IS_OSPI_MEMORY_TYPE(TYPE)          (((TYPE) == HAL_OSPI_MEMTYPE_MICRON)       || \
                                            ((TYPE) == HAL_OSPI_MEMTYPE_MACRONIX)     || \
                                            ((TYPE) == HAL_OSPI_MEMTYPE_APMEMORY)     || \
                                            ((TYPE) == HAL_OSPI_MEMTYPE_MACRONIX_RAM) || \
                                            ((TYPE) == HAL_OSPI_MEMTYPE_HYPERBUS))
#endif

#define IS_OSPI_DEVICE_SIZE(SIZE)          (((SIZE) >= 1U) && ((SIZE) <= 32U))

#define IS_OSPI_CS_HIGH_TIME(TIME)         (((TIME) >= 1U) && ((TIME) <= 8U))

#define IS_OSPI_FREE_RUN_CLK(CLK)          (((CLK) == HAL_OSPI_FREERUNCLK_DISABLE) || \
                                            ((CLK) == HAL_OSPI_FREERUNCLK_ENABLE))

#define IS_OSPI_CLOCK_MODE(MODE)           (((MODE) == HAL_OSPI_CLOCK_MODE_0) || \
                                            ((MODE) == HAL_OSPI_CLOCK_MODE_3))

#define IS_OSPI_WRAP_SIZE(SIZE)            (((SIZE) == HAL_OSPI_WRAP_NOT_SUPPORTED) || \
                                            ((SIZE) == HAL_OSPI_WRAP_16_BYTES)      || \
                                            ((SIZE) == HAL_OSPI_WRAP_32_BYTES)      || \
                                            ((SIZE) == HAL_OSPI_WRAP_64_BYTES)      || \
                                            ((SIZE) == HAL_OSPI_WRAP_128_BYTES))

#define IS_OSPI_CLK_PRESCALER(PRESCALER)   (((PRESCALER) >= 1U) && ((PRESCALER) <= 256U))

#define IS_OSPI_SAMPLE_SHIFTING(CYCLE)     (((CYCLE) == HAL_OSPI_SAMPLE_SHIFTING_NONE)      || \
                                            ((CYCLE) == HAL_OSPI_SAMPLE_SHIFTING_HALFCYCLE))

#define IS_OSPI_DHQC(CYCLE)                (((CYCLE) == HAL_OSPI_DHQC_DISABLE) || \
                                            ((CYCLE) == HAL_OSPI_DHQC_ENABLE))

#define IS_OSPI_OPERATION_TYPE(TYPE)       (((TYPE) == HAL_OSPI_OPTYPE_COMMON_CFG) || \
                                            ((TYPE) == HAL_OSPI_OPTYPE_READ_CFG)   || \
                                            ((TYPE) == HAL_OSPI_OPTYPE_WRITE_CFG))

#define IS_OSPI_FLASH_ID(FLASHID)          (((FLASHID) == HAL_OSPI_FLASH_ID_1) || \
                                            ((FLASHID) == HAL_OSPI_FLASH_ID_2))

#define IS_OSPI_INSTRUCTION_MODE(MODE)     (((MODE) == HAL_OSPI_INSTRUCTION_NONE)    || \
                                            ((MODE) == HAL_OSPI_INSTRUCTION_1_LINE)  || \
                                            ((MODE) == HAL_OSPI_INSTRUCTION_2_LINES) || \
                                            ((MODE) == HAL_OSPI_INSTRUCTION_4_LINES) || \
                                            ((MODE) == HAL_OSPI_INSTRUCTION_8_LINES))

#define IS_OSPI_INSTRUCTION_SIZE(SIZE)     (((SIZE) == HAL_OSPI_INSTRUCTION_8_BITS)  || \
                                            ((SIZE) == HAL_OSPI_INSTRUCTION_16_BITS) || \
                                            ((SIZE) == HAL_OSPI_INSTRUCTION_24_BITS) || \
                                            ((SIZE) == HAL_OSPI_INSTRUCTION_32_BITS))

#define IS_OSPI_INSTRUCTION_DTR_MODE(MODE) (((MODE) == HAL_OSPI_INSTRUCTION_DTR_DISABLE) || \
                                            ((MODE) == HAL_OSPI_INSTRUCTION_DTR_ENABLE))

#define IS_OSPI_ADDRESS_MODE(MODE)         (((MODE) == HAL_OSPI_ADDRESS_NONE)    || \
                                            ((MODE) == HAL_OSPI_ADDRESS_1_LINE)  || \
                                            ((MODE) == HAL_OSPI_ADDRESS_2_LINES) || \
                                            ((MODE) == HAL_OSPI_ADDRESS_4_LINES) || \
                                            ((MODE) == HAL_OSPI_ADDRESS_8_LINES))

#define IS_OSPI_ADDRESS_SIZE(SIZE)         (((SIZE) == HAL_OSPI_ADDRESS_8_BITS)  || \
                                            ((SIZE) == HAL_OSPI_ADDRESS_16_BITS) || \
                                            ((SIZE) == HAL_OSPI_ADDRESS_24_BITS) || \
                                            ((SIZE) == HAL_OSPI_ADDRESS_32_BITS))

#define IS_OSPI_ADDRESS_DTR_MODE(MODE)     (((MODE) == HAL_OSPI_ADDRESS_DTR_DISABLE) || \
                                            ((MODE) == HAL_OSPI_ADDRESS_DTR_ENABLE))

#define IS_OSPI_ALT_BYTES_MODE(MODE)       (((MODE) == HAL_OSPI_ALTERNATE_BYTES_NONE)    || \
                                            ((MODE) == HAL_OSPI_ALTERNATE_BYTES_1_LINE)  || \
                                            ((MODE) == HAL_OSPI_ALTERNATE_BYTES_2_LINES) || \
                                            ((MODE) == HAL_OSPI_ALTERNATE_BYTES_4_LINES) || \
                                            ((MODE) == HAL_OSPI_ALTERNATE_BYTES_8_LINES))

#define IS_OSPI_ALT_BYTES_SIZE(SIZE)       (((SIZE) == HAL_OSPI_ALTERNATE_BYTES_8_BITS)  || \
                                            ((SIZE) == HAL_OSPI_ALTERNATE_BYTES_16_BITS) || \
                                            ((SIZE) == HAL_OSPI_ALTERNATE_BYTES_24_BITS) || \
                                            ((SIZE) == HAL_OSPI_ALTERNATE_BYTES_32_BITS))

#define IS_OSPI_ALT_BYTES_DTR_MODE(MODE)   (((MODE) == HAL_OSPI_ALTERNATE_BYTES_DTR_DISABLE) || \
                                            ((MODE) == HAL_OSPI_ALTERNATE_BYTES_DTR_ENABLE))

#define IS_OSPI_DATA_MODE(MODE)            (((MODE) == HAL_OSPI_DATA_NONE)    || \
                                            ((MODE) == HAL_OSPI_DATA_1_LINE)  || \
                                            ((MODE) == HAL_OSPI_DATA_2_LINES) || \
                                            ((MODE) == HAL_OSPI_DATA_4_LINES) || \
                                            ((MODE) == HAL_OSPI_DATA_8_LINES))

#define IS_OSPI_NUMBER_DATA(NUMBER)        ((NUMBER) >= 1U)

#define IS_OSPI_DATA_DTR_MODE(MODE)        (((MODE) == HAL_OSPI_DATA_DTR_DISABLE) || \
                                            ((MODE) == HAL_OSPI_DATA_DTR_ENABLE))

#define IS_OSPI_DUMMY_CYCLES(NUMBER)       ((NUMBER) <= 31U)

#define IS_OSPI_DQS_MODE(MODE)             (((MODE) == HAL_OSPI_DQS_DISABLE) || \
                                            ((MODE) == HAL_OSPI_DQS_ENABLE))

#define IS_OSPI_SIOO_MODE(MODE)            (((MODE) == HAL_OSPI_SIOO_INST_EVERY_CMD) || \
                                            ((MODE) == HAL_OSPI_SIOO_INST_ONLY_FIRST_CMD))

#define IS_OSPI_RW_RECOVERY_TIME(NUMBER)   ((NUMBER) <= 255U)

#define IS_OSPI_ACCESS_TIME(NUMBER)        ((NUMBER) <= 255U)

#define IS_OSPI_WRITE_ZERO_LATENCY(MODE)   (((MODE) == HAL_OSPI_LATENCY_ON_WRITE) || \
                                            ((MODE) == HAL_OSPI_NO_LATENCY_ON_WRITE))

#define IS_OSPI_LATENCY_MODE(MODE)         (((MODE) == HAL_OSPI_VARIABLE_LATENCY) || \
                                            ((MODE) == HAL_OSPI_FIXED_LATENCY))

#define IS_OSPI_ADDRESS_SPACE(SPACE)       (((SPACE) == HAL_OSPI_MEMORY_ADDRESS_SPACE) || \
                                            ((SPACE) == HAL_OSPI_REGISTER_ADDRESS_SPACE))

#define IS_OSPI_MATCH_MODE(MODE)           (((MODE) == HAL_OSPI_MATCH_MODE_AND) || \
                                            ((MODE) == HAL_OSPI_MATCH_MODE_OR))

#define IS_OSPI_AUTOMATIC_STOP(MODE)       (((MODE) == HAL_OSPI_AUTOMATIC_STOP_ENABLE) || \
                                            ((MODE) == HAL_OSPI_AUTOMATIC_STOP_DISABLE))

#define IS_OSPI_INTERVAL(INTERVAL)         ((INTERVAL) <= 0xFFFFU)

#define IS_OSPI_STATUS_BYTES_SIZE(SIZE)    (((SIZE) >= 1U) && ((SIZE) <= 4U))

#define IS_OSPI_TIMEOUT_ACTIVATION(MODE)   (((MODE) == HAL_OSPI_TIMEOUT_COUNTER_DISABLE) || \
                                            ((MODE) == HAL_OSPI_TIMEOUT_COUNTER_ENABLE))

#define IS_OSPI_TIMEOUT_PERIOD(PERIOD)     ((PERIOD) <= 0xFFFFU)

#define IS_OSPI_CS_BOUNDARY(BOUNDARY)      ((BOUNDARY) <= 31U)

#define IS_OSPIM_PORT(NUMBER)              (((NUMBER) >= 1U) && ((NUMBER) <= 2U))

#define IS_OSPIM_IO_PORT(PORT)             (((PORT) == HAL_OSPIM_IOPORT_1_LOW)  || \
                                            ((PORT) == HAL_OSPIM_IOPORT_1_HIGH) || \
                                            ((PORT) == HAL_OSPIM_IOPORT_2_LOW)  || \
                                            ((PORT) == HAL_OSPIM_IOPORT_2_HIGH))
/**
  @endcond
  */

/* End of private macros -----------------------------------------------------*/

/**
  * @}
  */

/**
  * @}
  */

#endif /* OCTOSPI || OCTOSPI1 || OCTOSPI2 */

#ifdef __cplusplus
}
#endif

#endif /* STM32L4xx_HAL_OSPI_H */

/************************ (C) COPYRIGHT STMicroelectronics *****END OF FILE****/
