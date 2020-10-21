
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

/*! \file rfal_nfc.h
 *
 *  \brief RFAL NFC device
 *  
 *  This module provides the required features to behave as an NFC Poller 
 *  or Listener device. It grants an easy to use interface for the following
 *  activities: Technology Detection, Collision Resolution, Activation,
 *  Data Exchange, and Deactivation
 *  
 *  This layer is influenced by (but not fully aligned with) the NFC Forum 
 *  specifications, in particular: Activity 2.0 and NCI 2.0
 *
 *  
 *    
 * \addtogroup RFAL
 * @{
 * 
 * \addtogroup RFAL-HL
 * \brief RFAL Higher Layer
 * @{
 * 
 * \addtogroup NFC
 * \brief RFAL NFC Device
 * @{
 *  
 */

#ifndef RFAL_NFC_H
#define RFAL_NFC_H

/*
******************************************************************************
* INCLUDES
******************************************************************************
*/
#include "platform.h"
#include "st_errno.h"
#include "rfal_rf.h"
#include "rfal_nfca.h"
#include "rfal_nfcb.h"
#include "rfal_nfcf.h"
#include "rfal_nfcv.h"
#include "rfal_st25tb.h"
#include "rfal_nfcDep.h"
#include "rfal_isoDep.h"


/*
******************************************************************************
* GLOBAL DEFINES
******************************************************************************
*/

#define RFAL_NFC_TECH_NONE               0x0000U  /*!< No technology             */
#define RFAL_NFC_POLL_TECH_A             0x0001U  /*!< NFC-A technology Flag     */
#define RFAL_NFC_POLL_TECH_B             0x0002U  /*!< NFC-B technology Flag     */
#define RFAL_NFC_POLL_TECH_F             0x0004U  /*!< NFC-F technology Flag     */
#define RFAL_NFC_POLL_TECH_V             0x0008U  /*!< NFC-V technology Flag     */
#define RFAL_NFC_POLL_TECH_AP2P          0x0010U  /*!< AP2P technology Flag      */
#define RFAL_NFC_POLL_TECH_ST25TB        0x0020U  /*!< ST25TB technology Flag    */
#define RFAL_NFC_LISTEN_TECH_A           0x1000U  /*!< NFC-V technology Flag     */
#define RFAL_NFC_LISTEN_TECH_B           0x2000U  /*!< NFC-V technology Flag     */
#define RFAL_NFC_LISTEN_TECH_F           0x4000U  /*!< NFC-V technology Flag     */
#define RFAL_NFC_LISTEN_TECH_AP2P        0x8000U  /*!< NFC-V technology Flag     */


/*
******************************************************************************
* GLOBAL MACROS
******************************************************************************
*/

/*! Checks if a device is currently activated */
#define rfalNfcIsDevActivated( st )        ( ((st)>= RFAL_NFC_STATE_ACTIVATED) && ((st)<RFAL_NFC_STATE_DEACTIVATION) )

/*! Checks if a device is in discovery */
#define rfalNfcIsInDiscovery( st )         ( ((st)>= RFAL_NFC_STATE_START_DISCOVERY) && ((st)<RFAL_NFC_STATE_ACTIVATED) )

/*! Checks if remote device is in Poll mode */
#define rfalNfcIsRemDevPoller( tp )    ( ((tp)>= RFAL_NFC_POLL_TYPE_NFCA) && ((tp)<=RFAL_NFC_POLL_TYPE_AP2P ) )

/*! Checks if remote device is in Listen mode */
#define rfalNfcIsRemDevListener( tp )  ( ((int16_t)(tp)>= (int16_t)RFAL_NFC_LISTEN_TYPE_NFCA) && ((tp)<=RFAL_NFC_LISTEN_TYPE_AP2P) )

/*
******************************************************************************
* GLOBAL ENUMS
******************************************************************************
*/

/*
******************************************************************************
* GLOBAL TYPES
******************************************************************************
*/

/*! Main state                                                                       */
typedef enum{
    RFAL_NFC_STATE_NOTINIT                  =  0,   /*!< Not Initialized state       */
    RFAL_NFC_STATE_IDLE                     =  1,   /*!< Initialize state            */
    RFAL_NFC_STATE_START_DISCOVERY          =  2,   /*!< Start Discovery loop state  */
    RFAL_NFC_STATE_WAKEUP_MODE              =  3,   /*!< Wake-Up state               */
    RFAL_NFC_STATE_POLL_TECHDETECT          =  10,  /*!< Technology Detection state  */
    RFAL_NFC_STATE_POLL_COLAVOIDANCE        =  11,  /*!< Collision Avoidance state   */
    RFAL_NFC_STATE_POLL_SELECT              =  12,  /*!< Wait for Selection state    */
    RFAL_NFC_STATE_POLL_ACTIVATION          =  13,  /*!< Activation state            */
    RFAL_NFC_STATE_LISTEN_TECHDETECT        =  20,  /*!< Listen Tech Detect          */
    RFAL_NFC_STATE_LISTEN_COLAVOIDANCE      =  21,  /*!< Listen Collision Avoidance  */
    RFAL_NFC_STATE_LISTEN_ACTIVATION        =  22,  /*!< Listen Activation state     */
    RFAL_NFC_STATE_LISTEN_SLEEP             =  23,  /*!< Listen Sleep state          */
    RFAL_NFC_STATE_ACTIVATED                =  30,  /*!< Activated state             */
    RFAL_NFC_STATE_DATAEXCHANGE             =  31,  /*!< Data Exchange Start state   */
    RFAL_NFC_STATE_DATAEXCHANGE_DONE        =  33,  /*!< Data Exchange terminated    */
    RFAL_NFC_STATE_DEACTIVATION             =  34   /*!< Deactivation state          */
}rfalNfcState;


/*! Device type                                                                       */
typedef enum{
    RFAL_NFC_LISTEN_TYPE_NFCA               =  0,   /*!< NFC-A Listener device type  */
    RFAL_NFC_LISTEN_TYPE_NFCB               =  1,   /*!< NFC-B Listener device type  */
    RFAL_NFC_LISTEN_TYPE_NFCF               =  2,   /*!< NFC-F Listener device type  */
    RFAL_NFC_LISTEN_TYPE_NFCV               =  3,   /*!< NFC-V Listener device type  */
    RFAL_NFC_LISTEN_TYPE_ST25TB             =  4,   /*!< ST25TB Listener device type */
    RFAL_NFC_LISTEN_TYPE_AP2P               =  5,   /*!< AP2P Listener device type   */
    RFAL_NFC_POLL_TYPE_NFCA                 =  10,  /*!< NFC-A Poller device type    */
    RFAL_NFC_POLL_TYPE_NFCB                 =  11,  /*!< NFC-B Poller device type    */
    RFAL_NFC_POLL_TYPE_NFCF                 =  12,  /*!< NFC-F Poller device type    */
    RFAL_NFC_POLL_TYPE_NFCV                 =  13,  /*!< NFC-V Poller device type    */
    RFAL_NFC_POLL_TYPE_AP2P                 =  15   /*!< AP2P Poller device type     */
}rfalNfcDevType;


/*! Device interface                                                                 */
typedef enum{
    RFAL_NFC_INTERFACE_RF                   = 0,    /*!< RF Frame interface          */
    RFAL_NFC_INTERFACE_ISODEP               = 1,    /*!< ISO-DEP interface           */
    RFAL_NFC_INTERFACE_NFCDEP               = 2     /*!< NFC-DEP interface           */
}rfalNfcRfInterface;


/*! Device struct containing all its details                                          */
typedef struct{
    rfalNfcDevType type;                            /*!< Device's type                */
    union{                              /*  PRQA S 0750 # MISRA 19.2 - Members of the union will not be used concurrently, only one technology at a time */
        rfalNfcaListenDevice   nfca;                /*!< NFC-A Listen Device instance */
        rfalNfcbListenDevice   nfcb;                /*!< NFC-B Listen Device instance */
        rfalNfcfListenDevice   nfcf;                /*!< NFC-F Listen Device instance */
        rfalNfcvListenDevice   nfcv;                /*!< NFC-V Listen Device instance */
        rfalSt25tbListenDevice st25tb;              /*!< ST25TB Listen Device instance*/
    }dev;                                           /*!< Device's instance            */
                                                    
    uint8_t                    *nfcid;              /*!< Device's NFCID               */
    uint8_t                    nfcidLen;            /*!< Device's NFCID length        */
    rfalNfcRfInterface         rfInterface;         /*!< Device's interface           */
    
    union{                              /*  PRQA S 0750 # MISRA 19.2 - Members of the union will not be used concurrently, only one protocol at a time */            
        rfalIsoDepDevice       isoDep;              /*!< ISO-DEP instance             */
        rfalNfcDepDevice       nfcDep;              /*!< NFC-DEP instance             */
    }proto;                                         /*!< Device's protocol            */
}rfalNfcDevice;


/*! Discovery parameters                                                                                           */
typedef struct{
    rfalComplianceMode compMode;                        /*!< Compliancy mode to be used                            */
    uint16_t           techs2Find;                      /*!< Technologies to search for                            */
    uint16_t           totalDuration;                   /*!< Duration of a whole Poll + Listen cycle               */
    uint8_t            devLimit;                        /*!< Max number of devices                                 */
    rfalBitRate        maxBR;                           /*!< Max Bit rate to be used for communications            */
    
    rfalBitRate        nfcfBR;                          /*!< Bit rate to poll for NFC-F                            */
    uint8_t            nfcid3[RFAL_NFCDEP_NFCID3_LEN];  /*!< NFCID3 to be used on the ATR_REQ/ATR_RES              */
    uint8_t            GB[RFAL_NFCDEP_GB_MAX_LEN];      /*!< General bytes to be used on the ATR-REQ               */
    uint8_t            GBLen;                           /*!< Length of the General Bytes                           */
    rfalBitRate        ap2pBR;                          /*!< Bit rate to poll for AP2P                             */

    
    rfalLmConfPA       lmConfigPA;                      /*!< Configuration for Passive Listen mode NFC-A           */
    rfalLmConfPF       lmConfigPF;                      /*!< Configuration for Passive Listen mode NFC-A           */
    
    void               (*notifyCb)( rfalNfcState st );  /*!< Callback to Notify upper layer                        */
                                                        
    bool               wakeupEnabled;                   /*!< Enable Wake-Up mode before polling                    */
    bool               wakeupConfigDefault;             /*!< Wake-Up mode default configuration                    */
    rfalWakeUpConfig   wakeupConfig;                    /*!< Wake-Up mode configuration                            */
}rfalNfcDiscoverParam;


/*! Buffer union, only one interface is used at a time                                                             */
typedef union{  /*  PRQA S 0750 # MISRA 19.2 - Members of the union will not be used concurrently, only one interface at a time */
    uint8_t                  rfBuf[RFAL_FEATURE_NFC_RF_BUF_LEN]; /*!< RF buffer                                    */
    rfalIsoDepApduBufFormat  isoDepBuf;                          /*!< ISO-DEP buffer format (with header/prologue) */
    rfalNfcDepPduBufFormat   nfcDepBuf;                          /*!< NFC-DEP buffer format (with header/prologue) */
}rfalNfcBuffer;

/*******************************************************************************/

/*
******************************************************************************
* GLOBAL FUNCTION PROTOTYPES
******************************************************************************
*/

/*! 
 *****************************************************************************
 * \brief  RFAL NFC Worker
 *  
 * It runs the internal state machine and runs the RFAL RF worker.
 *****************************************************************************
 */
void rfalNfcWorker( void );

/*! 
 *****************************************************************************
 * \brief  RFAL NFC Initialize
 *  
 * It initializes this module and its dependencies
 *
 * \return ERR_WRONG_STATE  : Incorrect state for this operation
 * \return ERR_IO           : Generic internal error
 * \return ERR_NONE         : No error
 *****************************************************************************
 */
ReturnCode rfalNfcInitialize( void );

/*!
 *****************************************************************************
 * \brief  RFAL NFC Discovery
 *  
 * It set the device in Discovery state.
 * In discovery it will Poll and/or Listen for the technologies configured, 
 * and perform Wake-up mode if configured to do so.
 *
 * The device list passed on disParams must not be empty.
 * The number of devices on the list is indicated by the devLimit and shall
 * be at >= 1.
 *
 * \param[in]  disParams    : discovery configuration parameters
 *
 * \return ERR_WRONG_STATE  : Incorrect state for this operation
 * \return ERR_PARAM        : Invalid parameters
 * \return ERR_NONE         : No error
 *****************************************************************************
 */
ReturnCode rfalNfcDiscover( const rfalNfcDiscoverParam *disParams );

/*!
 *****************************************************************************
 * \brief  RFAL NFC Get State
 *  
 * It returns the current state
 *
 * \return rfalNfcState : the current state
 *****************************************************************************
 */
rfalNfcState rfalNfcGetState( void );

/*!
 *****************************************************************************
 * \brief  RFAL NFC Get Devices Found
 *  
 * It returns the location of the device list and the number of 
 * devices found.
 *
 * \param[out]  devList     : device list location
 * \param[out]  devCnt      : number of devices found
 *
 * \return ERR_WRONG_STATE  : Incorrect state for this operation
 *                            Discovery still ongoing
 * \return ERR_PARAM        : Invalid parameters
 * \return ERR_NONE         : No error
 *****************************************************************************
 */
ReturnCode rfalNfcGetDevicesFound( rfalNfcDevice **devList, uint8_t *devCnt );

/*!
 *****************************************************************************
 * \brief  RFAL NFC Get Active Device
 *  
 * It returns the location of the device current Active device
 *
 * \param[out]  dev           : device info location
 *
 * \return ERR_WRONG_STATE    : Incorrect state for this operation
 *                              No device activated
 * \return ERR_PARAM          : Invalid parameters
 * \return ERR_NONE           : No error
 *****************************************************************************
 */
ReturnCode rfalNfcGetActiveDevice( rfalNfcDevice **dev );


/*!
 *****************************************************************************
 * \brief  RFAL NFC Select Device
 *  
 * It selects the device to be activated.
 * It shall be called when more than one device has been identified to 
 * indiacte which device shall be actived
 * 
 * \param[in]  devIdx       : device index to be activated
 *
 * \return ERR_WRONG_STATE  : Incorrect state for this operation
 *                            Not in select state
 * \return ERR_PARAM        : Invalid parameters
 * \return ERR_NONE         : No error
 *****************************************************************************
 */
ReturnCode rfalNfcSelect( uint8_t devIdx );

/*!
 *****************************************************************************
 * \brief  RFAL NFC Start Data Exchange
 *  
 * After a device has been activated, it starts a data exchange.
 * It handles automatically which interface/protocol to be used and acts accordingly.
 *
 * In Listen mode the first frame/data shall be sent by the Reader/Initiator
 * therefore this method must be called first with txDataLen set to zero 
 * to retrieve the rxData and rcvLen locations.
 *
 *
 * \param[in]  txData       : data to be transmitted
 * \param[in]  txDataLen    : size of the data to be transmitted
 * \param[out] rxData       : location of the received data after operation is completed
 * \param[out] rvdLen       : location of thelength of the received data
 * \param[in]  fwt          : FWT to be used in case of RF interface.
 *                            If ISO-DEP or NFC-DEP interface is used, this will be ignored
 *
 * \return ERR_WRONG_STATE  : Incorrect state for this operation
 * \return ERR_PARAM        : Invalid parameters
 * \return ERR_NONE         : No error
 *****************************************************************************
 */
ReturnCode rfalNfcDataExchangeStart( uint8_t *txData, uint16_t txDataLen, uint8_t **rxData, uint16_t **rvdLen, uint32_t fwt );

/*! 
 *****************************************************************************
 * \brief  RFAL NFC Get Data Exchange Status
 *  
 * Gets current Data Exchange status
 *
 * \return  ERR_NONE         : Transceive done with no error
 * \return  ERR_BUSY         : Transceive ongoing
 *  \return ERR_AGAIN        : received one chaining block, copy received data 
 *                             and continue to call this method to retrieve the 
 *                             remaining blocks
 * \return  ERR_XXXX         : Error occurred
 * \return  ERR_TIMEOUT      : No response
 * \return  ERR_FRAMING      : Framing error detected
 * \return  ERR_PAR          : Parity error detected
 * \return  ERR_CRC          : CRC error detected
 * \return  ERR_LINK_LOSS    : Link Loss - External Field is Off
 * \return  ERR_RF_COLLISION : Collision detected
 * \return  ERR_IO           : Internal error
 *****************************************************************************
 */
ReturnCode rfalNfcDataExchangeGetStatus( void );

/*! 
 *****************************************************************************
 * \brief  RFAL NFC Deactivate
 *  
 * It triggers the deactivation procedure to terminate communications with 
 * remote device. At the end the field will be turned off.
 *
 * \param[in]  discovery    : TRUE if after deactivation go back into discovery
 *                          : FALSE if after deactivation remain in idle
 *
 * \return ERR_WRONG_STATE  : Incorrect state for this operation
 * \return ERR_NONE         : No error
 *****************************************************************************
 */
ReturnCode rfalNfcDeactivate( bool discovery );

#endif /* RFAL_NFC_H */


/**
  * @}
  *
  * @}
  *
  * @}
  */
