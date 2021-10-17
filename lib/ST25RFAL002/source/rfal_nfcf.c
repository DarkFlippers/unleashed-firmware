
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

/*! \file rfal_nfcf.c
 *
 *  \author Gustavo Patricio
 *
 *  \brief Implementation of NFC-F Poller (FeliCa PCD) device
 *
 *  The definitions and helpers methods provided by this module are 
 *  aligned with NFC-F (FeliCa - JIS X6319-4)
 *
 */

/*
 ******************************************************************************
 * INCLUDES
 ******************************************************************************
 */
#include "rfal_nfcf.h"
#include "utils.h"

/*
 ******************************************************************************
 * ENABLE SWITCH
 ******************************************************************************
 */

#ifndef RFAL_FEATURE_NFCF
    #define RFAL_FEATURE_NFCF   false    /* NFC-F module configuration missing. Disabled by default */
#endif

#if RFAL_FEATURE_NFCF

/*
 ******************************************************************************
 * GLOBAL DEFINES
 ******************************************************************************
 */
#define RFAL_NFCF_SENSF_REQ_LEN_MIN                5U     /*!< SENSF_RES minimum length                              */

#define RFAL_NFCF_READ_WO_ENCRYPTION_MIN_LEN       15U    /*!< Minimum length for a Check Command         T3T  5.4.1 */
#define RFAL_NFCF_WRITE_WO_ENCRYPTION_MIN_LEN      31U    /*!< Minimum length for an Update Command       T3T  5.5.1 */

#define RFAL_NFCF_CHECK_RES_MIN_LEN                11U    /*!< CHECK Response minimum length       T3T 1.0  Table 8  */
#define RFAL_NFCF_UPDATE_RES_MIN_LEN               11U    /*!< UPDATE Response minimum length      T3T 1.0  Table 8  */

#define RFAL_NFCF_CHECK_REQ_MAX_LEN                86U    /*!< Max length of a Check request        T3T 1.0  Table 7 */
#define RFAL_NFCF_CHECK_REQ_MAX_SERV               15U    /*!< Max Services number on Check request T3T 1.0  5.4.1.5 */
#define RFAL_NFCF_CHECK_REQ_MAX_BLOCK              15U    /*!< Max Blocks number on Check request  T3T 1.0  5.4.1.10 */
#define RFAL_NFCF_UPDATE_REQ_MAX_SERV              15U    /*!< Max Services number Update request  T3T 1.0  5.4.1.5  */
#define RFAL_NFCF_UPDATE_REQ_MAX_BLOCK             13U    /*!< Max Blocks number on Update request T3T 1.0  5.4.1.10 */


/*! MRT Check | Uupdate = (Tt3t x ((A+1) + n (B+1)) x 4^E) + dRWTt3t    T3T  5.8
    Max values used: A = 7 ; B = 7 ; E = 3 ; n = 15 (NFC Forum n = 15, JIS n = 32)
*/
#define RFAL_NFCF_MRT_CHECK_UPDATE   ((4096 * (8 + (15 * 8)) * 64 ) + 16)

/*
 ******************************************************************************
 * GLOBAL MACROS
 ******************************************************************************
 */
#define rfalNfcfSlots2CardNum( s )                 ((uint8_t)(s)+1U) /*!< Converts Time Slot Number (TSN) into num of slots  */

/*
******************************************************************************
* GLOBAL TYPES
******************************************************************************
*/

/*! Structure/Buffer to hold the SENSF_RES with LEN byte prepended                                 */
typedef struct{
    uint8_t           LEN;                                /*!< NFC-F LEN byte                      */
    rfalNfcfSensfRes  SENSF_RES;                          /*!< SENSF_RES                           */
} rfalNfcfSensfResBuf;


/*! Greedy collection for NFCF GRE_POLL_F  Activity 1.0 Table 10                                   */
typedef struct{
    uint8_t              pollFound;                       /*!< Number of devices found by the Poll */
    uint8_t              pollCollision;                   /*!< Number of collisions detected       */
    rfalFeliCaPollRes    POLL_F[RFAL_NFCF_POLL_MAXCARDS]; /*!< GRE_POLL_F   Activity 1.0 Table 10  */
} rfalNfcfGreedyF;


/*! NFC-F SENSF_REQ format  Digital 1.1  8.6.1                     */
typedef struct
{
    uint8_t  CMD;                          /*!< Command code: 00h  */
    uint8_t  SC[RFAL_NFCF_SENSF_SC_LEN];   /*!< System Code        */
    uint8_t  RC;                           /*!< Request Code       */
    uint8_t  TSN;                          /*!< Time Slot Number   */
} rfalNfcfSensfReq;


/*
******************************************************************************
* LOCAL VARIABLES
******************************************************************************
*/
static rfalNfcfGreedyF gRfalNfcfGreedyF;   /*!< Activity's NFCF Greedy collection */


/*
******************************************************************************
* LOCAL FUNCTION PROTOTYPES
******************************************************************************
*/
static void rfalNfcfComputeValidSENF( rfalNfcfListenDevice *outDevInfo, uint8_t *curDevIdx, uint8_t devLimit, bool overwrite, bool *nfcDepFound );


/*
******************************************************************************
* LOCAL VARIABLES
******************************************************************************
*/

/*******************************************************************************/
static void rfalNfcfComputeValidSENF( rfalNfcfListenDevice *outDevInfo, uint8_t *curDevIdx, uint8_t devLimit, bool overwrite, bool *nfcDepFound )
{
    uint8_t                   tmpIdx;
    bool                      duplicate;    
    const rfalNfcfSensfResBuf *sensfBuf;
    rfalNfcfSensfResBuf       sensfCopy;
    
    
    /*******************************************************************************/
    /* Go through all responses check if valid and duplicates                      */
    /*******************************************************************************/
    while( (gRfalNfcfGreedyF.pollFound > 0U) && ((*curDevIdx) < devLimit) )
    {
        duplicate = false;
        gRfalNfcfGreedyF.pollFound--;
        
        /* MISRA 11.3 - Cannot point directly into different object type, use local copy */
        ST_MEMCPY( (uint8_t*)&sensfCopy, (uint8_t*)&gRfalNfcfGreedyF.POLL_F[gRfalNfcfGreedyF.pollFound], sizeof(rfalNfcfSensfResBuf) );
        
        
        /* Point to received SENSF_RES */
        sensfBuf = &sensfCopy;
        
        
        /* Check for devices that are already in device list */
        for( tmpIdx = 0; tmpIdx < (*curDevIdx); tmpIdx++ )
        {
            if( ST_BYTECMP( sensfBuf->SENSF_RES.NFCID2, outDevInfo[tmpIdx].sensfRes.NFCID2, RFAL_NFCF_NFCID2_LEN ) == 0 )
            {
                duplicate = true;
                break;
            }
        }
        
        /* If is a duplicate skip this (and not to overwrite)*/        
        if(duplicate && !overwrite)
        {
            continue;
        }
        
        /* Check if response length is OK */
        if( (( sensfBuf->LEN - RFAL_NFCF_HEADER_LEN) < RFAL_NFCF_SENSF_RES_LEN_MIN) || ((sensfBuf->LEN - RFAL_NFCF_HEADER_LEN) > RFAL_NFCF_SENSF_RES_LEN_MAX) )
        {
            continue;
        }
        
        /* Check if the response is a SENSF_RES / Polling response */
        if( sensfBuf->SENSF_RES.CMD != (uint8_t)RFAL_NFCF_CMD_POLLING_RES )
        {
            continue;
        }
        
        /* Check if is an overwrite request or new device*/
        if(duplicate && overwrite)
        {
            /* overwrite deviceInfo/GRE_SENSF_RES with SENSF_RES */
            outDevInfo[tmpIdx].sensfResLen = (sensfBuf->LEN - RFAL_NFCF_LENGTH_LEN);
            ST_MEMCPY( &outDevInfo[tmpIdx].sensfRes, &sensfBuf->SENSF_RES, outDevInfo[tmpIdx].sensfResLen );
            continue;
        }
        else
        {
            /* fill deviceInfo/GRE_SENSF_RES with new SENSF_RES */
            outDevInfo[(*curDevIdx)].sensfResLen = (sensfBuf->LEN - RFAL_NFCF_LENGTH_LEN);
            ST_MEMCPY( &outDevInfo[(*curDevIdx)].sensfRes, &sensfBuf->SENSF_RES, outDevInfo[(*curDevIdx)].sensfResLen );            
        }
        
        /* Check if this device supports NFC-DEP and signal it (ACTIVITY 1.1   9.3.6.63) */        
        *nfcDepFound = rfalNfcfIsNfcDepSupported( &outDevInfo[(*curDevIdx)] );
                
        (*curDevIdx)++;
    }
}

/*
******************************************************************************
* GLOBAL FUNCTIONS
******************************************************************************
*/

/*******************************************************************************/
ReturnCode rfalNfcfPollerInitialize( rfalBitRate bitRate )
{
    ReturnCode ret;
    
    if( (bitRate != RFAL_BR_212) && (bitRate != RFAL_BR_424) )
    {
        return ERR_PARAM;
    }
    
    EXIT_ON_ERR( ret, rfalSetMode( RFAL_MODE_POLL_NFCF, bitRate, bitRate ) );
    rfalSetErrorHandling( RFAL_ERRORHANDLING_NFC );
    
    rfalSetGT( RFAL_GT_NFCF );
    rfalSetFDTListen( RFAL_FDT_LISTEN_NFCF_POLLER );
    rfalSetFDTPoll( RFAL_FDT_POLL_NFCF_POLLER );
    
    return ERR_NONE;
}



/*******************************************************************************/
ReturnCode rfalNfcfPollerPoll( rfalFeliCaPollSlots slots, uint16_t sysCode, uint8_t reqCode, rfalFeliCaPollRes *cardList, uint8_t *devCnt, uint8_t *collisions )
{
    return rfalFeliCaPoll( slots, sysCode, reqCode, cardList, rfalNfcfSlots2CardNum(slots), devCnt, collisions );
}

/*******************************************************************************/
ReturnCode rfalNfcfPollerCheckPresence( void )
{
    gRfalNfcfGreedyF.pollFound     = 0;
    gRfalNfcfGreedyF.pollCollision = 0;
        
    /* ACTIVITY 1.0 & 1.1 - 9.2.3.17 SENSF_REQ  must be with number of slots equal to 4
     *                                SC must be 0xFFFF
     *                                RC must be 0x00 (No system code info required) */
    return rfalFeliCaPoll( RFAL_FELICA_4_SLOTS, RFAL_NFCF_SYSTEMCODE, RFAL_FELICA_POLL_RC_NO_REQUEST, gRfalNfcfGreedyF.POLL_F, rfalNfcfSlots2CardNum(RFAL_FELICA_4_SLOTS), &gRfalNfcfGreedyF.pollFound, &gRfalNfcfGreedyF.pollCollision );
}


/*******************************************************************************/
ReturnCode rfalNfcfPollerCollisionResolution( rfalComplianceMode compMode, uint8_t devLimit, rfalNfcfListenDevice *nfcfDevList, uint8_t *devCnt )
{
    ReturnCode  ret;
    bool        nfcDepFound;
    
    if( (nfcfDevList == NULL) || (devCnt == NULL) )
    {
        return ERR_PARAM;
    }
            
    *devCnt      = 0;
    nfcDepFound  = false;
    
    
    /*******************************************************************************************/
    /* ACTIVITY 1.0 - 9.3.6.3 Copy valid SENSF_RES in GRE_POLL_F into GRE_SENSF_RES            */
    /* ACTIVITY 1.0 - 9.3.6.6 The NFC Forum Device MUST remove all entries from GRE_SENSF_RES[]*/
    /* ACTIVITY 1.1 - 9.3.63.59 Populate GRE_SENSF_RES with data from GRE_POLL_F               */
    /*                                                                                         */
    /* CON_DEVICES_LIMIT = 0 Just check if devices from Tech Detection exceeds -> always true  */
    /* Allow the number of slots open on Technology Detection                                  */
    /*******************************************************************************************/
    rfalNfcfComputeValidSENF( nfcfDevList, devCnt, ((devLimit == 0U) ? rfalNfcfSlots2CardNum( RFAL_FELICA_4_SLOTS ) : devLimit), false, &nfcDepFound );

    
    /*******************************************************************************/
    /* ACTIVITY 1.0 - 9.3.6.4                                                      */
    /* ACTIVITY 1.1 - 9.3.63.60 Check if devices found are lower than the limit    */
    /* and send a SENSF_REQ if so                                                  */
    /*******************************************************************************/
    if( *devCnt < devLimit )
    {
        /* ACTIVITY 1.0 - 9.3.6.5  Copy valid SENSF_RES and then to remove it
         * ACTIVITY 1.1 - 9.3.6.65 Copy and filter duplicates                                           
         * For now, due to some devices keep generating different nfcid2, we use 1.0  
         * Phones detected: Samsung Galaxy Nexus,Samsung Galaxy S3,Samsung Nexus S */
        *devCnt = 0;
        
        ret = rfalNfcfPollerPoll( RFAL_FELICA_16_SLOTS, RFAL_NFCF_SYSTEMCODE, RFAL_FELICA_POLL_RC_NO_REQUEST, gRfalNfcfGreedyF.POLL_F, &gRfalNfcfGreedyF.pollFound, &gRfalNfcfGreedyF.pollCollision );
        if( ret == ERR_NONE )
        {
            rfalNfcfComputeValidSENF( nfcfDevList, devCnt, devLimit, false, &nfcDepFound );
        }
      
      /*******************************************************************************/
      /* ACTIVITY 1.1 -  9.3.6.63 Check if any device supports NFC DEP               */
      /*******************************************************************************/
      if( nfcDepFound && (compMode == RFAL_COMPLIANCE_MODE_NFC) )
      {
          ret = rfalNfcfPollerPoll( RFAL_FELICA_16_SLOTS, RFAL_NFCF_SYSTEMCODE, RFAL_FELICA_POLL_RC_SYSTEM_CODE, gRfalNfcfGreedyF.POLL_F, &gRfalNfcfGreedyF.pollFound, &gRfalNfcfGreedyF.pollCollision );
          if( ret == ERR_NONE )
          {
              rfalNfcfComputeValidSENF( nfcfDevList, devCnt, devLimit, true, &nfcDepFound );
          }
      }
    }
    
    return ERR_NONE;
}

/*******************************************************************************/
ReturnCode rfalNfcfPollerCheck( const uint8_t* nfcid2, const rfalNfcfServBlockListParam *servBlock, uint8_t *rxBuf, uint16_t rxBufLen, uint16_t *rcvdLen )
{
    uint8_t       txBuf[RFAL_NFCF_CHECK_REQ_MAX_LEN];
    uint8_t       msgIt;
    uint8_t       i;
    ReturnCode    ret;
    const uint8_t *checkRes;
    
    /* Check parameters */
    if( (nfcid2 == NULL) || (rxBuf == NULL) || (servBlock == NULL)                           ||
        (servBlock->numBlock == 0U) || (servBlock->numBlock > RFAL_NFCF_CHECK_REQ_MAX_BLOCK) ||
        (servBlock->numServ == 0U) || (servBlock->numServ > RFAL_NFCF_CHECK_REQ_MAX_SERV)    ||
        (rxBufLen < (RFAL_NFCF_LENGTH_LEN + RFAL_NFCF_CHECK_RES_MIN_LEN))                      )
    {
        return ERR_PARAM;
    }
    
    msgIt = 0;
    
    /*******************************************************************************/
    /* Compose CHECK command/request                                               */
    
    txBuf[msgIt++] = RFAL_NFCF_CMD_READ_WITHOUT_ENCRYPTION;                               /* Command Code    */
    
    ST_MEMCPY( &txBuf[msgIt], nfcid2, RFAL_NFCF_NFCID2_LEN );                             /* NFCID2          */
    msgIt += RFAL_NFCF_NFCID2_LEN;
    
    txBuf[msgIt++] = servBlock->numServ;                                                  /* NoS             */
    for( i = 0; i < servBlock->numServ; i++)
    {
        txBuf[msgIt++] = (uint8_t)((servBlock->servList[i] >> 0U) & 0xFFU);               /* Service Code    */
        txBuf[msgIt++] = (uint8_t)((servBlock->servList[i] >> 8U) & 0xFFU);            
    }
    
    txBuf[msgIt++] = servBlock->numBlock;                                                 /* NoB             */
    for( i = 0; i < servBlock->numBlock; i++)
    {
        txBuf[msgIt++] = servBlock->blockList[i].conf;                                    /* Block list element conf (Flag|Access|Service) */
        if( (servBlock->blockList[i].conf & 0x80U) != 0U )                                /* Check if 2 or 3 byte block list element       */
        {
            txBuf[msgIt++] = (uint8_t)(servBlock->blockList[i].blockNum & 0xFFU);         /* 1byte Block Num */
        }
        else
        {
            txBuf[msgIt++] = (uint8_t)((servBlock->blockList[i].blockNum >> 0U) & 0xFFU); /* 2byte Block Num */
            txBuf[msgIt++] = (uint8_t)((servBlock->blockList[i].blockNum >> 8U) & 0xFFU);
        }
    }
    
    /*******************************************************************************/
    /* Transceive CHECK command/request                                            */
    ret = rfalTransceiveBlockingTxRx( txBuf, msgIt, rxBuf, rxBufLen, rcvdLen, RFAL_TXRX_FLAGS_DEFAULT, RFAL_NFCF_MRT_CHECK_UPDATE );
    
    if( ret == ERR_NONE )
    {
        /* Skip LEN byte */
        checkRes = (rxBuf + RFAL_NFCF_LENGTH_LEN);
        
        /* Check response length */
        if( *rcvdLen < (RFAL_NFCF_LENGTH_LEN + RFAL_NFCF_CHECKUPDATE_RES_ST2_POS) )
        {
            ret = ERR_PROTO;
        }
        /* Check for a valid response */
        else if( (checkRes[RFAL_NFCF_CMD_POS] != (uint8_t)RFAL_NFCF_CMD_READ_WITHOUT_ENCRYPTION_RES) ||
                 (checkRes[RFAL_NFCF_CHECKUPDATE_RES_ST1_POS] != RFAL_NFCF_STATUS_FLAG_SUCCESS)      || 
                 (checkRes[RFAL_NFCF_CHECKUPDATE_RES_ST2_POS] != RFAL_NFCF_STATUS_FLAG_SUCCESS)        )
        {
            ret = ERR_REQUEST;
        }
        /* CHECK succesfull, remove header */
        else
        {
            (*rcvdLen) -= (RFAL_NFCF_LENGTH_LEN + RFAL_NFCF_CHECKUPDATE_RES_NOB_POS);
            
            if( *rcvdLen > 0U )
            {
                ST_MEMMOVE( rxBuf, &checkRes[RFAL_NFCF_CHECKUPDATE_RES_NOB_POS], (*rcvdLen) );
            }
        }
    }
    
    return ret;
}


/*******************************************************************************/
ReturnCode rfalNfcfPollerUpdate( const uint8_t* nfcid2, const rfalNfcfServBlockListParam *servBlock,  uint8_t *txBuf, uint16_t txBufLen, const uint8_t *blockData, uint8_t *rxBuf, uint16_t rxBufLen )
{
    uint8_t       i;
    uint16_t      msgIt;
    uint16_t      rcvdLen;
    uint16_t      auxLen;
    const uint8_t *updateRes;
    ReturnCode    ret;

    /* Check parameters */
    if( (nfcid2 == NULL) || (rxBuf == NULL) || (servBlock == NULL) || (txBuf == NULL)         ||
        (servBlock->numBlock == 0U) || (servBlock->numBlock > RFAL_NFCF_UPDATE_REQ_MAX_BLOCK) ||
        (servBlock->numServ == 0U)   || (servBlock->numServ > RFAL_NFCF_UPDATE_REQ_MAX_SERV)  ||
        (rxBufLen < (RFAL_NFCF_LENGTH_LEN + RFAL_NFCF_UPDATE_RES_MIN_LEN))                      )
    {
        return ERR_PARAM;
    }
    
    /* Calculate required txBuffer lenth */
    auxLen = (uint16_t)( RFAL_NFCF_CMD_LEN + RFAL_NFCF_NFCID2_LEN + ( servBlock->numServ * sizeof(rfalNfcfServ) ) + 
              (servBlock->numBlock * sizeof(rfalNfcfBlockListElem)) + (uint16_t)((uint16_t)servBlock->numBlock * RFAL_NFCF_BLOCK_LEN) );
    
    /* Check whether the provided buffer is sufficient for this request */
    if( txBufLen < auxLen )
    {
        return ERR_PARAM;
    }
        
    msgIt = 0;
    
    /*******************************************************************************/
    /* Compose UPDATE command/request                                              */
    
    txBuf[msgIt++] = RFAL_NFCF_CMD_WRITE_WITHOUT_ENCRYPTION;                              /* Command Code    */
    
    ST_MEMCPY( &txBuf[msgIt], nfcid2, RFAL_NFCF_NFCID2_LEN );                             /* NFCID2          */
    msgIt += RFAL_NFCF_NFCID2_LEN;
    
    txBuf[msgIt++] = servBlock->numServ;                                                  /* NoS             */
    for( i = 0; i < servBlock->numServ; i++)
    {
        txBuf[msgIt++] = (uint8_t)((servBlock->servList[i] >> 0U) & 0xFFU);               /* Service Code    */
        txBuf[msgIt++] = (uint8_t)((servBlock->servList[i] >> 8U) & 0xFFU);            
    }
    
    txBuf[msgIt++] = servBlock->numBlock;                                                 /* NoB             */
    for( i = 0; i < servBlock->numBlock; i++)
    {
        txBuf[msgIt++] = servBlock->blockList[i].conf;                                    /* Block list element conf (Flag|Access|Service) */
        if( (servBlock->blockList[i].conf & 0x80U) != 0U )                                /* Check if 2 or 3 byte block list element       */
        {
            txBuf[msgIt++] = (uint8_t)(servBlock->blockList[i].blockNum & 0xFFU);         /* 1byte Block Num */
        }
        else
        {
            txBuf[msgIt++] = (uint8_t)((servBlock->blockList[i].blockNum >> 0U) & 0xFFU); /* 2byte Block Num */
            txBuf[msgIt++] = (uint8_t)((servBlock->blockList[i].blockNum >> 8U) & 0xFFU);
        }
    }
    
    auxLen = ((uint16_t)servBlock->numBlock * RFAL_NFCF_BLOCK_LEN);
    ST_MEMCPY( &txBuf[msgIt], blockData, auxLen );                                        /* Block Data      */
    msgIt += auxLen;
    
    
    /*******************************************************************************/
    /* Transceive UPDATE command/request                                           */
    ret = rfalTransceiveBlockingTxRx( txBuf, msgIt, rxBuf, rxBufLen, &rcvdLen, RFAL_TXRX_FLAGS_DEFAULT, RFAL_NFCF_MRT_CHECK_UPDATE );
    
    if( ret == ERR_NONE )
    {
        /* Skip LEN byte */
        updateRes = (rxBuf + RFAL_NFCF_LENGTH_LEN);
        
        /* Check response length */
        if( rcvdLen < (RFAL_NFCF_LENGTH_LEN + RFAL_NFCF_CHECKUPDATE_RES_ST2_POS) )
        {
            ret = ERR_PROTO;
        }
        /* Check for a valid response */
        else if( (updateRes[RFAL_NFCF_CMD_POS] != (uint8_t)RFAL_NFCF_CMD_WRITE_WITHOUT_ENCRYPTION_RES) ||
                 (updateRes[RFAL_NFCF_CHECKUPDATE_RES_ST1_POS] != RFAL_NFCF_STATUS_FLAG_SUCCESS)       ||
                 (updateRes[RFAL_NFCF_CHECKUPDATE_RES_ST2_POS] != RFAL_NFCF_STATUS_FLAG_SUCCESS)         )
        {
            ret = ERR_REQUEST;
        }
        else
        {
            /* MISRA 15.7 - Empty else */
        }
    }
    
    return ret;
}



/*******************************************************************************/
bool rfalNfcfListenerIsT3TReq( const uint8_t* buf, uint16_t bufLen, uint8_t* nfcid2 )
{
    /* Check cmd byte */
    switch( *buf )
    {
        case RFAL_NFCF_CMD_READ_WITHOUT_ENCRYPTION:
            if( bufLen < RFAL_NFCF_READ_WO_ENCRYPTION_MIN_LEN )
            {
                return false;
            }
            break;
            
        case RFAL_NFCF_CMD_WRITE_WITHOUT_ENCRYPTION:
            if( bufLen < RFAL_NFCF_WRITE_WO_ENCRYPTION_MIN_LEN )
            {
                return false;
            }
            break;
            
        default:
            return false;       
    }
    
    /* Output NFID2 if requested */
    if( nfcid2 != NULL )
    {
        ST_MEMCPY( nfcid2, &buf[RFAL_NFCF_CMD_LEN], RFAL_NFCF_NFCID2_LEN );
    }
    
    return true;
}

#endif /* RFAL_FEATURE_NFCF */
