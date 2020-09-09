/**
  ******************************************************************************
  * @file    stm32l4xx_ll_sdmmc.h
  * @author  MCD Application Team
  * @brief   Header file of SDMMC HAL module.
  ******************************************************************************
  * @attention
  *
  * <h2><center>&copy; Copyright (c) 2017 STMicroelectronics.
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
#ifndef STM32L4xx_LL_SDMMC_H
#define STM32L4xx_LL_SDMMC_H

#ifdef __cplusplus
 extern "C" {
#endif

#if defined(SDMMC1)

/* Includes ------------------------------------------------------------------*/
#include "stm32l4xx_hal_def.h"

/** @addtogroup STM32L4xx_Driver
  * @{
  */

/** @addtogroup SDMMC_LL
  * @{
  */

/* Exported types ------------------------------------------------------------*/
/** @defgroup SDMMC_LL_Exported_Types SDMMC_LL Exported Types
  * @{
  */

/**
  * @brief  SDMMC Configuration Structure definition
  */
typedef struct
{
  uint32_t ClockEdge;            /*!< Specifies the clock transition on which the bit capture is made.
                                      This parameter can be a value of @ref SDMMC_LL_Clock_Edge                 */

#if !defined(STM32L4R5xx) && !defined(STM32L4R7xx) && !defined(STM32L4R9xx) && !defined(STM32L4S5xx) && !defined(STM32L4S7xx) && !defined(STM32L4S9xx)
  uint32_t ClockBypass;          /*!< Specifies whether the SDMMC Clock divider bypass is
                                      enabled or disabled.
                                      This parameter can be a value of @ref SDMMC_LL_Clock_Bypass               */
#endif /* !STM32L4R5xx && !STM32L4R7xx && !STM32L4R9xx && !STM32L4S5xx && !STM32L4S7xx && !STM32L4S9xx */

  uint32_t ClockPowerSave;       /*!< Specifies whether SDMMC Clock output is enabled or
                                      disabled when the bus is idle.
                                      This parameter can be a value of @ref SDMMC_LL_Clock_Power_Save           */

  uint32_t BusWide;              /*!< Specifies the SDMMC bus width.
                                      This parameter can be a value of @ref SDMMC_LL_Bus_Wide                   */

  uint32_t HardwareFlowControl;  /*!< Specifies whether the SDMMC hardware flow control is enabled or disabled.
                                      This parameter can be a value of @ref SDMMC_LL_Hardware_Flow_Control      */

  uint32_t ClockDiv;             /*!< Specifies the clock frequency of the SDMMC controller.
                                      This parameter can be a value between Min_Data = 0 and Max_Data = 1023   */

#if defined(STM32L4R5xx) || defined(STM32L4R7xx) || defined(STM32L4R9xx) || defined(STM32L4S5xx) || defined(STM32L4S7xx) || defined(STM32L4S9xx)
  uint32_t Transceiver;          /*!< Specifies whether external Transceiver is enabled or disabled.
                                      This parameter can be a value of @ref SDMMC_LL_Transceiver */
#endif /* STM32L4R5xx || STM32L4R7xx || STM32L4R9xx || STM32L4S5xx || STM32L4S7xx || STM32L4S9xx */

}SDMMC_InitTypeDef;


/**
  * @brief  SDMMC Command Control structure
  */
typedef struct
{
  uint32_t Argument;            /*!< Specifies the SDMMC command argument which is sent
                                     to a card as part of a command message. If a command
                                     contains an argument, it must be loaded into this register
                                     before writing the command to the command register.              */

  uint32_t CmdIndex;            /*!< Specifies the SDMMC command index. It must be Min_Data = 0 and
                                     Max_Data = 64                                                    */

  uint32_t Response;            /*!< Specifies the SDMMC response type.
                                     This parameter can be a value of @ref SDMMC_LL_Response_Type         */

  uint32_t WaitForInterrupt;    /*!< Specifies whether SDMMC wait for interrupt request is
                                     enabled or disabled.
                                     This parameter can be a value of @ref SDMMC_LL_Wait_Interrupt_State  */

  uint32_t CPSM;                /*!< Specifies whether SDMMC Command path state machine (CPSM)
                                     is enabled or disabled.
                                     This parameter can be a value of @ref SDMMC_LL_CPSM_State            */
}SDMMC_CmdInitTypeDef;


/**
  * @brief  SDMMC Data Control structure
  */
typedef struct
{
  uint32_t DataTimeOut;         /*!< Specifies the data timeout period in card bus clock periods.  */

  uint32_t DataLength;          /*!< Specifies the number of data bytes to be transferred.         */

  uint32_t DataBlockSize;       /*!< Specifies the data block size for block transfer.
                                     This parameter can be a value of @ref SDMMC_LL_Data_Block_Size    */

  uint32_t TransferDir;         /*!< Specifies the data transfer direction, whether the transfer
                                     is a read or write.
                                     This parameter can be a value of @ref SDMMC_LL_Transfer_Direction */

  uint32_t TransferMode;        /*!< Specifies whether data transfer is in stream or block mode.
                                     This parameter can be a value of @ref SDMMC_LL_Transfer_Type      */

  uint32_t DPSM;                /*!< Specifies whether SDMMC Data path state machine (DPSM)
                                     is enabled or disabled.
                                     This parameter can be a value of @ref SDMMC_LL_DPSM_State         */
}SDMMC_DataInitTypeDef;

/**
  * @}
  */

/* Exported constants --------------------------------------------------------*/
/** @defgroup SDMMC_LL_Exported_Constants SDMMC_LL Exported Constants
  * @{
  */
#define SDMMC_ERROR_NONE                     ((uint32_t)0x00000000U)   /*!< No error                                                      */
#define SDMMC_ERROR_CMD_CRC_FAIL             ((uint32_t)0x00000001U)   /*!< Command response received (but CRC check failed)              */
#define SDMMC_ERROR_DATA_CRC_FAIL            ((uint32_t)0x00000002U)   /*!< Data block sent/received (CRC check failed)                   */
#define SDMMC_ERROR_CMD_RSP_TIMEOUT          ((uint32_t)0x00000004U)   /*!< Command response timeout                                      */
#define SDMMC_ERROR_DATA_TIMEOUT             ((uint32_t)0x00000008U)   /*!< Data timeout                                                  */
#define SDMMC_ERROR_TX_UNDERRUN              ((uint32_t)0x00000010U)   /*!< Transmit FIFO underrun                                        */
#define SDMMC_ERROR_RX_OVERRUN               ((uint32_t)0x00000020U)   /*!< Receive FIFO overrun                                          */
#define SDMMC_ERROR_ADDR_MISALIGNED          ((uint32_t)0x00000040U)   /*!< Misaligned address                                            */
#define SDMMC_ERROR_BLOCK_LEN_ERR            ((uint32_t)0x00000080U)   /*!< Transferred block length is not allowed for the card or the
                                                                            number of transferred bytes does not match the block length   */
#define SDMMC_ERROR_ERASE_SEQ_ERR            ((uint32_t)0x00000100U)   /*!< An error in the sequence of erase command occurs              */
#define SDMMC_ERROR_BAD_ERASE_PARAM          ((uint32_t)0x00000200U)   /*!< An invalid selection for erase groups                         */
#define SDMMC_ERROR_WRITE_PROT_VIOLATION     ((uint32_t)0x00000400U)   /*!< Attempt to program a write protect block                      */
#define SDMMC_ERROR_LOCK_UNLOCK_FAILED       ((uint32_t)0x00000800U)   /*!< Sequence or password error has been detected in unlock
                                                                            command or if there was an attempt to access a locked card    */
#define SDMMC_ERROR_COM_CRC_FAILED           ((uint32_t)0x00001000U)   /*!< CRC check of the previous command failed                      */
#define SDMMC_ERROR_ILLEGAL_CMD              ((uint32_t)0x00002000U)   /*!< Command is not legal for the card state                       */
#define SDMMC_ERROR_CARD_ECC_FAILED          ((uint32_t)0x00004000U)   /*!< Card internal ECC was applied but failed to correct the data  */
#define SDMMC_ERROR_CC_ERR                   ((uint32_t)0x00008000U)   /*!< Internal card controller error                                */
#define SDMMC_ERROR_GENERAL_UNKNOWN_ERR      ((uint32_t)0x00010000U)   /*!< General or unknown error                                      */
#define SDMMC_ERROR_STREAM_READ_UNDERRUN     ((uint32_t)0x00020000U)   /*!< The card could not sustain data reading in stream rmode       */
#define SDMMC_ERROR_STREAM_WRITE_OVERRUN     ((uint32_t)0x00040000U)   /*!< The card could not sustain data programming in stream mode    */
#define SDMMC_ERROR_CID_CSD_OVERWRITE        ((uint32_t)0x00080000U)   /*!< CID/CSD overwrite error                                       */
#define SDMMC_ERROR_WP_ERASE_SKIP            ((uint32_t)0x00100000U)   /*!< Only partial address space was erased                         */
#define SDMMC_ERROR_CARD_ECC_DISABLED        ((uint32_t)0x00200000U)   /*!< Command has been executed without using internal ECC          */
#define SDMMC_ERROR_ERASE_RESET              ((uint32_t)0x00400000U)   /*!< Erase sequence was cleared before executing because an out
                                                                            of erase sequence command was received                        */
#define SDMMC_ERROR_AKE_SEQ_ERR              ((uint32_t)0x00800000U)   /*!< Error in sequence of authentication                           */
#define SDMMC_ERROR_INVALID_VOLTRANGE        ((uint32_t)0x01000000U)   /*!< Error in case of invalid voltage range                        */
#define SDMMC_ERROR_ADDR_OUT_OF_RANGE        ((uint32_t)0x02000000U)   /*!< Error when addressed block is out of range                    */
#define SDMMC_ERROR_REQUEST_NOT_APPLICABLE   ((uint32_t)0x04000000U)   /*!< Error when command request is not applicable                  */
#define SDMMC_ERROR_INVALID_PARAMETER        ((uint32_t)0x08000000U)   /*!< the used parameter is not valid                               */
#define SDMMC_ERROR_UNSUPPORTED_FEATURE      ((uint32_t)0x10000000U)   /*!< Error when feature is not insupported                         */
#define SDMMC_ERROR_BUSY                     ((uint32_t)0x20000000U)   /*!< Error when transfer process is busy                           */
#define SDMMC_ERROR_DMA                      ((uint32_t)0x40000000U)   /*!< Error while DMA transfer                                      */
#define SDMMC_ERROR_TIMEOUT                  ((uint32_t)0x80000000U)   /*!< Timeout error                                                 */

/**
  * @brief SDMMC Commands Index
  */
#define SDMMC_CMD_GO_IDLE_STATE                       ((uint8_t)0U)   /*!< Resets the SD memory card.                                                               */
#define SDMMC_CMD_SEND_OP_COND                        ((uint8_t)1U)   /*!< Sends host capacity support information and activates the card's initialization process. */
#define SDMMC_CMD_ALL_SEND_CID                        ((uint8_t)2U)   /*!< Asks any card connected to the host to send the CID numbers on the CMD line.             */
#define SDMMC_CMD_SET_REL_ADDR                        ((uint8_t)3U)   /*!< Asks the card to publish a new relative address (RCA).                                   */
#define SDMMC_CMD_SET_DSR                             ((uint8_t)4U)   /*!< Programs the DSR of all cards.                                                           */
#define SDMMC_CMD_SDMMC_SEN_OP_COND                   ((uint8_t)5U)   /*!< Sends host capacity support information (HCS) and asks the accessed card to send its
                                                                           operating condition register (OCR) content in the response on the CMD line.                  */
#define SDMMC_CMD_HS_SWITCH                           ((uint8_t)6U)   /*!< Checks switchable function (mode 0) and switch card function (mode 1).                   */
#define SDMMC_CMD_SEL_DESEL_CARD                      ((uint8_t)7U)   /*!< Selects the card by its own relative address and gets deselected by any other address    */
#define SDMMC_CMD_HS_SEND_EXT_CSD                     ((uint8_t)8U)   /*!< Sends SD Memory Card interface condition, which includes host supply voltage information
                                                                           and asks the card whether card supports voltage.                                             */
#define SDMMC_CMD_SEND_CSD                            ((uint8_t)9U)   /*!< Addressed card sends its card specific data (CSD) on the CMD line.                       */
#define SDMMC_CMD_SEND_CID                            ((uint8_t)10U)  /*!< Addressed card sends its card identification (CID) on the CMD line.                      */
#if defined(STM32L4R5xx) || defined(STM32L4R7xx) || defined(STM32L4R9xx) || defined(STM32L4S5xx) || defined(STM32L4S7xx) || defined(STM32L4S9xx)
#define SDMMC_CMD_VOLTAGE_SWITCH                      ((uint8_t)11U)  /*!< SD card Voltage switch to 1.8V mode.                                                     */
#else
#define SDMMC_CMD_READ_DAT_UNTIL_STOP                 ((uint8_t)11U)  /*!< SD card doesn't support it.                                                              */
#endif
#define SDMMC_CMD_STOP_TRANSMISSION                   ((uint8_t)12U)  /*!< Forces the card to stop transmission.                                                    */
#define SDMMC_CMD_SEND_STATUS                         ((uint8_t)13U)  /*!< Addressed card sends its status register.                                                */
#define SDMMC_CMD_HS_BUSTEST_READ                     ((uint8_t)14U)  /*!< Reserved                                                                                 */
#define SDMMC_CMD_GO_INACTIVE_STATE                   ((uint8_t)15U)  /*!< Sends an addressed card into the inactive state.                                         */
#define SDMMC_CMD_SET_BLOCKLEN                        ((uint8_t)16U)  /*!< Sets the block length (in bytes for SDSC) for all following block commands
                                                                           (read, write, lock). Default block length is fixed to 512 Bytes. Not effective
                                                                           for SDHS and SDXC.                                                                       */
#define SDMMC_CMD_READ_SINGLE_BLOCK                   ((uint8_t)17U)  /*!< Reads single block of size selected by SET_BLOCKLEN in case of SDSC, and a block of
                                                                           fixed 512 bytes in case of SDHC and SDXC.                                                */
#define SDMMC_CMD_READ_MULT_BLOCK                     ((uint8_t)18U)  /*!< Continuously transfers data blocks from card to host until interrupted by
                                                                           STOP_TRANSMISSION command.                                                               */
#define SDMMC_CMD_HS_BUSTEST_WRITE                    ((uint8_t)19U)  /*!< 64 bytes tuning pattern is sent for SDR50 and SDR104.                                    */
#define SDMMC_CMD_WRITE_DAT_UNTIL_STOP                ((uint8_t)20U)  /*!< Speed class control command.                                                             */
#define SDMMC_CMD_SET_BLOCK_COUNT                     ((uint8_t)23U)  /*!< Specify block count for CMD18 and CMD25.                                                 */
#define SDMMC_CMD_WRITE_SINGLE_BLOCK                  ((uint8_t)24U)  /*!< Writes single block of size selected by SET_BLOCKLEN in case of SDSC, and a block of
                                                                           fixed 512 bytes in case of SDHC and SDXC.                                                */
#define SDMMC_CMD_WRITE_MULT_BLOCK                    ((uint8_t)25U)  /*!< Continuously writes blocks of data until a STOP_TRANSMISSION follows.                    */
#define SDMMC_CMD_PROG_CID                            ((uint8_t)26U)  /*!< Reserved for manufacturers.                                                              */
#define SDMMC_CMD_PROG_CSD                            ((uint8_t)27U)  /*!< Programming of the programmable bits of the CSD.                                         */
#define SDMMC_CMD_SET_WRITE_PROT                      ((uint8_t)28U)  /*!< Sets the write protection bit of the addressed group.                                    */
#define SDMMC_CMD_CLR_WRITE_PROT                      ((uint8_t)29U)  /*!< Clears the write protection bit of the addressed group.                                  */
#define SDMMC_CMD_SEND_WRITE_PROT                     ((uint8_t)30U)  /*!< Asks the card to send the status of the write protection bits.                           */
#define SDMMC_CMD_SD_ERASE_GRP_START                  ((uint8_t)32U)  /*!< Sets the address of the first write block to be erased. (For SD card only).              */
#define SDMMC_CMD_SD_ERASE_GRP_END                    ((uint8_t)33U)  /*!< Sets the address of the last write block of the continuous range to be erased.           */
#define SDMMC_CMD_ERASE_GRP_START                     ((uint8_t)35U)  /*!< Sets the address of the first write block to be erased. Reserved for each command
                                                                           system set by switch function command (CMD6).                                            */
#define SDMMC_CMD_ERASE_GRP_END                       ((uint8_t)36U)  /*!< Sets the address of the last write block of the continuous range to be erased.
                                                                           Reserved for each command system set by switch function command (CMD6).                  */
#define SDMMC_CMD_ERASE                               ((uint8_t)38U)  /*!< Reserved for SD security applications.                                                   */
#define SDMMC_CMD_FAST_IO                             ((uint8_t)39U)  /*!< SD card doesn't support it (Reserved).                                                   */
#define SDMMC_CMD_GO_IRQ_STATE                        ((uint8_t)40U)  /*!< SD card doesn't support it (Reserved).                                                   */
#define SDMMC_CMD_LOCK_UNLOCK                         ((uint8_t)42U)  /*!< Sets/resets the password or lock/unlock the card. The size of the data block is set by
                                                                           the SET_BLOCK_LEN command.                                                               */
#define SDMMC_CMD_APP_CMD                             ((uint8_t)55U)  /*!< Indicates to the card that the next command is an application specific command rather
                                                                           than a standard command.                                                                 */
#define SDMMC_CMD_GEN_CMD                             ((uint8_t)56U)  /*!< Used either to transfer a data block to the card or to get a data block from the card
                                                                           for general purpose/application specific commands.                                       */
#define SDMMC_CMD_NO_CMD                              ((uint8_t)64U)  /*!< No command                                                                               */

/**
  * @brief Following commands are SD Card Specific commands.
  *        SDMMC_APP_CMD should be sent before sending these commands.
  */
#define SDMMC_CMD_APP_SD_SET_BUSWIDTH                 ((uint8_t)6U)   /*!< (ACMD6) Defines the data bus width to be used for data transfer. The allowed data bus
                                                                            widths are given in SCR register.                                                       */
#define SDMMC_CMD_SD_APP_STATUS                       ((uint8_t)13U)  /*!< (ACMD13) Sends the SD status.                                                            */
#define SDMMC_CMD_SD_APP_SEND_NUM_WRITE_BLOCKS        ((uint8_t)22U)  /*!< (ACMD22) Sends the number of the written (without errors) write blocks. Responds with
                                                                           32bit+CRC data block.                                                                    */
#define SDMMC_CMD_SD_APP_OP_COND                      ((uint8_t)41U)  /*!< (ACMD41) Sends host capacity support information (HCS) and asks the accessed card to
                                                                           send its operating condition register (OCR) content in the response on the CMD line.     */
#define SDMMC_CMD_SD_APP_SET_CLR_CARD_DETECT          ((uint8_t)42U)  /*!< (ACMD42) Connect/Disconnect the 50 KOhm pull-up resistor on CD/DAT3 (pin 1) of the card  */
#define SDMMC_CMD_SD_APP_SEND_SCR                     ((uint8_t)51U)  /*!< Reads the SD Configuration Register (SCR).                                               */
#define SDMMC_CMD_SDMMC_RW_DIRECT                     ((uint8_t)52U)  /*!< For SD I/O card only, reserved for security specification.                               */
#define SDMMC_CMD_SDMMC_RW_EXTENDED                   ((uint8_t)53U)  /*!< For SD I/O card only, reserved for security specification.                               */

/**
  * @brief Following commands are SD Card Specific security commands.
  *        SDMMC_CMD_APP_CMD should be sent before sending these commands.
  */
#define SDMMC_CMD_SD_APP_GET_MKB                      ((uint8_t)43U)
#define SDMMC_CMD_SD_APP_GET_MID                      ((uint8_t)44U)
#define SDMMC_CMD_SD_APP_SET_CER_RN1                  ((uint8_t)45U)
#define SDMMC_CMD_SD_APP_GET_CER_RN2                  ((uint8_t)46U)
#define SDMMC_CMD_SD_APP_SET_CER_RES2                 ((uint8_t)47U)
#define SDMMC_CMD_SD_APP_GET_CER_RES1                 ((uint8_t)48U)
#define SDMMC_CMD_SD_APP_SECURE_READ_MULTIPLE_BLOCK   ((uint8_t)18U)
#define SDMMC_CMD_SD_APP_SECURE_WRITE_MULTIPLE_BLOCK  ((uint8_t)25U)
#define SDMMC_CMD_SD_APP_SECURE_ERASE                 ((uint8_t)38U)
#define SDMMC_CMD_SD_APP_CHANGE_SECURE_AREA           ((uint8_t)49U)
#define SDMMC_CMD_SD_APP_SECURE_WRITE_MKB             ((uint8_t)48U)

/**
  * @brief  Masks for errors Card Status R1 (OCR Register)
  */
#define SDMMC_OCR_ADDR_OUT_OF_RANGE        ((uint32_t)0x80000000U)
#define SDMMC_OCR_ADDR_MISALIGNED          ((uint32_t)0x40000000U)
#define SDMMC_OCR_BLOCK_LEN_ERR            ((uint32_t)0x20000000U)
#define SDMMC_OCR_ERASE_SEQ_ERR            ((uint32_t)0x10000000U)
#define SDMMC_OCR_BAD_ERASE_PARAM          ((uint32_t)0x08000000U)
#define SDMMC_OCR_WRITE_PROT_VIOLATION     ((uint32_t)0x04000000U)
#define SDMMC_OCR_LOCK_UNLOCK_FAILED       ((uint32_t)0x01000000U)
#define SDMMC_OCR_COM_CRC_FAILED           ((uint32_t)0x00800000U)
#define SDMMC_OCR_ILLEGAL_CMD              ((uint32_t)0x00400000U)
#define SDMMC_OCR_CARD_ECC_FAILED          ((uint32_t)0x00200000U)
#define SDMMC_OCR_CC_ERROR                 ((uint32_t)0x00100000U)
#define SDMMC_OCR_GENERAL_UNKNOWN_ERROR    ((uint32_t)0x00080000U)
#define SDMMC_OCR_STREAM_READ_UNDERRUN     ((uint32_t)0x00040000U)
#define SDMMC_OCR_STREAM_WRITE_OVERRUN     ((uint32_t)0x00020000U)
#define SDMMC_OCR_CID_CSD_OVERWRITE        ((uint32_t)0x00010000U)
#define SDMMC_OCR_WP_ERASE_SKIP            ((uint32_t)0x00008000U)
#define SDMMC_OCR_CARD_ECC_DISABLED        ((uint32_t)0x00004000U)
#define SDMMC_OCR_ERASE_RESET              ((uint32_t)0x00002000U)
#define SDMMC_OCR_AKE_SEQ_ERROR            ((uint32_t)0x00000008U)
#define SDMMC_OCR_ERRORBITS                ((uint32_t)0xFDFFE008U)

/**
  * @brief  Masks for R6 Response
  */
#define SDMMC_R6_GENERAL_UNKNOWN_ERROR     ((uint32_t)0x00002000U)
#define SDMMC_R6_ILLEGAL_CMD               ((uint32_t)0x00004000U)
#define SDMMC_R6_COM_CRC_FAILED            ((uint32_t)0x00008000U)

#define SDMMC_VOLTAGE_WINDOW_SD            ((uint32_t)0x80100000U)
#define SDMMC_HIGH_CAPACITY                ((uint32_t)0x40000000U)
#define SDMMC_STD_CAPACITY                 ((uint32_t)0x00000000U)
#define SDMMC_CHECK_PATTERN                ((uint32_t)0x000001AAU)
#define SD_SWITCH_1_8V_CAPACITY            ((uint32_t)0x01000000U)
#if defined(STM32L4R5xx) || defined(STM32L4R7xx) || defined(STM32L4R9xx) || defined(STM32L4S5xx) || defined(STM32L4S7xx) || defined(STM32L4S9xx)
#define SDMMC_DDR50_SWITCH_PATTERN         ((uint32_t)0x80FFFF04U)
#define SDMMC_SDR104_SWITCH_PATTERN        ((uint32_t)0x80FF1F03U)
#define SDMMC_SDR50_SWITCH_PATTERN         ((uint32_t)0x80FF1F02U)
#define SDMMC_SDR25_SWITCH_PATTERN         ((uint32_t)0x80FFFF01U)
#endif /* STM32L4R5xx || STM32L4R7xx || STM32L4R9xx || STM32L4S5xx || STM32L4S7xx || STM32L4S9xx */

#define SDMMC_MAX_VOLT_TRIAL               ((uint32_t)0x0000FFFFU)

#define SDMMC_MAX_TRIAL                    ((uint32_t)0x0000FFFFU)

#define SDMMC_ALLZERO                      ((uint32_t)0x00000000U)

#define SDMMC_WIDE_BUS_SUPPORT             ((uint32_t)0x00040000U)
#define SDMMC_SINGLE_BUS_SUPPORT           ((uint32_t)0x00010000U)
#define SDMMC_CARD_LOCKED                  ((uint32_t)0x02000000U)

#define SDMMC_DATATIMEOUT                  ((uint32_t)0xFFFFFFFFU)

#define SDMMC_0TO7BITS                     ((uint32_t)0x000000FFU)
#define SDMMC_8TO15BITS                    ((uint32_t)0x0000FF00U)
#define SDMMC_16TO23BITS                   ((uint32_t)0x00FF0000U)
#define SDMMC_24TO31BITS                   ((uint32_t)0xFF000000U)
#define SDMMC_MAX_DATA_LENGTH              ((uint32_t)0x01FFFFFFU)

#define SDMMC_HALFFIFO                     ((uint32_t)0x00000008U)
#define SDMMC_HALFFIFOBYTES                ((uint32_t)0x00000020U)

/**
  * @brief  Command Class supported
  */
#define SDMMC_CCCC_ERASE                   ((uint32_t)0x00000020U)

#define SDMMC_CMDTIMEOUT                   ((uint32_t)5000U)        /* Command send and response timeout     */
#define SDMMC_MAXERASETIMEOUT              ((uint32_t)63000U)       /* Max erase Timeout 63 s                */
#define SDMMC_STOPTRANSFERTIMEOUT          ((uint32_t)100000000U)   /* Timeout for STOP TRANSMISSION command */

/** @defgroup SDMMC_LL_Clock_Edge Clock Edge
  * @{
  */
#define SDMMC_CLOCK_EDGE_RISING               ((uint32_t)0x00000000U)
#define SDMMC_CLOCK_EDGE_FALLING              SDMMC_CLKCR_NEGEDGE

#define IS_SDMMC_CLOCK_EDGE(EDGE) (((EDGE) == SDMMC_CLOCK_EDGE_RISING) || \
                                   ((EDGE) == SDMMC_CLOCK_EDGE_FALLING))
/**
  * @}
  */

#if !defined(STM32L4R5xx) && !defined(STM32L4R7xx) && !defined(STM32L4R9xx) && !defined(STM32L4S5xx) && !defined(STM32L4S7xx) && !defined(STM32L4S9xx)
/** @defgroup SDMMC_LL_Clock_Bypass Clock Bypass
  * @{
  */
#define SDMMC_CLOCK_BYPASS_DISABLE             ((uint32_t)0x00000000U)
#define SDMMC_CLOCK_BYPASS_ENABLE              SDMMC_CLKCR_BYPASS

#define IS_SDMMC_CLOCK_BYPASS(BYPASS) (((BYPASS) == SDMMC_CLOCK_BYPASS_DISABLE) || \
                                       ((BYPASS) == SDMMC_CLOCK_BYPASS_ENABLE))
/**
  * @}
  */
#endif /* STM32L4R5xx || STM32L4R7xx || STM32L4R9xx || STM32L4S5xx || STM32L4S7xx || STM32L4S9xx */

/** @defgroup SDMMC_LL_Clock_Power_Save Clock Power Saving
  * @{
  */
#define SDMMC_CLOCK_POWER_SAVE_DISABLE         ((uint32_t)0x00000000U)
#define SDMMC_CLOCK_POWER_SAVE_ENABLE          SDMMC_CLKCR_PWRSAV

#define IS_SDMMC_CLOCK_POWER_SAVE(SAVE) (((SAVE) == SDMMC_CLOCK_POWER_SAVE_DISABLE) || \
                                         ((SAVE) == SDMMC_CLOCK_POWER_SAVE_ENABLE))
/**
  * @}
  */

/** @defgroup SDMMC_LL_Bus_Wide Bus Width
  * @{
  */
#define SDMMC_BUS_WIDE_1B                      ((uint32_t)0x00000000U)
#define SDMMC_BUS_WIDE_4B                      SDMMC_CLKCR_WIDBUS_0
#define SDMMC_BUS_WIDE_8B                      SDMMC_CLKCR_WIDBUS_1

#define IS_SDMMC_BUS_WIDE(WIDE) (((WIDE) == SDMMC_BUS_WIDE_1B) || \
                                 ((WIDE) == SDMMC_BUS_WIDE_4B) || \
                                 ((WIDE) == SDMMC_BUS_WIDE_8B))
/**
  * @}
  */

#if defined(STM32L4R5xx) || defined(STM32L4R7xx) || defined(STM32L4R9xx) || defined(STM32L4S5xx) || defined(STM32L4S7xx) || defined(STM32L4S9xx)
/** @defgroup SDMMC_LL_Speed_Mode
  * @{
  */
#define SDMMC_SPEED_MODE_AUTO                  ((uint32_t)0x00000000U)
#define SDMMC_SPEED_MODE_DEFAULT               ((uint32_t)0x00000001U)
#define SDMMC_SPEED_MODE_HIGH                  ((uint32_t)0x00000002U)
#define SDMMC_SPEED_MODE_ULTRA                 ((uint32_t)0x00000003U)
#define SDMMC_SPEED_MODE_DDR                   ((uint32_t)0x00000004U)

#define IS_SDMMC_SPEED_MODE(MODE) (((MODE) == SDMMC_SPEED_MODE_AUTO) || \
                                ((MODE) == SDMMC_SPEED_MODE_DEFAULT) || \
                                ((MODE) == SDMMC_SPEED_MODE_HIGH) || \
                                ((MODE) == SDMMC_SPEED_MODE_ULTRA) || \
                                ((MODE) == SDMMC_SPEED_MODE_DDR))

/**
  * @}
  */
#endif /* STM32L4R5xx || STM32L4R7xx || STM32L4R9xx || STM32L4S5xx || STM32L4S7xx || STM32L4S9xx */

/** @defgroup SDMMC_LL_Hardware_Flow_Control Hardware Flow Control
  * @{
  */
#define SDMMC_HARDWARE_FLOW_CONTROL_DISABLE    ((uint32_t)0x00000000U)
#define SDMMC_HARDWARE_FLOW_CONTROL_ENABLE     SDMMC_CLKCR_HWFC_EN

#define IS_SDMMC_HARDWARE_FLOW_CONTROL(CONTROL) (((CONTROL) == SDMMC_HARDWARE_FLOW_CONTROL_DISABLE) || \
                                                 ((CONTROL) == SDMMC_HARDWARE_FLOW_CONTROL_ENABLE))
/**
  * @}
  */

/** @defgroup SDMMC_LL_Clock_Division Clock Division
  * @{
  */
#if defined(STM32L4R5xx) || defined(STM32L4R7xx) || defined(STM32L4R9xx) || defined(STM32L4S5xx) || defined(STM32L4S7xx) || defined(STM32L4S9xx)
/* SDMMC_CK frequency = SDMMCCLK / [2 * CLKDIV] */
#define IS_SDMMC_CLKDIV(DIV)   ((DIV) < 0x400U)
#else
#define IS_SDMMC_CLKDIV(DIV)   ((DIV) <= 0xFFU)
#endif /* STM32L4R5xx || STM32L4R7xx || STM32L4R9xx || STM32L4S5xx || STM32L4S7xx || STM32L4S9xx */
/**
  * @}
  */

#if defined(STM32L4R5xx) || defined(STM32L4R7xx) || defined(STM32L4R9xx) || defined(STM32L4S5xx) || defined(STM32L4S7xx) || defined(STM32L4S9xx)
/** @defgroup SDMMC_LL_Transceiver Transceiver
  * @{
  */
#define SDMMC_TRANSCEIVER_DISABLE    ((uint32_t)0x00000000U)
#define SDMMC_TRANSCEIVER_ENABLE     ((uint32_t)0x00000001U)

#define IS_SDMMC_TRANSCEIVER(MODE) (((MODE) == SDMMC_TRANSCEIVER_DISABLE) || \
                                    ((MODE) == SDMMC_TRANSCEIVER_ENABLE))
/**
  * @}
  */
#endif /* STM32L4R5xx || STM32L4R7xx || STM32L4R9xx || STM32L4S5xx || STM32L4S7xx || STM32L4S9xx */

/** @defgroup SDMMC_LL_Command_Index Command Index
  * @{
  */
#define IS_SDMMC_CMD_INDEX(INDEX)            ((INDEX) < 0x40U)
/**
  * @}
  */

/** @defgroup SDMMC_LL_Response_Type Response Type
  * @{
  */
#define SDMMC_RESPONSE_NO                    ((uint32_t)0x00000000U)
#define SDMMC_RESPONSE_SHORT                 SDMMC_CMD_WAITRESP_0
#define SDMMC_RESPONSE_LONG                  SDMMC_CMD_WAITRESP

#define IS_SDMMC_RESPONSE(RESPONSE) (((RESPONSE) == SDMMC_RESPONSE_NO)    || \
                                     ((RESPONSE) == SDMMC_RESPONSE_SHORT) || \
                                     ((RESPONSE) == SDMMC_RESPONSE_LONG))
/**
  * @}
  */

/** @defgroup SDMMC_LL_Wait_Interrupt_State Wait Interrupt
  * @{
  */
#define SDMMC_WAIT_NO                        ((uint32_t)0x00000000U)
#define SDMMC_WAIT_IT                        SDMMC_CMD_WAITINT
#define SDMMC_WAIT_PEND                      SDMMC_CMD_WAITPEND

#define IS_SDMMC_WAIT(WAIT) (((WAIT) == SDMMC_WAIT_NO) || \
                             ((WAIT) == SDMMC_WAIT_IT) || \
                             ((WAIT) == SDMMC_WAIT_PEND))
/**
  * @}
  */

/** @defgroup SDMMC_LL_CPSM_State CPSM State
  * @{
  */
#define SDMMC_CPSM_DISABLE                   ((uint32_t)0x00000000U)
#define SDMMC_CPSM_ENABLE                    SDMMC_CMD_CPSMEN

#define IS_SDMMC_CPSM(CPSM) (((CPSM) == SDMMC_CPSM_DISABLE) || \
                             ((CPSM) == SDMMC_CPSM_ENABLE))
/**
  * @}
  */

/** @defgroup SDMMC_LL_Response_Registers Response Register
  * @{
  */
#define SDMMC_RESP1                          ((uint32_t)0x00000000U)
#define SDMMC_RESP2                          ((uint32_t)0x00000004U)
#define SDMMC_RESP3                          ((uint32_t)0x00000008U)
#define SDMMC_RESP4                          ((uint32_t)0x0000000CU)

#define IS_SDMMC_RESP(RESP) (((RESP) == SDMMC_RESP1) || \
                             ((RESP) == SDMMC_RESP2) || \
                             ((RESP) == SDMMC_RESP3) || \
                             ((RESP) == SDMMC_RESP4))

#if defined(STM32L4R5xx) || defined(STM32L4R7xx) || defined(STM32L4R9xx) || defined(STM32L4S5xx) || defined(STM32L4S7xx) || defined(STM32L4S9xx)
/** @defgroup SDMMC_Internal_DMA_Mode  SDMMC Internal DMA Mode
  * @{
  */
#define SDMMC_DISABLE_IDMA              ((uint32_t)0x00000000)
#define SDMMC_ENABLE_IDMA_SINGLE_BUFF   (SDMMC_IDMA_IDMAEN)
#define SDMMC_ENABLE_IDMA_DOUBLE_BUFF0  (SDMMC_IDMA_IDMAEN | SDMMC_IDMA_IDMABMODE)
#define SDMMC_ENABLE_IDMA_DOUBLE_BUFF1  (SDMMC_IDMA_IDMAEN | SDMMC_IDMA_IDMABMODE | SDMMC_IDMA_IDMABACT)

/**
  * @}
  */
#endif /* STM32L4R5xx || STM32L4R7xx || STM32L4R9xx || STM32L4S5xx || STM32L4S7xx || STM32L4S9xx */

/** @defgroup SDMMC_LL_Data_Length Data Lenght
  * @{
  */
#define IS_SDMMC_DATA_LENGTH(LENGTH) ((LENGTH) <= 0x01FFFFFFU)
/**
  * @}
  */

/** @defgroup SDMMC_LL_Data_Block_Size  Data Block Size
  * @{
  */
#define SDMMC_DATABLOCK_SIZE_1B               ((uint32_t)0x00000000U)
#define SDMMC_DATABLOCK_SIZE_2B               SDMMC_DCTRL_DBLOCKSIZE_0
#define SDMMC_DATABLOCK_SIZE_4B               SDMMC_DCTRL_DBLOCKSIZE_1
#define SDMMC_DATABLOCK_SIZE_8B               (SDMMC_DCTRL_DBLOCKSIZE_0|SDMMC_DCTRL_DBLOCKSIZE_1)
#define SDMMC_DATABLOCK_SIZE_16B              SDMMC_DCTRL_DBLOCKSIZE_2
#define SDMMC_DATABLOCK_SIZE_32B              (SDMMC_DCTRL_DBLOCKSIZE_0|SDMMC_DCTRL_DBLOCKSIZE_2)
#define SDMMC_DATABLOCK_SIZE_64B              (SDMMC_DCTRL_DBLOCKSIZE_1|SDMMC_DCTRL_DBLOCKSIZE_2)
#define SDMMC_DATABLOCK_SIZE_128B             (SDMMC_DCTRL_DBLOCKSIZE_0|SDMMC_DCTRL_DBLOCKSIZE_1|SDMMC_DCTRL_DBLOCKSIZE_2)
#define SDMMC_DATABLOCK_SIZE_256B             SDMMC_DCTRL_DBLOCKSIZE_3
#define SDMMC_DATABLOCK_SIZE_512B             (SDMMC_DCTRL_DBLOCKSIZE_0|SDMMC_DCTRL_DBLOCKSIZE_3)
#define SDMMC_DATABLOCK_SIZE_1024B            (SDMMC_DCTRL_DBLOCKSIZE_1|SDMMC_DCTRL_DBLOCKSIZE_3)
#define SDMMC_DATABLOCK_SIZE_2048B            (SDMMC_DCTRL_DBLOCKSIZE_0|SDMMC_DCTRL_DBLOCKSIZE_1|SDMMC_DCTRL_DBLOCKSIZE_3)
#define SDMMC_DATABLOCK_SIZE_4096B            (SDMMC_DCTRL_DBLOCKSIZE_2|SDMMC_DCTRL_DBLOCKSIZE_3)
#define SDMMC_DATABLOCK_SIZE_8192B            (SDMMC_DCTRL_DBLOCKSIZE_0|SDMMC_DCTRL_DBLOCKSIZE_2|SDMMC_DCTRL_DBLOCKSIZE_3)
#define SDMMC_DATABLOCK_SIZE_16384B           (SDMMC_DCTRL_DBLOCKSIZE_1|SDMMC_DCTRL_DBLOCKSIZE_2|SDMMC_DCTRL_DBLOCKSIZE_3)

#define IS_SDMMC_BLOCK_SIZE(SIZE) (((SIZE) == SDMMC_DATABLOCK_SIZE_1B)    || \
                                   ((SIZE) == SDMMC_DATABLOCK_SIZE_2B)    || \
                                   ((SIZE) == SDMMC_DATABLOCK_SIZE_4B)    || \
                                   ((SIZE) == SDMMC_DATABLOCK_SIZE_8B)    || \
                                   ((SIZE) == SDMMC_DATABLOCK_SIZE_16B)   || \
                                   ((SIZE) == SDMMC_DATABLOCK_SIZE_32B)   || \
                                   ((SIZE) == SDMMC_DATABLOCK_SIZE_64B)   || \
                                   ((SIZE) == SDMMC_DATABLOCK_SIZE_128B)  || \
                                   ((SIZE) == SDMMC_DATABLOCK_SIZE_256B)  || \
                                   ((SIZE) == SDMMC_DATABLOCK_SIZE_512B)  || \
                                   ((SIZE) == SDMMC_DATABLOCK_SIZE_1024B) || \
                                   ((SIZE) == SDMMC_DATABLOCK_SIZE_2048B) || \
                                   ((SIZE) == SDMMC_DATABLOCK_SIZE_4096B) || \
                                   ((SIZE) == SDMMC_DATABLOCK_SIZE_8192B) || \
                                   ((SIZE) == SDMMC_DATABLOCK_SIZE_16384B))
/**
  * @}
  */

/** @defgroup SDMMC_LL_Transfer_Direction Transfer Direction
  * @{
  */
#define SDMMC_TRANSFER_DIR_TO_CARD            ((uint32_t)0x00000000U)
#define SDMMC_TRANSFER_DIR_TO_SDMMC            SDMMC_DCTRL_DTDIR

#define IS_SDMMC_TRANSFER_DIR(DIR) (((DIR) == SDMMC_TRANSFER_DIR_TO_CARD) || \
                                    ((DIR) == SDMMC_TRANSFER_DIR_TO_SDMMC))
/**
  * @}
  */

/** @defgroup SDMMC_LL_Transfer_Type Transfer Type
  * @{
  */
#define SDMMC_TRANSFER_MODE_BLOCK             ((uint32_t)0x00000000U)
#if defined(STM32L4R5xx) || defined(STM32L4R7xx) || defined(STM32L4R9xx) || defined(STM32L4S5xx) || defined(STM32L4S7xx) || defined(STM32L4S9xx)
#define SDMMC_TRANSFER_MODE_STREAM            SDMMC_DCTRL_DTMODE_1
#else
#define SDMMC_TRANSFER_MODE_STREAM            SDMMC_DCTRL_DTMODE
#endif /* STM32L4R5xx || STM32L4R7xx || STM32L4R9xx || STM32L4S5xx || STM32L4S7xx || STM32L4S9xx */

#define IS_SDMMC_TRANSFER_MODE(MODE) (((MODE) == SDMMC_TRANSFER_MODE_BLOCK) || \
                                      ((MODE) == SDMMC_TRANSFER_MODE_STREAM))
/**
  * @}
  */

/** @defgroup SDMMC_LL_DPSM_State DPSM State
  * @{
  */
#define SDMMC_DPSM_DISABLE                    ((uint32_t)0x00000000U)
#define SDMMC_DPSM_ENABLE                     SDMMC_DCTRL_DTEN

#define IS_SDMMC_DPSM(DPSM) (((DPSM) == SDMMC_DPSM_DISABLE) ||\
                             ((DPSM) == SDMMC_DPSM_ENABLE))
/**
  * @}
  */

/** @defgroup SDMMC_LL_Read_Wait_Mode Read Wait Mode
  * @{
  */
#define SDMMC_READ_WAIT_MODE_DATA2                ((uint32_t)0x00000000U)
#define SDMMC_READ_WAIT_MODE_CLK                  (SDMMC_DCTRL_RWMOD)

#define IS_SDMMC_READWAIT_MODE(MODE) (((MODE) == SDMMC_READ_WAIT_MODE_CLK) || \
                                      ((MODE) == SDMMC_READ_WAIT_MODE_DATA2))
/**
  * @}
  */

/** @defgroup SDMMC_LL_Interrupt_sources Interrupt Sources
  * @{
  */
#define SDMMC_IT_CCRCFAIL                  SDMMC_MASK_CCRCFAILIE
#define SDMMC_IT_DCRCFAIL                  SDMMC_MASK_DCRCFAILIE
#define SDMMC_IT_CTIMEOUT                  SDMMC_MASK_CTIMEOUTIE
#define SDMMC_IT_DTIMEOUT                  SDMMC_MASK_DTIMEOUTIE
#define SDMMC_IT_TXUNDERR                  SDMMC_MASK_TXUNDERRIE
#define SDMMC_IT_RXOVERR                   SDMMC_MASK_RXOVERRIE
#define SDMMC_IT_CMDREND                   SDMMC_MASK_CMDRENDIE
#define SDMMC_IT_CMDSENT                   SDMMC_MASK_CMDSENTIE
#define SDMMC_IT_DATAEND                   SDMMC_MASK_DATAENDIE
#if defined(STM32L4R5xx) || defined(STM32L4R7xx) || defined(STM32L4R9xx) || defined(STM32L4S5xx) || defined(STM32L4S7xx) || defined(STM32L4S9xx)
#define SDMMC_IT_DHOLD                     SDMMC_MASK_DHOLDIE
#endif
#define SDMMC_IT_DBCKEND                   SDMMC_MASK_DBCKENDIE
#if !defined(STM32L4R5xx) && !defined(STM32L4R7xx) && !defined(STM32L4R9xx) && !defined(STM32L4S5xx) && !defined(STM32L4S7xx) && !defined(STM32L4S9xx)
#define SDMMC_IT_CMDACT                    SDMMC_MASK_CMDACTIE
#define SDMMC_IT_TXACT                     SDMMC_MASK_TXACTIE
#define SDMMC_IT_RXACT                     SDMMC_MASK_RXACTIE
#else
#define SDMMC_IT_DABORT                    SDMMC_MASK_DABORTIE
#endif
#define SDMMC_IT_TXFIFOHE                  SDMMC_MASK_TXFIFOHEIE
#define SDMMC_IT_RXFIFOHF                  SDMMC_MASK_RXFIFOHFIE
#if !defined(STM32L4R5xx) && !defined(STM32L4R7xx) && !defined(STM32L4R9xx) && !defined(STM32L4S5xx) && !defined(STM32L4S7xx) && !defined(STM32L4S9xx)
#define SDMMC_IT_TXFIFOF                   SDMMC_MASK_TXFIFOFIE
#endif
#define SDMMC_IT_RXFIFOF                   SDMMC_MASK_RXFIFOFIE
#define SDMMC_IT_TXFIFOE                   SDMMC_MASK_TXFIFOEIE
#if defined(STM32L4R5xx) || defined(STM32L4R7xx) || defined(STM32L4R9xx) || defined(STM32L4S5xx) || defined(STM32L4S7xx) || defined(STM32L4S9xx)
#define SDMMC_IT_BUSYD0END                 SDMMC_MASK_BUSYD0ENDIE
#else
#define SDMMC_IT_RXFIFOE                     SDMMC_MASK_RXFIFOEIE
#define SDMMC_IT_TXDAVL                      SDMMC_MASK_TXDAVLIE
#define SDMMC_IT_RXDAVL                      SDMMC_MASK_RXDAVLIE
#endif
#define SDMMC_IT_SDIOIT                    SDMMC_MASK_SDIOITIE
#if defined(STM32L4R5xx) || defined(STM32L4R7xx) || defined(STM32L4R9xx) || defined(STM32L4S5xx) || defined(STM32L4S7xx) || defined(STM32L4S9xx)
#define SDMMC_IT_ACKFAIL                   SDMMC_MASK_ACKFAILIE
#define SDMMC_IT_ACKTIMEOUT                SDMMC_MASK_ACKTIMEOUTIE
#define SDMMC_IT_VSWEND                    SDMMC_MASK_VSWENDIE
#define SDMMC_IT_CKSTOP                    SDMMC_MASK_CKSTOPIE
#define SDMMC_IT_IDMABTC                   SDMMC_MASK_IDMABTCIE
#endif /* STM32L4R5xx || STM32L4R7xx || STM32L4R9xx || STM32L4S5xx || STM32L4S7xx || STM32L4S9xx */
/**
  * @}
  */

/** @defgroup SDMMC_LL_Flags Flags
  * @{
  */
#define SDMMC_FLAG_CCRCFAIL                  SDMMC_STA_CCRCFAIL
#define SDMMC_FLAG_DCRCFAIL                  SDMMC_STA_DCRCFAIL
#define SDMMC_FLAG_CTIMEOUT                  SDMMC_STA_CTIMEOUT
#define SDMMC_FLAG_DTIMEOUT                  SDMMC_STA_DTIMEOUT
#define SDMMC_FLAG_TXUNDERR                  SDMMC_STA_TXUNDERR
#define SDMMC_FLAG_RXOVERR                   SDMMC_STA_RXOVERR
#define SDMMC_FLAG_CMDREND                   SDMMC_STA_CMDREND
#define SDMMC_FLAG_CMDSENT                   SDMMC_STA_CMDSENT
#define SDMMC_FLAG_DATAEND                   SDMMC_STA_DATAEND
#if defined(STM32L4R5xx) || defined(STM32L4R7xx) || defined(STM32L4R9xx) || defined(STM32L4S5xx) || defined(STM32L4S7xx) || defined(STM32L4S9xx)
#define SDMMC_FLAG_DHOLD                     SDMMC_STA_DHOLD
#endif
#define SDMMC_FLAG_DBCKEND                   SDMMC_STA_DBCKEND
#if defined(STM32L4R5xx) || defined(STM32L4R7xx) || defined(STM32L4R9xx) || defined(STM32L4S5xx) || defined(STM32L4S7xx) || defined(STM32L4S9xx)
#define SDMMC_FLAG_DABORT                    SDMMC_STA_DABORT
#define SDMMC_FLAG_DPSMACT                   SDMMC_STA_DPSMACT
#define SDMMC_FLAG_CMDACT                    SDMMC_STA_CPSMACT
#else
#define SDMMC_FLAG_CMDACT                    SDMMC_STA_CMDACT
#define SDMMC_FLAG_TXACT                     SDMMC_STA_TXACT
#define SDMMC_FLAG_RXACT                     SDMMC_STA_RXACT
#endif
#define SDMMC_FLAG_TXFIFOHE                  SDMMC_STA_TXFIFOHE
#define SDMMC_FLAG_RXFIFOHF                  SDMMC_STA_RXFIFOHF
#define SDMMC_FLAG_TXFIFOF                   SDMMC_STA_TXFIFOF
#define SDMMC_FLAG_RXFIFOF                   SDMMC_STA_RXFIFOF
#define SDMMC_FLAG_TXFIFOE                   SDMMC_STA_TXFIFOE
#define SDMMC_FLAG_RXFIFOE                   SDMMC_STA_RXFIFOE
#if defined(STM32L4R5xx) || defined(STM32L4R7xx) || defined(STM32L4R9xx) || defined(STM32L4S5xx) || defined(STM32L4S7xx) || defined(STM32L4S9xx)
#define SDMMC_FLAG_BUSYD0                    SDMMC_STA_BUSYD0
#define SDMMC_FLAG_BUSYD0END                 SDMMC_STA_BUSYD0END
#else
#define SDMMC_FLAG_TXDAVL                    SDMMC_STA_TXDAVL
#define SDMMC_FLAG_RXDAVL                    SDMMC_STA_RXDAVL
#endif
#define SDMMC_FLAG_SDIOIT                    SDMMC_STA_SDIOIT
#if defined(STM32L4R5xx) || defined(STM32L4R7xx) || defined(STM32L4R9xx) || defined(STM32L4S5xx) || defined(STM32L4S7xx) || defined(STM32L4S9xx)
#define SDMMC_FLAG_ACKFAIL                   SDMMC_STA_ACKFAIL
#define SDMMC_FLAG_ACKTIMEOUT                SDMMC_STA_ACKTIMEOUT
#define SDMMC_FLAG_VSWEND                    SDMMC_STA_VSWEND
#define SDMMC_FLAG_CKSTOP                    SDMMC_STA_CKSTOP
#define SDMMC_FLAG_IDMATE                    SDMMC_STA_IDMATE
#define SDMMC_FLAG_IDMABTC                   SDMMC_STA_IDMABTC
#endif /* STM32L4R5xx || STM32L4R7xx || STM32L4R9xx || STM32L4S5xx || STM32L4S7xx || STM32L4S9xx */

#if defined(STM32L4R5xx) || defined(STM32L4R7xx) || defined(STM32L4R9xx) || defined(STM32L4S5xx) || defined(STM32L4S7xx) || defined(STM32L4S9xx)
#define SDMMC_STATIC_FLAGS                   ((uint32_t)(SDMMC_FLAG_CCRCFAIL   | SDMMC_FLAG_DCRCFAIL | SDMMC_FLAG_CTIMEOUT |\
                                                         SDMMC_FLAG_DTIMEOUT   | SDMMC_FLAG_TXUNDERR | SDMMC_FLAG_RXOVERR  |\
                                                         SDMMC_FLAG_CMDREND    | SDMMC_FLAG_CMDSENT  | SDMMC_FLAG_DATAEND  |\
                                                         SDMMC_FLAG_DHOLD      | SDMMC_FLAG_DBCKEND  | SDMMC_FLAG_DABORT   |\
                                                         SDMMC_FLAG_BUSYD0END  | SDMMC_FLAG_SDIOIT   | SDMMC_FLAG_ACKFAIL  |\
                                                         SDMMC_FLAG_ACKTIMEOUT | SDMMC_FLAG_VSWEND   | SDMMC_FLAG_CKSTOP   |\
                                                         SDMMC_FLAG_IDMATE     | SDMMC_FLAG_IDMABTC))

#define SDMMC_STATIC_CMD_FLAGS               ((uint32_t)(SDMMC_FLAG_CCRCFAIL | SDMMC_FLAG_CTIMEOUT  | SDMMC_FLAG_CMDREND |\
                                                         SDMMC_FLAG_CMDSENT  | SDMMC_FLAG_BUSYD0END))

#define SDMMC_STATIC_DATA_FLAGS              ((uint32_t)(SDMMC_FLAG_DCRCFAIL | SDMMC_FLAG_DTIMEOUT | SDMMC_FLAG_TXUNDERR |\
                                                         SDMMC_FLAG_RXOVERR  | SDMMC_FLAG_DATAEND  | SDMMC_FLAG_DHOLD    |\
                                                         SDMMC_FLAG_DBCKEND  | SDMMC_FLAG_DABORT   | SDMMC_FLAG_IDMATE   |\
                                                         SDMMC_FLAG_IDMABTC))

#else
#define SDMMC_STATIC_FLAGS                   ((uint32_t)(SDMMC_FLAG_CCRCFAIL | SDMMC_FLAG_DCRCFAIL | SDMMC_FLAG_CTIMEOUT |\
                                                         SDMMC_FLAG_DTIMEOUT | SDMMC_FLAG_TXUNDERR | SDMMC_FLAG_RXOVERR  |\
                                                         SDMMC_FLAG_CMDREND  | SDMMC_FLAG_CMDSENT  | SDMMC_FLAG_DATAEND  |\
                                                         SDMMC_FLAG_DBCKEND  | SDMMC_FLAG_SDIOIT))

#define SDMMC_STATIC_CMD_FLAGS               ((uint32_t)(SDMMC_FLAG_CCRCFAIL | SDMMC_FLAG_CTIMEOUT | SDMMC_FLAG_CMDREND |\
                                                         SDMMC_FLAG_CMDSENT))

#define SDMMC_STATIC_DATA_FLAGS              ((uint32_t)(SDMMC_FLAG_DCRCFAIL | SDMMC_FLAG_DTIMEOUT | SDMMC_FLAG_TXUNDERR |\
                                                         SDMMC_FLAG_RXOVERR  | SDMMC_FLAG_DATAEND  | SDMMC_FLAG_DBCKEND))
#endif /* STM32L4R5xx || STM32L4R7xx || STM32L4R9xx || STM32L4S5xx || STM32L4S7xx || STM32L4S9xx */

/**
  * @}
  */

/**
  * @}
  */

/* Exported macro ------------------------------------------------------------*/
/** @defgroup SDMMC_LL_Exported_macros SDMMC_LL Exported Macros
  * @{
  */

/** @defgroup SDMMC_LL_Register Bits And Addresses Definitions
  * @brief SDMMC_LL registers bit address in the alias region
  * @{
  */
/* ---------------------- SDMMC registers bit mask --------------------------- */
/* --- CLKCR Register ---*/
/* CLKCR register clear mask */
#if defined(STM32L4R5xx) || defined(STM32L4R7xx) || defined(STM32L4R9xx) || defined(STM32L4S5xx) || defined(STM32L4S7xx) || defined(STM32L4S9xx)
#define CLKCR_CLEAR_MASK         ((uint32_t)(SDMMC_CLKCR_CLKDIV  | SDMMC_CLKCR_PWRSAV |\
                                             SDMMC_CLKCR_WIDBUS |\
                                             SDMMC_CLKCR_NEGEDGE | SDMMC_CLKCR_HWFC_EN |\
                                             SDMMC_CLKCR_DDR | SDMMC_CLKCR_BUSSPEED |\
                                             SDMMC_CLKCR_SELCLKRX))
#else
#define CLKCR_CLEAR_MASK         ((uint32_t)(SDMMC_CLKCR_CLKDIV  | SDMMC_CLKCR_PWRSAV |\
                                             SDMMC_CLKCR_BYPASS  | SDMMC_CLKCR_WIDBUS |\
                                             SDMMC_CLKCR_NEGEDGE | SDMMC_CLKCR_HWFC_EN))
#endif /* STM32L4R5xx || STM32L4R7xx || STM32L4R9xx || STM32L4S5xx || STM32L4S7xx || STM32L4S9xx */

/* --- DCTRL Register ---*/
/* SDMMC DCTRL Clear Mask */
#define DCTRL_CLEAR_MASK         ((uint32_t)(SDMMC_DCTRL_DTEN    | SDMMC_DCTRL_DTDIR |\
                                             SDMMC_DCTRL_DTMODE  | SDMMC_DCTRL_DBLOCKSIZE))

/* --- CMD Register ---*/
/* CMD Register clear mask */
#if defined(STM32L4R5xx) || defined(STM32L4R7xx) || defined(STM32L4R9xx) || defined(STM32L4S5xx) || defined(STM32L4S7xx) || defined(STM32L4S9xx)
#define CMD_CLEAR_MASK           ((uint32_t)(SDMMC_CMD_CMDINDEX | SDMMC_CMD_WAITRESP |\
                                             SDMMC_CMD_WAITINT  | SDMMC_CMD_WAITPEND |\
                                             SDMMC_CMD_CPSMEN   | SDMMC_CMD_CMDSUSPEND))
#else
#define CMD_CLEAR_MASK           ((uint32_t)(SDMMC_CMD_CMDINDEX | SDMMC_CMD_WAITRESP |\
                                             SDMMC_CMD_WAITINT  | SDMMC_CMD_WAITPEND |\
                                             SDMMC_CMD_CPSMEN   | SDMMC_CMD_SDIOSUSPEND))
#endif /* STM32L4R5xx || STM32L4R7xx || STM32L4R9xx || STM32L4S5xx || STM32L4S7xx || STM32L4S9xx */

#if defined(STM32L4R5xx) || defined(STM32L4R7xx) || defined(STM32L4R9xx) || defined(STM32L4S5xx) || defined(STM32L4S7xx) || defined(STM32L4S9xx)
/* SDMMC Initialization Frequency (400KHz max) for Peripheral CLK 110MHz*/
#define SDMMC_INIT_CLK_DIV ((uint8_t)0x8A)

/* SDMMC Default Speed Frequency (25Mhz max) for Peripheral CLK 110MHz*/
#define SDMMC_NSpeed_CLK_DIV ((uint8_t)0x3)

/* SDMMC High Speed Frequency (50Mhz max) for Peripheral CLK 110MHz*/
#define SDMMC_HSpeed_CLK_DIV ((uint8_t)0x2)

/* SDMMC Data Transfer Frequency (25MHz max) */
#define SDMMC_TRANSFER_CLK_DIV SDMMC_NSpeed_CLK_DIV
#else
/* SDMMC Initialization Frequency (400KHz max) */
#define SDMMC_INIT_CLK_DIV     ((uint8_t)0x76)    /* 48MHz / (SDMMC_INIT_CLK_DIV + 2) < 400KHz */

/* SDMMC Data Transfer Frequency (25MHz max) */
#define SDMMC_TRANSFER_CLK_DIV ((uint8_t)0x0)     /* 48MHz / (SDMMC_TRANSFER_CLK_DIV + 2) < 25MHz */
#endif /* STM32L4R5xx || STM32L4R7xx || STM32L4R9xx || STM32L4S5xx || STM32L4S7xx || STM32L4S9xx */

/**
  * @}
  */

/** @defgroup SDMMC_LL_Interrupt_Clock Interrupt And Clock Configuration
 *  @brief macros to handle interrupts and specific clock configurations
 * @{
 */

#if !defined(STM32L4R5xx) && !defined(STM32L4R7xx) && !defined(STM32L4R9xx) && !defined(STM32L4S5xx) && !defined(STM32L4S7xx) && !defined(STM32L4S9xx)
/**
  * @brief  Enable the SDMMC device.
  * @param  __INSTANCE__: SDMMC Instance
  * @retval None
  */
#define __SDMMC_ENABLE(__INSTANCE__)  ((__INSTANCE__)->CLKCR |= SDMMC_CLKCR_CLKEN)

/**
  * @brief  Disable the SDMMC device.
  * @param  __INSTANCE__: SDMMC Instance
  * @retval None
  */
#define __SDMMC_DISABLE(__INSTANCE__)  ((__INSTANCE__)->CLKCR &= ~SDMMC_CLKCR_CLKEN)

/**
  * @brief  Enable the SDMMC DMA transfer.
  * @param  __INSTANCE__: SDMMC Instance
  * @retval None
  */
#define __SDMMC_DMA_ENABLE(__INSTANCE__)  ((__INSTANCE__)->DCTRL |= SDMMC_DCTRL_DMAEN)

/**
  * @brief  Disable the SDMMC DMA transfer.
  * @param  __INSTANCE__: SDMMC Instance
  * @retval None
  */
#define __SDMMC_DMA_DISABLE(__INSTANCE__)  ((__INSTANCE__)->DCTRL &= ~SDMMC_DCTRL_DMAEN)
#endif /* !STM32L4R5xx && !STM32L4R7xx && !STM32L4R9xx && !STM32L4S5xx && !STM32L4S7xx && !STM32L4S9xx */

/**
  * @brief  Enable the SDMMC device interrupt.
  * @param  __INSTANCE__ : Pointer to SDMMC register base
  * @param  __INTERRUPT__ : specifies the SDMMC interrupt sources to be enabled.
  *         This parameter can be one or a combination of the following values:
  *            @arg SDMMC_IT_CCRCFAIL:   Command response received (CRC check failed) interrupt
  *            @arg SDMMC_IT_DCRCFAIL:   Data block sent/received (CRC check failed) interrupt
  *            @arg SDMMC_IT_CTIMEOUT:   Command response timeout interrupt
  *            @arg SDMMC_IT_DTIMEOUT:   Data timeout interrupt
  *            @arg SDMMC_IT_TXUNDERR:   Transmit FIFO underrun error interrupt
  *            @arg SDMMC_IT_RXOVERR:    Received FIFO overrun error interrupt
  *            @arg SDMMC_IT_CMDREND:    Command response received (CRC check passed) interrupt
  *            @arg SDMMC_IT_CMDSENT:    Command sent (no response required) interrupt
  *            @arg SDMMC_IT_DATAEND:    Data end (data counter, DATACOUNT, is zero) interrupt
  *            @arg SDMMC_IT_DHOLD:      Data transfer Hold interrupt
  *            @arg SDMMC_IT_DBCKEND:    Data block sent/received (CRC check passed) interrupt
  *            @arg SDMMC_IT_DABORT:     Data transfer aborted by CMD12 interrupt
  *            @arg SDMMC_IT_CMDACT:     Command transfer in progress interrupt
  *            @arg SDMMC_IT_TXACT:      Data transmit in progress interrupt
  *            @arg SDMMC_IT_RXACT:      Data receive in progress interrupt
  *            @arg SDMMC_IT_TXFIFOHE:   Transmit FIFO Half Empty interrupt
  *            @arg SDMMC_IT_RXFIFOHF:   Receive FIFO Half Full interrupt
  *            @arg SDMMC_IT_TXFIFOF:    Transmit FIFO full interrupt
  *            @arg SDMMC_IT_RXFIFOF:    Receive FIFO full interrupt
  *            @arg SDMMC_IT_TXFIFOE:    Transmit FIFO empty interrupt
  *            @arg SDMMC_IT_RXFIFOE:    Receive FIFO empty interrupt
  *            @arg SDMMC_IT_TXDAVL:     Data available in transmit FIFO interrupt
  *            @arg SDMMC_IT_RXDAVL:     Data available in receive FIFO interrupt
  *            @arg SDMMC_IT_BUSYD0END:  End of SDMMC_D0 Busy following a CMD response detected interrupt
  *            @arg SDMMC_IT_SDIOIT:     SDIO interrupt received interrupt
  *            @arg SDMMC_IT_ACKFAIL:    Boot Acknowledgment received interrupt
  *            @arg SDMMC_IT_ACKTIMEOUT: Boot Acknowledgment timeout interrupt
  *            @arg SDMMC_IT_VSWEND:     Voltage switch critical timing section completion interrupt
  *            @arg SDMMC_IT_CKSTOP:     SDMMC_CK stopped in Voltage switch procedure interrupt
  *            @arg SDMMC_IT_IDMABTC:    IDMA buffer transfer complete interrupt
  * @retval None
  */
#define __SDMMC_ENABLE_IT(__INSTANCE__, __INTERRUPT__)  ((__INSTANCE__)->MASK |= (__INTERRUPT__))

/**
  * @brief  Disable the SDMMC device interrupt.
  * @param  __INSTANCE__ : Pointer to SDMMC register base
  * @param  __INTERRUPT__ : specifies the SDMMC interrupt sources to be disabled.
  *          This parameter can be one or a combination of the following values:
  *            @arg SDMMC_IT_CCRCFAIL:   Command response received (CRC check failed) interrupt
  *            @arg SDMMC_IT_DCRCFAIL:   Data block sent/received (CRC check failed) interrupt
  *            @arg SDMMC_IT_CTIMEOUT:   Command response timeout interrupt
  *            @arg SDMMC_IT_DTIMEOUT:   Data timeout interrupt
  *            @arg SDMMC_IT_TXUNDERR:   Transmit FIFO underrun error interrupt
  *            @arg SDMMC_IT_RXOVERR:    Received FIFO overrun error interrupt
  *            @arg SDMMC_IT_CMDREND:    Command response received (CRC check passed) interrupt
  *            @arg SDMMC_IT_CMDSENT:    Command sent (no response required) interrupt
  *            @arg SDMMC_IT_DATAEND:    Data end (data counter, DATACOUNT, is zero) interrupt
  *            @arg SDMMC_IT_DHOLD:      Data transfer Hold interrupt
  *            @arg SDMMC_IT_DBCKEND:    Data block sent/received (CRC check passed) interrupt
  *            @arg SDMMC_IT_DABORT:     Data transfer aborted by CMD12 interrupt
  *            @arg SDMMC_IT_CMDACT:     Command transfer in progress interrupt
  *            @arg SDMMC_IT_TXACT:      Data transmit in progress interrupt
  *            @arg SDMMC_IT_RXACT:      Data receive in progress interrupt
  *            @arg SDMMC_IT_TXFIFOHE:   Transmit FIFO Half Empty interrupt
  *            @arg SDMMC_IT_RXFIFOHF:   Receive FIFO Half Full interrupt
  *            @arg SDMMC_IT_TXFIFOF:    Transmit FIFO full interrupt
  *            @arg SDMMC_IT_RXFIFOF:    Receive FIFO full interrupt
  *            @arg SDMMC_IT_TXFIFOE:    Transmit FIFO empty interrupt
  *            @arg SDMMC_IT_RXFIFOE:    Receive FIFO empty interrupt
  *            @arg SDMMC_IT_TXDAVL:     Data available in transmit FIFO interrupt
  *            @arg SDMMC_IT_RXDAVL:     Data available in receive FIFO interrupt
  *            @arg SDMMC_IT_BUSYD0END:  End of SDMMC_D0 Busy following a CMD response detected interrupt
  *            @arg SDMMC_IT_SDIOIT:     SDIO interrupt received interrupt
  *            @arg SDMMC_IT_ACKFAIL:    Boot Acknowledgment received interrupt
  *            @arg SDMMC_IT_ACKTIMEOUT: Boot Acknowledgment timeout interrupt
  *            @arg SDMMC_IT_VSWEND:     Voltage switch critical timing section completion interrupt
  *            @arg SDMMC_IT_CKSTOP:     SDMMC_CK stopped in Voltage switch procedure interrupt
  *            @arg SDMMC_IT_IDMABTC:    IDMA buffer transfer complete interrupt
  * @retval None
  */
#define __SDMMC_DISABLE_IT(__INSTANCE__, __INTERRUPT__)  ((__INSTANCE__)->MASK &= ~(__INTERRUPT__))

/**
  * @brief  Checks whether the specified SDMMC flag is set or not.
  * @param  __INSTANCE__ : Pointer to SDMMC register base
  * @param  __FLAG__: specifies the flag to check.
  *          This parameter can be one of the following values:
  *            @arg SDMMC_FLAG_CCRCFAIL:   Command response received (CRC check failed)
  *            @arg SDMMC_FLAG_DCRCFAIL:   Data block sent/received (CRC check failed)
  *            @arg SDMMC_FLAG_CTIMEOUT:   Command response timeout
  *            @arg SDMMC_FLAG_DTIMEOUT:   Data timeout
  *            @arg SDMMC_FLAG_TXUNDERR:   Transmit FIFO underrun error
  *            @arg SDMMC_FLAG_RXOVERR:    Received FIFO overrun error
  *            @arg SDMMC_FLAG_CMDREND:    Command response received (CRC check passed)
  *            @arg SDMMC_FLAG_CMDSENT:    Command sent (no response required)
  *            @arg SDMMC_FLAG_DATAEND:    Data end (data counter, DATACOUNT, is zero)
  *            @arg SDMMC_FLAG_DHOLD:      Data transfer Hold
  *            @arg SDMMC_FLAG_DBCKEND:    Data block sent/received (CRC check passed)
  *            @arg SDMMC_FLAG_DABORT:     Data transfer aborted by CMD12
  *            @arg SDMMC_FLAG_DPSMACT:    Data path state machine active
  *            @arg SDMMC_FLAG_CPSMACT:    Command path state machine active
  *            @arg SDMMC_FLAG_CMDACT:     Command transfer in progress
  *            @arg SDMMC_FLAG_TXACT:      Data transmit in progress
  *            @arg SDMMC_FLAG_RXACT:      Data receive in progress
  *            @arg SDMMC_FLAG_TXFIFOHE:   Transmit FIFO Half Empty
  *            @arg SDMMC_FLAG_RXFIFOHF:   Receive FIFO Half Full
  *            @arg SDMMC_FLAG_TXFIFOF:    Transmit FIFO full
  *            @arg SDMMC_FLAG_RXFIFOF:    Receive FIFO full
  *            @arg SDMMC_FLAG_TXFIFOE:    Transmit FIFO empty
  *            @arg SDMMC_FLAG_RXFIFOE:    Receive FIFO empty
  *            @arg SDMMC_FLAG_TXDAVL:     Data available in transmit FIFO
  *            @arg SDMMC_FLAG_RXDAVL:     Data available in receive FIFO
  *            @arg SDMMC_FLAG_BUSYD0:     Inverted value of SDMMC_D0 line (Busy)
  *            @arg SDMMC_FLAG_BUSYD0END:  End of SDMMC_D0 Busy following a CMD response detected
  *            @arg SDMMC_FLAG_SDIOIT:     SDIO interrupt received
  *            @arg SDMMC_FLAG_ACKFAIL:    Boot Acknowledgment received
  *            @arg SDMMC_FLAG_ACKTIMEOUT: Boot Acknowledgment timeout
  *            @arg SDMMC_FLAG_VSWEND:     Voltage switch critical timing section completion
  *            @arg SDMMC_FLAG_CKSTOP:     SDMMC_CK stopped in Voltage switch procedure
  *            @arg SDMMC_FLAG_IDMATE:     IDMA transfer error
  *            @arg SDMMC_FLAG_IDMABTC:    IDMA buffer transfer complete
  * @retval The new state of SDMMC_FLAG (SET or RESET).
  */
#define __SDMMC_GET_FLAG(__INSTANCE__, __FLAG__)  (((__INSTANCE__)->STA &(__FLAG__)) != 0U)


/**
  * @brief  Clears the SDMMC pending flags.
  * @param  __INSTANCE__ : Pointer to SDMMC register base
  * @param  __FLAG__: specifies the flag to clear.
  *          This parameter can be one or a combination of the following values:
  *            @arg SDMMC_FLAG_CCRCFAIL:   Command response received (CRC check failed)
  *            @arg SDMMC_FLAG_DCRCFAIL:   Data block sent/received (CRC check failed)
  *            @arg SDMMC_FLAG_CTIMEOUT:   Command response timeout
  *            @arg SDMMC_FLAG_DTIMEOUT:   Data timeout
  *            @arg SDMMC_FLAG_TXUNDERR:   Transmit FIFO underrun error
  *            @arg SDMMC_FLAG_RXOVERR:    Received FIFO overrun error
  *            @arg SDMMC_FLAG_CMDREND:    Command response received (CRC check passed)
  *            @arg SDMMC_FLAG_CMDSENT:    Command sent (no response required)
  *            @arg SDMMC_FLAG_DATAEND:    Data end (data counter, DATACOUNT, is zero)
  *            @arg SDMMC_FLAG_DHOLD:      Data transfer Hold
  *            @arg SDMMC_FLAG_DBCKEND:    Data block sent/received (CRC check passed)
  *            @arg SDMMC_FLAG_DABORT:     Data transfer aborted by CMD12
  *            @arg SDMMC_FLAG_BUSYD0END:  End of SDMMC_D0 Busy following a CMD response detected
  *            @arg SDMMC_FLAG_SDIOIT:     SDIO interrupt received
  *            @arg SDMMC_FLAG_ACKFAIL:    Boot Acknowledgment received
  *            @arg SDMMC_FLAG_ACKTIMEOUT: Boot Acknowledgment timeout
  *            @arg SDMMC_FLAG_VSWEND:     Voltage switch critical timing section completion
  *            @arg SDMMC_FLAG_CKSTOP:     SDMMC_CK stopped in Voltage switch procedure
  *            @arg SDMMC_FLAG_IDMATE:     IDMA transfer error
  *            @arg SDMMC_FLAG_IDMABTC:    IDMA buffer transfer complete
  * @retval None
  */
#define __SDMMC_CLEAR_FLAG(__INSTANCE__, __FLAG__)  ((__INSTANCE__)->ICR = (__FLAG__))

/**
  * @brief  Checks whether the specified SDMMC interrupt has occurred or not.
  * @param  __INSTANCE__ : Pointer to SDMMC register base
  * @param  __INTERRUPT__: specifies the SDMMC interrupt source to check.
  *          This parameter can be one of the following values:
  *            @arg SDMMC_IT_CCRCFAIL:   Command response received (CRC check failed) interrupt
  *            @arg SDMMC_IT_DCRCFAIL:   Data block sent/received (CRC check failed) interrupt
  *            @arg SDMMC_IT_CTIMEOUT:   Command response timeout interrupt
  *            @arg SDMMC_IT_DTIMEOUT:   Data timeout interrupt
  *            @arg SDMMC_IT_TXUNDERR:   Transmit FIFO underrun error interrupt
  *            @arg SDMMC_IT_RXOVERR:    Received FIFO overrun error interrupt
  *            @arg SDMMC_IT_CMDREND:    Command response received (CRC check passed) interrupt
  *            @arg SDMMC_IT_CMDSENT:    Command sent (no response required) interrupt
  *            @arg SDMMC_IT_DATAEND:    Data end (data counter, DATACOUNT, is zero) interrupt
  *            @arg SDMMC_IT_DHOLD:      Data transfer Hold interrupt
  *            @arg SDMMC_IT_DBCKEND:    Data block sent/received (CRC check passed) interrupt
  *            @arg SDMMC_IT_DABORT:     Data transfer aborted by CMD12 interrupt
  *            @arg SDMMC_IT_CMDACT:     Command transfer in progress interrupt
  *            @arg SDMMC_IT_TXACT:      Data transmit in progress interrupt
  *            @arg SDMMC_IT_RXACT:      Data receive in progress interrupt
  *            @arg SDMMC_IT_TXFIFOHE:   Transmit FIFO Half Empty interrupt
  *            @arg SDMMC_IT_RXFIFOHF:   Receive FIFO Half Full interrupt
  *            @arg SDMMC_IT_TXFIFOF:    Transmit FIFO full interrupt
  *            @arg SDMMC_IT_RXFIFOF:    Receive FIFO full interrupt
  *            @arg SDMMC_IT_TXFIFOE:    Transmit FIFO empty interrupt
  *            @arg SDMMC_IT_RXFIFOE:    Receive FIFO empty interrupt
  *            @arg SDMMC_IT_TXDAVL:     Data available in transmit FIFO interrupt
  *            @arg SDMMC_IT_RXDAVL:     Data available in receive FIFO interrupt
  *            @arg SDMMC_IT_BUSYD0END:  End of SDMMC_D0 Busy following a CMD response detected interrupt
  *            @arg SDMMC_IT_SDIOIT:     SDIO interrupt received interrupt
  *            @arg SDMMC_IT_ACKFAIL:    Boot Acknowledgment received interrupt
  *            @arg SDMMC_IT_ACKTIMEOUT: Boot Acknowledgment timeout interrupt
  *            @arg SDMMC_IT_VSWEND:     Voltage switch critical timing section completion interrupt
  *            @arg SDMMC_IT_CKSTOP:     SDMMC_CK stopped in Voltage switch procedure interrupt
  *            @arg SDMMC_IT_IDMABTC:    IDMA buffer transfer complete interrupt
  * @retval The new state of SDMMC_IT (SET or RESET).
  */
#define __SDMMC_GET_IT(__INSTANCE__, __INTERRUPT__)  (((__INSTANCE__)->STA &(__INTERRUPT__)) == (__INTERRUPT__))

/**
  * @brief  Clears the SDMMC's interrupt pending bits.
  * @param  __INSTANCE__ : Pointer to SDMMC register base
  * @param  __INTERRUPT__: specifies the interrupt pending bit to clear.
  *          This parameter can be one or a combination of the following values:
  *            @arg SDMMC_IT_CCRCFAIL:   Command response received (CRC check failed) interrupt
  *            @arg SDMMC_IT_DCRCFAIL:   Data block sent/received (CRC check failed) interrupt
  *            @arg SDMMC_IT_CTIMEOUT:   Command response timeout interrupt
  *            @arg SDMMC_IT_DTIMEOUT:   Data timeout interrupt
  *            @arg SDMMC_IT_TXUNDERR:   Transmit FIFO underrun error interrupt
  *            @arg SDMMC_IT_RXOVERR:    Received FIFO overrun error interrupt
  *            @arg SDMMC_IT_CMDREND:    Command response received (CRC check passed) interrupt
  *            @arg SDMMC_IT_CMDSENT:    Command sent (no response required) interrupt
  *            @arg SDMMC_IT_DATAEND:    Data end (data counter, DATACOUNT, is zero) interrupt
  *            @arg SDMMC_IT_DHOLD:      Data transfer Hold interrupt
  *            @arg SDMMC_IT_DBCKEND:    Data block sent/received (CRC check passed) interrupt
  *            @arg SDMMC_IT_DABORT:     Data transfer aborted by CMD12 interrupt
  *            @arg SDMMC_IT_BUSYD0END:  End of SDMMC_D0 Busy following a CMD response detected interrupt
  *            @arg SDMMC_IT_SDIOIT:     SDIO interrupt received interrupt
  *            @arg SDMMC_IT_ACKFAIL:    Boot Acknowledgment received interrupt
  *            @arg SDMMC_IT_ACKTIMEOUT: Boot Acknowledgment timeout interrupt
  *            @arg SDMMC_IT_VSWEND:     Voltage switch critical timing section completion interrupt
  *            @arg SDMMC_IT_CKSTOP:     SDMMC_CK stopped in Voltage switch procedure interrupt
  *            @arg SDMMC_IT_IDMABTC:    IDMA buffer transfer complete interrupt
  * @retval None
  */
#define __SDMMC_CLEAR_IT(__INSTANCE__, __INTERRUPT__)  ((__INSTANCE__)->ICR = (__INTERRUPT__))

/**
  * @brief  Enable Start the SD I/O Read Wait operation.
  * @param  __INSTANCE__ : Pointer to SDMMC register base
  * @retval None
  */
#define __SDMMC_START_READWAIT_ENABLE(__INSTANCE__)  ((__INSTANCE__)->DCTRL |= SDMMC_DCTRL_RWSTART)

/**
  * @brief  Disable Start the SD I/O Read Wait operations.
  * @param  __INSTANCE__ : Pointer to SDMMC register base
  * @retval None
  */
#define __SDMMC_START_READWAIT_DISABLE(__INSTANCE__)  ((__INSTANCE__)->DCTRL &= ~SDMMC_DCTRL_RWSTART)

/**
  * @brief  Enable Start the SD I/O Read Wait operation.
  * @param  __INSTANCE__ : Pointer to SDMMC register base
  * @retval None
  */
#define __SDMMC_STOP_READWAIT_ENABLE(__INSTANCE__)  ((__INSTANCE__)->DCTRL |= SDMMC_DCTRL_RWSTOP)

/**
  * @brief  Disable Stop the SD I/O Read Wait operations.
  * @param  __INSTANCE__ : Pointer to SDMMC register base
  * @retval None
  */
#define __SDMMC_STOP_READWAIT_DISABLE(__INSTANCE__)  ((__INSTANCE__)->DCTRL &= ~SDMMC_DCTRL_RWSTOP)

/**
  * @brief  Enable the SD I/O Mode Operation.
  * @param  __INSTANCE__ : Pointer to SDMMC register base
  * @retval None
  */
#define __SDMMC_OPERATION_ENABLE(__INSTANCE__)  ((__INSTANCE__)->DCTRL |= SDMMC_DCTRL_SDIOEN)

/**
  * @brief  Disable the SD I/O Mode Operation.
  * @param  __INSTANCE__ : Pointer to SDMMC register base
  * @retval None
  */
#define __SDMMC_OPERATION_DISABLE(__INSTANCE__)  ((__INSTANCE__)->DCTRL &= ~SDMMC_DCTRL_SDIOEN)

/**
  * @brief  Enable the SD I/O Suspend command sending.
  * @param  __INSTANCE__ : Pointer to SDMMC register base
  * @retval None
  */
#if !defined(STM32L4R5xx) && !defined(STM32L4R7xx) && !defined(STM32L4R9xx) && !defined(STM32L4S5xx) && !defined(STM32L4S7xx) && !defined(STM32L4S9xx)
#define __SDMMC_SUSPEND_CMD_ENABLE(__INSTANCE__)  ((__INSTANCE__)->CMD |= SDMMC_CMD_SDIOSUSPEND)
#else
#define __SDMMC_SUSPEND_CMD_ENABLE(__INSTANCE__)  ((__INSTANCE__)->CMD |= SDMMC_CMD_CMDSUSPEND)
#endif /* !STM32L4R5xx && !STM32L4R7xx && !STM32L4R9xx && !STM32L4S5xx && !STM32L4S7xx && !STM32L4S9xx */

/**
  * @brief  Disable the SD I/O Suspend command sending.
  * @param  __INSTANCE__ : Pointer to SDMMC register base
  * @retval None
  */
#if !defined(STM32L4R5xx) && !defined(STM32L4R7xx) && !defined(STM32L4R9xx) && !defined(STM32L4S5xx) && !defined(STM32L4S7xx) && !defined(STM32L4S9xx)
#define __SDMMC_SUSPEND_CMD_DISABLE(__INSTANCE__)  ((__INSTANCE__)->CMD &= ~SDMMC_CMD_SDIOSUSPEND)
#else
#define __SDMMC_SUSPEND_CMD_DISABLE(__INSTANCE__)  ((__INSTANCE__)->CMD &= ~SDMMC_CMD_CMDSUSPEND)
#endif /* !STM32L4R5xx && !STM32L4R7xx && !STM32L4R9xx && !STM32L4S5xx && !STM32L4S7xx && !STM32L4S9xx */

#if defined(STM32L4R5xx) || defined(STM32L4R7xx) || defined(STM32L4R9xx) || defined(STM32L4S5xx) || defined(STM32L4S7xx) || defined(STM32L4S9xx)
/**
  * @brief  Enable the CMDTRANS mode.
  * @param  __INSTANCE__ : Pointer to SDMMC register base
  * @retval None
  */
#define __SDMMC_CMDTRANS_ENABLE(__INSTANCE__)  ((__INSTANCE__)->CMD |= SDMMC_CMD_CMDTRANS)

/**
  * @brief  Disable the CMDTRANS mode.
  * @param  __INSTANCE__ : Pointer to SDMMC register base
  * @retval None
  */
#define __SDMMC_CMDTRANS_DISABLE(__INSTANCE__)  ((__INSTANCE__)->CMD &= ~SDMMC_CMD_CMDTRANS)

/**
  * @brief  Enable the CMDSTOP mode.
  * @param  __INSTANCE__ : Pointer to SDMMC register base
  * @retval None
  */
#define __SDMMC_CMDSTOP_ENABLE(__INSTANCE__)  ((__INSTANCE__)->CMD |= SDMMC_CMD_CMDSTOP)

/**
  * @brief  Disable the CMDSTOP mode.
  * @param  __INSTANCE__ : Pointer to SDMMC register base
  * @retval None
  */
#define __SDMMC_CMDSTOP_DISABLE(__INSTANCE__)  ((__INSTANCE__)->CMD &= ~SDMMC_CMD_CMDSTOP)
#endif /* STM32L4R5xx || STM32L4R7xx || STM32L4R9xx || STM32L4S5xx || STM32L4S7xx || STM32L4S9xx */

/**
  * @}
  */

/**
  * @}
  */

/* Exported functions --------------------------------------------------------*/
/** @addtogroup SDMMC_LL_Exported_Functions
  * @{
  */

/* Initialization/de-initialization functions  **********************************/
/** @addtogroup HAL_SDMMC_LL_Group1
  * @{
  */
HAL_StatusTypeDef SDMMC_Init(SDMMC_TypeDef *SDMMCx, SDMMC_InitTypeDef Init);
/**
  * @}
  */

/* I/O operation functions  *****************************************************/
/** @addtogroup HAL_SDMMC_LL_Group2
  * @{
  */
uint32_t          SDMMC_ReadFIFO(SDMMC_TypeDef *SDMMCx);
HAL_StatusTypeDef SDMMC_WriteFIFO(SDMMC_TypeDef *SDMMCx, uint32_t *pWriteData);
/**
  * @}
  */

/* Peripheral Control functions  ************************************************/
/** @addtogroup HAL_SDMMC_LL_Group3
  * @{
  */
HAL_StatusTypeDef SDMMC_PowerState_ON(SDMMC_TypeDef *SDMMCx);
#if defined(STM32L4R5xx) || defined(STM32L4R7xx) || defined(STM32L4R9xx) || defined(STM32L4S5xx) || defined(STM32L4S7xx) || defined(STM32L4S9xx)
HAL_StatusTypeDef SDMMC_PowerState_Cycle(SDMMC_TypeDef *SDMMCx);
#endif /* !STM32L4R5xx && !STM32L4R7xx && !STM32L4R9xx && !STM32L4S5xx && !STM32L4S7xx && !STM32L4S9xx */
HAL_StatusTypeDef SDMMC_PowerState_OFF(SDMMC_TypeDef *SDMMCx);
uint32_t          SDMMC_GetPowerState(SDMMC_TypeDef *SDMMCx);

/* Command path state machine (CPSM) management functions */
HAL_StatusTypeDef SDMMC_SendCommand(SDMMC_TypeDef *SDMMCx, SDMMC_CmdInitTypeDef *Command);
uint8_t           SDMMC_GetCommandResponse(SDMMC_TypeDef *SDMMCx);
uint32_t          SDMMC_GetResponse(SDMMC_TypeDef *SDMMCx, uint32_t Response);

/* Data path state machine (DPSM) management functions */
HAL_StatusTypeDef SDMMC_ConfigData(SDMMC_TypeDef *SDMMCx, SDMMC_DataInitTypeDef* Data);
uint32_t          SDMMC_GetDataCounter(SDMMC_TypeDef *SDMMCx);
uint32_t          SDMMC_GetFIFOCount(SDMMC_TypeDef *SDMMCx);

/* SDMMC Cards mode management functions */
HAL_StatusTypeDef SDMMC_SetSDMMCReadWaitMode(SDMMC_TypeDef *SDMMCx, uint32_t SDMMC_ReadWaitMode);

/* SDMMC Commands management functions */
uint32_t SDMMC_CmdBlockLength(SDMMC_TypeDef *SDMMCx, uint32_t BlockSize);
uint32_t SDMMC_CmdReadSingleBlock(SDMMC_TypeDef *SDMMCx, uint32_t ReadAdd);
uint32_t SDMMC_CmdReadMultiBlock(SDMMC_TypeDef *SDMMCx, uint32_t ReadAdd);
uint32_t SDMMC_CmdWriteSingleBlock(SDMMC_TypeDef *SDMMCx, uint32_t WriteAdd);
uint32_t SDMMC_CmdWriteMultiBlock(SDMMC_TypeDef *SDMMCx, uint32_t WriteAdd);
uint32_t SDMMC_CmdEraseStartAdd(SDMMC_TypeDef *SDMMCx, uint32_t StartAdd);
uint32_t SDMMC_CmdSDEraseStartAdd(SDMMC_TypeDef *SDMMCx, uint32_t StartAdd);
uint32_t SDMMC_CmdEraseEndAdd(SDMMC_TypeDef *SDMMCx, uint32_t EndAdd);
uint32_t SDMMC_CmdSDEraseEndAdd(SDMMC_TypeDef *SDMMCx, uint32_t EndAdd);
uint32_t SDMMC_CmdErase(SDMMC_TypeDef *SDMMCx);
uint32_t SDMMC_CmdStopTransfer(SDMMC_TypeDef *SDMMCx);
uint32_t SDMMC_CmdSelDesel(SDMMC_TypeDef *SDMMCx, uint64_t Addr);
uint32_t SDMMC_CmdGoIdleState(SDMMC_TypeDef *SDMMCx);
uint32_t SDMMC_CmdOperCond(SDMMC_TypeDef *SDMMCx);
uint32_t SDMMC_CmdAppCommand(SDMMC_TypeDef *SDMMCx, uint32_t Argument);
uint32_t SDMMC_CmdAppOperCommand(SDMMC_TypeDef *SDMMCx, uint32_t Argument);
uint32_t SDMMC_CmdBusWidth(SDMMC_TypeDef *SDMMCx, uint32_t BusWidth);
uint32_t SDMMC_CmdSendSCR(SDMMC_TypeDef *SDMMCx);
uint32_t SDMMC_CmdSendCID(SDMMC_TypeDef *SDMMCx);
uint32_t SDMMC_CmdSendCSD(SDMMC_TypeDef *SDMMCx, uint32_t Argument);
uint32_t SDMMC_CmdSetRelAdd(SDMMC_TypeDef *SDMMCx, uint16_t *pRCA);
uint32_t SDMMC_CmdSendStatus(SDMMC_TypeDef *SDMMCx, uint32_t Argument);
uint32_t SDMMC_CmdStatusRegister(SDMMC_TypeDef *SDMMCx);
#if defined(STM32L4R5xx) || defined(STM32L4R7xx) || defined(STM32L4R9xx) || defined(STM32L4S5xx) || defined(STM32L4S7xx) || defined(STM32L4S9xx)
uint32_t SDMMC_CmdVoltageSwitch(SDMMC_TypeDef *SDMMCx);
#endif /* STM32L4R5xx || STM32L4R7xx || STM32L4R9xx || STM32L4S5xx || STM32L4S7xx || STM32L4S9xx */
uint32_t SDMMC_CmdOpCondition(SDMMC_TypeDef *SDMMCx, uint32_t Argument);
uint32_t SDMMC_CmdSwitch(SDMMC_TypeDef *SDMMCx, uint32_t Argument);
#if defined(STM32L4R5xx) || defined(STM32L4R7xx) || defined(STM32L4R9xx) || defined(STM32L4S5xx) || defined(STM32L4S7xx) || defined(STM32L4S9xx)
uint32_t SDMMC_CmdSendEXTCSD(SDMMC_TypeDef *SDMMCx, uint32_t Argument);
#endif /* STM32L4R5xx || STM32L4R7xx || STM32L4R9xx || STM32L4S5xx || STM32L4S7xx || STM32L4S9xx */

/**
  * @}
  */

/**
  * @}
  */

/**
  * @}
  */

/**
  * @}
  */

  /**
  * @}
  */

/**
  * @}
  */
#endif /* SDMMC1 */

#ifdef __cplusplus
}
#endif

#endif /* STM32L4xx_LL_SDMMC_H */

/************************ (C) COPYRIGHT STMicroelectronics *****END OF FILE****/
