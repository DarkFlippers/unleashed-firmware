
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

/*! \file rfal_t1t.c
 *
 *  \author Gustavo Patricio
 *
 *  \brief Provides NFC-A T1T convenience methods and definitions
 *  
 *  This module provides an interface to perform as a NFC-A Reader/Writer
 *  to handle a Type 1 Tag T1T (Topaz) 
 *  
 */

/*
 ******************************************************************************
 * INCLUDES
 ******************************************************************************
 */
#include "rfal_t1t.h"
#include "utils.h"

/*
 ******************************************************************************
 * ENABLE SWITCH
 ******************************************************************************
 */

#ifndef RFAL_FEATURE_T1T
    #define RFAL_FEATURE_T1T   false    /* T1T module configuration missing. Disabled by default */
#endif

#if RFAL_FEATURE_T1T

/*
 ******************************************************************************
 * GLOBAL DEFINES
 ******************************************************************************
 */

#define RFAL_T1T_DRD_READ           (1236U*2U) /*!< DRD for Reads with n=9         => 1236/fc  ~= 91 us   T1T 1.2  4.4.2 */
#define RFAL_T1T_DRD_WRITE          36052U     /*!< DRD for Write with n=281       => 36052/fc ~= 2659 us T1T 1.2  4.4.2 */
#define RFAL_T1T_DRD_WRITE_E        70996U     /*!< DRD for Write/Erase with n=554 => 70996/fc ~= 5236 us T1T 1.2  4.4.2 */

#define RFAL_T1T_RID_RES_HR0_VAL    0x10U      /*!< HR0 indicating NDEF support  Digital 2.0 (Candidate) 11.6.2.1        */
#define RFAL_T1T_RID_RES_HR0_MASK   0xF0U      /*!< HR0 most significant nibble mask                                     */

/*
******************************************************************************
* GLOBAL TYPES
******************************************************************************
*/

/*! NFC-A T1T (Topaz) RID_REQ  Digital 1.1  10.6.1 & Table 49 */
typedef struct
{
    uint8_t cmd;                               /*!< T1T cmd: RID              */
    uint8_t add;                               /*!< ADD: undefined value      */
    uint8_t data;                              /*!< DATA: undefined value     */
    uint8_t uid[RFAL_T1T_UID_LEN];             /*!< UID-echo: undefined value */
} rfalT1TRidReq;


/*! NFC-A T1T (Topaz) RALL_REQ   T1T 1.2  Table 4 */
typedef struct
{
    uint8_t cmd;                               /*!< T1T cmd: RALL             */
    uint8_t add1;                              /*!< ADD: 0x00                 */
    uint8_t add0;                              /*!< ADD: 0x00                 */
    uint8_t uid[RFAL_T1T_UID_LEN];             /*!< UID                       */
} rfalT1TRallReq;


/*! NFC-A T1T (Topaz) WRITE_REQ   T1T 1.2  Table 4 */
typedef struct
{
    uint8_t cmd;                               /*!< T1T cmd: RALL             */
    uint8_t add;                               /*!< ADD                       */
    uint8_t data;                              /*!< DAT                       */
    uint8_t uid[RFAL_T1T_UID_LEN];             /*!< UID                       */
} rfalT1TWriteReq;


/*! NFC-A T1T (Topaz) WRITE_RES   T1T 1.2  Table 4 */
typedef struct
{
    uint8_t add;                               /*!< ADD                       */
    uint8_t data;                              /*!< DAT                       */
} rfalT1TWriteRes;

/*
******************************************************************************
* LOCAL FUNCTION PROTOTYPES
******************************************************************************
*/

/*
******************************************************************************
* GLOBAL FUNCTIONS
******************************************************************************
*/

ReturnCode rfalT1TPollerInitialize( void )
{
    ReturnCode ret;
    
    EXIT_ON_ERR(ret, rfalSetMode( RFAL_MODE_POLL_NFCA_T1T, RFAL_BR_106, RFAL_BR_106 ) );
    rfalSetErrorHandling( RFAL_ERRORHANDLING_NFC );
    
    rfalSetGT( RFAL_GT_NONE );                          /* T1T should only be initialized after NFC-A mode, therefore the GT has been fulfilled */ 
    rfalSetFDTListen( RFAL_FDT_LISTEN_NFCA_POLLER );    /* T1T uses NFC-A FDT Listen with n=9   Digital 1.1  10.7.2                             */
    rfalSetFDTPoll( RFAL_FDT_POLL_NFCA_T1T_POLLER );
    
    return ERR_NONE;
}


/*******************************************************************************/
ReturnCode rfalT1TPollerRid( rfalT1TRidRes *ridRes )
{
    ReturnCode     ret;
    rfalT1TRidReq  ridReq;
    uint16_t       rcvdLen;
    
    if( ridRes == NULL )
    {
        return ERR_PARAM;
    }
    
    /* Compute RID command and set Undefined Values to 0x00    Digital 1.1 10.6.1 */
    ST_MEMSET( &ridReq, 0x00, sizeof(rfalT1TRidReq) );
    ridReq.cmd = (uint8_t)RFAL_T1T_CMD_RID;
    
    EXIT_ON_ERR( ret, rfalTransceiveBlockingTxRx( (uint8_t*)&ridReq, sizeof(rfalT1TRidReq), (uint8_t*)ridRes, sizeof(rfalT1TRidRes), &rcvdLen, RFAL_TXRX_FLAGS_DEFAULT, RFAL_T1T_DRD_READ ) );
    
    /* Check expected RID response length and the HR0   Digital 2.0 (Candidate) 11.6.2.1 */
    if( (rcvdLen != sizeof(rfalT1TRidRes)) || ((ridRes->hr0 & RFAL_T1T_RID_RES_HR0_MASK) != RFAL_T1T_RID_RES_HR0_VAL) )
    {
        return ERR_PROTO;
    }
    
    return ERR_NONE;
}


/*******************************************************************************/
ReturnCode rfalT1TPollerRall( const uint8_t* uid, uint8_t* rxBuf, uint16_t rxBufLen, uint16_t *rxRcvdLen )
{
    rfalT1TRallReq rallReq;
    
    if( (rxBuf == NULL) || (uid == NULL) || (rxRcvdLen == NULL) )
    {
        return ERR_PARAM;
    }
    
    /* Compute RALL command and set Add to 0x00 */
    ST_MEMSET( &rallReq, 0x00, sizeof(rfalT1TRallReq) );
    rallReq.cmd = (uint8_t)RFAL_T1T_CMD_RALL;
    ST_MEMCPY(rallReq.uid, uid, RFAL_T1T_UID_LEN);
    
    return rfalTransceiveBlockingTxRx( (uint8_t*)&rallReq, sizeof(rfalT1TRallReq), (uint8_t*)rxBuf, rxBufLen, rxRcvdLen, RFAL_TXRX_FLAGS_DEFAULT, RFAL_T1T_DRD_READ );
}


/*******************************************************************************/
ReturnCode rfalT1TPollerWrite( const uint8_t* uid, uint8_t address, uint8_t data )
{
    rfalT1TWriteReq writeReq;
    rfalT1TWriteRes writeRes;
    uint16_t        rxRcvdLen;
    ReturnCode      err;
    
    if( uid == NULL )
    {
        return ERR_PARAM;
    }
    
    writeReq.cmd  = (uint8_t)RFAL_T1T_CMD_WRITE_E;
    writeReq.add  = address;
    writeReq.data = data;
    ST_MEMCPY(writeReq.uid, uid, RFAL_T1T_UID_LEN);
    
    err = rfalTransceiveBlockingTxRx( (uint8_t*)&writeReq, sizeof(rfalT1TWriteReq), (uint8_t*)&writeRes, sizeof(rfalT1TWriteRes), &rxRcvdLen, RFAL_TXRX_FLAGS_DEFAULT, RFAL_T1T_DRD_WRITE_E );
    
    if( err == ERR_NONE )
    {
        if( (writeReq.add != writeRes.add) || (writeReq.data != writeRes.data) || (rxRcvdLen != sizeof(rfalT1TWriteRes)) )
        {
            return ERR_PROTO;
        }
    }
    return err;
}

#endif /* RFAL_FEATURE_T1T */
