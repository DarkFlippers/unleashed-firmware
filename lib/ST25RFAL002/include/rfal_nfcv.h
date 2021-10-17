
/******************************************************************************
  * \attention
  *
  * <h2><center>&copy; COPYRIGHT 2020 STMicroelectronics</center></h2>
  *
  * Licensed under ST MYLIBERTY SOFTWARE LICENSE AGREEMENT (the "License");
  * You may not use this file except in compliance with the License.
  * You may obtain a copy of the License at:
  *
  *        www.st.com/myliberty
  *
  * Unless required by applicable law or agreed to in writing, software 
  * distributed under the License is distributed on an "AS IS" BASIS, 
  * WITHOUT WARRANTIES OR CONDITIONS OF ANY KIND, either express or implied,
  * AND SPECIFICALLY DISCLAIMING THE IMPLIED WARRANTIES OF MERCHANTABILITY,
  * FITNESS FOR A PARTICULAR PURPOSE, AND NON-INFRINGEMENT.
  * See the License for the specific language governing permissions and
  * limitations under the License.
  *
******************************************************************************/

/*
 *      PROJECT:   ST25R391x firmware
 *      Revision:
 *      LANGUAGE:  ISO C99
 */

/*! \file rfal_nfcv.h
 *
 *  \author Gustavo Patricio
 *
 *  \brief Implementation of NFC-V Poller (ISO15693) device
 *
 *  The definitions and helpers methods provided by this module 
 *  are aligned with NFC-V Digital 2.1
 *
 *
 * \addtogroup RFAL
 * @{
 *
 * \addtogroup RFAL-AL
 * \brief RFAL Abstraction Layer
 * @{
 *
 * \addtogroup NFC-V
 * \brief RFAL NFC-V Module
 * @{
 * 
 */

#ifndef RFAL_NFCV_H
#define RFAL_NFCV_H

/*
 ******************************************************************************
 * INCLUDES
 ******************************************************************************
 */
#include "platform.h"
#include "st_errno.h"
#include "rfal_rf.h"

/*
 ******************************************************************************
 * GLOBAL DEFINES
 ******************************************************************************
 */

#define RFAL_NFCV_UID_LEN                 8U              /*!< NFC-V UID length                                             */
#define RFAL_NFCV_MAX_BLOCK_LEN           32U             /*!< Max Block size: can be of up to 256 bits  ISO 15693 2000  5  */
#define RFAL_NFCV_BNO_LEN                 1U              /*!< NFC-V Block Number length                                    */
#define RFAL_NFCV_CRC_LEN                 2U              /*!< NFC-V CRC length                                             */
#define RFAL_NFCV_MAX_GEN_DATA_LEN        (RFAL_NFCV_MAX_BLOCK_LEN + RFAL_NFCV_BNO_LEN + RFAL_NFCV_UID_LEN) /*!<Max data    */ 
#define RFAL_NFCV_BLOCKNUM_LEN            1U              /*!< Block Number length on normal commands: 8 bits               */
#define RFAL_NFCV_BLOCKNUM_EXTENDED_LEN   2U              /*!< Block Number length on extended commands: 16 bits            */
#define RFAL_NFCV_PARAM_SKIP              0U              /*!< Skip proprietary Param Request                               */
                                                                                                                            
                                                                                                                            
                                                                                                                            
                                                                                                                            
/*! NFC-V RequestFlags   ISO15693 2000 7.3.1 */                                                                             
enum{                                                                                                                       
    RFAL_NFCV_REQ_FLAG_DEFAULT           = 0x02U,         /*!< Default Request Flags                                        */
    RFAL_NFCV_REQ_FLAG_SUB_CARRIER       = 0x01U,         /*!< Sub Carrier flag                                             */
    RFAL_NFCV_REQ_FLAG_DATA_RATE         = 0x02U,         /*!< Data Rate flag                                               */
    RFAL_NFCV_REQ_FLAG_INVENTORY         = 0x04U,         /*!< Inventory flag                                               */
    RFAL_NFCV_REQ_FLAG_PROTOCOL_EXT      = 0x08U,         /*!< Protocol Extension flag                                      */
    RFAL_NFCV_REQ_FLAG_SELECT            = 0x10U,         /*!< Select flag                                                  */
    RFAL_NFCV_REQ_FLAG_ADDRESS           = 0x20U,         /*!< Address flag                                                 */
    RFAL_NFCV_REQ_FLAG_OPTION            = 0x40U,         /*!< Option flag                                                  */
    RFAL_NFCV_REQ_FLAG_RFU               = 0x80U,         /*!< RFU flag                                                     */
    RFAL_NFCV_REQ_FLAG_AFI               = 0x10U,         /*!< AFI flag                                                     */
    RFAL_NFCV_REQ_FLAG_NB_SLOTS          = 0x20U,         /*!< Number of Slots flag                                         */
};                                                                                                                          
                                                                                                                            
/*! NFC-V Response Flags   ISO15693 2000 7.4.1 */                                                                           
enum{                                                                                                                       
    RFAL_NFCV_RES_FLAG_ERROR             = 0x01U,         /*!< Error flag                                                   */
    RFAL_NFCV_RES_FLAG_RFU1              = 0x02U,         /*!< RFU flag                                                     */
    RFAL_NFCV_RES_FLAG_RFU2              = 0x04U,         /*!< RFU flag                                                     */
    RFAL_NFCV_RES_FLAG_EXTENSION         = 0x08U,         /*!< Extension flag                                               */
    RFAL_NFCV_RES_FLAG_RFU3              = 0x10U,         /*!< RFU flag                                                     */
    RFAL_NFCV_RES_FLAG_RFU4              = 0x20U,         /*!< RFU flag                                                     */
    RFAL_NFCV_RES_FLAG_RFU5              = 0x40U,         /*!< RFU flag                                                     */
    RFAL_NFCV_RES_FLAG_RFU6              = 0x80U          /*!< RFU flag                                                     */
};                                                                                                                          
                                                                                                                            
/*! NFC-V Error code  ISO15693 2000 7.4.2 */                                                                                
enum{                                                                                                                       
    RFAL_NFCV_ERROR_CMD_NOT_SUPPORTED    = 0x01U,         /*!< The command is not supported, code is not recognised         */
    RFAL_NFCV_ERROR_CMD_NOT_RECOGNIZED   = 0x02U,         /*!< The command is not recognised, format error occurred         */
    RFAL_NFCV_ERROR_OPTION_NOT_SUPPORTED = 0x03U,         /*!< The option is not supported                                  */
    RFAL_NFCV_ERROR_UNKNOWN              = 0x0FU,         /*!< Unknown error                                                */
    RFAL_NFCV_ERROR_BLOCK_NOT_AVALIABLE  = 0x10U,         /*!< The specified block is not available                         */
    RFAL_NFCV_ERROR_BLOCK_ALREDY_LOCKED  = 0x11U,         /*!< The specified block is already locked                        */
    RFAL_NFCV_ERROR_BLOCK_LOCKED         = 0x12U,         /*!< The specified block is locked                                */
    RFAL_NFCV_ERROR_WRITE_FAILED         = 0x13U,         /*!< The specified block was not successfully programmed          */
    RFAL_NFCV_ERROR_BLOCK_FAILED         = 0x14U          /*!< The specified block was not successfully locked              */
};


/*! NFC-V command set   ISO15693 2000 9.1 */
enum 
{
    RFAL_NFCV_CMD_INVENTORY                     = 0x01U,  /*!< INVENTORY_REQ (Inventory) command                            */
    RFAL_NFCV_CMD_SLPV                          = 0x02U,  /*!< SLPV_REQ (Stay quiet) command                                */
    RFAL_NFCV_CMD_READ_SINGLE_BLOCK             = 0x20U,  /*!< Read single block command                                    */
    RFAL_NFCV_CMD_WRITE_SINGLE_BLOCK            = 0x21U,  /*!< Write single block command                                   */
    RFAL_NFCV_CMD_LOCK_BLOCK                    = 0x22U,  /*!< Lock block command                                           */
    RFAL_NFCV_CMD_READ_MULTIPLE_BLOCKS          = 0x23U,  /*!< Read multiple blocks command                                 */
    RFAL_NFCV_CMD_WRITE_MULTIPLE_BLOCKS         = 0x24U,  /*!< Write multiple blocks command                                */
    RFAL_NFCV_CMD_SELECT                        = 0x25U,  /*!< Select command                                               */
    RFAL_NFCV_CMD_RESET_TO_READY                = 0x26U,  /*!< Reset To Ready command                                       */
    RFAL_NFCV_CMD_GET_SYS_INFO                  = 0x2BU,  /*!< Get System Information command                               */
    RFAL_NFCV_CMD_EXTENDED_READ_SINGLE_BLOCK    = 0x30U,  /*!< Extended read single block command                           */
    RFAL_NFCV_CMD_EXTENDED_WRITE_SINGLE_BLOCK   = 0x31U,  /*!< Extended write single block command                          */
    RFAL_NFCV_CMD_EXTENDED_LOCK_SINGLE_BLOCK    = 0x32U,  /*!< Extended lock single block command                           */
    RFAL_NFCV_CMD_EXTENDED_READ_MULTIPLE_BLOCK  = 0x33U,  /*!< Extended read multiple block command                         */
    RFAL_NFCV_CMD_EXTENDED_WRITE_MULTIPLE_BLOCK = 0x34U,  /*!< Extended read multiple block command                         */
    RFAL_NFCV_CMD_EXTENDED_GET_SYS_INFO         = 0x3BU   /*!< Extended Get System Information command                      */
};

/*! ST25TV/ST25DV command set  */
enum 
{
    RFAL_NFCV_CMD_READ_CONFIGURATION                 = 0xA0U,  /*!< Read configuration command                                 */ 
    RFAL_NFCV_CMD_WRITE_CONFIGURATION                = 0xA1U,  /*!< Write configuration command                                */ 
    RFAL_NFCV_CMD_SET_EAS                            = 0xA2U,  /*!< Set EAS command                                            */
    RFAL_NFCV_CMD_RESET_EAS                          = 0xA3U,  /*!< Reset EAS command                                          */
    RFAL_NFCV_CMD_LOCK_EAS                           = 0xA4U,  /*!< Lock EAS command                                           */
    RFAL_NFCV_CMD_ENABLE_EAS                         = 0xA5U,  /*!< Enable EAS command                                         */
    RFAL_NFCV_CMD_KILL                               = 0xA6U,  /*!< Kill command                                               */
    RFAL_NFCV_CMD_WRITE_EAS_ID                       = 0xA7U,  /*!< Write EAS ID command                                       */
    RFAL_NFCV_CMD_WRITE_EAS_CONFIG                   = 0xA8U,  /*!< Write EAS CONFIG command                                   */
    RFAL_NFCV_CMD_MANAGE_GPO                         = 0xA9U,  /*!< Manage GPO command                                         */
    RFAL_NFCV_CMD_WRITE_MESSAGE                      = 0xAAU,  /*!< Write Message command                                      */
    RFAL_NFCV_CMD_READ_MESSAGE_LENGTH                = 0xABU,  /*!< Read Message Length command                                */
    RFAL_NFCV_CMD_READ_MESSAGE                       = 0xACU,  /*!< Read Message command                                       */
    RFAL_NFCV_CMD_READ_DYN_CONFIGURATION             = 0xADU,  /*!< Read Dynamic Configuration command                         */
    RFAL_NFCV_CMD_WRITE_DYN_CONFIGURATION            = 0xAEU,  /*!< Write Dynamic Configuration command                        */
    RFAL_NFCV_CMD_WRITE_PASSWORD                     = 0xB1U,  /*!< Write Kill Password / Write Password command               */
    RFAL_NFCV_CMD_LOCK_KILL                          = 0xB2U,  /*!< Lock Kill command                                          */
    RFAL_NFCV_CMD_PRESENT_PASSWORD                   = 0xB3U,  /*!< Present Password command                                   */ 
    RFAL_NFCV_CMD_GET_RANDOM_NUMBER                  = 0xB4U,  /*!< Get Random Number command                                  */ 
    RFAL_NFCV_CMD_FAST_READ_SINGLE_BLOCK             = 0xC0U,  /*!< Fast Read single block command                             */
    RFAL_NFCV_CMD_FAST_READ_MULTIPLE_BLOCKS          = 0xC3U,  /*!< Fast Read multiple blocks command                          */
    RFAL_NFCV_CMD_FAST_EXTENDED_READ_SINGLE_BLOCK    = 0xC4U,  /*!< Fast Extended Read single block command                    */
    RFAL_NFCV_CMD_FAST_EXTENDED_READ_MULTIPLE_BLOCKS = 0xC5U,  /*!< Fast Extended Read multiple blocks command                 */
    RFAL_NFCV_CMD_FAST_WRITE_MESSAGE                 = 0xCAU,  /*!< Fast Write Message                                         */
    RFAL_NFCV_CMD_FAST_READ_MESSAGE_LENGTH           = 0xCBU,  /*!< Fast Read Message Length                                   */
    RFAL_NFCV_CMD_FAST_READ_MESSAGE                  = 0xCCU,  /*!< Fast Read Message                                          */
    RFAL_NFCV_CMD_FAST_READ_DYN_CONFIGURATION        = 0xCDU,  /*!< Fast Read Dynamic configuration                            */
    RFAL_NFCV_CMD_FAST_WRITE_DYN_CONFIGURATION       = 0xCEU   /*!< Fast Write Dynamic Configuration                           */
};

/*! ISO 15693 Get System info parameter request field ISO15693 2018 Table 94 */
enum
{
    RFAL_NFCV_SYSINFO_DFSID      = 0x01U,                 /*!< Get System info DFSID flag                                   */
    RFAL_NFCV_SYSINFO_AFI        = 0x02U,                 /*!< Get System info AFI flag                                     */
    RFAL_NFCV_SYSINFO_MEMSIZE    = 0x04U,                 /*!< Get System info MEMSIZE flag                                 */
    RFAL_NFCV_SYSINFO_ICREF      = 0x08U,                 /*!< Get System info ICREF flag                                   */
    RFAL_NFCV_SYSINFO_MOI        = 0x10U,                 /*!< Get System info MOI flag                                     */
    RFAL_NFCV_SYSINFO_CMDLIST    = 0x20U,                 /*!< Get System info CMDLIST flag                                 */
    RFAL_NFCV_SYSINFO_CSI        = 0x40U,                 /*!< Get System info CSI flag                                     */
    RFAL_NFCV_SYSINFO_REQ_ALL    = 0x7FU                  /*!< Get System info request of all parameters                    */
};

/*
 ******************************************************************************
 * GLOBAL MACROS
 ******************************************************************************
 */


/*
******************************************************************************
* GLOBAL TYPES
******************************************************************************
*/

/*! NFC-V Number of slots  Digital 2.0  9.6.1 */
typedef enum 
{
    RFAL_NFCV_NUM_SLOTS_1    =  0x20,   /*!< Number of slots: 1             */
    RFAL_NFCV_NUM_SLOTS_16   =  0x00,   /*!< Number of slots: 16            */
} rfalNfcvNumSlots;


/*! NFC-V INVENTORY_RES format   Digital 2.0  9.6.2 */
typedef struct 
{
    uint8_t RES_FLAG;                   /*!< Response Flags                 */
    uint8_t DSFID;                      /*!< Data Storage Format Identifier */
    uint8_t UID[RFAL_NFCV_UID_LEN];     /*!< NFC-V device UID               */
    uint8_t crc[RFAL_CRC_LEN];          /*!< CRC                            */
} rfalNfcvInventoryRes;


/*! NFC-V Generic Req format  */
typedef struct
{
    uint8_t  REQ_FLAG;                              /*!< Request Flags      */
    uint8_t  CMD;                                   /*!< Command code       */
    union { /*  PRQA S 0750 # MISRA 19.2 - Both members are of the same type, just different names.  Thus no problem can occur. */
        uint8_t  UID[RFAL_NFCV_UID_LEN];            /*!< Mask Value         */
        uint8_t  data[RFAL_NFCV_MAX_GEN_DATA_LEN];  /*!< Data               */
    }payload;                                       /*!< Payload            */
} rfalNfcvGenericReq;


/*! NFC-V Generic Response format */
typedef struct
{
    uint8_t  RES_FLAG;                              /*!< Response Flags     */
    uint8_t  data[RFAL_NFCV_MAX_GEN_DATA_LEN];      /*!< Data               */
} rfalNfcvGenericRes;


/*! NFC-V listener device (VICC) struct  */
typedef struct
{
    rfalNfcvInventoryRes    InvRes;     /*!< INVENTORY_RES                  */
    bool                    isSleep;    /*!< Device sleeping flag           */
} rfalNfcvListenDevice;


/*
******************************************************************************
* GLOBAL FUNCTION PROTOTYPES
******************************************************************************
*/

/*! 
 *****************************************************************************
 * \brief  Initialize NFC-V Poller mode
 *  
 * This methods configures RFAL RF layer to perform as a 
 * NFC-F Poller/RW (ISO15693) including all default timings 
 *
 * \return ERR_WRONG_STATE  : RFAL not initialized or mode not set
 * \return ERR_PARAM        : Incorrect bitrate
 * \return ERR_NONE         : No error
 *****************************************************************************
 */
ReturnCode rfalNfcvPollerInitialize( void );

/*! 
 *****************************************************************************
 * \brief  NFC-V Poller Check Presence
 *  
 * This method checks if a NFC-V Listen device (VICC) is present on the field
 * by sending an Inventory (INVENTORY_REQ) 
 *  
 * \param[out] invRes : If received, the INVENTORY_RES
 *
 * \return ERR_WRONG_STATE  : RFAL not initialized or incorrect mode
 * \return ERR_PARAM        : Invalid parameters
 * \return ERR_IO           : Generic internal error
 * \return ERR_TIMEOUT      : Timeout error, no listener device detectedd
 * \return ERR_NONE         : No error, one or more device in the field
 *****************************************************************************
 */
ReturnCode rfalNfcvPollerCheckPresence( rfalNfcvInventoryRes *invRes );

/*! 
 *****************************************************************************
 * \brief NFC-F Poller Poll
 * 
 * This function sends to all VICCs in field the INVENTORY command with the 
 * given number of slots
 * 
 * If more than one slot is used the following EOF need to be handled
 * by the caller using rfalISO15693TransceiveEOFAnticollision()
 *
 * \param[in]  nSlots  : Number of Slots to be sent (1 or 16)
 * \param[in]  maskLen : Number bits on the Mask value
 * \param[in]  maskVal : location of the Mask value
 * \param[out] invRes  : location to place the INVENTORY_RES
 * \param[out] rcvdLen : number of bits received (without collision)
 * 
 * \return ERR_WRONG_STATE  : RFAL not initialized or incorrect mode
 * \return ERR_PARAM        : Invalid parameters
 * \return ERR_IO           : Generic internal error
 * \return ERR_RF_COLLISION : Collision detected 
 * \return ERR_CRC          : CRC error detected
 * \return ERR_PROTO        : Protocol error detected
 * \return ERR_NONE         : No error
 *****************************************************************************
 */ 
ReturnCode rfalNfcvPollerInventory( rfalNfcvNumSlots nSlots, uint8_t maskLen, const uint8_t *maskVal, rfalNfcvInventoryRes *invRes, uint16_t* rcvdLen );

/*! 
 *****************************************************************************
 * \brief  NFC-V Poller Full Collision Resolution
 *  
 * Performs a full Collision resolution as defined in Activity 2.0   9.3.7
 * Once done, the devCnt will indicate how many (if any) devices have 
 * been identified and their details are contained on nfcvDevList
 *
 * \param[in]  compMode     : compliance mode to be performed
 * \param[in]  devLimit     : device limit value, and size nfcaDevList
 * \param[out] nfcvDevList  : NFC-v listener devices list
 * \param[out] devCnt       : Devices found counter
 *
 * When compMode is set to ISO the function immediately goes to 16 slots improving
 * chances to detect more than only one strong card.
 *
 * \return ERR_WRONG_STATE  : RFAL not initialized or mode not set
 * \return ERR_PARAM        : Invalid parameters
 * \return ERR_IO           : Generic internal error
 * \return ERR_NONE         : No error
 *****************************************************************************
 */
ReturnCode rfalNfcvPollerCollisionResolution( rfalComplianceMode compMode, uint8_t devLimit, rfalNfcvListenDevice *nfcvDevList, uint8_t *devCnt );

/*!
 *****************************************************************************
 * \brief  NFC-V Poller Full Collision Resolution With Sleep
 *
 * Performs a full Collision resolution which is different from Activity 2.0 9.3.7.
 * The implementation uses SLPV (StayQuiet) command to make sure all cards are found.
 * Once done, the devCnt will indicate how many (if any) devices have
 * been identified and their details are contained on nfcvDevList
 *
 * \param[in]  devLimit     : device limit value, and size nfcaDevList
 * \param[out] nfcvDevList  : NFC-v listener devices list
 * \param[out] devCnt       : Devices found counter
 *
 * \return ERR_WRONG_STATE  : RFAL not initialized or mode not set
 * \return ERR_PARAM        : Invalid parameters
 * \return ERR_IO           : Generic internal error
 * \return ERR_NONE         : No error
 *****************************************************************************
 */
ReturnCode rfalNfcvPollerSleepCollisionResolution( uint8_t devLimit, rfalNfcvListenDevice *nfcvDevList, uint8_t *devCnt );

/*! 
 *****************************************************************************
 * \brief  NFC-V Poller Sleep
 *  
 * This function is used to send the SLPV_REQ (Stay Quiet) command to put the VICC 
 * with the given UID to state QUIET so that they do not reply to more Inventory
 * 
 * \param[in]  flags        : Flags to be used: Sub-carrier; Data_rate; Option
 *                            for NFC-Forum use: RFAL_NFCV_REQ_FLAG_DEFAULT
 * \param[in]  uid          : UID of the device to be put to Sleep
 *  
 * \return ERR_WRONG_STATE  : RFAL not initialized or incorrect mode
 * \return ERR_PARAM        : Invalid parameters
 * \return ERR_IO           : Generic internal error
 * \return ERR_NONE         : No error
 *****************************************************************************
 */
ReturnCode rfalNfcvPollerSleep( uint8_t flags, const uint8_t* uid );

/*! 
 *****************************************************************************
 * \brief  NFC-V Poller Select
 *  
 * Selects a device (VICC) by its UID 
 *
 * \param[in]  flags        : Flags to be used: Sub-carrier; Data_rate; Option
 *                            for NFC-Forum use: RFAL_NFCV_REQ_FLAG_DEFAULT
 * \param[in]  uid          : UID of the device to be put to be Selected
 *  
 * \return ERR_WRONG_STATE  : RFAL not initialized or incorrect mode
 * \return ERR_PARAM        : Invalid parameters
 * \return ERR_IO           : Generic internal error 
 * \return ERR_CRC          : CRC error detected
 * \return ERR_FRAMING      : Framing error detected
 * \return ERR_PROTO        : Protocol error detected
 * \return ERR_TIMEOUT      : Timeout error
 * \return ERR_NONE         : No error
 *****************************************************************************
 */
ReturnCode rfalNfcvPollerSelect( uint8_t flags, const uint8_t* uid );

/*! 
 *****************************************************************************
 * \brief  NFC-V Poller Read Single Block
 *  
 * Reads a Single Block from a device (VICC)  
 *
 * \param[in]  flags        : Flags to be used: Sub-carrier; Data_rate; Option
 *                            for NFC-Forum use: RFAL_NFCV_REQ_FLAG_DEFAULT
 * \param[in]  uid          : UID of the device to be put to be read
 *                             if UID is provided Addressed mode will be used
 * \param[in]  blockNum     : Number of the block to read
 * \param[out] rxBuf        : buffer to store response (also with RES_FLAGS)
 * \param[in]  rxBufLen     : length of rxBuf
 * \param[out] rcvLen       : number of bytes received
 *  
 * \return ERR_WRONG_STATE  : RFAL not initialized or incorrect mode
 * \return ERR_PARAM        : Invalid parameters
 * \return ERR_IO           : Generic internal error 
 * \return ERR_CRC          : CRC error detected
 * \return ERR_FRAMING      : Framing error detected
 * \return ERR_PROTO        : Protocol error detected
 * \return ERR_TIMEOUT      : Timeout error
 * \return ERR_NONE         : No error
 *****************************************************************************
 */
ReturnCode rfalNfcvPollerReadSingleBlock( uint8_t flags, const uint8_t* uid, uint8_t blockNum, uint8_t* rxBuf, uint16_t rxBufLen, uint16_t *rcvLen );

/*! 
 *****************************************************************************
 * \brief  NFC-V Poller Write Single Block
 *  
 * Writes a Single Block from a device (VICC)
 *
 * \param[in]  flags        : Flags to be used: Sub-carrier; Data_rate; Option
 *                            for NFC-Forum use: RFAL_NFCV_REQ_FLAG_DEFAULT
 * \param[in]  uid          : UID of the device to be put to be written
 *                             if UID is provided Addressed mode will be used
 * \param[in]  blockNum     : Number of the block to write
 * \param[in]  wrData       : data to be written on the given block
 * \param[in]  blockLen     : number of bytes of a block
 *  
 * \return ERR_WRONG_STATE  : RFAL not initialized or incorrect mode
 * \return ERR_PARAM        : Invalid parameters
 * \return ERR_IO           : Generic internal error 
 * \return ERR_CRC          : CRC error detected
 * \return ERR_FRAMING      : Framing error detected
 * \return ERR_PROTO        : Protocol error detected
 * \return ERR_TIMEOUT      : Timeout error
 * \return ERR_NONE         : No error
 *****************************************************************************
 */
ReturnCode rfalNfcvPollerWriteSingleBlock( uint8_t flags, const uint8_t* uid, uint8_t blockNum, const uint8_t* wrData, uint8_t blockLen );

/*! 
 *****************************************************************************
 * \brief  NFC-V Poller Read Multiple Blocks
 *  
 * Reads Multiple Blocks from a device (VICC)  
 *
 * \param[in]  flags          : Flags to be used: Sub-carrier; Data_rate; Option
 *                              for NFC-Forum use: RFAL_NFCV_REQ_FLAG_DEFAULT
 * \param[in]  uid            : UID of the device to be put to be read
 *                               if UID is provided Addressed mode will be used
 * \param[in]  firstBlockNum  : first block to be read
 * \param[in]  numOfBlocks    : number of block to read
 * \param[out] rxBuf          : buffer to store response (also with RES_FLAGS)
 * \param[in]  rxBufLen       : length of rxBuf
 * \param[out] rcvLen         : number of bytes received
 *  
 * \return ERR_WRONG_STATE    : RFAL not initialized or incorrect mode
 * \return ERR_PARAM          : Invalid parameters
 * \return ERR_IO             : Generic internal error 
 * \return ERR_CRC            : CRC error detected
 * \return ERR_FRAMING        : Framing error detected
 * \return ERR_PROTO          : Protocol error detected
 * \return ERR_TIMEOUT        : Timeout error
 * \return ERR_NONE           : No error
 *****************************************************************************
 */
ReturnCode rfalNfcvPollerReadMultipleBlocks( uint8_t flags, const uint8_t* uid, uint8_t firstBlockNum, uint8_t numOfBlocks, uint8_t* rxBuf, uint16_t rxBufLen, uint16_t *rcvLen );

/*! 
 *****************************************************************************
 * \brief  NFC-V Poller Write Multiple Blocks
 *  
 * Reads Multiple Blocks from a device (VICC)
 * In order to not limit the length of the Write multiple command, a buffer
 * must be provided where the request will be composed and then sent.
 *
 * \param[in]  flags          : Flags to be used: Sub-carrier; Data_rate; Option
 *                              for NFC-Forum use: RFAL_NFCV_REQ_FLAG_DEFAULT
 * \param[in]  uid            : UID of the device to be put to be read
 *                               if UID is provided Addressed mode will be used
 * \param[in]  firstBlockNum  : first block to be write
 * \param[in]  numOfBlocks    : number of consecutive blocks to write
 * \param[in]  txBuf          : buffer where the request will be composed
 * \param[in]  txBufLen       : length of txBuf
 * \param[in]  blockLen       : number of bytes of a block
 * \param[in]  wrData         : data to be written
 * \param[in]  wrDataLen      : length of the data do be written. Must be
 *                              aligned with number of blocks to write and
 *                              the size of a block
 *  
 * \return ERR_WRONG_STATE    : RFAL not initialized or incorrect mode
 * \return ERR_PARAM          : Invalid parameters
 * \return ERR_IO             : Generic internal error 
 * \return ERR_CRC            : CRC error detected
 * \return ERR_FRAMING        : Framing error detected
 * \return ERR_PROTO          : Protocol error detected
 * \return ERR_TIMEOUT        : Timeout error
 * \return ERR_NONE           : No error
 *****************************************************************************
 */
ReturnCode rfalNfcvPollerWriteMultipleBlocks( uint8_t flags, const uint8_t* uid, uint8_t firstBlockNum, uint8_t numOfBlocks, uint8_t *txBuf, uint16_t txBufLen, uint8_t blockLen, const uint8_t* wrData, uint16_t wrDataLen );

/*! 
 *****************************************************************************
 * \brief  NFC-V Poller Extended Lock Single Block
 *  
 * Blocks a Single Block from a device (VICC) supporting extended commands
 *
 * \param[in]  flags        : Flags to be used: Sub-carrier; Data_rate; Option
 *                            for NFC-Forum use: RFAL_NFCV_REQ_FLAG_DEFAULT
 * \param[in]  uid          : UID of the device
 *                             if UID is provided Addressed mode will be used
 * \param[in]  blockNum     : Number of the block to be locked
 *  
 * \return ERR_WRONG_STATE  : RFAL not initialized or incorrect mode
 * \return ERR_PARAM        : Invalid parameters
 * \return ERR_IO           : Generic internal error 
 * \return ERR_CRC          : CRC error detected
 * \return ERR_FRAMING      : Framing error detected
 * \return ERR_PROTO        : Protocol error detected
 * \return ERR_TIMEOUT      : Timeout error
 * \return ERR_NONE         : No error
 *****************************************************************************
 */
ReturnCode rfalNfcvPollerLockBlock( uint8_t flags, const uint8_t* uid, uint8_t blockNum );
    
/*! 
 *****************************************************************************
 * \brief  NFC-V Poller Extended Lock Single Block
 *  
 * Blocks a Single Block from a device (VICC) supporting extended commands
 *
 * \param[in]  flags        : Flags to be used: Sub-carrier; Data_rate; Option
 *                            for NFC-Forum use: RFAL_NFCV_REQ_FLAG_DEFAULT
 * \param[in]  uid          : UID of the device
 *                             if UID is provided Addressed mode will be used
 * \param[in]  blockNum     : Number of the block to be locked (16 bits)
 *  
 * \return ERR_WRONG_STATE  : RFAL not initialized or incorrect mode
 * \return ERR_PARAM        : Invalid parameters
 * \return ERR_IO           : Generic internal error 
 * \return ERR_CRC          : CRC error detected
 * \return ERR_FRAMING      : Framing error detected
 * \return ERR_PROTO        : Protocol error detected
 * \return ERR_TIMEOUT      : Timeout error
 * \return ERR_NONE         : No error
 *****************************************************************************
 */
ReturnCode rfalNfcvPollerExtendedLockSingleBlock( uint8_t flags, const uint8_t* uid, uint16_t blockNum );

/*! 
 *****************************************************************************
 * \brief  NFC-V Poller Extended Read Single Block
 *  
 * Reads a Single Block from a device (VICC) supporting extended commands
 *
 * \param[in]  flags        : Flags to be used: Sub-carrier; Data_rate; Option
 *                            for NFC-Forum use: RFAL_NFCV_REQ_FLAG_DEFAULT
 * \param[in]  uid          : UID of the device to be put to be read
 *                             if UID is provided Addressed mode will be used
 * \param[in]  blockNum     : Number of the block to read (16 bits)
 * \param[out] rxBuf        : buffer to store response (also with RES_FLAGS)
 * \param[in]  rxBufLen     : length of rxBuf
 * \param[out] rcvLen       : number of bytes received
 *  
 * \return ERR_WRONG_STATE  : RFAL not initialized or incorrect mode
 * \return ERR_PARAM        : Invalid parameters
 * \return ERR_IO           : Generic internal error 
 * \return ERR_CRC          : CRC error detected
 * \return ERR_FRAMING      : Framing error detected
 * \return ERR_PROTO        : Protocol error detected
 * \return ERR_TIMEOUT      : Timeout error
 * \return ERR_NONE         : No error
 *****************************************************************************
 */
ReturnCode rfalNfcvPollerExtendedReadSingleBlock( uint8_t flags, const uint8_t* uid, uint16_t blockNum, uint8_t* rxBuf, uint16_t rxBufLen, uint16_t *rcvLen );

/*! 
 *****************************************************************************
 * \brief  NFC-V Poller Extended Write Single Block
 *  
 * Writes a Single Block from a device (VICC) supporting extended commands
 *
 * \param[in]  flags        : Flags to be used: Sub-carrier; Data_rate; Option
 *                            for NFC-Forum use: RFAL_NFCV_REQ_FLAG_DEFAULT
 * \param[in]  uid          : UID of the device
 *                             if UID is provided Addressed mode will be used
 * \param[in]  blockNum     : Number of the block to write (16 bits)
 * \param[in]  wrData       : data to be written on the given block
 * \param[in]  blockLen     : number of bytes of a block
 *  
 * \return ERR_WRONG_STATE  : RFAL not initialized or incorrect mode
 * \return ERR_PARAM        : Invalid parameters
 * \return ERR_IO           : Generic internal error 
 * \return ERR_CRC          : CRC error detected
 * \return ERR_FRAMING      : Framing error detected
 * \return ERR_PROTO        : Protocol error detected
 * \return ERR_TIMEOUT      : Timeout error
 * \return ERR_NONE         : No error
 *****************************************************************************
 */
ReturnCode rfalNfcvPollerExtendedWriteSingleBlock( uint8_t flags, const uint8_t* uid, uint16_t blockNum, const uint8_t* wrData, uint8_t blockLen );

/*! 
 *****************************************************************************
 * \brief  NFC-V Poller Extended Read Multiple Blocks
 *  
 * Reads Multiple Blocks from a device (VICC) supporting extended commands  
 *
 * \param[in]  flags          : Flags to be used: Sub-carrier; Data_rate; Option
 *                              for NFC-Forum use: RFAL_NFCV_REQ_FLAG_DEFAULT
 * \param[in]  uid            : UID of the device to be put to be read
 *                               if UID is provided Addressed mode will be used
 * \param[in]  firstBlockNum  : first block to be read (16 bits)
 * \param[in]  numOfBlocks    : number of consecutive blocks to read (16 bits)
 * \param[out] rxBuf          : buffer to store response (also with RES_FLAGS)
 * \param[in]  rxBufLen       : length of rxBuf
 * \param[out] rcvLen         : number of bytes received
 *  
 * \return ERR_WRONG_STATE    : RFAL not initialized or incorrect mode
 * \return ERR_PARAM          : Invalid parameters
 * \return ERR_IO             : Generic internal error 
 * \return ERR_CRC            : CRC error detected
 * \return ERR_FRAMING        : Framing error detected
 * \return ERR_PROTO          : Protocol error detected
 * \return ERR_TIMEOUT        : Timeout error
 * \return ERR_NONE           : No error
 *****************************************************************************
 */
ReturnCode rfalNfcvPollerExtendedReadMultipleBlocks( uint8_t flags, const uint8_t* uid, uint16_t firstBlockNum, uint16_t numOfBlocks, uint8_t* rxBuf, uint16_t rxBufLen, uint16_t *rcvLen );

/*! 
 *****************************************************************************
 * \brief  NFC-V Poller Extended Write Multiple Blocks
 *  
 * Writes Multiple Blocks from a device (VICC) supporting extended commands 
 * In order to not limit the length of the Write multiple command, a buffer
 * must be provided where the request will be composed and then sent.
 *
 * \param[in]  flags          : Flags to be used: Sub-carrier; Data_rate; Option
 *                              for NFC-Forum use: RFAL_NFCV_REQ_FLAG_DEFAULT
 * \param[in]  uid            : UID of the device to be put to be read
 *                               if UID is provided Addressed mode will be used
 * \param[in]  firstBlockNum  : first block to be write (16 bits)
 * \param[in]  numOfBlocks    : number of consecutive blocks to write (16 bits)
 * \param[in]  txBuf          : buffer where the request will be composed
 * \param[in]  txBufLen       : length of txBuf
 * \param[in]  blockLen       : number of bytes of a block
 * \param[in]  wrData         : data to be written
 * \param[in]  wrDataLen      : length of the data do be written. Must be
 *                              aligned with number of blocks to write and
 *                              the size of a block
 *  
 * \return ERR_WRONG_STATE    : RFAL not initialized or incorrect mode
 * \return ERR_PARAM          : Invalid parameters
 * \return ERR_IO             : Generic internal error 
 * \return ERR_CRC            : CRC error detected
 * \return ERR_FRAMING        : Framing error detected
 * \return ERR_PROTO          : Protocol error detected
 * \return ERR_TIMEOUT        : Timeout error
 * \return ERR_NONE           : No error
 *****************************************************************************
 */
ReturnCode rfalNfcvPollerExtendedWriteMultipleBlocks( uint8_t flags, const uint8_t* uid, uint16_t firstBlockNum, uint16_t numOfBlocks, uint8_t *txBuf, uint16_t txBufLen, uint8_t blockLen, const uint8_t* wrData, uint16_t wrDataLen );

/*! 
 *****************************************************************************
 * \brief  NFC-V Get System Information
 *  
 * Sends Get System Information command  
 *
 * \param[in]  flags          : Flags to be used: Sub-carrier; Data_rate; Option
 *                              for NFC-Forum use: RFAL_NFCV_REQ_FLAG_DEFAULT
 * \param[in]  uid            : UID of the device to be put to be read
 *                               if UID is provided Addressed mode will be used
 * \param[out] rxBuf          : buffer to store response (also with RES_FLAGS)
 * \param[in]  rxBufLen       : length of rxBuf
 * \param[out] rcvLen         : number of bytes received
 *  
 * \return ERR_WRONG_STATE    : RFAL not initialized or incorrect mode
 * \return ERR_PARAM          : Invalid parameters
 * \return ERR_IO             : Generic internal error 
 * \return ERR_CRC            : CRC error detected
 * \return ERR_FRAMING        : Framing error detected
 * \return ERR_PROTO          : Protocol error detected
 * \return ERR_TIMEOUT        : Timeout error
 * \return ERR_NONE           : No error
 *****************************************************************************
 */
ReturnCode rfalNfcvPollerGetSystemInformation( uint8_t flags, const uint8_t* uid, uint8_t* rxBuf, uint16_t rxBufLen, uint16_t *rcvLen );

/*! 
 *****************************************************************************
 * \brief  NFC-V Extended Get System Information
 *  
 * Sends Extended Get System Information command  
 *
 * \param[in]  flags          : Flags to be used: Sub-carrier; Data_rate; Option
 *                              for NFC-Forum use: RFAL_NFCV_REQ_FLAG_DEFAULT
 * \param[in]  uid            : UID of the device to be put to be read
 *                               if UID is provided Addressed mode will be used
 * \param[in]  requestField   : Get System info parameter request field
 * \param[out] rxBuf          : buffer to store response (also with RES_FLAGS)
 * \param[in]  rxBufLen       : length of rxBuf
 * \param[out] rcvLen         : number of bytes received
 *  
 * \return ERR_WRONG_STATE    : RFAL not initialized or incorrect mode
 * \return ERR_PARAM          : Invalid parameters
 * \return ERR_IO             : Generic internal error 
 * \return ERR_CRC            : CRC error detected
 * \return ERR_FRAMING        : Framing error detected
 * \return ERR_PROTO          : Protocol error detected
 * \return ERR_TIMEOUT        : Timeout error
 * \return ERR_NONE           : No error
 *****************************************************************************
 */
ReturnCode rfalNfcvPollerExtendedGetSystemInformation( uint8_t flags, const uint8_t* uid, uint8_t requestField, uint8_t* rxBuf, uint16_t rxBufLen, uint16_t *rcvLen );


/*! 
 *****************************************************************************
 * \brief  NFC-V Transceive Request
 *  
 * Performs a generic transceive with an ISO15693 tag
 *
 * \param[in]  cmd            : NFC-V command
 * \param[in]  flags          : Flags to be used: Sub-carrier; Data_rate; Option
 *                              for NFC-Forum use: RFAL_NFCV_REQ_FLAG_DEFAULT
 * \param[in]  param          : Prepend parameter on certain proprietary requests
 *                              For default commands skip: RFAL_NFCV_PARAM_SKIP
 * \param[in]  uid            : UID of the device to be put to be read
 *                               if UID is provided Addressed mode will be used
 * \param[in]  data           : command parameters append after UID
 * \param[in]  dataLen        : command parameters Len
 * \param[out] rxBuf          : buffer to store response (also with RES_FLAGS)
 * \param[in]  rxBufLen       : length of rxBuf
 * \param[out] rcvLen         : number of bytes received
 *  
 * \return ERR_WRONG_STATE    : RFAL not initialized or incorrect mode
 * \return ERR_PARAM          : Invalid parameters
 * \return ERR_IO             : Generic internal error 
 * \return ERR_CRC            : CRC error detected
 * \return ERR_FRAMING        : Framing error detected
 * \return ERR_PROTO          : Protocol error detected
 * \return ERR_TIMEOUT        : Timeout error
 * \return ERR_NONE           : No error
 *****************************************************************************
 */
ReturnCode rfalNfcvPollerTransceiveReq( uint8_t cmd, uint8_t flags, uint8_t param, const uint8_t* uid, const uint8_t *data, uint16_t dataLen, uint8_t* rxBuf, uint16_t rxBufLen, uint16_t *rcvLen );

#endif /* RFAL_NFCV_H */

/**
  * @}
  *
  * @}
  *
  * @}
  */

