
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

/*! \file rfal_nfcv.c
 *
 *  \author Gustavo Patricio
 *
 *  \brief Implementation of NFC-V Poller (ISO15693) device
 *
 *  The definitions and helpers methods provided by this module are 
 *  aligned with NFC-V (ISO15693)
 *
 *  The definitions and helpers methods provided by this module 
 *  are aligned with NFC-V Digital 2.1
 *
 */

/*
 ******************************************************************************
 * INCLUDES
 ******************************************************************************
 */
#include "rfal_nfcv.h"
#include "utils.h"

/*
 ******************************************************************************
 * ENABLE SWITCH
 ******************************************************************************
 */

#ifndef RFAL_FEATURE_NFCV
    #define RFAL_FEATURE_NFCV   false    /* NFC-V module configuration missing. Disabled by default */
#endif

#if RFAL_FEATURE_NFCV

/*
 ******************************************************************************
 * GLOBAL DEFINES
 ******************************************************************************
 */

#define RFAL_NFCV_INV_REQ_FLAG            0x06U  /*!< INVENTORY_REQ  INV_FLAG  Digital  2.1  9.6.1                      */
#define RFAL_NFCV_MASKVAL_MAX_LEN         8U     /*!< Mask value max length: 64 bits  (UID length)                      */
#define RFAL_NFCV_MASKVAL_MAX_1SLOT_LEN   64U    /*!< Mask value max length in 1 Slot mode in bits  Digital 2.1 9.6.1.6 */
#define RFAL_NFCV_MASKVAL_MAX_16SLOT_LEN  60U    /*!< Mask value max length in 16 Slot mode in bits Digital 2.1 9.6.1.6 */
#define RFAL_NFCV_MAX_SLOTS               16U    /*!< NFC-V max number of Slots                                         */
#define RFAL_NFCV_INV_REQ_HEADER_LEN      3U     /*!< INVENTORY_REQ header length (INV_FLAG, CMD, MASK_LEN)             */
#define RFAL_NFCV_INV_RES_LEN             10U    /*!< INVENTORY_RES length                                              */
#define RFAL_NFCV_WR_MUL_REQ_HEADER_LEN   4U     /*!< Write Multiple header length (INV_FLAG, CMD, [UID], BNo, Bno)     */


#define RFAL_NFCV_CMD_LEN                 1U     /*!< Commandbyte length                                                */
#define RFAL_NFCV_FLAG_POS                0U     /*!< Flag byte position                                                */
#define RFAL_NFCV_FLAG_LEN                1U     /*!< Flag byte length                                                  */
#define RFAL_NFCV_DATASTART_POS           1U     /*!< Position of start of data                                         */
#define RFAL_NFCV_DSFI_LEN                1U     /*!< DSFID length                                                      */
#define RFAL_NFCV_SLPREQ_REQ_FLAG         0x22U  /*!< SLPV_REQ request flags Digital 2.0 (Candidate) 9.7.1.1            */
#define RFAL_NFCV_RES_FLAG_NOERROR        0x00U  /*!< RES_FLAG indicating no error (checked during activation)          */

#define RFAL_NFCV_MAX_COLL_SUPPORTED      16U    /*!< Maximum number of collisions supported by the Anticollision loop  */

#define RFAL_NFCV_FDT_MAX                 rfalConvMsTo1fc(20) /*!< Maximum Wait time FDTV,EOF and MAX2   Digital 2.1 B.5*/
#define RFAL_NFCV_FDT_MAX1                4394U  /*!< Read alike command FWT FDTV,LISTEN,MAX1  Digital 2.0 B.5          */


/*! Time from special frame to EOF 
 *                    ISO15693 2009 10.4.2                 : 20ms
 *                    NFC Forum defines Digital 2.0  9.7.4 : FDTV,EOF = [10 ; 20]ms 
 */
#define RFAL_NFCV_FDT_EOF                 20U



/*! Time between slots - ISO 15693 defines t3min depending on modulation depth and data rate.
 *  With only high-bitrate supported, AM modulation and a length of 12 bytes (96bits) for INV_RES we get:
 *                    - ISO t3min = 96/26 ms + 300us = 4 ms
 *                    - NFC Forum defines FDTV,INVENT_NORES = (4394 + 2048)/fc. Digital 2.0  B.5*/
#define RFAL_NFCV_FDT_V_INVENT_NORES      4U



/*
 ******************************************************************************
 * GLOBAL MACROS
 ******************************************************************************
 */
 
 /*! Checks if a valid INVENTORY_RES is valid    Digital 2.2  9.6.2.1 & 9.6.2.3  */
 #define rfalNfcvCheckInvRes( f, l )     (((l)==rfalConvBytesToBits(RFAL_NFCV_INV_RES_LEN + RFAL_NFCV_CRC_LEN)) && ((f)==RFAL_NFCV_RES_FLAG_NOERROR))



/*
******************************************************************************
* GLOBAL TYPES
******************************************************************************
*/

/*! NFC-V INVENTORY_REQ format   Digital 2.0 9.6.1 */
typedef struct
{
    uint8_t  INV_FLAG;                              /*!< Inventory Flags    */
    uint8_t  CMD;                                   /*!< Command code: 01h  */
    uint8_t  MASK_LEN;                              /*!< Mask Value Length  */
    uint8_t  MASK_VALUE[RFAL_NFCV_MASKVAL_MAX_LEN]; /*!< Mask Value         */
} rfalNfcvInventoryReq;


/*! NFC-V SLP_REQ format   Digital 2.0 (Candidate) 9.7.1 */
typedef struct
{
    uint8_t  REQ_FLAG;                              /*!< Request Flags      */
    uint8_t  CMD;                                   /*!< Command code: 02h  */
    uint8_t  UID[RFAL_NFCV_UID_LEN];                /*!< Mask Value         */
} rfalNfcvSlpvReq;


/*! Container for a collision found during Anticollision loop */
typedef struct
{
    uint8_t  maskLen;
    uint8_t  maskVal[RFAL_NFCV_MASKVAL_MAX_LEN];
}rfalNfcvCollision;


/*
******************************************************************************
* LOCAL FUNCTION PROTOTYPES
******************************************************************************
*/
static ReturnCode rfalNfcvParseError( uint8_t err );

/*
******************************************************************************
* LOCAL VARIABLES
******************************************************************************
*/

/*
******************************************************************************
* LOCAL FUNCTIONS
******************************************************************************
*/

/*******************************************************************************/
static ReturnCode rfalNfcvParseError( uint8_t err )
{
    switch(err)
    {
        case RFAL_NFCV_ERROR_CMD_NOT_SUPPORTED:
        case RFAL_NFCV_ERROR_OPTION_NOT_SUPPORTED:
            return ERR_NOTSUPP;
            
        case RFAL_NFCV_ERROR_CMD_NOT_RECOGNIZED:
            return ERR_PROTO;
            
        case RFAL_NFCV_ERROR_WRITE_FAILED:
            return ERR_WRITE;
            
        default:
            return ERR_REQUEST;
    }
}

/*
******************************************************************************
* GLOBAL FUNCTIONS
******************************************************************************
*/

/*******************************************************************************/
ReturnCode rfalNfcvPollerInitialize( void )
{
    ReturnCode ret;
            
    EXIT_ON_ERR( ret, rfalSetMode( RFAL_MODE_POLL_NFCV, RFAL_BR_26p48, RFAL_BR_26p48 ) );
    rfalSetErrorHandling( RFAL_ERRORHANDLING_NFC );
    
    rfalSetGT( RFAL_GT_NFCV );
    rfalSetFDTListen( RFAL_FDT_LISTEN_NFCV_POLLER );
    rfalSetFDTPoll( RFAL_FDT_POLL_NFCV_POLLER );
    
    return ERR_NONE;
}

/*******************************************************************************/
ReturnCode rfalNfcvPollerCheckPresence( rfalNfcvInventoryRes *invRes )
{
    ReturnCode ret;
    
    /* INVENTORY_REQ with 1 slot and no Mask   Activity 2.0 (Candidate) 9.2.3.32 */
    ret = rfalNfcvPollerInventory( RFAL_NFCV_NUM_SLOTS_1, 0, NULL, invRes, NULL );
    
    if( (ret == ERR_RF_COLLISION) || (ret == ERR_CRC)  || 
        (ret == ERR_FRAMING)      || (ret == ERR_PROTO)  )
    {
        ret = ERR_NONE;
    }
    
    return ret;
}

/*******************************************************************************/
ReturnCode rfalNfcvPollerInventory( rfalNfcvNumSlots nSlots, uint8_t maskLen, const uint8_t *maskVal, rfalNfcvInventoryRes *invRes, uint16_t* rcvdLen )
{
    ReturnCode           ret;
    rfalNfcvInventoryReq invReq;
    uint16_t             rxLen;
    
    if( ((maskVal == NULL) && (maskLen != 0U)) || (invRes == NULL) )
    {
        return ERR_PARAM;
    }
    
    invReq.INV_FLAG = (RFAL_NFCV_INV_REQ_FLAG | (uint8_t)nSlots);
    invReq.CMD      = RFAL_NFCV_CMD_INVENTORY;
    invReq.MASK_LEN = (uint8_t)MIN( maskLen, ((nSlots == RFAL_NFCV_NUM_SLOTS_1) ? RFAL_NFCV_MASKVAL_MAX_1SLOT_LEN : RFAL_NFCV_MASKVAL_MAX_16SLOT_LEN) );   /* Digital 2.0  9.6.1.6 */
    
    if( (rfalConvBitsToBytes(invReq.MASK_LEN) > 0U) && (maskVal != NULL) )  /* MISRA 21.18 & 1.3 */
    {
        ST_MEMCPY( invReq.MASK_VALUE, maskVal, rfalConvBitsToBytes(invReq.MASK_LEN) );
    }
    
    ret = rfalISO15693TransceiveAnticollisionFrame( (uint8_t*)&invReq, (uint8_t)(RFAL_NFCV_INV_REQ_HEADER_LEN + rfalConvBitsToBytes(invReq.MASK_LEN)), (uint8_t*)invRes, sizeof(rfalNfcvInventoryRes), &rxLen );
    
    /* Check for optional output parameter */
    if( rcvdLen != NULL )
    {
        *rcvdLen = rxLen;
    }
    
    if( ret == ERR_NONE )
    {
        /* Check for valid INVENTORY_RES   Digital 2.2  9.6.2.1 & 9.6.2.3 */
        if( !rfalNfcvCheckInvRes( invRes->RES_FLAG, rxLen ) )
        {
            return ERR_PROTO;
        }
    }
    
    return ret;
}

/*******************************************************************************/
ReturnCode rfalNfcvPollerCollisionResolution( rfalComplianceMode compMode, uint8_t devLimit, rfalNfcvListenDevice *nfcvDevList, uint8_t *devCnt )
{
    ReturnCode        ret;
    uint8_t           slotNum;
    uint16_t          rcvdLen;
    uint8_t           colIt;
    uint8_t           colCnt;
    uint8_t           colPos;
    bool              colPending;
    rfalNfcvCollision colFound[RFAL_NFCV_MAX_COLL_SUPPORTED];
    
    
    if( (nfcvDevList == NULL) || (devCnt == NULL) )
    {
        return ERR_PARAM;
    }

    /* Initialize parameters */
    *devCnt = 0;
    colIt         = 0;
    colCnt        = 0;
    colPending    = false;
    ST_MEMSET(colFound, 0x00, (sizeof(rfalNfcvCollision)*RFAL_NFCV_MAX_COLL_SUPPORTED) );

    if( devLimit > 0U )       /* MISRA 21.18 */
    {
        ST_MEMSET(nfcvDevList, 0x00, (sizeof(rfalNfcvListenDevice)*devLimit) );
    }

    NO_WARNING(colPending);   /* colPending is not exposed externally, in future it might become exposed/ouput parameter */

    if( compMode == RFAL_COMPLIANCE_MODE_NFC )
    {
        /* Send INVENTORY_REQ with one slot   Activity 2.1  9.3.7.1  (Symbol 0)  */
        ret = rfalNfcvPollerInventory( RFAL_NFCV_NUM_SLOTS_1, 0, NULL, &nfcvDevList->InvRes, NULL );

        if( ret == ERR_TIMEOUT )  /* Exit if no device found     Activity 2.1  9.3.7.2 (Symbol 1)  */
        {
            return ERR_NONE;
        }
        if( ret == ERR_NONE )     /* Device found without transmission error/collision    Activity 2.1  9.3.7.3 (Symbol 2)  */
        {
            (*devCnt)++;
            return ERR_NONE;
        }

        /* A Collision has been identified  Activity 2.1  9.3.7.4  (Symbol 3) */
        colPending = true;
        colCnt        = 1;

        /* Check if the Collision Resolution is set to perform only Collision detection   Activity 2.1  9.3.7.5 (Symbol 4)*/
        if( devLimit == 0U )
        {
            return ERR_RF_COLLISION;
        }

        platformDelay(RFAL_NFCV_FDT_V_INVENT_NORES);

        /*******************************************************************************/
        /* Collisions pending, Anticollision loop must be executed                     */
        /*******************************************************************************/
    }
    else
    { 
        /* Advance to 16 slots below without mask. Will give a good chance to identify multiple cards */
        colPending = true;
        colCnt        = 1;
    }
    
    
    /* Execute until all collisions are resolved Activity 2.1 9.3.7.18  (Symbol 17) */
    do
    {
        /* Activity 2.1  9.3.7.7  (Symbol 6 / 7) */
        colPending = false;
        slotNum    = 0;
        
        do
        {
            if( slotNum == 0U )
            {
                /* Send INVENTORY_REQ with 16 slots   Activity 2.1  9.3.7.9  (Symbol 8) */
                ret = rfalNfcvPollerInventory( RFAL_NFCV_NUM_SLOTS_16, colFound[colIt].maskLen, colFound[colIt].maskVal, &nfcvDevList[(*devCnt)].InvRes, &rcvdLen );
            }
            else
            {
                ret = rfalISO15693TransceiveEOFAnticollision( (uint8_t*)&nfcvDevList[(*devCnt)].InvRes, sizeof(rfalNfcvInventoryRes), &rcvdLen );
            }
            slotNum++;
            
            /*******************************************************************************/
            if( ret != ERR_TIMEOUT )
            {
                if( rcvdLen < rfalConvBytesToBits(RFAL_NFCV_INV_RES_LEN + RFAL_NFCV_CRC_LEN) )
                { /* If only a partial frame was received make sure the FDT_V_INVENT_NORES is fulfilled */
                    platformDelay(RFAL_NFCV_FDT_V_INVENT_NORES);
                }
                
                /* Check if response is a correct frame (no TxRx error)  Activity 2.1  9.3.7.11  (Symbol 10)*/
                if( (ret == ERR_NONE) || (ret == ERR_PROTO) )
                {
                    /* Check if the device found is already on the list and its response is a valid INVENTORY_RES */
                    if( rfalNfcvCheckInvRes( nfcvDevList[(*devCnt)].InvRes.RES_FLAG, rcvdLen ) )
                    {
                        /* Activity 2.1  9.3.7.12  (Symbol 11) */
                        (*devCnt)++;
                    }
                }
                else /* Treat everything else as collision */
                {
                    /* Activity 2.1  9.3.7.17  (Symbol 16) */
                    colPending = true;
                    

                    /*******************************************************************************/
                    /* Ensure that this collision still fits on the container */
                    if( colCnt < RFAL_NFCV_MAX_COLL_SUPPORTED )
                    {
                        /* Store this collision on the container to be resolved later */
                        /* Activity 2.1  9.3.7.17  (Symbol 16): add the collision information
                         * (MASK_VAL + SN) to the list containing the collision information */
                        ST_MEMCPY(colFound[colCnt].maskVal, colFound[colIt].maskVal, RFAL_NFCV_UID_LEN);
                        colPos = colFound[colIt].maskLen;
                        colFound[colCnt].maskVal[(colPos/RFAL_BITS_IN_BYTE)]      &= (uint8_t)((1U << (colPos % RFAL_BITS_IN_BYTE)) - 1U);
                        colFound[colCnt].maskVal[(colPos/RFAL_BITS_IN_BYTE)]      |= (uint8_t)((slotNum-1U) << (colPos % RFAL_BITS_IN_BYTE));
                        colFound[colCnt].maskVal[((colPos/RFAL_BITS_IN_BYTE)+1U)]  = (uint8_t)((slotNum-1U) >> (RFAL_BITS_IN_BYTE - (colPos % RFAL_BITS_IN_BYTE)));

                        colFound[colCnt].maskLen = (colFound[colIt].maskLen + 4U);

                        colCnt++;
                    }
                }
            }
            else 
            { 
                /* Timeout */
                platformDelay(RFAL_NFCV_FDT_V_INVENT_NORES);
            }
            
            /* Check if devices found have reached device limit   Activity 2.1  9.3.7.13  (Symbol 12) */
            if( *devCnt >= devLimit )
            {
                return ERR_NONE;
            }
            
        } while( slotNum < RFAL_NFCV_MAX_SLOTS );  /* Slot loop             */
        colIt++;
    } while( colIt < colCnt );                     /* Collisions found loop */
    
    return ERR_NONE;
}

/*******************************************************************************/
ReturnCode rfalNfcvPollerSleepCollisionResolution( uint8_t devLimit, rfalNfcvListenDevice *nfcvDevList, uint8_t *devCnt )
{
    uint8_t    tmpDevCnt;
    ReturnCode ret;
    uint8_t    i;

    if( (nfcvDevList == NULL) || (devCnt == NULL) )
    {
        return ERR_PARAM;
    }

    *devCnt = 0;

    do
    {
        tmpDevCnt = 0;
        ret = rfalNfcvPollerCollisionResolution( RFAL_COMPLIANCE_MODE_ISO, (devLimit - *devCnt), &nfcvDevList[*devCnt], &tmpDevCnt );

        for( i = *devCnt; i < (*devCnt + tmpDevCnt); i++ )
        {
            rfalNfcvPollerSleep( 0x00, nfcvDevList[i].InvRes.UID );
            nfcvDevList[i].isSleep = true;
        }
        *devCnt += tmpDevCnt;
    }
    while( (ret == ERR_NONE) && (tmpDevCnt > 0U) && (*devCnt < devLimit) );

    return ret;
}

/*******************************************************************************/
ReturnCode rfalNfcvPollerSleep( uint8_t flags, const uint8_t* uid )
{
    ReturnCode      ret;
    rfalNfcvSlpvReq slpReq;
    uint8_t         rxBuf;    /* dummy buffer, just to perform Rx */
    
    if( uid == NULL )
    {
        return ERR_PARAM;
    }
    
    /* Compute SLPV_REQ */
    slpReq.REQ_FLAG = (flags | (uint8_t)RFAL_NFCV_REQ_FLAG_ADDRESS);   /* Should be with UID according Digital 2.0 (Candidate) 9.7.1.1 */
    slpReq.CMD      = RFAL_NFCV_CMD_SLPV;
    ST_MEMCPY( slpReq.UID, uid, RFAL_NFCV_UID_LEN );
    
    /* NFC Forum device SHALL wait at least FDTVpp to consider the SLPV acknowledged (FDTVpp = FDTVpoll)  Digital 2.0 (Candidate)  9.7  9.8.2  */
    ret = rfalTransceiveBlockingTxRx( (uint8_t*)&slpReq, sizeof(rfalNfcvSlpvReq), &rxBuf, sizeof(rxBuf), NULL, RFAL_TXRX_FLAGS_DEFAULT, RFAL_NFCV_FDT_MAX1 );
    if( ret != ERR_TIMEOUT )
    {
        return ret;
    }
    
    return ERR_NONE;
}

/*******************************************************************************/
ReturnCode rfalNfcvPollerSelect( uint8_t flags, const uint8_t* uid )
{
    uint16_t           rcvLen;
    rfalNfcvGenericRes res;
    
    if( uid == NULL )
    {
        return ERR_PARAM;
    }
    
    return rfalNfcvPollerTransceiveReq( RFAL_NFCV_CMD_SELECT, flags, RFAL_NFCV_PARAM_SKIP, uid, NULL, 0U, (uint8_t*)&res, sizeof(rfalNfcvGenericRes), &rcvLen );
}

/*******************************************************************************/
ReturnCode rfalNfcvPollerReadSingleBlock( uint8_t flags, const uint8_t* uid, uint8_t blockNum, uint8_t* rxBuf, uint16_t rxBufLen, uint16_t *rcvLen )
{
    uint8_t bn;

    bn = blockNum;

    return rfalNfcvPollerTransceiveReq( RFAL_NFCV_CMD_READ_SINGLE_BLOCK, flags, RFAL_NFCV_PARAM_SKIP, uid, &bn, sizeof(uint8_t), rxBuf, rxBufLen, rcvLen );
}

/*******************************************************************************/
ReturnCode rfalNfcvPollerWriteSingleBlock( uint8_t flags, const uint8_t* uid, uint8_t blockNum, const uint8_t* wrData, uint8_t blockLen )
{
    uint8_t            data[(RFAL_NFCV_BLOCKNUM_LEN + RFAL_NFCV_MAX_BLOCK_LEN)];
    uint8_t            dataLen;
    uint16_t           rcvLen;
    rfalNfcvGenericRes res;
    
    /* Check for valid parameters */
    if( (blockLen == 0U) || (blockLen > (uint8_t)RFAL_NFCV_MAX_BLOCK_LEN) || (wrData == NULL) )
    {
        return ERR_PARAM;
    }
    
    dataLen = 0U;
    
    /* Compute Request Data */
    data[dataLen++] = blockNum;                    /* Set Block Number (8 bits)  */
    ST_MEMCPY( &data[dataLen], wrData, blockLen ); /* Append Block data to write */
    dataLen += blockLen;
    
    return rfalNfcvPollerTransceiveReq( RFAL_NFCV_CMD_WRITE_SINGLE_BLOCK, flags, RFAL_NFCV_PARAM_SKIP, uid, data, dataLen, (uint8_t*)&res, sizeof(rfalNfcvGenericRes), &rcvLen );
}

/*******************************************************************************/
ReturnCode rfalNfcvPollerLockBlock( uint8_t flags, const uint8_t* uid, uint8_t blockNum )
{
    uint16_t           rcvLen;
    rfalNfcvGenericRes res;
    uint8_t            bn;
    
    bn = blockNum;
    
    return rfalNfcvPollerTransceiveReq( RFAL_NFCV_CMD_LOCK_BLOCK, flags, RFAL_NFCV_PARAM_SKIP, uid, &bn, sizeof(uint8_t), (uint8_t*)&res, sizeof(rfalNfcvGenericRes), &rcvLen );
}

/*******************************************************************************/
ReturnCode rfalNfcvPollerReadMultipleBlocks( uint8_t flags, const uint8_t* uid, uint8_t firstBlockNum, uint8_t numOfBlocks, uint8_t* rxBuf, uint16_t rxBufLen, uint16_t *rcvLen )
{
    uint8_t            data[(RFAL_NFCV_BLOCKNUM_LEN + RFAL_NFCV_BLOCKNUM_LEN)];
    uint8_t            dataLen;
    
    dataLen = 0U;
    
    /* Compute Request Data */
    data[dataLen++] = firstBlockNum;                    /* Set first Block Number       */
    data[dataLen++] = numOfBlocks;                      /* Set number of blocks to read */
    
    return rfalNfcvPollerTransceiveReq( RFAL_NFCV_CMD_READ_MULTIPLE_BLOCKS, flags, RFAL_NFCV_PARAM_SKIP, uid, data, dataLen, rxBuf, rxBufLen, rcvLen );
}

/*******************************************************************************/
ReturnCode rfalNfcvPollerWriteMultipleBlocks( uint8_t flags, const uint8_t* uid, uint8_t firstBlockNum, uint8_t numOfBlocks, uint8_t *txBuf, uint16_t txBufLen, uint8_t blockLen, const uint8_t* wrData, uint16_t wrDataLen )
{
    ReturnCode         ret;
    uint16_t           rcvLen;
    uint16_t           reqLen;
    rfalNfcvGenericRes res;
    uint16_t           msgIt;

    /* Calculate required buffer length */
    reqLen = (uint16_t)((uid != NULL) ? (RFAL_NFCV_WR_MUL_REQ_HEADER_LEN + RFAL_NFCV_UID_LEN + wrDataLen) : (RFAL_NFCV_WR_MUL_REQ_HEADER_LEN + wrDataLen));
  
    if( (reqLen > txBufLen) || (blockLen > (uint8_t)RFAL_NFCV_MAX_BLOCK_LEN) || ((((uint16_t)numOfBlocks) * (uint16_t)blockLen) != wrDataLen) || (numOfBlocks == 0U) || (wrData == NULL) )
    {
        return ERR_PARAM;
    }
    
    msgIt = 0;
    
    /* Compute Request Command */
    txBuf[msgIt++] = (uint8_t)(flags & (~((uint32_t)RFAL_NFCV_REQ_FLAG_ADDRESS)));
    txBuf[msgIt++] = RFAL_NFCV_CMD_WRITE_MULTIPLE_BLOCKS;
    
    /* Check if Request is to be sent in Addressed mode. Select mode flag shall be set by user */
    if( uid != NULL )
    {
        txBuf[RFAL_NFCV_FLAG_POS] |= (uint8_t)RFAL_NFCV_REQ_FLAG_ADDRESS;
        ST_MEMCPY( &txBuf[msgIt], uid, RFAL_NFCV_UID_LEN );
        msgIt += (uint8_t)RFAL_NFCV_UID_LEN;
    }
    
    txBuf[msgIt++] = firstBlockNum;
    txBuf[msgIt++] = (numOfBlocks - 1U);
    
    if( wrDataLen > 0U )         /* MISRA 21.18 */
    {
        ST_MEMCPY( &txBuf[msgIt], wrData, wrDataLen );
        msgIt += wrDataLen;
    }
    
    /* Transceive Command */
    ret = rfalTransceiveBlockingTxRx( txBuf, msgIt, (uint8_t*)&res, sizeof(rfalNfcvGenericRes), &rcvLen, RFAL_TXRX_FLAGS_DEFAULT, RFAL_NFCV_FDT_MAX );

    if( ret != ERR_NONE )
    {
        return ret;
    }
    
    /* Check if the response minimum length has been received */
    if( rcvLen < (uint8_t)RFAL_NFCV_FLAG_LEN )
    {
        return ERR_PROTO;
    }
    
    /* Check if an error has been signalled */
    if( (res.RES_FLAG & (uint8_t)RFAL_NFCV_RES_FLAG_ERROR) != 0U )
    {
        return rfalNfcvParseError( *res.data );
    }
    
    return ERR_NONE;
}

/*******************************************************************************/
ReturnCode rfalNfcvPollerExtendedReadSingleBlock( uint8_t flags, const uint8_t* uid, uint16_t blockNum, uint8_t* rxBuf, uint16_t rxBufLen, uint16_t *rcvLen )
{
    uint8_t data[RFAL_NFCV_BLOCKNUM_EXTENDED_LEN];
    uint8_t dataLen;
        
    dataLen = 0U;
    
    /* Compute Request Data */
    data[dataLen++] = (uint8_t)blockNum; /* TS T5T 1.0 BNo is considered as a multi-byte field. TS T5T 1.0 5.1.1.13 multi-byte field follows [DIGITAL]. [DIGITAL] 9.3.1 A multiple byte field is transmitted LSB first. */
    data[dataLen++] = (uint8_t)((blockNum >> 8U) & 0xFFU);
    
    return rfalNfcvPollerTransceiveReq( RFAL_NFCV_CMD_EXTENDED_READ_SINGLE_BLOCK, flags, RFAL_NFCV_PARAM_SKIP, uid, data, dataLen, rxBuf, rxBufLen, rcvLen );
}


/*******************************************************************************/
ReturnCode rfalNfcvPollerExtendedWriteSingleBlock( uint8_t flags, const uint8_t* uid, uint16_t blockNum, const uint8_t* wrData, uint8_t blockLen )
{
    uint8_t            data[(RFAL_NFCV_BLOCKNUM_EXTENDED_LEN + RFAL_NFCV_MAX_BLOCK_LEN)];
    uint8_t            dataLen;
    uint16_t           rcvLen;
    rfalNfcvGenericRes res;
    
    /* Check for valid parameters */
    if( (blockLen == 0U) || (blockLen > (uint8_t)RFAL_NFCV_MAX_BLOCK_LEN) )
    {
        return ERR_PARAM;
    }
    
    dataLen = 0U;
    
    /* Compute Request Data */
    data[dataLen++] = (uint8_t)blockNum;                    /* TS T5T 1.0 BNo is considered as a multi-byte field. TS T5T 1.0 5.1.1.13 multi-byte field follows [DIGITAL]. [DIGITAL] 9.3.1 A multiple byte field is transmitted LSB first. */
    data[dataLen++] = (uint8_t)((blockNum >> 8U) & 0xFFU);
    ST_MEMCPY( &data[dataLen], wrData, blockLen );         /* Append Block data to write */
    dataLen += blockLen;
    
    return rfalNfcvPollerTransceiveReq( RFAL_NFCV_CMD_EXTENDED_WRITE_SINGLE_BLOCK, flags, RFAL_NFCV_PARAM_SKIP, uid, data, dataLen, (uint8_t*)&res, sizeof(rfalNfcvGenericRes), &rcvLen );
}

/*******************************************************************************/
ReturnCode rfalNfcvPollerExtendedLockSingleBlock( uint8_t flags, const uint8_t* uid, uint16_t blockNum )
{
    uint8_t            data[RFAL_NFCV_BLOCKNUM_EXTENDED_LEN];
    uint8_t            dataLen;
    uint16_t           rcvLen;
    rfalNfcvGenericRes res;
    
    dataLen = 0U;
    
    /* Compute Request Data */
    data[dataLen++] = (uint8_t)blockNum;                   /* TS T5T 1.0 BNo is considered as a multi-byte field. TS T5T 1.0 5.1.1.13 multi-byte field follows [DIGITAL]. [DIGITAL] 9.3.1 A multiple byte field is transmitted LSB first. */
    data[dataLen++] = (uint8_t)((blockNum >> 8U) & 0xFFU);

    return rfalNfcvPollerTransceiveReq( RFAL_NFCV_CMD_EXTENDED_LOCK_SINGLE_BLOCK, flags, RFAL_NFCV_PARAM_SKIP, uid, data, dataLen, (uint8_t*)&res, sizeof(rfalNfcvGenericRes), &rcvLen );
}

/*******************************************************************************/
ReturnCode rfalNfcvPollerExtendedReadMultipleBlocks( uint8_t flags, const uint8_t* uid, uint16_t firstBlockNum, uint16_t numOfBlocks, uint8_t* rxBuf, uint16_t rxBufLen, uint16_t *rcvLen )
{
    uint8_t data[(RFAL_NFCV_BLOCKNUM_EXTENDED_LEN + RFAL_NFCV_BLOCKNUM_EXTENDED_LEN)];
    uint8_t dataLen;
        
    dataLen = 0U;
    
    /* Compute Request Data */
    data[dataLen++] = (uint8_t)((firstBlockNum >> 0U) & 0xFFU);
    data[dataLen++] = (uint8_t)((firstBlockNum >> 8U) & 0xFFU);
    data[dataLen++] = (uint8_t)((numOfBlocks >> 0U) & 0xFFU);
    data[dataLen++] = (uint8_t)((numOfBlocks >> 8U) & 0xFFU);
    
    return rfalNfcvPollerTransceiveReq( RFAL_NFCV_CMD_EXTENDED_READ_MULTIPLE_BLOCK, flags, RFAL_NFCV_PARAM_SKIP, uid, data, dataLen, rxBuf, rxBufLen, rcvLen );
}

/*******************************************************************************/
ReturnCode rfalNfcvPollerExtendedWriteMultipleBlocks( uint8_t flags, const uint8_t* uid, uint16_t firstBlockNum, uint16_t numOfBlocks, uint8_t *txBuf, uint16_t txBufLen, uint8_t blockLen, const uint8_t* wrData, uint16_t wrDataLen )
{
    ReturnCode         ret;
    uint16_t           rcvLen;
    uint16_t           reqLen;
    rfalNfcvGenericRes res;
    uint16_t           msgIt;
    uint16_t           nBlocks;

    /* Calculate required buffer length */
    reqLen = ((uid != NULL) ? (RFAL_NFCV_WR_MUL_REQ_HEADER_LEN + RFAL_NFCV_UID_LEN + wrDataLen) : (RFAL_NFCV_WR_MUL_REQ_HEADER_LEN + wrDataLen) );
  
    if( (reqLen > txBufLen) || (blockLen > (uint8_t)RFAL_NFCV_MAX_BLOCK_LEN) || (( (uint16_t)numOfBlocks * (uint16_t)blockLen) != wrDataLen) || (numOfBlocks == 0U) )
    {
        return ERR_PARAM;
    }
    
    msgIt   = 0;
    nBlocks = (numOfBlocks - 1U);
    
    /* Compute Request Command */
    txBuf[msgIt++] = (uint8_t)(flags & (~((uint32_t)RFAL_NFCV_REQ_FLAG_ADDRESS)));
    txBuf[msgIt++] = RFAL_NFCV_CMD_EXTENDED_WRITE_MULTIPLE_BLOCK;
    
    /* Check if Request is to be sent in Addressed mode. Select mode flag shall be set by user */
    if( uid != NULL )
    {
        txBuf[RFAL_NFCV_FLAG_POS] |= (uint8_t)RFAL_NFCV_REQ_FLAG_ADDRESS;
        ST_MEMCPY( &txBuf[msgIt], uid, RFAL_NFCV_UID_LEN );
        msgIt += (uint8_t)RFAL_NFCV_UID_LEN;
    }

    txBuf[msgIt++] = (uint8_t)((firstBlockNum >> 0) & 0xFFU);
    txBuf[msgIt++] = (uint8_t)((firstBlockNum >> 8) & 0xFFU);
    txBuf[msgIt++] = (uint8_t)((nBlocks >> 0) & 0xFFU);
    txBuf[msgIt++] = (uint8_t)((nBlocks >> 8) & 0xFFU);
    
    if( wrDataLen > 0U )         /* MISRA 21.18 */
    {
        ST_MEMCPY( &txBuf[msgIt], wrData, wrDataLen );
        msgIt += wrDataLen;
    }
    
    /* Transceive Command */
    ret = rfalTransceiveBlockingTxRx( txBuf, msgIt, (uint8_t*)&res, sizeof(rfalNfcvGenericRes), &rcvLen, RFAL_TXRX_FLAGS_DEFAULT, RFAL_NFCV_FDT_MAX );

    if( ret != ERR_NONE )
    {
        return ret;
    }
    
    /* Check if the response minimum length has been received */
    if( rcvLen < (uint8_t)RFAL_NFCV_FLAG_LEN )
    {
        return ERR_PROTO;
    }
    
    /* Check if an error has been signalled */
    if( (res.RES_FLAG & (uint8_t)RFAL_NFCV_RES_FLAG_ERROR) != 0U )
    {
        return rfalNfcvParseError( *res.data );
    }
    
    return ERR_NONE;
}

/*******************************************************************************/
ReturnCode rfalNfcvPollerGetSystemInformation( uint8_t flags, const uint8_t* uid, uint8_t* rxBuf, uint16_t rxBufLen, uint16_t *rcvLen )
{
    return rfalNfcvPollerTransceiveReq( RFAL_NFCV_CMD_GET_SYS_INFO, flags, RFAL_NFCV_PARAM_SKIP, uid, NULL, 0U, rxBuf, rxBufLen, rcvLen );
}

/*******************************************************************************/
ReturnCode rfalNfcvPollerExtendedGetSystemInformation( uint8_t flags, const uint8_t* uid, uint8_t requestField, uint8_t* rxBuf, uint16_t rxBufLen, uint16_t *rcvLen )
{
    return rfalNfcvPollerTransceiveReq( RFAL_NFCV_CMD_EXTENDED_GET_SYS_INFO, flags, requestField, uid, NULL, 0U, rxBuf, rxBufLen, rcvLen ); 
}

/*******************************************************************************/
ReturnCode rfalNfcvPollerTransceiveReq( uint8_t cmd, uint8_t flags, uint8_t param, const uint8_t* uid, const uint8_t *data, uint16_t dataLen, uint8_t* rxBuf, uint16_t rxBufLen, uint16_t *rcvLen )
{
    ReturnCode         ret;
    rfalNfcvGenericReq req;
    uint8_t            msgIt;
    rfalBitRate        rxBR;
    bool               fastMode;
    
    msgIt    = 0;
    fastMode = false;
    
    /* Check for valid parameters */
    if( (rxBuf == NULL) || (rcvLen == NULL) || ((dataLen > 0U) && (data == NULL))                                  || 
        (dataLen > ((uid != NULL) ? RFAL_NFCV_MAX_GEN_DATA_LEN : (RFAL_NFCV_MAX_GEN_DATA_LEN - RFAL_NFCV_UID_LEN)))  )
    {
        return ERR_PARAM;
    }
    
    
    /* Check if the command is an ST's Fast command */
    if( (cmd == (uint8_t)RFAL_NFCV_CMD_FAST_READ_SINGLE_BLOCK)    || (cmd == (uint8_t)RFAL_NFCV_CMD_FAST_EXTENDED_READ_SINGLE_BLOCK)    || 
        (cmd == (uint8_t)RFAL_NFCV_CMD_FAST_READ_MULTIPLE_BLOCKS) || (cmd == (uint8_t)RFAL_NFCV_CMD_FAST_EXTENDED_READ_MULTIPLE_BLOCKS) ||
        (cmd == (uint8_t)RFAL_NFCV_CMD_FAST_WRITE_MESSAGE)        || (cmd == (uint8_t)RFAL_NFCV_CMD_FAST_READ_MESSAGE_LENGTH)           ||
        (cmd == (uint8_t)RFAL_NFCV_CMD_FAST_READ_MESSAGE)         || (cmd == (uint8_t)RFAL_NFCV_CMD_FAST_READ_DYN_CONFIGURATION)        ||               
        (cmd == (uint8_t)RFAL_NFCV_CMD_FAST_WRITE_DYN_CONFIGURATION) )
    {
        /* Store current Rx bit rate and move to fast mode */
        rfalGetBitRate( NULL, &rxBR );
        rfalSetBitRate( RFAL_BR_KEEP, RFAL_BR_52p97 );
        
        fastMode = true;
    }
    
    
    /* Compute Request Command */
    req.REQ_FLAG  = (uint8_t)(flags & (~((uint32_t)RFAL_NFCV_REQ_FLAG_ADDRESS)));
    req.CMD       = cmd;
    
    /* Prepend parameter on ceratin proprietary requests: IC Manuf, Parameters */
    if( param != RFAL_NFCV_PARAM_SKIP )
    {
        req.payload.data[msgIt++] = param;
    }
    
    /* Check if Request is to be sent in Addressed mode. Select mode flag shall be set by user */
    if( uid != NULL )
    {
        req.REQ_FLAG |= (uint8_t)RFAL_NFCV_REQ_FLAG_ADDRESS;
        ST_MEMCPY( &req.payload.data[msgIt], uid, RFAL_NFCV_UID_LEN );
        msgIt += RFAL_NFCV_UID_LEN;
    }
    
    if( dataLen > 0U )
    {
        ST_MEMCPY( &req.payload.data[msgIt], data, dataLen);
        msgIt += (uint8_t)dataLen;
    }
    
    /* Transceive Command */
    ret = rfalTransceiveBlockingTxRx( (uint8_t*)&req, (RFAL_NFCV_CMD_LEN + RFAL_NFCV_FLAG_LEN +(uint16_t)msgIt), rxBuf, rxBufLen, rcvLen, RFAL_TXRX_FLAGS_DEFAULT, RFAL_NFCV_FDT_MAX );
    
    /* If the Option Flag is set in certain commands an EOF needs to be sent after 20ms to retrieve the VICC response      ISO15693-3 2009  10.4.2 & 10.4.3 & 10.4.5 */
    if( ((flags & (uint8_t)RFAL_NFCV_REQ_FLAG_OPTION) != 0U) && ((cmd == (uint8_t)RFAL_NFCV_CMD_WRITE_SINGLE_BLOCK) || (cmd == (uint8_t)RFAL_NFCV_CMD_WRITE_MULTIPLE_BLOCKS)        ||
                                                        (cmd == (uint8_t)RFAL_NFCV_CMD_LOCK_BLOCK) || (cmd == (uint8_t)RFAL_NFCV_CMD_EXTENDED_WRITE_SINGLE_BLOCK)                   ||
                                                        (cmd == (uint8_t)RFAL_NFCV_CMD_EXTENDED_LOCK_SINGLE_BLOCK) || (cmd == (uint8_t)RFAL_NFCV_CMD_EXTENDED_WRITE_MULTIPLE_BLOCK))  )
    {
        ret = rfalISO15693TransceiveEOF( rxBuf, (uint8_t)rxBufLen, rcvLen );
    }

    /* Restore Rx BitRate */
    if( fastMode )
    {
        rfalSetBitRate( RFAL_BR_KEEP, rxBR );
    }
    
    if( ret != ERR_NONE )
    {
        return ret;
    }
    
    /* Check if the response minimum length has been received */
    if( (*rcvLen) < (uint8_t)RFAL_NFCV_FLAG_LEN )
    {
        return ERR_PROTO;
    }
    
    /* Check if an error has been signalled */
    if( (rxBuf[RFAL_NFCV_FLAG_POS] & (uint8_t)RFAL_NFCV_RES_FLAG_ERROR) != 0U )
    {
        return rfalNfcvParseError( rxBuf[RFAL_NFCV_DATASTART_POS] );
    }
    
    return ERR_NONE;
}

#endif /* RFAL_FEATURE_NFCV */
