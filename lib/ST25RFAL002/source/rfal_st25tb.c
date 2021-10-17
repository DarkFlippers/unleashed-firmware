
/******************************************************************************
  * \attention
  *
  * <h2><center>&copy; COPYRIGHT 2020 STMicroelectronics</center></h2>
  *
  * Licensed under ST MYLIBERTY SOFTWARE LICENSE AGREEMENT (the "License");
  * You may not use this file except in compliance with the License.
  * You may obtain a copy of the License at:
  *
  *      www.st.com/myliberty
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

/*! \file rfal_st25tb.c
 *
 *  \author Gustavo Patricio
 *
 *  \brief Implementation of ST25TB interface 
 *
 */

/*
 ******************************************************************************
 * INCLUDES
 ******************************************************************************
 */
#include "rfal_st25tb.h"
#include "utils.h"

/*
 ******************************************************************************
 * ENABLE SWITCH
 ******************************************************************************
 */
#ifndef RFAL_FEATURE_ST25TB
    #define RFAL_FEATURE_ST25TB   false    /* ST25TB module configuration missing. Disabled by default */
#endif

#if RFAL_FEATURE_ST25TB

/*
 ******************************************************************************
 * GLOBAL DEFINES
 ******************************************************************************
 */

#define RFAL_ST25TB_CMD_LEN          1U                                 /*!< ST25TB length of a command                       */
#define RFAL_ST25TB_SLOTS            16U                                /*!< ST25TB number of slots                           */
#define RFAL_ST25TB_SLOTNUM_MASK     0x0FU                              /*!< ST25TB Slot Number bit mask on SlotMarker        */
#define RFAL_ST25TB_SLOTNUM_SHIFT    4U                                 /*!< ST25TB Slot Number shift on SlotMarker           */

#define RFAL_ST25TB_INITIATE_CMD1    0x06U                              /*!< ST25TB Initiate command byte1                    */
#define RFAL_ST25TB_INITIATE_CMD2    0x00U                              /*!< ST25TB Initiate command byte2                    */
#define RFAL_ST25TB_PCALL_CMD1       0x06U                              /*!< ST25TB Pcall16 command byte1                     */
#define RFAL_ST25TB_PCALL_CMD2       0x04U                              /*!< ST25TB Pcall16 command byte2                     */
#define RFAL_ST25TB_SELECT_CMD       0x0EU                              /*!< ST25TB Select command                            */
#define RFAL_ST25TB_GET_UID_CMD      0x0BU                              /*!< ST25TB Get UID command                           */
#define RFAL_ST25TB_COMPLETION_CMD   0x0FU                              /*!< ST25TB Completion command                        */
#define RFAL_ST25TB_RESET_INV_CMD    0x0CU                              /*!< ST25TB Reset to Inventory command                */
#define RFAL_ST25TB_READ_BLOCK_CMD   0x08U                              /*!< ST25TB Read Block command                        */
#define RFAL_ST25TB_WRITE_BLOCK_CMD  0x09U                              /*!< ST25TB Write Block command                       */


#define RFAL_ST25TB_T0               2157U                              /*!< ST25TB t0  159 us   ST25TB RF characteristics    */
#define RFAL_ST25TB_T1               2048U                              /*!< ST25TB t1  151 us   ST25TB RF characteristics    */

#define RFAL_ST25TB_FWT             (RFAL_ST25TB_T0 + RFAL_ST25TB_T1)   /*!< ST25TB FWT  = T0 + T1                            */
#define RFAL_ST25TB_TW              rfalConvMsTo1fc(7U)                 /*!< ST25TB TW : Programming time for write max 7ms   */


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

/*! Initiate Request */
typedef struct
{
    uint8_t  cmd1;                       /*!< Initiate Request cmd1: 0x06 */
    uint8_t  cmd2;                       /*!< Initiate Request cmd2: 0x00 */
} rfalSt25tbInitiateReq;

/*! Pcall16 Request */
typedef struct
{
    uint8_t  cmd1;                       /*!< Pcal16 Request cmd1: 0x06   */
    uint8_t  cmd2;                       /*!< Pcal16 Request cmd2: 0x04   */
} rfalSt25tbPcallReq;


/*! Select Request */
typedef struct
{
    uint8_t  cmd;                       /*!< Select Request cmd: 0x0E     */
    uint8_t  chipId;                    /*!< Chip ID                      */
} rfalSt25tbSelectReq;

/*! Read Block Request */
typedef struct
{
    uint8_t  cmd;                       /*!< Select Request cmd: 0x08     */
    uint8_t  address;                   /*!< Block address                */
} rfalSt25tbReadBlockReq;

/*! Write Block Request */
typedef struct
{
    uint8_t              cmd;           /*!< Select Request cmd: 0x09     */
    uint8_t              address;       /*!< Block address                */
    rfalSt25tbBlock data;               /*!< Block Data                   */
} rfalSt25tbWriteBlockReq;


/*
******************************************************************************
* LOCAL FUNCTION PROTOTYPES
******************************************************************************
*/
/*! 
 *****************************************************************************
 * \brief  ST25TB Poller Do Collision Resolution
 *  
 * This method performs ST25TB Collision resolution loop for each slot
 *   
 * \param[in]  devLimit      : device limit value, and size st25tbDevList
 * \param[out] st25tbDevList : ST35TB listener device info
 * \param[out] devCnt        : Devices found counter
 * 
 * \return colPending         : true if a collision was detected
 *****************************************************************************
 */
static bool rfalSt25tbPollerDoCollisionResolution( uint8_t devLimit, rfalSt25tbListenDevice *st25tbDevList, uint8_t *devCnt );

/*
******************************************************************************
* LOCAL FUNCTION PROTOTYPES
******************************************************************************
*/


static bool rfalSt25tbPollerDoCollisionResolution( uint8_t devLimit, rfalSt25tbListenDevice *st25tbDevList, uint8_t *devCnt )
{
    uint8_t    i;
    uint8_t    chipId;
    ReturnCode ret;
    bool col;

    col = false;
    
    for(i = 0; i < RFAL_ST25TB_SLOTS; i++)
    {
        platformDelay(1);  /* Wait t2: Answer to new request delay  */
        
        if( i==0U )
        {
            /* Step 2: Send Pcall16 */
            ret = rfalSt25tbPollerPcall( &chipId );
        }
        else
        {
            /* Step 3-17: Send Pcall16 */
            ret = rfalSt25tbPollerSlotMarker( i, &chipId );
        }
        
        if( ret == ERR_NONE )
        {
            /* Found another device */
            st25tbDevList[*devCnt].chipID       = chipId;
            st25tbDevList[*devCnt].isDeselected = false;
            
            /* Select Device, retrieve its UID  */
            ret = rfalSt25tbPollerSelect( chipId );

            /* By Selecting this device, the previous gets Deselected */
            if( (*devCnt) > 0U )
            {
                st25tbDevList[(*devCnt)-1U].isDeselected = true;
            }

            if( ERR_NONE == ret )
            {
                rfalSt25tbPollerGetUID( &st25tbDevList[*devCnt].UID );
            }

            if( ERR_NONE == ret )
            {
                (*devCnt)++;
            }
        }
        else if( (ret == ERR_CRC) || (ret == ERR_FRAMING) )
        {
            col = true;
        }
        else
        {
            /* MISRA 15.7 - Empty else */
        }
        
        if( *devCnt >= devLimit )
        {
            break;
        }
    }
    return col;
}


/*
******************************************************************************
* LOCAL VARIABLES
******************************************************************************
*/

/*
******************************************************************************
* GLOBAL FUNCTIONS
******************************************************************************
*/

/*******************************************************************************/
ReturnCode rfalSt25tbPollerInitialize( void )
{
    return rfalNfcbPollerInitialize();
}


/*******************************************************************************/
ReturnCode rfalSt25tbPollerCheckPresence( uint8_t *chipId )
{
    ReturnCode ret;
    uint8_t    chipIdRes;

    chipIdRes = 0x00;
   
    /* Send Initiate Request */
    ret = rfalSt25tbPollerInitiate( &chipIdRes );
    
    /*  Check if a transmission error was detected */
    if( (ret == ERR_CRC) || (ret == ERR_FRAMING) )
    {
        return ERR_NONE;
    }
    
    /* Copy chip ID if requested */
    if( chipId != NULL )
    {
        *chipId = chipIdRes;
    }
    
    return ret;
}


/*******************************************************************************/
ReturnCode rfalSt25tbPollerInitiate( uint8_t *chipId )
{
    ReturnCode            ret;
    uint16_t              rxLen;
    rfalSt25tbInitiateReq initiateReq;
    uint8_t               rxBuf[RFAL_ST25TB_CHIP_ID_LEN + RFAL_ST25TB_CRC_LEN]; /* In case we receive less data that CRC, RF layer will not remove the CRC from buffer */
    
    /* Compute Initiate Request */
    initiateReq.cmd1   = RFAL_ST25TB_INITIATE_CMD1;
    initiateReq.cmd2   = RFAL_ST25TB_INITIATE_CMD2;
    
    /* Send Initiate Request */
    ret = rfalTransceiveBlockingTxRx( (uint8_t*)&initiateReq, sizeof(rfalSt25tbInitiateReq), (uint8_t*)rxBuf, sizeof(rxBuf), &rxLen, RFAL_TXRX_FLAGS_DEFAULT, RFAL_ST25TB_FWT );
    
    /* Check for valid Select Response   */
    if( (ret == ERR_NONE) && (rxLen != RFAL_ST25TB_CHIP_ID_LEN) )
    {
        return ERR_PROTO;
    }
    
    /* Copy chip ID if requested */
    if( chipId != NULL )
    {
        *chipId = *rxBuf;
    }
    
    return ret;
}


/*******************************************************************************/
ReturnCode rfalSt25tbPollerPcall( uint8_t *chipId )
{
    ReturnCode         ret;
    uint16_t           rxLen;
    rfalSt25tbPcallReq pcallReq;

    /* Compute Pcal16 Request */
    pcallReq.cmd1   = RFAL_ST25TB_PCALL_CMD1;
    pcallReq.cmd2   = RFAL_ST25TB_PCALL_CMD2;
    
    /* Send Pcal16 Request */
    ret = rfalTransceiveBlockingTxRx( (uint8_t*)&pcallReq, sizeof(rfalSt25tbPcallReq), (uint8_t*)chipId, RFAL_ST25TB_CHIP_ID_LEN, &rxLen, RFAL_TXRX_FLAGS_DEFAULT, RFAL_ST25TB_FWT );
    
    /* Check for valid Select Response   */
    if( (ret == ERR_NONE) && (rxLen != RFAL_ST25TB_CHIP_ID_LEN) )
    {
        return ERR_PROTO;
    }
    
    return ret;
}


/*******************************************************************************/
ReturnCode rfalSt25tbPollerSlotMarker( uint8_t slotNum, uint8_t *chipIdRes )
{
    ReturnCode ret;
    uint16_t   rxLen;
    uint8_t    slotMarker;

    if( (slotNum == 0U) || (slotNum > 15U) )
    {
        return ERR_PARAM;
    }
    
    /* Compute SlotMarker */
    slotMarker = ( ((slotNum & RFAL_ST25TB_SLOTNUM_MASK) << RFAL_ST25TB_SLOTNUM_SHIFT) | RFAL_ST25TB_PCALL_CMD1 );
    
    
    /* Send SlotMarker */
    ret = rfalTransceiveBlockingTxRx( (uint8_t*)&slotMarker, RFAL_ST25TB_CMD_LEN, (uint8_t*)chipIdRes, RFAL_ST25TB_CHIP_ID_LEN, &rxLen, RFAL_TXRX_FLAGS_DEFAULT, RFAL_ST25TB_FWT );
    
    /* Check for valid ChipID Response   */
    if( (ret == ERR_NONE) && (rxLen != RFAL_ST25TB_CHIP_ID_LEN) )
    {
        return ERR_PROTO;
    }
    
    return ret;
}


/*******************************************************************************/
ReturnCode rfalSt25tbPollerSelect( uint8_t chipId )
{
    ReturnCode          ret;
    uint16_t            rxLen;    
    rfalSt25tbSelectReq selectReq;
    uint8_t             chipIdRes;

    /* Compute Select Request */
    selectReq.cmd    = RFAL_ST25TB_SELECT_CMD;
    selectReq.chipId = chipId;
    
    /* Send Select Request */
    ret = rfalTransceiveBlockingTxRx( (uint8_t*)&selectReq, sizeof(rfalSt25tbSelectReq), (uint8_t*)&chipIdRes, RFAL_ST25TB_CHIP_ID_LEN, &rxLen, RFAL_TXRX_FLAGS_DEFAULT, RFAL_ST25TB_FWT );
    
    /* Check for valid Select Response   */
    if( (ret == ERR_NONE) && ((rxLen != RFAL_ST25TB_CHIP_ID_LEN) || (chipIdRes != chipId)) )
    {
        return ERR_PROTO;
    }
    
    return ret;
}


/*******************************************************************************/
ReturnCode rfalSt25tbPollerGetUID( rfalSt25tbUID *UID )
{
    ReturnCode ret;
    uint16_t   rxLen;
    uint8_t    getUidReq;
    

    /* Compute Get UID Request */
    getUidReq = RFAL_ST25TB_GET_UID_CMD;
    
    /* Send Select Request */
    ret = rfalTransceiveBlockingTxRx( (uint8_t*)&getUidReq, RFAL_ST25TB_CMD_LEN, (uint8_t*)UID, sizeof(rfalSt25tbUID), &rxLen, RFAL_TXRX_FLAGS_DEFAULT, RFAL_ST25TB_FWT );
    
    /* Check for valid UID Response */
    if( (ret == ERR_NONE) && (rxLen != RFAL_ST25TB_UID_LEN) )
    {
        return ERR_PROTO;
    }
    
    return ret;
}


/*******************************************************************************/
ReturnCode rfalSt25tbPollerCollisionResolution( uint8_t devLimit, rfalSt25tbListenDevice *st25tbDevList, uint8_t *devCnt )
{
    
    uint8_t    chipId;
    ReturnCode ret;
    bool       detected;  /* collision or device was detected */
    
    if( (st25tbDevList == NULL) || (devCnt == NULL) || (devLimit == 0U) )
    {
        return ERR_PARAM;
    }
    
    *devCnt = 0;
    
    /* Step 1: Send Initiate */
    ret = rfalSt25tbPollerInitiate( &chipId );
    if( ret == ERR_NONE )
    {
        /* If only 1 answer is detected */
        st25tbDevList[*devCnt].chipID       = chipId;
        st25tbDevList[*devCnt].isDeselected = false;
        
        /* Retrieve its UID and keep it Selected*/
        ret = rfalSt25tbPollerSelect( chipId );
        
        if( ERR_NONE == ret )
        {
            ret = rfalSt25tbPollerGetUID( &st25tbDevList[*devCnt].UID );
        }
        
        if( ERR_NONE == ret )
        {
            (*devCnt)++;
        }
    }
    /* Always proceed to Pcall16 anticollision as phase differences of tags can lead to no tag recognized, even if there is one */
    if( *devCnt < devLimit )
    {
        /* Multiple device responses */
        do
        {
            detected = rfalSt25tbPollerDoCollisionResolution( devLimit, st25tbDevList, devCnt );
        }
        while( (detected == true) && (*devCnt < devLimit) );
    }

    return ERR_NONE;
}


/*******************************************************************************/
ReturnCode rfalSt25tbPollerReadBlock( uint8_t blockAddress, rfalSt25tbBlock *blockData  )
{
    ReturnCode             ret;
    uint16_t               rxLen;
    rfalSt25tbReadBlockReq readBlockReq;
    

    /* Compute Read Block Request */
    readBlockReq.cmd     = RFAL_ST25TB_READ_BLOCK_CMD;
    readBlockReq.address = blockAddress;
    
    /* Send Read Block Request */
    ret = rfalTransceiveBlockingTxRx( (uint8_t*)&readBlockReq, sizeof(rfalSt25tbReadBlockReq), (uint8_t*)blockData, sizeof(rfalSt25tbBlock), &rxLen, RFAL_TXRX_FLAGS_DEFAULT, RFAL_ST25TB_FWT );
    
    /* Check for valid UID Response */
    if( (ret == ERR_NONE) && (rxLen != RFAL_ST25TB_BLOCK_LEN) )
    {
        return ERR_PROTO;
    }
    
    return ret;
}


/*******************************************************************************/
ReturnCode rfalSt25tbPollerWriteBlock( uint8_t blockAddress, const rfalSt25tbBlock *blockData  )
{
    ReturnCode              ret;
    uint16_t                rxLen;
    rfalSt25tbWriteBlockReq writeBlockReq;
    rfalSt25tbBlock         tmpBlockData; 
    

    /* Compute Write Block Request */
    writeBlockReq.cmd     = RFAL_ST25TB_WRITE_BLOCK_CMD;
    writeBlockReq.address = blockAddress;
    ST_MEMCPY( &writeBlockReq.data, blockData, RFAL_ST25TB_BLOCK_LEN );
    
    /* Send Write Block Request */
    ret = rfalTransceiveBlockingTxRx( (uint8_t*)&writeBlockReq, sizeof(rfalSt25tbWriteBlockReq), tmpBlockData, RFAL_ST25TB_BLOCK_LEN, &rxLen, RFAL_TXRX_FLAGS_DEFAULT, (RFAL_ST25TB_FWT + RFAL_ST25TB_TW) );
    
    
    /* Check if there was any error besides timeout */
    if( ret != ERR_TIMEOUT )
    {
        /* Check if an unexpected answer was received */
        if( ret == ERR_NONE )
        {
            return ERR_PROTO;
        }
    
        /* Check whether a transmission error occurred */
        if( (ret != ERR_CRC) && (ret != ERR_FRAMING) && (ret != ERR_NOMEM) && (ret != ERR_RF_COLLISION) )
        {
            return ret;
        }
        
        /* If a transmission error occurred (maybe noise while commiting data) wait maximum programming time and verify data afterwards */
        rfalSetGT( (RFAL_ST25TB_FWT + RFAL_ST25TB_TW) );
        rfalFieldOnAndStartGT();
    }
    
    ret = rfalSt25tbPollerReadBlock(blockAddress, &tmpBlockData);
    if( ret == ERR_NONE )
    {
        if( ST_BYTECMP( &tmpBlockData, blockData, RFAL_ST25TB_BLOCK_LEN ) == 0 )
        {
            return ERR_NONE;
        }
        return ERR_PROTO;
    }
    return ret;
}


/*******************************************************************************/
ReturnCode rfalSt25tbPollerCompletion( void )
{
    uint8_t  completionReq;

    /* Compute Completion Request */
    completionReq = RFAL_ST25TB_COMPLETION_CMD;
    
    /* Send Completion Request, no response is expected */
    return rfalTransceiveBlockingTxRx( (uint8_t*)&completionReq, RFAL_ST25TB_CMD_LEN, NULL, 0, NULL, RFAL_TXRX_FLAGS_DEFAULT, RFAL_ST25TB_FWT );
}


/*******************************************************************************/
ReturnCode rfalSt25tbPollerResetToInventory( void )
{
    uint8_t resetInvReq;

    /* Compute Completion Request */
    resetInvReq = RFAL_ST25TB_RESET_INV_CMD;
    
    /* Send Completion Request, no response is expected */
    return rfalTransceiveBlockingTxRx( (uint8_t*)&resetInvReq, RFAL_ST25TB_CMD_LEN, NULL, 0, NULL, RFAL_TXRX_FLAGS_DEFAULT, RFAL_ST25TB_FWT );
}

#endif /* RFAL_FEATURE_ST25TB */
