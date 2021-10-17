
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

/*! \file rfal_t1t.h
 *
 *  \author Gustavo Patricio
 *
 *  \brief Provides NFC-A T1T convenience methods and definitions
 *  
 *  This module provides an interface to perform as a NFC-A Reader/Writer
 *  to handle a Type 1 Tag T1T (Topaz)
 *  
 *  
 * \addtogroup RFAL
 * @{
 *
 * \addtogroup RFAL-AL
 * \brief RFAL Abstraction Layer
 * @{
 *
 * \addtogroup T1T
 * \brief RFAL T1T Module
 * @{
 *  
 */


#ifndef RFAL_T1T_H
#define RFAL_T1T_H

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
#define RFAL_T1T_UID_LEN               4   /*!< T1T UID length of cascade level 1 only tag  */
#define RFAL_T1T_HR_LENGTH             2   /*!< T1T HR(Header ROM) length                   */

#define RFAL_T1T_HR0_NDEF_MASK      0xF0   /*!< T1T HR0 NDEF capability mask  T1T 1.2 2.2.2 */
#define RFAL_T1T_HR0_NDEF_SUPPORT   0x10   /*!< T1T HR0 NDEF capable value    T1T 1.2 2.2.2 */


/*! NFC-A T1T (Topaz) command set */
typedef enum
{
    RFAL_T1T_CMD_RID      = 0x78,          /*!< T1T Read UID                                */
    RFAL_T1T_CMD_RALL     = 0x00,          /*!< T1T Read All                                */
    RFAL_T1T_CMD_READ     = 0x01,          /*!< T1T Read                                    */
    RFAL_T1T_CMD_WRITE_E  = 0x53,          /*!< T1T Write with erase (single byte)          */
    RFAL_T1T_CMD_WRITE_NE = 0x1A           /*!< T1T Write with no erase (single byte)       */
} rfalT1Tcmds;


/*
******************************************************************************
* GLOBAL TYPES
******************************************************************************
*/


/*! NFC-A T1T (Topaz) RID_RES  Digital 1.1  10.6.2 & Table 50 */
typedef struct
{
    uint8_t hr0;                           /*!< T1T Header ROM: HR0                         */
    uint8_t hr1;                           /*!< T1T Header ROM: HR1                         */
    uint8_t uid[RFAL_T1T_UID_LEN];         /*!< T1T UID                                     */
} rfalT1TRidRes;

/*
******************************************************************************
* GLOBAL FUNCTION PROTOTYPES
******************************************************************************
*/


/*! 
 *****************************************************************************
 * \brief  Initialize NFC-A T1T Poller mode
 *  
 * This methods configures RFAL RF layer to perform as a 
 * NFC-A T1T Poller/RW (Topaz) including all default timings 
 *
 * \return ERR_WRONG_STATE  : RFAL not initialized or mode not set
 * \return ERR_NONE         : No error
 *****************************************************************************
 */
ReturnCode rfalT1TPollerInitialize( void );


/*! 
 *****************************************************************************
 * \brief  NFC-A T1T Poller RID
 *  
 * This method reads the UID of a NFC-A T1T Listener device  
 *
 *
 * \param[out]  ridRes : pointer to place the RID_RES
 * 
 * \return ERR_WRONG_STATE  : RFAL not initialized or mode not set
 * \return ERR_PARAM        : Invalid parameter
 * \return ERR_NONE         : No error
 *****************************************************************************
 */
ReturnCode rfalT1TPollerRid( rfalT1TRidRes *ridRes );


/*! 
 *****************************************************************************
 * \brief  NFC-A T1T Poller RALL
 *  
 * This method send a Read All command to a NFC-A T1T Listener device  
 *
 *
 * \param[in]   uid       : the UID of the device to read data
 * \param[out]  rxBuf     : pointer to place the read data
 * \param[in]   rxBufLen  : size of rxBuf
 * \param[out]  rxRcvdLen : actual received data
 * 
 * \return ERR_WRONG_STATE  : RFAL not initialized or mode not set
 * \return ERR_PARAM        : Invalid parameter
 * \return ERR_NONE         : No error
 *****************************************************************************
 */
ReturnCode rfalT1TPollerRall( const uint8_t* uid, uint8_t* rxBuf, uint16_t rxBufLen, uint16_t *rxRcvdLen );


/*! 
 *****************************************************************************
 * \brief  NFC-A T1T Poller Write
 *  
 * This method writes the given data on the address of a NFC-A T1T Listener device  
 *
 *
 * \param[in]   uid       : the UID of the device to read data
 * \param[in]   address   : address to write the data
 * \param[in]   data      : the data to be written
 * 
 * \return ERR_WRONG_STATE  : RFAL not initialized or mode not set
 * \return ERR_PARAM        : Invalid parameter
 * \return ERR_NONE         : No error
 *****************************************************************************
 */
ReturnCode rfalT1TPollerWrite( const uint8_t* uid, uint8_t address, uint8_t data );

#endif /* RFAL_T1T_H */

/**
  * @}
  *
  * @}
  *
  * @}
  */
