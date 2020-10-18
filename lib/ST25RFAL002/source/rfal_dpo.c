
/******************************************************************************
  * @attention
  *
  * <h2><center>&copy; COPYRIGHT 2020 STMicroelectronics</center></h2>
  *
  * Licensed under ST MYLIBERTY SOFTWARE LICENSE AGREEMENT (the "License");
  * You may not use this file except in compliance with the License.
  * You may obtain a copy of the License at:
  *
  *        http://www.st.com/myliberty
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
 *      $Revision: $
 *      LANGUAGE:  ISO C99
 */
 
/*! \file rfal_dpo.c
 *
 *  \author Martin Zechleitner
 *
 *  \brief Functions to manage and set dynamic power settings.
 *  
 */

/*
 ******************************************************************************
 * INCLUDES
 ******************************************************************************
 */
#include "rfal_dpoTbl.h"
#include "rfal_dpo.h"
#include "platform.h"
#include "rfal_rf.h"
#include "rfal_chip.h"
#include "rfal_analogConfig.h"
#include "utils.h"


/*
 ******************************************************************************
 * ENABLE SWITCH
 ******************************************************************************
 */

#ifndef RFAL_FEATURE_DPO
    #define RFAL_FEATURE_DPO   false    /* Dynamic Power Module configuration missing. Disabled by default */
#endif

#if RFAL_FEATURE_DPO


/*
 ******************************************************************************
 * DEFINES
 ******************************************************************************
 */
#define RFAL_DPO_ANALOGCONFIG_SHIFT       13U
#define RFAL_DPO_ANALOGCONFIG_MASK        0x6000U
    
/*
 ******************************************************************************
 * LOCAL DATA TYPES
 ******************************************************************************
 */

static bool                gRfalDpoIsEnabled = false;
static uint8_t*            gRfalCurrentDpo;
static uint8_t             gRfalDpoTableEntries;
static uint8_t             gRfalDpo[RFAL_DPO_TABLE_SIZE_MAX];
static uint8_t             gRfalDpoTableEntry;
static rfalDpoMeasureFunc  gRfalDpoMeasureCallback = NULL;

/*
 ******************************************************************************
 * GLOBAL FUNCTIONS
 ******************************************************************************
 */
void rfalDpoInitialize( void )
{
    /* Use the default Dynamic Power values */
    gRfalCurrentDpo = (uint8_t*) rfalDpoDefaultSettings;
    gRfalDpoTableEntries = (sizeof(rfalDpoDefaultSettings) / RFAL_DPO_TABLE_PARAMETER);
    
    ST_MEMCPY( gRfalDpo, gRfalCurrentDpo, sizeof(rfalDpoDefaultSettings) );
    
    /* by default use amplitude measurement */
    gRfalDpoMeasureCallback = rfalChipMeasureAmplitude;
    
    /* by default DPO is disabled */
    gRfalDpoIsEnabled = false;
    
    gRfalDpoTableEntry = 0;
}

void rfalDpoSetMeasureCallback( rfalDpoMeasureFunc pMeasureFunc )
{
  gRfalDpoMeasureCallback = pMeasureFunc;
}

/*******************************************************************************/
ReturnCode rfalDpoTableWrite( rfalDpoEntry* powerTbl, uint8_t powerTblEntries )
{
    uint8_t entry = 0;
    
    /* check if the table size parameter is too big */
    if( (powerTblEntries * RFAL_DPO_TABLE_PARAMETER) > RFAL_DPO_TABLE_SIZE_MAX)
    {
        return ERR_NOMEM;
    }
    
    /* check if the first increase entry is 0xFF */
    if( (powerTblEntries == 0) || (powerTbl == NULL) )
    {
        return ERR_PARAM;
    }
                
    /* check if the entries of the dynamic power table are valid */
    for (entry = 0; entry < powerTblEntries; entry++)
    {
        if(powerTbl[entry].inc < powerTbl[entry].dec)
        {
            return ERR_PARAM;
        }
    }
    
    /* copy the data set  */
    ST_MEMCPY( gRfalDpo, powerTbl, (powerTblEntries * RFAL_DPO_TABLE_PARAMETER) );
    gRfalCurrentDpo = gRfalDpo;
    gRfalDpoTableEntries = powerTblEntries;
    
    if(gRfalDpoTableEntry > powerTblEntries)
    {
      /* is always greater then zero, otherwise we already returned ERR_PARAM */
      gRfalDpoTableEntry = (powerTblEntries - 1); 
    }
    
    return ERR_NONE;
}

/*******************************************************************************/
ReturnCode rfalDpoTableRead( rfalDpoEntry* tblBuf, uint8_t tblBufEntries, uint8_t* tableEntries )
{
    /* wrong request */
    if( (tblBuf == NULL) || (tblBufEntries < gRfalDpoTableEntries) || (tableEntries == NULL) )
    {
        return ERR_PARAM;
    }
        
    /* Copy the whole Table to the given buffer */
    ST_MEMCPY( tblBuf, gRfalCurrentDpo, (tblBufEntries * RFAL_DPO_TABLE_PARAMETER) );
    *tableEntries = gRfalDpoTableEntries;
    
    return ERR_NONE;
}

/*******************************************************************************/
ReturnCode rfalDpoAdjust( void )
{
    uint8_t     refValue = 0;
    uint16_t    modeID;
    rfalBitRate br;
    rfalDpoEntry* dpoTable = (rfalDpoEntry*) gRfalCurrentDpo;    
    
    /* Check if the Power Adjustment is disabled and                  *
     * if the callback to the measurement method is properly set      */
    if( (gRfalCurrentDpo == NULL) || (!gRfalDpoIsEnabled) || (gRfalDpoMeasureCallback == NULL) )
    {
        return ERR_PARAM;
    }
    
    /* Ensure that the current mode is Passive Poller */
    if( !rfalIsModePassivePoll( rfalGetMode() ) )
    {
        return ERR_WRONG_STATE;
    }
      
    /* Ensure a proper measure reference value */
    if( ERR_NONE != gRfalDpoMeasureCallback( &refValue ) )
    {
        return ERR_IO;
    }

    
    if( refValue >= dpoTable[gRfalDpoTableEntry].inc )
    { /* Increase the output power */
        /* the top of the table represents the highest amplitude value*/
        if( gRfalDpoTableEntry == 0 )
        {
            /* maximum driver value has been reached */
        }
        else
        {
            /* go up in the table to decrease the driver resistance */
            gRfalDpoTableEntry--;
        }
    }
    else if(refValue <= dpoTable[gRfalDpoTableEntry].dec)
    { /* decrease the output power */
        /* The bottom is the highest possible value */
        if( (gRfalDpoTableEntry + 1) >= gRfalDpoTableEntries)
        {
            /* minimum driver value has been reached */
        }
        else
        {
            /* go down in the table to increase the driver resistance */
            gRfalDpoTableEntry++;
        }
    }
    else
    {
        /* Fall through to always write dpo and its associated analog configs */
    }
    
    /* Get the new value for RFO resistance form the table and apply the new RFO resistance setting */ 
    rfalChipSetRFO( dpoTable[gRfalDpoTableEntry].rfoRes );
    
    /* Apply the DPO Analog Config according to this treshold */
    /* Technology field is being extended for DPO: 2msb are used for treshold step (only 4 allowed) */
    rfalGetBitRate( &br, NULL );                                                                    /* Obtain current Tx bitrate       */
    modeID  = rfalAnalogConfigGenModeID( rfalGetMode(), br, RFAL_ANALOG_CONFIG_DPO );               /* Generate Analog Config mode ID  */
    modeID |= ((gRfalDpoTableEntry << RFAL_DPO_ANALOGCONFIG_SHIFT) & RFAL_DPO_ANALOGCONFIG_MASK);   /* Add DPO treshold step|level     */
    rfalSetAnalogConfig( modeID );                                                                  /* Apply DPO Analog Config         */
    
    return ERR_NONE;
}

/*******************************************************************************/
rfalDpoEntry* rfalDpoGetCurrentTableEntry( void )
{
    rfalDpoEntry* dpoTable = (rfalDpoEntry*) gRfalCurrentDpo; 
    return &dpoTable[gRfalDpoTableEntry];
}

/*******************************************************************************/
void rfalDpoSetEnabled( bool enable )
{
    gRfalDpoIsEnabled = enable;
}

/*******************************************************************************/
bool rfalDpoIsEnabled( void )
{
    return gRfalDpoIsEnabled;
}

#endif /* RFAL_FEATURE_DPO */
