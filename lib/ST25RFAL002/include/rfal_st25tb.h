
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

/*! \file rfal_st25tb.h
 *
 *  \author Gustavo Patricio
 *
 *  \brief Implementation of ST25TB interface 
 *
 *
 * \addtogroup RFAL
 * @{
 *
 * \addtogroup RFAL-AL
 * \brief RFAL Abstraction Layer
 * @{
 *
 * \addtogroup ST25TB
 * \brief RFAL ST25TB Module
 * @{
 * 
 */


#ifndef RFAL_ST25TB_H
#define RFAL_ST25TB_H

/*
 ******************************************************************************
 * INCLUDES
 ******************************************************************************
 */
#include "platform.h"
#include "st_errno.h"
#include "rfal_rf.h"
#include "rfal_nfcb.h"

/*
 ******************************************************************************
 * GLOBAL DEFINES
 ******************************************************************************
 */

#define RFAL_ST25TB_CHIP_ID_LEN      1U       /*!< ST25TB chip ID length       */
#define RFAL_ST25TB_CRC_LEN          2U       /*!< ST25TB CRC length           */
#define RFAL_ST25TB_UID_LEN          8U       /*!< ST25TB Unique ID length     */
#define RFAL_ST25TB_BLOCK_LEN        4U       /*!< ST25TB Data Block length    */

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
typedef uint8_t rfalSt25tbUID[RFAL_ST25TB_UID_LEN];        /*!< ST25TB UID type          */
typedef uint8_t rfalSt25tbBlock[RFAL_ST25TB_BLOCK_LEN];    /*!< ST25TB Block type        */


/*! ST25TB listener device (PICC) struct  */
typedef struct
{
    uint8_t           chipID;                              /*!< Device's session Chip ID */
    rfalSt25tbUID     UID;                                 /*!< Device's UID             */
    bool              isDeselected;                        /*!< Device deselect flag     */
}rfalSt25tbListenDevice;


/*
******************************************************************************
* GLOBAL FUNCTION PROTOTYPES
******************************************************************************
*/

/*! 
 *****************************************************************************
 * \brief  Initialize ST25TB Poller mode
 *  
 * This methods configures RFAL RF layer to perform as a 
 * ST25TB Poller/RW including all default timings
 *
 * \return ERR_WRONG_STATE  : RFAL not initialized or mode not set
 * \return ERR_NONE         : No error
 *****************************************************************************
 */
ReturnCode rfalSt25tbPollerInitialize( void );


/*! 
 *****************************************************************************
 * \brief  ST25TB Poller Check Presence
 *  
 * This method checks if a ST25TB Listen device (PICC) is present on the field
 * by sending an Initiate command
 * 
 * \param[out] chipId : if successfully retrieved, the device's chip ID
 * 
 * \return ERR_WRONG_STATE  : RFAL not initialized or incorrect mode
 * \return ERR_PARAM        : Invalid parameters
 * \return ERR_IO           : Generic internal error
 * \return ERR_TIMEOUT      : Timeout error, no listener device detected
 * \return ERR_RF_COLLISION : Collision detected one or more device in the field
 * \return ERR_PROTO        : Protocol error detected
 * \return ERR_NONE         : No error
 *****************************************************************************
 */
ReturnCode rfalSt25tbPollerCheckPresence( uint8_t *chipId );


/*! 
 *****************************************************************************
 * \brief  ST25TB Poller Collision Resolution
 *  
 * This method performs ST25TB Collision resolution, selects the each device,
 * retrieves its UID and then deselects.
 * In case only one device is identified the ST25TB device is left in select
 * state.
 *   
 * \param[in]  devLimit      : device limit value, and size st25tbDevList
 * \param[out] st25tbDevList : ST35TB listener device info
 * \param[out] devCnt        : Devices found counter
 * 
 * \return ERR_WRONG_STATE  : RFAL not initialized or incorrect mode
 * \return ERR_PARAM        : Invalid parameters
 * \return ERR_IO           : Generic internal error
 * \return ERR_TIMEOUT      : Timeout error, no listener device detected
 * \return ERR_RF_COLLISION : Collision detected one or more device in the field
 * \return ERR_PROTO        : Protocol error detected
 * \return ERR_NONE         : No error
 *****************************************************************************
 */
ReturnCode rfalSt25tbPollerCollisionResolution( uint8_t devLimit, rfalSt25tbListenDevice *st25tbDevList, uint8_t *devCnt );

/*! 
 *****************************************************************************
 * \brief  ST25TB Poller Initiate
 *  
 * This method sends an Initiate command 
 * 
 * If a single device responds the chip ID will be retrieved
 *   
 * \param[out]  chipId      : chip ID of the device 
 * 
 * \return ERR_WRONG_STATE  : RFAL not initialized or incorrect mode
 * \return ERR_PARAM        : Invalid parameters
 * \return ERR_IO           : Generic internal error
 * \return ERR_TIMEOUT      : Timeout error, no listener device detected
 * \return ERR_PROTO        : Protocol error detected
 * \return ERR_NONE         : No error
 *****************************************************************************
 */
ReturnCode rfalSt25tbPollerInitiate( uint8_t *chipId );


/*! 
 *****************************************************************************
 * \brief  ST25TB Poller Pcall
 *  
 * This method sends a Pcall command 
 * If successful the device's chip ID will be retrieved
 *   
 * \param[out]  chipId      : Chip ID of the device 
 * 
 * \return ERR_WRONG_STATE  : RFAL not initialized or incorrect mode
 * \return ERR_PARAM        : Invalid parameters
 * \return ERR_IO           : Generic internal error
 * \return ERR_TIMEOUT      : Timeout error, no listener device detected
 * \return ERR_PROTO        : Protocol error detected
 * \return ERR_NONE         : No error
 *****************************************************************************
 */
ReturnCode rfalSt25tbPollerPcall( uint8_t *chipId );


/*! 
 *****************************************************************************
 * \brief  ST25TB Poller Slot Marker
 *  
 * This method sends a Slot Marker
 * 
 * If a single device responds the chip ID will be retrieved
 *
 * \param[in]  slotNum      : Slot Number    
 * \param[out]  chipIdRes   : Chip ID of the device 
 * 
 * \return ERR_WRONG_STATE  : RFAL not initialized or incorrect mode
 * \return ERR_PARAM        : Invalid parameters
 * \return ERR_IO           : Generic internal error
 * \return ERR_TIMEOUT      : Timeout error, no listener device detected
 * \return ERR_PROTO        : Protocol error detected
 * \return ERR_NONE         : No error
 *****************************************************************************
 */
ReturnCode rfalSt25tbPollerSlotMarker( uint8_t slotNum, uint8_t *chipIdRes );


/*! 
 *****************************************************************************
 * \brief  ST25TB Poller Select
 *  
 * This method sends a ST25TB Select command with the given chip ID.
 * 
 * If the device is already in Selected state and receives an incorrect chip 
 * ID, it goes into Deselected state
 *   
 * \param[in]  chipId       : chip ID of the device to be selected
 * 
 * \return ERR_WRONG_STATE  : RFAL not initialized or incorrect mode
 * \return ERR_PARAM        : Invalid parameters
 * \return ERR_IO           : Generic internal error
 * \return ERR_TIMEOUT      : Timeout error, no listener device detected
 * \return ERR_PROTO        : Protocol error detected
 * \return ERR_NONE         : No error
 *****************************************************************************
 */
ReturnCode rfalSt25tbPollerSelect( uint8_t chipId );


/*! 
 *****************************************************************************
 * \brief  ST25TB Get UID
 *  
 * This method sends a Get_UID command
 * 
 * If a single device responds the chip UID will be retrieved
 *
 * \param[out]  UID      : UID of the found device
 * 
 * \return ERR_WRONG_STATE  : RFAL not initialized or incorrect mode
 * \return ERR_PARAM        : Invalid parameters
 * \return ERR_IO           : Generic internal error
 * \return ERR_TIMEOUT      : Timeout error, no listener device detected
 * \return ERR_PROTO        : Protocol error detected
 * \return ERR_NONE         : No error
 *****************************************************************************
 */
ReturnCode rfalSt25tbPollerGetUID( rfalSt25tbUID *UID );


/*! 
 *****************************************************************************
 * \brief  ST25TB Poller Read Block
 *  
 * This method reads a block of the ST25TB
 * 
 * \param[in]   blockAddress : address of the block to be read
 * \param[out]  blockData    : location to place the data read from block
 * 
 * \return ERR_WRONG_STATE  : RFAL not initialized or incorrect mode
 * \return ERR_PARAM        : Invalid parameters
 * \return ERR_IO           : Generic internal error
 * \return ERR_TIMEOUT      : Timeout error, no listener device detected
 * \return ERR_PROTO        : Protocol error detected
 * \return ERR_NONE         : No error
 *****************************************************************************
 */
ReturnCode rfalSt25tbPollerReadBlock( uint8_t blockAddress, rfalSt25tbBlock *blockData  );


/*! 
 *****************************************************************************
 * \brief  ST25TB Poller Write Block
 *  
 * This method writes a block of the ST25TB
 * 
 * \param[in]  blockAddress : address of the block to be written
 * \param[in]  blockData    : data to be written on the block
 * 
 * \return ERR_WRONG_STATE  : RFAL not initialized or incorrect mode
 * \return ERR_PARAM        : Invalid parameters
 * \return ERR_IO           : Generic internal error
 * \return ERR_TIMEOUT      : Timeout error, no listener device detected
 * \return ERR_PROTO        : Protocol error detected
 * \return ERR_NONE         : No error
 *****************************************************************************
 */
ReturnCode rfalSt25tbPollerWriteBlock( uint8_t blockAddress, const rfalSt25tbBlock *blockData  );


/*! 
 *****************************************************************************
 * \brief  ST25TB Poller Completion 
 *  
 * This method sends a completion command to the ST25TB. After the 
 * completion the card no longer will reply to any command.
 * 
 * \return ERR_WRONG_STATE  : RFAL not initialized or incorrect mode
 * \return ERR_PARAM        : Invalid parameters
 * \return ERR_IO           : Generic internal error
 * \return ERR_TIMEOUT      : Timeout error, no listener device detected
 * \return ERR_PROTO        : Protocol error detected, invalid SENSB_RES received
 * \return ERR_NONE         : No error
 *****************************************************************************
 */
ReturnCode rfalSt25tbPollerCompletion( void );


/*! 
 *****************************************************************************
 * \brief  ST25TB Poller Reset to Inventory
 *  
 * This method sends a Reset to Inventory command to the ST25TB.
 * 
 * \return ERR_WRONG_STATE  : RFAL not initialized or incorrect mode
 * \return ERR_PARAM        : Invalid parameters
 * \return ERR_IO           : Generic internal error
 * \return ERR_TIMEOUT      : Timeout error, no listener device detected
 * \return ERR_PROTO        : Protocol error detected, invalid SENSB_RES received
 * \return ERR_NONE         : No error
 *****************************************************************************
 */
ReturnCode rfalSt25tbPollerResetToInventory( void );


#endif /* RFAL_ST25TB_H */

/**
  * @}
  *
  * @}
  *
  * @}
  */

