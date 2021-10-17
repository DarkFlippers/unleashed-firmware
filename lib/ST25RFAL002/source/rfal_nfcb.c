
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

/*! \file rfal_nfcb.c
 *
 *  \author Gustavo Patricio
 *
 *  \brief Implementation of NFC-B (ISO14443B) helpers 
 *
 */

/*
 ******************************************************************************
 * INCLUDES
 ******************************************************************************
 */
#include "rfal_nfcb.h"
#include "utils.h"

/*
 ******************************************************************************
 * ENABLE SWITCH
 ******************************************************************************
 */

#ifndef RFAL_FEATURE_NFCB
    #define RFAL_FEATURE_NFCB   false    /* NFC-B module configuration missing. Disabled by default */
#endif

#if RFAL_FEATURE_NFCB

/*
 ******************************************************************************
 * GLOBAL DEFINES
 ******************************************************************************
 */

#define RFAL_NFCB_SENSB_REQ_EXT_SENSB_RES_SUPPORTED  0x10U /*!< Bit mask for Extended SensB Response support in SENSB_REQ */
#define RFAL_NFCB_SENSB_RES_PROT_TYPE_RFU            0x08U /*!< Bit mask for Protocol Type RFU in SENSB_RES               */
#define RFAL_NFCB_SLOT_MARKER_SC_SHIFT               4U    /*!< Slot Code position on SLOT_MARKER APn                     */

#define RFAL_NFCB_SLOTMARKER_SLOTCODE_MIN            1U    /*!< SLOT_MARKER Slot Code minimum   Digital 1.1  Table 37     */ 
#define RFAL_NFCB_SLOTMARKER_SLOTCODE_MAX            16U   /*!< SLOT_MARKER Slot Code maximum   Digital 1.1  Table 37     */

#define RFAL_NFCB_ACTIVATION_FWT                    (RFAL_NFCB_FWTSENSB + RFAL_NFCB_DTPOLL_20)  /*!< FWT(SENSB) + dTbPoll  Digital 2.0  7.9.1.3  */

/*! Advanced and Extended bit mask in Parameter of SENSB_REQ */
#define RFAL_NFCB_SENSB_REQ_PARAM                   (RFAL_NFCB_SENSB_REQ_ADV_FEATURE | RFAL_NFCB_SENSB_REQ_EXT_SENSB_RES_SUPPORTED)


/*! NFC-B commands definition */
enum
{
    RFAL_NFCB_CMD_SENSB_REQ = 0x05,   /*!< SENSB_REQ (REQB) & SLOT_MARKER  Digital 1.1 Table 24 */
    RFAL_NFCB_CMD_SENSB_RES = 0x50,   /*!< SENSB_RES (ATQB) & SLOT_MARKER  Digital 1.1 Table 27 */
    RFAL_NFCB_CMD_SLPB_REQ  = 0x50,   /*!< SLPB_REQ (HLTB command)  Digital 1.1 Table 38        */
    RFAL_NFCB_CMD_SLPB_RES  = 0x00    /*!< SLPB_RES (HLTB Answer)   Digital 1.1 Table 39        */
};

/*
 ******************************************************************************
 * GLOBAL MACROS
 ******************************************************************************
 */

#define rfalNfcbNI2NumberOfSlots( ni )  (uint8_t)(1U << (ni))  /*!< Converts the Number of slots Identifier to slot number */

/*
******************************************************************************
* GLOBAL TYPES
******************************************************************************
*/

/*! ALLB_REQ (WUPB) and SENSB_REQ (REQB) Command Format   Digital 1.1  7.6.1 */
typedef struct
{
    uint8_t  cmd;                            /*!< xxxxB_REQ: 05h       */
    uint8_t  AFI;                            /*!< NFC Identifier       */
    uint8_t  PARAM;                          /*!< Application Data     */
} rfalNfcbSensbReq;

/*! SLOT_MARKER Command format  Digital 1.1  7.7.1 */
typedef struct
{
    uint8_t  APn;    /*!< Slot number 2..16 | 0101b */
} rfalNfcbSlotMarker;

/*! SLPB_REQ (HLTB) Command Format   Digital 1.1  7.8.1 */
typedef struct
{
    uint8_t  cmd;                            /*!< SLPB_REQ: 50h        */
    uint8_t  nfcid0[RFAL_NFCB_NFCID0_LEN];   /*!< NFC Identifier (PUPI)*/
} rfalNfcbSlpbReq;


/*! SLPB_RES (HLTB) Response Format   Digital 1.1  7.8.2 */
typedef struct
{
    uint8_t  cmd;                            /*!< SLPB_RES: 00h        */
} rfalNfcbSlpbRes;


/*! RFAL NFC-B instance */
typedef struct
{
    uint8_t  AFI;                            /*!< AFI to be used       */
    uint8_t  PARAM;                          /*!< PARAM to be used     */
} rfalNfcb;

/*
******************************************************************************
* LOCAL FUNCTION PROTOTYPES
******************************************************************************
*/
static ReturnCode rfalNfcbCheckSensbRes( const rfalNfcbSensbRes *sensbRes, uint8_t sensbResLen );


/*
******************************************************************************
* LOCAL VARIABLES
******************************************************************************
*/

static rfalNfcb gRfalNfcb; /*!< RFAL NFC-B Instance */


/*
******************************************************************************
* LOCAL FUNCTIONS
******************************************************************************
*/

/*******************************************************************************/
static ReturnCode rfalNfcbCheckSensbRes( const rfalNfcbSensbRes *sensbRes, uint8_t sensbResLen )
{
    /* Check response length */
    if( ( (sensbResLen != RFAL_NFCB_SENSB_RES_LEN) && (sensbResLen != RFAL_NFCB_SENSB_RES_EXT_LEN) ) )
    {
        return ERR_PROTO;
    }
    
    /* Check SENSB_RES and Protocol Type   Digital 1.1 7.6.2.19 */
    if( ((sensbRes->protInfo.FsciProType & RFAL_NFCB_SENSB_RES_PROT_TYPE_RFU) != 0U) || (sensbRes->cmd != (uint8_t)RFAL_NFCB_CMD_SENSB_RES) )
    {
        return ERR_PROTO;
    }
    return ERR_NONE;
}

/*
******************************************************************************
* GLOBAL FUNCTIONS
******************************************************************************
*/

/*******************************************************************************/
ReturnCode rfalNfcbPollerInitialize( void )
{
    ReturnCode ret;
    
    EXIT_ON_ERR( ret, rfalSetMode( RFAL_MODE_POLL_NFCB, RFAL_BR_106, RFAL_BR_106 ) );
    rfalSetErrorHandling( RFAL_ERRORHANDLING_NFC );
    
    rfalSetGT( RFAL_GT_NFCB );
    rfalSetFDTListen( RFAL_FDT_LISTEN_NFCB_POLLER );
    rfalSetFDTPoll( RFAL_FDT_POLL_NFCB_POLLER );
    
    gRfalNfcb.AFI    = RFAL_NFCB_AFI;
    gRfalNfcb.PARAM  = RFAL_NFCB_PARAM;
    
    return ERR_NONE;
}

/*******************************************************************************/
ReturnCode rfalNfcbPollerInitializeWithParams( uint8_t AFI, uint8_t PARAM )
{
    ReturnCode ret;
        
    EXIT_ON_ERR( ret, rfalNfcbPollerInitialize() );
    
    gRfalNfcb.AFI   = AFI;
    gRfalNfcb.PARAM = (PARAM & RFAL_NFCB_SENSB_REQ_PARAM);
    
    return ERR_NONE;
}


/*******************************************************************************/
ReturnCode rfalNfcbPollerCheckPresence( rfalNfcbSensCmd cmd, rfalNfcbSlots slots, rfalNfcbSensbRes *sensbRes, uint8_t *sensbResLen )
{
    uint16_t         rxLen;
    ReturnCode       ret;
    rfalNfcbSensbReq sensbReq;
    

    /* Check if the command requested and given the slot number are valid */
    if( ((RFAL_NFCB_SENS_CMD_SENSB_REQ != cmd) && (RFAL_NFCB_SENS_CMD_ALLB_REQ != cmd)) ||
        (slots > RFAL_NFCB_SLOT_NUM_16) || (sensbRes == NULL) || (sensbResLen == NULL)    )
    {
        return ERR_PARAM;
    }
    
    *sensbResLen = 0;
    ST_MEMSET(sensbRes, 0x00, sizeof(rfalNfcbSensbRes) );
    
    /* Compute SENSB_REQ */
    sensbReq.cmd   = RFAL_NFCB_CMD_SENSB_REQ;
    sensbReq.AFI   = gRfalNfcb.AFI;
    sensbReq.PARAM = (((uint8_t)gRfalNfcb.PARAM & RFAL_NFCB_SENSB_REQ_PARAM) | (uint8_t)cmd | (uint8_t)slots);
    
    /* Send SENSB_REQ and disable AGC to detect collisions */
    ret = rfalTransceiveBlockingTxRx( (uint8_t*)&sensbReq, sizeof(rfalNfcbSensbReq), (uint8_t*)sensbRes, sizeof(rfalNfcbSensbRes), &rxLen, RFAL_TXRX_FLAGS_DEFAULT, RFAL_NFCB_FWTSENSB );
    
    *sensbResLen = (uint8_t)rxLen;
    
    /*  Check if a transmission error was detected */
    if( (ret == ERR_CRC) || (ret == ERR_FRAMING) )
    {
        /* Invalidate received frame as an error was detected (CollisionResolution checks if valid) */
        *sensbResLen = 0;
        return ERR_NONE;
    }
    
    if( ret == ERR_NONE )
    {
        return rfalNfcbCheckSensbRes( sensbRes, *sensbResLen );
    }
    
    return ret;
}


/*******************************************************************************/
ReturnCode rfalNfcbPollerSleep( const uint8_t* nfcid0 )
{
    uint16_t        rxLen;
    ReturnCode      ret;
    rfalNfcbSlpbReq slpbReq;
    rfalNfcbSlpbRes slpbRes;
    
    if( nfcid0 == NULL )
    {
        return ERR_PARAM;
    }
    
    /* Compute SLPB_REQ */
    slpbReq.cmd = RFAL_NFCB_CMD_SLPB_REQ;
    ST_MEMCPY( slpbReq.nfcid0, nfcid0, RFAL_NFCB_NFCID0_LEN );
    
    EXIT_ON_ERR( ret, rfalTransceiveBlockingTxRx( (uint8_t*)&slpbReq, sizeof(rfalNfcbSlpbReq), (uint8_t*)&slpbRes, sizeof(rfalNfcbSlpbRes), &rxLen, RFAL_TXRX_FLAGS_DEFAULT, RFAL_NFCB_ACTIVATION_FWT ));
    
    /* Check SLPB_RES */
    if( (rxLen != sizeof(rfalNfcbSlpbRes)) || (slpbRes.cmd != (uint8_t)RFAL_NFCB_CMD_SLPB_RES) )
    {
        return ERR_PROTO;
    }
    return ERR_NONE;
}


/*******************************************************************************/
ReturnCode rfalNfcbPollerSlotMarker( uint8_t slotCode, rfalNfcbSensbRes *sensbRes, uint8_t *sensbResLen )
{
    ReturnCode         ret;
    rfalNfcbSlotMarker slotMarker;
    uint16_t           rxLen;
    
    /* Check parameters */
    if( (sensbRes == NULL) || (sensbResLen == NULL)    || 
        (slotCode < RFAL_NFCB_SLOTMARKER_SLOTCODE_MIN) || 
        (slotCode > RFAL_NFCB_SLOTMARKER_SLOTCODE_MAX)   )
    {
        return ERR_PARAM;
    }
    /* Compose and send SLOT_MARKER with disabled AGC to detect collisions  */
    slotMarker.APn = ((slotCode << RFAL_NFCB_SLOT_MARKER_SC_SHIFT) | (uint8_t)RFAL_NFCB_CMD_SENSB_REQ);
    
    ret = rfalTransceiveBlockingTxRx( (uint8_t*)&slotMarker, sizeof(rfalNfcbSlotMarker), (uint8_t*)sensbRes, sizeof(rfalNfcbSensbRes), &rxLen, RFAL_TXRX_FLAGS_DEFAULT, RFAL_NFCB_ACTIVATION_FWT );
    
    *sensbResLen = (uint8_t)rxLen;
    
    /* Check if a transmission error was detected */
    if( (ret == ERR_CRC) || (ret == ERR_FRAMING) )
    {
        return ERR_RF_COLLISION;
    }
    
    if( ret == ERR_NONE )
    {
        return rfalNfcbCheckSensbRes( sensbRes, *sensbResLen );
    }
    
    return ret;
}


ReturnCode rfalNfcbPollerTechnologyDetection( rfalComplianceMode compMode, rfalNfcbSensbRes *sensbRes, uint8_t *sensbResLen )
{
    NO_WARNING(compMode);
    
    return rfalNfcbPollerCheckPresence( RFAL_NFCB_SENS_CMD_SENSB_REQ, RFAL_NFCB_SLOT_NUM_1, sensbRes, sensbResLen );
}


/*******************************************************************************/
ReturnCode rfalNfcbPollerCollisionResolution( rfalComplianceMode compMode, uint8_t devLimit, rfalNfcbListenDevice *nfcbDevList, uint8_t *devCnt )
{
    bool colPending; /* dummy */
    return rfalNfcbPollerSlottedCollisionResolution( compMode, devLimit, RFAL_NFCB_SLOT_NUM_1, RFAL_NFCB_SLOT_NUM_16, nfcbDevList, devCnt, &colPending );
}


/*******************************************************************************/
ReturnCode rfalNfcbPollerSlottedCollisionResolution( rfalComplianceMode compMode, uint8_t devLimit, rfalNfcbSlots initSlots, rfalNfcbSlots endSlots, rfalNfcbListenDevice *nfcbDevList, uint8_t *devCnt, bool *colPending )
{
    ReturnCode    ret;
        uint8_t slotsNum;
        uint8_t       slotCode;
        uint8_t       curDevCnt;
        
        
        /* Check parameters. In ISO | Activity 1.0 mode the initial slots must be 1 as continuation of Technology Detection */
        if( (nfcbDevList == NULL) || (devCnt == NULL)  || (colPending == NULL) || (initSlots > RFAL_NFCB_SLOT_NUM_16) || 
            (endSlots > RFAL_NFCB_SLOT_NUM_16) || ((compMode == RFAL_COMPLIANCE_MODE_ISO) && (initSlots != RFAL_NFCB_SLOT_NUM_1)) )
        {
            return ERR_PARAM;
        }
        
        /* Initialise as no error in case Activity 1.0 where the previous SENSB_RES from technology detection should be used */
        ret         = ERR_NONE;
        *devCnt     = 0;
        curDevCnt   = 0;
        *colPending = false;
           
        
        /* Send ALLB_REQ   Activity 1.1   9.3.5.2 and 9.3.5.3  (Symbol 1 and 2) */
        if( compMode != RFAL_COMPLIANCE_MODE_ISO )
        {
           ret =  rfalNfcbPollerCheckPresence( RFAL_NFCB_SENS_CMD_ALLB_REQ, initSlots, &nfcbDevList->sensbRes, &nfcbDevList->sensbResLen );
           if( (ret != ERR_NONE) && (initSlots == RFAL_NFCB_SLOT_NUM_1) )
           {
               return ret;
           }
        }

        
        /* Check if there was a transmission error on WUPB  EMVCo 2.6  9.3.3.1 */
        if( (compMode == RFAL_COMPLIANCE_MODE_EMV) && (nfcbDevList->sensbResLen == 0U) )
        {
            return ERR_FRAMING;
        }
        
        for( slotsNum = (uint8_t)initSlots; slotsNum <= (uint8_t)endSlots; slotsNum++ )
        {
            do {
                /* Activity 1.1  9.3.5.23  -  Symbol 22 */
                if( (compMode == RFAL_COMPLIANCE_MODE_NFC) && (curDevCnt != 0U) )
                {
                    rfalNfcbPollerSleep( nfcbDevList[((*devCnt) - (uint8_t)1U)].sensbRes.nfcid0 );
                    nfcbDevList[((*devCnt) - (uint8_t)1U)].isSleep = true;
                }
                
                /* Send SENSB_REQ with number of slots if not the first Activity 1.1  9.3.5.24  -  Symbol 23 */
                if( (slotsNum != (uint8_t)initSlots) || *colPending )
                {
                    /* PRQA S 4342 1 # MISRA 10.5 - Layout of rfalNfcbSlots and above loop guarantee that no invalid enum values are created. */
                    ret = rfalNfcbPollerCheckPresence( RFAL_NFCB_SENS_CMD_SENSB_REQ, (rfalNfcbSlots)slotsNum, &nfcbDevList[*devCnt].sensbRes, &nfcbDevList[*devCnt].sensbResLen );
                }
                
                /* Activity 1.1  9.3.5.6  -  Symbol 5 */
                slotCode    = 0;
                curDevCnt   = 0;
                *colPending = false;

                do{
                    /* Activity 1.1  9.3.5.26  -  Symbol 25 */
                    if( slotCode != 0U )
                    {
                        ret = rfalNfcbPollerSlotMarker( slotCode, &nfcbDevList[*devCnt].sensbRes, &nfcbDevList[*devCnt].sensbResLen );
                    }
                    
                    /* Activity 1.1  9.3.5.7 and 9.3.5.8  -  Symbol 6 */
                    if( ret != ERR_TIMEOUT )
                    {
                        /* Activity 1.1  9.3.5.8  -  Symbol 7 */
                        if( (rfalNfcbCheckSensbRes( &nfcbDevList[*devCnt].sensbRes, nfcbDevList[*devCnt].sensbResLen) == ERR_NONE) && (ret == ERR_NONE) )
                        {
                            nfcbDevList[*devCnt].isSleep = false;
                            
                            if( compMode == RFAL_COMPLIANCE_MODE_EMV )
                            {
                                (*devCnt)++;
                                return ret;
                            }
                            else if( compMode == RFAL_COMPLIANCE_MODE_ISO )
                            {
                                /* Activity 1.0  9.3.5.8  -  Symbol 7 */
                                (*devCnt)++;
                                curDevCnt++;
                                
                                /* Activity 1.0  9.3.5.10  -  Symbol 9 */
                                if( (*devCnt >= devLimit) || (slotsNum == (uint8_t)RFAL_NFCB_SLOT_NUM_1) )
                                {
                                    return ret;
                                }

                                /* Activity 1.0  9.3.5.11  -  Symbol 10 */
                                rfalNfcbPollerSleep( nfcbDevList[*devCnt-1U].sensbRes.nfcid0 );
                                nfcbDevList[*devCnt-1U].isSleep =  true;
                            }
                            else if( compMode == RFAL_COMPLIANCE_MODE_NFC )
                            {
                                /* Activity 1.1  9.3.5.10 and 9.3.5.11  -  Symbol 9 and Symbol 11*/
                                if(curDevCnt != 0U)
                                {
                                    rfalNfcbPollerSleep( nfcbDevList[(*devCnt) - (uint8_t)1U].sensbRes.nfcid0 );
                                    nfcbDevList[(*devCnt) - (uint8_t)1U].isSleep = true;
                                }
                                
                                /* Activity 1.1  9.3.5.12  -  Symbol 11 */
                                (*devCnt)++;
                                curDevCnt++;
                                
                                /* Activity 1.1  9.3.5.6  -  Symbol 13 */
                                if( (*devCnt >= devLimit) || (slotsNum == (uint8_t)RFAL_NFCB_SLOT_NUM_1) )
                                {
                                    return ret;
                                }
                            }
                            else
                            {
                                /* MISRA 15.7 - Empty else */
                            }
                        }
                        else
                        {
                            /* If deviceLimit is set to 0 the NFC Forum Device is configured to perform collision detection only  Activity 1.0 and 1.1  9.3.5.5  - Symbol 4 */
                            if( (devLimit == 0U) && (slotsNum == (uint8_t)RFAL_NFCB_SLOT_NUM_1) )
                            {
                                return ERR_RF_COLLISION;
                            }
                            
                            /* Activity 1.1  9.3.5.9  -  Symbol 8 */
                            *colPending = true;
                        }
                    }
                    
                    /* Activity 1.1  9.3.5.15  -  Symbol 14 */
                    slotCode++;
                }
                while( slotCode < rfalNfcbNI2NumberOfSlots(slotsNum) );
                
                /* Activity 1.1  9.3.5.17  -  Symbol 16 */
                if( !(*colPending) )
                {
                    return ERR_NONE;
                }
            
            /* Activity 1.1  9.3.5.18  -  Symbol 17 */
            } while (curDevCnt != 0U);     /* If a collision is detected and card(s) were found on this loop keep the same number of available slots */
        }
        
        return ERR_NONE;
}


/*******************************************************************************/
uint32_t rfalNfcbTR2ToFDT( uint8_t tr2Code )
{
    /*******************************************************************************/
    /* MISRA 8.9 An object should be defined at block scope if its identifier only appears in a single function */
    /*! TR2 Table according to Digital 1.1 Table 33 */
    const uint16_t rfalNfcbTr2Table[] = { 1792, 3328, 5376, 9472 };
    /*******************************************************************************/

    return rfalNfcbTr2Table[ (tr2Code & RFAL_NFCB_SENSB_RES_PROTO_TR2_MASK) ];
}

#endif /* RFAL_FEATURE_NFCB */
