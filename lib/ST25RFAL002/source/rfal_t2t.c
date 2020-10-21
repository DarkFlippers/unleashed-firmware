
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

/*! \file rfal_t2t.c
 *
 *  \author 
 *
 *  \brief Provides NFC-A T2T convenience methods and definitions
 *  
 *  This module provides an interface to perform as a NFC-A Reader/Writer
 *  to handle a Type 2 Tag T2T 
 *  
 */

/*
 ******************************************************************************
 * INCLUDES
 ******************************************************************************
 */
 #include "rfal_t2t.h"
 #include "utils.h"
 
 /*
 ******************************************************************************
 * ENABLE SWITCH
 ******************************************************************************
 */

#ifndef RFAL_FEATURE_T2T
    #define RFAL_FEATURE_T2T   false    /* T2T module configuration missing. Disabled by default */
#endif

#if RFAL_FEATURE_T2T

 /*
 ******************************************************************************
 * GLOBAL DEFINES
 ******************************************************************************
 */
 #define RFAL_FDT_POLL_READ_MAX                 rfalConvMsTo1fc(5U)  /*!< Maximum Wait time for Read command as defined in TS T2T 1.0 table 18   */
 #define RFAL_FDT_POLL_WRITE_MAX                rfalConvMsTo1fc(10U) /*!< Maximum Wait time for Write command as defined in TS T2T 1.0 table 18  */ 
 #define RFAL_FDT_POLL_SL_MAX                   rfalConvMsTo1fc(1U)  /*!< Maximum Wait time for Sector Select as defined in TS T2T 1.0 table 18  */ 
 #define RFAL_T2T_ACK_NACK_LEN                  1U                   /*!< Len of NACK in bytes (4 bits)                                          */
 #define RFAL_T2T_ACK                           0x0AU                /*!< ACK value                                                              */
 #define RFAL_T2T_ACK_MASK                      0x0FU                /*!< ACK value                                                              */
 
 
 #define RFAL_T2T_SECTOR_SELECT_P1_BYTE2        0xFFU                /*!< Sector Select Packet 1 byte 2                                          */
 #define RFAL_T2T_SECTOR_SELECT_P2_RFU_LEN      3U                   /*!< Sector Select RFU length                                               */
 
 
 
 /*
******************************************************************************
* GLOBAL TYPES
******************************************************************************
*/

/*! NFC-A T2T command set    T2T 1.0 5.1 */
typedef enum
{
    RFAL_T2T_CMD_READ           = 0x30,     /*!< T2T Read                                */
    RFAL_T2T_CMD_WRITE          = 0xA2,     /*!< T2T Write                               */
    RFAL_T2T_CMD_SECTOR_SELECT  = 0xC2      /*!< T2T Sector Select                       */
} rfalT2Tcmds;


 /*! NFC-A T2T READ     T2T 1.0 5.2 and table 11 */
typedef struct
{
    uint8_t code;                           /*!< Command code                            */
    uint8_t blNo;                           /*!< Block number                            */
} rfalT2TReadReq;


 /*! NFC-A T2T WRITE    T2T 1.0 5.3 and table 12 */
typedef struct
{
    uint8_t code;                           /*!< Command code                            */
    uint8_t blNo;                           /*!< Block number                            */
    uint8_t data[RFAL_T2T_WRITE_DATA_LEN];  /*!< Data                                    */
} rfalT2TWriteReq;


/*! NFC-A T2T SECTOR SELECT Packet 1   T2T 1.0 5.4 and table 13 */
typedef struct
{
    uint8_t code;                           /*!< Command code                            */
    uint8_t byte2;                          /*!< Sector Select Packet 1 byte 2           */
} rfalT2TSectorSelectP1Req;


/*! NFC-A T2T SECTOR SELECT Packet 2   T2T 1.0 5.4 and table 13 */
typedef struct
{
    uint8_t secNo;                                   /*!< Block number                   */
    uint8_t rfu[RFAL_T2T_SECTOR_SELECT_P2_RFU_LEN];  /*!< Sector Select Packet RFU       */
} rfalT2TSectorSelectP2Req;



/*
 ******************************************************************************
 * GLOBAL FUNCTIONS
 ******************************************************************************
 */

 ReturnCode rfalT2TPollerRead( uint8_t blockNum, uint8_t* rxBuf, uint16_t rxBufLen, uint16_t *rcvLen )
 {
    ReturnCode      ret;
    rfalT2TReadReq  req;
     
    if( (rxBuf == NULL) || (rcvLen == NULL) )
    {
        return ERR_PARAM;
    }
    
    req.code = (uint8_t)RFAL_T2T_CMD_READ;
    req.blNo = blockNum;
    
    /* Transceive Command */
    ret = rfalTransceiveBlockingTxRx( (uint8_t*)&req, sizeof(rfalT2TReadReq), rxBuf, rxBufLen, rcvLen, RFAL_TXRX_FLAGS_DEFAULT, RFAL_FDT_POLL_READ_MAX );
    
    /* T2T 1.0 5.2.1.7 The Reader/Writer SHALL treat a NACK in response to a READ Command as a Protocol Error */
    if( (ret == ERR_INCOMPLETE_BYTE) && (*rcvLen == RFAL_T2T_ACK_NACK_LEN) && ((*rxBuf & RFAL_T2T_ACK_MASK) != RFAL_T2T_ACK) )
    {
        return ERR_PROTO;
    }
    return ret;
 }
 
 
 /*******************************************************************************/
 ReturnCode rfalT2TPollerWrite( uint8_t blockNum, const uint8_t* wrData )
 {
    ReturnCode         ret;
    rfalT2TWriteReq    req;
    uint8_t            res;
    uint16_t           rxLen;
    
    req.code = (uint8_t)RFAL_T2T_CMD_WRITE;
    req.blNo = blockNum;
    ST_MEMCPY(req.data, wrData, RFAL_T2T_WRITE_DATA_LEN);
    
     
    /* Transceive WRITE Command */
    ret = rfalTransceiveBlockingTxRx( (uint8_t*)&req, sizeof(rfalT2TWriteReq), &res, sizeof(uint8_t), &rxLen, RFAL_TXRX_FLAGS_DEFAULT, RFAL_FDT_POLL_READ_MAX );
    
    /* Check for a valid ACK */
    if( (ret == ERR_INCOMPLETE_BYTE) || (ret == ERR_NONE) )
    {
        ret = ERR_PROTO;
        
        if( (rxLen == RFAL_T2T_ACK_NACK_LEN) && ((res & RFAL_T2T_ACK_MASK) == RFAL_T2T_ACK) )
        {
            ret = ERR_NONE;
        }
    }
    
    return ret;
 }

 
 /*******************************************************************************/
 ReturnCode rfalT2TPollerSectorSelect( uint8_t sectorNum )
 {
    rfalT2TSectorSelectP1Req p1Req;
    rfalT2TSectorSelectP2Req p2Req;
    ReturnCode               ret;
    uint8_t                  res;
    uint16_t                 rxLen;
    
    
    /* Compute SECTOR SELECT Packet 1  */
    p1Req.code  = (uint8_t)RFAL_T2T_CMD_SECTOR_SELECT;
    p1Req.byte2 = RFAL_T2T_SECTOR_SELECT_P1_BYTE2;
    
    /* Transceive SECTOR SELECT Packet 1 */
    ret = rfalTransceiveBlockingTxRx( (uint8_t*)&p1Req, sizeof(rfalT2TSectorSelectP1Req), &res, sizeof(uint8_t), &rxLen, RFAL_TXRX_FLAGS_DEFAULT, RFAL_FDT_POLL_SL_MAX );
    
    /* Check and report any transmission error */
    if( (ret != ERR_INCOMPLETE_BYTE) && (ret != ERR_NONE) )
    {
        return ret;
    }
    
    /* Ensure that an ACK was received */
    if( (ret != ERR_INCOMPLETE_BYTE) || (rxLen != RFAL_T2T_ACK_NACK_LEN) || ((res & RFAL_T2T_ACK_MASK) != RFAL_T2T_ACK) )
    {
        return ERR_PROTO;
    }
    
    
    /* Compute SECTOR SELECT Packet 2  */
    p2Req.secNo  = sectorNum;
    ST_MEMSET( &p2Req.rfu, 0x00, RFAL_T2T_SECTOR_SELECT_P2_RFU_LEN );
    
    
    /* Transceive SECTOR SELECT Packet 2 */
    ret = rfalTransceiveBlockingTxRx( (uint8_t*)&p2Req, sizeof(rfalT2TSectorSelectP2Req), &res, sizeof(uint8_t), &rxLen, RFAL_TXRX_FLAGS_DEFAULT, RFAL_FDT_POLL_SL_MAX );
    
    /* T2T 1.0 5.4.1.14 The Reader/Writer SHALL treat any response received before the end of PATT2T,SL,MAX as a Protocol Error */
    if( (ret == ERR_NONE) || (ret == ERR_INCOMPLETE_BYTE) )
    {
        return ERR_PROTO;
    }
    
    /* T2T 1.0 5.4.1.13 The Reader/Writer SHALL treat the transmission of the SECTOR SELECT Command Packet 2 as being successful when it receives no response until PATT2T,SL,MAX. */ 
    if( ret == ERR_TIMEOUT )
    {
        return ERR_NONE;
    }
    
    return ret;
 }

#endif /* RFAL_FEATURE_T2T */
