
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

/*! \file rfal_nfca.c
 *
 *  \author Gustavo Patricio
 *
 *  \brief Provides several NFC-A convenience methods and definitions
 *  
 *  It provides a Poller (ISO14443A PCD) interface and as well as 
 *  some NFC-A Listener (ISO14443A PICC) helpers.
 *
 *  The definitions and helpers methods provided by this module are only
 *  up to ISO14443-3 layer
 *  
 */

/*
 ******************************************************************************
 * INCLUDES
 ******************************************************************************
 */
#include "rfal_nfca.h"
#include "utils.h"

/*
 ******************************************************************************
 * ENABLE SWITCH
 ******************************************************************************
 */

#ifndef RFAL_FEATURE_NFCA
    #define RFAL_FEATURE_NFCA   false    /* NFC-A module configuration missing. Disabled by default */
#endif

#if RFAL_FEATURE_NFCA

/*
 ******************************************************************************
 * GLOBAL DEFINES
 ******************************************************************************
 */

#define RFAL_NFCA_SLP_FWT           rfalConvMsTo1fc(1)    /*!< Check 1ms for any modulation  ISO14443-3 6.4.3   */
#define RFAL_NFCA_SLP_CMD           0x50U                 /*!< SLP cmd (byte1)    Digital 1.1  6.9.1 & Table 20 */
#define RFAL_NFCA_SLP_BYTE2         0x00U                 /*!< SLP byte2          Digital 1.1  6.9.1 & Table 20 */
#define RFAL_NFCA_SLP_CMD_POS       0U                    /*!< SLP cmd position   Digital 1.1  6.9.1 & Table 20 */
#define RFAL_NFCA_SLP_BYTE2_POS     1U                    /*!< SLP byte2 position Digital 1.1  6.9.1 & Table 20 */

#define RFAL_NFCA_SDD_CT            0x88U                 /*!< Cascade Tag value Digital 1.1 6.7.2              */
#define RFAL_NFCA_SDD_CT_LEN        1U                    /*!< Cascade Tag length                               */

#define RFAL_NFCA_SLP_REQ_LEN       2U                    /*!< SLP_REQ length                                   */

#define RFAL_NFCA_SEL_CMD_LEN       1U                    /*!< SEL_CMD length                                   */
#define RFAL_NFCA_SEL_PAR_LEN       1U                    /*!< SEL_PAR length                                   */
#define RFAL_NFCA_SEL_SELPAR        rfalNfcaSelPar(7U, 0U)/*!< SEL_PAR on Select is always with 4 data/nfcid    */
#define RFAL_NFCA_BCC_LEN           1U                    /*!< BCC length                                       */

#define RFAL_NFCA_SDD_REQ_LEN       (RFAL_NFCA_SEL_CMD_LEN + RFAL_NFCA_SEL_PAR_LEN)   /*!< SDD_REQ length       */
#define RFAL_NFCA_SDD_RES_LEN       (RFAL_NFCA_CASCADE_1_UID_LEN + RFAL_NFCA_BCC_LEN) /*!< SDD_RES length       */

#define RFAL_NFCA_T_RETRANS         5U                    /*!< t RETRANSMISSION [3, 33]ms   EMVCo 2.6  A.5      */
#define RFAL_NFCA_N_RETRANS         2U                    /*!< Number of retries            EMVCo 2.6  9.6.1.3  */
 

/*! SDD_REQ (Select) Cascade Levels  */
enum
{
    RFAL_NFCA_SEL_CASCADE_L1 = 0,  /*!< SDD_REQ Cascade Level 1 */
    RFAL_NFCA_SEL_CASCADE_L2 = 1,  /*!< SDD_REQ Cascade Level 2 */
    RFAL_NFCA_SEL_CASCADE_L3 = 2   /*!< SDD_REQ Cascade Level 3 */
};

/*! SDD_REQ (Select) request Cascade Level command   Digital 1.1 Table 15 */
enum
{
    RFAL_NFCA_CMD_SEL_CL1 = 0x93, /*!< SDD_REQ command Cascade Level 1 */
    RFAL_NFCA_CMD_SEL_CL2 = 0x95, /*!< SDD_REQ command Cascade Level 2 */
    RFAL_NFCA_CMD_SEL_CL3 = 0x97, /*!< SDD_REQ command Cascade Level 3 */
};

/*
******************************************************************************
* GLOBAL MACROS
******************************************************************************
*/
#define rfalNfcaSelPar( nBy, nbi )         (uint8_t)((((nBy)<<4U) & 0xF0U) | ((nbi)&0x0FU) )         /*!< Calculates SEL_PAR with the bytes/bits to be sent */
#define rfalNfcaCLn2SELCMD( cl )           (uint8_t)((uint8_t)(RFAL_NFCA_CMD_SEL_CL1) + (2U*(cl)))   /*!< Calculates SEL_CMD with the given cascade level   */
#define rfalNfcaNfcidLen2CL( len )         ((len) / 5U)                                              /*!< Calculates cascade level by the NFCID length      */
#define rfalNfcaRunBlocking( e, fn )       do{ (e)=(fn); rfalWorker(); }while( (e) == ERR_BUSY )     /*!< Macro used for the blocking methods               */

/*
******************************************************************************
* GLOBAL TYPES
******************************************************************************
*/

/*! Colission Resolution states */
typedef enum{
    RFAL_NFCA_CR_IDLE,                     /*!< IDLE state                      */
    RFAL_NFCA_CR_CL,                       /*!< New Cascading Level state       */
    RFAL_NFCA_CR_SDD,                      /*!< Perform anticollsion state      */
    RFAL_NFCA_CR_SEL,                      /*!< Perform CL Selection state      */
    RFAL_NFCA_CR_DONE                      /*!< Collision Resolution done state */
}colResState;


/*! Colission Resolution context */
typedef struct{
    uint8_t               devLimit;        /*!< Device limit to be used                                 */
    rfalComplianceMode    compMode;        /*!< Compliancy mode to be used                              */
    rfalNfcaListenDevice* nfcaDevList;     /*!< Location of the device list                             */
    uint8_t*              devCnt;          /*!< Location of the device counter                          */
    bool                  collPending;     /*!< Collision pending flag                                  */
    
    bool*                 collPend;         /*!< Location of collision pending flag (Single CR)          */
    rfalNfcaSelReq        selReq;           /*!< SelReqused during anticollision (Single CR)             */
    rfalNfcaSelRes*       selRes;           /*!< Location to place of the SEL_RES(SAK) (Single CR)       */
    uint8_t*              nfcId1;           /*!< Location to place the NFCID1 (Single CR)                */
    uint8_t*              nfcId1Len;        /*!< Location to place the NFCID1 length (Single CR)         */
    uint8_t               cascadeLv;        /*!< Current Cascading Level (Single CR)                     */
    colResState           state;            /*!< Single Collision Resolution state (Single CR)           */
    uint8_t               bytesTxRx;        /*!< TxRx bytes used during anticollision loop (Single CR)   */
    uint8_t               bitsTxRx;         /*!< TxRx bits used during anticollision loop (Single CR)    */
    uint16_t              rxLen;    
    uint32_t              tmrFDT;           /*!< FDT timer used between SED_REQs  (Single CR)            */
    uint8_t               retries;          /*!< Retries to be performed upon a timeout error (Single CR)*/
    uint8_t               backtrackCnt;     /*!< Backtrack retries (Single CR)                           */
    bool                  doBacktrack;      /*!< Backtrack flag (Single CR)                              */
}colResParams;


/*! RFAL NFC-A instance */
typedef struct{
    colResParams          CR;               /*!< Collision Resolution context                            */
} rfalNfca;


/*! SLP_REQ (HLTA) format   Digital 1.1  6.9.1 & Table 20 */
typedef struct
{
    uint8_t      frame[RFAL_NFCA_SLP_REQ_LEN];  /*!< SLP:  0x50 0x00  */
} rfalNfcaSlpReq;


/*
******************************************************************************
* LOCAL VARIABLES
******************************************************************************
*/
static rfalNfca gNfca;  /*!< RFAL NFC-A instance  */

/*
******************************************************************************
* LOCAL FUNCTION PROTOTYPES
******************************************************************************
*/
static uint8_t    rfalNfcaCalculateBcc( const uint8_t* buf, uint8_t bufLen );
static ReturnCode rfalNfcaPollerStartSingleCollisionResolution( uint8_t devLimit, bool *collPending, rfalNfcaSelRes *selRes, uint8_t *nfcId1, uint8_t *nfcId1Len );
static ReturnCode rfalNfcaPollerGetSingleCollisionResolutionStatus( void );

/*
 ******************************************************************************
 * LOCAL FUNCTIONS
 ******************************************************************************
 */

static uint8_t rfalNfcaCalculateBcc( const uint8_t* buf, uint8_t bufLen )
{
    uint8_t i;
    uint8_t BCC;
    
    BCC = 0;
    
    /* BCC is XOR over first 4 bytes of the SDD_RES  Digital 1.1 6.7.2 */
    for(i = 0; i < bufLen; i++)
    {
        BCC ^= buf[i];
    }
    
    return BCC;
}

/*******************************************************************************/
static ReturnCode rfalNfcaPollerStartSingleCollisionResolution( uint8_t devLimit, bool *collPending, rfalNfcaSelRes *selRes, uint8_t *nfcId1, uint8_t *nfcId1Len )
{
    /* Check parameters */
    if( (collPending == NULL) || (selRes == NULL) || (nfcId1 == NULL) || (nfcId1Len == NULL) )
    {
        return ERR_PARAM;
    }
    
    /* Initialize output parameters */
    *collPending = false;  /* Activity 1.1  9.3.4.6 */
    *nfcId1Len   = 0;
    ST_MEMSET( nfcId1, 0x00, RFAL_NFCA_CASCADE_3_UID_LEN );
    
    
    /* Save parameters */
    gNfca.CR.devLimit    = devLimit;
    gNfca.CR.collPend    = collPending;
    gNfca.CR.selRes      = selRes;
    gNfca.CR.nfcId1      = nfcId1;
    gNfca.CR.nfcId1Len   = nfcId1Len;

    platformTimerDestroy( gNfca.CR.tmrFDT );
    gNfca.CR.tmrFDT      = 0U;
    gNfca.CR.retries     = RFAL_NFCA_N_RETRANS;
    gNfca.CR.cascadeLv   = (uint8_t)RFAL_NFCA_SEL_CASCADE_L1;
    gNfca.CR.state       = RFAL_NFCA_CR_CL;
   
    gNfca.CR.doBacktrack  = false;
    gNfca.CR.backtrackCnt = 3U;
    
    return ERR_NONE;
}


/*******************************************************************************/
static ReturnCode rfalNfcaPollerGetSingleCollisionResolutionStatus( void )
{
    ReturnCode ret;
    uint8_t    collBit = 1U;  /* standards mandate or recommend collision bit to be set to One. */
    
    
    /* Check if FDT timer is still running */
    if( !platformTimerIsExpired( gNfca.CR.tmrFDT ) && (gNfca.CR.tmrFDT != 0U) )
    {
        return ERR_BUSY;
    }
    
    /*******************************************************************************/
    /* Go through all Cascade Levels     Activity 1.1  9.3.4 */    
    if( gNfca.CR.cascadeLv > (uint8_t)RFAL_NFCA_SEL_CASCADE_L3 )
    {
        return ERR_INTERNAL;
    }
    
    switch( gNfca.CR.state )
    {
        /*******************************************************************************/
        case RFAL_NFCA_CR_CL:
            
            /* Initialize the SDD_REQ to send for the new cascade level */
            ST_MEMSET( (uint8_t*)&gNfca.CR.selReq, 0x00, sizeof(rfalNfcaSelReq) );
        
            gNfca.CR.bytesTxRx = RFAL_NFCA_SDD_REQ_LEN;
            gNfca.CR.bitsTxRx  = 0U;
            gNfca.CR.state     = RFAL_NFCA_CR_SDD;
        
            /* fall through */
        
        /*******************************************************************************/
        case RFAL_NFCA_CR_SDD:   /*  PRQA S 2003 # MISRA 16.3 - Intentional fall through */
            
            /* Calculate SEL_CMD and SEL_PAR with the bytes/bits to be sent */
            gNfca.CR.selReq.selCmd = rfalNfcaCLn2SELCMD( gNfca.CR.cascadeLv );
            gNfca.CR.selReq.selPar = rfalNfcaSelPar(gNfca.CR.bytesTxRx, gNfca.CR.bitsTxRx);
        
            /* Send SDD_REQ (Anticollision frame) */
            ret = rfalISO14443ATransceiveAnticollisionFrame( (uint8_t*)&gNfca.CR.selReq, &gNfca.CR.bytesTxRx, &gNfca.CR.bitsTxRx, &gNfca.CR.rxLen, RFAL_NFCA_FDTMIN );

            /* Retry upon timeout  EMVCo 2.6  9.6.1.3 */
            if( (ret == ERR_TIMEOUT) && (gNfca.CR.devLimit==0U) && (gNfca.CR.retries != 0U) )
            {
                gNfca.CR.retries--;
                platformTimerDestroy( gNfca.CR.tmrFDT );
                gNfca.CR.tmrFDT = platformTimerCreate( RFAL_NFCA_T_RETRANS );
                break;
            }
            
            /* Covert rxLen into bytes */
            gNfca.CR.rxLen = rfalConvBitsToBytes( gNfca.CR.rxLen );
            
            
            if( (ret == ERR_TIMEOUT) && (gNfca.CR.backtrackCnt != 0U) && (!gNfca.CR.doBacktrack)
                && !((RFAL_NFCA_SDD_REQ_LEN == gNfca.CR.bytesTxRx) && (0U == gNfca.CR.bitsTxRx))     )
            {
                /* In multiple card scenarios it may always happen that some 
                 * collisions of a weaker tag go unnoticed. If then a later 
                 * collision is recognized and the strong tag has a 0 at the 
                 * collision position then no tag will respond. Catch this 
                 * corner case and then try with the bit being sent as zero. */
                rfalNfcaSensRes sensRes;
                ret = ERR_RF_COLLISION;
                rfalNfcaPollerCheckPresence( RFAL_14443A_SHORTFRAME_CMD_REQA, &sensRes );
                /* Algorithm below does a post-increment, decrement to go back to current position */
                if (0U == gNfca.CR.bitsTxRx)
                {
                    gNfca.CR.bitsTxRx = 7;
                    gNfca.CR.bytesTxRx--;
                }
                else
                {
                    gNfca.CR.bitsTxRx--;
                }
                collBit = (uint8_t)( ((uint8_t*)&gNfca.CR.selReq)[gNfca.CR.bytesTxRx] & (1U << gNfca.CR.bitsTxRx) );
                collBit = (uint8_t)((0U==collBit)?1U:0U); // invert the collision bit
                gNfca.CR.doBacktrack = true;
                gNfca.CR.backtrackCnt--;
            }
            else
            {
                gNfca.CR.doBacktrack = false;
            }

            if( ret == ERR_RF_COLLISION )
            {
                /* Check received length */
                if( (gNfca.CR.bytesTxRx + ((gNfca.CR.bitsTxRx != 0U) ? 1U : 0U)) > (RFAL_NFCA_SDD_RES_LEN + RFAL_NFCA_SDD_REQ_LEN) )
                {
                    return ERR_PROTO;
                }

                if( ((gNfca.CR.bytesTxRx + ((gNfca.CR.bitsTxRx != 0U) ? 1U : 0U)) > (RFAL_NFCA_CASCADE_1_UID_LEN + RFAL_NFCA_SDD_REQ_LEN)) && (gNfca.CR.backtrackCnt != 0U) )
                { /* Collision in BCC: Anticollide only UID part */
                    gNfca.CR.backtrackCnt--;
                    gNfca.CR.bytesTxRx = RFAL_NFCA_CASCADE_1_UID_LEN + RFAL_NFCA_SDD_REQ_LEN - 1U;
                    gNfca.CR.bitsTxRx = 7;
                    collBit = (uint8_t)( ((uint8_t*)&gNfca.CR.selReq)[gNfca.CR.bytesTxRx] & (1U << gNfca.CR.bitsTxRx) ); /* Not a real collision, extract the actual bit for the subsequent code */
                }
                
                if( (gNfca.CR.devLimit == 0U) && !(*gNfca.CR.collPend) )
                {   
                    /* Activity 1.0 & 1.1  9.3.4.12: If CON_DEVICES_LIMIT has a value of 0, then 
                     * NFC Forum Device is configured to perform collision detection only       */
                    *gNfca.CR.collPend = true;
                    return ERR_IGNORE;
                }
                
                *gNfca.CR.collPend = true;
                
                /* Set and select the collision bit, with the number of bytes/bits successfully TxRx */
                if (collBit != 0U)
                {
                    ((uint8_t*)&gNfca.CR.selReq)[gNfca.CR.bytesTxRx] = (uint8_t)(((uint8_t*)&gNfca.CR.selReq)[gNfca.CR.bytesTxRx] | (1U << gNfca.CR.bitsTxRx));   /* MISRA 10.3 */
                }
                else
                {
                    ((uint8_t*)&gNfca.CR.selReq)[gNfca.CR.bytesTxRx] = (uint8_t)(((uint8_t*)&gNfca.CR.selReq)[gNfca.CR.bytesTxRx] & ~(1U << gNfca.CR.bitsTxRx));  /* MISRA 10.3 */
                }

                gNfca.CR.bitsTxRx++;
                
                /* Check if number of bits form a byte */
                if( gNfca.CR.bitsTxRx == RFAL_BITS_IN_BYTE )
                {
                    gNfca.CR.bitsTxRx = 0;
                    gNfca.CR.bytesTxRx++;
                }
                break;
            }
            
            /*******************************************************************************/
            /* Check if Collision loop has failed */
            if( ret != ERR_NONE )
            {
                return ret;
            }
            
            
            /* If collisions are to be reported check whether the response is complete */
            if( (gNfca.CR.devLimit == 0U) && (gNfca.CR.rxLen != sizeof(rfalNfcaSddRes)) )
            {
                return ERR_PROTO;
            }
            
            /* Check if the received BCC match */
            if( gNfca.CR.selReq.bcc != rfalNfcaCalculateBcc( gNfca.CR.selReq.nfcid1, RFAL_NFCA_CASCADE_1_UID_LEN ) )
            {
                return ERR_PROTO;
            }
            
            /*******************************************************************************/
            /* Anticollision OK, Select this Cascade Level */
            gNfca.CR.selReq.selPar = RFAL_NFCA_SEL_SELPAR;
            
            gNfca.CR.retries = RFAL_NFCA_N_RETRANS;
            gNfca.CR.state   = RFAL_NFCA_CR_SEL;
            break;
            
        /*******************************************************************************/
        case RFAL_NFCA_CR_SEL:
            
            /* Send SEL_REQ (Select command) - Retry upon timeout  EMVCo 2.6  9.6.1.3 */
            ret = rfalTransceiveBlockingTxRx( (uint8_t*)&gNfca.CR.selReq, sizeof(rfalNfcaSelReq), (uint8_t*)gNfca.CR.selRes, sizeof(rfalNfcaSelRes), &gNfca.CR.rxLen, RFAL_TXRX_FLAGS_DEFAULT, RFAL_NFCA_FDTMIN );
                
            /* Retry upon timeout  EMVCo 2.6  9.6.1.3 */
            if( (ret == ERR_TIMEOUT) && (gNfca.CR.devLimit==0U) && (gNfca.CR.retries != 0U) )
            {
                gNfca.CR.retries--;
                platformTimerDestroy( gNfca.CR.tmrFDT );
                gNfca.CR.tmrFDT = platformTimerCreate( RFAL_NFCA_T_RETRANS );
                break;
            }
            
            if( ret != ERR_NONE )
            {
                return ret;
            }
            
            /* Ensure proper response length */
            if( gNfca.CR.rxLen != sizeof(rfalNfcaSelRes) )
            {
                return ERR_PROTO;
            }
            
            /*******************************************************************************/
            /* Check cascade byte, if cascade tag then go next cascade level */
            if( *gNfca.CR.selReq.nfcid1 == RFAL_NFCA_SDD_CT )
            {
                /* Cascade Tag present, store nfcid1 bytes (excluding cascade tag) and continue for next CL */
                ST_MEMCPY( &gNfca.CR.nfcId1[*gNfca.CR.nfcId1Len], &((uint8_t*)&gNfca.CR.selReq.nfcid1)[RFAL_NFCA_SDD_CT_LEN], (RFAL_NFCA_CASCADE_1_UID_LEN - RFAL_NFCA_SDD_CT_LEN) );
                *gNfca.CR.nfcId1Len += (RFAL_NFCA_CASCADE_1_UID_LEN - RFAL_NFCA_SDD_CT_LEN);
                
                /* Go to next cascade level */
                gNfca.CR.state = RFAL_NFCA_CR_CL;
                gNfca.CR.cascadeLv++;
            }
            else
            {
                /* UID Selection complete, Stop Cascade Level loop */
                ST_MEMCPY( &gNfca.CR.nfcId1[*gNfca.CR.nfcId1Len], (uint8_t*)&gNfca.CR.selReq.nfcid1, RFAL_NFCA_CASCADE_1_UID_LEN );
                *gNfca.CR.nfcId1Len += RFAL_NFCA_CASCADE_1_UID_LEN;
                
                gNfca.CR.state = RFAL_NFCA_CR_DONE;
                break;                             /* Only flag operation complete on the next execution */
            }
            break;
        
        /*******************************************************************************/
        case RFAL_NFCA_CR_DONE:
            return ERR_NONE;
        
        /*******************************************************************************/
        default:
            return ERR_WRONG_STATE;
    }
    return ERR_BUSY;
}

/*
******************************************************************************
* GLOBAL FUNCTIONS
******************************************************************************
*/

/*******************************************************************************/
ReturnCode rfalNfcaPollerInitialize( void )
{
    ReturnCode ret;
    
    EXIT_ON_ERR( ret, rfalSetMode( RFAL_MODE_POLL_NFCA, RFAL_BR_106, RFAL_BR_106 ) );
    rfalSetErrorHandling( RFAL_ERRORHANDLING_NFC );
    
    rfalSetGT( RFAL_GT_NFCA );
    rfalSetFDTListen( RFAL_FDT_LISTEN_NFCA_POLLER );
    rfalSetFDTPoll( RFAL_FDT_POLL_NFCA_POLLER );
    
    return ERR_NONE;
}


/*******************************************************************************/
ReturnCode rfalNfcaPollerCheckPresence( rfal14443AShortFrameCmd cmd, rfalNfcaSensRes *sensRes )
{
    ReturnCode ret;
    uint16_t   rcvLen;
    
    /* Digital 1.1 6.10.1.3  For Commands ALL_REQ, SENS_REQ, SDD_REQ, and SEL_REQ, the NFC Forum Device      *
     *              MUST treat receipt of a Listen Frame at a time after FDT(Listen, min) as a Timeour Error */
    
    ret = rfalISO14443ATransceiveShortFrame(  cmd, (uint8_t*)sensRes, (uint8_t)rfalConvBytesToBits(sizeof(rfalNfcaSensRes)), &rcvLen, RFAL_NFCA_FDTMIN  );
    if( (ret == ERR_RF_COLLISION) || (ret == ERR_CRC)  || (ret == ERR_NOMEM) || (ret == ERR_FRAMING) || (ret == ERR_PAR) )
    {
       ret = ERR_NONE;
    }

    return ret;
}


/*******************************************************************************/
ReturnCode rfalNfcaPollerTechnologyDetection( rfalComplianceMode compMode, rfalNfcaSensRes *sensRes )
{
    ReturnCode ret;
    
    EXIT_ON_ERR( ret, rfalNfcaPollerCheckPresence( ((compMode == RFAL_COMPLIANCE_MODE_EMV) ? RFAL_14443A_SHORTFRAME_CMD_WUPA : RFAL_14443A_SHORTFRAME_CMD_REQA), sensRes ) );
    
    /* Send SLP_REQ as  Activity 1.1  9.2.3.6 and EMVCo 2.6  9.2.1.3 */
    if( compMode != RFAL_COMPLIANCE_MODE_ISO)
    {
        rfalNfcaPollerSleep();
    }
    return ERR_NONE;
}


/*******************************************************************************/
ReturnCode rfalNfcaPollerSingleCollisionResolution( uint8_t devLimit, bool *collPending, rfalNfcaSelRes *selRes, uint8_t *nfcId1, uint8_t *nfcId1Len )
{
    
    ReturnCode ret;
    
    EXIT_ON_ERR( ret, rfalNfcaPollerStartSingleCollisionResolution( devLimit, collPending, selRes, nfcId1, nfcId1Len ) );
    rfalNfcaRunBlocking( ret, rfalNfcaPollerGetSingleCollisionResolutionStatus() );
    
    return ret;
}


/*******************************************************************************/
ReturnCode rfalNfcaPollerStartFullCollisionResolution( rfalComplianceMode compMode, uint8_t devLimit, rfalNfcaListenDevice *nfcaDevList, uint8_t *devCnt )
{
    ReturnCode      ret;
    rfalNfcaSensRes sensRes;
    uint16_t        rcvLen;
    
    if( (nfcaDevList == NULL) || (devCnt == NULL) )
    {
        return ERR_PARAM;
    }
    
    *devCnt = 0;
    ret     = ERR_NONE;
    
    /*******************************************************************************/
    /* Send ALL_REQ before Anticollision if a Sleep was sent before  Activity 1.1  9.3.4.1 and EMVco 2.6  9.3.2.1 */
    if( compMode != RFAL_COMPLIANCE_MODE_ISO )
    {
        ret = rfalISO14443ATransceiveShortFrame( RFAL_14443A_SHORTFRAME_CMD_WUPA, (uint8_t*)&nfcaDevList->sensRes, (uint8_t)rfalConvBytesToBits(sizeof(rfalNfcaSensRes)), &rcvLen, RFAL_NFCA_FDTMIN  );
        if(ret != ERR_NONE)
        {
            if( (compMode == RFAL_COMPLIANCE_MODE_EMV) || ((ret != ERR_RF_COLLISION) && (ret != ERR_CRC) && (ret != ERR_FRAMING) && (ret != ERR_PAR)) )
            {
                return ret;
            }
        }
        
        /* Check proper SENS_RES/ATQA size */
        if( (ret == ERR_NONE) && (rfalConvBytesToBits(sizeof(rfalNfcaSensRes)) != rcvLen) )
        {
            return ERR_PROTO;
        }
    }
    
    /*******************************************************************************/
    /* Store the SENS_RES from Technology Detection or from WUPA */ 
    sensRes = nfcaDevList->sensRes;
    
    if( devLimit > 0U )  /* MISRA 21.18 */
    {
        ST_MEMSET( nfcaDevList, 0x00, (sizeof(rfalNfcaListenDevice) * devLimit) );
    }
    
    /* Restore the prev SENS_RES, assuming that the SENS_RES received is from first device
     * When only one device is detected it's not woken up then we'll have no SENS_RES (ATQA) */
    nfcaDevList->sensRes = sensRes;
    
    /* Save parameters */
    gNfca.CR.devCnt      = devCnt;
    gNfca.CR.devLimit    = devLimit;
    gNfca.CR.nfcaDevList = nfcaDevList;
    gNfca.CR.compMode    = compMode;
    
    
    #if RFAL_FEATURE_T1T
    /*******************************************************************************/
    /* Only check for T1T if previous SENS_RES was received without a transmission  *
     * error. When collisions occur bits in the SENS_RES may look like a T1T        */
    /* If T1T Anticollision is not supported  Activity 1.1  9.3.4.3 */
    if( rfalNfcaIsSensResT1T( &nfcaDevList->sensRes ) && (devLimit != 0U) && (ret == ERR_NONE) && (compMode != RFAL_COMPLIANCE_MODE_EMV) )
    {
        /* RID_REQ shall be performed              Activity 1.1  9.3.4.24 */
        rfalT1TPollerInitialize();
        EXIT_ON_ERR( ret, rfalT1TPollerRid( &nfcaDevList->ridRes ) );
        
        *devCnt = 1U;
        nfcaDevList->isSleep   = false;
        nfcaDevList->type      = RFAL_NFCA_T1T;
        nfcaDevList->nfcId1Len = RFAL_NFCA_CASCADE_1_UID_LEN;
        ST_MEMCPY( &nfcaDevList->nfcId1, &nfcaDevList->ridRes.uid, RFAL_NFCA_CASCADE_1_UID_LEN );
        
        return ERR_NONE;
    }
    #endif /* RFAL_FEATURE_T1T */
    
    return rfalNfcaPollerStartSingleCollisionResolution( devLimit, &gNfca.CR.collPending, &nfcaDevList->selRes, (uint8_t*)&nfcaDevList->nfcId1, &nfcaDevList->nfcId1Len );
}


/*******************************************************************************/
ReturnCode rfalNfcaPollerGetFullCollisionResolutionStatus( void )
{
    ReturnCode ret;
    uint8_t    newDevType;
    
    if( (gNfca.CR.nfcaDevList == NULL) || (gNfca.CR.devCnt == NULL) )
    {
        return ERR_WRONG_STATE;
    }
    
    /*******************************************************************************/
    /* Check whether a T1T has already been detected */
    if( rfalNfcaIsSensResT1T( &gNfca.CR.nfcaDevList->sensRes ) && (gNfca.CR.nfcaDevList->type == RFAL_NFCA_T1T) )
    {
        /* T1T doesn't support Anticollision */
        return ERR_NONE;
    }
    
    
    /*******************************************************************************/
    EXIT_ON_ERR( ret, rfalNfcaPollerGetSingleCollisionResolutionStatus() );

    /* Assign Listen Device */
    newDevType = ((uint8_t)gNfca.CR.nfcaDevList[*gNfca.CR.devCnt].selRes.sak) & RFAL_NFCA_SEL_RES_CONF_MASK;  /* MISRA 10.8 */
    /* PRQA S 4342 1 # MISRA 10.5 - Guaranteed that no invalid enum values are created: see guard_eq_RFAL_NFCA_T2T, .... */
    gNfca.CR.nfcaDevList[*gNfca.CR.devCnt].type    = (rfalNfcaListenDeviceType) newDevType;
    gNfca.CR.nfcaDevList[*gNfca.CR.devCnt].isSleep = false;
    (*gNfca.CR.devCnt)++;

    
    /* If a collision was detected and device counter is lower than limit  Activity 1.1  9.3.4.21 */
    if( (*gNfca.CR.devCnt < gNfca.CR.devLimit) && (gNfca.CR.collPending) )
    {
        /* Put this device to Sleep  Activity 1.1  9.3.4.22 */
        rfalNfcaPollerSleep();
        gNfca.CR.nfcaDevList[(*gNfca.CR.devCnt - 1U)].isSleep = true;
        
        
        /* Send a new SENS_REQ to check for other cards  Activity 1.1  9.3.4.23 */
        ret = rfalNfcaPollerCheckPresence( RFAL_14443A_SHORTFRAME_CMD_REQA, &gNfca.CR.nfcaDevList[*gNfca.CR.devCnt].sensRes );
        if( ret == ERR_TIMEOUT )
        {
            /* No more devices found, exit */
            gNfca.CR.collPending = false;
        }
        else
        {
            /* Another device found, continue loop */
            gNfca.CR.collPending = true;
        }
    }
    else
    {
        /* Exit loop */
        gNfca.CR.collPending = false;
    }
        
    
    /*******************************************************************************/
    /* Check if collision resolution shall continue */
    if( (*gNfca.CR.devCnt < gNfca.CR.devLimit) && (gNfca.CR.collPending) )
    {
        EXIT_ON_ERR( ret, rfalNfcaPollerStartSingleCollisionResolution(  gNfca.CR.devLimit, 
                                                                         &gNfca.CR.collPending, 
                                                                         &gNfca.CR.nfcaDevList[*gNfca.CR.devCnt].selRes, 
                                                                         (uint8_t*)&gNfca.CR.nfcaDevList[*gNfca.CR.devCnt].nfcId1, 
                                                                         &gNfca.CR.nfcaDevList[*gNfca.CR.devCnt].nfcId1Len ) );
    
        return ERR_BUSY;
    }
    
    return ERR_NONE;
}


/*******************************************************************************/
ReturnCode rfalNfcaPollerFullCollisionResolution( rfalComplianceMode compMode, uint8_t devLimit, rfalNfcaListenDevice *nfcaDevList, uint8_t *devCnt )
{
    ReturnCode ret;
    
    EXIT_ON_ERR( ret, rfalNfcaPollerStartFullCollisionResolution( compMode, devLimit, nfcaDevList, devCnt ) );
    rfalNfcaRunBlocking( ret, rfalNfcaPollerGetFullCollisionResolutionStatus() );
    
    return ret;
}

ReturnCode rfalNfcaPollerSleepFullCollisionResolution( uint8_t devLimit, rfalNfcaListenDevice *nfcaDevList, uint8_t *devCnt )
{
    bool       firstRound;
    uint8_t    tmpDevCnt;
    ReturnCode ret;


    if( (nfcaDevList == NULL) || (devCnt == NULL) )
    {
        return ERR_PARAM;
    }

    /* Only use ALL_REQ (WUPA) on the first round */
    firstRound = true;  
    *devCnt    = 0;
    
    
    /* Perform collision resolution until no new device is found */
    do
    {
        tmpDevCnt = 0;
        ret = rfalNfcaPollerFullCollisionResolution( (firstRound ? RFAL_COMPLIANCE_MODE_NFC : RFAL_COMPLIANCE_MODE_ISO), (devLimit - *devCnt), &nfcaDevList[*devCnt], &tmpDevCnt );

        if( (ret == ERR_NONE) && (tmpDevCnt > 0U) )
        {
            *devCnt += tmpDevCnt;

            /* Check whether to seacrh for more devices */
            if( *devCnt < devLimit )
            {
                /* Set last found device to sleep (all others are slept already) */
                rfalNfcaPollerSleep();
                nfcaDevList[((*devCnt)-1U)].isSleep = true;
                
                /* Check if any other device is present */
                ret = rfalNfcaPollerCheckPresence( RFAL_14443A_SHORTFRAME_CMD_REQA, &nfcaDevList[*devCnt].sensRes );
                if( ret == ERR_NONE )
                {
                    firstRound = false;
                    continue;
                }
            }
        }
        break;
    }
    while( true );

    return ((*devCnt > 0U) ? ERR_NONE : ret);
}


/*******************************************************************************/
ReturnCode rfalNfcaPollerSelect( const uint8_t *nfcid1, uint8_t nfcidLen, rfalNfcaSelRes *selRes )
{
    uint8_t        i;
    uint8_t        cl;
    uint8_t        nfcidOffset;
    uint16_t       rxLen;
    ReturnCode     ret;
    rfalNfcaSelReq selReq;
    
    if( (nfcid1 == NULL) || (nfcidLen > RFAL_NFCA_CASCADE_3_UID_LEN) || (selRes == NULL) )
    {
        return ERR_PARAM;
    }
    
    
    /* Calculate Cascate Level */
    cl          = rfalNfcaNfcidLen2CL( nfcidLen );
    nfcidOffset = 0;
    
    /*******************************************************************************/
    /* Go through all Cascade Levels     Activity 1.1  9.4.4 */
    for( i = RFAL_NFCA_SEL_CASCADE_L1; i <= cl; i++ )
    {
        /* Assign SEL_CMD according to the CLn and SEL_PAR*/
        selReq.selCmd = rfalNfcaCLn2SELCMD(i);
        selReq.selPar = RFAL_NFCA_SEL_SELPAR;
        
        /* Compute NFCID/Data on the SEL_REQ command   Digital 1.1  Table 18 */
        if( cl != i )
        {
            *selReq.nfcid1 = RFAL_NFCA_SDD_CT;
            ST_MEMCPY( &selReq.nfcid1[RFAL_NFCA_SDD_CT_LEN], &nfcid1[nfcidOffset], (RFAL_NFCA_CASCADE_1_UID_LEN - RFAL_NFCA_SDD_CT_LEN) );
            nfcidOffset += (RFAL_NFCA_CASCADE_1_UID_LEN - RFAL_NFCA_SDD_CT_LEN);
        }
        else
        {
            ST_MEMCPY( selReq.nfcid1, &nfcid1[nfcidOffset], RFAL_NFCA_CASCADE_1_UID_LEN );
        }
        
        /* Calculate nfcid's BCC */
        selReq.bcc = rfalNfcaCalculateBcc( (uint8_t*)&selReq.nfcid1, sizeof(selReq.nfcid1) );
        
        /*******************************************************************************/
        /* Send SEL_REQ  */
        EXIT_ON_ERR( ret, rfalTransceiveBlockingTxRx( (uint8_t*)&selReq, sizeof(rfalNfcaSelReq), (uint8_t*)selRes, sizeof(rfalNfcaSelRes), &rxLen, RFAL_TXRX_FLAGS_DEFAULT, RFAL_NFCA_FDTMIN ) );
        
        /* Ensure proper response length */
        if( rxLen != sizeof(rfalNfcaSelRes) )
        {
            return ERR_PROTO;
        }
    }
    
    /* REMARK: Could check if NFCID1 is complete */
    
    return ERR_NONE;
}


/*******************************************************************************/
ReturnCode rfalNfcaPollerSleep( void )
{
    rfalNfcaSlpReq slpReq;
    uint8_t        rxBuf;    /* dummy buffer, just to perform Rx */
    
    slpReq.frame[RFAL_NFCA_SLP_CMD_POS]   = RFAL_NFCA_SLP_CMD;
    slpReq.frame[RFAL_NFCA_SLP_BYTE2_POS] = RFAL_NFCA_SLP_BYTE2;
    
    rfalTransceiveBlockingTxRx( (uint8_t*)&slpReq, sizeof(rfalNfcaSlpReq), &rxBuf, sizeof(rxBuf), NULL, RFAL_TXRX_FLAGS_DEFAULT, RFAL_NFCA_SLP_FWT );
        
    /* ISO14443-3 6.4.3  HLTA - If PICC responds with any modulation during 1 ms this response shall be interpreted as not acknowledge 
       Digital 2.0  6.9.2.1 & EMVCo 3.0  5.6.2.1 - consider the HLTA command always acknowledged
       No check to be compliant with NFC and EMVCo, and to improve interoprability (Kovio RFID Tag)
    */
    
    return ERR_NONE;
}


/*******************************************************************************/
bool rfalNfcaListenerIsSleepReq( const uint8_t *buf, uint16_t bufLen )
{
    /* Check if length and payload match */
    if( (bufLen != sizeof(rfalNfcaSlpReq)) || (buf[RFAL_NFCA_SLP_CMD_POS] != RFAL_NFCA_SLP_CMD) || (buf[RFAL_NFCA_SLP_BYTE2_POS] != RFAL_NFCA_SLP_BYTE2) )
    {
        return false;
    }
    
    return true;
}

/* If the guards here don't compile then the code above cannot work anymore. */
extern uint8_t guard_eq_RFAL_NFCA_T2T[((RFAL_NFCA_SEL_RES_CONF_MASK&(uint8_t)RFAL_NFCA_T2T) == (uint8_t)RFAL_NFCA_T2T)?1:(-1)];
extern uint8_t guard_eq_RFAL_NFCA_T4T[((RFAL_NFCA_SEL_RES_CONF_MASK&(uint8_t)RFAL_NFCA_T4T) == (uint8_t)RFAL_NFCA_T4T)?1:(-1)];
extern uint8_t guard_eq_RFAL_NFCA_NFCDEP[((RFAL_NFCA_SEL_RES_CONF_MASK&(uint8_t)RFAL_NFCA_NFCDEP) == (uint8_t)RFAL_NFCA_NFCDEP)?1:(-1)];
extern uint8_t guard_eq_RFAL_NFCA_T4T_NFCDEP[((RFAL_NFCA_SEL_RES_CONF_MASK&(uint8_t)RFAL_NFCA_T4T_NFCDEP) == (uint8_t)RFAL_NFCA_T4T_NFCDEP)?1:(-1)];
#endif /* RFAL_FEATURE_NFCA */
