
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

/*! \file rfal_nfcb.h
 *
 *  \author Gustavo Patricio
 *
 *  \brief Implementation of NFC-B (ISO14443B) helpers 
 *  
 *  It provides a NFC-B Poller (ISO14443B PCD) interface and 
 *  also provides some NFC-B Listener (ISO14443B PICC) helpers
 *
 *  The definitions and helpers methods provided by this module are only
 *  up to ISO14443-3 layer (excluding ATTRIB)
 *  
 *  
 * \addtogroup RFAL
 * @{
 *
 * \addtogroup RFAL-AL
 * \brief RFAL Abstraction Layer
 * @{
 *
 * \addtogroup NFC-B
 * \brief RFAL NFC-B Module
 * @{
 * 
 */


#ifndef RFAL_NFCB_H
#define RFAL_NFCB_H

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

#define RFAL_NFCB_FWTSENSB                       7680U                /*!< NFC-B FWT(SENSB)  Digital 2.0  B.3        */
#define RFAL_NFCB_DFWT                           49152U               /*!< NFC-B dFWT Delta 2.0  7.9.1.3 & B.3       */
#define RFAL_NFCB_DTPOLL_10                      rfalConvMsTo1fc(20)  /*!< NFC-B Delta Tb Poll  Digital 1.0  A.2     */
#define RFAL_NFCB_DTPOLL_20                      rfalConvMsTo1fc(17)  /*!< NFC-B Delta Tb Poll  Digital 2.1  B.3     */

#define RFAL_NFCB_AFI                            0x00U   /*!< NFC-B default Application Family   Digital 1.1 7.6.1.1 */
#define RFAL_NFCB_PARAM                          0x00U   /*!< NFC-B default SENSB_REQ PARAM                          */
#define RFAL_NFCB_CRC_LEN                        2U      /*!< NFC-B CRC length and CRC_B(AID)   Digital 1.1 Table 28 */
#define RFAL_NFCB_NFCID0_LEN                     4U      /*!< Length of NFC-B NFCID0                                 */
#define RFAL_NFCB_CMD_LEN                        1U      /*!< Length of NFC-B Command                                */

#define RFAL_NFCB_SENSB_RES_LEN                  12U     /*!< Standard length of SENSB_RES without SFGI byte         */
#define RFAL_NFCB_SENSB_RES_EXT_LEN              13U     /*!< Extended length of SENSB_RES with SFGI byte            */

#define RFAL_NFCB_SENSB_REQ_ADV_FEATURE          0x20U   /*!< Bit mask for Advance Feature in SENSB_REQ              */
#define RFAL_NFCB_SENSB_RES_FSCI_MASK            0x0FU   /*!< Bit mask for FSCI value in SENSB_RES                   */
#define RFAL_NFCB_SENSB_RES_FSCI_SHIFT           4U      /*!< Shift for FSCI value in SENSB_RES                      */
#define RFAL_NFCB_SENSB_RES_PROTO_RFU_MASK       0x08U   /*!< Bit mask for Protocol Type RFU in SENSB_RES            */
#define RFAL_NFCB_SENSB_RES_PROTO_TR2_MASK       0x03U   /*!< Bit mask for Protocol Type TR2 in SENSB_RES            */
#define RFAL_NFCB_SENSB_RES_PROTO_TR2_SHIFT      1U      /*!< Shift for Protocol Type TR2 in SENSB_RES               */
#define RFAL_NFCB_SENSB_RES_PROTO_ISO_MASK       0x01U   /*!< Bit mask Protocol Type ISO14443 Compliant in SENSB_RES */
#define RFAL_NFCB_SENSB_RES_FWI_MASK             0x0FU   /*!< Bit mask for FWI value in SENSB_RES                    */
#define RFAL_NFCB_SENSB_RES_FWI_SHIFT            4U      /*!< Bit mask for FWI value in SENSB_RES                    */
#define RFAL_NFCB_SENSB_RES_ADC_MASK             0x0CU   /*!< Bit mask for ADC value in SENSB_RES                    */
#define RFAL_NFCB_SENSB_RES_ADC_ADV_FEATURE_MASK 0x08U   /*!< Bit mask for ADC.Advanced Proto Features in SENSB_RES  */
#define RFAL_NFCB_SENSB_RES_ADC_PROPRIETARY_MASK 0x04U   /*!< Bit mask for ADC.Proprietary Application in SENSB_RES  */
#define RFAL_NFCB_SENSB_RES_FO_DID_MASK          0x01U   /*!< Bit mask for DID in SENSB_RES                          */
#define RFAL_NFCB_SENSB_RES_FO_NAD_MASK          0x02U   /*!< Bit mask for DID in SENSB_RES                          */
#define RFAL_NFCB_SENSB_RES_FO_MASK              0x03U   /*!< Bit mask for FO value in SENSB_RES (NAD and DID)       */
#define RFAL_NFCB_SENSB_RES_SFGI_MASK            0x0FU   /*!< Bit mask for SFGI in SENSB_RES                         */
#define RFAL_NFCB_SENSB_RES_SFGI_SHIFT           4U      /*!< Shift for SFGI in SENSB_RES                            */

/*
******************************************************************************
* GLOBAL MACROS
******************************************************************************
*/

/*! Get device's FSCI given its SENSB_RES  Digital 1.1 7.6.2  */
#define rfalNfcbGetFSCI( sensbRes )        ((((rfalNfcbSensbRes*)(sensbRes))->protInfo.FsciProType >> RFAL_NFCB_SENSB_RES_FSCI_SHIFT) & RFAL_NFCB_SENSB_RES_FSCI_MASK )

/*! Checks if the given NFC-B device indicates ISO-DEP support */
#define rfalNfcbIsIsoDepSupported( dev )  ( (((rfalNfcbListenDevice*)(dev))->sensbRes.protInfo.FsciProType & RFAL_NFCB_SENSB_RES_PROTO_ISO_MASK) != 0U )

/*
******************************************************************************
* GLOBAL TYPES
******************************************************************************
*/

/*! SENSB_REQ and ALLB_REQ param   Digital 1.1 7.6.1  */
typedef enum
{
     RFAL_NFCB_SENS_CMD_ALLB_REQ  = 0x08,  /*!< ALLB_REQ  (WUPB)  */
     RFAL_NFCB_SENS_CMD_SENSB_REQ = 0x00   /*!< SENSB_REQ (REQB)  */
} rfalNfcbSensCmd;


/*! Number of Slots (NI) codes used for NFC-B anti collision  Digital 1.1 Table 26 */
typedef enum
{
    RFAL_NFCB_SLOT_NUM_1  = 0,         /*!< N=0 :  1 slot   */
    RFAL_NFCB_SLOT_NUM_2  = 1,         /*!< N=1 :  2 slots  */
    RFAL_NFCB_SLOT_NUM_4  = 2,         /*!< N=2 :  4 slots  */
    RFAL_NFCB_SLOT_NUM_8  = 3,         /*!< N=3 :  8 slots  */
    RFAL_NFCB_SLOT_NUM_16 = 4          /*!< N=4 : 16 slots  */
}rfalNfcbSlots;


/*! SENSB_RES (ATQB) Application Data Format   Digital 1.1 Table 28 */
typedef struct
{
    uint8_t  AFI;                      /*!< Application Family Identifier */
    uint8_t  CRC_B[RFAL_NFCB_CRC_LEN]; /*!< CRC_B of AID                  */
    uint8_t  numApps;                  /*!< Number of Applications        */
} rfalNfcbSensbResAppData;


/*! SENSB_RES Protocol Info format Digital 1.1 Table 29 */
typedef struct
{
    uint8_t  BRC;                      /*!< Bit Rate Capability                                                            */
    uint8_t  FsciProType;              /*!< Frame Size Card Integer [4b] | Protocol Type[4 bits]                           */
    uint8_t  FwiAdcFo;                 /*!< Frame Waiting Integer [4b] | Application Data Coding [2b] | Frame Options [2b] */
    uint8_t  SFGI;                     /*!< Optional: Start-Up Frame Guard Time Integer[4b] | RFU [4b]                     */
} rfalNfcbSensbResProtocolInfo;


/*! SENSB_RES format   Digital 1.1  7.6.2 */
typedef struct
{
    uint8_t                      cmd;                           /*!< SENSB_RES: 50h       */
    uint8_t                      nfcid0[RFAL_NFCB_NFCID0_LEN];  /*!< NFC Identifier (PUPI)*/
    rfalNfcbSensbResAppData      appData;                       /*!< Application Data     */ 
    rfalNfcbSensbResProtocolInfo protInfo;                      /*!< Protocol Information */
} rfalNfcbSensbRes;


/*! NFC-B listener device (PICC) struct  */
typedef struct
{
    uint8_t           sensbResLen;                              /*!< SENSB_RES length      */   
    rfalNfcbSensbRes  sensbRes;                                 /*!< SENSB_RES             */
    bool              isSleep;                                  /*!< Device sleeping flag  */
}rfalNfcbListenDevice;

/*
******************************************************************************
* GLOBAL FUNCTION PROTOTYPES
******************************************************************************
*/

/*! 
 *****************************************************************************
 * \brief  Initialize NFC-B Poller mode
 *  
 * This methods configures RFAL RF layer to perform as a 
 * NFC-B Poller/RW (ISO14443B PCD) including all default timings
 * 
 * It sets NFC-B parameters (AFI, PARAM) to default values
 *
 * \return ERR_WRONG_STATE  : RFAL not initialized or mode not set
 * \return ERR_NONE         : No error
 *****************************************************************************
 */
ReturnCode rfalNfcbPollerInitialize( void );


/*! 
 *****************************************************************************
 * \brief  Set NFC-B Poller parameters
 *  
 * This methods configures RFAL RF layer to perform as a 
 * NFCA Poller/RW (ISO14443A PCD) including all default timings
 * 
 * Additionally configures NFC-B specific parameters to be used on the 
 * following communications
 * 
 * \param[in]  AFI   : Application Family Identifier to be used
 * \param[in]  PARAM : PARAM to be used, it announces whether Advanced
 *                     Features or Extended SENSB_RES is supported
 * 
 * \return ERR_WRONG_STATE  : RFAL not initialized or mode not set
 * \return ERR_NONE         : No error
 *****************************************************************************
 */
ReturnCode rfalNfcbPollerInitializeWithParams( uint8_t AFI, uint8_t PARAM );


/*! 
 *****************************************************************************
 * \brief  NFC-B Poller Check Presence
 *  
 * This method checks if a NFC-B Listen device (PICC) is present on the field
 * by sending an ALLB_REQ (WUPB) or SENSB_REQ (REQB)
 *  
 * \param[in]  cmd         : Indicate if to send an ALL_REQ or a SENS_REQ
 * \param[in]  slots       : The number of slots to be announced
 * \param[out] sensbRes    : If received, the SENSB_RES
 * \param[out] sensbResLen : If received, the SENSB_RES length
 * 
 *
 * \return ERR_WRONG_STATE  : RFAL not initialized or incorrect mode
 * \return ERR_PARAM        : Invalid parameters
 * \return ERR_IO           : Generic internal error
 * \return ERR_TIMEOUT      : Timeout error, no listener device detected
 * \return ERR_RF_COLLISION : Collision detected one or more device in the field
 * \return ERR_PAR          : Parity error detected, one or more device in the field
 * \return ERR_CRC          : CRC error detected, one or more device in the field
 * \return ERR_FRAMING      : Framing error detected, one or more device in the field
 * \return ERR_PROTO        : Protocol error detected, invalid SENSB_RES received
 * \return ERR_NONE         : No error, SENSB_RES received
 *****************************************************************************
 */
ReturnCode rfalNfcbPollerCheckPresence( rfalNfcbSensCmd cmd, rfalNfcbSlots slots, rfalNfcbSensbRes *sensbRes, uint8_t *sensbResLen );


/*! 
 *****************************************************************************
 * \brief  NFC-B Poller Sleep
 *  
 * This function is used to send the SLPB_REQ (HLTB) command to put the PICC with 
 * the given NFCID0 to state HALT so that they do not reply to further SENSB_REQ 
 * commands (only to ALLB_REQ)
 * 
 * \param[in]  nfcid0       : NFCID of the device to be put to Sleep
 *  
 * \return ERR_WRONG_STATE  : RFAL not initialized or incorrect mode
 * \return ERR_PARAM        : Invalid parameters
 * \return ERR_IO           : Generic internal error
 * \return ERR_NONE         : No error
 *****************************************************************************
 */
ReturnCode rfalNfcbPollerSleep( const uint8_t* nfcid0 );


/*! 
 *****************************************************************************
 * \brief  NFC-B Poller Slot Marker
 *  
 * This method selects a NFC-B Slot marker frame 
 *  
 * \param[in]  slotCode     : Slot Code [1-15] 
 * \param[out] sensbRes     : If received, the SENSB_RES
 * \param[out] sensbResLen  : If received, the SENSB_RES length
 *
 * \return ERR_WRONG_STATE  : RFAL not initialized or incorrect mode
 * \return ERR_PARAM        : Invalid parameters
 * \return ERR_IO           : Generic internal error
 * \return ERR_TIMEOUT      : Timeout error
 * \return ERR_PAR          : Parity error detected
 * \return ERR_CRC          : CRC error detected
 * \return ERR_FRAMING      : Framing error detected
 * \return ERR_PROTO        : Protocol error detected
 * \return ERR_NONE         : No error, SEL_RES received
 *****************************************************************************
 */
ReturnCode rfalNfcbPollerSlotMarker( uint8_t slotCode, rfalNfcbSensbRes *sensbRes, uint8_t *sensbResLen );

/*! 
 *****************************************************************************
 * \brief  NFC-B Technology Detection
 *  
 * This method performs NFC-B Technology Detection as defined in the spec
 * given in the compliance mode
 *  
 * \param[in]  compMode    : compliance mode to be performed
 * \param[out] sensbRes    : location to store the SENSB_RES, if received
 * \param[out] sensbResLen : length of the SENSB_RES, if received
 *  
 * \return ERR_WRONG_STATE  : RFAL not initialized or incorrect mode
 * \return ERR_PARAM        : Invalid parameters
 * \return ERR_IO           : Generic internal error
 * \return ERR_NONE         : No error, one or more device in the field
 *****************************************************************************
 */
ReturnCode rfalNfcbPollerTechnologyDetection( rfalComplianceMode compMode, rfalNfcbSensbRes *sensbRes, uint8_t *sensbResLen );

/*! 
 *****************************************************************************
 * \brief  NFC-B Poller Collision Resolution
 *  
 * NFC-B Collision resolution  Listener device/card (PICC) as 
 * defined in Activity 1.1  9.3.5
 * 
 * This function is used to perform collision resolution for detection in case 
 * of multiple NFC Forum Devices with Technology B detected. 
 * Target with valid SENSB_RES will be stored in devInfo and nfcbDevCount incremented.  
 *
 * \param[in]  compMode    : compliance mode to be performed
 * \param[in]  devLimit    : device limit value, and size nfcbDevList
 * \param[out] nfcbDevList : NFC-B listener device info
 * \param[out] devCnt      : devices found counter
 *
 * \return ERR_WRONG_STATE  : RFAL not initialized or mode not set
 * \return ERR_PARAM        : Invalid parameters
 * \return ERR_IO           : Generic internal error
 * \return ERR_PROTO        : Protocol error detected
 * \return ERR_NONE         : No error
 *****************************************************************************
 */
ReturnCode rfalNfcbPollerCollisionResolution( rfalComplianceMode compMode, uint8_t devLimit, rfalNfcbListenDevice *nfcbDevList, uint8_t *devCnt );

/*! 
 *****************************************************************************
 * \brief  NFC-B Poller Collision Resolution Slotted
 *  
 * NFC-B Collision resolution  Listener device/card (PICC). The sequence can 
 * be configured to be according to NFC Forum Activity 1.1  9.3.5, ISO10373
 * or EMVCo 
 * 
 * This function is used to perform collision resolution for detection in case 
 * of multiple NFC Forum Devices with Technology B are detected. 
 * Target with valid SENSB_RES will be stored in devInfo and nfcbDevCount incremented.  
 * 
 * This method provides the means to perform a collision resolution loop with specific
 * initial and end number of slots. This allows to user to start the loop already with 
 * greater number of slots, and or limit the end number of slots. At the end a flag
 * indicating whether there were collisions pending is returned.
 * 
 * If RFAL_COMPLIANCE_MODE_ISO is used \a initSlots must be set to RFAL_NFCB_SLOT_NUM_1
 *  
 *
 * \param[in]  compMode    : compliance mode to be performed
 * \param[in]  devLimit    : device limit value, and size nfcbDevList
 * \param[in]  initSlots   : number of slots to open initially 
 * \param[in]  endSlots    : number of slots when to stop collision resolution 
 * \param[out] nfcbDevList : NFC-B listener device info
 * \param[out] devCnt      : devices found counter
 * \param[out] colPending  : flag indicating whether collision are still pending
 *
 * \return ERR_WRONG_STATE  : RFAL not initialized or mode not set
 * \return ERR_PARAM        : Invalid parameters
 * \return ERR_IO           : Generic internal error
 * \return ERR_PROTO        : Protocol error detected
 * \return ERR_NONE         : No error
 *****************************************************************************
 */
ReturnCode rfalNfcbPollerSlottedCollisionResolution( rfalComplianceMode compMode, uint8_t devLimit, rfalNfcbSlots initSlots, rfalNfcbSlots endSlots, rfalNfcbListenDevice *nfcbDevList, uint8_t *devCnt, bool *colPending );


/*! 
 *****************************************************************************
 * \brief  NFC-B TR2 code to FDT
 *    
 *  Converts the TR2 code as defined in Digital 1.1 Table 33 Minimum 
 *  TR2 Coding to Frame Delay Time (FDT) in 1/Fc
 *
 * \param[in]  tr2Code : TR2 code as defined in Digital 1.1 Table 33
 * 
 * \return FDT in 1/Fc
 *****************************************************************************
 */
uint32_t rfalNfcbTR2ToFDT( uint8_t tr2Code );


#endif /* RFAL_NFCB_H */

/**
  * @}
  *
  * @}
  *
  * @}
  */
