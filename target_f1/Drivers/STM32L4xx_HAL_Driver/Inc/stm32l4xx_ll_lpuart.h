/**
  ******************************************************************************
  * @file    stm32l4xx_ll_lpuart.h
  * @author  MCD Application Team
  * @brief   Header file of LPUART LL module.
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
#ifndef STM32L4xx_LL_LPUART_H
#define STM32L4xx_LL_LPUART_H

#ifdef __cplusplus
extern "C" {
#endif

/* Includes ------------------------------------------------------------------*/
#include "stm32l4xx.h"

/** @addtogroup STM32L4xx_LL_Driver
  * @{
  */

#if defined (LPUART1)

/** @defgroup LPUART_LL LPUART
  * @{
  */

/* Private types -------------------------------------------------------------*/
/* Private variables ---------------------------------------------------------*/
#if defined(USART_PRESC_PRESCALER)
/** @defgroup LPUART_LL_Private_Variables LPUART Private Variables
  * @{
  */
/* Array used to get the LPUART prescaler division decimal values versus @ref LPUART_LL_EC_PRESCALER values */
static const uint16_t LPUART_PRESCALER_TAB[] =
{
  (uint16_t)1,
  (uint16_t)2,
  (uint16_t)4,
  (uint16_t)6,
  (uint16_t)8,
  (uint16_t)10,
  (uint16_t)12,
  (uint16_t)16,
  (uint16_t)32,
  (uint16_t)64,
  (uint16_t)128,
  (uint16_t)256
};
/**
  * @}
  */
#endif /* USART_PRESC_PRESCALER */

/* Private constants ---------------------------------------------------------*/
/** @defgroup LPUART_LL_Private_Constants LPUART Private Constants
  * @{
  */
/* Defines used in Baud Rate related macros and corresponding register setting computation */
#define LPUART_LPUARTDIV_FREQ_MUL     256U
#define LPUART_BRR_MASK               0x000FFFFFU
#define LPUART_BRR_MIN_VALUE          0x00000300U
/**
  * @}
  */


/* Private macros ------------------------------------------------------------*/
#if defined(USE_FULL_LL_DRIVER)
/** @defgroup LPUART_LL_Private_Macros LPUART Private Macros
  * @{
  */
/**
  * @}
  */
#endif /*USE_FULL_LL_DRIVER*/

/* Exported types ------------------------------------------------------------*/
#if defined(USE_FULL_LL_DRIVER)
/** @defgroup LPUART_LL_ES_INIT LPUART Exported Init structures
  * @{
  */

/**
  * @brief LL LPUART Init Structure definition
  */
typedef struct
{
#if defined(USART_PRESC_PRESCALER)
  uint32_t PrescalerValue;            /*!< Specifies the Prescaler to compute the communication baud rate.
                                           This parameter can be a value of @ref LPUART_LL_EC_PRESCALER.

                                           This feature can be modified afterwards using unitary function @ref LL_LPUART_SetPrescaler().*/

#endif /* USART_PRESC_PRESCALER */
  uint32_t BaudRate;                  /*!< This field defines expected LPUART communication baud rate.

                                           This feature can be modified afterwards using unitary function @ref LL_LPUART_SetBaudRate().*/

  uint32_t DataWidth;                 /*!< Specifies the number of data bits transmitted or received in a frame.
                                           This parameter can be a value of @ref LPUART_LL_EC_DATAWIDTH.

                                           This feature can be modified afterwards using unitary function @ref LL_LPUART_SetDataWidth().*/

  uint32_t StopBits;                  /*!< Specifies the number of stop bits transmitted.
                                           This parameter can be a value of @ref LPUART_LL_EC_STOPBITS.

                                           This feature can be modified afterwards using unitary function @ref LL_LPUART_SetStopBitsLength().*/

  uint32_t Parity;                    /*!< Specifies the parity mode.
                                           This parameter can be a value of @ref LPUART_LL_EC_PARITY.

                                           This feature can be modified afterwards using unitary function @ref LL_LPUART_SetParity().*/

  uint32_t TransferDirection;         /*!< Specifies whether the Receive and/or Transmit mode is enabled or disabled.
                                           This parameter can be a value of @ref LPUART_LL_EC_DIRECTION.

                                           This feature can be modified afterwards using unitary function @ref LL_LPUART_SetTransferDirection().*/

  uint32_t HardwareFlowControl;       /*!< Specifies whether the hardware flow control mode is enabled or disabled.
                                           This parameter can be a value of @ref LPUART_LL_EC_HWCONTROL.

                                           This feature can be modified afterwards using unitary function @ref LL_LPUART_SetHWFlowCtrl().*/

} LL_LPUART_InitTypeDef;

/**
  * @}
  */
#endif /* USE_FULL_LL_DRIVER */

/* Exported constants --------------------------------------------------------*/
/** @defgroup LPUART_LL_Exported_Constants LPUART Exported Constants
  * @{
  */

/** @defgroup LPUART_LL_EC_CLEAR_FLAG Clear Flags Defines
  * @brief    Flags defines which can be used with LL_LPUART_WriteReg function
  * @{
  */
#define LL_LPUART_ICR_PECF                 USART_ICR_PECF                /*!< Parity error flag */
#define LL_LPUART_ICR_FECF                 USART_ICR_FECF                /*!< Framing error flag */
#define LL_LPUART_ICR_NCF                  USART_ICR_NECF                /*!< Noise error detected flag */
#define LL_LPUART_ICR_ORECF                USART_ICR_ORECF               /*!< Overrun error flag */
#define LL_LPUART_ICR_IDLECF               USART_ICR_IDLECF              /*!< Idle line detected flag */
#if defined(USART_CR1_FIFOEN)
#define LL_LPUART_ICR_TXFECF               USART_ICR_TXFECF              /*!< TX FIFO Empty Clear flag */
#endif /* USART_CR1_FIFOEN */
#define LL_LPUART_ICR_TCCF                 USART_ICR_TCCF                /*!< Transmission complete flag */
#define LL_LPUART_ICR_CTSCF                USART_ICR_CTSCF               /*!< CTS flag */
#define LL_LPUART_ICR_CMCF                 USART_ICR_CMCF                /*!< Character match flag */
#define LL_LPUART_ICR_WUCF                 USART_ICR_WUCF                /*!< Wakeup from Stop mode flag */
/**
  * @}
  */

/** @defgroup LPUART_LL_EC_GET_FLAG Get Flags Defines
  * @brief    Flags defines which can be used with LL_LPUART_ReadReg function
  * @{
  */
#define LL_LPUART_ISR_PE                   USART_ISR_PE                  /*!< Parity error flag */
#define LL_LPUART_ISR_FE                   USART_ISR_FE                  /*!< Framing error flag */
#define LL_LPUART_ISR_NE                   USART_ISR_NE                  /*!< Noise detected flag */
#define LL_LPUART_ISR_ORE                  USART_ISR_ORE                 /*!< Overrun error flag */
#define LL_LPUART_ISR_IDLE                 USART_ISR_IDLE                /*!< Idle line detected flag */
#if defined(USART_CR1_FIFOEN)
#define LL_LPUART_ISR_RXNE_RXFNE           USART_ISR_RXNE_RXFNE          /*!< Read data register or RX FIFO not empty flag */
#else
#define LL_LPUART_ISR_RXNE                 USART_ISR_RXNE                /*!< Read data register not empty flag */
#endif /* USART_CR1_FIFOEN */
#define LL_LPUART_ISR_TC                   USART_ISR_TC                  /*!< Transmission complete flag */
#if defined(USART_CR1_FIFOEN)
#define LL_LPUART_ISR_TXE_TXFNF            USART_ISR_TXE_TXFNF           /*!< Transmit data register empty or TX FIFO Not Full flag*/
#else
#define LL_LPUART_ISR_TXE                  USART_ISR_TXE                 /*!< Transmit data register empty flag */
#endif /* USART_CR1_FIFOEN */
#define LL_LPUART_ISR_CTSIF                USART_ISR_CTSIF               /*!< CTS interrupt flag */
#define LL_LPUART_ISR_CTS                  USART_ISR_CTS                 /*!< CTS flag */
#define LL_LPUART_ISR_BUSY                 USART_ISR_BUSY                /*!< Busy flag */
#define LL_LPUART_ISR_CMF                  USART_ISR_CMF                 /*!< Character match flag */
#define LL_LPUART_ISR_SBKF                 USART_ISR_SBKF                /*!< Send break flag */
#define LL_LPUART_ISR_RWU                  USART_ISR_RWU                 /*!< Receiver wakeup from Mute mode flag */
#define LL_LPUART_ISR_WUF                  USART_ISR_WUF                 /*!< Wakeup from Stop mode flag */
#define LL_LPUART_ISR_TEACK                USART_ISR_TEACK               /*!< Transmit enable acknowledge flag */
#define LL_LPUART_ISR_REACK                USART_ISR_REACK               /*!< Receive enable acknowledge flag */
#if defined(USART_CR1_FIFOEN)
#define LL_LPUART_ISR_TXFE                 USART_ISR_TXFE                /*!< TX FIFO empty flag */
#define LL_LPUART_ISR_RXFF                 USART_ISR_RXFF                /*!< RX FIFO full flag */
#define LL_LPUART_ISR_RXFT                 USART_ISR_RXFT                /*!< RX FIFO threshold flag */
#define LL_LPUART_ISR_TXFT                 USART_ISR_TXFT                /*!< TX FIFO threshold flag */
#endif /* USART_CR1_FIFOEN */
/**
  * @}
  */

/** @defgroup LPUART_LL_EC_IT IT Defines
  * @brief    IT defines which can be used with LL_LPUART_ReadReg and  LL_LPUART_WriteReg functions
  * @{
  */
#define LL_LPUART_CR1_IDLEIE               USART_CR1_IDLEIE              /*!< IDLE interrupt enable */
#if defined(USART_CR1_FIFOEN)
#define LL_LPUART_CR1_RXNEIE_RXFNEIE       USART_CR1_RXNEIE_RXFNEIE      /*!< Read data register and RXFIFO not empty interrupt enable */
#else
#define LL_LPUART_CR1_RXNEIE               USART_CR1_RXNEIE              /*!< Read data register not empty interrupt enable */
#endif /* USART_CR1_FIFOEN */
#define LL_LPUART_CR1_TCIE                 USART_CR1_TCIE                /*!< Transmission complete interrupt enable */
#if defined(USART_CR1_FIFOEN)
#define LL_LPUART_CR1_TXEIE_TXFNFIE        USART_CR1_TXEIE_TXFNFIE       /*!< Transmit data register empty and TX FIFO not full interrupt enable */
#else
#define LL_LPUART_CR1_TXEIE                USART_CR1_TXEIE               /*!< Transmit data register empty interrupt enable */
#endif /* USART_CR1_FIFOEN */
#define LL_LPUART_CR1_PEIE                 USART_CR1_PEIE                /*!< Parity error */
#define LL_LPUART_CR1_CMIE                 USART_CR1_CMIE                /*!< Character match interrupt enable */
#if defined(USART_CR1_FIFOEN)
#define LL_LPUART_CR1_TXFEIE               USART_CR1_TXFEIE              /*!< TX FIFO empty interrupt enable */
#define LL_LPUART_CR1_RXFFIE               USART_CR1_RXFFIE              /*!< RX FIFO full interrupt enable */
#endif /* USART_CR1_FIFOEN */
#define LL_LPUART_CR3_EIE                  USART_CR3_EIE                 /*!< Error interrupt enable */
#define LL_LPUART_CR3_CTSIE                USART_CR3_CTSIE               /*!< CTS interrupt enable */
#define LL_LPUART_CR3_WUFIE                USART_CR3_WUFIE               /*!< Wakeup from Stop mode interrupt enable */
#if defined(USART_CR1_FIFOEN)
#define LL_LPUART_CR3_TXFTIE               USART_CR3_TXFTIE              /*!< TX FIFO threshold interrupt enable */
#define LL_LPUART_CR3_RXFTIE               USART_CR3_RXFTIE              /*!< RX FIFO threshold interrupt enable */
#endif /* USART_CR1_FIFOEN */
/**
  * @}
  */
#if defined(USART_CR1_FIFOEN)

/** @defgroup LPUART_LL_EC_FIFOTHRESHOLD FIFO Threshold
  * @{
  */
#define LL_LPUART_FIFOTHRESHOLD_1_8        0x00000000U /*!< FIFO reaches 1/8 of its depth */
#define LL_LPUART_FIFOTHRESHOLD_1_4        0x00000001U /*!< FIFO reaches 1/4 of its depth */
#define LL_LPUART_FIFOTHRESHOLD_1_2        0x00000002U /*!< FIFO reaches 1/2 of its depth */
#define LL_LPUART_FIFOTHRESHOLD_3_4        0x00000003U /*!< FIFO reaches 3/4 of its depth */
#define LL_LPUART_FIFOTHRESHOLD_7_8        0x00000004U /*!< FIFO reaches 7/8 of its depth */
#define LL_LPUART_FIFOTHRESHOLD_8_8        0x00000005U /*!< FIFO becomes empty for TX and full for RX */
/**
  * @}
  */
#endif /* USART_CR1_FIFOEN */

/** @defgroup LPUART_LL_EC_DIRECTION Direction
  * @{
  */
#define LL_LPUART_DIRECTION_NONE           0x00000000U                        /*!< Transmitter and Receiver are disabled */
#define LL_LPUART_DIRECTION_RX             USART_CR1_RE                       /*!< Transmitter is disabled and Receiver is enabled */
#define LL_LPUART_DIRECTION_TX             USART_CR1_TE                       /*!< Transmitter is enabled and Receiver is disabled */
#define LL_LPUART_DIRECTION_TX_RX          (USART_CR1_TE |USART_CR1_RE)       /*!< Transmitter and Receiver are enabled */
/**
  * @}
  */

/** @defgroup LPUART_LL_EC_PARITY Parity Control
  * @{
  */
#define LL_LPUART_PARITY_NONE              0x00000000U                        /*!< Parity control disabled */
#define LL_LPUART_PARITY_EVEN              USART_CR1_PCE                      /*!< Parity control enabled and Even Parity is selected */
#define LL_LPUART_PARITY_ODD               (USART_CR1_PCE | USART_CR1_PS)     /*!< Parity control enabled and Odd Parity is selected */
/**
  * @}
  */

/** @defgroup LPUART_LL_EC_WAKEUP Wakeup
  * @{
  */
#define LL_LPUART_WAKEUP_IDLELINE          0x00000000U                        /*!<  LPUART wake up from Mute mode on Idle Line */
#define LL_LPUART_WAKEUP_ADDRESSMARK       USART_CR1_WAKE                     /*!<  LPUART wake up from Mute mode on Address Mark */
/**
  * @}
  */

/** @defgroup LPUART_LL_EC_DATAWIDTH Datawidth
  * @{
  */
#define LL_LPUART_DATAWIDTH_7B             USART_CR1_M1                       /*!< 7 bits word length : Start bit, 7 data bits, n stop bits */
#define LL_LPUART_DATAWIDTH_8B             0x00000000U                        /*!< 8 bits word length : Start bit, 8 data bits, n stop bits */
#define LL_LPUART_DATAWIDTH_9B             USART_CR1_M0                       /*!< 9 bits word length : Start bit, 9 data bits, n stop bits */
/**
  * @}
  */
#if defined(USART_PRESC_PRESCALER)

/** @defgroup LPUART_LL_EC_PRESCALER Clock Source Prescaler
  * @{
  */
#define LL_LPUART_PRESCALER_DIV1           0x00000000U                                                                   /*!< Input clock not devided   */
#define LL_LPUART_PRESCALER_DIV2           (USART_PRESC_PRESCALER_0)                                                     /*!< Input clock devided by 2  */
#define LL_LPUART_PRESCALER_DIV4           (USART_PRESC_PRESCALER_1)                                                     /*!< Input clock devided by 4  */
#define LL_LPUART_PRESCALER_DIV6           (USART_PRESC_PRESCALER_1 | USART_PRESC_PRESCALER_0)                           /*!< Input clock devided by 6  */
#define LL_LPUART_PRESCALER_DIV8           (USART_PRESC_PRESCALER_2)                                                     /*!< Input clock devided by 8  */
#define LL_LPUART_PRESCALER_DIV10          (USART_PRESC_PRESCALER_2 | USART_PRESC_PRESCALER_0)                           /*!< Input clock devided by 10 */
#define LL_LPUART_PRESCALER_DIV12          (USART_PRESC_PRESCALER_2 | USART_PRESC_PRESCALER_1)                           /*!< Input clock devided by 12 */
#define LL_LPUART_PRESCALER_DIV16          (USART_PRESC_PRESCALER_2 | USART_PRESC_PRESCALER_1 | USART_PRESC_PRESCALER_0) /*!< Input clock devided by 16 */
#define LL_LPUART_PRESCALER_DIV32          (USART_PRESC_PRESCALER_3)                                                     /*!< Input clock devided by 32 */
#define LL_LPUART_PRESCALER_DIV64          (USART_PRESC_PRESCALER_3 | USART_PRESC_PRESCALER_0)                           /*!< Input clock devided by 64 */
#define LL_LPUART_PRESCALER_DIV128         (USART_PRESC_PRESCALER_3 | USART_PRESC_PRESCALER_1)                           /*!< Input clock devided by 128 */
#define LL_LPUART_PRESCALER_DIV256         (USART_PRESC_PRESCALER_3 | USART_PRESC_PRESCALER_1 | USART_PRESC_PRESCALER_0) /*!< Input clock devided by 256 */
/**
  * @}
  */
#endif /* USART_PRESC_PRESCALER */

/** @defgroup LPUART_LL_EC_STOPBITS Stop Bits
  * @{
  */
#define LL_LPUART_STOPBITS_1               0x00000000U                        /*!< 1 stop bit */
#define LL_LPUART_STOPBITS_2               USART_CR2_STOP_1                   /*!< 2 stop bits */
/**
  * @}
  */

/** @defgroup LPUART_LL_EC_TXRX TX RX Pins Swap
  * @{
  */
#define LL_LPUART_TXRX_STANDARD            0x00000000U                        /*!< TX/RX pins are used as defined in standard pinout */
#define LL_LPUART_TXRX_SWAPPED             (USART_CR2_SWAP)                   /*!< TX and RX pins functions are swapped.             */
/**
  * @}
  */

/** @defgroup LPUART_LL_EC_RXPIN_LEVEL RX Pin Active Level Inversion
  * @{
  */
#define LL_LPUART_RXPIN_LEVEL_STANDARD     0x00000000U                        /*!< RX pin signal works using the standard logic levels */
#define LL_LPUART_RXPIN_LEVEL_INVERTED     (USART_CR2_RXINV)                  /*!< RX pin signal values are inverted.                  */
/**
  * @}
  */

/** @defgroup LPUART_LL_EC_TXPIN_LEVEL TX Pin Active Level Inversion
  * @{
  */
#define LL_LPUART_TXPIN_LEVEL_STANDARD     0x00000000U                        /*!< TX pin signal works using the standard logic levels */
#define LL_LPUART_TXPIN_LEVEL_INVERTED     (USART_CR2_TXINV)                  /*!< TX pin signal values are inverted.                  */
/**
  * @}
  */

/** @defgroup LPUART_LL_EC_BINARY_LOGIC Binary Data Inversion
  * @{
  */
#define LL_LPUART_BINARY_LOGIC_POSITIVE    0x00000000U                        /*!< Logical data from the data register are send/received in positive/direct logic. (1=H, 0=L) */
#define LL_LPUART_BINARY_LOGIC_NEGATIVE    USART_CR2_DATAINV                  /*!< Logical data from the data register are send/received in negative/inverse logic. (1=L, 0=H). The parity bit is also inverted. */
/**
  * @}
  */

/** @defgroup LPUART_LL_EC_BITORDER Bit Order
  * @{
  */
#define LL_LPUART_BITORDER_LSBFIRST        0x00000000U                        /*!< data is transmitted/received with data bit 0 first, following the start bit */
#define LL_LPUART_BITORDER_MSBFIRST        USART_CR2_MSBFIRST                 /*!< data is transmitted/received with the MSB first, following the start bit */
/**
  * @}
  */

/** @defgroup LPUART_LL_EC_ADDRESS_DETECT Address Length Detection
  * @{
  */
#define LL_LPUART_ADDRESS_DETECT_4B        0x00000000U                        /*!< 4-bit address detection method selected */
#define LL_LPUART_ADDRESS_DETECT_7B        USART_CR2_ADDM7                    /*!< 7-bit address detection (in 8-bit data mode) method selected */
/**
  * @}
  */

/** @defgroup LPUART_LL_EC_HWCONTROL Hardware Control
  * @{
  */
#define LL_LPUART_HWCONTROL_NONE           0x00000000U                        /*!< CTS and RTS hardware flow control disabled */
#define LL_LPUART_HWCONTROL_RTS            USART_CR3_RTSE                     /*!< RTS output enabled, data is only requested when there is space in the receive buffer */
#define LL_LPUART_HWCONTROL_CTS            USART_CR3_CTSE                     /*!< CTS mode enabled, data is only transmitted when the nCTS input is asserted (tied to 0) */
#define LL_LPUART_HWCONTROL_RTS_CTS        (USART_CR3_RTSE | USART_CR3_CTSE)  /*!< CTS and RTS hardware flow control enabled */
/**
  * @}
  */

/** @defgroup LPUART_LL_EC_WAKEUP_ON Wakeup Activation
  * @{
  */
#define LL_LPUART_WAKEUP_ON_ADDRESS        0x00000000U                             /*!< Wake up active on address match */
#define LL_LPUART_WAKEUP_ON_STARTBIT       USART_CR3_WUS_1                         /*!< Wake up active on Start bit detection */
#define LL_LPUART_WAKEUP_ON_RXNE           (USART_CR3_WUS_0 | USART_CR3_WUS_1)     /*!< Wake up active on RXNE */
/**
  * @}
  */

/** @defgroup LPUART_LL_EC_DE_POLARITY Driver Enable Polarity
  * @{
  */
#define LL_LPUART_DE_POLARITY_HIGH         0x00000000U                        /*!< DE signal is active high */
#define LL_LPUART_DE_POLARITY_LOW          USART_CR3_DEP                      /*!< DE signal is active low */
/**
  * @}
  */

/** @defgroup LPUART_LL_EC_DMA_REG_DATA DMA Register Data
  * @{
  */
#define LL_LPUART_DMA_REG_DATA_TRANSMIT    0x00000000U                       /*!< Get address of data register used for transmission */
#define LL_LPUART_DMA_REG_DATA_RECEIVE     0x00000001U                       /*!< Get address of data register used for reception */
/**
  * @}
  */

/**
  * @}
  */

/* Exported macro ------------------------------------------------------------*/
/** @defgroup LPUART_LL_Exported_Macros LPUART Exported Macros
  * @{
  */

/** @defgroup LPUART_LL_EM_WRITE_READ Common Write and read registers Macros
  * @{
  */

/**
  * @brief  Write a value in LPUART register
  * @param  __INSTANCE__ LPUART Instance
  * @param  __REG__ Register to be written
  * @param  __VALUE__ Value to be written in the register
  * @retval None
  */
#define LL_LPUART_WriteReg(__INSTANCE__, __REG__, __VALUE__) WRITE_REG(__INSTANCE__->__REG__, (__VALUE__))

/**
  * @brief  Read a value in LPUART register
  * @param  __INSTANCE__ LPUART Instance
  * @param  __REG__ Register to be read
  * @retval Register value
  */
#define LL_LPUART_ReadReg(__INSTANCE__, __REG__) READ_REG(__INSTANCE__->__REG__)
/**
  * @}
  */

/** @defgroup LPUART_LL_EM_Exported_Macros_Helper Helper Macros
  * @{
  */

/**
  * @brief  Compute LPUARTDIV value according to Peripheral Clock and
  *         expected Baud Rate (20-bit value of LPUARTDIV is returned)
  * @param  __PERIPHCLK__ Peripheral Clock frequency used for LPUART Instance
  @if USART_PRESC_PRESCALER
  * @param  __PRESCALER__ This parameter can be one of the following values:
  *         @arg @ref LL_LPUART_PRESCALER_DIV1
  *         @arg @ref LL_LPUART_PRESCALER_DIV2
  *         @arg @ref LL_LPUART_PRESCALER_DIV4
  *         @arg @ref LL_LPUART_PRESCALER_DIV6
  *         @arg @ref LL_LPUART_PRESCALER_DIV8
  *         @arg @ref LL_LPUART_PRESCALER_DIV10
  *         @arg @ref LL_LPUART_PRESCALER_DIV12
  *         @arg @ref LL_LPUART_PRESCALER_DIV16
  *         @arg @ref LL_LPUART_PRESCALER_DIV32
  *         @arg @ref LL_LPUART_PRESCALER_DIV64
  *         @arg @ref LL_LPUART_PRESCALER_DIV128
  *         @arg @ref LL_LPUART_PRESCALER_DIV256
  @endif
  * @param  __BAUDRATE__ Baud Rate value to achieve
  * @retval LPUARTDIV value to be used for BRR register filling
  */
#if defined(USART_PRESC_PRESCALER)
#define __LL_LPUART_DIV(__PERIPHCLK__, __PRESCALER__, __BAUDRATE__) (uint32_t)((((((uint64_t)(__PERIPHCLK__)/(uint64_t)(LPUART_PRESCALER_TAB[(uint16_t)(__PRESCALER__)])) * LPUART_LPUARTDIV_FREQ_MUL)\
                                                                                + (uint32_t)((__BAUDRATE__)/2U))/(__BAUDRATE__)) & LPUART_BRR_MASK)
#else
#define __LL_LPUART_DIV(__PERIPHCLK__, __BAUDRATE__) (uint32_t)(((((uint64_t)(__PERIPHCLK__)*LPUART_LPUARTDIV_FREQ_MUL) + (uint32_t)((__BAUDRATE__)/2U))/(__BAUDRATE__))\
                                                                & LPUART_BRR_MASK)
#endif /* USART_PRESC_PRESCALER */

/**
  * @}
  */

/**
  * @}
  */

/* Exported functions --------------------------------------------------------*/
/** @defgroup LPUART_LL_Exported_Functions LPUART Exported Functions
  * @{
  */

/** @defgroup LPUART_LL_EF_Configuration Configuration functions
  * @{
  */

/**
  * @brief  LPUART Enable
  * @rmtoll CR1          UE            LL_LPUART_Enable
  * @param  LPUARTx LPUART Instance
  * @retval None
  */
__STATIC_INLINE void LL_LPUART_Enable(USART_TypeDef *LPUARTx)
{
  SET_BIT(LPUARTx->CR1, USART_CR1_UE);
}

/**
  * @brief  LPUART Disable
  * @note   When LPUART is disabled, LPUART prescalers and outputs are stopped immediately,
  *         and current operations are discarded. The configuration of the LPUART is kept, but all the status
  *         flags, in the LPUARTx_ISR are set to their default values.
  * @note   In order to go into low-power mode without generating errors on the line,
  *         the TE bit must be reset before and the software must wait
  *         for the TC bit in the LPUART_ISR to be set before resetting the UE bit.
  *         The DMA requests are also reset when UE = 0 so the DMA channel must
  *         be disabled before resetting the UE bit.
  * @rmtoll CR1          UE            LL_LPUART_Disable
  * @param  LPUARTx LPUART Instance
  * @retval None
  */
__STATIC_INLINE void LL_LPUART_Disable(USART_TypeDef *LPUARTx)
{
  CLEAR_BIT(LPUARTx->CR1, USART_CR1_UE);
}

/**
  * @brief  Indicate if LPUART is enabled
  * @rmtoll CR1          UE            LL_LPUART_IsEnabled
  * @param  LPUARTx LPUART Instance
  * @retval State of bit (1 or 0).
  */
__STATIC_INLINE uint32_t LL_LPUART_IsEnabled(USART_TypeDef *LPUARTx)
{
  return ((READ_BIT(LPUARTx->CR1, USART_CR1_UE) == (USART_CR1_UE)) ? 1UL : 0UL);
}

#if defined(USART_CR1_FIFOEN)
/**
  * @brief  FIFO Mode Enable
  * @rmtoll CR1          FIFOEN        LL_LPUART_EnableFIFO
  * @param  LPUARTx LPUART Instance
  * @retval None
  */
__STATIC_INLINE void LL_LPUART_EnableFIFO(USART_TypeDef *LPUARTx)
{
  SET_BIT(LPUARTx->CR1, USART_CR1_FIFOEN);
}

/**
  * @brief  FIFO Mode Disable
  * @rmtoll CR1          FIFOEN        LL_LPUART_DisableFIFO
  * @param  LPUARTx LPUART Instance
  * @retval None
  */
__STATIC_INLINE void LL_LPUART_DisableFIFO(USART_TypeDef *LPUARTx)
{
  CLEAR_BIT(LPUARTx->CR1, USART_CR1_FIFOEN);
}

/**
  * @brief  Indicate if FIFO Mode is enabled
  * @rmtoll CR1          FIFOEN        LL_LPUART_IsEnabledFIFO
  * @param  LPUARTx LPUART Instance
  * @retval State of bit (1 or 0).
  */
__STATIC_INLINE uint32_t LL_LPUART_IsEnabledFIFO(USART_TypeDef *LPUARTx)
{
  return ((READ_BIT(LPUARTx->CR1, USART_CR1_FIFOEN) == (USART_CR1_FIFOEN)) ? 1UL : 0UL);
}

/**
  * @brief  Configure TX FIFO Threshold
  * @rmtoll CR3          TXFTCFG       LL_LPUART_SetTXFIFOThreshold
  * @param  LPUARTx LPUART Instance
  * @param  Threshold This parameter can be one of the following values:
  *         @arg @ref LL_LPUART_FIFOTHRESHOLD_1_8
  *         @arg @ref LL_LPUART_FIFOTHRESHOLD_1_4
  *         @arg @ref LL_LPUART_FIFOTHRESHOLD_1_2
  *         @arg @ref LL_LPUART_FIFOTHRESHOLD_3_4
  *         @arg @ref LL_LPUART_FIFOTHRESHOLD_7_8
  *         @arg @ref LL_LPUART_FIFOTHRESHOLD_8_8
  * @retval None
  */
__STATIC_INLINE void LL_LPUART_SetTXFIFOThreshold(USART_TypeDef *LPUARTx, uint32_t Threshold)
{
  MODIFY_REG(LPUARTx->CR3, USART_CR3_TXFTCFG, Threshold << USART_CR3_TXFTCFG_Pos);
}

/**
  * @brief  Return TX FIFO Threshold Configuration
  * @rmtoll CR3          TXFTCFG       LL_LPUART_GetTXFIFOThreshold
  * @param  LPUARTx LPUART Instance
  * @retval Returned value can be one of the following values:
  *         @arg @ref LL_LPUART_FIFOTHRESHOLD_1_8
  *         @arg @ref LL_LPUART_FIFOTHRESHOLD_1_4
  *         @arg @ref LL_LPUART_FIFOTHRESHOLD_1_2
  *         @arg @ref LL_LPUART_FIFOTHRESHOLD_3_4
  *         @arg @ref LL_LPUART_FIFOTHRESHOLD_7_8
  *         @arg @ref LL_LPUART_FIFOTHRESHOLD_8_8
  */
__STATIC_INLINE uint32_t LL_LPUART_GetTXFIFOThreshold(USART_TypeDef *LPUARTx)
{
  return (uint32_t)(READ_BIT(LPUARTx->CR3, USART_CR3_TXFTCFG) >> USART_CR3_TXFTCFG_Pos);
}

/**
  * @brief  Configure RX FIFO Threshold
  * @rmtoll CR3          RXFTCFG       LL_LPUART_SetRXFIFOThreshold
  * @param  LPUARTx LPUART Instance
  * @param  Threshold This parameter can be one of the following values:
  *         @arg @ref LL_LPUART_FIFOTHRESHOLD_1_8
  *         @arg @ref LL_LPUART_FIFOTHRESHOLD_1_4
  *         @arg @ref LL_LPUART_FIFOTHRESHOLD_1_2
  *         @arg @ref LL_LPUART_FIFOTHRESHOLD_3_4
  *         @arg @ref LL_LPUART_FIFOTHRESHOLD_7_8
  *         @arg @ref LL_LPUART_FIFOTHRESHOLD_8_8
  * @retval None
  */
__STATIC_INLINE void LL_LPUART_SetRXFIFOThreshold(USART_TypeDef *LPUARTx, uint32_t Threshold)
{
  MODIFY_REG(LPUARTx->CR3, USART_CR3_RXFTCFG, Threshold << USART_CR3_RXFTCFG_Pos);
}

/**
  * @brief  Return RX FIFO Threshold Configuration
  * @rmtoll CR3          RXFTCFG       LL_LPUART_GetRXFIFOThreshold
  * @param  LPUARTx LPUART Instance
  * @retval Returned value can be one of the following values:
  *         @arg @ref LL_LPUART_FIFOTHRESHOLD_1_8
  *         @arg @ref LL_LPUART_FIFOTHRESHOLD_1_4
  *         @arg @ref LL_LPUART_FIFOTHRESHOLD_1_2
  *         @arg @ref LL_LPUART_FIFOTHRESHOLD_3_4
  *         @arg @ref LL_LPUART_FIFOTHRESHOLD_7_8
  *         @arg @ref LL_LPUART_FIFOTHRESHOLD_8_8
  */
__STATIC_INLINE uint32_t LL_LPUART_GetRXFIFOThreshold(USART_TypeDef *LPUARTx)
{
  return (uint32_t)(READ_BIT(LPUARTx->CR3, USART_CR3_RXFTCFG) >> USART_CR3_RXFTCFG_Pos);
}

/**
  * @brief  Configure TX and RX FIFOs Threshold
  * @rmtoll CR3          TXFTCFG       LL_LPUART_ConfigFIFOsThreshold\n
  *         CR3          RXFTCFG       LL_LPUART_ConfigFIFOsThreshold
  * @param  LPUARTx LPUART Instance
  * @param  TXThreshold This parameter can be one of the following values:
  *         @arg @ref LL_LPUART_FIFOTHRESHOLD_1_8
  *         @arg @ref LL_LPUART_FIFOTHRESHOLD_1_4
  *         @arg @ref LL_LPUART_FIFOTHRESHOLD_1_2
  *         @arg @ref LL_LPUART_FIFOTHRESHOLD_3_4
  *         @arg @ref LL_LPUART_FIFOTHRESHOLD_7_8
  *         @arg @ref LL_LPUART_FIFOTHRESHOLD_8_8
  * @param  RXThreshold This parameter can be one of the following values:
  *         @arg @ref LL_LPUART_FIFOTHRESHOLD_1_8
  *         @arg @ref LL_LPUART_FIFOTHRESHOLD_1_4
  *         @arg @ref LL_LPUART_FIFOTHRESHOLD_1_2
  *         @arg @ref LL_LPUART_FIFOTHRESHOLD_3_4
  *         @arg @ref LL_LPUART_FIFOTHRESHOLD_7_8
  *         @arg @ref LL_LPUART_FIFOTHRESHOLD_8_8
  * @retval None
  */
__STATIC_INLINE void LL_LPUART_ConfigFIFOsThreshold(USART_TypeDef *LPUARTx, uint32_t TXThreshold, uint32_t RXThreshold)
{
  MODIFY_REG(LPUARTx->CR3, USART_CR3_TXFTCFG | USART_CR3_RXFTCFG, (TXThreshold << USART_CR3_TXFTCFG_Pos) | (RXThreshold << USART_CR3_RXFTCFG_Pos));
}
#endif /* USART_CR1_FIFOEN */

/**
  * @brief  LPUART enabled in STOP Mode
  * @note   When this function is enabled, LPUART is able to wake up the MCU from Stop mode, provided that
  *         LPUART clock selection is HSI or LSE in RCC.
  * @rmtoll CR1          UESM          LL_LPUART_EnableInStopMode
  * @param  LPUARTx LPUART Instance
  * @retval None
  */
__STATIC_INLINE void LL_LPUART_EnableInStopMode(USART_TypeDef *LPUARTx)
{
  SET_BIT(LPUARTx->CR1, USART_CR1_UESM);
}

/**
  * @brief  LPUART disabled in STOP Mode
  * @note   When this function is disabled, LPUART is not able to wake up the MCU from Stop mode
  * @rmtoll CR1          UESM          LL_LPUART_DisableInStopMode
  * @param  LPUARTx LPUART Instance
  * @retval None
  */
__STATIC_INLINE void LL_LPUART_DisableInStopMode(USART_TypeDef *LPUARTx)
{
  CLEAR_BIT(LPUARTx->CR1, USART_CR1_UESM);
}

/**
  * @brief  Indicate if LPUART is enabled in STOP Mode
  *         (able to wake up MCU from Stop mode or not)
  * @rmtoll CR1          UESM          LL_LPUART_IsEnabledInStopMode
  * @param  LPUARTx LPUART Instance
  * @retval State of bit (1 or 0).
  */
__STATIC_INLINE uint32_t LL_LPUART_IsEnabledInStopMode(USART_TypeDef *LPUARTx)
{
  return ((READ_BIT(LPUARTx->CR1, USART_CR1_UESM) == (USART_CR1_UESM)) ? 1UL : 0UL);
}

#if defined(USART_CR3_UCESM)
/**
  * @brief  LPUART Clock enabled in STOP Mode
  * @note   When this function is called, LPUART Clock is enabled while in STOP mode
  * @rmtoll CR3          UCESM         LL_LPUART_EnableClockInStopMode
  * @param  LPUARTx LPUART Instance
  * @retval None
  */
__STATIC_INLINE void LL_LPUART_EnableClockInStopMode(USART_TypeDef *LPUARTx)
{
  SET_BIT(LPUARTx->CR3, USART_CR3_UCESM);
}

/**
  * @brief  LPUART clock disabled in STOP Mode
  * @note   When this function is called, LPUART Clock is disabled while in STOP mode
  * @rmtoll CR3          UCESM         LL_LPUART_DisableClockInStopMode
  * @param  LPUARTx LPUART Instance
  * @retval None
  */
__STATIC_INLINE void LL_LPUART_DisableClockInStopMode(USART_TypeDef *LPUARTx)
{
  CLEAR_BIT(LPUARTx->CR3, USART_CR3_UCESM);
}

/**
  * @brief  Indicate if LPUART clock is enabled in STOP Mode
  * @rmtoll CR3          UCESM         LL_LPUART_IsClockEnabledInStopMode
  * @param  LPUARTx LPUART Instance
  * @retval State of bit (1 or 0).
  */
__STATIC_INLINE uint32_t LL_LPUART_IsClockEnabledInStopMode(USART_TypeDef *LPUARTx)
{
  return ((READ_BIT(LPUARTx->CR3, USART_CR3_UCESM) == (USART_CR3_UCESM)) ? 1UL : 0UL);
}

#endif /* USART_CR3_UCESM */
/**
  * @brief  Receiver Enable (Receiver is enabled and begins searching for a start bit)
  * @rmtoll CR1          RE            LL_LPUART_EnableDirectionRx
  * @param  LPUARTx LPUART Instance
  * @retval None
  */
__STATIC_INLINE void LL_LPUART_EnableDirectionRx(USART_TypeDef *LPUARTx)
{
  SET_BIT(LPUARTx->CR1, USART_CR1_RE);
}

/**
  * @brief  Receiver Disable
  * @rmtoll CR1          RE            LL_LPUART_DisableDirectionRx
  * @param  LPUARTx LPUART Instance
  * @retval None
  */
__STATIC_INLINE void LL_LPUART_DisableDirectionRx(USART_TypeDef *LPUARTx)
{
  CLEAR_BIT(LPUARTx->CR1, USART_CR1_RE);
}

/**
  * @brief  Transmitter Enable
  * @rmtoll CR1          TE            LL_LPUART_EnableDirectionTx
  * @param  LPUARTx LPUART Instance
  * @retval None
  */
__STATIC_INLINE void LL_LPUART_EnableDirectionTx(USART_TypeDef *LPUARTx)
{
  SET_BIT(LPUARTx->CR1, USART_CR1_TE);
}

/**
  * @brief  Transmitter Disable
  * @rmtoll CR1          TE            LL_LPUART_DisableDirectionTx
  * @param  LPUARTx LPUART Instance
  * @retval None
  */
__STATIC_INLINE void LL_LPUART_DisableDirectionTx(USART_TypeDef *LPUARTx)
{
  CLEAR_BIT(LPUARTx->CR1, USART_CR1_TE);
}

/**
  * @brief  Configure simultaneously enabled/disabled states
  *         of Transmitter and Receiver
  * @rmtoll CR1          RE            LL_LPUART_SetTransferDirection\n
  *         CR1          TE            LL_LPUART_SetTransferDirection
  * @param  LPUARTx LPUART Instance
  * @param  TransferDirection This parameter can be one of the following values:
  *         @arg @ref LL_LPUART_DIRECTION_NONE
  *         @arg @ref LL_LPUART_DIRECTION_RX
  *         @arg @ref LL_LPUART_DIRECTION_TX
  *         @arg @ref LL_LPUART_DIRECTION_TX_RX
  * @retval None
  */
__STATIC_INLINE void LL_LPUART_SetTransferDirection(USART_TypeDef *LPUARTx, uint32_t TransferDirection)
{
  MODIFY_REG(LPUARTx->CR1, USART_CR1_RE | USART_CR1_TE, TransferDirection);
}

/**
  * @brief  Return enabled/disabled states of Transmitter and Receiver
  * @rmtoll CR1          RE            LL_LPUART_GetTransferDirection\n
  *         CR1          TE            LL_LPUART_GetTransferDirection
  * @param  LPUARTx LPUART Instance
  * @retval Returned value can be one of the following values:
  *         @arg @ref LL_LPUART_DIRECTION_NONE
  *         @arg @ref LL_LPUART_DIRECTION_RX
  *         @arg @ref LL_LPUART_DIRECTION_TX
  *         @arg @ref LL_LPUART_DIRECTION_TX_RX
  */
__STATIC_INLINE uint32_t LL_LPUART_GetTransferDirection(USART_TypeDef *LPUARTx)
{
  return (uint32_t)(READ_BIT(LPUARTx->CR1, USART_CR1_RE | USART_CR1_TE));
}

/**
  * @brief  Configure Parity (enabled/disabled and parity mode if enabled)
  * @note   This function selects if hardware parity control (generation and detection) is enabled or disabled.
  *         When the parity control is enabled (Odd or Even), computed parity bit is inserted at the MSB position
  *         (depending on data width) and parity is checked on the received data.
  * @rmtoll CR1          PS            LL_LPUART_SetParity\n
  *         CR1          PCE           LL_LPUART_SetParity
  * @param  LPUARTx LPUART Instance
  * @param  Parity This parameter can be one of the following values:
  *         @arg @ref LL_LPUART_PARITY_NONE
  *         @arg @ref LL_LPUART_PARITY_EVEN
  *         @arg @ref LL_LPUART_PARITY_ODD
  * @retval None
  */
__STATIC_INLINE void LL_LPUART_SetParity(USART_TypeDef *LPUARTx, uint32_t Parity)
{
  MODIFY_REG(LPUARTx->CR1, USART_CR1_PS | USART_CR1_PCE, Parity);
}

/**
  * @brief  Return Parity configuration (enabled/disabled and parity mode if enabled)
  * @rmtoll CR1          PS            LL_LPUART_GetParity\n
  *         CR1          PCE           LL_LPUART_GetParity
  * @param  LPUARTx LPUART Instance
  * @retval Returned value can be one of the following values:
  *         @arg @ref LL_LPUART_PARITY_NONE
  *         @arg @ref LL_LPUART_PARITY_EVEN
  *         @arg @ref LL_LPUART_PARITY_ODD
  */
__STATIC_INLINE uint32_t LL_LPUART_GetParity(USART_TypeDef *LPUARTx)
{
  return (uint32_t)(READ_BIT(LPUARTx->CR1, USART_CR1_PS | USART_CR1_PCE));
}

/**
  * @brief  Set Receiver Wake Up method from Mute mode.
  * @rmtoll CR1          WAKE          LL_LPUART_SetWakeUpMethod
  * @param  LPUARTx LPUART Instance
  * @param  Method This parameter can be one of the following values:
  *         @arg @ref LL_LPUART_WAKEUP_IDLELINE
  *         @arg @ref LL_LPUART_WAKEUP_ADDRESSMARK
  * @retval None
  */
__STATIC_INLINE void LL_LPUART_SetWakeUpMethod(USART_TypeDef *LPUARTx, uint32_t Method)
{
  MODIFY_REG(LPUARTx->CR1, USART_CR1_WAKE, Method);
}

/**
  * @brief  Return Receiver Wake Up method from Mute mode
  * @rmtoll CR1          WAKE          LL_LPUART_GetWakeUpMethod
  * @param  LPUARTx LPUART Instance
  * @retval Returned value can be one of the following values:
  *         @arg @ref LL_LPUART_WAKEUP_IDLELINE
  *         @arg @ref LL_LPUART_WAKEUP_ADDRESSMARK
  */
__STATIC_INLINE uint32_t LL_LPUART_GetWakeUpMethod(USART_TypeDef *LPUARTx)
{
  return (uint32_t)(READ_BIT(LPUARTx->CR1, USART_CR1_WAKE));
}

/**
  * @brief  Set Word length (nb of data bits, excluding start and stop bits)
  * @rmtoll CR1          M             LL_LPUART_SetDataWidth
  * @param  LPUARTx LPUART Instance
  * @param  DataWidth This parameter can be one of the following values:
  *         @arg @ref LL_LPUART_DATAWIDTH_7B
  *         @arg @ref LL_LPUART_DATAWIDTH_8B
  *         @arg @ref LL_LPUART_DATAWIDTH_9B
  * @retval None
  */
__STATIC_INLINE void LL_LPUART_SetDataWidth(USART_TypeDef *LPUARTx, uint32_t DataWidth)
{
  MODIFY_REG(LPUARTx->CR1, USART_CR1_M, DataWidth);
}

/**
  * @brief  Return Word length (i.e. nb of data bits, excluding start and stop bits)
  * @rmtoll CR1          M             LL_LPUART_GetDataWidth
  * @param  LPUARTx LPUART Instance
  * @retval Returned value can be one of the following values:
  *         @arg @ref LL_LPUART_DATAWIDTH_7B
  *         @arg @ref LL_LPUART_DATAWIDTH_8B
  *         @arg @ref LL_LPUART_DATAWIDTH_9B
  */
__STATIC_INLINE uint32_t LL_LPUART_GetDataWidth(USART_TypeDef *LPUARTx)
{
  return (uint32_t)(READ_BIT(LPUARTx->CR1, USART_CR1_M));
}

/**
  * @brief  Allow switch between Mute Mode and Active mode
  * @rmtoll CR1          MME           LL_LPUART_EnableMuteMode
  * @param  LPUARTx LPUART Instance
  * @retval None
  */
__STATIC_INLINE void LL_LPUART_EnableMuteMode(USART_TypeDef *LPUARTx)
{
  SET_BIT(LPUARTx->CR1, USART_CR1_MME);
}

/**
  * @brief  Prevent Mute Mode use. Set Receiver in active mode permanently.
  * @rmtoll CR1          MME           LL_LPUART_DisableMuteMode
  * @param  LPUARTx LPUART Instance
  * @retval None
  */
__STATIC_INLINE void LL_LPUART_DisableMuteMode(USART_TypeDef *LPUARTx)
{
  CLEAR_BIT(LPUARTx->CR1, USART_CR1_MME);
}

/**
  * @brief  Indicate if switch between Mute Mode and Active mode is allowed
  * @rmtoll CR1          MME           LL_LPUART_IsEnabledMuteMode
  * @param  LPUARTx LPUART Instance
  * @retval State of bit (1 or 0).
  */
__STATIC_INLINE uint32_t LL_LPUART_IsEnabledMuteMode(USART_TypeDef *LPUARTx)
{
  return ((READ_BIT(LPUARTx->CR1, USART_CR1_MME) == (USART_CR1_MME)) ? 1UL : 0UL);
}

#if defined(USART_PRESC_PRESCALER)
/**
  * @brief  Configure Clock source prescaler for baudrate generator and oversampling
  * @rmtoll PRESC        PRESCALER     LL_LPUART_SetPrescaler
  * @param  LPUARTx LPUART Instance
  * @param  PrescalerValue This parameter can be one of the following values:
  *         @arg @ref LL_LPUART_PRESCALER_DIV1
  *         @arg @ref LL_LPUART_PRESCALER_DIV2
  *         @arg @ref LL_LPUART_PRESCALER_DIV4
  *         @arg @ref LL_LPUART_PRESCALER_DIV6
  *         @arg @ref LL_LPUART_PRESCALER_DIV8
  *         @arg @ref LL_LPUART_PRESCALER_DIV10
  *         @arg @ref LL_LPUART_PRESCALER_DIV12
  *         @arg @ref LL_LPUART_PRESCALER_DIV16
  *         @arg @ref LL_LPUART_PRESCALER_DIV32
  *         @arg @ref LL_LPUART_PRESCALER_DIV64
  *         @arg @ref LL_LPUART_PRESCALER_DIV128
  *         @arg @ref LL_LPUART_PRESCALER_DIV256
  * @retval None
  */
__STATIC_INLINE void LL_LPUART_SetPrescaler(USART_TypeDef *LPUARTx, uint32_t PrescalerValue)
{
  MODIFY_REG(LPUARTx->PRESC, USART_PRESC_PRESCALER, (uint16_t)PrescalerValue);
}

/**
  * @brief  Retrieve the Clock source prescaler for baudrate generator and oversampling
  * @rmtoll PRESC        PRESCALER     LL_LPUART_GetPrescaler
  * @param  LPUARTx LPUART Instance
  * @retval Returned value can be one of the following values:
  *         @arg @ref LL_LPUART_PRESCALER_DIV1
  *         @arg @ref LL_LPUART_PRESCALER_DIV2
  *         @arg @ref LL_LPUART_PRESCALER_DIV4
  *         @arg @ref LL_LPUART_PRESCALER_DIV6
  *         @arg @ref LL_LPUART_PRESCALER_DIV8
  *         @arg @ref LL_LPUART_PRESCALER_DIV10
  *         @arg @ref LL_LPUART_PRESCALER_DIV12
  *         @arg @ref LL_LPUART_PRESCALER_DIV16
  *         @arg @ref LL_LPUART_PRESCALER_DIV32
  *         @arg @ref LL_LPUART_PRESCALER_DIV64
  *         @arg @ref LL_LPUART_PRESCALER_DIV128
  *         @arg @ref LL_LPUART_PRESCALER_DIV256
  */
__STATIC_INLINE uint32_t LL_LPUART_GetPrescaler(USART_TypeDef *LPUARTx)
{
  return (uint32_t)(READ_BIT(LPUARTx->PRESC, USART_PRESC_PRESCALER));
}
#endif /* USART_PRESC_PRESCALER */

/**
  * @brief  Set the length of the stop bits
  * @rmtoll CR2          STOP          LL_LPUART_SetStopBitsLength
  * @param  LPUARTx LPUART Instance
  * @param  StopBits This parameter can be one of the following values:
  *         @arg @ref LL_LPUART_STOPBITS_1
  *         @arg @ref LL_LPUART_STOPBITS_2
  * @retval None
  */
__STATIC_INLINE void LL_LPUART_SetStopBitsLength(USART_TypeDef *LPUARTx, uint32_t StopBits)
{
  MODIFY_REG(LPUARTx->CR2, USART_CR2_STOP, StopBits);
}

/**
  * @brief  Retrieve the length of the stop bits
  * @rmtoll CR2          STOP          LL_LPUART_GetStopBitsLength
  * @param  LPUARTx LPUART Instance
  * @retval Returned value can be one of the following values:
  *         @arg @ref LL_LPUART_STOPBITS_1
  *         @arg @ref LL_LPUART_STOPBITS_2
  */
__STATIC_INLINE uint32_t LL_LPUART_GetStopBitsLength(USART_TypeDef *LPUARTx)
{
  return (uint32_t)(READ_BIT(LPUARTx->CR2, USART_CR2_STOP));
}

/**
  * @brief  Configure Character frame format (Datawidth, Parity control, Stop Bits)
  * @note   Call of this function is equivalent to following function call sequence :
  *         - Data Width configuration using @ref LL_LPUART_SetDataWidth() function
  *         - Parity Control and mode configuration using @ref LL_LPUART_SetParity() function
  *         - Stop bits configuration using @ref LL_LPUART_SetStopBitsLength() function
  * @rmtoll CR1          PS            LL_LPUART_ConfigCharacter\n
  *         CR1          PCE           LL_LPUART_ConfigCharacter\n
  *         CR1          M             LL_LPUART_ConfigCharacter\n
  *         CR2          STOP          LL_LPUART_ConfigCharacter
  * @param  LPUARTx LPUART Instance
  * @param  DataWidth This parameter can be one of the following values:
  *         @arg @ref LL_LPUART_DATAWIDTH_7B
  *         @arg @ref LL_LPUART_DATAWIDTH_8B
  *         @arg @ref LL_LPUART_DATAWIDTH_9B
  * @param  Parity This parameter can be one of the following values:
  *         @arg @ref LL_LPUART_PARITY_NONE
  *         @arg @ref LL_LPUART_PARITY_EVEN
  *         @arg @ref LL_LPUART_PARITY_ODD
  * @param  StopBits This parameter can be one of the following values:
  *         @arg @ref LL_LPUART_STOPBITS_1
  *         @arg @ref LL_LPUART_STOPBITS_2
  * @retval None
  */
__STATIC_INLINE void LL_LPUART_ConfigCharacter(USART_TypeDef *LPUARTx, uint32_t DataWidth, uint32_t Parity,
                                               uint32_t StopBits)
{
  MODIFY_REG(LPUARTx->CR1, USART_CR1_PS | USART_CR1_PCE | USART_CR1_M, Parity | DataWidth);
  MODIFY_REG(LPUARTx->CR2, USART_CR2_STOP, StopBits);
}

/**
  * @brief  Configure TX/RX pins swapping setting.
  * @rmtoll CR2          SWAP          LL_LPUART_SetTXRXSwap
  * @param  LPUARTx LPUART Instance
  * @param  SwapConfig This parameter can be one of the following values:
  *         @arg @ref LL_LPUART_TXRX_STANDARD
  *         @arg @ref LL_LPUART_TXRX_SWAPPED
  * @retval None
  */
__STATIC_INLINE void LL_LPUART_SetTXRXSwap(USART_TypeDef *LPUARTx, uint32_t SwapConfig)
{
  MODIFY_REG(LPUARTx->CR2, USART_CR2_SWAP, SwapConfig);
}

/**
  * @brief  Retrieve TX/RX pins swapping configuration.
  * @rmtoll CR2          SWAP          LL_LPUART_GetTXRXSwap
  * @param  LPUARTx LPUART Instance
  * @retval Returned value can be one of the following values:
  *         @arg @ref LL_LPUART_TXRX_STANDARD
  *         @arg @ref LL_LPUART_TXRX_SWAPPED
  */
__STATIC_INLINE uint32_t LL_LPUART_GetTXRXSwap(USART_TypeDef *LPUARTx)
{
  return (uint32_t)(READ_BIT(LPUARTx->CR2, USART_CR2_SWAP));
}

/**
  * @brief  Configure RX pin active level logic
  * @rmtoll CR2          RXINV         LL_LPUART_SetRXPinLevel
  * @param  LPUARTx LPUART Instance
  * @param  PinInvMethod This parameter can be one of the following values:
  *         @arg @ref LL_LPUART_RXPIN_LEVEL_STANDARD
  *         @arg @ref LL_LPUART_RXPIN_LEVEL_INVERTED
  * @retval None
  */
__STATIC_INLINE void LL_LPUART_SetRXPinLevel(USART_TypeDef *LPUARTx, uint32_t PinInvMethod)
{
  MODIFY_REG(LPUARTx->CR2, USART_CR2_RXINV, PinInvMethod);
}

/**
  * @brief  Retrieve RX pin active level logic configuration
  * @rmtoll CR2          RXINV         LL_LPUART_GetRXPinLevel
  * @param  LPUARTx LPUART Instance
  * @retval Returned value can be one of the following values:
  *         @arg @ref LL_LPUART_RXPIN_LEVEL_STANDARD
  *         @arg @ref LL_LPUART_RXPIN_LEVEL_INVERTED
  */
__STATIC_INLINE uint32_t LL_LPUART_GetRXPinLevel(USART_TypeDef *LPUARTx)
{
  return (uint32_t)(READ_BIT(LPUARTx->CR2, USART_CR2_RXINV));
}

/**
  * @brief  Configure TX pin active level logic
  * @rmtoll CR2          TXINV         LL_LPUART_SetTXPinLevel
  * @param  LPUARTx LPUART Instance
  * @param  PinInvMethod This parameter can be one of the following values:
  *         @arg @ref LL_LPUART_TXPIN_LEVEL_STANDARD
  *         @arg @ref LL_LPUART_TXPIN_LEVEL_INVERTED
  * @retval None
  */
__STATIC_INLINE void LL_LPUART_SetTXPinLevel(USART_TypeDef *LPUARTx, uint32_t PinInvMethod)
{
  MODIFY_REG(LPUARTx->CR2, USART_CR2_TXINV, PinInvMethod);
}

/**
  * @brief  Retrieve TX pin active level logic configuration
  * @rmtoll CR2          TXINV         LL_LPUART_GetTXPinLevel
  * @param  LPUARTx LPUART Instance
  * @retval Returned value can be one of the following values:
  *         @arg @ref LL_LPUART_TXPIN_LEVEL_STANDARD
  *         @arg @ref LL_LPUART_TXPIN_LEVEL_INVERTED
  */
__STATIC_INLINE uint32_t LL_LPUART_GetTXPinLevel(USART_TypeDef *LPUARTx)
{
  return (uint32_t)(READ_BIT(LPUARTx->CR2, USART_CR2_TXINV));
}

/**
  * @brief  Configure Binary data logic.
  *
  * @note   Allow to define how Logical data from the data register are send/received :
  *         either in positive/direct logic (1=H, 0=L) or in negative/inverse logic (1=L, 0=H)
  * @rmtoll CR2          DATAINV       LL_LPUART_SetBinaryDataLogic
  * @param  LPUARTx LPUART Instance
  * @param  DataLogic This parameter can be one of the following values:
  *         @arg @ref LL_LPUART_BINARY_LOGIC_POSITIVE
  *         @arg @ref LL_LPUART_BINARY_LOGIC_NEGATIVE
  * @retval None
  */
__STATIC_INLINE void LL_LPUART_SetBinaryDataLogic(USART_TypeDef *LPUARTx, uint32_t DataLogic)
{
  MODIFY_REG(LPUARTx->CR2, USART_CR2_DATAINV, DataLogic);
}

/**
  * @brief  Retrieve Binary data configuration
  * @rmtoll CR2          DATAINV       LL_LPUART_GetBinaryDataLogic
  * @param  LPUARTx LPUART Instance
  * @retval Returned value can be one of the following values:
  *         @arg @ref LL_LPUART_BINARY_LOGIC_POSITIVE
  *         @arg @ref LL_LPUART_BINARY_LOGIC_NEGATIVE
  */
__STATIC_INLINE uint32_t LL_LPUART_GetBinaryDataLogic(USART_TypeDef *LPUARTx)
{
  return (uint32_t)(READ_BIT(LPUARTx->CR2, USART_CR2_DATAINV));
}

/**
  * @brief  Configure transfer bit order (either Less or Most Significant Bit First)
  * @note   MSB First means data is transmitted/received with the MSB first, following the start bit.
  *         LSB First means data is transmitted/received with data bit 0 first, following the start bit.
  * @rmtoll CR2          MSBFIRST      LL_LPUART_SetTransferBitOrder
  * @param  LPUARTx LPUART Instance
  * @param  BitOrder This parameter can be one of the following values:
  *         @arg @ref LL_LPUART_BITORDER_LSBFIRST
  *         @arg @ref LL_LPUART_BITORDER_MSBFIRST
  * @retval None
  */
__STATIC_INLINE void LL_LPUART_SetTransferBitOrder(USART_TypeDef *LPUARTx, uint32_t BitOrder)
{
  MODIFY_REG(LPUARTx->CR2, USART_CR2_MSBFIRST, BitOrder);
}

/**
  * @brief  Return transfer bit order (either Less or Most Significant Bit First)
  * @note   MSB First means data is transmitted/received with the MSB first, following the start bit.
  *         LSB First means data is transmitted/received with data bit 0 first, following the start bit.
  * @rmtoll CR2          MSBFIRST      LL_LPUART_GetTransferBitOrder
  * @param  LPUARTx LPUART Instance
  * @retval Returned value can be one of the following values:
  *         @arg @ref LL_LPUART_BITORDER_LSBFIRST
  *         @arg @ref LL_LPUART_BITORDER_MSBFIRST
  */
__STATIC_INLINE uint32_t LL_LPUART_GetTransferBitOrder(USART_TypeDef *LPUARTx)
{
  return (uint32_t)(READ_BIT(LPUARTx->CR2, USART_CR2_MSBFIRST));
}

/**
  * @brief  Set Address of the LPUART node.
  * @note   This is used in multiprocessor communication during Mute mode or Stop mode,
  *         for wake up with address mark detection.
  * @note   4bits address node is used when 4-bit Address Detection is selected in ADDM7.
  *         (b7-b4 should be set to 0)
  *         8bits address node is used when 7-bit Address Detection is selected in ADDM7.
  *         (This is used in multiprocessor communication during Mute mode or Stop mode,
  *         for wake up with 7-bit address mark detection.
  *         The MSB of the character sent by the transmitter should be equal to 1.
  *         It may also be used for character detection during normal reception,
  *         Mute mode inactive (for example, end of block detection in ModBus protocol).
  *         In this case, the whole received character (8-bit) is compared to the ADD[7:0]
  *         value and CMF flag is set on match)
  * @rmtoll CR2          ADD           LL_LPUART_ConfigNodeAddress\n
  *         CR2          ADDM7         LL_LPUART_ConfigNodeAddress
  * @param  LPUARTx LPUART Instance
  * @param  AddressLen This parameter can be one of the following values:
  *         @arg @ref LL_LPUART_ADDRESS_DETECT_4B
  *         @arg @ref LL_LPUART_ADDRESS_DETECT_7B
  * @param  NodeAddress 4 or 7 bit Address of the LPUART node.
  * @retval None
  */
__STATIC_INLINE void LL_LPUART_ConfigNodeAddress(USART_TypeDef *LPUARTx, uint32_t AddressLen, uint32_t NodeAddress)
{
  MODIFY_REG(LPUARTx->CR2, USART_CR2_ADD | USART_CR2_ADDM7,
             (uint32_t)(AddressLen | (NodeAddress << USART_CR2_ADD_Pos)));
}

/**
  * @brief  Return 8 bit Address of the LPUART node as set in ADD field of CR2.
  * @note   If 4-bit Address Detection is selected in ADDM7,
  *         only 4bits (b3-b0) of returned value are relevant (b31-b4 are not relevant)
  *         If 7-bit Address Detection is selected in ADDM7,
  *         only 8bits (b7-b0) of returned value are relevant (b31-b8 are not relevant)
  * @rmtoll CR2          ADD           LL_LPUART_GetNodeAddress
  * @param  LPUARTx LPUART Instance
  * @retval Address of the LPUART node (Value between Min_Data=0 and Max_Data=255)
  */
__STATIC_INLINE uint32_t LL_LPUART_GetNodeAddress(USART_TypeDef *LPUARTx)
{
  return (uint32_t)(READ_BIT(LPUARTx->CR2, USART_CR2_ADD) >> USART_CR2_ADD_Pos);
}

/**
  * @brief  Return Length of Node Address used in Address Detection mode (7-bit or 4-bit)
  * @rmtoll CR2          ADDM7         LL_LPUART_GetNodeAddressLen
  * @param  LPUARTx LPUART Instance
  * @retval Returned value can be one of the following values:
  *         @arg @ref LL_LPUART_ADDRESS_DETECT_4B
  *         @arg @ref LL_LPUART_ADDRESS_DETECT_7B
  */
__STATIC_INLINE uint32_t LL_LPUART_GetNodeAddressLen(USART_TypeDef *LPUARTx)
{
  return (uint32_t)(READ_BIT(LPUARTx->CR2, USART_CR2_ADDM7));
}

/**
  * @brief  Enable RTS HW Flow Control
  * @rmtoll CR3          RTSE          LL_LPUART_EnableRTSHWFlowCtrl
  * @param  LPUARTx LPUART Instance
  * @retval None
  */
__STATIC_INLINE void LL_LPUART_EnableRTSHWFlowCtrl(USART_TypeDef *LPUARTx)
{
  SET_BIT(LPUARTx->CR3, USART_CR3_RTSE);
}

/**
  * @brief  Disable RTS HW Flow Control
  * @rmtoll CR3          RTSE          LL_LPUART_DisableRTSHWFlowCtrl
  * @param  LPUARTx LPUART Instance
  * @retval None
  */
__STATIC_INLINE void LL_LPUART_DisableRTSHWFlowCtrl(USART_TypeDef *LPUARTx)
{
  CLEAR_BIT(LPUARTx->CR3, USART_CR3_RTSE);
}

/**
  * @brief  Enable CTS HW Flow Control
  * @rmtoll CR3          CTSE          LL_LPUART_EnableCTSHWFlowCtrl
  * @param  LPUARTx LPUART Instance
  * @retval None
  */
__STATIC_INLINE void LL_LPUART_EnableCTSHWFlowCtrl(USART_TypeDef *LPUARTx)
{
  SET_BIT(LPUARTx->CR3, USART_CR3_CTSE);
}

/**
  * @brief  Disable CTS HW Flow Control
  * @rmtoll CR3          CTSE          LL_LPUART_DisableCTSHWFlowCtrl
  * @param  LPUARTx LPUART Instance
  * @retval None
  */
__STATIC_INLINE void LL_LPUART_DisableCTSHWFlowCtrl(USART_TypeDef *LPUARTx)
{
  CLEAR_BIT(LPUARTx->CR3, USART_CR3_CTSE);
}

/**
  * @brief  Configure HW Flow Control mode (both CTS and RTS)
  * @rmtoll CR3          RTSE          LL_LPUART_SetHWFlowCtrl\n
  *         CR3          CTSE          LL_LPUART_SetHWFlowCtrl
  * @param  LPUARTx LPUART Instance
  * @param  HardwareFlowControl This parameter can be one of the following values:
  *         @arg @ref LL_LPUART_HWCONTROL_NONE
  *         @arg @ref LL_LPUART_HWCONTROL_RTS
  *         @arg @ref LL_LPUART_HWCONTROL_CTS
  *         @arg @ref LL_LPUART_HWCONTROL_RTS_CTS
  * @retval None
  */
__STATIC_INLINE void LL_LPUART_SetHWFlowCtrl(USART_TypeDef *LPUARTx, uint32_t HardwareFlowControl)
{
  MODIFY_REG(LPUARTx->CR3, USART_CR3_RTSE | USART_CR3_CTSE, HardwareFlowControl);
}

/**
  * @brief  Return HW Flow Control configuration (both CTS and RTS)
  * @rmtoll CR3          RTSE          LL_LPUART_GetHWFlowCtrl\n
  *         CR3          CTSE          LL_LPUART_GetHWFlowCtrl
  * @param  LPUARTx LPUART Instance
  * @retval Returned value can be one of the following values:
  *         @arg @ref LL_LPUART_HWCONTROL_NONE
  *         @arg @ref LL_LPUART_HWCONTROL_RTS
  *         @arg @ref LL_LPUART_HWCONTROL_CTS
  *         @arg @ref LL_LPUART_HWCONTROL_RTS_CTS
  */
__STATIC_INLINE uint32_t LL_LPUART_GetHWFlowCtrl(USART_TypeDef *LPUARTx)
{
  return (uint32_t)(READ_BIT(LPUARTx->CR3, USART_CR3_RTSE | USART_CR3_CTSE));
}

/**
  * @brief  Enable Overrun detection
  * @rmtoll CR3          OVRDIS        LL_LPUART_EnableOverrunDetect
  * @param  LPUARTx LPUART Instance
  * @retval None
  */
__STATIC_INLINE void LL_LPUART_EnableOverrunDetect(USART_TypeDef *LPUARTx)
{
  CLEAR_BIT(LPUARTx->CR3, USART_CR3_OVRDIS);
}

/**
  * @brief  Disable Overrun detection
  * @rmtoll CR3          OVRDIS        LL_LPUART_DisableOverrunDetect
  * @param  LPUARTx LPUART Instance
  * @retval None
  */
__STATIC_INLINE void LL_LPUART_DisableOverrunDetect(USART_TypeDef *LPUARTx)
{
  SET_BIT(LPUARTx->CR3, USART_CR3_OVRDIS);
}

/**
  * @brief  Indicate if Overrun detection is enabled
  * @rmtoll CR3          OVRDIS        LL_LPUART_IsEnabledOverrunDetect
  * @param  LPUARTx LPUART Instance
  * @retval State of bit (1 or 0).
  */
__STATIC_INLINE uint32_t LL_LPUART_IsEnabledOverrunDetect(USART_TypeDef *LPUARTx)
{
  return ((READ_BIT(LPUARTx->CR3, USART_CR3_OVRDIS) != USART_CR3_OVRDIS) ? 1UL : 0UL);
}

/**
  * @brief  Select event type for Wake UP Interrupt Flag (WUS[1:0] bits)
  * @rmtoll CR3          WUS           LL_LPUART_SetWKUPType
  * @param  LPUARTx LPUART Instance
  * @param  Type This parameter can be one of the following values:
  *         @arg @ref LL_LPUART_WAKEUP_ON_ADDRESS
  *         @arg @ref LL_LPUART_WAKEUP_ON_STARTBIT
  *         @arg @ref LL_LPUART_WAKEUP_ON_RXNE
  * @retval None
  */
__STATIC_INLINE void LL_LPUART_SetWKUPType(USART_TypeDef *LPUARTx, uint32_t Type)
{
  MODIFY_REG(LPUARTx->CR3, USART_CR3_WUS, Type);
}

/**
  * @brief  Return event type for Wake UP Interrupt Flag (WUS[1:0] bits)
  * @rmtoll CR3          WUS           LL_LPUART_GetWKUPType
  * @param  LPUARTx LPUART Instance
  * @retval Returned value can be one of the following values:
  *         @arg @ref LL_LPUART_WAKEUP_ON_ADDRESS
  *         @arg @ref LL_LPUART_WAKEUP_ON_STARTBIT
  *         @arg @ref LL_LPUART_WAKEUP_ON_RXNE
  */
__STATIC_INLINE uint32_t LL_LPUART_GetWKUPType(USART_TypeDef *LPUARTx)
{
  return (uint32_t)(READ_BIT(LPUARTx->CR3, USART_CR3_WUS));
}

/**
  * @brief  Configure LPUART BRR register for achieving expected Baud Rate value.
  *
  * @note   Compute and set LPUARTDIV value in BRR Register (full BRR content)
  *         according to used Peripheral Clock and expected Baud Rate values
  * @note   Peripheral clock and Baud Rate values provided as function parameters should be valid
  *         (Baud rate value != 0).
  * @note   Provided that LPUARTx_BRR must be > = 0x300 and LPUART_BRR is 20-bit,
  *         a care should be taken when generating high baud rates using high PeriphClk
  *         values. PeriphClk must be in the range [3 x BaudRate, 4096 x BaudRate].
  * @rmtoll BRR          BRR           LL_LPUART_SetBaudRate
  * @param  LPUARTx LPUART Instance
  * @param  PeriphClk Peripheral Clock
  @if USART_PRESC_PRESCALER
  * @param  PrescalerValue This parameter can be one of the following values:
  *         @arg @ref LL_LPUART_PRESCALER_DIV1
  *         @arg @ref LL_LPUART_PRESCALER_DIV2
  *         @arg @ref LL_LPUART_PRESCALER_DIV4
  *         @arg @ref LL_LPUART_PRESCALER_DIV6
  *         @arg @ref LL_LPUART_PRESCALER_DIV8
  *         @arg @ref LL_LPUART_PRESCALER_DIV10
  *         @arg @ref LL_LPUART_PRESCALER_DIV12
  *         @arg @ref LL_LPUART_PRESCALER_DIV16
  *         @arg @ref LL_LPUART_PRESCALER_DIV32
  *         @arg @ref LL_LPUART_PRESCALER_DIV64
  *         @arg @ref LL_LPUART_PRESCALER_DIV128
  *         @arg @ref LL_LPUART_PRESCALER_DIV256
  @endif
  * @param  BaudRate Baud Rate
  * @retval None
  */
#if defined(USART_PRESC_PRESCALER)
__STATIC_INLINE void LL_LPUART_SetBaudRate(USART_TypeDef *LPUARTx, uint32_t PeriphClk, uint32_t PrescalerValue,
                                           uint32_t BaudRate)
#else
__STATIC_INLINE void LL_LPUART_SetBaudRate(USART_TypeDef *LPUARTx, uint32_t PeriphClk, uint32_t BaudRate)
#endif /* USART_PRESC_PRESCALER */
{
#if defined(USART_PRESC_PRESCALER)
  LPUARTx->BRR = __LL_LPUART_DIV(PeriphClk, PrescalerValue, BaudRate);
#else
  LPUARTx->BRR = __LL_LPUART_DIV(PeriphClk, BaudRate);
#endif /* USART_PRESC_PRESCALER */
}

/**
  * @brief  Return current Baud Rate value, according to LPUARTDIV present in BRR register
  *         (full BRR content), and to used Peripheral Clock values
  * @note   In case of non-initialized or invalid value stored in BRR register, value 0 will be returned.
  * @rmtoll BRR          BRR           LL_LPUART_GetBaudRate
  * @param  LPUARTx LPUART Instance
  * @param  PeriphClk Peripheral Clock
  @if USART_PRESC_PRESCALER
  * @param  PrescalerValue This parameter can be one of the following values:
  *         @arg @ref LL_LPUART_PRESCALER_DIV1
  *         @arg @ref LL_LPUART_PRESCALER_DIV2
  *         @arg @ref LL_LPUART_PRESCALER_DIV4
  *         @arg @ref LL_LPUART_PRESCALER_DIV6
  *         @arg @ref LL_LPUART_PRESCALER_DIV8
  *         @arg @ref LL_LPUART_PRESCALER_DIV10
  *         @arg @ref LL_LPUART_PRESCALER_DIV12
  *         @arg @ref LL_LPUART_PRESCALER_DIV16
  *         @arg @ref LL_LPUART_PRESCALER_DIV32
  *         @arg @ref LL_LPUART_PRESCALER_DIV64
  *         @arg @ref LL_LPUART_PRESCALER_DIV128
  *         @arg @ref LL_LPUART_PRESCALER_DIV256
  @endif
  * @retval Baud Rate
  */
#if defined(USART_PRESC_PRESCALER)
__STATIC_INLINE uint32_t LL_LPUART_GetBaudRate(USART_TypeDef *LPUARTx, uint32_t PeriphClk, uint32_t PrescalerValue)
#else
__STATIC_INLINE uint32_t LL_LPUART_GetBaudRate(USART_TypeDef *LPUARTx, uint32_t PeriphClk)
#endif /* USART_PRESC_PRESCALER */
{
  register uint32_t lpuartdiv;
  register uint32_t brrresult;
#if defined(USART_PRESC_PRESCALER)
  register uint32_t periphclkpresc = (uint32_t)(PeriphClk / (LPUART_PRESCALER_TAB[(uint16_t)PrescalerValue]));
#endif /* USART_PRESC_PRESCALER */

  lpuartdiv = LPUARTx->BRR & LPUART_BRR_MASK;

  if (lpuartdiv >= LPUART_BRR_MIN_VALUE)
  {
#if defined(USART_PRESC_PRESCALER)
    brrresult = (uint32_t)(((uint64_t)(periphclkpresc) * LPUART_LPUARTDIV_FREQ_MUL) / lpuartdiv);
#else
    brrresult = (uint32_t)(((uint64_t)(PeriphClk) * LPUART_LPUARTDIV_FREQ_MUL) / lpuartdiv);
#endif /* USART_PRESC_PRESCALER */
  }
  else
  {
    brrresult = 0x0UL;
  }

  return (brrresult);
}

/**
  * @}
  */

/** @defgroup LPUART_LL_EF_Configuration_HalfDuplex Configuration functions related to Half Duplex feature
  * @{
  */

/**
  * @brief  Enable Single Wire Half-Duplex mode
  * @rmtoll CR3          HDSEL         LL_LPUART_EnableHalfDuplex
  * @param  LPUARTx LPUART Instance
  * @retval None
  */
__STATIC_INLINE void LL_LPUART_EnableHalfDuplex(USART_TypeDef *LPUARTx)
{
  SET_BIT(LPUARTx->CR3, USART_CR3_HDSEL);
}

/**
  * @brief  Disable Single Wire Half-Duplex mode
  * @rmtoll CR3          HDSEL         LL_LPUART_DisableHalfDuplex
  * @param  LPUARTx LPUART Instance
  * @retval None
  */
__STATIC_INLINE void LL_LPUART_DisableHalfDuplex(USART_TypeDef *LPUARTx)
{
  CLEAR_BIT(LPUARTx->CR3, USART_CR3_HDSEL);
}

/**
  * @brief  Indicate if Single Wire Half-Duplex mode is enabled
  * @rmtoll CR3          HDSEL         LL_LPUART_IsEnabledHalfDuplex
  * @param  LPUARTx LPUART Instance
  * @retval State of bit (1 or 0).
  */
__STATIC_INLINE uint32_t LL_LPUART_IsEnabledHalfDuplex(USART_TypeDef *LPUARTx)
{
  return ((READ_BIT(LPUARTx->CR3, USART_CR3_HDSEL) == (USART_CR3_HDSEL)) ? 1UL : 0UL);
}

/**
  * @}
  */

/** @defgroup LPUART_LL_EF_Configuration_DE Configuration functions related to Driver Enable feature
  * @{
  */

/**
  * @brief  Set DEDT (Driver Enable De-Assertion Time), Time value expressed on 5 bits ([4:0] bits).
  * @rmtoll CR1          DEDT          LL_LPUART_SetDEDeassertionTime
  * @param  LPUARTx LPUART Instance
  * @param  Time Value between Min_Data=0 and Max_Data=31
  * @retval None
  */
__STATIC_INLINE void LL_LPUART_SetDEDeassertionTime(USART_TypeDef *LPUARTx, uint32_t Time)
{
  MODIFY_REG(LPUARTx->CR1, USART_CR1_DEDT, Time << USART_CR1_DEDT_Pos);
}

/**
  * @brief  Return DEDT (Driver Enable De-Assertion Time)
  * @rmtoll CR1          DEDT          LL_LPUART_GetDEDeassertionTime
  * @param  LPUARTx LPUART Instance
  * @retval Time value expressed on 5 bits ([4:0] bits) : c
  */
__STATIC_INLINE uint32_t LL_LPUART_GetDEDeassertionTime(USART_TypeDef *LPUARTx)
{
  return (uint32_t)(READ_BIT(LPUARTx->CR1, USART_CR1_DEDT) >> USART_CR1_DEDT_Pos);
}

/**
  * @brief  Set DEAT (Driver Enable Assertion Time), Time value expressed on 5 bits ([4:0] bits).
  * @rmtoll CR1          DEAT          LL_LPUART_SetDEAssertionTime
  * @param  LPUARTx LPUART Instance
  * @param  Time Value between Min_Data=0 and Max_Data=31
  * @retval None
  */
__STATIC_INLINE void LL_LPUART_SetDEAssertionTime(USART_TypeDef *LPUARTx, uint32_t Time)
{
  MODIFY_REG(LPUARTx->CR1, USART_CR1_DEAT, Time << USART_CR1_DEAT_Pos);
}

/**
  * @brief  Return DEAT (Driver Enable Assertion Time)
  * @rmtoll CR1          DEAT          LL_LPUART_GetDEAssertionTime
  * @param  LPUARTx LPUART Instance
  * @retval Time value expressed on 5 bits ([4:0] bits) : Time Value between Min_Data=0 and Max_Data=31
  */
__STATIC_INLINE uint32_t LL_LPUART_GetDEAssertionTime(USART_TypeDef *LPUARTx)
{
  return (uint32_t)(READ_BIT(LPUARTx->CR1, USART_CR1_DEAT) >> USART_CR1_DEAT_Pos);
}

/**
  * @brief  Enable Driver Enable (DE) Mode
  * @rmtoll CR3          DEM           LL_LPUART_EnableDEMode
  * @param  LPUARTx LPUART Instance
  * @retval None
  */
__STATIC_INLINE void LL_LPUART_EnableDEMode(USART_TypeDef *LPUARTx)
{
  SET_BIT(LPUARTx->CR3, USART_CR3_DEM);
}

/**
  * @brief  Disable Driver Enable (DE) Mode
  * @rmtoll CR3          DEM           LL_LPUART_DisableDEMode
  * @param  LPUARTx LPUART Instance
  * @retval None
  */
__STATIC_INLINE void LL_LPUART_DisableDEMode(USART_TypeDef *LPUARTx)
{
  CLEAR_BIT(LPUARTx->CR3, USART_CR3_DEM);
}

/**
  * @brief  Indicate if Driver Enable (DE) Mode is enabled
  * @rmtoll CR3          DEM           LL_LPUART_IsEnabledDEMode
  * @param  LPUARTx LPUART Instance
  * @retval State of bit (1 or 0).
  */
__STATIC_INLINE uint32_t LL_LPUART_IsEnabledDEMode(USART_TypeDef *LPUARTx)
{
  return ((READ_BIT(LPUARTx->CR3, USART_CR3_DEM) == (USART_CR3_DEM)) ? 1UL : 0UL);
}

/**
  * @brief  Select Driver Enable Polarity
  * @rmtoll CR3          DEP           LL_LPUART_SetDESignalPolarity
  * @param  LPUARTx LPUART Instance
  * @param  Polarity This parameter can be one of the following values:
  *         @arg @ref LL_LPUART_DE_POLARITY_HIGH
  *         @arg @ref LL_LPUART_DE_POLARITY_LOW
  * @retval None
  */
__STATIC_INLINE void LL_LPUART_SetDESignalPolarity(USART_TypeDef *LPUARTx, uint32_t Polarity)
{
  MODIFY_REG(LPUARTx->CR3, USART_CR3_DEP, Polarity);
}

/**
  * @brief  Return Driver Enable Polarity
  * @rmtoll CR3          DEP           LL_LPUART_GetDESignalPolarity
  * @param  LPUARTx LPUART Instance
  * @retval Returned value can be one of the following values:
  *         @arg @ref LL_LPUART_DE_POLARITY_HIGH
  *         @arg @ref LL_LPUART_DE_POLARITY_LOW
  */
__STATIC_INLINE uint32_t LL_LPUART_GetDESignalPolarity(USART_TypeDef *LPUARTx)
{
  return (uint32_t)(READ_BIT(LPUARTx->CR3, USART_CR3_DEP));
}

/**
  * @}
  */

/** @defgroup LPUART_LL_EF_FLAG_Management FLAG_Management
  * @{
  */

/**
  * @brief  Check if the LPUART Parity Error Flag is set or not
  * @rmtoll ISR          PE            LL_LPUART_IsActiveFlag_PE
  * @param  LPUARTx LPUART Instance
  * @retval State of bit (1 or 0).
  */
__STATIC_INLINE uint32_t LL_LPUART_IsActiveFlag_PE(USART_TypeDef *LPUARTx)
{
  return ((READ_BIT(LPUARTx->ISR, USART_ISR_PE) == (USART_ISR_PE)) ? 1UL : 0UL);
}

/**
  * @brief  Check if the LPUART Framing Error Flag is set or not
  * @rmtoll ISR          FE            LL_LPUART_IsActiveFlag_FE
  * @param  LPUARTx LPUART Instance
  * @retval State of bit (1 or 0).
  */
__STATIC_INLINE uint32_t LL_LPUART_IsActiveFlag_FE(USART_TypeDef *LPUARTx)
{
  return ((READ_BIT(LPUARTx->ISR, USART_ISR_FE) == (USART_ISR_FE)) ? 1UL : 0UL);
}

/**
  * @brief  Check if the LPUART Noise error detected Flag is set or not
  * @rmtoll ISR          NE            LL_LPUART_IsActiveFlag_NE
  * @param  LPUARTx LPUART Instance
  * @retval State of bit (1 or 0).
  */
__STATIC_INLINE uint32_t LL_LPUART_IsActiveFlag_NE(USART_TypeDef *LPUARTx)
{
  return ((READ_BIT(LPUARTx->ISR, USART_ISR_NE) == (USART_ISR_NE)) ? 1UL : 0UL);
}

/**
  * @brief  Check if the LPUART OverRun Error Flag is set or not
  * @rmtoll ISR          ORE           LL_LPUART_IsActiveFlag_ORE
  * @param  LPUARTx LPUART Instance
  * @retval State of bit (1 or 0).
  */
__STATIC_INLINE uint32_t LL_LPUART_IsActiveFlag_ORE(USART_TypeDef *LPUARTx)
{
  return ((READ_BIT(LPUARTx->ISR, USART_ISR_ORE) == (USART_ISR_ORE)) ? 1UL : 0UL);
}

/**
  * @brief  Check if the LPUART IDLE line detected Flag is set or not
  * @rmtoll ISR          IDLE          LL_LPUART_IsActiveFlag_IDLE
  * @param  LPUARTx LPUART Instance
  * @retval State of bit (1 or 0).
  */
__STATIC_INLINE uint32_t LL_LPUART_IsActiveFlag_IDLE(USART_TypeDef *LPUARTx)
{
  return ((READ_BIT(LPUARTx->ISR, USART_ISR_IDLE) == (USART_ISR_IDLE)) ? 1UL : 0UL);
}

#if defined(USART_CR1_FIFOEN)
/* Legacy define */
#define LL_LPUART_IsActiveFlag_RXNE  LL_LPUART_IsActiveFlag_RXNE_RXFNE

/**
  * @brief  Check if the LPUART Read Data Register or LPUART RX FIFO Not Empty Flag is set or not
  * @rmtoll ISR          RXNE_RXFNE    LL_LPUART_IsActiveFlag_RXNE_RXFNE
  * @param  LPUARTx LPUART Instance
  * @retval State of bit (1 or 0).
  */
__STATIC_INLINE uint32_t LL_LPUART_IsActiveFlag_RXNE_RXFNE(USART_TypeDef *LPUARTx)
{
  return ((READ_BIT(LPUARTx->ISR, USART_ISR_RXNE_RXFNE) == (USART_ISR_RXNE_RXFNE)) ? 1UL : 0UL);
}
#else
/**
  * @brief  Check if the LPUART Read Data Register Not Empty Flag is set or not
  * @rmtoll ISR          RXNE          LL_LPUART_IsActiveFlag_RXNE
  * @param  LPUARTx LPUART Instance
  * @retval State of bit (1 or 0).
  */
__STATIC_INLINE uint32_t LL_LPUART_IsActiveFlag_RXNE(USART_TypeDef *LPUARTx)
{
  return ((READ_BIT(LPUARTx->ISR, USART_ISR_RXNE) == (USART_ISR_RXNE)) ? 1UL : 0UL);
}
#endif /* USART_CR1_FIFOEN */

/**
  * @brief  Check if the LPUART Transmission Complete Flag is set or not
  * @rmtoll ISR          TC            LL_LPUART_IsActiveFlag_TC
  * @param  LPUARTx LPUART Instance
  * @retval State of bit (1 or 0).
  */
__STATIC_INLINE uint32_t LL_LPUART_IsActiveFlag_TC(USART_TypeDef *LPUARTx)
{
  return ((READ_BIT(LPUARTx->ISR, USART_ISR_TC) == (USART_ISR_TC)) ? 1UL : 0UL);
}

#if defined(USART_CR1_FIFOEN)
/* Legacy define */
#define LL_LPUART_IsActiveFlag_TXE  LL_LPUART_IsActiveFlag_TXE_TXFNF

/**
  * @brief  Check if the LPUART Transmit Data Register Empty or LPUART TX FIFO Not Full Flag is set or not
  * @rmtoll ISR          TXE_TXFNF     LL_LPUART_IsActiveFlag_TXE_TXFNF
  * @param  LPUARTx LPUART Instance
  * @retval State of bit (1 or 0).
  */
__STATIC_INLINE uint32_t LL_LPUART_IsActiveFlag_TXE_TXFNF(USART_TypeDef *LPUARTx)
{
  return ((READ_BIT(LPUARTx->ISR, USART_ISR_TXE_TXFNF) == (USART_ISR_TXE_TXFNF)) ? 1UL : 0UL);
}
#else
/**
  * @brief  Check if the LPUART Transmit Data Register Empty Flag is set or not
  * @rmtoll ISR          TXE           LL_LPUART_IsActiveFlag_TXE
  * @param  LPUARTx LPUART Instance
  * @retval State of bit (1 or 0).
  */
__STATIC_INLINE uint32_t LL_LPUART_IsActiveFlag_TXE(USART_TypeDef *LPUARTx)
{
  return ((READ_BIT(LPUARTx->ISR, USART_ISR_TXE) == (USART_ISR_TXE)) ? 1UL : 0UL);
}
#endif /* USART_CR1_FIFOEN */

/**
  * @brief  Check if the LPUART CTS interrupt Flag is set or not
  * @rmtoll ISR          CTSIF         LL_LPUART_IsActiveFlag_nCTS
  * @param  LPUARTx LPUART Instance
  * @retval State of bit (1 or 0).
  */
__STATIC_INLINE uint32_t LL_LPUART_IsActiveFlag_nCTS(USART_TypeDef *LPUARTx)
{
  return ((READ_BIT(LPUARTx->ISR, USART_ISR_CTSIF) == (USART_ISR_CTSIF)) ? 1UL : 0UL);
}

/**
  * @brief  Check if the LPUART CTS Flag is set or not
  * @rmtoll ISR          CTS           LL_LPUART_IsActiveFlag_CTS
  * @param  LPUARTx LPUART Instance
  * @retval State of bit (1 or 0).
  */
__STATIC_INLINE uint32_t LL_LPUART_IsActiveFlag_CTS(USART_TypeDef *LPUARTx)
{
  return ((READ_BIT(LPUARTx->ISR, USART_ISR_CTS) == (USART_ISR_CTS)) ? 1UL : 0UL);
}

/**
  * @brief  Check if the LPUART Busy Flag is set or not
  * @rmtoll ISR          BUSY          LL_LPUART_IsActiveFlag_BUSY
  * @param  LPUARTx LPUART Instance
  * @retval State of bit (1 or 0).
  */
__STATIC_INLINE uint32_t LL_LPUART_IsActiveFlag_BUSY(USART_TypeDef *LPUARTx)
{
  return ((READ_BIT(LPUARTx->ISR, USART_ISR_BUSY) == (USART_ISR_BUSY)) ? 1UL : 0UL);
}

/**
  * @brief  Check if the LPUART Character Match Flag is set or not
  * @rmtoll ISR          CMF           LL_LPUART_IsActiveFlag_CM
  * @param  LPUARTx LPUART Instance
  * @retval State of bit (1 or 0).
  */
__STATIC_INLINE uint32_t LL_LPUART_IsActiveFlag_CM(USART_TypeDef *LPUARTx)
{
  return ((READ_BIT(LPUARTx->ISR, USART_ISR_CMF) == (USART_ISR_CMF)) ? 1UL : 0UL);
}

/**
  * @brief  Check if the LPUART Send Break Flag is set or not
  * @rmtoll ISR          SBKF          LL_LPUART_IsActiveFlag_SBK
  * @param  LPUARTx LPUART Instance
  * @retval State of bit (1 or 0).
  */
__STATIC_INLINE uint32_t LL_LPUART_IsActiveFlag_SBK(USART_TypeDef *LPUARTx)
{
  return ((READ_BIT(LPUARTx->ISR, USART_ISR_SBKF) == (USART_ISR_SBKF)) ? 1UL : 0UL);
}

/**
  * @brief  Check if the LPUART Receive Wake Up from mute mode Flag is set or not
  * @rmtoll ISR          RWU           LL_LPUART_IsActiveFlag_RWU
  * @param  LPUARTx LPUART Instance
  * @retval State of bit (1 or 0).
  */
__STATIC_INLINE uint32_t LL_LPUART_IsActiveFlag_RWU(USART_TypeDef *LPUARTx)
{
  return ((READ_BIT(LPUARTx->ISR, USART_ISR_RWU) == (USART_ISR_RWU)) ? 1UL : 0UL);
}

/**
  * @brief  Check if the LPUART Wake Up from stop mode Flag is set or not
  * @rmtoll ISR          WUF           LL_LPUART_IsActiveFlag_WKUP
  * @param  LPUARTx LPUART Instance
  * @retval State of bit (1 or 0).
  */
__STATIC_INLINE uint32_t LL_LPUART_IsActiveFlag_WKUP(USART_TypeDef *LPUARTx)
{
  return ((READ_BIT(LPUARTx->ISR, USART_ISR_WUF) == (USART_ISR_WUF)) ? 1UL : 0UL);
}

/**
  * @brief  Check if the LPUART Transmit Enable Acknowledge Flag is set or not
  * @rmtoll ISR          TEACK         LL_LPUART_IsActiveFlag_TEACK
  * @param  LPUARTx LPUART Instance
  * @retval State of bit (1 or 0).
  */
__STATIC_INLINE uint32_t LL_LPUART_IsActiveFlag_TEACK(USART_TypeDef *LPUARTx)
{
  return ((READ_BIT(LPUARTx->ISR, USART_ISR_TEACK) == (USART_ISR_TEACK)) ? 1UL : 0UL);
}

/**
  * @brief  Check if the LPUART Receive Enable Acknowledge Flag is set or not
  * @rmtoll ISR          REACK         LL_LPUART_IsActiveFlag_REACK
  * @param  LPUARTx LPUART Instance
  * @retval State of bit (1 or 0).
  */
__STATIC_INLINE uint32_t LL_LPUART_IsActiveFlag_REACK(USART_TypeDef *LPUARTx)
{
  return ((READ_BIT(LPUARTx->ISR, USART_ISR_REACK) == (USART_ISR_REACK)) ? 1UL : 0UL);
}

#if defined(USART_CR1_FIFOEN)
/**
  * @brief  Check if the LPUART TX FIFO Empty Flag is set or not
  * @rmtoll ISR          TXFE          LL_LPUART_IsActiveFlag_TXFE
  * @param  LPUARTx LPUART Instance
  * @retval State of bit (1 or 0).
  */
__STATIC_INLINE uint32_t LL_LPUART_IsActiveFlag_TXFE(USART_TypeDef *LPUARTx)
{
  return ((READ_BIT(LPUARTx->ISR, USART_ISR_TXFE) == (USART_ISR_TXFE)) ? 1UL : 0UL);
}

/**
  * @brief  Check if the LPUART RX FIFO Full Flag is set or not
  * @rmtoll ISR          RXFF          LL_LPUART_IsActiveFlag_RXFF
  * @param  LPUARTx LPUART Instance
  * @retval State of bit (1 or 0).
  */
__STATIC_INLINE uint32_t LL_LPUART_IsActiveFlag_RXFF(USART_TypeDef *LPUARTx)
{
  return ((READ_BIT(LPUARTx->ISR, USART_ISR_RXFF) == (USART_ISR_RXFF)) ? 1UL : 0UL);
}

/**
  * @brief  Check if the LPUART TX FIFO Threshold Flag is set or not
  * @rmtoll ISR          TXFT          LL_LPUART_IsActiveFlag_TXFT
  * @param  LPUARTx LPUART Instance
  * @retval State of bit (1 or 0).
  */
__STATIC_INLINE uint32_t LL_LPUART_IsActiveFlag_TXFT(USART_TypeDef *LPUARTx)
{
  return ((READ_BIT(LPUARTx->ISR, USART_ISR_TXFT) == (USART_ISR_TXFT)) ? 1UL : 0UL);
}

/**
  * @brief  Check if the LPUART RX FIFO Threshold Flag is set or not
  * @rmtoll ISR          RXFT          LL_LPUART_IsActiveFlag_RXFT
  * @param  LPUARTx LPUART Instance
  * @retval State of bit (1 or 0).
  */
__STATIC_INLINE uint32_t LL_LPUART_IsActiveFlag_RXFT(USART_TypeDef *LPUARTx)
{
  return ((READ_BIT(LPUARTx->ISR, USART_ISR_RXFT) == (USART_ISR_RXFT)) ? 1UL : 0UL);
}
#endif /* USART_CR1_FIFOEN */

/**
  * @brief  Clear Parity Error Flag
  * @rmtoll ICR          PECF          LL_LPUART_ClearFlag_PE
  * @param  LPUARTx LPUART Instance
  * @retval None
  */
__STATIC_INLINE void LL_LPUART_ClearFlag_PE(USART_TypeDef *LPUARTx)
{
  WRITE_REG(LPUARTx->ICR, USART_ICR_PECF);
}

/**
  * @brief  Clear Framing Error Flag
  * @rmtoll ICR          FECF          LL_LPUART_ClearFlag_FE
  * @param  LPUARTx LPUART Instance
  * @retval None
  */
__STATIC_INLINE void LL_LPUART_ClearFlag_FE(USART_TypeDef *LPUARTx)
{
  WRITE_REG(LPUARTx->ICR, USART_ICR_FECF);
}

/**
  * @brief  Clear Noise detected Flag
  * @rmtoll ICR          NECF          LL_LPUART_ClearFlag_NE
  * @param  LPUARTx LPUART Instance
  * @retval None
  */
__STATIC_INLINE void LL_LPUART_ClearFlag_NE(USART_TypeDef *LPUARTx)
{
  WRITE_REG(LPUARTx->ICR, USART_ICR_NECF);
}

/**
  * @brief  Clear OverRun Error Flag
  * @rmtoll ICR          ORECF         LL_LPUART_ClearFlag_ORE
  * @param  LPUARTx LPUART Instance
  * @retval None
  */
__STATIC_INLINE void LL_LPUART_ClearFlag_ORE(USART_TypeDef *LPUARTx)
{
  WRITE_REG(LPUARTx->ICR, USART_ICR_ORECF);
}

/**
  * @brief  Clear IDLE line detected Flag
  * @rmtoll ICR          IDLECF        LL_LPUART_ClearFlag_IDLE
  * @param  LPUARTx LPUART Instance
  * @retval None
  */
__STATIC_INLINE void LL_LPUART_ClearFlag_IDLE(USART_TypeDef *LPUARTx)
{
  WRITE_REG(LPUARTx->ICR, USART_ICR_IDLECF);
}

#if defined(USART_CR1_FIFOEN)
/**
  * @brief  Clear TX FIFO Empty Flag
  * @rmtoll ICR          TXFECF        LL_LPUART_ClearFlag_TXFE
  * @param  LPUARTx LPUART Instance
  * @retval None
  */
__STATIC_INLINE void LL_LPUART_ClearFlag_TXFE(USART_TypeDef *LPUARTx)
{
  WRITE_REG(LPUARTx->ICR, USART_ICR_TXFECF);
}
#endif /* USART_CR1_FIFOEN */

/**
  * @brief  Clear Transmission Complete Flag
  * @rmtoll ICR          TCCF          LL_LPUART_ClearFlag_TC
  * @param  LPUARTx LPUART Instance
  * @retval None
  */
__STATIC_INLINE void LL_LPUART_ClearFlag_TC(USART_TypeDef *LPUARTx)
{
  WRITE_REG(LPUARTx->ICR, USART_ICR_TCCF);
}

/**
  * @brief  Clear CTS Interrupt Flag
  * @rmtoll ICR          CTSCF         LL_LPUART_ClearFlag_nCTS
  * @param  LPUARTx LPUART Instance
  * @retval None
  */
__STATIC_INLINE void LL_LPUART_ClearFlag_nCTS(USART_TypeDef *LPUARTx)
{
  WRITE_REG(LPUARTx->ICR, USART_ICR_CTSCF);
}

/**
  * @brief  Clear Character Match Flag
  * @rmtoll ICR          CMCF          LL_LPUART_ClearFlag_CM
  * @param  LPUARTx LPUART Instance
  * @retval None
  */
__STATIC_INLINE void LL_LPUART_ClearFlag_CM(USART_TypeDef *LPUARTx)
{
  WRITE_REG(LPUARTx->ICR, USART_ICR_CMCF);
}

/**
  * @brief  Clear Wake Up from stop mode Flag
  * @rmtoll ICR          WUCF          LL_LPUART_ClearFlag_WKUP
  * @param  LPUARTx LPUART Instance
  * @retval None
  */
__STATIC_INLINE void LL_LPUART_ClearFlag_WKUP(USART_TypeDef *LPUARTx)
{
  WRITE_REG(LPUARTx->ICR, USART_ICR_WUCF);
}

/**
  * @}
  */

/** @defgroup LPUART_LL_EF_IT_Management IT_Management
  * @{
  */

/**
  * @brief  Enable IDLE Interrupt
  * @rmtoll CR1          IDLEIE        LL_LPUART_EnableIT_IDLE
  * @param  LPUARTx LPUART Instance
  * @retval None
  */
__STATIC_INLINE void LL_LPUART_EnableIT_IDLE(USART_TypeDef *LPUARTx)
{
  SET_BIT(LPUARTx->CR1, USART_CR1_IDLEIE);
}

#if defined(USART_CR1_FIFOEN)
/* Legacy define */
#define LL_LPUART_EnableIT_RXNE  LL_LPUART_EnableIT_RXNE_RXFNE

/**
  * @brief  Enable RX Not Empty and RX FIFO Not Empty Interrupt
  * @rmtoll CR1        RXNEIE_RXFNEIE  LL_LPUART_EnableIT_RXNE_RXFNE
  * @param  LPUARTx LPUART Instance
  * @retval None
  */
__STATIC_INLINE void LL_LPUART_EnableIT_RXNE_RXFNE(USART_TypeDef *LPUARTx)
{
  SET_BIT(LPUARTx->CR1, USART_CR1_RXNEIE_RXFNEIE);
}
#else

/**
  * @brief  Enable RX Not Empty Interrupt
  * @rmtoll CR1          RXNEIE        LL_LPUART_EnableIT_RXNE
  * @param  LPUARTx LPUART Instance
  * @retval None
  */
__STATIC_INLINE void LL_LPUART_EnableIT_RXNE(USART_TypeDef *LPUARTx)
{
  SET_BIT(LPUARTx->CR1, USART_CR1_RXNEIE);
}
#endif /* USART_CR1_FIFOEN */

/**
  * @brief  Enable Transmission Complete Interrupt
  * @rmtoll CR1          TCIE          LL_LPUART_EnableIT_TC
  * @param  LPUARTx LPUART Instance
  * @retval None
  */
__STATIC_INLINE void LL_LPUART_EnableIT_TC(USART_TypeDef *LPUARTx)
{
  SET_BIT(LPUARTx->CR1, USART_CR1_TCIE);
}

#if defined(USART_CR1_FIFOEN)
/* Legacy define */
#define LL_LPUART_EnableIT_TXE  LL_LPUART_EnableIT_TXE_TXFNF

/**
  * @brief  Enable TX Empty and TX FIFO Not Full Interrupt
  * @rmtoll CR1         TXEIE_TXFNFIE  LL_LPUART_EnableIT_TXE_TXFNF
  * @param  LPUARTx LPUART Instance
  * @retval None
  */
__STATIC_INLINE void LL_LPUART_EnableIT_TXE_TXFNF(USART_TypeDef *LPUARTx)
{
  SET_BIT(LPUARTx->CR1, USART_CR1_TXEIE_TXFNFIE);
}
#else

/**
  * @brief  Enable TX Empty Interrupt
  * @rmtoll CR1          TXEIE         LL_LPUART_EnableIT_TXE
  * @param  LPUARTx LPUART Instance
  * @retval None
  */
__STATIC_INLINE void LL_LPUART_EnableIT_TXE(USART_TypeDef *LPUARTx)
{
  SET_BIT(LPUARTx->CR1, USART_CR1_TXEIE);
}
#endif /* USART_CR1_FIFOEN */

/**
  * @brief  Enable Parity Error Interrupt
  * @rmtoll CR1          PEIE          LL_LPUART_EnableIT_PE
  * @param  LPUARTx LPUART Instance
  * @retval None
  */
__STATIC_INLINE void LL_LPUART_EnableIT_PE(USART_TypeDef *LPUARTx)
{
  SET_BIT(LPUARTx->CR1, USART_CR1_PEIE);
}

/**
  * @brief  Enable Character Match Interrupt
  * @rmtoll CR1          CMIE          LL_LPUART_EnableIT_CM
  * @param  LPUARTx LPUART Instance
  * @retval None
  */
__STATIC_INLINE void LL_LPUART_EnableIT_CM(USART_TypeDef *LPUARTx)
{
  SET_BIT(LPUARTx->CR1, USART_CR1_CMIE);
}

#if defined(USART_CR1_FIFOEN)
/**
  * @brief  Enable TX FIFO Empty Interrupt
  * @rmtoll CR1          TXFEIE        LL_LPUART_EnableIT_TXFE
  * @param  LPUARTx LPUART Instance
  * @retval None
  */
__STATIC_INLINE void LL_LPUART_EnableIT_TXFE(USART_TypeDef *LPUARTx)
{
  SET_BIT(LPUARTx->CR1, USART_CR1_TXFEIE);
}

/**
  * @brief  Enable RX FIFO Full Interrupt
  * @rmtoll CR1          RXFFIE        LL_LPUART_EnableIT_RXFF
  * @param  LPUARTx LPUART Instance
  * @retval None
  */
__STATIC_INLINE void LL_LPUART_EnableIT_RXFF(USART_TypeDef *LPUARTx)
{
  SET_BIT(LPUARTx->CR1, USART_CR1_RXFFIE);
}
#endif /* USART_CR1_FIFOEN */

/**
  * @brief  Enable Error Interrupt
  * @note   When set, Error Interrupt Enable Bit is enabling interrupt generation in case of a framing
  *         error, overrun error or noise flag (FE=1 or ORE=1 or NF=1 in the LPUARTx_ISR register).
  *         - 0: Interrupt is inhibited
  *         - 1: An interrupt is generated when FE=1 or ORE=1 or NF=1 in the LPUARTx_ISR register.
  * @rmtoll CR3          EIE           LL_LPUART_EnableIT_ERROR
  * @param  LPUARTx LPUART Instance
  * @retval None
  */
__STATIC_INLINE void LL_LPUART_EnableIT_ERROR(USART_TypeDef *LPUARTx)
{
  SET_BIT(LPUARTx->CR3, USART_CR3_EIE);
}

/**
  * @brief  Enable CTS Interrupt
  * @rmtoll CR3          CTSIE         LL_LPUART_EnableIT_CTS
  * @param  LPUARTx LPUART Instance
  * @retval None
  */
__STATIC_INLINE void LL_LPUART_EnableIT_CTS(USART_TypeDef *LPUARTx)
{
  SET_BIT(LPUARTx->CR3, USART_CR3_CTSIE);
}

/**
  * @brief  Enable Wake Up from Stop Mode Interrupt
  * @rmtoll CR3          WUFIE         LL_LPUART_EnableIT_WKUP
  * @param  LPUARTx LPUART Instance
  * @retval None
  */
__STATIC_INLINE void LL_LPUART_EnableIT_WKUP(USART_TypeDef *LPUARTx)
{
  SET_BIT(LPUARTx->CR3, USART_CR3_WUFIE);
}

#if defined(USART_CR1_FIFOEN)
/**
  * @brief  Enable TX FIFO Threshold Interrupt
  * @rmtoll CR3          TXFTIE        LL_LPUART_EnableIT_TXFT
  * @param  LPUARTx LPUART Instance
  * @retval None
  */
__STATIC_INLINE void LL_LPUART_EnableIT_TXFT(USART_TypeDef *LPUARTx)
{
  SET_BIT(LPUARTx->CR3, USART_CR3_TXFTIE);
}

/**
  * @brief  Enable RX FIFO Threshold Interrupt
  * @rmtoll CR3          RXFTIE        LL_LPUART_EnableIT_RXFT
  * @param  LPUARTx LPUART Instance
  * @retval None
  */
__STATIC_INLINE void LL_LPUART_EnableIT_RXFT(USART_TypeDef *LPUARTx)
{
  SET_BIT(LPUARTx->CR3, USART_CR3_RXFTIE);
}
#endif /* USART_CR1_FIFOEN */

/**
  * @brief  Disable IDLE Interrupt
  * @rmtoll CR1          IDLEIE        LL_LPUART_DisableIT_IDLE
  * @param  LPUARTx LPUART Instance
  * @retval None
  */
__STATIC_INLINE void LL_LPUART_DisableIT_IDLE(USART_TypeDef *LPUARTx)
{
  CLEAR_BIT(LPUARTx->CR1, USART_CR1_IDLEIE);
}

#if defined(USART_CR1_FIFOEN)
/* Legacy define */
#define LL_LPUART_DisableIT_RXNE  LL_LPUART_DisableIT_RXNE_RXFNE

/**
  * @brief  Disable RX Not Empty and RX FIFO Not Empty Interrupt
  * @rmtoll CR1        RXNEIE_RXFNEIE  LL_LPUART_DisableIT_RXNE_RXFNE
  * @param  LPUARTx LPUART Instance
  * @retval None
  */
__STATIC_INLINE void LL_LPUART_DisableIT_RXNE_RXFNE(USART_TypeDef *LPUARTx)
{
  CLEAR_BIT(LPUARTx->CR1, USART_CR1_RXNEIE_RXFNEIE);
}
#else

/**
  * @brief  Disable RX Not Empty Interrupt
  * @rmtoll CR1          RXNEIE        LL_LPUART_DisableIT_RXNE
  * @param  LPUARTx LPUART Instance
  * @retval None
  */
__STATIC_INLINE void LL_LPUART_DisableIT_RXNE(USART_TypeDef *LPUARTx)
{
  CLEAR_BIT(LPUARTx->CR1, USART_CR1_RXNEIE);
}
#endif /* USART_CR1_FIFOEN */

/**
  * @brief  Disable Transmission Complete Interrupt
  * @rmtoll CR1          TCIE          LL_LPUART_DisableIT_TC
  * @param  LPUARTx LPUART Instance
  * @retval None
  */
__STATIC_INLINE void LL_LPUART_DisableIT_TC(USART_TypeDef *LPUARTx)
{
  CLEAR_BIT(LPUARTx->CR1, USART_CR1_TCIE);
}

#if defined(USART_CR1_FIFOEN)
/* Legacy define */
#define LL_LPUART_DisableIT_TXE  LL_LPUART_DisableIT_TXE_TXFNF

/**
  * @brief  Disable TX Empty and TX FIFO Not Full Interrupt
  * @rmtoll CR1        TXEIE_TXFNFIE  LL_LPUART_DisableIT_TXE_TXFNF
  * @param  LPUARTx LPUART Instance
  * @retval None
  */
__STATIC_INLINE void LL_LPUART_DisableIT_TXE_TXFNF(USART_TypeDef *LPUARTx)
{
  CLEAR_BIT(LPUARTx->CR1, USART_CR1_TXEIE_TXFNFIE);
}
#else

/**
  * @brief  Disable TX Empty Interrupt
  * @rmtoll CR1          TXEIE         LL_LPUART_DisableIT_TXE
  * @param  LPUARTx LPUART Instance
  * @retval None
  */
__STATIC_INLINE void LL_LPUART_DisableIT_TXE(USART_TypeDef *LPUARTx)
{
  CLEAR_BIT(LPUARTx->CR1, USART_CR1_TXEIE);
}
#endif /* USART_CR1_FIFOEN */

/**
  * @brief  Disable Parity Error Interrupt
  * @rmtoll CR1          PEIE          LL_LPUART_DisableIT_PE
  * @param  LPUARTx LPUART Instance
  * @retval None
  */
__STATIC_INLINE void LL_LPUART_DisableIT_PE(USART_TypeDef *LPUARTx)
{
  CLEAR_BIT(LPUARTx->CR1, USART_CR1_PEIE);
}

/**
  * @brief  Disable Character Match Interrupt
  * @rmtoll CR1          CMIE          LL_LPUART_DisableIT_CM
  * @param  LPUARTx LPUART Instance
  * @retval None
  */
__STATIC_INLINE void LL_LPUART_DisableIT_CM(USART_TypeDef *LPUARTx)
{
  CLEAR_BIT(LPUARTx->CR1, USART_CR1_CMIE);
}

#if defined(USART_CR1_FIFOEN)
/**
  * @brief  Disable TX FIFO Empty Interrupt
  * @rmtoll CR1          TXFEIE        LL_LPUART_DisableIT_TXFE
  * @param  LPUARTx LPUART Instance
  * @retval None
  */
__STATIC_INLINE void LL_LPUART_DisableIT_TXFE(USART_TypeDef *LPUARTx)
{
  CLEAR_BIT(LPUARTx->CR1, USART_CR1_TXFEIE);
}

/**
  * @brief  Disable RX FIFO Full Interrupt
  * @rmtoll CR1          RXFFIE        LL_LPUART_DisableIT_RXFF
  * @param  LPUARTx LPUART Instance
  * @retval None
  */
__STATIC_INLINE void LL_LPUART_DisableIT_RXFF(USART_TypeDef *LPUARTx)
{
  CLEAR_BIT(LPUARTx->CR1, USART_CR1_RXFFIE);
}
#endif /* USART_CR1_FIFOEN */

/**
  * @brief  Disable Error Interrupt
  * @note   When set, Error Interrupt Enable Bit is enabling interrupt generation in case of a framing
  *         error, overrun error or noise flag (FE=1 or ORE=1 or NF=1 in the LPUARTx_ISR register).
  *         - 0: Interrupt is inhibited
  *         - 1: An interrupt is generated when FE=1 or ORE=1 or NF=1 in the LPUARTx_ISR register.
  * @rmtoll CR3          EIE           LL_LPUART_DisableIT_ERROR
  * @param  LPUARTx LPUART Instance
  * @retval None
  */
__STATIC_INLINE void LL_LPUART_DisableIT_ERROR(USART_TypeDef *LPUARTx)
{
  CLEAR_BIT(LPUARTx->CR3, USART_CR3_EIE);
}

/**
  * @brief  Disable CTS Interrupt
  * @rmtoll CR3          CTSIE         LL_LPUART_DisableIT_CTS
  * @param  LPUARTx LPUART Instance
  * @retval None
  */
__STATIC_INLINE void LL_LPUART_DisableIT_CTS(USART_TypeDef *LPUARTx)
{
  CLEAR_BIT(LPUARTx->CR3, USART_CR3_CTSIE);
}

/**
  * @brief  Disable Wake Up from Stop Mode Interrupt
  * @rmtoll CR3          WUFIE         LL_LPUART_DisableIT_WKUP
  * @param  LPUARTx LPUART Instance
  * @retval None
  */
__STATIC_INLINE void LL_LPUART_DisableIT_WKUP(USART_TypeDef *LPUARTx)
{
  CLEAR_BIT(LPUARTx->CR3, USART_CR3_WUFIE);
}

#if defined(USART_CR1_FIFOEN)
/**
  * @brief  Disable TX FIFO Threshold Interrupt
  * @rmtoll CR3          TXFTIE        LL_LPUART_DisableIT_TXFT
  * @param  LPUARTx LPUART Instance
  * @retval None
  */
__STATIC_INLINE void LL_LPUART_DisableIT_TXFT(USART_TypeDef *LPUARTx)
{
  CLEAR_BIT(LPUARTx->CR3, USART_CR3_TXFTIE);
}

/**
  * @brief  Disable RX FIFO Threshold Interrupt
  * @rmtoll CR3          RXFTIE        LL_LPUART_DisableIT_RXFT
  * @param  LPUARTx LPUART Instance
  * @retval None
  */
__STATIC_INLINE void LL_LPUART_DisableIT_RXFT(USART_TypeDef *LPUARTx)
{
  CLEAR_BIT(LPUARTx->CR3, USART_CR3_RXFTIE);
}
#endif /* USART_CR1_FIFOEN */

/**
  * @brief  Check if the LPUART IDLE Interrupt  source is enabled or disabled.
  * @rmtoll CR1          IDLEIE        LL_LPUART_IsEnabledIT_IDLE
  * @param  LPUARTx LPUART Instance
  * @retval State of bit (1 or 0).
  */
__STATIC_INLINE uint32_t LL_LPUART_IsEnabledIT_IDLE(USART_TypeDef *LPUARTx)
{
  return ((READ_BIT(LPUARTx->CR1, USART_CR1_IDLEIE) == (USART_CR1_IDLEIE)) ? 1UL : 0UL);
}

#if defined(USART_CR1_FIFOEN)
/* Legacy define */
#define LL_LPUART_IsEnabledIT_RXNE  LL_LPUART_IsEnabledIT_RXNE_RXFNE

/**
  * @brief  Check if the LPUART RX Not Empty and LPUART RX FIFO Not Empty Interrupt is enabled or disabled.
  * @rmtoll CR1        RXNEIE_RXFNEIE  LL_LPUART_IsEnabledIT_RXNE_RXFNE
  * @param  LPUARTx LPUART Instance
  * @retval State of bit (1 or 0).
  */
__STATIC_INLINE uint32_t LL_LPUART_IsEnabledIT_RXNE_RXFNE(USART_TypeDef *LPUARTx)
{
  return ((READ_BIT(LPUARTx->CR1, USART_CR1_RXNEIE_RXFNEIE) == (USART_CR1_RXNEIE_RXFNEIE)) ? 1UL : 0UL);
}
#else

/**
  * @brief  Check if the LPUART RX Not Empty Interrupt is enabled or disabled.
  * @rmtoll CR1          RXNEIE        LL_LPUART_IsEnabledIT_RXNE
  * @param  LPUARTx LPUART Instance
  * @retval State of bit (1 or 0).
  */
__STATIC_INLINE uint32_t LL_LPUART_IsEnabledIT_RXNE(USART_TypeDef *LPUARTx)
{
  return ((READ_BIT(LPUARTx->CR1, USART_CR1_RXNEIE) == (USART_CR1_RXNEIE)) ? 1UL : 0UL);
}
#endif /* USART_CR1_FIFOEN */

/**
  * @brief  Check if the LPUART Transmission Complete Interrupt is enabled or disabled.
  * @rmtoll CR1          TCIE          LL_LPUART_IsEnabledIT_TC
  * @param  LPUARTx LPUART Instance
  * @retval State of bit (1 or 0).
  */
__STATIC_INLINE uint32_t LL_LPUART_IsEnabledIT_TC(USART_TypeDef *LPUARTx)
{
  return ((READ_BIT(LPUARTx->CR1, USART_CR1_TCIE) == (USART_CR1_TCIE)) ? 1UL : 0UL);
}

#if defined(USART_CR1_FIFOEN)
/* Legacy define */
#define LL_LPUART_IsEnabledIT_TXE  LL_LPUART_IsEnabledIT_TXE_TXFNF

/**
  * @brief  Check if the LPUART TX Empty and LPUART TX FIFO Not Full Interrupt is enabled or disabled
  * @rmtoll CR1         TXEIE_TXFNFIE  LL_LPUART_IsEnabledIT_TXE_TXFNF
  * @param  LPUARTx LPUART Instance
  * @retval State of bit (1 or 0).
  */
__STATIC_INLINE uint32_t LL_LPUART_IsEnabledIT_TXE_TXFNF(USART_TypeDef *LPUARTx)
{
  return ((READ_BIT(LPUARTx->CR1, USART_CR1_TXEIE_TXFNFIE) == (USART_CR1_TXEIE_TXFNFIE)) ? 1UL : 0UL);
}
#else

/**
  * @brief  Check if the LPUART TX Empty Interrupt is enabled or disabled.
  * @rmtoll CR1          TXEIE         LL_LPUART_IsEnabledIT_TXE
  * @param  LPUARTx LPUART Instance
  * @retval State of bit (1 or 0).
  */
__STATIC_INLINE uint32_t LL_LPUART_IsEnabledIT_TXE(USART_TypeDef *LPUARTx)
{
  return ((READ_BIT(LPUARTx->CR1, USART_CR1_TXEIE) == (USART_CR1_TXEIE)) ? 1UL : 0UL);
}
#endif /* USART_CR1_FIFOEN */

/**
  * @brief  Check if the LPUART Parity Error Interrupt is enabled or disabled.
  * @rmtoll CR1          PEIE          LL_LPUART_IsEnabledIT_PE
  * @param  LPUARTx LPUART Instance
  * @retval State of bit (1 or 0).
  */
__STATIC_INLINE uint32_t LL_LPUART_IsEnabledIT_PE(USART_TypeDef *LPUARTx)
{
  return ((READ_BIT(LPUARTx->CR1, USART_CR1_PEIE) == (USART_CR1_PEIE)) ? 1UL : 0UL);
}

/**
  * @brief  Check if the LPUART Character Match Interrupt is enabled or disabled.
  * @rmtoll CR1          CMIE          LL_LPUART_IsEnabledIT_CM
  * @param  LPUARTx LPUART Instance
  * @retval State of bit (1 or 0).
  */
__STATIC_INLINE uint32_t LL_LPUART_IsEnabledIT_CM(USART_TypeDef *LPUARTx)
{
  return ((READ_BIT(LPUARTx->CR1, USART_CR1_CMIE) == (USART_CR1_CMIE)) ? 1UL : 0UL);
}

#if defined(USART_CR1_FIFOEN)
/**
  * @brief  Check if the LPUART TX FIFO Empty Interrupt is enabled or disabled
  * @rmtoll CR1          TXFEIE        LL_LPUART_IsEnabledIT_TXFE
  * @param  LPUARTx LPUART Instance
  * @retval State of bit (1 or 0).
  */
__STATIC_INLINE uint32_t LL_LPUART_IsEnabledIT_TXFE(USART_TypeDef *LPUARTx)
{
  return ((READ_BIT(LPUARTx->CR1, USART_CR1_TXFEIE) == (USART_CR1_TXFEIE)) ? 1UL : 0UL);
}

/**
  * @brief  Check if the LPUART RX FIFO Full Interrupt is enabled or disabled
  * @rmtoll CR1          RXFFIE        LL_LPUART_IsEnabledIT_RXFF
  * @param  LPUARTx LPUART Instance
  * @retval State of bit (1 or 0).
  */
__STATIC_INLINE uint32_t LL_LPUART_IsEnabledIT_RXFF(USART_TypeDef *LPUARTx)
{
  return ((READ_BIT(LPUARTx->CR1, USART_CR1_RXFFIE) == (USART_CR1_RXFFIE)) ? 1UL : 0UL);
}
#endif /* USART_CR1_FIFOEN */

/**
  * @brief  Check if the LPUART Error Interrupt is enabled or disabled.
  * @rmtoll CR3          EIE           LL_LPUART_IsEnabledIT_ERROR
  * @param  LPUARTx LPUART Instance
  * @retval State of bit (1 or 0).
  */
__STATIC_INLINE uint32_t LL_LPUART_IsEnabledIT_ERROR(USART_TypeDef *LPUARTx)
{
  return ((READ_BIT(LPUARTx->CR3, USART_CR3_EIE) == (USART_CR3_EIE)) ? 1UL : 0UL);
}

/**
  * @brief  Check if the LPUART CTS Interrupt is enabled or disabled.
  * @rmtoll CR3          CTSIE         LL_LPUART_IsEnabledIT_CTS
  * @param  LPUARTx LPUART Instance
  * @retval State of bit (1 or 0).
  */
__STATIC_INLINE uint32_t LL_LPUART_IsEnabledIT_CTS(USART_TypeDef *LPUARTx)
{
  return ((READ_BIT(LPUARTx->CR3, USART_CR3_CTSIE) == (USART_CR3_CTSIE)) ? 1UL : 0UL);
}

/**
  * @brief  Check if the LPUART Wake Up from Stop Mode Interrupt is enabled or disabled.
  * @rmtoll CR3          WUFIE         LL_LPUART_IsEnabledIT_WKUP
  * @param  LPUARTx LPUART Instance
  * @retval State of bit (1 or 0).
  */
__STATIC_INLINE uint32_t LL_LPUART_IsEnabledIT_WKUP(USART_TypeDef *LPUARTx)
{
  return ((READ_BIT(LPUARTx->CR3, USART_CR3_WUFIE) == (USART_CR3_WUFIE)) ? 1UL : 0UL);
}

#if defined(USART_CR1_FIFOEN)
/**
  * @brief  Check if LPUART TX FIFO Threshold Interrupt is enabled or disabled
  * @rmtoll CR3          TXFTIE        LL_LPUART_IsEnabledIT_TXFT
  * @param  LPUARTx LPUART Instance
  * @retval State of bit (1 or 0).
  */
__STATIC_INLINE uint32_t LL_LPUART_IsEnabledIT_TXFT(USART_TypeDef *LPUARTx)
{
  return ((READ_BIT(LPUARTx->CR3, USART_CR3_TXFTIE) == (USART_CR3_TXFTIE)) ? 1UL : 0UL);
}

/**
  * @brief  Check if LPUART RX FIFO Threshold Interrupt is enabled or disabled
  * @rmtoll CR3          RXFTIE        LL_LPUART_IsEnabledIT_RXFT
  * @param  LPUARTx LPUART Instance
  * @retval State of bit (1 or 0).
  */
__STATIC_INLINE uint32_t LL_LPUART_IsEnabledIT_RXFT(USART_TypeDef *LPUARTx)
{
  return ((READ_BIT(LPUARTx->CR3, USART_CR3_RXFTIE) == (USART_CR3_RXFTIE)) ? 1UL : 0UL);
}
#endif /* USART_CR1_FIFOEN */

/**
  * @}
  */

/** @defgroup LPUART_LL_EF_DMA_Management DMA_Management
  * @{
  */

/**
  * @brief  Enable DMA Mode for reception
  * @rmtoll CR3          DMAR          LL_LPUART_EnableDMAReq_RX
  * @param  LPUARTx LPUART Instance
  * @retval None
  */
__STATIC_INLINE void LL_LPUART_EnableDMAReq_RX(USART_TypeDef *LPUARTx)
{
  SET_BIT(LPUARTx->CR3, USART_CR3_DMAR);
}

/**
  * @brief  Disable DMA Mode for reception
  * @rmtoll CR3          DMAR          LL_LPUART_DisableDMAReq_RX
  * @param  LPUARTx LPUART Instance
  * @retval None
  */
__STATIC_INLINE void LL_LPUART_DisableDMAReq_RX(USART_TypeDef *LPUARTx)
{
  CLEAR_BIT(LPUARTx->CR3, USART_CR3_DMAR);
}

/**
  * @brief  Check if DMA Mode is enabled for reception
  * @rmtoll CR3          DMAR          LL_LPUART_IsEnabledDMAReq_RX
  * @param  LPUARTx LPUART Instance
  * @retval State of bit (1 or 0).
  */
__STATIC_INLINE uint32_t LL_LPUART_IsEnabledDMAReq_RX(USART_TypeDef *LPUARTx)
{
  return ((READ_BIT(LPUARTx->CR3, USART_CR3_DMAR) == (USART_CR3_DMAR)) ? 1UL : 0UL);
}

/**
  * @brief  Enable DMA Mode for transmission
  * @rmtoll CR3          DMAT          LL_LPUART_EnableDMAReq_TX
  * @param  LPUARTx LPUART Instance
  * @retval None
  */
__STATIC_INLINE void LL_LPUART_EnableDMAReq_TX(USART_TypeDef *LPUARTx)
{
  SET_BIT(LPUARTx->CR3, USART_CR3_DMAT);
}

/**
  * @brief  Disable DMA Mode for transmission
  * @rmtoll CR3          DMAT          LL_LPUART_DisableDMAReq_TX
  * @param  LPUARTx LPUART Instance
  * @retval None
  */
__STATIC_INLINE void LL_LPUART_DisableDMAReq_TX(USART_TypeDef *LPUARTx)
{
  CLEAR_BIT(LPUARTx->CR3, USART_CR3_DMAT);
}

/**
  * @brief  Check if DMA Mode is enabled for transmission
  * @rmtoll CR3          DMAT          LL_LPUART_IsEnabledDMAReq_TX
  * @param  LPUARTx LPUART Instance
  * @retval State of bit (1 or 0).
  */
__STATIC_INLINE uint32_t LL_LPUART_IsEnabledDMAReq_TX(USART_TypeDef *LPUARTx)
{
  return ((READ_BIT(LPUARTx->CR3, USART_CR3_DMAT) == (USART_CR3_DMAT)) ? 1UL : 0UL);
}

/**
  * @brief  Enable DMA Disabling on Reception Error
  * @rmtoll CR3          DDRE          LL_LPUART_EnableDMADeactOnRxErr
  * @param  LPUARTx LPUART Instance
  * @retval None
  */
__STATIC_INLINE void LL_LPUART_EnableDMADeactOnRxErr(USART_TypeDef *LPUARTx)
{
  SET_BIT(LPUARTx->CR3, USART_CR3_DDRE);
}

/**
  * @brief  Disable DMA Disabling on Reception Error
  * @rmtoll CR3          DDRE          LL_LPUART_DisableDMADeactOnRxErr
  * @param  LPUARTx LPUART Instance
  * @retval None
  */
__STATIC_INLINE void LL_LPUART_DisableDMADeactOnRxErr(USART_TypeDef *LPUARTx)
{
  CLEAR_BIT(LPUARTx->CR3, USART_CR3_DDRE);
}

/**
  * @brief  Indicate if DMA Disabling on Reception Error is disabled
  * @rmtoll CR3          DDRE          LL_LPUART_IsEnabledDMADeactOnRxErr
  * @param  LPUARTx LPUART Instance
  * @retval State of bit (1 or 0).
  */
__STATIC_INLINE uint32_t LL_LPUART_IsEnabledDMADeactOnRxErr(USART_TypeDef *LPUARTx)
{
  return ((READ_BIT(LPUARTx->CR3, USART_CR3_DDRE) == (USART_CR3_DDRE)) ? 1UL : 0UL);
}

/**
  * @brief  Get the LPUART data register address used for DMA transfer
  * @rmtoll RDR          RDR           LL_LPUART_DMA_GetRegAddr\n
  * @rmtoll TDR          TDR           LL_LPUART_DMA_GetRegAddr
  * @param  LPUARTx LPUART Instance
  * @param  Direction This parameter can be one of the following values:
  *         @arg @ref LL_LPUART_DMA_REG_DATA_TRANSMIT
  *         @arg @ref LL_LPUART_DMA_REG_DATA_RECEIVE
  * @retval Address of data register
  */
__STATIC_INLINE uint32_t LL_LPUART_DMA_GetRegAddr(USART_TypeDef *LPUARTx, uint32_t Direction)
{
  register uint32_t data_reg_addr;

  if (Direction == LL_LPUART_DMA_REG_DATA_TRANSMIT)
  {
    /* return address of TDR register */
    data_reg_addr = (uint32_t) &(LPUARTx->TDR);
  }
  else
  {
    /* return address of RDR register */
    data_reg_addr = (uint32_t) &(LPUARTx->RDR);
  }

  return data_reg_addr;
}

/**
  * @}
  */

/** @defgroup LPUART_LL_EF_Data_Management Data_Management
  * @{
  */

/**
  * @brief  Read Receiver Data register (Receive Data value, 8 bits)
  * @rmtoll RDR          RDR           LL_LPUART_ReceiveData8
  * @param  LPUARTx LPUART Instance
  * @retval Time Value between Min_Data=0x00 and Max_Data=0xFF
  */
__STATIC_INLINE uint8_t LL_LPUART_ReceiveData8(USART_TypeDef *LPUARTx)
{
  return (uint8_t)(READ_BIT(LPUARTx->RDR, USART_RDR_RDR) & 0xFFU);
}

/**
  * @brief  Read Receiver Data register (Receive Data value, 9 bits)
  * @rmtoll RDR          RDR           LL_LPUART_ReceiveData9
  * @param  LPUARTx LPUART Instance
  * @retval Time Value between Min_Data=0x00 and Max_Data=0x1FF
  */
__STATIC_INLINE uint16_t LL_LPUART_ReceiveData9(USART_TypeDef *LPUARTx)
{
  return (uint16_t)(READ_BIT(LPUARTx->RDR, USART_RDR_RDR));
}

/**
  * @brief  Write in Transmitter Data Register (Transmit Data value, 8 bits)
  * @rmtoll TDR          TDR           LL_LPUART_TransmitData8
  * @param  LPUARTx LPUART Instance
  * @param  Value between Min_Data=0x00 and Max_Data=0xFF
  * @retval None
  */
__STATIC_INLINE void LL_LPUART_TransmitData8(USART_TypeDef *LPUARTx, uint8_t Value)
{
  LPUARTx->TDR = Value;
}

/**
  * @brief  Write in Transmitter Data Register (Transmit Data value, 9 bits)
  * @rmtoll TDR          TDR           LL_LPUART_TransmitData9
  * @param  LPUARTx LPUART Instance
  * @param  Value between Min_Data=0x00 and Max_Data=0x1FF
  * @retval None
  */
__STATIC_INLINE void LL_LPUART_TransmitData9(USART_TypeDef *LPUARTx, uint16_t Value)
{
  LPUARTx->TDR = Value & 0x1FFUL;
}

/**
  * @}
  */

/** @defgroup LPUART_LL_EF_Execution Execution
  * @{
  */

/**
  * @brief  Request Break sending
  * @rmtoll RQR          SBKRQ         LL_LPUART_RequestBreakSending
  * @param  LPUARTx LPUART Instance
  * @retval None
  */
__STATIC_INLINE void LL_LPUART_RequestBreakSending(USART_TypeDef *LPUARTx)
{
  SET_BIT(LPUARTx->RQR, (uint16_t)USART_RQR_SBKRQ);
}

/**
  * @brief  Put LPUART in mute mode and set the RWU flag
  * @rmtoll RQR          MMRQ          LL_LPUART_RequestEnterMuteMode
  * @param  LPUARTx LPUART Instance
  * @retval None
  */
__STATIC_INLINE void LL_LPUART_RequestEnterMuteMode(USART_TypeDef *LPUARTx)
{
  SET_BIT(LPUARTx->RQR, (uint16_t)USART_RQR_MMRQ);
}

/**
  @if USART_CR1_FIFOEN
  * @brief  Request a Receive Data and FIFO flush
  * @note   Allows to discard the received data without reading them, and avoid an overrun
  *         condition.
  @else
  * @brief  Request a Receive Data flush
  @endif
  * @rmtoll RQR          RXFRQ         LL_LPUART_RequestRxDataFlush
  * @param  LPUARTx LPUART Instance
  * @retval None
  */
__STATIC_INLINE void LL_LPUART_RequestRxDataFlush(USART_TypeDef *LPUARTx)
{
  SET_BIT(LPUARTx->RQR, (uint16_t)USART_RQR_RXFRQ);
}

/**
  * @}
  */

#if defined(USE_FULL_LL_DRIVER)
/** @defgroup LPUART_LL_EF_Init Initialization and de-initialization functions
  * @{
  */
ErrorStatus LL_LPUART_DeInit(USART_TypeDef *LPUARTx);
ErrorStatus LL_LPUART_Init(USART_TypeDef *LPUARTx, LL_LPUART_InitTypeDef *LPUART_InitStruct);
void        LL_LPUART_StructInit(LL_LPUART_InitTypeDef *LPUART_InitStruct);
/**
  * @}
  */
#endif /* USE_FULL_LL_DRIVER */

/**
  * @}
  */

/**
  * @}
  */

#endif /* LPUART1 */

/**
  * @}
  */

#ifdef __cplusplus
}
#endif

#endif /* STM32L4xx_LL_LPUART_H */

/************************ (C) COPYRIGHT STMicroelectronics *****END OF FILE****/
