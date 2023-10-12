
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
 *      PROJECT:   ST25R3916 firmware
 *      Revision: 
 *      LANGUAGE:  ISO C99
 */

/*! \file
 *
 *  \author Gustavo Patricio 
 *
 *  \brief RF Abstraction Layer (RFAL)
 *  
 *  RFAL implementation for ST25R3916
 */

/*
******************************************************************************
* INCLUDES
******************************************************************************
*/

#include "rfal_chip.h"
#include "utils.h"
#include "st25r3916.h"
#include "st25r3916_com.h"
#include "st25r3916_irq.h"
#include "rfal_analogConfig.h"
#include "rfal_iso15693_2.h"
#include "rfal_crc.h"

/*
 ******************************************************************************
 * ENABLE SWITCHES
 ******************************************************************************
 */

#ifndef RFAL_FEATURE_LISTEN_MODE
#define RFAL_FEATURE_LISTEN_MODE false /* Listen Mode configuration missing. Disabled by default */
#endif /* RFAL_FEATURE_LISTEN_MODE */

#ifndef RFAL_FEATURE_WAKEUP_MODE
#define RFAL_FEATURE_WAKEUP_MODE \
    false /* Wake-Up mode configuration missing. Disabled by default */
#endif /* RFAL_FEATURE_WAKEUP_MODE */

#ifndef RFAL_FEATURE_LOWPOWER_MODE
#define RFAL_FEATURE_LOWPOWER_MODE \
    false /* Low Power mode configuration missing. Disabled by default */
#endif /* RFAL_FEATURE_LOWPOWER_MODE */

/*
******************************************************************************
* GLOBAL TYPES
******************************************************************************
*/

/*! Struct that holds all involved on a Transceive including the context passed by the caller     */
typedef struct {
    rfalTransceiveState state; /*!< Current transceive state                            */
    rfalTransceiveState lastState; /*!< Last transceive state (debug purposes)              */
    ReturnCode status; /*!< Current status/error of the transceive              */

    rfalTransceiveContext ctx; /*!< The transceive context given by the caller          */
} rfalTxRx;

/*! Struct that holds all context for the Listen Mode                                             */
typedef struct {
    rfalLmState state; /*!< Current Listen Mode state                           */
    uint32_t mdMask; /*!< Listen Mode mask used                               */
    uint32_t mdReg; /*!< Listen Mode register value used                     */
    uint32_t mdIrqs; /*!< Listen Mode IRQs used                               */
    rfalBitRate brDetected; /*!< Last bit rate detected                              */

    uint8_t* rxBuf; /*!< Location to store incoming data in Listen Mode      */
    uint16_t rxBufLen; /*!< Length of rxBuf                                     */
    uint16_t* rxLen; /*!< Pointer to write the data length placed into rxBuf  */
    bool dataFlag; /*!< Listen Mode current Data Flag                       */
    bool iniFlag; /*!< Listen Mode initialized Flag  (FeliCa slots)        */
} rfalLm;

/*! Struct that holds all context for the Wake-Up Mode                                             */
typedef struct {
    rfalWumState state; /*!< Current Wake-Up Mode state                           */
    rfalWakeUpConfig cfg; /*!< Current Wake-Up Mode context                         */
} rfalWum;

/*! Struct that holds all context for the Low Power Mode                                             */
typedef struct {
    bool isRunning;
} rfalLpm;

/*! Struct that holds the timings GT and FDTs                           */
typedef struct {
    uint32_t GT; /*!< GT in 1/fc                */
    uint32_t FDTListen; /*!< FDTListen in 1/fc         */
    uint32_t FDTPoll; /*!< FDTPoll in 1/fc           */
    uint8_t nTRFW; /*!< n*TRFW used during RF CA  */
} rfalTimings;

/*! Struct that holds the software timers                               */
typedef struct {
    uint32_t GT; /*!< RFAL's GT timer           */
    uint32_t RXE; /*!< Timer between RXS and RXE */
    uint32_t txRx; /*!< Transceive sanity timer   */
} rfalTimers;

/*! Struct that holds the RFAL's callbacks                              */
typedef struct {
    rfalPreTxRxCallback preTxRx; /*!< RFAL's Pre TxRx callback  */
    rfalPostTxRxCallback postTxRx; /*!< RFAL's Post TxRx callback */
    RfalStateChangedCallback state_changed_cb;
    void* ctx;
} rfalCallbacks;

/*! Struct that holds counters to control the FIFO on Tx and Rx                                                                          */
typedef struct {
    uint16_t
        expWL; /*!< The amount of bytes expected to be Tx when a WL interrupt occurs                          */
    uint16_t
        bytesTotal; /*!< Total bytes to be transmitted OR the total bytes received                                  */
    uint16_t
        bytesWritten; /*!< Amount of bytes already written on FIFO (Tx) OR read (RX) from FIFO and written on rxBuffer*/
    uint8_t status
        [ST25R3916_FIFO_STATUS_LEN]; /*!< FIFO Status Registers                                              */
} rfalFIFO;

/*! Struct that holds RFAL's configuration settings                                                      */
typedef struct {
    uint8_t obsvModeTx; /*!< RFAL's config of the ST25R3916's observation mode while Tx */
    uint8_t obsvModeRx; /*!< RFAL's config of the ST25R3916's observation mode while Rx */
    rfalEHandling eHandling; /*!< RFAL's error handling config/mode                          */
} rfalConfigs;

/*! Struct that holds NFC-F data - Used only inside rfalFelicaPoll() (static to avoid adding it into stack) */
typedef struct {
    rfalFeliCaPollRes
        pollResponses[RFAL_FELICA_POLL_MAX_SLOTS]; /* FeliCa Poll response container for 16 slots */
} rfalNfcfWorkingData;

/*! Struct that holds NFC-V current context
 *
 * This buffer has to be big enough for coping with maximum response size (hamming coded)
 *    - inventory requests responses: 14*2+2 bytes 
 *    - read single block responses: (32+4)*2+2 bytes
 *    - read multiple block could be very long... -> not supported
 *    - current implementation expects it be written in one bulk into FIFO
 *    - needs to be above FIFO water level of ST25R3916 (200)
 *    - the coding function needs to be able to 
 *      put more than FIFO water level bytes into it (n*64+1)>200                                                          */
typedef struct {
    uint8_t codingBuffer[(
        (2 + 255 + 3) * 2)]; /*!< Coding buffer,   length MUST be above 257: [257; ...]    */
    uint16_t
        nfcvOffset; /*!< Offset needed for ISO15693 coding function                             */
    rfalTransceiveContext
        origCtx; /*!< context provided by user                                               */
    uint16_t
        ignoreBits; /*!< Number of bits at the beginning of a frame to be ignored when decoding */
} rfalNfcvWorkingData;

/*! RFAL instance  */
typedef struct {
    rfalState state; /*!< RFAL's current state                          */
    rfalMode mode; /*!< RFAL's current mode                           */
    rfalBitRate txBR; /*!< RFAL's current Tx Bit Rate                    */
    rfalBitRate rxBR; /*!< RFAL's current Rx Bit Rate                    */
    bool field; /*!< Current field state (On / Off)                */

    rfalConfigs conf; /*!< RFAL's configuration settings                 */
    rfalTimings timings; /*!< RFAL's timing setting                         */
    rfalTxRx TxRx; /*!< RFAL's transceive management                  */
    rfalFIFO fifo; /*!< RFAL's FIFO management                        */
    rfalTimers tmr; /*!< RFAL's Software timers                        */
    rfalCallbacks callbacks; /*!< RFAL's callbacks                              */

#if RFAL_FEATURE_LISTEN_MODE
    rfalLm Lm; /*!< RFAL's listen mode management                 */
#endif /* RFAL_FEATURE_LISTEN_MODE */

#if RFAL_FEATURE_WAKEUP_MODE
    rfalWum wum; /*!< RFAL's Wake-up mode management                */
#endif /* RFAL_FEATURE_WAKEUP_MODE */

#if RFAL_FEATURE_LOWPOWER_MODE
    rfalLpm lpm; /*!< RFAL's Low power mode management              */
#endif /* RFAL_FEATURE_LOWPOWER_MODE */

#if RFAL_FEATURE_NFCF
    rfalNfcfWorkingData nfcfData; /*!< RFAL's working data when supporting NFC-F     */
#endif /* RFAL_FEATURE_NFCF */

#if RFAL_FEATURE_NFCV
    rfalNfcvWorkingData nfcvData; /*!< RFAL's working data when performing NFC-V     */
#endif /* RFAL_FEATURE_NFCV */

} rfal;

/*! Felica's command set */
typedef enum {
    FELICA_CMD_POLLING =
        0x00, /*!< Felica Poll/REQC command (aka SENSF_REQ) to identify a card    */
    FELICA_CMD_POLLING_RES =
        0x01, /*!< Felica Poll/REQC command (aka SENSF_RES) response              */
    FELICA_CMD_REQUEST_SERVICE =
        0x02, /*!< verify the existence of Area and Service                       */
    FELICA_CMD_REQUEST_RESPONSE =
        0x04, /*!< verify the existence of a card                                 */
    FELICA_CMD_READ_WITHOUT_ENCRYPTION =
        0x06, /*!< read Block Data from a Service that requires no authentication */
    FELICA_CMD_WRITE_WITHOUT_ENCRYPTION =
        0x08, /*!< write Block Data to a Service that requires no authentication  */
    FELICA_CMD_REQUEST_SYSTEM_CODE =
        0x0C, /*!< acquire the System Code registered to a card                   */
    FELICA_CMD_AUTHENTICATION1 =
        0x10, /*!< authenticate a card                                            */
    FELICA_CMD_AUTHENTICATION2 =
        0x12, /*!< allow a card to authenticate a Reader/Writer                   */
    FELICA_CMD_READ = 0x14, /*!< read Block Data from a Service that requires authentication    */
    FELICA_CMD_WRITE = 0x16, /*!< write Block Data to a Service that requires authentication     */
} t_rfalFeliCaCmd;

/*! Union representing all PTMem sections */
typedef union { /*  PRQA S 0750 # MISRA 19.2 - Both members are of the same type, just different names.  Thus no problem can occur. */
    uint8_t PTMem_A
        [ST25R3916_PTM_A_LEN]; /*!< PT_Memory area allocated for NFC-A configuration               */
    uint8_t PTMem_F
        [ST25R3916_PTM_F_LEN]; /*!< PT_Memory area allocated for NFC-F configuration               */
    uint8_t
        TSN[ST25R3916_PTM_TSN_LEN]; /*!< PT_Memory area allocated for TSN - Random numbers              */
} t_rfalPTMem;

/*
******************************************************************************
* GLOBAL DEFINES
******************************************************************************
*/

#define RFAL_FIFO_IN_WL \
    200U /*!< Number of bytes in the FIFO when WL interrupt occurs while Tx                   */
#define RFAL_FIFO_OUT_WL    \
    (ST25R3916_FIFO_DEPTH - \
     RFAL_FIFO_IN_WL) /*!< Number of bytes sent/out of the FIFO when WL interrupt occurs while Tx          */

#define RFAL_FIFO_STATUS_REG1 \
    0U /*!< Location of FIFO status register 1 in local copy                                */
#define RFAL_FIFO_STATUS_REG2 \
    1U /*!< Location of FIFO status register 2 in local copy                                */
#define RFAL_FIFO_STATUS_INVALID \
    0xFFU /*!< Value indicating that the local FIFO status in invalid|cleared                  */

#define RFAL_ST25R3916_GPT_MAX_1FC \
    rfalConv8fcTo1fc(              \
        0xFFFFU) /*!< Max GPT steps in 1fc (0xFFFF steps of 8/fc    => 0xFFFF * 590ns  = 38,7ms)      */
#define RFAL_ST25R3916_NRT_MAX_1FC \
    rfalConv4096fcTo1fc(           \
        0xFFFFU) /*!< Max NRT steps in 1fc (0xFFFF steps of 4096/fc => 0xFFFF * 302us  = 19.8s )      */
#define RFAL_ST25R3916_NRT_DISABLED \
    0U /*!< NRT Disabled: All 0 No-response timer is not started, wait forever              */
#define RFAL_ST25R3916_MRT_MAX_1FC \
    rfalConv64fcTo1fc(             \
        0x00FFU) /*!< Max MRT steps in 1fc (0x00FF steps of 64/fc   => 0x00FF * 4.72us = 1.2ms )      */
#define RFAL_ST25R3916_MRT_MIN_1FC \
    rfalConv64fcTo1fc(             \
        0x0004U) /*!< Min MRT steps in 1fc ( 0<=mrt<=4 ; 4 (64/fc)  => 0x0004 * 4.72us = 18.88us )    */
#define RFAL_ST25R3916_GT_MAX_1FC \
    rfalConvMsTo1fc(              \
        6000U) /*!< Max GT value allowed in 1/fc (SFGI=14 => SFGT + dSFGT = 5.4s)                   */
#define RFAL_ST25R3916_GT_MIN_1FC \
    rfalConvMsTo1fc(              \
        RFAL_ST25R3916_SW_TMR_MIN_1MS) /*!< Min GT value allowed in 1/fc                                                    */
#define RFAL_ST25R3916_SW_TMR_MIN_1MS \
    1U /*!< Min value of a SW timer in ms                                                   */

#define RFAL_OBSMODE_DISABLE \
    0x00U /*!< Observation Mode disabled                                                       */

#define RFAL_RX_INCOMPLETE_MAXLEN \
    (uint8_t)1U /*!< Threshold value where incoming rx may be considered as incomplete               */
#define RFAL_EMVCO_RX_MAXLEN \
    (uint8_t)4U /*!< Maximum value where EMVCo to apply special error handling                       */

#define RFAL_NORXE_TOUT \
    50U /*!< Timeout to be used on a potential missing RXE - Silicon ST25R3916 Errata #TBD   */

#define RFAL_ISO14443A_SDD_RES_LEN \
    5U /*!< SDD_RES | Anticollision (UID CLn) length  -  rfalNfcaSddRes                     */
#define RFAL_ISO14443A_CRC_INTVAL \
    0x6363 /*!< ISO14443 CRC Initial Value|Register                                             */

#define RFAL_FELICA_POLL_DELAY_TIME \
    512U /*!<  FeliCa Poll Processing time is 2.417 ms ~512*64/fc Digital 1.1 A4              */
#define RFAL_FELICA_POLL_SLOT_TIME \
    256U /*!<  FeliCa Poll Time Slot duration is 1.208 ms ~256*64/fc Digital 1.1 A4           */

#define RFAL_LM_SENSF_RD0_POS \
    17U /*!<  FeliCa SENSF_RES Request Data RD0 position                                     */
#define RFAL_LM_SENSF_RD1_POS \
    18U /*!<  FeliCa SENSF_RES Request Data RD1 position                                     */

#define RFAL_LM_NFCID_INCOMPLETE \
    0x04U /*!<  NFCA NFCID not complete bit in SEL_RES (SAK)                                   */

#define RFAL_ISO15693_IGNORE_BITS \
    rfalConvBytesToBits(          \
        2U) /*!< Ignore collisions before the UID (RES_FLAG + DSFID)                             */
#define RFAL_ISO15693_INV_RES_LEN \
    12U /*!< ISO15693 Inventory response length with CRC (bytes)                             */
#define RFAL_ISO15693_INV_RES_DUR \
    4U /*!< ISO15693 Inventory response duration @ 26 kbps (ms)                             */

#define RFAL_WU_MIN_WEIGHT_VAL \
    4U /*!< ST25R3916 minimum Wake-up weight value                                         */

/*******************************************************************************/

#define RFAL_LM_GT   \
    rfalConvUsTo1fc( \
        100U) /*!< Listen Mode Guard Time enforced (GT - Passive; TIRFG - Active)                  */
#define RFAL_FDT_POLL_ADJUSTMENT \
    rfalConvUsTo1fc(             \
        80U) /*!< FDT Poll adjustment: Time between the expiration of GPT to the actual Tx        */
#define RFAL_FDT_LISTEN_MRT_ADJUSTMENT \
    64U /*!< MRT jitter adjustment: timeout will be between [ tout ; tout + 64 cycles ]      */
#define RFAL_AP2P_FIELDOFF_TRFW \
    rfalConv8fcTo1fc(           \
        64U) /*!< Time after TXE and Field Off in AP2P Trfw: 37.76us -> 64  (8/fc)                */

#ifndef RFAL_ST25R3916_AAT_SETTLE
#define RFAL_ST25R3916_AAT_SETTLE \
    5U /*!< Time in ms required for AAT pins and Osc to settle after en bit set */
#endif /* RFAL_ST25R3916_AAT_SETTLE */

/*! FWT adjustment: 
 *    64 : NRT jitter between TXE and NRT start      */
#define RFAL_FWT_ADJUSTMENT 64U

/*! FWT ISO14443A adjustment:  
 *   512  : 4bit length
 *    64  : Half a bit duration due to ST25R3916 Coherent receiver (1/fc)         */
#define RFAL_FWT_A_ADJUSTMENT (512U + 64U)

/*! FWT ISO14443B adjustment:  
 *    SOF (14etu) + 1Byte (10etu) + 1etu (IRQ comes 1etu after first byte) - 3etu (ST25R3916 sends TXE 3etu after) */
#define RFAL_FWT_B_ADJUSTMENT ((14U + 10U + 1U - 3U) * 128U)

/*! FWT FeliCa 212 adjustment:  
 *    1024 : Length of the two Sync bytes at 212kbps */
#define RFAL_FWT_F_212_ADJUSTMENT 1024U

/*! FWT FeliCa 424 adjustment:  
 *    512 : Length of the two Sync bytes at 424kbps  */
#define RFAL_FWT_F_424_ADJUSTMENT 512U

/*! Time between our field Off and other peer field On : Tadt + (n x Trfw)
 * Ecma 340 11.1.2 - Tadt: [56.64 , 188.72] us ;  n: [0 , 3]  ; Trfw = 37.76 us        
 * Should be: 189 + (3*38) = 303us ; we'll use a more relaxed setting: 605 us    */
#define RFAL_AP2P_FIELDON_TADTTRFW rfalConvUsTo1fc(605U)

/*! FDT Listen adjustment for ISO14443A   EMVCo 2.6  4.8.1.3  ;  Digital 1.1  6.10
 *
 *  276: Time from the rising pulse of the pause of the logic '1' (i.e. the time point to measure the deaftime from), 
 *       to the actual end of the EOF sequence (the point where the MRT starts). Please note that the ST25R391x uses the 
 *       ISO14443-2 definition where the EOF consists of logic '0' followed by sequence Y. 
 *  -64: Further adjustment for receiver to be ready just before first bit
 */
#define RFAL_FDT_LISTEN_A_ADJUSTMENT (276U - 64U)

/*! FDT Listen adjustment for ISO14443B   EMVCo 2.6  4.8.1.6  ;  Digital 1.1  7.9
 *
 *  340: Time from the rising edge of the EoS to the starting point of the MRT timer (sometime after the final high 
 *       part of the EoS is completed)
 */
#define RFAL_FDT_LISTEN_B_ADJUSTMENT 340U

/*! FDT Listen adjustment for ISO15693
 * ISO15693 2000  8.4  t1 MIN = 4192/fc
 * ISO15693 2009  9.1  t1 MIN = 4320/fc
 * Digital 2.1 B.5 FDTV,LISTEN,MIN  = 4310/fc
 * Set FDT Listen one step earlier than on the more recent spec versions for greater interoperability
 */
#define RFAL_FDT_LISTEN_V_ADJUSTMENT 64U

/*! FDT Poll adjustment for ISO14443B Correlator - sst 5 etu */
#define RFAL_FDT_LISTEN_B_ADJT_CORR 128U

/*! FDT Poll adjustment for ISO14443B Correlator sst window - 5 etu */
#define RFAL_FDT_LISTEN_B_ADJT_CORR_SST 20U

/*
******************************************************************************
* GLOBAL MACROS
******************************************************************************
*/

/*! Calculates Transceive Sanity Timer. It accounts for the slowest bit rate and the longest data format
 *    1s for transmission and reception of a 4K message at 106kpbs (~425ms each direction)
 *       plus TxRx preparation and FIFO load over Serial Interface                                      */
#define rfalCalcSanityTmr(fwt) (uint16_t)(1000U + rfalConv1fcToMs((fwt)))

#define rfalGennTRFW(n) \
    (((n) + 1U) &       \
     ST25R3916_REG_AUX_nfc_n_mask) /*!< Generates the next n*TRRW used for RFCA       */

#define rfalCalcNumBytes(nBits) \
    (((uint32_t)(nBits) + 7U) / \
     8U) /*!< Returns the number of bytes required to fit given the number of bits */

#define rfalTimerStart(timer, time_ms)                      \
    do {                                                    \
        platformTimerDestroy(timer);                        \
        (timer) = platformTimerCreate((uint16_t)(time_ms)); \
    } while(0) /*!< Configures and starts timer         */
#define rfalTimerisExpired(timer) \
    platformTimerIsExpired(       \
        timer) /*!< Checks if timer has expired                                         */
#define rfalTimerDestroy(timer) \
    platformTimerDestroy(       \
        timer) /*!< Destroys timer                                                      */

#define rfalST25R3916ObsModeDisable() \
    st25r3916WriteTestRegister(       \
        0x01U,                        \
        (0x40U)) /*!< Disable ST25R3916 Observation mode                                   */
#define rfalST25R3916ObsModeTx() \
    st25r3916WriteTestRegister(  \
        0x01U,                   \
        (0x40U |                 \
         gRFAL.conf              \
             .obsvModeTx)) /*!< Enable Tx Observation mode                                           */
#define rfalST25R3916ObsModeRx() \
    st25r3916WriteTestRegister(  \
        0x01U,                   \
        (0x40U |                 \
         gRFAL.conf              \
             .obsvModeRx)) /*!< Enable Rx Observation mode                                           */

#define rfalCheckDisableObsMode()      \
    if(gRFAL.conf.obsvModeRx != 0U) {  \
        rfalST25R3916ObsModeDisable(); \
    } /*!< Checks if the observation mode is enabled, and applies on ST25R3916  */
#define rfalCheckEnableObsModeTx()    \
    if(gRFAL.conf.obsvModeTx != 0U) { \
        rfalST25R3916ObsModeTx();     \
    } /*!< Checks if the observation mode is enabled, and applies on ST25R3916  */
#define rfalCheckEnableObsModeRx()    \
    if(gRFAL.conf.obsvModeRx != 0U) { \
        rfalST25R3916ObsModeRx();     \
    } /*!< Checks if the observation mode is enabled, and applies on ST25R3916  */

#define rfalGetIncmplBits(FIFOStatus2) \
    (((FIFOStatus2) >> 1) &            \
     0x07U) /*!< Returns the number of bits from fifo status                  */
#define rfalIsIncompleteByteError(error) \
    (((error) >= ERR_INCOMPLETE_BYTE) && \
     ((error) <=                         \
      ERR_INCOMPLETE_BYTE_07)) /*!< Checks if given error is a Incomplete error                  */

#define rfalAdjACBR(b)                            \
    (((uint16_t)(b) >= (uint16_t)RFAL_BR_52p97) ? \
         (uint16_t)(b) :                          \
         ((uint16_t)(b) +                         \
          1U)) /*!< Adjusts ST25R391x Bit rate to Analog Configuration              */
#define rfalConvBR2ACBR(b)                                      \
    (((rfalAdjACBR((b))) << RFAL_ANALOG_CONFIG_BITRATE_SHIFT) & \
     RFAL_ANALOG_CONFIG_BITRATE_MASK) /*!< Converts ST25R391x Bit rate to Analog Configuration bit rate id */

#define rfalConvTDFormat(v) \
    ((uint16_t)(v) << 8U) /*!< Converts a uint8_t to the format used in SW Tag Detection */

/*
 ******************************************************************************
 * LOCAL VARIABLES
 ******************************************************************************
 */

static rfal gRFAL; /*!< RFAL module instance               */

/*
******************************************************************************
* LOCAL FUNCTION PROTOTYPES
******************************************************************************
*/

static void rfalTransceiveTx(void);
static void rfalTransceiveRx(void);
static ReturnCode rfalTransceiveRunBlockingTx(void);
static void rfalPrepareTransceive(void);
static void rfalCleanupTransceive(void);
static void rfalErrorHandling(void);

static ReturnCode rfalRunTransceiveWorker(void);
#if RFAL_FEATURE_LISTEN_MODE
static ReturnCode rfalRunListenModeWorker(void);
#endif /* RFAL_FEATURE_LISTEN_MODE */
#if RFAL_FEATURE_WAKEUP_MODE
static void rfalRunWakeUpModeWorker(void);
static uint16_t rfalWakeUpModeFilter(uint16_t curRef, uint16_t curVal, uint8_t weight);
#endif /* RFAL_FEATURE_WAKEUP_MODE */

static void rfalFIFOStatusUpdate(void);
static void rfalFIFOStatusClear(void);
static bool rfalFIFOStatusIsMissingPar(void);
static bool rfalFIFOStatusIsIncompleteByte(void);
static uint16_t rfalFIFOStatusGetNumBytes(void);
static uint8_t rfalFIFOGetNumIncompleteBits(void);

/*
******************************************************************************
* GLOBAL FUNCTIONS
******************************************************************************
*/

/*******************************************************************************/
ReturnCode rfalInitialize(void) {
    ReturnCode err;

    EXIT_ON_ERR(err, st25r3916Initialize());

    st25r3916ClearInterrupts();

    /* Disable any previous observation mode */
    rfalST25R3916ObsModeDisable();

    /*******************************************************************************/
    /* Apply RF Chip generic initialization */
    rfalSetAnalogConfig((RFAL_ANALOG_CONFIG_TECH_CHIP | RFAL_ANALOG_CONFIG_CHIP_INIT));

    // TODO:
    // I don't want to mess with config table ("Default Analog Configuration for Chip-Specific Reset", rfal_analogConfigTbl.h)
    // so with every rfalSetAnalogConfig((RFAL_ANALOG_CONFIG_CHIP_INIT)) currently we need to clear pulldown bits
    // luckily for us this is done only here

    // disable pulldowns
    st25r3916ClrRegisterBits(
        ST25R3916_REG_IO_CONF2,
        (ST25R3916_REG_IO_CONF2_miso_pd1 | ST25R3916_REG_IO_CONF2_miso_pd2));

    /*******************************************************************************/
    /* Enable External Field Detector as: Automatics */
    st25r3916ChangeRegisterBits(
        ST25R3916_REG_OP_CONTROL,
        ST25R3916_REG_OP_CONTROL_en_fd_mask,
        ST25R3916_REG_OP_CONTROL_en_fd_auto_efd);

    /* Clear FIFO status local copy */
    rfalFIFOStatusClear();

    /*******************************************************************************/
    gRFAL.state = RFAL_STATE_INIT;
    gRFAL.mode = RFAL_MODE_NONE;
    gRFAL.field = false;

    /* Set RFAL default configs */
    gRFAL.conf.obsvModeRx = RFAL_OBSMODE_DISABLE;
    gRFAL.conf.obsvModeTx = RFAL_OBSMODE_DISABLE;
    gRFAL.conf.eHandling = RFAL_ERRORHANDLING_NONE;

    /* Transceive set to IDLE */
    gRFAL.TxRx.lastState = RFAL_TXRX_STATE_IDLE;
    gRFAL.TxRx.state = RFAL_TXRX_STATE_IDLE;

    /* Disable all timings */
    gRFAL.timings.FDTListen = RFAL_TIMING_NONE;
    gRFAL.timings.FDTPoll = RFAL_TIMING_NONE;
    gRFAL.timings.GT = RFAL_TIMING_NONE;
    gRFAL.timings.nTRFW = 0U;

    /* Destroy any previous pending timers */
    rfalTimerDestroy(gRFAL.tmr.GT);
    rfalTimerDestroy(gRFAL.tmr.txRx);
    rfalTimerDestroy(gRFAL.tmr.RXE);
    gRFAL.tmr.GT = RFAL_TIMING_NONE;
    gRFAL.tmr.txRx = RFAL_TIMING_NONE;
    gRFAL.tmr.RXE = RFAL_TIMING_NONE;

    gRFAL.callbacks.preTxRx = NULL;
    gRFAL.callbacks.postTxRx = NULL;
    gRFAL.callbacks.state_changed_cb = NULL;
    gRFAL.callbacks.ctx = NULL;

#if RFAL_FEATURE_NFCV
    /* Initialize NFC-V Data */
    gRFAL.nfcvData.ignoreBits = 0;
#endif /* RFAL_FEATURE_NFCV */

#if RFAL_FEATURE_LISTEN_MODE
    /* Initialize Listen Mode */
    gRFAL.Lm.state = RFAL_LM_STATE_NOT_INIT;
    gRFAL.Lm.brDetected = RFAL_BR_KEEP;
    gRFAL.Lm.iniFlag = false;
#endif /* RFAL_FEATURE_LISTEN_MODE */

#if RFAL_FEATURE_WAKEUP_MODE
    /* Initialize Wake-Up Mode */
    gRFAL.wum.state = RFAL_WUM_STATE_NOT_INIT;
#endif /* RFAL_FEATURE_WAKEUP_MODE */

#if RFAL_FEATURE_LOWPOWER_MODE
    /* Initialize Low Power Mode */
    gRFAL.lpm.isRunning = false;
#endif /* RFAL_FEATURE_LOWPOWER_MODE */

    /*******************************************************************************/
    /* Perform Automatic Calibration (if configured to do so).                     *
     * Registers set by rfalSetAnalogConfig will tell rfalCalibrate what to perform*/
    rfalCalibrate();

    return ERR_NONE;
}

/*******************************************************************************/
ReturnCode rfalCalibrate(void) {
    uint16_t resValue;

    /* Check if RFAL is not initialized */
    if(gRFAL.state == RFAL_STATE_IDLE) {
        return ERR_WRONG_STATE;
    }

    /*******************************************************************************/
    /* Perform ST25R3916 regulators and antenna calibration                        */
    /*******************************************************************************/

    /* Automatic regulator adjustment only performed if not set manually on Analog Configs */
    if(st25r3916CheckReg(
           ST25R3916_REG_REGULATOR_CONTROL, ST25R3916_REG_REGULATOR_CONTROL_reg_s, 0x00)) {
        /* Adjust the regulators so that Antenna Calibrate has better Regulator values */
        st25r3916AdjustRegulators(&resValue);
    }

    return ERR_NONE;
}

/*******************************************************************************/
ReturnCode rfalAdjustRegulators(uint16_t* result) {
    return st25r3916AdjustRegulators(result);
}

/*******************************************************************************/
void rfalSetUpperLayerCallback(rfalUpperLayerCallback pFunc) {
    st25r3916IRQCallbackSet(pFunc);
}

/*******************************************************************************/
void rfalSetPreTxRxCallback(rfalPreTxRxCallback pFunc) {
    gRFAL.callbacks.preTxRx = pFunc;
}

/*******************************************************************************/
void rfalSetPostTxRxCallback(rfalPostTxRxCallback pFunc) {
    gRFAL.callbacks.postTxRx = pFunc;
}

void rfal_set_state_changed_callback(RfalStateChangedCallback callback) {
    gRFAL.callbacks.state_changed_cb = callback;
}

void rfal_set_callback_context(void* context) {
    gRFAL.callbacks.ctx = context;
}

/*******************************************************************************/
ReturnCode rfalDeinitialize(void) {
    /* Deinitialize chip */
    st25r3916Deinitialize();

    /* Set Analog configurations for deinitialization */
    rfalSetAnalogConfig((RFAL_ANALOG_CONFIG_TECH_CHIP | RFAL_ANALOG_CONFIG_CHIP_DEINIT));

    gRFAL.state = RFAL_STATE_IDLE;
    return ERR_NONE;
}

/*******************************************************************************/
void rfalSetObsvMode(uint8_t txMode, uint8_t rxMode) {
    gRFAL.conf.obsvModeTx = txMode;
    gRFAL.conf.obsvModeRx = rxMode;
}

/*******************************************************************************/
void rfalGetObsvMode(uint8_t* txMode, uint8_t* rxMode) {
    if(txMode != NULL) {
        *txMode = gRFAL.conf.obsvModeTx;
    }

    if(rxMode != NULL) {
        *rxMode = gRFAL.conf.obsvModeRx;
    }
}

/*******************************************************************************/
void rfalDisableObsvMode(void) {
    gRFAL.conf.obsvModeTx = RFAL_OBSMODE_DISABLE;
    gRFAL.conf.obsvModeRx = RFAL_OBSMODE_DISABLE;
}

/*******************************************************************************/
ReturnCode rfalSetMode(rfalMode mode, rfalBitRate txBR, rfalBitRate rxBR) {
    /* Check if RFAL is not initialized */
    if(gRFAL.state == RFAL_STATE_IDLE) {
        return ERR_WRONG_STATE;
    }

    /* Check allowed bit rate value */
    if((txBR == RFAL_BR_KEEP) || (rxBR == RFAL_BR_KEEP)) {
        return ERR_PARAM;
    }

    switch(mode) {
    /*******************************************************************************/
    case RFAL_MODE_POLL_NFCA:

        /* Disable wake up mode, if set */
        st25r3916ClrRegisterBits(ST25R3916_REG_OP_CONTROL, ST25R3916_REG_OP_CONTROL_wu);

        /* Enable ISO14443A mode */
        st25r3916WriteRegister(ST25R3916_REG_MODE, ST25R3916_REG_MODE_om_iso14443a);

        /* Set Analog configurations for this mode and bit rate */
        rfalSetAnalogConfig(
            (RFAL_ANALOG_CONFIG_POLL | RFAL_ANALOG_CONFIG_TECH_NFCA |
             RFAL_ANALOG_CONFIG_BITRATE_COMMON | RFAL_ANALOG_CONFIG_TX));
        rfalSetAnalogConfig(
            (RFAL_ANALOG_CONFIG_POLL | RFAL_ANALOG_CONFIG_TECH_NFCA |
             RFAL_ANALOG_CONFIG_BITRATE_COMMON | RFAL_ANALOG_CONFIG_RX));
        break;

    /*******************************************************************************/
    case RFAL_MODE_POLL_NFCA_T1T:
        /* Disable wake up mode, if set */
        st25r3916ClrRegisterBits(ST25R3916_REG_OP_CONTROL, ST25R3916_REG_OP_CONTROL_wu);

        /* Enable Topaz mode */
        st25r3916WriteRegister(ST25R3916_REG_MODE, ST25R3916_REG_MODE_om_topaz);

        /* Set Analog configurations for this mode and bit rate */
        rfalSetAnalogConfig(
            (RFAL_ANALOG_CONFIG_POLL | RFAL_ANALOG_CONFIG_TECH_NFCA |
             RFAL_ANALOG_CONFIG_BITRATE_COMMON | RFAL_ANALOG_CONFIG_TX));
        rfalSetAnalogConfig(
            (RFAL_ANALOG_CONFIG_POLL | RFAL_ANALOG_CONFIG_TECH_NFCA |
             RFAL_ANALOG_CONFIG_BITRATE_COMMON | RFAL_ANALOG_CONFIG_RX));
        break;

    /*******************************************************************************/
    case RFAL_MODE_POLL_NFCB:

        /* Disable wake up mode, if set */
        st25r3916ClrRegisterBits(ST25R3916_REG_OP_CONTROL, ST25R3916_REG_OP_CONTROL_wu);

        /* Enable ISO14443B mode */
        st25r3916WriteRegister(ST25R3916_REG_MODE, ST25R3916_REG_MODE_om_iso14443b);

        /* Set the EGT, SOF, EOF and EOF */
        st25r3916ChangeRegisterBits(
            ST25R3916_REG_ISO14443B_1,
            (ST25R3916_REG_ISO14443B_1_egt_mask | ST25R3916_REG_ISO14443B_1_sof_mask |
             ST25R3916_REG_ISO14443B_1_eof),
            ((0U << ST25R3916_REG_ISO14443B_1_egt_shift) | ST25R3916_REG_ISO14443B_1_sof_0_10etu |
             ST25R3916_REG_ISO14443B_1_sof_1_2etu | ST25R3916_REG_ISO14443B_1_eof_10etu));

        /* Set the minimum TR1, SOF, EOF and EOF12 */
        st25r3916ChangeRegisterBits(
            ST25R3916_REG_ISO14443B_2,
            (ST25R3916_REG_ISO14443B_2_tr1_mask | ST25R3916_REG_ISO14443B_2_no_sof |
             ST25R3916_REG_ISO14443B_2_no_eof),
            (ST25R3916_REG_ISO14443B_2_tr1_80fs80fs));

        /* Set Analog configurations for this mode and bit rate */
        rfalSetAnalogConfig(
            (RFAL_ANALOG_CONFIG_POLL | RFAL_ANALOG_CONFIG_TECH_NFCB |
             RFAL_ANALOG_CONFIG_BITRATE_COMMON | RFAL_ANALOG_CONFIG_TX));
        rfalSetAnalogConfig(
            (RFAL_ANALOG_CONFIG_POLL | RFAL_ANALOG_CONFIG_TECH_NFCB |
             RFAL_ANALOG_CONFIG_BITRATE_COMMON | RFAL_ANALOG_CONFIG_RX));
        break;

    /*******************************************************************************/
    case RFAL_MODE_POLL_B_PRIME:

        /* Disable wake up mode, if set */
        st25r3916ClrRegisterBits(ST25R3916_REG_OP_CONTROL, ST25R3916_REG_OP_CONTROL_wu);

        /* Enable ISO14443B mode */
        st25r3916WriteRegister(ST25R3916_REG_MODE, ST25R3916_REG_MODE_om_iso14443b);

        /* Set the EGT, SOF, EOF and EOF */
        st25r3916ChangeRegisterBits(
            ST25R3916_REG_ISO14443B_1,
            (ST25R3916_REG_ISO14443B_1_egt_mask | ST25R3916_REG_ISO14443B_1_sof_mask |
             ST25R3916_REG_ISO14443B_1_eof),
            ((0U << ST25R3916_REG_ISO14443B_1_egt_shift) | ST25R3916_REG_ISO14443B_1_sof_0_10etu |
             ST25R3916_REG_ISO14443B_1_sof_1_2etu | ST25R3916_REG_ISO14443B_1_eof_10etu));

        /* Set the minimum TR1, EOF and EOF12 */
        st25r3916ChangeRegisterBits(
            ST25R3916_REG_ISO14443B_2,
            (ST25R3916_REG_ISO14443B_2_tr1_mask | ST25R3916_REG_ISO14443B_2_no_sof |
             ST25R3916_REG_ISO14443B_2_no_eof),
            (ST25R3916_REG_ISO14443B_2_tr1_80fs80fs | ST25R3916_REG_ISO14443B_2_no_sof));

        /* Set Analog configurations for this mode and bit rate */
        rfalSetAnalogConfig(
            (RFAL_ANALOG_CONFIG_POLL | RFAL_ANALOG_CONFIG_TECH_NFCB |
             RFAL_ANALOG_CONFIG_BITRATE_COMMON | RFAL_ANALOG_CONFIG_TX));
        rfalSetAnalogConfig(
            (RFAL_ANALOG_CONFIG_POLL | RFAL_ANALOG_CONFIG_TECH_NFCB |
             RFAL_ANALOG_CONFIG_BITRATE_COMMON | RFAL_ANALOG_CONFIG_RX));
        break;

    /*******************************************************************************/
    case RFAL_MODE_POLL_B_CTS:

        /* Disable wake up mode, if set */
        st25r3916ClrRegisterBits(ST25R3916_REG_OP_CONTROL, ST25R3916_REG_OP_CONTROL_wu);

        /* Enable ISO14443B mode */
        st25r3916WriteRegister(ST25R3916_REG_MODE, ST25R3916_REG_MODE_om_iso14443b);

        /* Set the EGT, SOF, EOF and EOF */
        st25r3916ChangeRegisterBits(
            ST25R3916_REG_ISO14443B_1,
            (ST25R3916_REG_ISO14443B_1_egt_mask | ST25R3916_REG_ISO14443B_1_sof_mask |
             ST25R3916_REG_ISO14443B_1_eof),
            ((0U << ST25R3916_REG_ISO14443B_1_egt_shift) | ST25R3916_REG_ISO14443B_1_sof_0_10etu |
             ST25R3916_REG_ISO14443B_1_sof_1_2etu | ST25R3916_REG_ISO14443B_1_eof_10etu));

        /* Set the minimum TR1, clear SOF, EOF and EOF12 */
        st25r3916ChangeRegisterBits(
            ST25R3916_REG_ISO14443B_2,
            (ST25R3916_REG_ISO14443B_2_tr1_mask | ST25R3916_REG_ISO14443B_2_no_sof |
             ST25R3916_REG_ISO14443B_2_no_eof),
            (ST25R3916_REG_ISO14443B_2_tr1_80fs80fs | ST25R3916_REG_ISO14443B_2_no_sof |
             ST25R3916_REG_ISO14443B_2_no_eof));

        /* Set Analog configurations for this mode and bit rate */
        rfalSetAnalogConfig(
            (RFAL_ANALOG_CONFIG_POLL | RFAL_ANALOG_CONFIG_TECH_NFCB |
             RFAL_ANALOG_CONFIG_BITRATE_COMMON | RFAL_ANALOG_CONFIG_TX));
        rfalSetAnalogConfig(
            (RFAL_ANALOG_CONFIG_POLL | RFAL_ANALOG_CONFIG_TECH_NFCB |
             RFAL_ANALOG_CONFIG_BITRATE_COMMON | RFAL_ANALOG_CONFIG_RX));
        break;

    /*******************************************************************************/
    case RFAL_MODE_POLL_NFCF:

        /* Disable wake up mode, if set */
        st25r3916ClrRegisterBits(ST25R3916_REG_OP_CONTROL, ST25R3916_REG_OP_CONTROL_wu);

        /* Enable FeliCa mode */
        st25r3916WriteRegister(ST25R3916_REG_MODE, ST25R3916_REG_MODE_om_felica);

        /* Set Analog configurations for this mode and bit rate */
        rfalSetAnalogConfig(
            (RFAL_ANALOG_CONFIG_POLL | RFAL_ANALOG_CONFIG_TECH_NFCF |
             RFAL_ANALOG_CONFIG_BITRATE_COMMON | RFAL_ANALOG_CONFIG_TX));
        rfalSetAnalogConfig(
            (RFAL_ANALOG_CONFIG_POLL | RFAL_ANALOG_CONFIG_TECH_NFCF |
             RFAL_ANALOG_CONFIG_BITRATE_COMMON | RFAL_ANALOG_CONFIG_RX));
        break;

    /*******************************************************************************/
    case RFAL_MODE_POLL_NFCV:
    case RFAL_MODE_POLL_PICOPASS:

#if !RFAL_FEATURE_NFCV
        return ERR_DISABLED;
#else

        /* Disable wake up mode, if set */
        st25r3916ClrRegisterBits(ST25R3916_REG_OP_CONTROL, ST25R3916_REG_OP_CONTROL_wu);

        /* Set Analog configurations for this mode and bit rate */
        rfalSetAnalogConfig(
            (RFAL_ANALOG_CONFIG_POLL | RFAL_ANALOG_CONFIG_TECH_NFCV |
             RFAL_ANALOG_CONFIG_BITRATE_COMMON | RFAL_ANALOG_CONFIG_TX));
        rfalSetAnalogConfig(
            (RFAL_ANALOG_CONFIG_POLL | RFAL_ANALOG_CONFIG_TECH_NFCV |
             RFAL_ANALOG_CONFIG_BITRATE_COMMON | RFAL_ANALOG_CONFIG_RX));
        break;

#endif /* RFAL_FEATURE_NFCV */

    /*******************************************************************************/
    case RFAL_MODE_POLL_ACTIVE_P2P:

        /* Set NFCIP1 active communication Initiator mode and Automatic Response RF Collision Avoidance to always after EOF */
        st25r3916WriteRegister(
            ST25R3916_REG_MODE,
            (ST25R3916_REG_MODE_targ_init | ST25R3916_REG_MODE_om_nfc |
             ST25R3916_REG_MODE_nfc_ar_eof));

        /* External Field Detector enabled as Automatics on rfalInitialize() */

        /* Set NRT to start at end of TX (own) field */
        st25r3916ChangeRegisterBits(
            ST25R3916_REG_TIMER_EMV_CONTROL,
            ST25R3916_REG_TIMER_EMV_CONTROL_nrt_nfc,
            ST25R3916_REG_TIMER_EMV_CONTROL_nrt_nfc_off);

        /* Set GPT to start after end of TX, as GPT is used in active communication mode to timeout the field switching off */
        /* The field is turned off 37.76us after the end of the transmission  Trfw                                          */
        st25r3916SetStartGPTimer(
            (uint16_t)rfalConv1fcTo8fc(RFAL_AP2P_FIELDOFF_TRFW),
            ST25R3916_REG_TIMER_EMV_CONTROL_gptc_etx_nfc);

        /* Set PPon2 timer with the max time between our field Off and other peer field On : Tadt + (n x Trfw)    */
        st25r3916WriteRegister(
            ST25R3916_REG_PPON2, (uint8_t)rfalConv1fcTo64fc(RFAL_AP2P_FIELDON_TADTTRFW));

        /* Set Analog configurations for this mode and bit rate */
        rfalSetAnalogConfig(
            (RFAL_ANALOG_CONFIG_POLL | RFAL_ANALOG_CONFIG_TECH_AP2P |
             RFAL_ANALOG_CONFIG_BITRATE_COMMON | RFAL_ANALOG_CONFIG_TX));
        rfalSetAnalogConfig(
            (RFAL_ANALOG_CONFIG_POLL | RFAL_ANALOG_CONFIG_TECH_AP2P |
             RFAL_ANALOG_CONFIG_BITRATE_COMMON | RFAL_ANALOG_CONFIG_RX));
        break;

    /*******************************************************************************/
    case RFAL_MODE_LISTEN_ACTIVE_P2P:

        /* Set NFCIP1 active communication Target mode and Automatic Response RF Collision Avoidance to always after EOF */
        st25r3916WriteRegister(
            ST25R3916_REG_MODE,
            (ST25R3916_REG_MODE_targ_targ | ST25R3916_REG_MODE_om_targ_nfcip |
             ST25R3916_REG_MODE_nfc_ar_eof));

        /* Set TARFG: 0 (75us+0ms=75us), as Target no Guard time needed */
        st25r3916WriteRegister(ST25R3916_REG_FIELD_ON_GT, 0U);

        /* External Field Detector enabled as Automatics on rfalInitialize() */

        /* Set NRT to start at end of TX (own) field */
        st25r3916ChangeRegisterBits(
            ST25R3916_REG_TIMER_EMV_CONTROL,
            ST25R3916_REG_TIMER_EMV_CONTROL_nrt_nfc,
            ST25R3916_REG_TIMER_EMV_CONTROL_nrt_nfc_off);

        /* Set GPT to start after end of TX, as GPT is used in active communication mode to timeout the field switching off */
        /* The field is turned off 37.76us after the end of the transmission  Trfw                                          */
        st25r3916SetStartGPTimer(
            (uint16_t)rfalConv1fcTo8fc(RFAL_AP2P_FIELDOFF_TRFW),
            ST25R3916_REG_TIMER_EMV_CONTROL_gptc_etx_nfc);

        /* Set PPon2 timer with the max time between our field Off and other peer field On : Tadt + (n x Trfw)    */
        st25r3916WriteRegister(
            ST25R3916_REG_PPON2, (uint8_t)rfalConv1fcTo64fc(RFAL_AP2P_FIELDON_TADTTRFW));

        /* Set Analog configurations for this mode and bit rate */
        rfalSetAnalogConfig(
            (RFAL_ANALOG_CONFIG_LISTEN | RFAL_ANALOG_CONFIG_TECH_AP2P |
             RFAL_ANALOG_CONFIG_BITRATE_COMMON | RFAL_ANALOG_CONFIG_TX));
        rfalSetAnalogConfig(
            (RFAL_ANALOG_CONFIG_LISTEN | RFAL_ANALOG_CONFIG_TECH_AP2P |
             RFAL_ANALOG_CONFIG_BITRATE_COMMON | RFAL_ANALOG_CONFIG_RX));
        break;

    /*******************************************************************************/
    case RFAL_MODE_LISTEN_NFCA:

        /* Disable wake up mode, if set */
        st25r3916ClrRegisterBits(ST25R3916_REG_OP_CONTROL, ST25R3916_REG_OP_CONTROL_wu);

        /* Enable Passive Target NFC-A mode, disable any Collision Avoidance */
        st25r3916WriteRegister(
            ST25R3916_REG_MODE,
            (ST25R3916_REG_MODE_targ | ST25R3916_REG_MODE_om_targ_nfca |
             ST25R3916_REG_MODE_nfc_ar_off));

        /* Set Analog configurations for this mode */
        rfalSetAnalogConfig(
            (RFAL_ANALOG_CONFIG_LISTEN | RFAL_ANALOG_CONFIG_TECH_NFCA |
             RFAL_ANALOG_CONFIG_BITRATE_COMMON | RFAL_ANALOG_CONFIG_TX));
        rfalSetAnalogConfig(
            (RFAL_ANALOG_CONFIG_LISTEN | RFAL_ANALOG_CONFIG_TECH_NFCA |
             RFAL_ANALOG_CONFIG_BITRATE_COMMON | RFAL_ANALOG_CONFIG_RX));
        break;

    /*******************************************************************************/
    case RFAL_MODE_LISTEN_NFCF:

        /* Disable wake up mode, if set */
        st25r3916ClrRegisterBits(ST25R3916_REG_OP_CONTROL, ST25R3916_REG_OP_CONTROL_wu);

        /* Enable Passive Target NFC-F mode, disable any Collision Avoidance */
        st25r3916WriteRegister(
            ST25R3916_REG_MODE,
            (ST25R3916_REG_MODE_targ | ST25R3916_REG_MODE_om_targ_nfcf |
             ST25R3916_REG_MODE_nfc_ar_off));

        /* Set Analog configurations for this mode */
        rfalSetAnalogConfig(
            (RFAL_ANALOG_CONFIG_LISTEN | RFAL_ANALOG_CONFIG_TECH_NFCF |
             RFAL_ANALOG_CONFIG_BITRATE_COMMON | RFAL_ANALOG_CONFIG_TX));
        rfalSetAnalogConfig(
            (RFAL_ANALOG_CONFIG_LISTEN | RFAL_ANALOG_CONFIG_TECH_NFCF |
             RFAL_ANALOG_CONFIG_BITRATE_COMMON | RFAL_ANALOG_CONFIG_RX));
        break;

    /*******************************************************************************/
    case RFAL_MODE_LISTEN_NFCB:
        return ERR_NOTSUPP;

    /*******************************************************************************/
    default:
        return ERR_NOT_IMPLEMENTED;
    }

    /* Set state as STATE_MODE_SET only if not initialized yet (PSL) */
    gRFAL.state = ((gRFAL.state < RFAL_STATE_MODE_SET) ? RFAL_STATE_MODE_SET : gRFAL.state);
    gRFAL.mode = mode;

    /* Apply the given bit rate */
    return rfalSetBitRate(txBR, rxBR);
}

/*******************************************************************************/
rfalMode rfalGetMode(void) {
    return gRFAL.mode;
}

/*******************************************************************************/
ReturnCode rfalSetBitRate(rfalBitRate txBR, rfalBitRate rxBR) {
    ReturnCode ret;

    /* Check if RFAL is not initialized */
    if(gRFAL.state == RFAL_STATE_IDLE) {
        return ERR_WRONG_STATE;
    }

    /* Store the new Bit Rates */
    gRFAL.txBR = ((txBR == RFAL_BR_KEEP) ? gRFAL.txBR : txBR);
    gRFAL.rxBR = ((rxBR == RFAL_BR_KEEP) ? gRFAL.rxBR : rxBR);

    /* Update the bitrate reg if not in NFCV mode (streaming) */
    if((RFAL_MODE_POLL_NFCV != gRFAL.mode) && (RFAL_MODE_POLL_PICOPASS != gRFAL.mode)) {
        /* Set bit rate register */
        EXIT_ON_ERR(ret, st25r3916SetBitrate((uint8_t)gRFAL.txBR, (uint8_t)gRFAL.rxBR));
    }

    switch(gRFAL.mode) {
    /*******************************************************************************/
    case RFAL_MODE_POLL_NFCA:
    case RFAL_MODE_POLL_NFCA_T1T:

        /* Set Analog configurations for this bit rate */
        rfalSetAnalogConfig((RFAL_ANALOG_CONFIG_TECH_CHIP | RFAL_ANALOG_CONFIG_CHIP_POLL_COMMON));
        rfalSetAnalogConfig( (rfalAnalogConfigId)(RFAL_ANALOG_CONFIG_POLL | RFAL_ANALOG_CONFIG_TECH_NFCA | rfalConvBR2ACBR(gRFAL.txBR) | RFAL_ANALOG_CONFIG_TX ) );
        rfalSetAnalogConfig( (rfalAnalogConfigId)(RFAL_ANALOG_CONFIG_POLL | RFAL_ANALOG_CONFIG_TECH_NFCA | rfalConvBR2ACBR(gRFAL.rxBR) | RFAL_ANALOG_CONFIG_RX ) );
        break;

    /*******************************************************************************/
    case RFAL_MODE_POLL_NFCB:
    case RFAL_MODE_POLL_B_PRIME:
    case RFAL_MODE_POLL_B_CTS:

        /* Set Analog configurations for this bit rate */
        rfalSetAnalogConfig((RFAL_ANALOG_CONFIG_TECH_CHIP | RFAL_ANALOG_CONFIG_CHIP_POLL_COMMON));
        rfalSetAnalogConfig( (rfalAnalogConfigId)(RFAL_ANALOG_CONFIG_POLL | RFAL_ANALOG_CONFIG_TECH_NFCB | rfalConvBR2ACBR(gRFAL.txBR) | RFAL_ANALOG_CONFIG_TX ) );
        rfalSetAnalogConfig( (rfalAnalogConfigId)(RFAL_ANALOG_CONFIG_POLL | RFAL_ANALOG_CONFIG_TECH_NFCB | rfalConvBR2ACBR(gRFAL.rxBR) | RFAL_ANALOG_CONFIG_RX ) );
        break;

    /*******************************************************************************/
    case RFAL_MODE_POLL_NFCF:

        /* Set Analog configurations for this bit rate */
        rfalSetAnalogConfig((RFAL_ANALOG_CONFIG_TECH_CHIP | RFAL_ANALOG_CONFIG_CHIP_POLL_COMMON));
        rfalSetAnalogConfig( (rfalAnalogConfigId)(RFAL_ANALOG_CONFIG_POLL | RFAL_ANALOG_CONFIG_TECH_NFCF | rfalConvBR2ACBR(gRFAL.txBR) | RFAL_ANALOG_CONFIG_TX ) );
        rfalSetAnalogConfig( (rfalAnalogConfigId)(RFAL_ANALOG_CONFIG_POLL | RFAL_ANALOG_CONFIG_TECH_NFCF | rfalConvBR2ACBR(gRFAL.rxBR) | RFAL_ANALOG_CONFIG_RX ) );
        break;

    /*******************************************************************************/
    case RFAL_MODE_POLL_NFCV:
    case RFAL_MODE_POLL_PICOPASS:

#if !RFAL_FEATURE_NFCV
        return ERR_DISABLED;
#else

        if(((gRFAL.rxBR != RFAL_BR_26p48) && (gRFAL.rxBR != RFAL_BR_52p97)) ||
           ((gRFAL.txBR != RFAL_BR_1p66) && (gRFAL.txBR != RFAL_BR_26p48))) {
            return ERR_PARAM;
        }

        {
            const struct iso15693StreamConfig* isoStreamConfig;
            struct st25r3916StreamConfig streamConf;
            iso15693PhyConfig_t config;

            config.coding =
                ((gRFAL.txBR == RFAL_BR_1p66) ? ISO15693_VCD_CODING_1_256 :
                                                ISO15693_VCD_CODING_1_4);
            switch(gRFAL.rxBR) {
            case RFAL_BR_52p97:
                config.speedMode = 1;
                break;
            default:
                config.speedMode = 0;
                break;
            }

            iso15693PhyConfigure(&config, &isoStreamConfig);

            /* MISRA 11.3 - Cannot point directly into different object type, copy to local var */
            streamConf.din = isoStreamConfig->din;
            streamConf.dout = isoStreamConfig->dout;
            streamConf.report_period_length = isoStreamConfig->report_period_length;
            streamConf.useBPSK = isoStreamConfig->useBPSK;
            st25r3916StreamConfigure(&streamConf);
        }

        /* Set Analog configurations for this bit rate */
        rfalSetAnalogConfig((RFAL_ANALOG_CONFIG_TECH_CHIP | RFAL_ANALOG_CONFIG_CHIP_POLL_COMMON));
        rfalSetAnalogConfig( (rfalAnalogConfigId)(RFAL_ANALOG_CONFIG_POLL | RFAL_ANALOG_CONFIG_TECH_NFCV | rfalConvBR2ACBR(gRFAL.txBR) | RFAL_ANALOG_CONFIG_TX ) );
        rfalSetAnalogConfig( (rfalAnalogConfigId)(RFAL_ANALOG_CONFIG_POLL | RFAL_ANALOG_CONFIG_TECH_NFCV | rfalConvBR2ACBR(gRFAL.rxBR) | RFAL_ANALOG_CONFIG_RX ) );
        break;

#endif /* RFAL_FEATURE_NFCV */

    /*******************************************************************************/
    case RFAL_MODE_POLL_ACTIVE_P2P:

        /* Set Analog configurations for this bit rate */
        rfalSetAnalogConfig((RFAL_ANALOG_CONFIG_TECH_CHIP | RFAL_ANALOG_CONFIG_CHIP_POLL_COMMON));
        rfalSetAnalogConfig( (rfalAnalogConfigId)(RFAL_ANALOG_CONFIG_POLL | RFAL_ANALOG_CONFIG_TECH_AP2P | rfalConvBR2ACBR(gRFAL.txBR) | RFAL_ANALOG_CONFIG_TX ) );
        rfalSetAnalogConfig( (rfalAnalogConfigId)(RFAL_ANALOG_CONFIG_POLL | RFAL_ANALOG_CONFIG_TECH_AP2P | rfalConvBR2ACBR(gRFAL.rxBR) | RFAL_ANALOG_CONFIG_RX ) );
        break;

    /*******************************************************************************/
    case RFAL_MODE_LISTEN_ACTIVE_P2P:

        /* Set Analog configurations for this bit rate */
        rfalSetAnalogConfig(
            (RFAL_ANALOG_CONFIG_TECH_CHIP | RFAL_ANALOG_CONFIG_CHIP_LISTEN_COMMON));
        rfalSetAnalogConfig( (rfalAnalogConfigId)(RFAL_ANALOG_CONFIG_LISTEN | RFAL_ANALOG_CONFIG_TECH_AP2P | rfalConvBR2ACBR(gRFAL.txBR) | RFAL_ANALOG_CONFIG_TX ) );
        rfalSetAnalogConfig( (rfalAnalogConfigId)(RFAL_ANALOG_CONFIG_LISTEN | RFAL_ANALOG_CONFIG_TECH_AP2P | rfalConvBR2ACBR(gRFAL.rxBR) | RFAL_ANALOG_CONFIG_RX ) );
        break;

    /*******************************************************************************/
    case RFAL_MODE_LISTEN_NFCA:

        /* Set Analog configurations for this bit rate */
        rfalSetAnalogConfig(
            (RFAL_ANALOG_CONFIG_TECH_CHIP | RFAL_ANALOG_CONFIG_CHIP_LISTEN_COMMON));
        rfalSetAnalogConfig( (rfalAnalogConfigId)(RFAL_ANALOG_CONFIG_LISTEN | RFAL_ANALOG_CONFIG_TECH_NFCA | rfalConvBR2ACBR(gRFAL.txBR) | RFAL_ANALOG_CONFIG_TX ) );
        rfalSetAnalogConfig( (rfalAnalogConfigId)(RFAL_ANALOG_CONFIG_LISTEN | RFAL_ANALOG_CONFIG_TECH_NFCA | rfalConvBR2ACBR(gRFAL.rxBR) | RFAL_ANALOG_CONFIG_RX ) );
        break;

    /*******************************************************************************/
    case RFAL_MODE_LISTEN_NFCF:

        /* Set Analog configurations for this bit rate */
        rfalSetAnalogConfig(
            (RFAL_ANALOG_CONFIG_TECH_CHIP | RFAL_ANALOG_CONFIG_CHIP_LISTEN_COMMON));
        rfalSetAnalogConfig( (rfalAnalogConfigId)(RFAL_ANALOG_CONFIG_LISTEN | RFAL_ANALOG_CONFIG_TECH_NFCF | rfalConvBR2ACBR(gRFAL.txBR) | RFAL_ANALOG_CONFIG_TX ) );
        rfalSetAnalogConfig( (rfalAnalogConfigId)(RFAL_ANALOG_CONFIG_LISTEN | RFAL_ANALOG_CONFIG_TECH_NFCF | rfalConvBR2ACBR(gRFAL.rxBR) | RFAL_ANALOG_CONFIG_RX ) );
        break;

    /*******************************************************************************/
    case RFAL_MODE_LISTEN_NFCB:
    case RFAL_MODE_NONE:
        return ERR_WRONG_STATE;

    /*******************************************************************************/
    default:
        return ERR_NOT_IMPLEMENTED;
    }

    return ERR_NONE;
}

/*******************************************************************************/
ReturnCode rfalGetBitRate(rfalBitRate* txBR, rfalBitRate* rxBR) {
    if((gRFAL.state == RFAL_STATE_IDLE) || (gRFAL.mode == RFAL_MODE_NONE)) {
        return ERR_WRONG_STATE;
    }

    if(txBR != NULL) {
        *txBR = gRFAL.txBR;
    }

    if(rxBR != NULL) {
        *rxBR = gRFAL.rxBR;
    }

    return ERR_NONE;
}

/*******************************************************************************/
void rfalSetErrorHandling(rfalEHandling eHandling) {
    switch(eHandling) {
    case RFAL_ERRORHANDLING_NFC:
    case RFAL_ERRORHANDLING_NONE:
        st25r3916ClrRegisterBits(ST25R3916_REG_EMD_SUP_CONF, ST25R3916_REG_EMD_SUP_CONF_emd_emv);
        break;

    case RFAL_ERRORHANDLING_EMVCO:
        /* MISRA 16.4: no empty default statement (in case RFAL_SW_EMD is defined) */
#ifndef RFAL_SW_EMD
        st25r3916ModifyRegister(
            ST25R3916_REG_EMD_SUP_CONF,
            (ST25R3916_REG_EMD_SUP_CONF_emd_emv | ST25R3916_REG_EMD_SUP_CONF_emd_thld_mask),
            (ST25R3916_REG_EMD_SUP_CONF_emd_emv_on | RFAL_EMVCO_RX_MAXLEN));
#endif /* RFAL_SW_EMD */
        break;
    default:
        /* MISRA 16.4: no empty default statement (a comment being enough) */
        break;
    }

    gRFAL.conf.eHandling = eHandling;
}

/*******************************************************************************/
rfalEHandling rfalGetErrorHandling(void) {
    return gRFAL.conf.eHandling;
}

/*******************************************************************************/
void rfalSetFDTPoll(uint32_t FDTPoll) {
    gRFAL.timings.FDTPoll = MIN(FDTPoll, RFAL_ST25R3916_GPT_MAX_1FC);
}

/*******************************************************************************/
uint32_t rfalGetFDTPoll(void) {
    return gRFAL.timings.FDTPoll;
}

/*******************************************************************************/
void rfalSetFDTListen(uint32_t FDTListen) {
    gRFAL.timings.FDTListen = MIN(FDTListen, RFAL_ST25R3916_MRT_MAX_1FC);
}

/*******************************************************************************/
uint32_t rfalGetFDTListen(void) {
    return gRFAL.timings.FDTListen;
}

/*******************************************************************************/
void rfalSetGT(uint32_t GT) {
    gRFAL.timings.GT = MIN(GT, RFAL_ST25R3916_GT_MAX_1FC);
}

/*******************************************************************************/
uint32_t rfalGetGT(void) {
    return gRFAL.timings.GT;
}

/*******************************************************************************/
bool rfalIsGTExpired(void) {
    if(gRFAL.tmr.GT != RFAL_TIMING_NONE) {
        if(!rfalTimerisExpired(gRFAL.tmr.GT)) {
            return false;
        }
    }
    return true;
}

/*******************************************************************************/
ReturnCode rfalFieldOnAndStartGT(void) {
    ReturnCode ret;

    /* Check if RFAL has been initialized (Oscillator should be running) and also
     * if a direct register access has been performed and left the Oscillator Off */
    if(!st25r3916IsOscOn() || (gRFAL.state < RFAL_STATE_INIT)) {
        return ERR_WRONG_STATE;
    }

    ret = ERR_NONE;

    /* Set Analog configurations for Field On event */
    rfalSetAnalogConfig((RFAL_ANALOG_CONFIG_TECH_CHIP | RFAL_ANALOG_CONFIG_CHIP_FIELD_ON));

    /*******************************************************************************/
    /* Perform collision avoidance and turn field On if not already On */
    if(!st25r3916IsTxEnabled() || !gRFAL.field) {
        /* Set TARFG: 0 (75us+0ms=75us), GT is fulfilled using a SW timer */
        st25r3916WriteRegister(ST25R3916_REG_FIELD_ON_GT, 0U);

        /* Use Thresholds set by AnalogConfig */
        ret = st25r3916PerformCollisionAvoidance(
            ST25R3916_CMD_INITIAL_RF_COLLISION,
            ST25R3916_THRESHOLD_DO_NOT_SET,
            ST25R3916_THRESHOLD_DO_NOT_SET,
            gRFAL.timings.nTRFW);

        /* n * TRFW timing shall vary  Activity 2.1  3.3.1.1 */
        gRFAL.timings.nTRFW = rfalGennTRFW(gRFAL.timings.nTRFW);

        gRFAL.field = st25r3916IsTxEnabled(); //(ret == ERR_NONE);

        /* Only turn on Receiver and Transmitter if field was successfully turned On */
        if(gRFAL.field) {
            st25r3916TxRxOn(); /* Enable Tx and Rx (Tx is already On)*/
        }
    }

    /*******************************************************************************/
    /* Start GT timer in case the GT value is set */
    if((gRFAL.timings.GT != RFAL_TIMING_NONE)) {
        /* Ensure that a SW timer doesn't have a lower value then the minimum  */
        rfalTimerStart(
            gRFAL.tmr.GT, rfalConv1fcToMs(MAX((gRFAL.timings.GT), RFAL_ST25R3916_GT_MIN_1FC)));
    }

    return ret;
}

/*******************************************************************************/
ReturnCode rfalFieldOff(void) {
    /* Check whether a TxRx is not yet finished */
    if(gRFAL.TxRx.state != RFAL_TXRX_STATE_IDLE) {
        rfalCleanupTransceive();
    }

    /* Disable Tx and Rx */
    st25r3916TxRxOff();

    /* Set Analog configurations for Field Off event */
    rfalSetAnalogConfig((RFAL_ANALOG_CONFIG_TECH_CHIP | RFAL_ANALOG_CONFIG_CHIP_FIELD_OFF));
    gRFAL.field = false;

    return ERR_NONE;
}

/*******************************************************************************/
ReturnCode rfalStartTransceive(const rfalTransceiveContext* ctx) {
    uint32_t FxTAdj; /* FWT or FDT adjustment calculation */

    /* Check for valid parameters */
    if(ctx == NULL) {
        return ERR_PARAM;
    }

    /* Ensure that RFAL is already Initialized and the mode has been set */
    if((gRFAL.state >= RFAL_STATE_MODE_SET) /*&& (gRFAL.TxRx.state == RFAL_TXRX_STATE_INIT )*/) {
        /*******************************************************************************/
        /* Check whether the field is already On, otherwise no TXE will be received  */
        if(!st25r3916IsTxEnabled() &&
           (!rfalIsModePassiveListen(gRFAL.mode) && (ctx->txBuf != NULL))) {
            return ERR_WRONG_STATE;
        }

        gRFAL.TxRx.ctx = *ctx;

        /*******************************************************************************/
        if(gRFAL.timings.FDTListen != RFAL_TIMING_NONE) {
            /* Calculate MRT adjustment accordingly to the current mode */
            FxTAdj = RFAL_FDT_LISTEN_MRT_ADJUSTMENT;
            if(gRFAL.mode == RFAL_MODE_POLL_NFCA) {
                FxTAdj += (uint32_t)RFAL_FDT_LISTEN_A_ADJUSTMENT;
            }
            if(gRFAL.mode == RFAL_MODE_POLL_NFCA_T1T) {
                FxTAdj += (uint32_t)RFAL_FDT_LISTEN_A_ADJUSTMENT;
            }
            if(gRFAL.mode == RFAL_MODE_POLL_NFCB) {
                FxTAdj += (uint32_t)RFAL_FDT_LISTEN_B_ADJUSTMENT;
            }
            if(gRFAL.mode == RFAL_MODE_POLL_NFCV) {
                FxTAdj += (uint32_t)RFAL_FDT_LISTEN_V_ADJUSTMENT;
            }

            /* Ensure that MRT is using 64/fc steps */
            st25r3916ClrRegisterBits(
                ST25R3916_REG_TIMER_EMV_CONTROL, ST25R3916_REG_TIMER_EMV_CONTROL_mrt_step);

            /* If Correlator is being used further adjustment is required for NFCB */
            if((st25r3916CheckReg(ST25R3916_REG_AUX, ST25R3916_REG_AUX_dis_corr, 0x00U)) &&
               (gRFAL.mode == RFAL_MODE_POLL_NFCB)) {
                FxTAdj += (uint32_t)
                    RFAL_FDT_LISTEN_B_ADJT_CORR; /* Reduce FDT(Listen)                   */
                st25r3916SetRegisterBits(
                    ST25R3916_REG_CORR_CONF1,
                    ST25R3916_REG_CORR_CONF1_corr_s3); /* Ensure BPSK start to 33 pilot pulses */
                st25r3916ChangeRegisterBits(
                    ST25R3916_REG_SUBC_START_TIME,
                    ST25R3916_REG_SUBC_START_TIME_sst_mask,
                    RFAL_FDT_LISTEN_B_ADJT_CORR_SST); /* Set sst                              */
            }

            /* Set Minimum FDT(Listen) in which PICC is not allowed to send a response */
            st25r3916WriteRegister(
                ST25R3916_REG_MASK_RX_TIMER,
                (uint8_t)rfalConv1fcTo64fc(
                    (FxTAdj > gRFAL.timings.FDTListen) ? RFAL_ST25R3916_MRT_MIN_1FC :
                                                         (gRFAL.timings.FDTListen - FxTAdj)));
        }

        /*******************************************************************************/
        /* FDT Poll will be loaded in rfalPrepareTransceive() once the previous was expired */

        /*******************************************************************************/
        if((gRFAL.TxRx.ctx.fwt != RFAL_FWT_NONE) && (gRFAL.TxRx.ctx.fwt != 0U)) {
            /* Ensure proper timing configuration */
            if(gRFAL.timings.FDTListen >= gRFAL.TxRx.ctx.fwt) {
                return ERR_PARAM;
            }

            FxTAdj = RFAL_FWT_ADJUSTMENT;
            if(gRFAL.mode == RFAL_MODE_POLL_NFCA) {
                FxTAdj += (uint32_t)RFAL_FWT_A_ADJUSTMENT;
            }
            if(gRFAL.mode == RFAL_MODE_POLL_NFCA_T1T) {
                FxTAdj += (uint32_t)RFAL_FWT_A_ADJUSTMENT;
            }
            if(gRFAL.mode == RFAL_MODE_POLL_NFCB) {
                FxTAdj += (uint32_t)RFAL_FWT_B_ADJUSTMENT;
            }
            if((gRFAL.mode == RFAL_MODE_POLL_NFCF) || (gRFAL.mode == RFAL_MODE_POLL_ACTIVE_P2P)) {
                FxTAdj +=
                    (uint32_t)((gRFAL.txBR == RFAL_BR_212) ? RFAL_FWT_F_212_ADJUSTMENT : RFAL_FWT_F_424_ADJUSTMENT);
            }

            /* Ensure that the given FWT doesn't exceed NRT maximum */
            gRFAL.TxRx.ctx.fwt = MIN((gRFAL.TxRx.ctx.fwt + FxTAdj), RFAL_ST25R3916_NRT_MAX_1FC);

            /* Set FWT in the NRT */
            st25r3916SetNoResponseTime(rfalConv1fcTo64fc(gRFAL.TxRx.ctx.fwt));
        } else {
            /* Disable NRT, no NRE will be triggered, therefore wait endlessly for Rx */
            st25r3916SetNoResponseTime(RFAL_ST25R3916_NRT_DISABLED);
        }

        gRFAL.state = RFAL_STATE_TXRX;
        gRFAL.TxRx.state = RFAL_TXRX_STATE_TX_IDLE;
        gRFAL.TxRx.status = ERR_BUSY;

#if RFAL_FEATURE_NFCV
        /*******************************************************************************/
        if((RFAL_MODE_POLL_NFCV == gRFAL.mode) ||
           (RFAL_MODE_POLL_PICOPASS ==
            gRFAL.mode)) { /* Exchange receive buffer with internal buffer */
            gRFAL.nfcvData.origCtx = gRFAL.TxRx.ctx;

            gRFAL.TxRx.ctx.rxBuf =
                ((gRFAL.nfcvData.origCtx.rxBuf != NULL) ? gRFAL.nfcvData.codingBuffer : NULL);
            gRFAL.TxRx.ctx.rxBufLen =
                (uint16_t)rfalConvBytesToBits(sizeof(gRFAL.nfcvData.codingBuffer));
            gRFAL.TxRx.ctx.flags =
                (uint32_t)RFAL_TXRX_FLAGS_CRC_TX_MANUAL | (uint32_t)RFAL_TXRX_FLAGS_CRC_RX_KEEP |
                (uint32_t)RFAL_TXRX_FLAGS_NFCIP1_OFF |
                (uint32_t)(gRFAL.nfcvData.origCtx.flags & (uint32_t)RFAL_TXRX_FLAGS_AGC_OFF) |
                (uint32_t)RFAL_TXRX_FLAGS_PAR_RX_KEEP | (uint32_t)RFAL_TXRX_FLAGS_PAR_TX_NONE;

            /* In NFCV a TxRx with a valid txBuf and txBufSize==0 indicates to send an EOF */
            /* Skip logic below that would go directly into receive                        */
            if(gRFAL.TxRx.ctx.txBuf != NULL) {
                return ERR_NONE;
            }
        }
#endif /* RFAL_FEATURE_NFCV */

        /*******************************************************************************/
        /* Check if the Transceive start performing Tx or goes directly to Rx          */
        if((gRFAL.TxRx.ctx.txBuf == NULL) || (gRFAL.TxRx.ctx.txBufLen == 0U)) {
            /* Clear FIFO, Clear and Enable the Interrupts */
            rfalPrepareTransceive();

            /* In AP2P check the field status */
            if(rfalIsModeActiveComm(gRFAL.mode)) {
                /* Disable our field upon a Rx reEnable, and start PPON2 manually */
                st25r3916TxOff();
                st25r3916ExecuteCommand(ST25R3916_CMD_START_PPON2_TIMER);
            }

            /* No Tx done, enable the Receiver */
            st25r3916ExecuteCommand(ST25R3916_CMD_UNMASK_RECEIVE_DATA);

            /* Start NRT manually, if FWT = 0 (wait endlessly for Rx) chip will ignore anyhow */
            st25r3916ExecuteCommand(ST25R3916_CMD_START_NO_RESPONSE_TIMER);

            gRFAL.TxRx.state = RFAL_TXRX_STATE_RX_IDLE;
        }

        return ERR_NONE;
    }

    return ERR_WRONG_STATE;
}

/*******************************************************************************/
bool rfalIsTransceiveInTx(void) {
    return (
        (gRFAL.TxRx.state >= RFAL_TXRX_STATE_TX_IDLE) &&
        (gRFAL.TxRx.state < RFAL_TXRX_STATE_RX_IDLE));
}

/*******************************************************************************/
bool rfalIsTransceiveInRx(void) {
    return (gRFAL.TxRx.state >= RFAL_TXRX_STATE_RX_IDLE);
}

/*******************************************************************************/
ReturnCode rfalTransceiveBlockingTx(
    uint8_t* txBuf,
    uint16_t txBufLen,
    uint8_t* rxBuf,
    uint16_t rxBufLen,
    uint16_t* actLen,
    uint32_t flags,
    uint32_t fwt) {
    ReturnCode ret;
    rfalTransceiveContext ctx;

    rfalCreateByteFlagsTxRxContext(ctx, txBuf, txBufLen, rxBuf, rxBufLen, actLen, flags, fwt);
    EXIT_ON_ERR(ret, rfalStartTransceive(&ctx));

    return rfalTransceiveRunBlockingTx();
}

ReturnCode rfalTransceiveBitsBlockingTx(
    uint8_t* txBuf,
    uint16_t txBufLen,
    uint8_t* rxBuf,
    uint16_t rxBufLen,
    uint16_t* actLen,
    uint32_t flags,
    uint32_t fwt) {
    ReturnCode ret;
    rfalTransceiveContext ctx = {
        .rxBuf = rxBuf,
        .rxBufLen = rxBufLen,
        .rxRcvdLen = actLen,
        .txBuf = txBuf,
        .txBufLen = txBufLen,
        .flags = flags,
        .fwt = fwt,
    };

    EXIT_ON_ERR(ret, rfalStartTransceive(&ctx));

    return rfalTransceiveRunBlockingTx();
}

/*******************************************************************************/
static ReturnCode rfalTransceiveRunBlockingTx(void) {
    ReturnCode ret;

    do {
        rfalWorker();
        ret = rfalGetTransceiveStatus();
    } while(rfalIsTransceiveInTx() && (ret == ERR_BUSY));

    if(rfalIsTransceiveInRx()) {
        return ERR_NONE;
    }

    return ret;
}

/*******************************************************************************/
ReturnCode rfalTransceiveBlockingRx(void) {
    ReturnCode ret;

    do {
        rfalWorker();
        ret = rfalGetTransceiveStatus();
    } while(rfalIsTransceiveInRx() && (ret == ERR_BUSY));

    return ret;
}

/*******************************************************************************/
ReturnCode rfalTransceiveBlockingTxRx(
    uint8_t* txBuf,
    uint16_t txBufLen,
    uint8_t* rxBuf,
    uint16_t rxBufLen,
    uint16_t* actLen,
    uint32_t flags,
    uint32_t fwt) {
    ReturnCode ret;

    EXIT_ON_ERR(
        ret, rfalTransceiveBlockingTx(txBuf, txBufLen, rxBuf, rxBufLen, actLen, flags, fwt));
    ret = rfalTransceiveBlockingRx();

    /* Convert received bits to bytes */
    if(actLen != NULL) {
        *actLen = rfalConvBitsToBytes(*actLen);
    }

    return ret;
}

ReturnCode rfalTransceiveBitsBlockingTxRx(
    uint8_t* txBuf,
    uint16_t txBufLen,
    uint8_t* rxBuf,
    uint16_t rxBufLen,
    uint16_t* actLen,
    uint32_t flags,
    uint32_t fwt) {
    ReturnCode ret;

    EXIT_ON_ERR(
        ret, rfalTransceiveBitsBlockingTx(txBuf, txBufLen, rxBuf, rxBufLen, actLen, flags, fwt));
    ret = rfalTransceiveBlockingRx();

    return ret;
}

/*******************************************************************************/
static ReturnCode rfalRunTransceiveWorker(void) {
    if(gRFAL.state == RFAL_STATE_TXRX) {
        /*******************************************************************************/
        /* Check Transceive Sanity Timer has expired */
        if(gRFAL.tmr.txRx != RFAL_TIMING_NONE) {
            if(rfalTimerisExpired(gRFAL.tmr.txRx)) {
                /* If sanity timer has expired abort ongoing transceive and signal error */
                gRFAL.TxRx.status = ERR_IO;
                gRFAL.TxRx.state = RFAL_TXRX_STATE_RX_FAIL;
            }
        }

        /*******************************************************************************/
        /* Run Tx or Rx state machines */
        if(rfalIsTransceiveInTx()) {
            rfalTransceiveTx();
            return rfalGetTransceiveStatus();
        }
        if(rfalIsTransceiveInRx()) {
            rfalTransceiveRx();
            return rfalGetTransceiveStatus();
        }
    }
    return ERR_WRONG_STATE;
}

/*******************************************************************************/
rfalTransceiveState rfalGetTransceiveState(void) {
    return gRFAL.TxRx.state;
}

/*******************************************************************************/
ReturnCode rfalGetTransceiveStatus(void) {
    return ((gRFAL.TxRx.state == RFAL_TXRX_STATE_IDLE) ? gRFAL.TxRx.status : ERR_BUSY);
}

/*******************************************************************************/
ReturnCode rfalGetTransceiveRSSI(uint16_t* rssi) {
    uint16_t amRSSI;
    uint16_t pmRSSI;
    bool isSumMode;

    if(rssi == NULL) {
        return ERR_PARAM;
    }

    st25r3916GetRSSI(&amRSSI, &pmRSSI);

    /* Check if Correlator Summation mode is being used */
    isSumMode =
        (st25r3916CheckReg(
             ST25R3916_REG_CORR_CONF1,
             ST25R3916_REG_CORR_CONF1_corr_s4,
             ST25R3916_REG_CORR_CONF1_corr_s4) ?
             st25r3916CheckReg(ST25R3916_REG_AUX, ST25R3916_REG_AUX_dis_corr, 0x00) :
             false);
    if(isSumMode) {
        /*******************************************************************************/
        /* Using SQRT from math.h and float. If due to compiler, resources or performance 
         * issue this cannot be used, other approaches can be foreseen with less accuracy:
         *    Use a simpler sqrt algorithm 
         *    *rssi = MAX( amRSSI, pmRSSI );
         *    *rssi = ( (amRSSI + pmRSSI) / 2);
         */
        *rssi = (uint16_t)sqrt(
            ((double)amRSSI * (double)amRSSI) +
            ((double)pmRSSI *
             (double)
                 pmRSSI)); /*  PRQA S 5209 # MISRA 4.9 - External function (sqrt()) requires double */
    } else {
        /* Check which channel was used */
        *rssi =
            (st25r3916CheckReg(
                 ST25R3916_REG_AUX_DISPLAY,
                 ST25R3916_REG_AUX_DISPLAY_a_cha,
                 ST25R3916_REG_AUX_DISPLAY_a_cha) ?
                 pmRSSI :
                 amRSSI);
    }
    return ERR_NONE;
}

/*******************************************************************************/
void rfalWorker(void) {
    platformProtectWorker(); /* Protect RFAL Worker/Task/Process */

    switch(gRFAL.state) {
    case RFAL_STATE_TXRX:
        rfalRunTransceiveWorker();
        break;

#if RFAL_FEATURE_LISTEN_MODE
    case RFAL_STATE_LM:
        rfalRunListenModeWorker();
        break;
#endif /* RFAL_FEATURE_LISTEN_MODE */

#if RFAL_FEATURE_WAKEUP_MODE
    case RFAL_STATE_WUM:
        rfalRunWakeUpModeWorker();
        break;
#endif /* RFAL_FEATURE_WAKEUP_MODE */

    /* Nothing to be done */
    default:
        /* MISRA 16.4: no empty default statement (a comment being enough) */
        break;
    }

    platformUnprotectWorker(); /* Unprotect RFAL Worker/Task/Process */
}

/*******************************************************************************/
static void rfalErrorHandling(void) {
    uint16_t fifoBytesToRead;

    fifoBytesToRead = rfalFIFOStatusGetNumBytes();

#ifdef RFAL_SW_EMD
    /*******************************************************************************/
    /* EMVCo                                                                       */
    /*******************************************************************************/
    if(gRFAL.conf.eHandling == RFAL_ERRORHANDLING_EMVCO) {
        bool rxHasIncParError;

        /*******************************************************************************/
        /* EMD Handling - NFC Forum Digital 1.1  4.1.1.1 ; EMVCo v2.5  4.9.2           */
        /* ReEnable the receiver on frames with a length < 4 bytes, upon:              */
        /*   - Collision or Framing error detected                                     */
        /*   - Residual bits are detected (hard framing error)                         */
        /*   - Parity error                                                            */
        /*   - CRC error                                                               */
        /*******************************************************************************/

        /* Check if reception has incomplete bytes or parity error */
        rxHasIncParError =
            (rfalFIFOStatusIsIncompleteByte() ? true :
                                                rfalFIFOStatusIsMissingPar()); /* MISRA 13.5 */

        /* In case there are residual bits decrement FIFO bytes */
        /* Ensure FIFO contains some byte as the FIFO might be empty upon Framing errors */
        if((fifoBytesToRead > 0U) && rxHasIncParError) {
            fifoBytesToRead--;
        }

        if(((gRFAL.fifo.bytesTotal + fifoBytesToRead) < RFAL_EMVCO_RX_MAXLEN) &&
           ((gRFAL.TxRx.status == ERR_RF_COLLISION) || (gRFAL.TxRx.status == ERR_FRAMING) ||
            (gRFAL.TxRx.status == ERR_PAR) || (gRFAL.TxRx.status == ERR_CRC) ||
            rxHasIncParError)) {
            /* Ignore this reception, ReEnable receiver which also clears the FIFO */
            st25r3916ExecuteCommand(ST25R3916_CMD_UNMASK_RECEIVE_DATA);

            /* Ensure that the NRT has not expired meanwhile */
            if(st25r3916CheckReg(
                   ST25R3916_REG_NFCIP1_BIT_RATE, ST25R3916_REG_NFCIP1_BIT_RATE_nrt_on, 0x00)) {
                if(st25r3916CheckReg(
                       ST25R3916_REG_AUX_DISPLAY, ST25R3916_REG_AUX_DISPLAY_rx_act, 0x00)) {
                    /* Abort reception */
                    st25r3916ExecuteCommand(ST25R3916_CMD_MASK_RECEIVE_DATA);
                    gRFAL.TxRx.state = RFAL_TXRX_STATE_RX_FAIL;
                    return;
                }
            }

            rfalFIFOStatusClear();
            gRFAL.fifo.bytesTotal = 0;
            gRFAL.TxRx.status = ERR_BUSY;
            gRFAL.TxRx.state = RFAL_TXRX_STATE_RX_WAIT_RXS;
        }
        return;
    }
#endif

    /*******************************************************************************/
    /* ISO14443A Mode                                                              */
    /*******************************************************************************/
    if(gRFAL.mode == RFAL_MODE_POLL_NFCA) {
        /*******************************************************************************/
        /* If we received a frame with a incomplete byte we`ll raise a specific error  *
         * ( support for T2T 4 bit ACK / NAK, MIFARE and Kovio )                       */
        /*******************************************************************************/
        if((gRFAL.TxRx.status == ERR_PAR) || (gRFAL.TxRx.status == ERR_CRC)) {
            if(rfalFIFOStatusIsIncompleteByte()) {
                st25r3916ReadFifo((uint8_t*)(gRFAL.TxRx.ctx.rxBuf), fifoBytesToRead);
                if((gRFAL.TxRx.ctx.rxRcvdLen) != NULL) {
                    *gRFAL.TxRx.ctx.rxRcvdLen = rfalFIFOGetNumIncompleteBits();
                }

                gRFAL.TxRx.status = ERR_INCOMPLETE_BYTE;
                gRFAL.TxRx.state = RFAL_TXRX_STATE_RX_FAIL;
            }
        }
    }
}

/*******************************************************************************/
static void rfalCleanupTransceive(void) {
    /*******************************************************************************/
    /* Transceive flags                                                            */
    /*******************************************************************************/

    /* Restore default settings on NFCIP1 mode, Receiving parity + CRC bits and manual Tx Parity*/
    st25r3916ClrRegisterBits(
        ST25R3916_REG_ISO14443A_NFC,
        (ST25R3916_REG_ISO14443A_NFC_no_tx_par | ST25R3916_REG_ISO14443A_NFC_no_rx_par |
         ST25R3916_REG_ISO14443A_NFC_nfc_f0));

    /* Restore AGC enabled */
    st25r3916SetRegisterBits(ST25R3916_REG_RX_CONF2, ST25R3916_REG_RX_CONF2_agc_en);

    /*******************************************************************************/

    /*******************************************************************************/
    /* Transceive timers                                                           */
    /*******************************************************************************/
    rfalTimerDestroy(gRFAL.tmr.txRx);
    rfalTimerDestroy(gRFAL.tmr.RXE);
    gRFAL.tmr.txRx = RFAL_TIMING_NONE;
    gRFAL.tmr.RXE = RFAL_TIMING_NONE;
    /*******************************************************************************/

    /*******************************************************************************/
    /* Execute Post Transceive Callback                                            */
    /*******************************************************************************/
    if(gRFAL.callbacks.postTxRx != NULL) {
        gRFAL.callbacks.postTxRx(gRFAL.callbacks.ctx);
    }
    /*******************************************************************************/
}

/*******************************************************************************/
static void rfalPrepareTransceive(void) {
    uint32_t maskInterrupts;
    uint8_t reg;

    /* If we are in RW or AP2P mode */
    if(!rfalIsModePassiveListen(gRFAL.mode)) {
        /* Reset receive logic with STOP command */
        st25r3916ExecuteCommand(ST25R3916_CMD_STOP);

        /* Reset Rx Gain */
        st25r3916ExecuteCommand(ST25R3916_CMD_RESET_RXGAIN);
    } else {
        /* In Passive Listen Mode do not use STOP as it stops FDT timer */
        st25r3916ExecuteCommand(ST25R3916_CMD_CLEAR_FIFO);
    }

    /*******************************************************************************/
    /* FDT Poll                                                                    */
    /*******************************************************************************/
    if(rfalIsModePassiveComm(gRFAL.mode)) /* Passive Comms */
    {
        /* In Passive communications General Purpose Timer is used to measure FDT Poll */
        if(gRFAL.timings.FDTPoll != RFAL_TIMING_NONE) {
            /* Configure GPT to start at RX end */
            st25r3916SetStartGPTimer(
                (uint16_t)rfalConv1fcTo8fc(MIN(
                    gRFAL.timings.FDTPoll, (gRFAL.timings.FDTPoll - RFAL_FDT_POLL_ADJUSTMENT))),
                ST25R3916_REG_TIMER_EMV_CONTROL_gptc_erx);
        }
    }

    /*******************************************************************************/
    /* Execute Pre Transceive Callback                                             */
    /*******************************************************************************/
    if(gRFAL.callbacks.preTxRx != NULL) {
        gRFAL.callbacks.preTxRx(gRFAL.callbacks.ctx);
    }
    /*******************************************************************************/

    maskInterrupts =
        (ST25R3916_IRQ_MASK_FWL | ST25R3916_IRQ_MASK_TXE | ST25R3916_IRQ_MASK_RXS |
         ST25R3916_IRQ_MASK_RXE | ST25R3916_IRQ_MASK_PAR | ST25R3916_IRQ_MASK_CRC |
         ST25R3916_IRQ_MASK_ERR1 | ST25R3916_IRQ_MASK_ERR2 | ST25R3916_IRQ_MASK_NRE);

    /*******************************************************************************/
    /* Transceive flags                                                            */
    /*******************************************************************************/

    reg =
        (ST25R3916_REG_ISO14443A_NFC_no_tx_par_off | ST25R3916_REG_ISO14443A_NFC_no_rx_par_off |
         ST25R3916_REG_ISO14443A_NFC_nfc_f0_off);

    /* Check if NFCIP1 mode is to be enabled */
    if((gRFAL.TxRx.ctx.flags & (uint8_t)RFAL_TXRX_FLAGS_NFCIP1_ON) != 0U) {
        reg |= ST25R3916_REG_ISO14443A_NFC_nfc_f0;
    }

    /* Check if Parity check is to be skipped and to keep the parity + CRC bits in FIFO */
    if((gRFAL.TxRx.ctx.flags & (uint8_t)RFAL_TXRX_FLAGS_PAR_RX_KEEP) != 0U) {
        reg |= ST25R3916_REG_ISO14443A_NFC_no_rx_par;
    }

    /* Check if automatic Parity bits is to be disabled */
    if((gRFAL.TxRx.ctx.flags & (uint8_t)RFAL_TXRX_FLAGS_PAR_TX_NONE) != 0U) {
        reg |= ST25R3916_REG_ISO14443A_NFC_no_tx_par;
    }

    /* Apply current TxRx flags on ISO14443A and NFC 106kb/s Settings Register */
    st25r3916ChangeRegisterBits(
        ST25R3916_REG_ISO14443A_NFC,
        (ST25R3916_REG_ISO14443A_NFC_no_tx_par | ST25R3916_REG_ISO14443A_NFC_no_rx_par |
         ST25R3916_REG_ISO14443A_NFC_nfc_f0),
        reg);

    /* Check if AGC is to be disabled */
    if((gRFAL.TxRx.ctx.flags & (uint8_t)RFAL_TXRX_FLAGS_AGC_OFF) != 0U) {
        st25r3916ClrRegisterBits(ST25R3916_REG_RX_CONF2, ST25R3916_REG_RX_CONF2_agc_en);
    } else {
        st25r3916SetRegisterBits(ST25R3916_REG_RX_CONF2, ST25R3916_REG_RX_CONF2_agc_en);
    }
    /*******************************************************************************/

    /*******************************************************************************/
    /* EMVCo NRT mode                                                              */
    /*******************************************************************************/
    if(gRFAL.conf.eHandling == RFAL_ERRORHANDLING_EMVCO) {
        st25r3916SetRegisterBits(
            ST25R3916_REG_TIMER_EMV_CONTROL, ST25R3916_REG_TIMER_EMV_CONTROL_nrt_emv);
        maskInterrupts |= ST25R3916_IRQ_MASK_RX_REST;
    } else {
        st25r3916ClrRegisterBits(
            ST25R3916_REG_TIMER_EMV_CONTROL, ST25R3916_REG_TIMER_EMV_CONTROL_nrt_emv);
    }
    /*******************************************************************************/

    /* In Passive Listen mode additionally enable External Field interrupts  */
    if(rfalIsModePassiveListen(gRFAL.mode)) {
        maskInterrupts |=
            (ST25R3916_IRQ_MASK_EOF |
             ST25R3916_IRQ_MASK_WU_F); /* Enable external Field interrupts to detect Link Loss and SENF_REQ auto responses */
    }

    /* In Active comms enable also External Field interrupts and set RF Collision Avoindance */
    if(rfalIsModeActiveComm(gRFAL.mode)) {
        maskInterrupts |=
            (ST25R3916_IRQ_MASK_EOF | ST25R3916_IRQ_MASK_EON | ST25R3916_IRQ_MASK_PPON2 |
             ST25R3916_IRQ_MASK_CAT | ST25R3916_IRQ_MASK_CAC);

        /* Set n=0 for subsequent RF Collision Avoidance */
        st25r3916ChangeRegisterBits(ST25R3916_REG_AUX, ST25R3916_REG_AUX_nfc_n_mask, 0);
    }

    /*******************************************************************************/
    /* Start transceive Sanity Timer if a FWT is used */
    if((gRFAL.TxRx.ctx.fwt != RFAL_FWT_NONE) && (gRFAL.TxRx.ctx.fwt != 0U)) {
        rfalTimerStart(gRFAL.tmr.txRx, rfalCalcSanityTmr(gRFAL.TxRx.ctx.fwt));
    }
    /*******************************************************************************/

    /*******************************************************************************/
    /* Clear and enable these interrupts */
    st25r3916GetInterrupt(maskInterrupts);
    st25r3916EnableInterrupts(maskInterrupts);

    /* Clear FIFO status local copy */
    rfalFIFOStatusClear();
}

/*******************************************************************************/
static void rfalTransceiveTx(void) {
    volatile uint32_t irqs;
    uint16_t tmp;
    ReturnCode ret;

    /* Suppress warning in case NFC-V feature is disabled */
    ret = ERR_NONE;
    NO_WARNING(ret);

    irqs = ST25R3916_IRQ_MASK_NONE;

    if(gRFAL.TxRx.state != gRFAL.TxRx.lastState) {
        /* rfalLogD( "RFAL: lastSt: %d curSt: %d \r\n", gRFAL.TxRx.lastState, gRFAL.TxRx.state ); */
        gRFAL.TxRx.lastState = gRFAL.TxRx.state;
    }

    switch(gRFAL.TxRx.state) {
    /*******************************************************************************/
    case RFAL_TXRX_STATE_TX_IDLE:

        /* Nothing to do */

        gRFAL.TxRx.state = RFAL_TXRX_STATE_TX_WAIT_GT;
        /* fall through */

    /*******************************************************************************/
    case RFAL_TXRX_STATE_TX_WAIT_GT: /*  PRQA S 2003 # MISRA 16.3 - Intentional fall through */

        if(!rfalIsGTExpired()) {
            break;
        }

        rfalTimerDestroy(gRFAL.tmr.GT);
        gRFAL.tmr.GT = RFAL_TIMING_NONE;

        gRFAL.TxRx.state = RFAL_TXRX_STATE_TX_WAIT_FDT;
        /* fall through */

    /*******************************************************************************/
    case RFAL_TXRX_STATE_TX_WAIT_FDT: /*  PRQA S 2003 # MISRA 16.3 - Intentional fall through */

        /* Only in Passive communications GPT is used to measure FDT Poll */
        if(rfalIsModePassiveComm(gRFAL.mode)) {
            if(st25r3916IsGPTRunning()) {
                break;
            }
        }

        gRFAL.TxRx.state = RFAL_TXRX_STATE_TX_TRANSMIT;
        /* fall through */

    /*******************************************************************************/
    case RFAL_TXRX_STATE_TX_TRANSMIT: /*  PRQA S 2003 # MISRA 16.3 - Intentional fall through */

        /* Clear FIFO, Clear and Enable the Interrupts */
        rfalPrepareTransceive();

        /* ST25R3916 has a fixed FIFO water level */
        gRFAL.fifo.expWL = RFAL_FIFO_OUT_WL;

#if RFAL_FEATURE_NFCV
        /*******************************************************************************/
        /* In NFC-V streaming mode, the FIFO needs to be loaded with the coded bits    */
        if((RFAL_MODE_POLL_NFCV == gRFAL.mode) || (RFAL_MODE_POLL_PICOPASS == gRFAL.mode)) {
#if 0
                /* Debugging code: output the payload bits by writing into the FIFO and subsequent clearing */
                st25r3916WriteFifo(gRFAL.TxRx.ctx.txBuf, rfalConvBitsToBytes(gRFAL.TxRx.ctx.txBufLen));
                st25r3916ExecuteCommand( ST25R3916_CMD_CLEAR_FIFO );
#endif
            /* Calculate the bytes needed to be Written into FIFO (a incomplete byte will be added as 1byte) */
            gRFAL.nfcvData.nfcvOffset = 0;
            ret = iso15693VCDCode(
                gRFAL.TxRx.ctx.txBuf,
                rfalConvBitsToBytes(gRFAL.TxRx.ctx.txBufLen),
                (((gRFAL.nfcvData.origCtx.flags & (uint32_t)RFAL_TXRX_FLAGS_CRC_TX_MANUAL) != 0U) ?
                     false :
                     true),
                (((gRFAL.nfcvData.origCtx.flags & (uint32_t)RFAL_TXRX_FLAGS_NFCV_FLAG_MANUAL) !=
                  0U) ?
                     false :
                     true),
                (RFAL_MODE_POLL_PICOPASS == gRFAL.mode),
                &gRFAL.fifo.bytesTotal,
                &gRFAL.nfcvData.nfcvOffset,
                gRFAL.nfcvData.codingBuffer,
                MIN((uint16_t)ST25R3916_FIFO_DEPTH, (uint16_t)sizeof(gRFAL.nfcvData.codingBuffer)),
                &gRFAL.fifo.bytesWritten);

            if((ret != ERR_NONE) && (ret != ERR_AGAIN)) {
                gRFAL.TxRx.status = ret;
                gRFAL.TxRx.state = RFAL_TXRX_STATE_TX_FAIL;
                break;
            }
            /* Set the number of full bytes and bits to be transmitted */
            st25r3916SetNumTxBits((uint16_t)rfalConvBytesToBits(gRFAL.fifo.bytesTotal));

            /* Load FIFO with coded bytes */
            st25r3916WriteFifo(gRFAL.nfcvData.codingBuffer, gRFAL.fifo.bytesWritten);

        }
        /*******************************************************************************/
        else
#endif /* RFAL_FEATURE_NFCV */
        {
            /* Calculate the bytes needed to be Written into FIFO (a incomplete byte will be added as 1byte) */
            gRFAL.fifo.bytesTotal = (uint16_t)rfalCalcNumBytes(gRFAL.TxRx.ctx.txBufLen);

            /* Set the number of full bytes and bits to be transmitted */
            st25r3916SetNumTxBits(gRFAL.TxRx.ctx.txBufLen);

            /* Load FIFO with total length or FIFO's maximum */
            gRFAL.fifo.bytesWritten = MIN(gRFAL.fifo.bytesTotal, ST25R3916_FIFO_DEPTH);
            st25r3916WriteFifo(gRFAL.TxRx.ctx.txBuf, gRFAL.fifo.bytesWritten);
        }

        /*Check if Observation Mode is enabled and set it on ST25R391x */
        rfalCheckEnableObsModeTx();

        /*******************************************************************************/
        /* If we're in Passive Listen mode ensure that the external field is still On  */
        if(rfalIsModePassiveListen(gRFAL.mode)) {
            if(!rfalIsExtFieldOn()) {
                gRFAL.TxRx.status = ERR_LINK_LOSS;
                gRFAL.TxRx.state = RFAL_TXRX_STATE_TX_FAIL;
                break;
            }
        }

        /*******************************************************************************/
        /* Trigger/Start transmission                                                  */
        if((gRFAL.TxRx.ctx.flags & (uint32_t)RFAL_TXRX_FLAGS_CRC_TX_MANUAL) != 0U) {
            st25r3916ExecuteCommand(ST25R3916_CMD_TRANSMIT_WITHOUT_CRC);
        } else {
            st25r3916ExecuteCommand(ST25R3916_CMD_TRANSMIT_WITH_CRC);
        }

        /* Check if a WL level is expected or TXE should come */
        gRFAL.TxRx.state =
            ((gRFAL.fifo.bytesWritten < gRFAL.fifo.bytesTotal) ? RFAL_TXRX_STATE_TX_WAIT_WL :
                                                                 RFAL_TXRX_STATE_TX_WAIT_TXE);
        break;

    /*******************************************************************************/
    case RFAL_TXRX_STATE_TX_WAIT_WL:

        irqs = st25r3916GetInterrupt((ST25R3916_IRQ_MASK_FWL | ST25R3916_IRQ_MASK_TXE));
        if(irqs == ST25R3916_IRQ_MASK_NONE) {
            break; /* No interrupt to process */
        }

        if(((irqs & ST25R3916_IRQ_MASK_FWL) != 0U) && ((irqs & ST25R3916_IRQ_MASK_TXE) == 0U)) {
            gRFAL.TxRx.state = RFAL_TXRX_STATE_TX_RELOAD_FIFO;
        } else {
            gRFAL.TxRx.status = ERR_IO;
            gRFAL.TxRx.state = RFAL_TXRX_STATE_TX_FAIL;
            break;
        }

        /* fall through */

    /*******************************************************************************/
    case RFAL_TXRX_STATE_TX_RELOAD_FIFO: /*  PRQA S 2003 # MISRA 16.3 - Intentional fall through */

#if RFAL_FEATURE_NFCV
        /*******************************************************************************/
        /* In NFC-V streaming mode, the FIFO needs to be loaded with the coded bits    */
        if((RFAL_MODE_POLL_NFCV == gRFAL.mode) || (RFAL_MODE_POLL_PICOPASS == gRFAL.mode)) {
            uint16_t maxLen;

            /* Load FIFO with the remaining length or maximum available (which fit on the coding buffer) */
            maxLen =
                (uint16_t)MIN((gRFAL.fifo.bytesTotal - gRFAL.fifo.bytesWritten), gRFAL.fifo.expWL);
            maxLen = (uint16_t)MIN(maxLen, sizeof(gRFAL.nfcvData.codingBuffer));
            tmp = 0;

            /* Calculate the bytes needed to be Written into FIFO (a incomplete byte will be added as 1byte) */
            ret = iso15693VCDCode(
                gRFAL.TxRx.ctx.txBuf,
                rfalConvBitsToBytes(gRFAL.TxRx.ctx.txBufLen),
                (((gRFAL.nfcvData.origCtx.flags & (uint32_t)RFAL_TXRX_FLAGS_CRC_TX_MANUAL) != 0U) ?
                     false :
                     true),
                (((gRFAL.nfcvData.origCtx.flags & (uint32_t)RFAL_TXRX_FLAGS_NFCV_FLAG_MANUAL) !=
                  0U) ?
                     false :
                     true),
                (RFAL_MODE_POLL_PICOPASS == gRFAL.mode),
                &gRFAL.fifo.bytesTotal,
                &gRFAL.nfcvData.nfcvOffset,
                gRFAL.nfcvData.codingBuffer,
                maxLen,
                &tmp);

            if((ret != ERR_NONE) && (ret != ERR_AGAIN)) {
                gRFAL.TxRx.status = ret;
                gRFAL.TxRx.state = RFAL_TXRX_STATE_TX_FAIL;
                break;
            }

            /* Load FIFO with coded bytes */
            st25r3916WriteFifo(gRFAL.nfcvData.codingBuffer, tmp);
        }
        /*******************************************************************************/
        else
#endif /* RFAL_FEATURE_NFCV */
        {
            /* Load FIFO with the remaining length or maximum available */
            tmp = MIN(
                (gRFAL.fifo.bytesTotal - gRFAL.fifo.bytesWritten),
                gRFAL.fifo.expWL); /* tmp holds the number of bytes written on this iteration */
            st25r3916WriteFifo(&gRFAL.TxRx.ctx.txBuf[gRFAL.fifo.bytesWritten], tmp);
        }

        /* Update total written bytes to FIFO */
        gRFAL.fifo.bytesWritten += tmp;

        /* Check if a WL level is expected or TXE should come */
        gRFAL.TxRx.state =
            ((gRFAL.fifo.bytesWritten < gRFAL.fifo.bytesTotal) ? RFAL_TXRX_STATE_TX_WAIT_WL :
                                                                 RFAL_TXRX_STATE_TX_WAIT_TXE);
        break;

    /*******************************************************************************/
    case RFAL_TXRX_STATE_TX_WAIT_TXE:

        irqs = st25r3916GetInterrupt((ST25R3916_IRQ_MASK_FWL | ST25R3916_IRQ_MASK_TXE));
        if(irqs == ST25R3916_IRQ_MASK_NONE) {
            break; /* No interrupt to process */
        }

        if((irqs & ST25R3916_IRQ_MASK_TXE) != 0U) {
            gRFAL.TxRx.state = RFAL_TXRX_STATE_TX_DONE;
        } else if((irqs & ST25R3916_IRQ_MASK_FWL) != 0U) {
            break; /* Ignore ST25R3916 FIFO WL if total TxLen is already on the FIFO */
        } else {
            gRFAL.TxRx.status = ERR_IO;
            gRFAL.TxRx.state = RFAL_TXRX_STATE_TX_FAIL;
            break;
        }

        /* fall through */

    /*******************************************************************************/
    case RFAL_TXRX_STATE_TX_DONE: /*  PRQA S 2003 # MISRA 16.3 - Intentional fall through */

        /* If no rxBuf is provided do not wait/expect Rx */
        if(gRFAL.TxRx.ctx.rxBuf == NULL) {
            /*Check if Observation Mode was enabled and disable it on ST25R391x */
            rfalCheckDisableObsMode();

            /* Clean up Transceive */
            rfalCleanupTransceive();

            gRFAL.TxRx.status = ERR_NONE;
            gRFAL.TxRx.state = RFAL_TXRX_STATE_IDLE;
            break;
        }

        rfalCheckEnableObsModeRx();

        /* Goto Rx */
        gRFAL.TxRx.state = RFAL_TXRX_STATE_RX_IDLE;
        break;

    /*******************************************************************************/
    case RFAL_TXRX_STATE_TX_FAIL:

        /* Error should be assigned by previous state */
        if(gRFAL.TxRx.status == ERR_BUSY) {
            gRFAL.TxRx.status = ERR_SYSTEM;
        }

        /*Check if Observation Mode was enabled and disable it on ST25R391x */
        rfalCheckDisableObsMode();

        /* Clean up Transceive */
        rfalCleanupTransceive();

        gRFAL.TxRx.state = RFAL_TXRX_STATE_IDLE;
        break;

    /*******************************************************************************/
    default:
        gRFAL.TxRx.status = ERR_SYSTEM;
        gRFAL.TxRx.state = RFAL_TXRX_STATE_TX_FAIL;
        break;
    }
}

/*******************************************************************************/
static void rfalTransceiveRx(void) {
    volatile uint32_t irqs;
    uint16_t tmp;
    uint16_t aux;

    irqs = ST25R3916_IRQ_MASK_NONE;

    if(gRFAL.TxRx.state != gRFAL.TxRx.lastState) {
        /* rfalLogD( "RFAL: lastSt: %d curSt: %d \r\n", gRFAL.TxRx.lastState, gRFAL.TxRx.state ); */
        gRFAL.TxRx.lastState = gRFAL.TxRx.state;
    }

    switch(gRFAL.TxRx.state) {
    /*******************************************************************************/
    case RFAL_TXRX_STATE_RX_IDLE:

        /* Clear rx counters */
        gRFAL.fifo.bytesWritten = 0; /* Total bytes written on RxBuffer         */
        gRFAL.fifo.bytesTotal = 0; /* Total bytes in FIFO will now be from Rx */
        if(gRFAL.TxRx.ctx.rxRcvdLen != NULL) {
            *gRFAL.TxRx.ctx.rxRcvdLen = 0;
        }

        gRFAL.TxRx.state =
            (rfalIsModeActiveComm(gRFAL.mode) ? RFAL_TXRX_STATE_RX_WAIT_EON :
                                                RFAL_TXRX_STATE_RX_WAIT_RXS);
        break;

    /*******************************************************************************/
    case RFAL_TXRX_STATE_RX_WAIT_RXS:

        /*******************************************************************************/
        irqs = st25r3916GetInterrupt(
            (ST25R3916_IRQ_MASK_RXS | ST25R3916_IRQ_MASK_NRE | ST25R3916_IRQ_MASK_EOF));
        if(irqs == ST25R3916_IRQ_MASK_NONE) {
            break; /* No interrupt to process */
        }

        /* Only raise Timeout if NRE is detected with no Rx Start (NRT EMV mode) */
        if(((irqs & ST25R3916_IRQ_MASK_NRE) != 0U) && ((irqs & ST25R3916_IRQ_MASK_RXS) == 0U)) {
            gRFAL.TxRx.status = ERR_TIMEOUT;
            gRFAL.TxRx.state = RFAL_TXRX_STATE_RX_FAIL;
            break;
        }

        /* Only raise Link Loss if EOF is detected with no Rx Start */
        if(((irqs & ST25R3916_IRQ_MASK_EOF) != 0U) && ((irqs & ST25R3916_IRQ_MASK_RXS) == 0U)) {
            /* In AP2P a Field On has already occurred - treat this as timeout | mute */
            gRFAL.TxRx.status = (rfalIsModeActiveComm(gRFAL.mode) ? ERR_TIMEOUT : ERR_LINK_LOSS);
            gRFAL.TxRx.state = RFAL_TXRX_STATE_RX_FAIL;
            break;
        }

        if((irqs & ST25R3916_IRQ_MASK_RXS) != 0U) {
            /*******************************************************************************/
            /* REMARK: Silicon workaround ST25R3916 Errata #TBD                            */
            /* Rarely on corrupted frames I_rxs gets signaled but I_rxe is not signaled    */
            /* Use a SW timer to handle an eventual missing RXE                            */
            rfalTimerStart(gRFAL.tmr.RXE, RFAL_NORXE_TOUT);
            /*******************************************************************************/

            gRFAL.TxRx.state = RFAL_TXRX_STATE_RX_WAIT_RXE;
        } else {
            gRFAL.TxRx.status = ERR_IO;
            gRFAL.TxRx.state = RFAL_TXRX_STATE_RX_FAIL;
            break;
        }

        /* remove NRE that might appear together (NRT EMV mode), and remove RXS, but keep EOF if present for next state */
        irqs &= ~(ST25R3916_IRQ_MASK_RXS | ST25R3916_IRQ_MASK_NRE);

        /* fall through */

    /*******************************************************************************/
    case RFAL_TXRX_STATE_RX_WAIT_RXE: /*  PRQA S 2003 # MISRA 16.3 - Intentional fall through */

        /*******************************************************************************/
        /* REMARK: Silicon workaround ST25R3916 Errata #TBD                            */
        /* ST25R396 may indicate RXS without RXE afterwards, this happens rarely on    */
        /* corrupted frames.                                                           */
        /* SW timer is used to timeout upon a missing RXE                              */
        if(rfalTimerisExpired(gRFAL.tmr.RXE)) {
            gRFAL.TxRx.status = ERR_FRAMING;
            gRFAL.TxRx.state = RFAL_TXRX_STATE_RX_FAIL;
        }
        /*******************************************************************************/

        irqs |= st25r3916GetInterrupt(
            (ST25R3916_IRQ_MASK_RXE | ST25R3916_IRQ_MASK_FWL | ST25R3916_IRQ_MASK_EOF |
             ST25R3916_IRQ_MASK_RX_REST | ST25R3916_IRQ_MASK_WU_F));
        if(irqs == ST25R3916_IRQ_MASK_NONE) {
            break; /* No interrupt to process */
        }

        if((irqs & ST25R3916_IRQ_MASK_RX_REST) != 0U) {
            /* RX_REST indicates that Receiver has been reset due to EMD, therefore a RXS + RXE should *
                 * follow if a good reception is followed within the valid initial timeout                   */

            /* Check whether NRT has expired already, if so signal a timeout */
            if(st25r3916GetInterrupt(ST25R3916_IRQ_MASK_NRE) != 0U) {
                gRFAL.TxRx.status = ERR_TIMEOUT;
                gRFAL.TxRx.state = RFAL_TXRX_STATE_RX_FAIL;
                break;
            }
            if(st25r3916CheckReg(
                   ST25R3916_REG_NFCIP1_BIT_RATE,
                   ST25R3916_REG_NFCIP1_BIT_RATE_nrt_on,
                   0)) /* MISRA 13.5 */
            {
                gRFAL.TxRx.status = ERR_TIMEOUT;
                gRFAL.TxRx.state = RFAL_TXRX_STATE_RX_FAIL;
                break;
            }

            /* Discard any previous RXS */
            st25r3916GetInterrupt(ST25R3916_IRQ_MASK_RXS);

            /* Check whether a following reception has already started */
            if(st25r3916CheckReg(
                   ST25R3916_REG_AUX_DISPLAY,
                   ST25R3916_REG_AUX_DISPLAY_rx_act,
                   ST25R3916_REG_AUX_DISPLAY_rx_act)) {
                gRFAL.TxRx.state = RFAL_TXRX_STATE_RX_WAIT_RXE;
                break;
            }

            gRFAL.TxRx.state = RFAL_TXRX_STATE_RX_WAIT_RXS;
            break;
        }

        if(((irqs & ST25R3916_IRQ_MASK_FWL) != 0U) && ((irqs & ST25R3916_IRQ_MASK_RXE) == 0U)) {
            gRFAL.TxRx.state = RFAL_TXRX_STATE_RX_READ_FIFO;
            break;
        }

        /* Automatic responses allowed during TxRx only for the SENSF_REQ */
        if((irqs & ST25R3916_IRQ_MASK_WU_F) != 0U) {
            gRFAL.TxRx.state = RFAL_TXRX_STATE_RX_WAIT_RXS;
            break;
        }

        /* After RXE retrieve and check for any error irqs */
        irqs |= st25r3916GetInterrupt(
            (ST25R3916_IRQ_MASK_CRC | ST25R3916_IRQ_MASK_PAR | ST25R3916_IRQ_MASK_ERR1 |
             ST25R3916_IRQ_MASK_ERR2 | ST25R3916_IRQ_MASK_COL));

        gRFAL.TxRx.state = RFAL_TXRX_STATE_RX_ERR_CHECK;
        /* fall through */

    /*******************************************************************************/
    case RFAL_TXRX_STATE_RX_ERR_CHECK: /*  PRQA S 2003 # MISRA 16.3 - Intentional fall through */

        if((irqs & ST25R3916_IRQ_MASK_ERR1) != 0U) {
            gRFAL.TxRx.status = ERR_FRAMING;
            gRFAL.TxRx.state = RFAL_TXRX_STATE_RX_READ_DATA;

            /* Check if there's a specific error handling for this */
            rfalErrorHandling();
            break;
        }
        /* Discard Soft Framing errors in AP2P and CE */
        else if(rfalIsModePassivePoll(gRFAL.mode) && ((irqs & ST25R3916_IRQ_MASK_ERR2) != 0U)) {
            gRFAL.TxRx.status = ERR_FRAMING;
            gRFAL.TxRx.state = RFAL_TXRX_STATE_RX_READ_DATA;

            /* Check if there's a specific error handling for this */
            rfalErrorHandling();
            break;
        } else if((irqs & ST25R3916_IRQ_MASK_PAR) != 0U) {
            gRFAL.TxRx.status = ERR_PAR;
            gRFAL.TxRx.state = RFAL_TXRX_STATE_RX_READ_DATA;

            /* Check if there's a specific error handling for this */
            rfalErrorHandling();
            break;
        } else if((irqs & ST25R3916_IRQ_MASK_CRC) != 0U) {
            gRFAL.TxRx.status = ERR_CRC;
            gRFAL.TxRx.state = RFAL_TXRX_STATE_RX_READ_DATA;

            /* Check if there's a specific error handling for this */
            rfalErrorHandling();
            break;
        } else if((irqs & ST25R3916_IRQ_MASK_COL) != 0U) {
            gRFAL.TxRx.status = ERR_RF_COLLISION;
            gRFAL.TxRx.state = RFAL_TXRX_STATE_RX_READ_DATA;

            /* Check if there's a specific error handling for this */
            rfalErrorHandling();
            break;
        } else if(rfalIsModePassiveListen(gRFAL.mode) && ((irqs & ST25R3916_IRQ_MASK_EOF) != 0U)) {
            gRFAL.TxRx.status = ERR_LINK_LOSS;
            gRFAL.TxRx.state = RFAL_TXRX_STATE_RX_FAIL;
            break;
        } else if((irqs & ST25R3916_IRQ_MASK_RXE) != 0U) {
            /* Reception ended without any error indication,                  *
                 * check FIFO status for malformed or incomplete frames           */

            /* Check if the reception ends with an incomplete byte (residual bits) */
            if(rfalFIFOStatusIsIncompleteByte()) {
                gRFAL.TxRx.status = ERR_INCOMPLETE_BYTE;
            }
            /* Check if the reception ends missing parity bit */
            else if(rfalFIFOStatusIsMissingPar()) {
                gRFAL.TxRx.status = ERR_FRAMING;
            } else {
                /* MISRA 15.7 - Empty else */
            }

            gRFAL.TxRx.state = RFAL_TXRX_STATE_RX_READ_DATA;
        } else {
            gRFAL.TxRx.status = ERR_IO;
            gRFAL.TxRx.state = RFAL_TXRX_STATE_RX_FAIL;
            break;
        }

        /* fall through */

    /*******************************************************************************/
    case RFAL_TXRX_STATE_RX_READ_DATA: /*  PRQA S 2003 # MISRA 16.3 - Intentional fall through */

        tmp = rfalFIFOStatusGetNumBytes();

        /*******************************************************************************/
        /* Check if CRC should not be placed in rxBuf                                  */
        if(((gRFAL.TxRx.ctx.flags & (uint32_t)RFAL_TXRX_FLAGS_CRC_RX_KEEP) == 0U)) {
            /* if received frame was bigger than CRC */
            if((uint16_t)(gRFAL.fifo.bytesTotal + tmp) > 0U) {
                /* By default CRC will not be placed into the rxBuffer */
                if((tmp > RFAL_CRC_LEN)) {
                    tmp -= RFAL_CRC_LEN;
                }
                /* If the CRC was already placed into rxBuffer (due to WL interrupt where CRC was already in FIFO Read)
                     * cannot remove it from rxBuf. Can only remove it from rxBufLen not indicate the presence of CRC    */
                else if(gRFAL.fifo.bytesTotal > RFAL_CRC_LEN) {
                    gRFAL.fifo.bytesTotal -= RFAL_CRC_LEN;
                } else {
                    /* MISRA 15.7 - Empty else */
                }
            }
        }

        gRFAL.fifo.bytesTotal += tmp; /* add to total bytes counter */

        /*******************************************************************************/
        /* Check if remaining bytes fit on the rxBuf available                         */
        if(gRFAL.fifo.bytesTotal > rfalConvBitsToBytes(gRFAL.TxRx.ctx.rxBufLen)) {
            tmp =
                (uint16_t)(rfalConvBitsToBytes(gRFAL.TxRx.ctx.rxBufLen) - gRFAL.fifo.bytesWritten);

            /* Transmission errors have precedence over buffer error */
            if(gRFAL.TxRx.status == ERR_BUSY) {
                gRFAL.TxRx.status = ERR_NOMEM;
            }
        }

        /*******************************************************************************/
        /* Retrieve remaining bytes from FIFO to rxBuf, and assign total length rcvd   */
        st25r3916ReadFifo(&gRFAL.TxRx.ctx.rxBuf[gRFAL.fifo.bytesWritten], tmp);
        if(gRFAL.TxRx.ctx.rxRcvdLen != NULL) {
            (*gRFAL.TxRx.ctx.rxRcvdLen) = (uint16_t)rfalConvBytesToBits(gRFAL.fifo.bytesTotal);
            if(rfalFIFOStatusIsIncompleteByte()) {
                (*gRFAL.TxRx.ctx.rxRcvdLen) -=
                    (RFAL_BITS_IN_BYTE - rfalFIFOGetNumIncompleteBits());
            }
        }

#if RFAL_FEATURE_NFCV
        /*******************************************************************************/
        /* Decode sub bit stream into payload bits for NFCV, if no error found so far  */
        if(((RFAL_MODE_POLL_NFCV == gRFAL.mode) || (RFAL_MODE_POLL_PICOPASS == gRFAL.mode)) &&
           (gRFAL.TxRx.status == ERR_BUSY)) {
            ReturnCode ret;
            uint16_t offset = 0; /* REMARK offset not currently used */

            ret = iso15693VICCDecode(
                gRFAL.TxRx.ctx.rxBuf,
                gRFAL.fifo.bytesTotal,
                gRFAL.nfcvData.origCtx.rxBuf,
                rfalConvBitsToBytes(gRFAL.nfcvData.origCtx.rxBufLen),
                &offset,
                gRFAL.nfcvData.origCtx.rxRcvdLen,
                gRFAL.nfcvData.ignoreBits,
                (RFAL_MODE_POLL_PICOPASS == gRFAL.mode));

            if(((ERR_NONE == ret) || (ERR_CRC == ret)) &&
               (((uint32_t)RFAL_TXRX_FLAGS_CRC_RX_KEEP & gRFAL.nfcvData.origCtx.flags) == 0U) &&
               ((*gRFAL.nfcvData.origCtx.rxRcvdLen % RFAL_BITS_IN_BYTE) == 0U) &&
               (*gRFAL.nfcvData.origCtx.rxRcvdLen >= rfalConvBytesToBits(RFAL_CRC_LEN))) {
                *gRFAL.nfcvData.origCtx.rxRcvdLen -=
                    (uint16_t)rfalConvBytesToBits(RFAL_CRC_LEN); /* Remove CRC */
            }
#if 0
                /* Debugging code: output the payload bits by writing into the FIFO and subsequent clearing */
                st25r3916WriteFifo(gRFAL.nfcvData.origCtx.rxBuf, rfalConvBitsToBytes( *gRFAL.nfcvData.origCtx.rxRcvdLen));
                st25r3916ExecuteCommand( ST25R3916_CMD_CLEAR_FIFO );
#endif

            /* Restore original ctx */
            gRFAL.TxRx.ctx = gRFAL.nfcvData.origCtx;
            gRFAL.TxRx.status = ((ret != ERR_NONE) ? ret : ERR_BUSY);
        }
#endif /* RFAL_FEATURE_NFCV */

        /*******************************************************************************/
        /* If an error as been marked/detected don't fall into to RX_DONE  */
        if(gRFAL.TxRx.status != ERR_BUSY) {
            gRFAL.TxRx.state = RFAL_TXRX_STATE_RX_FAIL;
            break;
        }

        if(rfalIsModeActiveComm(gRFAL.mode)) {
            gRFAL.TxRx.state = RFAL_TXRX_STATE_RX_WAIT_EOF;
            break;
        }

        gRFAL.TxRx.state = RFAL_TXRX_STATE_RX_DONE;
        /* fall through */

    /*******************************************************************************/
    case RFAL_TXRX_STATE_RX_DONE: /*  PRQA S 2003 # MISRA 16.3 - Intentional fall through */

        /*Check if Observation Mode was enabled and disable it on ST25R391x */
        rfalCheckDisableObsMode();

        /* Clean up Transceive */
        rfalCleanupTransceive();

        gRFAL.TxRx.status = ERR_NONE;
        gRFAL.TxRx.state = RFAL_TXRX_STATE_IDLE;
        break;

    /*******************************************************************************/
    case RFAL_TXRX_STATE_RX_READ_FIFO:

        /*******************************************************************************/
        /* REMARK: Silicon workaround ST25R3916 Errata #TBD                            */
        /* Rarely on corrupted frames I_rxs gets signaled but I_rxe is not signaled    */
        /* Use a SW timer to handle an eventual missing RXE                            */
        rfalTimerStart(gRFAL.tmr.RXE, RFAL_NORXE_TOUT);
        /*******************************************************************************/

        tmp = rfalFIFOStatusGetNumBytes();
        gRFAL.fifo.bytesTotal += tmp;

        /*******************************************************************************/
        /* Calculate the amount of bytes that still fits in rxBuf                      */
        aux =
            ((gRFAL.fifo.bytesTotal > rfalConvBitsToBytes(gRFAL.TxRx.ctx.rxBufLen)) ?
                 (rfalConvBitsToBytes(gRFAL.TxRx.ctx.rxBufLen) - gRFAL.fifo.bytesWritten) :
                 tmp);

        /*******************************************************************************/
        /* Retrieve incoming bytes from FIFO to rxBuf, and store already read amount   */
        st25r3916ReadFifo(&gRFAL.TxRx.ctx.rxBuf[gRFAL.fifo.bytesWritten], aux);
        gRFAL.fifo.bytesWritten += aux;

        /*******************************************************************************/
        /* If the bytes already read were not the full FIFO WL, dump the remaining     *
             * FIFO so that ST25R391x can continue with reception                          */
        if(aux < tmp) {
            st25r3916ReadFifo(NULL, (tmp - aux));
        }

        rfalFIFOStatusClear();
        gRFAL.TxRx.state = RFAL_TXRX_STATE_RX_WAIT_RXE;
        break;

    /*******************************************************************************/
    case RFAL_TXRX_STATE_RX_FAIL:

        /*Check if Observation Mode was enabled and disable it on ST25R391x */
        rfalCheckDisableObsMode();

        /* Clean up Transceive */
        rfalCleanupTransceive();

        /* Error should be assigned by previous state */
        if(gRFAL.TxRx.status == ERR_BUSY) {
            gRFAL.TxRx.status = ERR_SYSTEM;
        }

        /*rfalLogD( "RFAL: curSt: %d  Error: %d \r\n", gRFAL.TxRx.state, gRFAL.TxRx.status );*/
        gRFAL.TxRx.state = RFAL_TXRX_STATE_IDLE;
        break;

    /*******************************************************************************/
    case RFAL_TXRX_STATE_RX_WAIT_EON:

        irqs = st25r3916GetInterrupt(
            (ST25R3916_IRQ_MASK_EON | ST25R3916_IRQ_MASK_NRE | ST25R3916_IRQ_MASK_PPON2));
        if(irqs == ST25R3916_IRQ_MASK_NONE) {
            break; /* No interrupt to process */
        }

        if((irqs & ST25R3916_IRQ_MASK_EON) != 0U) {
            gRFAL.TxRx.state = RFAL_TXRX_STATE_RX_WAIT_RXS;
        }

        if((irqs & ST25R3916_IRQ_MASK_NRE) != 0U) {
            gRFAL.TxRx.status = ERR_TIMEOUT;
            gRFAL.TxRx.state = RFAL_TXRX_STATE_RX_FAIL;
        }
        if((irqs & ST25R3916_IRQ_MASK_PPON2) != 0U) {
            gRFAL.TxRx.status = ERR_LINK_LOSS;
            gRFAL.TxRx.state = RFAL_TXRX_STATE_RX_FAIL;
        }
        break;

    /*******************************************************************************/
    case RFAL_TXRX_STATE_RX_WAIT_EOF:

        irqs = st25r3916GetInterrupt((ST25R3916_IRQ_MASK_CAT | ST25R3916_IRQ_MASK_CAC));
        if(irqs == ST25R3916_IRQ_MASK_NONE) {
            break; /* No interrupt to process */
        }

        if((irqs & ST25R3916_IRQ_MASK_CAT) != 0U) {
            gRFAL.TxRx.state = RFAL_TXRX_STATE_RX_DONE;
        } else if((irqs & ST25R3916_IRQ_MASK_CAC) != 0U) {
            gRFAL.TxRx.status = ERR_RF_COLLISION;
            gRFAL.TxRx.state = RFAL_TXRX_STATE_RX_FAIL;
        } else {
            gRFAL.TxRx.status = ERR_IO;
            gRFAL.TxRx.state = RFAL_TXRX_STATE_RX_FAIL;
        }
        break;

    /*******************************************************************************/
    default:
        gRFAL.TxRx.status = ERR_SYSTEM;
        gRFAL.TxRx.state = RFAL_TXRX_STATE_RX_FAIL;
        break;
    }
}

/*******************************************************************************/
static void rfalFIFOStatusUpdate(void) {
    if(gRFAL.fifo.status[RFAL_FIFO_STATUS_REG2] == RFAL_FIFO_STATUS_INVALID) {
        st25r3916ReadMultipleRegisters(
            ST25R3916_REG_FIFO_STATUS1, gRFAL.fifo.status, ST25R3916_FIFO_STATUS_LEN);
    }
}

/*******************************************************************************/
static void rfalFIFOStatusClear(void) {
    gRFAL.fifo.status[RFAL_FIFO_STATUS_REG2] = RFAL_FIFO_STATUS_INVALID;
}

/*******************************************************************************/
static uint16_t rfalFIFOStatusGetNumBytes(void) {
    uint16_t result;

    rfalFIFOStatusUpdate();

    result =
        ((((uint16_t)gRFAL.fifo.status[RFAL_FIFO_STATUS_REG2] &
           ST25R3916_REG_FIFO_STATUS2_fifo_b_mask) >>
          ST25R3916_REG_FIFO_STATUS2_fifo_b_shift)
         << RFAL_BITS_IN_BYTE);
    result |= (((uint16_t)gRFAL.fifo.status[RFAL_FIFO_STATUS_REG1]) & 0x00FFU);
    return result;
}

/*******************************************************************************/
static bool rfalFIFOStatusIsIncompleteByte(void) {
    rfalFIFOStatusUpdate();
    return (
        (gRFAL.fifo.status[RFAL_FIFO_STATUS_REG2] & ST25R3916_REG_FIFO_STATUS2_fifo_lb_mask) !=
        0U);
}

/*******************************************************************************/
static bool rfalFIFOStatusIsMissingPar(void) {
    rfalFIFOStatusUpdate();
    return ((gRFAL.fifo.status[RFAL_FIFO_STATUS_REG2] & ST25R3916_REG_FIFO_STATUS2_np_lb) != 0U);
}

/*******************************************************************************/
static uint8_t rfalFIFOGetNumIncompleteBits(void) {
    rfalFIFOStatusUpdate();
    return (
        (gRFAL.fifo.status[RFAL_FIFO_STATUS_REG2] & ST25R3916_REG_FIFO_STATUS2_fifo_lb_mask) >>
        ST25R3916_REG_FIFO_STATUS2_fifo_lb_shift);
}

#if RFAL_FEATURE_NFCA

/*******************************************************************************/
ReturnCode rfalISO14443ATransceiveShortFrame(
    rfal14443AShortFrameCmd txCmd,
    uint8_t* rxBuf,
    uint8_t rxBufLen,
    uint16_t* rxRcvdLen,
    uint32_t fwt) {
    ReturnCode ret;
    uint8_t directCmd;

    /* Check if RFAL is properly initialized */
    if(!st25r3916IsTxEnabled() || (gRFAL.state < RFAL_STATE_MODE_SET) ||
       ((gRFAL.mode != RFAL_MODE_POLL_NFCA) && (gRFAL.mode != RFAL_MODE_POLL_NFCA_T1T))) {
        return ERR_WRONG_STATE;
    }

    /* Check for valid parameters */
    if((rxBuf == NULL) || (rxRcvdLen == NULL) || (fwt == RFAL_FWT_NONE)) {
        return ERR_PARAM;
    }

    /*******************************************************************************/
    /* Select the Direct Command to be performed                                   */
    switch(txCmd) {
    case RFAL_14443A_SHORTFRAME_CMD_WUPA:
        directCmd = ST25R3916_CMD_TRANSMIT_WUPA;
        break;

    case RFAL_14443A_SHORTFRAME_CMD_REQA:
        directCmd = ST25R3916_CMD_TRANSMIT_REQA;
        break;

    default:
        return ERR_PARAM;
    }

    /* Disable CRC while receiving since ATQA has no CRC included */
    st25r3916SetRegisterBits(ST25R3916_REG_AUX, ST25R3916_REG_AUX_no_crc_rx);

    /*******************************************************************************/
    /* Wait for GT and FDT */
    while(!rfalIsGTExpired()) { /* MISRA 15.6: mandatory brackets */
    };
    while(st25r3916IsGPTRunning()) { /* MISRA 15.6: mandatory brackets */
    };

    rfalTimerDestroy(gRFAL.tmr.GT);
    gRFAL.tmr.GT = RFAL_TIMING_NONE;

    /*******************************************************************************/
    /* Prepare for Transceive, Receive only (bypass Tx states) */
    gRFAL.TxRx.ctx.flags =
        ((uint32_t)RFAL_TXRX_FLAGS_CRC_TX_MANUAL | (uint32_t)RFAL_TXRX_FLAGS_CRC_RX_KEEP);
    gRFAL.TxRx.ctx.rxBuf = rxBuf;
    gRFAL.TxRx.ctx.rxBufLen = rxBufLen;
    gRFAL.TxRx.ctx.rxRcvdLen = rxRcvdLen;
    gRFAL.TxRx.ctx.fwt = fwt;

    /*******************************************************************************/
    /* Load NRT with FWT */
    st25r3916SetNoResponseTime(rfalConv1fcTo64fc(
        MIN((fwt + RFAL_FWT_ADJUSTMENT + RFAL_FWT_A_ADJUSTMENT), RFAL_ST25R3916_NRT_MAX_1FC)));

    if(gRFAL.timings.FDTListen != RFAL_TIMING_NONE) {
        /* Ensure that MRT is using 64/fc steps */
        st25r3916ClrRegisterBits(
            ST25R3916_REG_TIMER_EMV_CONTROL, ST25R3916_REG_TIMER_EMV_CONTROL_mrt_step);

        /* Set Minimum FDT(Listen) in which PICC is not allowed to send a response */
        st25r3916WriteRegister(
            ST25R3916_REG_MASK_RX_TIMER,
            (uint8_t)rfalConv1fcTo64fc(
                ((RFAL_FDT_LISTEN_MRT_ADJUSTMENT + RFAL_FDT_LISTEN_A_ADJUSTMENT) >
                 gRFAL.timings.FDTListen) ?
                    RFAL_ST25R3916_MRT_MIN_1FC :
                    (gRFAL.timings.FDTListen -
                     (RFAL_FDT_LISTEN_MRT_ADJUSTMENT + RFAL_FDT_LISTEN_A_ADJUSTMENT))));
    }

    /* In Passive communications General Purpose Timer is used to measure FDT Poll */
    if(gRFAL.timings.FDTPoll != RFAL_TIMING_NONE) {
        /* Configure GPT to start at RX end */
        st25r3916SetStartGPTimer(
            (uint16_t)rfalConv1fcTo8fc(
                MIN(gRFAL.timings.FDTPoll, (gRFAL.timings.FDTPoll - RFAL_FDT_POLL_ADJUSTMENT))),
            ST25R3916_REG_TIMER_EMV_CONTROL_gptc_erx);
    }

    /*******************************************************************************/
    rfalPrepareTransceive();

    /* Also enable bit collision interrupt */
    st25r3916GetInterrupt(ST25R3916_IRQ_MASK_COL);
    st25r3916EnableInterrupts(ST25R3916_IRQ_MASK_COL);

    /*Check if Observation Mode is enabled and set it on ST25R391x */
    rfalCheckEnableObsModeTx();

    /*******************************************************************************/
    /* Clear nbtx bits before sending WUPA/REQA - otherwise ST25R3916 will report parity error, Note2 of the register */
    st25r3916WriteRegister(ST25R3916_REG_NUM_TX_BYTES2, 0);

    /* Send either WUPA or REQA. All affected tags will backscatter ATQA and change to READY state */
    st25r3916ExecuteCommand(directCmd);

    /* Wait for TXE */
    if(st25r3916WaitForInterruptsTimed(
           ST25R3916_IRQ_MASK_TXE,
           (uint16_t)MAX(rfalConv1fcToMs(fwt), RFAL_ST25R3916_SW_TMR_MIN_1MS)) == 0U) {
        ret = ERR_IO;
    } else {
        /*Check if Observation Mode is enabled and set it on ST25R391x */
        rfalCheckEnableObsModeRx();

        /* Jump into a transceive Rx state for reception (bypass Tx states) */
        gRFAL.state = RFAL_STATE_TXRX;
        gRFAL.TxRx.state = RFAL_TXRX_STATE_RX_IDLE;
        gRFAL.TxRx.status = ERR_BUSY;

        /* Execute Transceive Rx blocking */
        ret = rfalTransceiveBlockingRx();
    }

    /* Disable Collision interrupt */
    st25r3916DisableInterrupts((ST25R3916_IRQ_MASK_COL));

    /* ReEnable CRC on Rx */
    st25r3916ClrRegisterBits(ST25R3916_REG_AUX, ST25R3916_REG_AUX_no_crc_rx);

    return ret;
}

/*******************************************************************************/
ReturnCode rfalISO14443ATransceiveAnticollisionFrame(
    uint8_t* buf,
    uint8_t* bytesToSend,
    uint8_t* bitsToSend,
    uint16_t* rxLength,
    uint32_t fwt) {
    ReturnCode ret;
    rfalTransceiveContext ctx;
    uint8_t collByte;
    uint8_t collData;

    /* Check if RFAL is properly initialized */
    if((gRFAL.state < RFAL_STATE_MODE_SET) || (gRFAL.mode != RFAL_MODE_POLL_NFCA)) {
        return ERR_WRONG_STATE;
    }

    /* Check for valid parameters */
    if((buf == NULL) || (bytesToSend == NULL) || (bitsToSend == NULL) || (rxLength == NULL)) {
        return ERR_PARAM;
    }

    /*******************************************************************************/
    /* Set specific Analog Config for Anticolission if needed */
    rfalSetAnalogConfig(
        (RFAL_ANALOG_CONFIG_POLL | RFAL_ANALOG_CONFIG_TECH_NFCA |
         RFAL_ANALOG_CONFIG_BITRATE_COMMON | RFAL_ANALOG_CONFIG_ANTICOL));

    /*******************************************************************************/
    /* Enable anti collision to recognise collision in first byte of SENS_REQ */
    st25r3916SetRegisterBits(ST25R3916_REG_ISO14443A_NFC, ST25R3916_REG_ISO14443A_NFC_antcl);

    /* Disable CRC while receiving */
    st25r3916SetRegisterBits(ST25R3916_REG_AUX, ST25R3916_REG_AUX_no_crc_rx);

    /*******************************************************************************/
    /* Prepare for Transceive                                                      */
    ctx.flags = ((uint32_t)RFAL_TXRX_FLAGS_CRC_TX_MANUAL | (uint32_t)RFAL_TXRX_FLAGS_CRC_RX_KEEP);
    ctx.txBuf = buf;
    ctx.txBufLen = (uint16_t)(rfalConvBytesToBits(*bytesToSend) + *bitsToSend);
    ctx.rxBuf = &buf[*bytesToSend];
    ctx.rxBufLen = (uint16_t)rfalConvBytesToBits(RFAL_ISO14443A_SDD_RES_LEN);
    ctx.rxRcvdLen = rxLength;
    ctx.fwt = fwt;

    /* Disable Automatic Gain Control (AGC) for better detection of collisions if using Coherent Receiver */
    ctx.flags |=
        (st25r3916CheckReg(
             ST25R3916_REG_AUX, ST25R3916_REG_AUX_dis_corr, ST25R3916_REG_AUX_dis_corr) ?
             (uint32_t)RFAL_TXRX_FLAGS_AGC_OFF :
             0x00U);

    rfalStartTransceive(&ctx);

    /* Additionally enable bit collision interrupt */
    st25r3916GetInterrupt(ST25R3916_IRQ_MASK_COL);
    st25r3916EnableInterrupts(ST25R3916_IRQ_MASK_COL);

    /*******************************************************************************/
    collByte = 0;

    /* save the collision byte */
    if((*bitsToSend) > 0U) {
        buf[(*bytesToSend)] <<= (RFAL_BITS_IN_BYTE - (*bitsToSend));
        buf[(*bytesToSend)] >>= (RFAL_BITS_IN_BYTE - (*bitsToSend));
        collByte = buf[(*bytesToSend)];
    }

    /*******************************************************************************/
    /* Run Transceive blocking */
    ret = rfalTransceiveRunBlockingTx();
    if(ret == ERR_NONE) {
        ret = rfalTransceiveBlockingRx();

        /*******************************************************************************/
        if((*bitsToSend) > 0U) {
            buf[(*bytesToSend)] >>= (*bitsToSend);
            buf[(*bytesToSend)] <<= (*bitsToSend);
            buf[(*bytesToSend)] |= collByte;
        }

        if((ERR_RF_COLLISION == ret)) {
            /* read out collision register */
            st25r3916ReadRegister(ST25R3916_REG_COLLISION_STATUS, &collData);

            (*bytesToSend) =
                ((collData >> ST25R3916_REG_COLLISION_STATUS_c_byte_shift) &
                 0x0FU); // 4-bits Byte information
            (*bitsToSend) =
                ((collData >> ST25R3916_REG_COLLISION_STATUS_c_bit_shift) &
                 0x07U); // 3-bits bit information
        }
    }

    /*******************************************************************************/
    /* Disable Collision interrupt */
    st25r3916DisableInterrupts((ST25R3916_IRQ_MASK_COL));

    /* Disable anti collision again */
    st25r3916ClrRegisterBits(ST25R3916_REG_ISO14443A_NFC, ST25R3916_REG_ISO14443A_NFC_antcl);

    /* ReEnable CRC on Rx */
    st25r3916ClrRegisterBits(ST25R3916_REG_AUX, ST25R3916_REG_AUX_no_crc_rx);
    /*******************************************************************************/

    /* Restore common Analog configurations for this mode */
    rfalSetAnalogConfig(
        (RFAL_ANALOG_CONFIG_POLL | RFAL_ANALOG_CONFIG_TECH_NFCA | rfalConvBR2ACBR(gRFAL.txBR) |
         RFAL_ANALOG_CONFIG_TX));
    rfalSetAnalogConfig(
        (RFAL_ANALOG_CONFIG_POLL | RFAL_ANALOG_CONFIG_TECH_NFCA | rfalConvBR2ACBR(gRFAL.rxBR) |
         RFAL_ANALOG_CONFIG_RX));

    return ret;
}

#endif /* RFAL_FEATURE_NFCA */

#if RFAL_FEATURE_NFCV

/*******************************************************************************/
ReturnCode rfalISO15693TransceiveAnticollisionFrame(
    uint8_t* txBuf,
    uint8_t txBufLen,
    uint8_t* rxBuf,
    uint8_t rxBufLen,
    uint16_t* actLen) {
    ReturnCode ret;
    rfalTransceiveContext ctx;

    /* Check if RFAL is properly initialized */
    if((gRFAL.state < RFAL_STATE_MODE_SET) || (gRFAL.mode != RFAL_MODE_POLL_NFCV)) {
        return ERR_WRONG_STATE;
    }

    /*******************************************************************************/
    /* Set specific Analog Config for Anticolission if needed */
    rfalSetAnalogConfig(
        (RFAL_ANALOG_CONFIG_POLL | RFAL_ANALOG_CONFIG_TECH_NFCV |
         RFAL_ANALOG_CONFIG_BITRATE_COMMON | RFAL_ANALOG_CONFIG_ANTICOL));

    /* Ignoring collisions before the UID (RES_FLAG + DSFID) */
    gRFAL.nfcvData.ignoreBits = (uint16_t)RFAL_ISO15693_IGNORE_BITS;

    /*******************************************************************************/
    /* Prepare for Transceive  */
    ctx.flags =
        ((txBufLen == 0U) ? (uint32_t)RFAL_TXRX_FLAGS_CRC_TX_MANUAL :
                            (uint32_t)RFAL_TXRX_FLAGS_CRC_TX_AUTO) |
        (uint32_t)RFAL_TXRX_FLAGS_CRC_RX_KEEP | (uint32_t)RFAL_TXRX_FLAGS_AGC_OFF |
        ((txBufLen == 0U) ?
             (uint32_t)RFAL_TXRX_FLAGS_NFCV_FLAG_MANUAL :
             (uint32_t)
                 RFAL_TXRX_FLAGS_NFCV_FLAG_AUTO); /* Disable Automatic Gain Control (AGC) for better detection of collision */
    ctx.txBuf = txBuf;
    ctx.txBufLen = (uint16_t)rfalConvBytesToBits(txBufLen);
    ctx.rxBuf = rxBuf;
    ctx.rxBufLen = (uint16_t)rfalConvBytesToBits(rxBufLen);
    ctx.rxRcvdLen = actLen;
    ctx.fwt = rfalConv64fcTo1fc(ISO15693_FWT);

    rfalStartTransceive(&ctx);

    /*******************************************************************************/
    /* Run Transceive blocking */
    ret = rfalTransceiveRunBlockingTx();
    if(ret == ERR_NONE) {
        ret = rfalTransceiveBlockingRx();
    }

    /* Check if a Transmission error and received data is less then expected */
    if(((ret == ERR_RF_COLLISION) || (ret == ERR_CRC) || (ret == ERR_FRAMING)) &&
       (rfalConvBitsToBytes(*ctx.rxRcvdLen) < RFAL_ISO15693_INV_RES_LEN)) {
        /* If INVENTORY_RES is shorter than expected, tag is still modulating *
         * Ensure that response is complete before next frame                 */
        platformDelay((
            uint8_t)((RFAL_ISO15693_INV_RES_LEN - rfalConvBitsToBytes(*ctx.rxRcvdLen)) / ((RFAL_ISO15693_INV_RES_LEN / RFAL_ISO15693_INV_RES_DUR) + 1U)));
    }

    /* Restore common Analog configurations for this mode */
    rfalSetAnalogConfig(
        (RFAL_ANALOG_CONFIG_POLL | RFAL_ANALOG_CONFIG_TECH_NFCV | rfalConvBR2ACBR(gRFAL.txBR) |
         RFAL_ANALOG_CONFIG_TX));
    rfalSetAnalogConfig(
        (RFAL_ANALOG_CONFIG_POLL | RFAL_ANALOG_CONFIG_TECH_NFCV | rfalConvBR2ACBR(gRFAL.rxBR) |
         RFAL_ANALOG_CONFIG_RX));

    gRFAL.nfcvData.ignoreBits = 0;
    return ret;
}

/*******************************************************************************/
ReturnCode
    rfalISO15693TransceiveEOFAnticollision(uint8_t* rxBuf, uint8_t rxBufLen, uint16_t* actLen) {
    uint8_t dummy;

    return rfalISO15693TransceiveAnticollisionFrame(&dummy, 0, rxBuf, rxBufLen, actLen);
}

/*******************************************************************************/
ReturnCode rfalISO15693TransceiveEOF(uint8_t* rxBuf, uint8_t rxBufLen, uint16_t* actLen) {
    ReturnCode ret;
    uint8_t dummy;

    /* Check if RFAL is properly initialized */
    if((gRFAL.state < RFAL_STATE_MODE_SET) || (gRFAL.mode != RFAL_MODE_POLL_NFCV)) {
        return ERR_WRONG_STATE;
    }

    /*******************************************************************************/
    /* Run Transceive blocking */
    ret = rfalTransceiveBlockingTxRx(
        &dummy,
        0,
        rxBuf,
        rxBufLen,
        actLen,
        ((uint32_t)RFAL_TXRX_FLAGS_CRC_TX_MANUAL | (uint32_t)RFAL_TXRX_FLAGS_CRC_RX_KEEP |
         (uint32_t)RFAL_TXRX_FLAGS_AGC_ON),
        rfalConv64fcTo1fc(ISO15693_FWT));
    return ret;
}

#endif /* RFAL_FEATURE_NFCV */

#if RFAL_FEATURE_NFCF

/*******************************************************************************/
ReturnCode rfalFeliCaPoll(
    rfalFeliCaPollSlots slots,
    uint16_t sysCode,
    uint8_t reqCode,
    rfalFeliCaPollRes* pollResList,
    uint8_t pollResListSize,
    uint8_t* devicesDetected,
    uint8_t* collisionsDetected) {
    ReturnCode ret;
    uint8_t frame
        [RFAL_FELICA_POLL_REQ_LEN - RFAL_FELICA_LEN_LEN]; // LEN is added by ST25R391x automatically
    uint16_t actLen;
    uint8_t frameIdx;
    uint8_t devDetected;
    uint8_t colDetected;
    rfalEHandling curHandling;
    uint8_t nbSlots;

    /* Check if RFAL is properly initialized */
    if((gRFAL.state < RFAL_STATE_MODE_SET) || (gRFAL.mode != RFAL_MODE_POLL_NFCF)) {
        return ERR_WRONG_STATE;
    }

    frameIdx = 0;
    colDetected = 0;
    devDetected = 0;
    nbSlots = (uint8_t)slots;

    /*******************************************************************************/
    /* Compute SENSF_REQ frame */
    frame[frameIdx++] = (uint8_t)FELICA_CMD_POLLING; /* CMD: SENF_REQ                       */
    frame[frameIdx++] = (uint8_t)(sysCode >> 8); /* System Code (SC)                    */
    frame[frameIdx++] = (uint8_t)(sysCode & 0xFFU); /* System Code (SC)                    */
    frame[frameIdx++] = reqCode; /* Communication Parameter Request (RC)*/
    frame[frameIdx++] = nbSlots; /* TimeSlot (TSN)                      */

    /*******************************************************************************/
    /* NRT should not stop on reception - Use EMVCo mode to run NRT in nrt_emv     *
     * ERRORHANDLING_EMVCO has no special handling for NFC-F mode                  */
    curHandling = gRFAL.conf.eHandling;
    rfalSetErrorHandling(RFAL_ERRORHANDLING_EMVCO);

    /*******************************************************************************/
    /* Run transceive blocking, 
     * Calculate Total Response Time in(64/fc): 
     *                       512 PICC process time + (n * 256 Time Slot duration)  */
    ret = rfalTransceiveBlockingTx(
        frame,
        (uint16_t)frameIdx,
        (uint8_t*)gRFAL.nfcfData.pollResponses,
        RFAL_FELICA_POLL_RES_LEN,
        &actLen,
        (RFAL_TXRX_FLAGS_DEFAULT),
        rfalConv64fcTo1fc(
            RFAL_FELICA_POLL_DELAY_TIME +
            (RFAL_FELICA_POLL_SLOT_TIME * ((uint32_t)nbSlots + 1U))));

    /*******************************************************************************/
    /* If Tx OK, Wait for all responses, store them as soon as they appear         */
    if(ret == ERR_NONE) {
        bool timeout;

        do {
            ret = rfalTransceiveBlockingRx();
            if(ret == ERR_TIMEOUT) {
                /* Upon timeout the full Poll Delay + (Slot time)*(nbSlots) has expired */
                timeout = true;
            } else {
                /* Reception done, reEnabled Rx for following Slot */
                st25r3916ExecuteCommand(ST25R3916_CMD_UNMASK_RECEIVE_DATA);
                st25r3916ExecuteCommand(ST25R3916_CMD_RESET_RXGAIN);

                /* If the reception was OK, new device found */
                if(ret == ERR_NONE) {
                    devDetected++;

                    /* Overwrite the Transceive context for the next reception */
                    gRFAL.TxRx.ctx.rxBuf = (uint8_t*)gRFAL.nfcfData.pollResponses[devDetected];
                }
                /* If the reception was not OK, mark as collision */
                else {
                    colDetected++;
                }

                /* Check whether NRT has expired meanwhile  */
                timeout = st25r3916CheckReg(
                    ST25R3916_REG_NFCIP1_BIT_RATE, ST25R3916_REG_NFCIP1_BIT_RATE_nrt_on, 0x00);
                if(!timeout) {
                    /* Jump again into transceive Rx state for the following reception */
                    gRFAL.TxRx.status = ERR_BUSY;
                    gRFAL.state = RFAL_STATE_TXRX;
                    gRFAL.TxRx.state = RFAL_TXRX_STATE_RX_IDLE;
                }
            }
        } while(((nbSlots--) != 0U) && !timeout);
    }

    /*******************************************************************************/
    /* Restore NRT to normal mode - back to previous error handling */
    rfalSetErrorHandling(curHandling);

    /*******************************************************************************/
    /* Assign output parameters if requested                                       */

    if((pollResList != NULL) && (pollResListSize > 0U) && (devDetected > 0U)) {
        ST_MEMCPY(
            pollResList,
            gRFAL.nfcfData.pollResponses,
            (RFAL_FELICA_POLL_RES_LEN * (uint32_t)MIN(pollResListSize, devDetected)));
    }

    if(devicesDetected != NULL) {
        *devicesDetected = devDetected;
    }

    if(collisionsDetected != NULL) {
        *collisionsDetected = colDetected;
    }

    return (((colDetected != 0U) || (devDetected != 0U)) ? ERR_NONE : ret);
}

#endif /* RFAL_FEATURE_NFCF */

/*****************************************************************************
 *  Listen Mode                                                              *  
 *****************************************************************************/

/*******************************************************************************/
bool rfalIsExtFieldOn(void) {
    return st25r3916IsExtFieldOn();
}

#if RFAL_FEATURE_LISTEN_MODE

/*******************************************************************************/
ReturnCode rfalListenStart(
    uint32_t lmMask,
    const rfalLmConfPA* confA,
    const rfalLmConfPB* confB,
    const rfalLmConfPF* confF,
    uint8_t* rxBuf,
    uint16_t rxBufLen,
    uint16_t* rxLen) {
    t_rfalPTMem
        PTMem; /*  PRQA S 0759 # MISRA 19.2 - Allocating Union where members are of the same type, just different names.  Thus no problem can occur. */
    uint8_t* pPTMem;
    uint8_t autoResp;

    /* Check if RFAL is initialized */
    if(gRFAL.state < RFAL_STATE_INIT) {
        return ERR_WRONG_STATE;
    }

    gRFAL.Lm.state = RFAL_LM_STATE_NOT_INIT;
    gRFAL.Lm.mdIrqs = ST25R3916_IRQ_MASK_NONE;
    gRFAL.Lm.mdReg =
        (ST25R3916_REG_MODE_targ_init | ST25R3916_REG_MODE_om_nfc | ST25R3916_REG_MODE_nfc_ar_off);

    /* By default disable all automatic responses */
    autoResp =
        (ST25R3916_REG_PASSIVE_TARGET_d_106_ac_a | ST25R3916_REG_PASSIVE_TARGET_rfu |
         ST25R3916_REG_PASSIVE_TARGET_d_212_424_1r | ST25R3916_REG_PASSIVE_TARGET_d_ac_ap2p);

    /*******************************************************************************/
    if((lmMask & RFAL_LM_MASK_NFCA) != 0U) {
        /* Check if the conf has been provided */
        if(confA == NULL) {
            return ERR_PARAM;
        }

        pPTMem = (uint8_t*)PTMem.PTMem_A;

        /*******************************************************************************/
        /* Check and set supported NFCID Length */
        switch(confA->nfcidLen) {
        case RFAL_LM_NFCID_LEN_04:
            st25r3916ChangeRegisterBits(
                ST25R3916_REG_AUX, ST25R3916_REG_AUX_nfc_id_mask, ST25R3916_REG_AUX_nfc_id_4bytes);
            break;

        case RFAL_LM_NFCID_LEN_07:
            st25r3916ChangeRegisterBits(
                ST25R3916_REG_AUX, ST25R3916_REG_AUX_nfc_id_mask, ST25R3916_REG_AUX_nfc_id_7bytes);
            break;

        default:
            return ERR_PARAM;
        }

        /*******************************************************************************/
        /* Set NFCID */
        ST_MEMCPY(pPTMem, confA->nfcid, RFAL_NFCID1_TRIPLE_LEN);
        pPTMem = &pPTMem[RFAL_NFCID1_TRIPLE_LEN]; /* MISRA 18.4 */

        /* Set SENS_RES */
        ST_MEMCPY(pPTMem, confA->SENS_RES, RFAL_LM_SENS_RES_LEN);
        pPTMem = &pPTMem[RFAL_LM_SENS_RES_LEN]; /* MISRA 18.4 */

        /* Set SEL_RES */
        *pPTMem++ =
            ((confA->nfcidLen == RFAL_LM_NFCID_LEN_04) ?
                 (confA->SEL_RES & ~RFAL_LM_NFCID_INCOMPLETE) :
                 (confA->SEL_RES | RFAL_LM_NFCID_INCOMPLETE));
        *pPTMem++ = (confA->SEL_RES & ~RFAL_LM_NFCID_INCOMPLETE);
        *pPTMem++ = (confA->SEL_RES & ~RFAL_LM_NFCID_INCOMPLETE);

        /* Write into PTMem-A */
        st25r3916WritePTMem(PTMem.PTMem_A, ST25R3916_PTM_A_LEN);

        /*******************************************************************************/
        /* Enable automatic responses for A */
        autoResp &= ~ST25R3916_REG_PASSIVE_TARGET_d_106_ac_a;

        /* Set Target mode, Bit Rate detection and Listen Mode for NFC-F */
        gRFAL.Lm.mdReg |=
            (ST25R3916_REG_MODE_targ_targ | ST25R3916_REG_MODE_om3 | ST25R3916_REG_MODE_om0 |
             ST25R3916_REG_MODE_nfc_ar_off);

        gRFAL.Lm.mdIrqs |=
            (ST25R3916_IRQ_MASK_WU_A | ST25R3916_IRQ_MASK_WU_A_X | ST25R3916_IRQ_MASK_RXE_PTA);
    }

    /*******************************************************************************/
    if((lmMask & RFAL_LM_MASK_NFCB) != 0U) {
        /* Check if the conf has been provided */
        if(confB == NULL) {
            return ERR_PARAM;
        }

        return ERR_NOTSUPP;
    }

    /*******************************************************************************/
    if((lmMask & RFAL_LM_MASK_NFCF) != 0U) {
        pPTMem = (uint8_t*)PTMem.PTMem_F;

        /* Check if the conf has been provided */
        if(confF == NULL) {
            return ERR_PARAM;
        }

        /*******************************************************************************/
        /* Set System Code */
        ST_MEMCPY(pPTMem, confF->SC, RFAL_LM_SENSF_SC_LEN);
        pPTMem = &pPTMem[RFAL_LM_SENSF_SC_LEN]; /* MISRA 18.4 */

        /* Set SENSF_RES */
        ST_MEMCPY(pPTMem, confF->SENSF_RES, RFAL_LM_SENSF_RES_LEN);

        /* Set RD bytes to 0x00 as ST25R3916 cannot support advances features */
        pPTMem[RFAL_LM_SENSF_RD0_POS] =
            0x00; /* NFC Forum Digital 1.1 Table 46: 0x00                   */
        pPTMem[RFAL_LM_SENSF_RD1_POS] =
            0x00; /* NFC Forum Digital 1.1 Table 47: No automatic bit rates */

        pPTMem = &pPTMem[RFAL_LM_SENS_RES_LEN]; /* MISRA 18.4 */

        /* Write into PTMem-F */
        st25r3916WritePTMemF(PTMem.PTMem_F, ST25R3916_PTM_F_LEN);

        /*******************************************************************************/
        /* Write 24 TSN "Random" Numbers at first initialization and let it rollover   */
        if(!gRFAL.Lm.iniFlag) {
            pPTMem = (uint8_t*)PTMem.TSN;

            *pPTMem++ = 0x12;
            *pPTMem++ = 0x34;
            *pPTMem++ = 0x56;
            *pPTMem++ = 0x78;
            *pPTMem++ = 0x9A;
            *pPTMem++ = 0xBC;
            *pPTMem++ = 0xDF;
            *pPTMem++ = 0x21;
            *pPTMem++ = 0x43;
            *pPTMem++ = 0x65;
            *pPTMem++ = 0x87;
            *pPTMem++ = 0xA9;

            /* Write into PTMem-TSN */
            st25r3916WritePTMemTSN(PTMem.TSN, ST25R3916_PTM_TSN_LEN);
        }

        /*******************************************************************************/
        /* Enable automatic responses for F */
        autoResp &= ~(ST25R3916_REG_PASSIVE_TARGET_d_212_424_1r);

        /* Set Target mode, Bit Rate detection and Listen Mode for NFC-F */
        gRFAL.Lm.mdReg |=
            (ST25R3916_REG_MODE_targ_targ | ST25R3916_REG_MODE_om3 | ST25R3916_REG_MODE_om2 |
             ST25R3916_REG_MODE_nfc_ar_off);

        /* In CE NFC-F any data without error will be passed to FIFO, to support CUP */
        gRFAL.Lm.mdIrqs |=
            (ST25R3916_IRQ_MASK_WU_F | ST25R3916_IRQ_MASK_RXE_PTA | ST25R3916_IRQ_MASK_RXE);
    }

    /*******************************************************************************/
    if((lmMask & RFAL_LM_MASK_ACTIVE_P2P) != 0U) {
        /* Enable Reception of P2P frames */
        autoResp &= ~(ST25R3916_REG_PASSIVE_TARGET_d_ac_ap2p);

        /* Set Target mode, Bit Rate detection and Automatic Response RF Collision Avoidance */
        gRFAL.Lm.mdReg |=
            (ST25R3916_REG_MODE_targ_targ | ST25R3916_REG_MODE_om3 | ST25R3916_REG_MODE_om2 |
             ST25R3916_REG_MODE_om0 | ST25R3916_REG_MODE_nfc_ar_auto_rx);

        /* n * TRFW timing shall vary  Activity 2.1  3.4.1.1 */
        st25r3916ChangeRegisterBits(
            ST25R3916_REG_AUX, ST25R3916_REG_AUX_nfc_n_mask, gRFAL.timings.nTRFW);
        gRFAL.timings.nTRFW = rfalGennTRFW(gRFAL.timings.nTRFW);

        gRFAL.Lm.mdIrqs |= (ST25R3916_IRQ_MASK_RXE);
    }

    /* Check if one of the modes were selected */
    if((gRFAL.Lm.mdReg & ST25R3916_REG_MODE_targ) == ST25R3916_REG_MODE_targ_targ) {
        gRFAL.state = RFAL_STATE_LM;
        gRFAL.Lm.mdMask = lmMask;

        gRFAL.Lm.rxBuf = rxBuf;
        gRFAL.Lm.rxBufLen = rxBufLen;
        gRFAL.Lm.rxLen = rxLen;
        *gRFAL.Lm.rxLen = 0;
        gRFAL.Lm.dataFlag = false;
        gRFAL.Lm.iniFlag = true;

        /* Apply the Automatic Responses configuration */
        st25r3916ChangeRegisterBits(
            ST25R3916_REG_PASSIVE_TARGET,
            (ST25R3916_REG_PASSIVE_TARGET_d_106_ac_a | ST25R3916_REG_PASSIVE_TARGET_rfu |
             ST25R3916_REG_PASSIVE_TARGET_d_212_424_1r | ST25R3916_REG_PASSIVE_TARGET_d_ac_ap2p),
            autoResp);

        /* Disable GPT trigger source */
        st25r3916ChangeRegisterBits(
            ST25R3916_REG_TIMER_EMV_CONTROL,
            ST25R3916_REG_TIMER_EMV_CONTROL_gptc_mask,
            ST25R3916_REG_TIMER_EMV_CONTROL_gptc_no_trigger);

        /* On Bit Rate Detection Mode ST25R391x will filter incoming frames during MRT time starting on External Field On event, use 512/fc steps */
        st25r3916SetRegisterBits(
            ST25R3916_REG_TIMER_EMV_CONTROL, ST25R3916_REG_TIMER_EMV_CONTROL_mrt_step_512);
        st25r3916WriteRegister(
            ST25R3916_REG_MASK_RX_TIMER, (uint8_t)rfalConv1fcTo512fc(RFAL_LM_GT));

        /* Restore default settings on NFCIP1 mode, Receiving parity + CRC bits and manual Tx Parity*/
        st25r3916ClrRegisterBits(
            ST25R3916_REG_ISO14443A_NFC,
            (ST25R3916_REG_ISO14443A_NFC_no_tx_par | ST25R3916_REG_ISO14443A_NFC_no_rx_par |
             ST25R3916_REG_ISO14443A_NFC_nfc_f0));

        /* External Field Detector enabled as Automatics on rfalInitialize() */

        /* Set Analog configurations for generic Listen mode */
        /* Not on SetState(POWER OFF) as otherwise would be applied on every Field Event */
        rfalSetAnalogConfig((RFAL_ANALOG_CONFIG_TECH_CHIP | RFAL_ANALOG_CONFIG_CHIP_LISTEN_ON));

        /* Initialize as POWER_OFF and set proper mode in RF Chip */
        rfalListenSetState(RFAL_LM_STATE_POWER_OFF);
    } else {
        return ERR_REQUEST; /* Listen Start called but no mode was enabled */
    }

    return ERR_NONE;
}

/*******************************************************************************/
static ReturnCode rfalRunListenModeWorker(void) {
    volatile uint32_t irqs;
    uint8_t tmp;

    if(gRFAL.state != RFAL_STATE_LM) {
        return ERR_WRONG_STATE;
    }

    switch(gRFAL.Lm.state) {
    /*******************************************************************************/
    case RFAL_LM_STATE_POWER_OFF:

        irqs = st25r3916GetInterrupt((ST25R3916_IRQ_MASK_EON));
        if(irqs == ST25R3916_IRQ_MASK_NONE) {
            break; /* No interrupt to process */
        }

        if((irqs & ST25R3916_IRQ_MASK_EON) != 0U) {
            rfalListenSetState(RFAL_LM_STATE_IDLE);
        } else {
            break;
        }
        /* fall through */

    /*******************************************************************************/
    case RFAL_LM_STATE_IDLE: /*  PRQA S 2003 # MISRA 16.3 - Intentional fall through */

        irqs = st25r3916GetInterrupt(
            (ST25R3916_IRQ_MASK_NFCT | ST25R3916_IRQ_MASK_WU_F | ST25R3916_IRQ_MASK_RXE |
             ST25R3916_IRQ_MASK_EOF | ST25R3916_IRQ_MASK_RXE_PTA));
        if(irqs == ST25R3916_IRQ_MASK_NONE) {
            break; /* No interrupt to process */
        }

        if((irqs & ST25R3916_IRQ_MASK_NFCT) != 0U) {
            /* Retrieve detected bitrate */
            uint8_t newBr;
            st25r3916ReadRegister(ST25R3916_REG_NFCIP1_BIT_RATE, &newBr);
            newBr >>= ST25R3916_REG_NFCIP1_BIT_RATE_nfc_rate_shift;

            if(newBr > ST25R3916_REG_BIT_RATE_rxrate_424) {
                newBr = ST25R3916_REG_BIT_RATE_rxrate_424;
            }

            gRFAL.Lm.brDetected =
                (rfalBitRate)(newBr); /* PRQA S 4342 # MISRA 10.5 - Guaranteed that no invalid enum values may be created. See also equalityGuard_RFAL_BR_106 ff.*/
        }

        if(((irqs & ST25R3916_IRQ_MASK_WU_F) != 0U) && (gRFAL.Lm.brDetected != RFAL_BR_KEEP)) {
            rfalListenSetState(RFAL_LM_STATE_READY_F);
        } else if(((irqs & ST25R3916_IRQ_MASK_RXE) != 0U) && (gRFAL.Lm.brDetected != RFAL_BR_KEEP)) {
            irqs = st25r3916GetInterrupt(
                (ST25R3916_IRQ_MASK_WU_F | ST25R3916_IRQ_MASK_RXE | ST25R3916_IRQ_MASK_EOF |
                 ST25R3916_IRQ_MASK_CRC | ST25R3916_IRQ_MASK_PAR | ST25R3916_IRQ_MASK_ERR2 |
                 ST25R3916_IRQ_MASK_ERR1));

            if(((irqs & ST25R3916_IRQ_MASK_CRC) != 0U) ||
               ((irqs & ST25R3916_IRQ_MASK_PAR) != 0U) ||
               ((irqs & ST25R3916_IRQ_MASK_ERR1) != 0U)) {
                st25r3916ExecuteCommand(ST25R3916_CMD_CLEAR_FIFO);
                st25r3916ExecuteCommand(ST25R3916_CMD_UNMASK_RECEIVE_DATA);
                st25r3916TxOff();
                break; /* A bad reception occurred, remain in same state */
            }

            /* Retrieve received data */
            *gRFAL.Lm.rxLen = st25r3916GetNumFIFOBytes();
            st25r3916ReadFifo(
                gRFAL.Lm.rxBuf, MIN(*gRFAL.Lm.rxLen, rfalConvBitsToBytes(gRFAL.Lm.rxBufLen)));

            /*******************************************************************************/
            /* REMARK: Silicon workaround ST25R3916 Errata #TBD                            */
            /* In bitrate detection mode CRC is now checked for NFC-A frames               */
            if((*gRFAL.Lm.rxLen > RFAL_CRC_LEN) && (gRFAL.Lm.brDetected == RFAL_BR_106)) {
                if(rfalCrcCalculateCcitt(
                       RFAL_ISO14443A_CRC_INTVAL, gRFAL.Lm.rxBuf, *gRFAL.Lm.rxLen) != 0U) {
                    st25r3916ExecuteCommand(ST25R3916_CMD_CLEAR_FIFO);
                    st25r3916ExecuteCommand(ST25R3916_CMD_UNMASK_RECEIVE_DATA);
                    st25r3916TxOff();
                    break; /* A bad reception occurred, remain in same state */
                }
            }
            /*******************************************************************************/

            /* Check if the data we got has at least the CRC and remove it, otherwise leave at 0 */
            *gRFAL.Lm.rxLen -= ((*gRFAL.Lm.rxLen > RFAL_CRC_LEN) ? RFAL_CRC_LEN : *gRFAL.Lm.rxLen);
            *gRFAL.Lm.rxLen = (uint16_t)rfalConvBytesToBits(*gRFAL.Lm.rxLen);
            gRFAL.Lm.dataFlag = true;

            /*Check if Observation Mode was enabled and disable it on ST25R391x */
            rfalCheckDisableObsMode();
        } else if(
            ((irqs & ST25R3916_IRQ_MASK_RXE_PTA) != 0U) && (gRFAL.Lm.brDetected != RFAL_BR_KEEP)) {
            if(((gRFAL.Lm.mdMask & RFAL_LM_MASK_NFCA) != 0U) &&
               (gRFAL.Lm.brDetected == RFAL_BR_106)) {
                st25r3916ReadRegister(ST25R3916_REG_PASSIVE_TARGET_STATUS, &tmp);
                if(tmp > ST25R3916_REG_PASSIVE_TARGET_STATUS_pta_st_idle) {
                    rfalListenSetState(RFAL_LM_STATE_READY_A);
                }
            }
        } else if(((irqs & ST25R3916_IRQ_MASK_EOF) != 0U) && (!gRFAL.Lm.dataFlag)) {
            rfalListenSetState(RFAL_LM_STATE_POWER_OFF);
        } else {
            /* MISRA 15.7 - Empty else */
        }
        break;

    /*******************************************************************************/
    case RFAL_LM_STATE_READY_F:

        irqs = st25r3916GetInterrupt(
            (ST25R3916_IRQ_MASK_WU_F | ST25R3916_IRQ_MASK_RXE | ST25R3916_IRQ_MASK_EOF));
        if(irqs == ST25R3916_IRQ_MASK_NONE) {
            break; /* No interrupt to process */
        }

        if((irqs & ST25R3916_IRQ_MASK_WU_F) != 0U) {
            break;
        } else if((irqs & ST25R3916_IRQ_MASK_RXE) != 0U) {
            /* Retrieve the error flags/irqs */
            irqs |= st25r3916GetInterrupt(
                (ST25R3916_IRQ_MASK_CRC | ST25R3916_IRQ_MASK_ERR2 | ST25R3916_IRQ_MASK_ERR1));

            if(((irqs & ST25R3916_IRQ_MASK_CRC) != 0U) ||
               ((irqs & ST25R3916_IRQ_MASK_ERR1) != 0U)) {
                st25r3916ExecuteCommand(ST25R3916_CMD_CLEAR_FIFO);
                st25r3916ExecuteCommand(ST25R3916_CMD_UNMASK_RECEIVE_DATA);
                break; /* A bad reception occurred, remain in same state */
            }

            /* Retrieve received data */
            *gRFAL.Lm.rxLen = st25r3916GetNumFIFOBytes();
            st25r3916ReadFifo(
                gRFAL.Lm.rxBuf, MIN(*gRFAL.Lm.rxLen, rfalConvBitsToBytes(gRFAL.Lm.rxBufLen)));

            /* Check if the data we got has at least the CRC and remove it, otherwise leave at 0 */
            *gRFAL.Lm.rxLen -= ((*gRFAL.Lm.rxLen > RFAL_CRC_LEN) ? RFAL_CRC_LEN : *gRFAL.Lm.rxLen);
            *gRFAL.Lm.rxLen = (uint16_t)rfalConvBytesToBits(*gRFAL.Lm.rxLen);
            gRFAL.Lm.dataFlag = true;
        } else if((irqs & ST25R3916_IRQ_MASK_EOF) != 0U) {
            rfalListenSetState(RFAL_LM_STATE_POWER_OFF);
        } else {
            /* MISRA 15.7 - Empty else */
        }
        break;

    /*******************************************************************************/
    case RFAL_LM_STATE_READY_A:

        irqs = st25r3916GetInterrupt((ST25R3916_IRQ_MASK_EOF | ST25R3916_IRQ_MASK_WU_A));
        if(irqs == ST25R3916_IRQ_MASK_NONE) {
            break; /* No interrupt to process */
        }

        if((irqs & ST25R3916_IRQ_MASK_WU_A) != 0U) {
            rfalListenSetState(RFAL_LM_STATE_ACTIVE_A);
        } else if((irqs & ST25R3916_IRQ_MASK_EOF) != 0U) {
            rfalListenSetState(RFAL_LM_STATE_POWER_OFF);
        } else {
            /* MISRA 15.7 - Empty else */
        }
        break;

    /*******************************************************************************/
    case RFAL_LM_STATE_ACTIVE_A:
    case RFAL_LM_STATE_ACTIVE_Ax:

        irqs = st25r3916GetInterrupt((ST25R3916_IRQ_MASK_RXE | ST25R3916_IRQ_MASK_EOF));
        if(irqs == ST25R3916_IRQ_MASK_NONE) {
            break; /* No interrupt to process */
        }

        if((irqs & ST25R3916_IRQ_MASK_RXE) != 0U) {
            /* Retrieve the error flags/irqs */
            irqs |= st25r3916GetInterrupt(
                (ST25R3916_IRQ_MASK_PAR | ST25R3916_IRQ_MASK_CRC | ST25R3916_IRQ_MASK_ERR2 |
                 ST25R3916_IRQ_MASK_ERR1));
            *gRFAL.Lm.rxLen = st25r3916GetNumFIFOBytes();

            if(((irqs & ST25R3916_IRQ_MASK_CRC) != 0U) ||
               ((irqs & ST25R3916_IRQ_MASK_ERR1) != 0U) ||
               ((irqs & ST25R3916_IRQ_MASK_PAR) != 0U) || (*gRFAL.Lm.rxLen <= RFAL_CRC_LEN)) {
                /* Clear rx context and FIFO */
                *gRFAL.Lm.rxLen = 0;
                st25r3916ExecuteCommand(ST25R3916_CMD_CLEAR_FIFO);
                st25r3916ExecuteCommand(ST25R3916_CMD_UNMASK_RECEIVE_DATA);

                /* Check if we should go to IDLE or Sleep */
                if(gRFAL.Lm.state == RFAL_LM_STATE_ACTIVE_Ax) {
                    rfalListenSleepStart(
                        RFAL_LM_STATE_SLEEP_A, gRFAL.Lm.rxBuf, gRFAL.Lm.rxBufLen, gRFAL.Lm.rxLen);
                } else {
                    rfalListenSetState(RFAL_LM_STATE_IDLE);
                }

                st25r3916DisableInterrupts(ST25R3916_IRQ_MASK_RXE);
                break;
            }

            /* Remove CRC from length */
            *gRFAL.Lm.rxLen -= RFAL_CRC_LEN;

            /* Retrieve received data */
            st25r3916ReadFifo(
                gRFAL.Lm.rxBuf, MIN(*gRFAL.Lm.rxLen, rfalConvBitsToBytes(gRFAL.Lm.rxBufLen)));
            *gRFAL.Lm.rxLen = (uint16_t)rfalConvBytesToBits(*gRFAL.Lm.rxLen);
            gRFAL.Lm.dataFlag = true;
        } else if((irqs & ST25R3916_IRQ_MASK_EOF) != 0U) {
            rfalListenSetState(RFAL_LM_STATE_POWER_OFF);
        } else {
            /* MISRA 15.7 - Empty else */
        }
        break;

    /*******************************************************************************/
    case RFAL_LM_STATE_SLEEP_A:
    case RFAL_LM_STATE_SLEEP_B:
    case RFAL_LM_STATE_SLEEP_AF:

        irqs = st25r3916GetInterrupt(
            (ST25R3916_IRQ_MASK_NFCT | ST25R3916_IRQ_MASK_WU_F | ST25R3916_IRQ_MASK_RXE |
             ST25R3916_IRQ_MASK_EOF | ST25R3916_IRQ_MASK_RXE_PTA));
        if(irqs == ST25R3916_IRQ_MASK_NONE) {
            break; /* No interrupt to process */
        }

        if((irqs & ST25R3916_IRQ_MASK_NFCT) != 0U) {
            uint8_t newBr;
            /* Retrieve detected bitrate */
            st25r3916ReadRegister(ST25R3916_REG_NFCIP1_BIT_RATE, &newBr);
            newBr >>= ST25R3916_REG_NFCIP1_BIT_RATE_nfc_rate_shift;

            if(newBr > ST25R3916_REG_BIT_RATE_rxrate_424) {
                newBr = ST25R3916_REG_BIT_RATE_rxrate_424;
            }

            gRFAL.Lm.brDetected =
                (rfalBitRate)(newBr); /* PRQA S 4342 # MISRA 10.5 - Guaranteed that no invalid enum values may be created. See also equalityGuard_RFAL_BR_106 ff.*/
        }

        if(((irqs & ST25R3916_IRQ_MASK_WU_F) != 0U) && (gRFAL.Lm.brDetected != RFAL_BR_KEEP)) {
            rfalListenSetState(RFAL_LM_STATE_READY_F);
        } else if(((irqs & ST25R3916_IRQ_MASK_RXE) != 0U) && (gRFAL.Lm.brDetected != RFAL_BR_KEEP)) {
            /* Clear rx context and FIFO */
            *gRFAL.Lm.rxLen = 0;
            st25r3916ExecuteCommand(ST25R3916_CMD_CLEAR_FIFO);
            st25r3916ExecuteCommand(ST25R3916_CMD_UNMASK_RECEIVE_DATA);

            /* REMARK: In order to support CUP or proprietary frames, handling could be added here */
        } else if(
            ((irqs & ST25R3916_IRQ_MASK_RXE_PTA) != 0U) && (gRFAL.Lm.brDetected != RFAL_BR_KEEP)) {
            if(((gRFAL.Lm.mdMask & RFAL_LM_MASK_NFCA) != 0U) &&
               (gRFAL.Lm.brDetected == RFAL_BR_106)) {
                st25r3916ReadRegister(ST25R3916_REG_PASSIVE_TARGET_STATUS, &tmp);
                if(tmp > ST25R3916_REG_PASSIVE_TARGET_STATUS_pta_st_halt) {
                    rfalListenSetState(RFAL_LM_STATE_READY_Ax);
                }
            }
        } else if((irqs & ST25R3916_IRQ_MASK_EOF) != 0U) {
            rfalListenSetState(RFAL_LM_STATE_POWER_OFF);
        } else {
            /* MISRA 15.7 - Empty else */
        }
        break;

    /*******************************************************************************/
    case RFAL_LM_STATE_READY_Ax:

        irqs = st25r3916GetInterrupt((ST25R3916_IRQ_MASK_EOF | ST25R3916_IRQ_MASK_WU_A_X));
        if(irqs == ST25R3916_IRQ_MASK_NONE) {
            break; /* No interrupt to process */
        }

        if((irqs & ST25R3916_IRQ_MASK_WU_A_X) != 0U) {
            rfalListenSetState(RFAL_LM_STATE_ACTIVE_Ax);
        } else if((irqs & ST25R3916_IRQ_MASK_EOF) != 0U) {
            rfalListenSetState(RFAL_LM_STATE_POWER_OFF);
        } else {
            /* MISRA 15.7 - Empty else */
        }
        break;

    /*******************************************************************************/
    case RFAL_LM_STATE_CARDEMU_4A:
    case RFAL_LM_STATE_CARDEMU_4B:
    case RFAL_LM_STATE_CARDEMU_3:
    case RFAL_LM_STATE_TARGET_F:
    case RFAL_LM_STATE_TARGET_A:
        break;

    /*******************************************************************************/
    default:
        return ERR_WRONG_STATE;
    }
    return ERR_NONE;
}

/*******************************************************************************/
ReturnCode rfalListenStop(void) {
    /* Check if RFAL is initialized */
    if(gRFAL.state < RFAL_STATE_INIT) {
        return ERR_WRONG_STATE;
    }

    gRFAL.Lm.state = RFAL_LM_STATE_NOT_INIT;

    /*Check if Observation Mode was enabled and disable it on ST25R391x */
    rfalCheckDisableObsMode();

    /* Re-Enable the Oscillator if not running */
    st25r3916OscOn();

    /* Disable Receiver and Transmitter */
    rfalFieldOff();

    /* Disable all automatic responses */
    st25r3916SetRegisterBits(
        ST25R3916_REG_PASSIVE_TARGET,
        (ST25R3916_REG_PASSIVE_TARGET_d_212_424_1r | ST25R3916_REG_PASSIVE_TARGET_rfu |
         ST25R3916_REG_PASSIVE_TARGET_d_106_ac_a | ST25R3916_REG_PASSIVE_TARGET_d_ac_ap2p));

    /* As there's no Off mode, set default value: ISO14443A with automatic RF Collision Avoidance Off */
    st25r3916WriteRegister(
        ST25R3916_REG_MODE,
        (ST25R3916_REG_MODE_om_iso14443a | ST25R3916_REG_MODE_tr_am_ook |
         ST25R3916_REG_MODE_nfc_ar_off));

    st25r3916DisableInterrupts(
        (ST25R3916_IRQ_MASK_RXE_PTA | ST25R3916_IRQ_MASK_WU_F | ST25R3916_IRQ_MASK_WU_A |
         ST25R3916_IRQ_MASK_WU_A_X | ST25R3916_IRQ_MASK_RFU2 | ST25R3916_IRQ_MASK_OSC));
    st25r3916GetInterrupt(
        (ST25R3916_IRQ_MASK_RXE_PTA | ST25R3916_IRQ_MASK_WU_F | ST25R3916_IRQ_MASK_WU_A |
         ST25R3916_IRQ_MASK_WU_A_X | ST25R3916_IRQ_MASK_RFU2));

    /* Set Analog configurations for Listen Off event */
    rfalSetAnalogConfig((RFAL_ANALOG_CONFIG_TECH_CHIP | RFAL_ANALOG_CONFIG_CHIP_LISTEN_OFF));

    return ERR_NONE;
}

/*******************************************************************************/
ReturnCode
    rfalListenSleepStart(rfalLmState sleepSt, uint8_t* rxBuf, uint16_t rxBufLen, uint16_t* rxLen) {
    /* Check if RFAL is not initialized */
    if(gRFAL.state < RFAL_STATE_INIT) {
        return ERR_WRONG_STATE;
    }

    switch(sleepSt) {
    /*******************************************************************************/
    case RFAL_LM_STATE_SLEEP_A:

        /* Enable automatic responses for A */
        st25r3916ClrRegisterBits(
            ST25R3916_REG_PASSIVE_TARGET, (ST25R3916_REG_PASSIVE_TARGET_d_106_ac_a));

        /* Reset NFCA target */
        st25r3916ExecuteCommand(ST25R3916_CMD_GOTO_SLEEP);

        /* Set Target mode, Bit Rate detection and Listen Mode for NFC-A */
        st25r3916ChangeRegisterBits(
            ST25R3916_REG_MODE,
            (ST25R3916_REG_MODE_targ | ST25R3916_REG_MODE_om_mask |
             ST25R3916_REG_MODE_nfc_ar_mask),
            (ST25R3916_REG_MODE_targ_targ | ST25R3916_REG_MODE_om3 | ST25R3916_REG_MODE_om0 |
             ST25R3916_REG_MODE_nfc_ar_off));
        break;

    /*******************************************************************************/
    case RFAL_LM_STATE_SLEEP_AF:

        /* Enable automatic responses for A + F */
        st25r3916ClrRegisterBits(
            ST25R3916_REG_PASSIVE_TARGET,
            (ST25R3916_REG_PASSIVE_TARGET_d_212_424_1r | ST25R3916_REG_PASSIVE_TARGET_d_106_ac_a));

        /* Reset NFCA target state */
        st25r3916ExecuteCommand(ST25R3916_CMD_GOTO_SLEEP);

        /* Set Target mode, Bit Rate detection, Listen Mode for NFC-A and NFC-F */
        st25r3916ChangeRegisterBits(
            ST25R3916_REG_MODE,
            (ST25R3916_REG_MODE_targ | ST25R3916_REG_MODE_om_mask |
             ST25R3916_REG_MODE_nfc_ar_mask),
            (ST25R3916_REG_MODE_targ_targ | ST25R3916_REG_MODE_om3 | ST25R3916_REG_MODE_om2 |
             ST25R3916_REG_MODE_om0 | ST25R3916_REG_MODE_nfc_ar_off));
        break;

    /*******************************************************************************/
    case RFAL_LM_STATE_SLEEP_B:
        /* REMARK: Support for CE-B would be added here  */
        return ERR_NOT_IMPLEMENTED;

    /*******************************************************************************/
    default:
        return ERR_PARAM;
    }

    /* Ensure that the  NFCIP1 mode is disabled */
    st25r3916ClrRegisterBits(ST25R3916_REG_ISO14443A_NFC, ST25R3916_REG_ISO14443A_NFC_nfc_f0);

    st25r3916ExecuteCommand(ST25R3916_CMD_UNMASK_RECEIVE_DATA);

    /* Clear and enable required IRQs */
    st25r3916ClearAndEnableInterrupts(
        (ST25R3916_IRQ_MASK_NFCT | ST25R3916_IRQ_MASK_RXS | ST25R3916_IRQ_MASK_CRC |
         ST25R3916_IRQ_MASK_ERR1 | ST25R3916_IRQ_MASK_ERR2 | ST25R3916_IRQ_MASK_PAR |
         ST25R3916_IRQ_MASK_EON | ST25R3916_IRQ_MASK_EOF | gRFAL.Lm.mdIrqs));

    /* Check whether the field was turn off right after the Sleep request */
    if(!rfalIsExtFieldOn()) {
        /*rfalLogD( "RFAL: curState: %02X newState: %02X \r\n", gRFAL.Lm.state, RFAL_LM_STATE_NOT_INIT );*/

        rfalListenStop();
        return ERR_LINK_LOSS;
    }

    /*rfalLogD( "RFAL: curState: %02X newState: %02X \r\n", gRFAL.Lm.state, sleepSt );*/

    /* Set the new Sleep State*/
    gRFAL.Lm.state = sleepSt;
    gRFAL.state = RFAL_STATE_LM;

    gRFAL.Lm.rxBuf = rxBuf;
    gRFAL.Lm.rxBufLen = rxBufLen;
    gRFAL.Lm.rxLen = rxLen;
    *gRFAL.Lm.rxLen = 0;
    gRFAL.Lm.dataFlag = false;

    return ERR_NONE;
}

/*******************************************************************************/
rfalLmState rfalListenGetState(bool* dataFlag, rfalBitRate* lastBR) {
    /* Allow state retrieval even if gRFAL.state != RFAL_STATE_LM so  *
     * that this Lm state can be used by caller after activation      */

    if(lastBR != NULL) {
        *lastBR = gRFAL.Lm.brDetected;
    }

    if(dataFlag != NULL) {
        *dataFlag = gRFAL.Lm.dataFlag;
    }

    return gRFAL.Lm.state;
}

/*******************************************************************************/
ReturnCode rfalListenSetState(rfalLmState newSt) {
    ReturnCode ret;
    rfalLmState newState;
    bool reSetState;

    /* Check if RFAL is initialized */
    if(gRFAL.state < RFAL_STATE_INIT) {
        return ERR_WRONG_STATE;
    }

    /* SetState clears the Data flag */
    gRFAL.Lm.dataFlag = false;
    newState = newSt;
    ret = ERR_NONE;

    do {
        reSetState = false;

        /*******************************************************************************/
        switch(newState) {
        /*******************************************************************************/
        case RFAL_LM_STATE_POWER_OFF:

            /* Enable the receiver and reset logic */
            st25r3916SetRegisterBits(ST25R3916_REG_OP_CONTROL, ST25R3916_REG_OP_CONTROL_rx_en);
            st25r3916ExecuteCommand(ST25R3916_CMD_STOP);

            if((gRFAL.Lm.mdMask & RFAL_LM_MASK_NFCA) != 0U) {
                /* Enable automatic responses for A */
                st25r3916ClrRegisterBits(
                    ST25R3916_REG_PASSIVE_TARGET, ST25R3916_REG_PASSIVE_TARGET_d_106_ac_a);

                /* Prepares the NFCIP-1 Passive target logic to wait in the Sense/Idle state */
                st25r3916ExecuteCommand(ST25R3916_CMD_GOTO_SENSE);
            }

            if((gRFAL.Lm.mdMask & RFAL_LM_MASK_NFCF) != 0U) {
                /* Enable automatic responses for F */
                st25r3916ClrRegisterBits(
                    ST25R3916_REG_PASSIVE_TARGET, (ST25R3916_REG_PASSIVE_TARGET_d_212_424_1r));
            }

            if((gRFAL.Lm.mdMask & RFAL_LM_MASK_ACTIVE_P2P) != 0U) {
                /* Ensure automatic response RF Collision Avoidance is back to only after Rx */
                st25r3916ChangeRegisterBits(
                    ST25R3916_REG_MODE,
                    ST25R3916_REG_MODE_nfc_ar_mask,
                    ST25R3916_REG_MODE_nfc_ar_auto_rx);

                /* Ensure that our field is Off, as automatic response RF Collision Avoidance may have been triggered */
                st25r3916TxOff();
            }

            /*******************************************************************************/
            /* Ensure that the  NFCIP1 mode is disabled */
            st25r3916ClrRegisterBits(
                ST25R3916_REG_ISO14443A_NFC, ST25R3916_REG_ISO14443A_NFC_nfc_f0);

            /*******************************************************************************/
            /* Clear and enable required IRQs */
            st25r3916DisableInterrupts(ST25R3916_IRQ_MASK_ALL);

            st25r3916ClearAndEnableInterrupts(
                (ST25R3916_IRQ_MASK_NFCT | ST25R3916_IRQ_MASK_RXS | ST25R3916_IRQ_MASK_CRC |
                 ST25R3916_IRQ_MASK_ERR1 | ST25R3916_IRQ_MASK_OSC | ST25R3916_IRQ_MASK_ERR2 |
                 ST25R3916_IRQ_MASK_PAR | ST25R3916_IRQ_MASK_EON | ST25R3916_IRQ_MASK_EOF |
                 gRFAL.Lm.mdIrqs));

            /*******************************************************************************/
            /* Clear the bitRate previously detected */
            gRFAL.Lm.brDetected = RFAL_BR_KEEP;

            /*******************************************************************************/
            /* Apply the initial mode */
            st25r3916ChangeRegisterBits(
                ST25R3916_REG_MODE,
                (ST25R3916_REG_MODE_targ | ST25R3916_REG_MODE_om_mask |
                 ST25R3916_REG_MODE_nfc_ar_mask),
                (uint8_t)gRFAL.Lm.mdReg);

            /*******************************************************************************/
            /* Check if external Field is already On */
            if(rfalIsExtFieldOn()) {
                reSetState = true;
                newState = RFAL_LM_STATE_IDLE; /* Set IDLE state */
            }
#if 1 /* Perform bit rate detection in Low power mode */
            else {
                st25r3916ClrRegisterBits(
                    ST25R3916_REG_OP_CONTROL,
                    (ST25R3916_REG_OP_CONTROL_tx_en | ST25R3916_REG_OP_CONTROL_rx_en |
                     ST25R3916_REG_OP_CONTROL_en));
            }
#endif
            break;

        /*******************************************************************************/
        case RFAL_LM_STATE_IDLE:

            /*******************************************************************************/
            /* Check if device is coming from Low Power bit rate detection */
            if(!st25r3916CheckReg(
                   ST25R3916_REG_OP_CONTROL,
                   ST25R3916_REG_OP_CONTROL_en,
                   ST25R3916_REG_OP_CONTROL_en)) {
                /* Exit Low Power mode and confirm the temporarily enable */
                st25r3916SetRegisterBits(
                    ST25R3916_REG_OP_CONTROL,
                    (ST25R3916_REG_OP_CONTROL_en | ST25R3916_REG_OP_CONTROL_rx_en));

                if(!st25r3916CheckReg(
                       ST25R3916_REG_AUX_DISPLAY,
                       ST25R3916_REG_AUX_DISPLAY_osc_ok,
                       ST25R3916_REG_AUX_DISPLAY_osc_ok)) {
                    /* Wait for Oscillator ready */
                    if(st25r3916WaitForInterruptsTimed(
                           ST25R3916_IRQ_MASK_OSC, ST25R3916_TOUT_OSC_STABLE) == 0U) {
                        ret = ERR_IO;
                        break;
                    }
                }
            } else {
                st25r3916GetInterrupt(ST25R3916_IRQ_MASK_OSC);
            }

            /*******************************************************************************/
            /* In Active P2P the Initiator may:  Turn its field On;  LM goes into IDLE state;
                 *      Initiator sends an unexpected frame raising a Protocol error; Initiator 
                 *      turns its field Off and ST25R3916 performs the automatic RF Collision 
                 *      Avoidance keeping our field On; upon a Protocol error upper layer sets 
                 *      again the state to IDLE to clear dataFlag and wait for next data.
                 *      
                 * Ensure that when upper layer calls SetState(IDLE), it restores initial 
                 * configuration and that check whether an external Field is still present     */
            if((gRFAL.Lm.mdMask & RFAL_LM_MASK_ACTIVE_P2P) != 0U) {
                /* Ensure nfc_ar is reset and back to only after Rx */
                st25r3916ExecuteCommand(ST25R3916_CMD_STOP);
                st25r3916ChangeRegisterBits(
                    ST25R3916_REG_MODE,
                    ST25R3916_REG_MODE_nfc_ar_mask,
                    ST25R3916_REG_MODE_nfc_ar_auto_rx);

                /* Ensure that our field is Off, as automatic response RF Collision Avoidance may have been triggered */
                st25r3916TxOff();

                /* If external Field is no longer detected go back to POWER_OFF */
                if(!st25r3916IsExtFieldOn()) {
                    reSetState = true;
                    newState = RFAL_LM_STATE_POWER_OFF; /* Set POWER_OFF state */
                }
            }
            /*******************************************************************************/

            /* If we are in ACTIVE_A, reEnable Listen for A before going to IDLE, otherwise do nothing */
            if(gRFAL.Lm.state == RFAL_LM_STATE_ACTIVE_A) {
                /* Enable automatic responses for A and Reset NFCA target state */
                st25r3916ClrRegisterBits(
                    ST25R3916_REG_PASSIVE_TARGET, (ST25R3916_REG_PASSIVE_TARGET_d_106_ac_a));
                st25r3916ExecuteCommand(ST25R3916_CMD_GOTO_SENSE);
            }

            /* ReEnable the receiver */
            st25r3916ExecuteCommand(ST25R3916_CMD_CLEAR_FIFO);
            st25r3916ExecuteCommand(ST25R3916_CMD_UNMASK_RECEIVE_DATA);

            /*******************************************************************************/
            /*Check if Observation Mode is enabled and set it on ST25R391x */
            rfalCheckEnableObsModeRx();
            break;

        /*******************************************************************************/
        case RFAL_LM_STATE_READY_F:

            /*******************************************************************************/
            /* If we're coming from BitRate detection mode, the Bit Rate Definition reg 
                 * still has the last bit rate used.
                 * If a frame is received between setting the mode to Listen NFCA and 
                 * setting Bit Rate Definition reg, it will raise a framing error.
                 * Set the bitrate immediately, and then the normal SetMode procedure          */
            st25r3916SetBitrate((uint8_t)gRFAL.Lm.brDetected, (uint8_t)gRFAL.Lm.brDetected);
            /*******************************************************************************/

            /* Disable automatic responses for NFC-A */
            st25r3916SetRegisterBits(
                ST25R3916_REG_PASSIVE_TARGET, (ST25R3916_REG_PASSIVE_TARGET_d_106_ac_a));

            /* Set Mode NFC-F only */
            ret = rfalSetMode(RFAL_MODE_LISTEN_NFCF, gRFAL.Lm.brDetected, gRFAL.Lm.brDetected);
            gRFAL.state = RFAL_STATE_LM; /* Keep in Listen Mode */

            /* ReEnable the receiver */
            st25r3916ExecuteCommand(ST25R3916_CMD_CLEAR_FIFO);
            st25r3916ExecuteCommand(ST25R3916_CMD_UNMASK_RECEIVE_DATA);

            /* Clear any previous transmission errors (if Reader polled for other/unsupported technologies) */
            st25r3916GetInterrupt(
                (ST25R3916_IRQ_MASK_PAR | ST25R3916_IRQ_MASK_CRC | ST25R3916_IRQ_MASK_ERR2 |
                 ST25R3916_IRQ_MASK_ERR1));

            st25r3916EnableInterrupts(
                ST25R3916_IRQ_MASK_RXE); /* Start looking for any incoming data */
            break;

        /*******************************************************************************/
        case RFAL_LM_STATE_CARDEMU_3:

            /* Set Listen NFCF mode  */
            ret = rfalSetMode(RFAL_MODE_LISTEN_NFCF, gRFAL.Lm.brDetected, gRFAL.Lm.brDetected);
            break;

        /*******************************************************************************/
        case RFAL_LM_STATE_READY_Ax:
        case RFAL_LM_STATE_READY_A:

            /*******************************************************************************/
            /* If we're coming from BitRate detection mode, the Bit Rate Definition reg 
                 * still has the last bit rate used.
                 * If a frame is received between setting the mode to Listen NFCA and 
                 * setting Bit Rate Definition reg, it will raise a framing error.
                 * Set the bitrate immediately, and then the normal SetMode procedure          */
            st25r3916SetBitrate((uint8_t)gRFAL.Lm.brDetected, (uint8_t)gRFAL.Lm.brDetected);
            /*******************************************************************************/

            /* Disable automatic responses for NFC-F */
            st25r3916SetRegisterBits(
                ST25R3916_REG_PASSIVE_TARGET, (ST25R3916_REG_PASSIVE_TARGET_d_212_424_1r));

            /* Set Mode NFC-A only */
            ret = rfalSetMode(RFAL_MODE_LISTEN_NFCA, gRFAL.Lm.brDetected, gRFAL.Lm.brDetected);

            gRFAL.state = RFAL_STATE_LM; /* Keep in Listen Mode */
            break;

        /*******************************************************************************/
        case RFAL_LM_STATE_ACTIVE_Ax:
        case RFAL_LM_STATE_ACTIVE_A:

            /* Disable automatic responses for A */
            st25r3916SetRegisterBits(
                ST25R3916_REG_PASSIVE_TARGET, (ST25R3916_REG_PASSIVE_TARGET_d_106_ac_a));

            /* Clear any previous transmission errors (if Reader polled for other/unsupported technologies) */
            st25r3916GetInterrupt(
                (ST25R3916_IRQ_MASK_PAR | ST25R3916_IRQ_MASK_CRC | ST25R3916_IRQ_MASK_ERR2 |
                 ST25R3916_IRQ_MASK_ERR1));

            st25r3916EnableInterrupts(
                ST25R3916_IRQ_MASK_RXE); /* Start looking for any incoming data */
            break;

        case RFAL_LM_STATE_TARGET_F:
            /* Disable Automatic response SENSF_REQ */
            st25r3916SetRegisterBits(
                ST25R3916_REG_PASSIVE_TARGET, (ST25R3916_REG_PASSIVE_TARGET_d_212_424_1r));
            break;

        /*******************************************************************************/
        case RFAL_LM_STATE_SLEEP_A:
        case RFAL_LM_STATE_SLEEP_B:
        case RFAL_LM_STATE_SLEEP_AF:
            /* These sleep states have to be set by the rfalListenSleepStart() method */
            return ERR_REQUEST;

        /*******************************************************************************/
        case RFAL_LM_STATE_CARDEMU_4A:
        case RFAL_LM_STATE_CARDEMU_4B:
        case RFAL_LM_STATE_TARGET_A:
            /* States not handled by the LM, just keep state context */
            break;

        /*******************************************************************************/
        default:
            return ERR_WRONG_STATE;
        }
    } while(reSetState);

    gRFAL.Lm.state = newState;

    // Call callback on state change
    if(gRFAL.callbacks.state_changed_cb) {
        gRFAL.callbacks.state_changed_cb(gRFAL.callbacks.ctx);
    }

    return ret;
}

#endif /* RFAL_FEATURE_LISTEN_MODE */

/*******************************************************************************
 *  Wake-Up Mode                                                               *
 *******************************************************************************/

#if RFAL_FEATURE_WAKEUP_MODE

/*******************************************************************************/
ReturnCode rfalWakeUpModeStart(const rfalWakeUpConfig* config) {
    uint8_t aux;
    uint8_t reg;
    uint32_t irqs;

    /* Check if RFAL is not initialized */
    if(gRFAL.state < RFAL_STATE_INIT) {
        return ERR_WRONG_STATE;
    }

    /* The Wake-Up procedure is explained in detail in Application Note: AN4985 */

    if(config == NULL) {
        gRFAL.wum.cfg.period = RFAL_WUM_PERIOD_200MS;
        gRFAL.wum.cfg.irqTout = false;
        gRFAL.wum.cfg.indAmp.enabled = true;
        gRFAL.wum.cfg.indPha.enabled = false;
        gRFAL.wum.cfg.cap.enabled = false;
        gRFAL.wum.cfg.indAmp.delta = 2U;
        gRFAL.wum.cfg.indAmp.reference = RFAL_WUM_REFERENCE_AUTO;
        gRFAL.wum.cfg.indAmp.autoAvg = false;

        /*******************************************************************************/
        /* Check if AAT is enabled and if so make use of the SW Tag Detection          */
        if(st25r3916CheckReg(
               ST25R3916_REG_IO_CONF2,
               ST25R3916_REG_IO_CONF2_aat_en,
               ST25R3916_REG_IO_CONF2_aat_en)) {
            gRFAL.wum.cfg.swTagDetect = true;
            gRFAL.wum.cfg.indAmp.autoAvg = true;
            gRFAL.wum.cfg.indAmp.aaWeight = RFAL_WUM_AA_WEIGHT_16;
        }
    } else {
        gRFAL.wum.cfg = *config;
    }

    /* Check for valid configuration */
    if((!gRFAL.wum.cfg.cap.enabled && !gRFAL.wum.cfg.indAmp.enabled &&
        !gRFAL.wum.cfg.indPha.enabled) ||
       (gRFAL.wum.cfg.cap.enabled &&
        (gRFAL.wum.cfg.indAmp.enabled || gRFAL.wum.cfg.indPha.enabled)) ||
       (gRFAL.wum.cfg.cap.enabled && gRFAL.wum.cfg.swTagDetect) ||
       ((gRFAL.wum.cfg.indAmp.reference > RFAL_WUM_REFERENCE_AUTO) ||
        (gRFAL.wum.cfg.indPha.reference > RFAL_WUM_REFERENCE_AUTO) ||
        (gRFAL.wum.cfg.cap.reference > RFAL_WUM_REFERENCE_AUTO))) {
        return ERR_PARAM;
    }

    irqs = ST25R3916_IRQ_MASK_NONE;

    /* Disable Tx, Rx, External Field Detector and set default ISO14443A mode */
    st25r3916TxRxOff();
    st25r3916ClrRegisterBits(ST25R3916_REG_OP_CONTROL, ST25R3916_REG_OP_CONTROL_en_fd_mask);
    st25r3916ChangeRegisterBits(
        ST25R3916_REG_MODE,
        (ST25R3916_REG_MODE_targ | ST25R3916_REG_MODE_om_mask),
        (ST25R3916_REG_MODE_targ_init | ST25R3916_REG_MODE_om_iso14443a));

    /* Set Analog configurations for Wake-up On event */
    rfalSetAnalogConfig((RFAL_ANALOG_CONFIG_TECH_CHIP | RFAL_ANALOG_CONFIG_CHIP_WAKEUP_ON));

    /*******************************************************************************/
    /* Prepare Wake-Up Timer Control Register */
    reg =
        (uint8_t)(((uint8_t)gRFAL.wum.cfg.period & 0x0FU) << ST25R3916_REG_WUP_TIMER_CONTROL_wut_shift);
    reg |=
        (uint8_t)(((uint8_t)gRFAL.wum.cfg.period < (uint8_t)RFAL_WUM_PERIOD_100MS) ? ST25R3916_REG_WUP_TIMER_CONTROL_wur : 0x00U);

    if(gRFAL.wum.cfg.irqTout || gRFAL.wum.cfg.swTagDetect) {
        reg |= ST25R3916_REG_WUP_TIMER_CONTROL_wto;
        irqs |= ST25R3916_IRQ_MASK_WT;
    }

    /* Check if HW Wake-up is to be used or SW Tag detection */
    if(gRFAL.wum.cfg.swTagDetect) {
        gRFAL.wum.cfg.indAmp.reference = 0U;
        gRFAL.wum.cfg.indPha.reference = 0U;
        gRFAL.wum.cfg.cap.reference = 0U;
    } else {
        /*******************************************************************************/
        /* Check if Inductive Amplitude is to be performed */
        if(gRFAL.wum.cfg.indAmp.enabled) {
            aux =
                (uint8_t)((gRFAL.wum.cfg.indAmp.delta) << ST25R3916_REG_AMPLITUDE_MEASURE_CONF_am_d_shift);
            aux |=
                (uint8_t)(gRFAL.wum.cfg.indAmp.aaInclMeas ? ST25R3916_REG_AMPLITUDE_MEASURE_CONF_am_aam : 0x00U);
            aux |=
                (uint8_t)(((uint8_t)gRFAL.wum.cfg.indAmp.aaWeight << ST25R3916_REG_AMPLITUDE_MEASURE_CONF_am_aew_shift) & ST25R3916_REG_AMPLITUDE_MEASURE_CONF_am_aew_mask);
            aux |=
                (uint8_t)(gRFAL.wum.cfg.indAmp.autoAvg ? ST25R3916_REG_AMPLITUDE_MEASURE_CONF_am_ae : 0x00U);

            st25r3916WriteRegister(ST25R3916_REG_AMPLITUDE_MEASURE_CONF, aux);

            /* Only need to set the reference if not using Auto Average */
            if(!gRFAL.wum.cfg.indAmp.autoAvg) {
                if(gRFAL.wum.cfg.indAmp.reference == RFAL_WUM_REFERENCE_AUTO) {
                    st25r3916MeasureAmplitude(&aux);
                    gRFAL.wum.cfg.indAmp.reference = aux;
                }
                st25r3916WriteRegister(
                    ST25R3916_REG_AMPLITUDE_MEASURE_REF, (uint8_t)gRFAL.wum.cfg.indAmp.reference);
            }

            reg |= ST25R3916_REG_WUP_TIMER_CONTROL_wam;
            irqs |= ST25R3916_IRQ_MASK_WAM;
        }

        /*******************************************************************************/
        /* Check if Inductive Phase is to be performed */
        if(gRFAL.wum.cfg.indPha.enabled) {
            aux =
                (uint8_t)((gRFAL.wum.cfg.indPha.delta) << ST25R3916_REG_PHASE_MEASURE_CONF_pm_d_shift);
            aux |=
                (uint8_t)(gRFAL.wum.cfg.indPha.aaInclMeas ? ST25R3916_REG_PHASE_MEASURE_CONF_pm_aam : 0x00U);
            aux |=
                (uint8_t)(((uint8_t)gRFAL.wum.cfg.indPha.aaWeight << ST25R3916_REG_PHASE_MEASURE_CONF_pm_aew_shift) & ST25R3916_REG_PHASE_MEASURE_CONF_pm_aew_mask);
            aux |=
                (uint8_t)(gRFAL.wum.cfg.indPha.autoAvg ? ST25R3916_REG_PHASE_MEASURE_CONF_pm_ae : 0x00U);

            st25r3916WriteRegister(ST25R3916_REG_PHASE_MEASURE_CONF, aux);

            /* Only need to set the reference if not using Auto Average */
            if(!gRFAL.wum.cfg.indPha.autoAvg) {
                if(gRFAL.wum.cfg.indPha.reference == RFAL_WUM_REFERENCE_AUTO) {
                    st25r3916MeasurePhase(&aux);
                    gRFAL.wum.cfg.indPha.reference = aux;
                }
                st25r3916WriteRegister(
                    ST25R3916_REG_PHASE_MEASURE_REF, (uint8_t)gRFAL.wum.cfg.indPha.reference);
            }

            reg |= ST25R3916_REG_WUP_TIMER_CONTROL_wph;
            irqs |= ST25R3916_IRQ_MASK_WPH;
        }

        /*******************************************************************************/
        /* Check if Capacitive is to be performed */
        if(gRFAL.wum.cfg.cap.enabled) {
            /*******************************************************************************/
            /* Perform Capacitive sensor calibration */

            /* Disable Oscillator and Field */
            st25r3916ClrRegisterBits(
                ST25R3916_REG_OP_CONTROL,
                (ST25R3916_REG_OP_CONTROL_en | ST25R3916_REG_OP_CONTROL_tx_en));

            /* Sensor gain should be configured on Analog Config: RFAL_ANALOG_CONFIG_CHIP_WAKEUP_ON */

            /* Perform calibration procedure */
            st25r3916CalibrateCapacitiveSensor(NULL);

            /*******************************************************************************/
            aux =
                (uint8_t)((gRFAL.wum.cfg.cap.delta) << ST25R3916_REG_CAPACITANCE_MEASURE_CONF_cm_d_shift);
            aux |=
                (uint8_t)(gRFAL.wum.cfg.cap.aaInclMeas ? ST25R3916_REG_CAPACITANCE_MEASURE_CONF_cm_aam : 0x00U);
            aux |=
                (uint8_t)(((uint8_t)gRFAL.wum.cfg.cap.aaWeight << ST25R3916_REG_CAPACITANCE_MEASURE_CONF_cm_aew_shift) & ST25R3916_REG_CAPACITANCE_MEASURE_CONF_cm_aew_mask);
            aux |=
                (uint8_t)(gRFAL.wum.cfg.cap.autoAvg ? ST25R3916_REG_CAPACITANCE_MEASURE_CONF_cm_ae : 0x00U);

            st25r3916WriteRegister(ST25R3916_REG_CAPACITANCE_MEASURE_CONF, aux);

            /* Only need to set the reference if not using Auto Average */
            if(!gRFAL.wum.cfg.cap.autoAvg || gRFAL.wum.cfg.swTagDetect) {
                if(gRFAL.wum.cfg.indPha.reference == RFAL_WUM_REFERENCE_AUTO) {
                    st25r3916MeasureCapacitance(&aux);
                    gRFAL.wum.cfg.cap.reference = aux;
                }
                st25r3916WriteRegister(
                    ST25R3916_REG_CAPACITANCE_MEASURE_REF, (uint8_t)gRFAL.wum.cfg.cap.reference);
            }

            reg |= ST25R3916_REG_WUP_TIMER_CONTROL_wcap;
            irqs |= ST25R3916_IRQ_MASK_WCAP;
        }
    }

    /* Disable and clear all interrupts except Wake-Up IRQs */
    st25r3916DisableInterrupts(ST25R3916_IRQ_MASK_ALL);
    st25r3916GetInterrupt(irqs);
    st25r3916EnableInterrupts(irqs);

    /* Enable Low Power Wake-Up Mode (Disable: Oscilattor, Tx, Rx and External Field Detector) */
    st25r3916WriteRegister(ST25R3916_REG_WUP_TIMER_CONTROL, reg);
    st25r3916ChangeRegisterBits(
        ST25R3916_REG_OP_CONTROL,
        (ST25R3916_REG_OP_CONTROL_en | ST25R3916_REG_OP_CONTROL_rx_en |
         ST25R3916_REG_OP_CONTROL_tx_en | ST25R3916_REG_OP_CONTROL_en_fd_mask |
         ST25R3916_REG_OP_CONTROL_wu),
        ST25R3916_REG_OP_CONTROL_wu);

    gRFAL.wum.state = RFAL_WUM_STATE_ENABLED;
    gRFAL.state = RFAL_STATE_WUM;

    return ERR_NONE;
}

/*******************************************************************************/
bool rfalWakeUpModeHasWoke(void) {
    return (gRFAL.wum.state >= RFAL_WUM_STATE_ENABLED_WOKE);
}

/*******************************************************************************/
static uint16_t rfalWakeUpModeFilter(uint16_t curRef, uint16_t curVal, uint8_t weight) {
    uint16_t newRef;

    /* Perform the averaging|filter as describded in ST25R3916 DS */

    /* Avoid signed arithmetics by splitting in two cases */
    if(curVal > curRef) {
        newRef = curRef + ((curVal - curRef) / weight);

        /* In order for the reference to converge to final value   *
         * increment once the diff is smaller that the weight      */
        if((curVal != curRef) && (curRef == newRef)) {
            newRef &= 0xFF00U;
            newRef += 0x0100U;
        }
    } else {
        newRef = curRef - ((curRef - curVal) / weight);

        /* In order for the reference to converge to final value   *
         * decrement once the diff is smaller that the weight      */
        if((curVal != curRef) && (curRef == newRef)) {
            newRef &= 0xFF00U;
        }
    }

    return newRef;
}

/*******************************************************************************/
static void rfalRunWakeUpModeWorker(void) {
    uint32_t irqs;
    uint8_t reg;
    uint16_t value;
    uint16_t delta;

    if(gRFAL.state != RFAL_STATE_WUM) {
        return;
    }

    switch(gRFAL.wum.state) {
    case RFAL_WUM_STATE_ENABLED:
    case RFAL_WUM_STATE_ENABLED_WOKE:

        irqs = st25r3916GetInterrupt(
            (ST25R3916_IRQ_MASK_WT | ST25R3916_IRQ_MASK_WAM | ST25R3916_IRQ_MASK_WPH |
             ST25R3916_IRQ_MASK_WCAP));
        if(irqs == ST25R3916_IRQ_MASK_NONE) {
            break; /* No interrupt to process */
        }

        /*******************************************************************************/
        /* Check and mark which measurement(s) cause interrupt */
        if((irqs & ST25R3916_IRQ_MASK_WAM) != 0U) {
            st25r3916ReadRegister(ST25R3916_REG_AMPLITUDE_MEASURE_RESULT, &reg);
            gRFAL.wum.state = RFAL_WUM_STATE_ENABLED_WOKE;
        }

        if((irqs & ST25R3916_IRQ_MASK_WPH) != 0U) {
            st25r3916ReadRegister(ST25R3916_REG_PHASE_MEASURE_RESULT, &reg);
            gRFAL.wum.state = RFAL_WUM_STATE_ENABLED_WOKE;
        }

        if((irqs & ST25R3916_IRQ_MASK_WCAP) != 0U) {
            st25r3916ReadRegister(ST25R3916_REG_CAPACITANCE_MEASURE_RESULT, &reg);
            gRFAL.wum.state = RFAL_WUM_STATE_ENABLED_WOKE;
        }

        if((irqs & ST25R3916_IRQ_MASK_WT) != 0U) {
            /*******************************************************************************/
            if(gRFAL.wum.cfg.swTagDetect) {
                /* Enable Ready mode and wait the settle time */
                st25r3916ChangeRegisterBits(
                    ST25R3916_REG_OP_CONTROL,
                    (ST25R3916_REG_OP_CONTROL_en | ST25R3916_REG_OP_CONTROL_wu),
                    ST25R3916_REG_OP_CONTROL_en);
                platformDelay(RFAL_ST25R3916_AAT_SETTLE);

                /*******************************************************************************/
                if(gRFAL.wum.cfg.indAmp.enabled) {
                    /* Perform amplitude measurement */
                    st25r3916MeasureAmplitude(&reg);

                    /* Convert inputs to TD format */
                    value = rfalConvTDFormat(reg);
                    delta = rfalConvTDFormat(gRFAL.wum.cfg.indAmp.delta);

                    /* Set first measurement as reference */
                    if(gRFAL.wum.cfg.indAmp.reference == 0U) {
                        gRFAL.wum.cfg.indAmp.reference = value;
                    }

                    /* Check if device should be woken */
                    if((value >= (gRFAL.wum.cfg.indAmp.reference + delta)) ||
                       (value <= (gRFAL.wum.cfg.indAmp.reference - delta))) {
                        gRFAL.wum.state = RFAL_WUM_STATE_ENABLED_WOKE;
                        break;
                    }

                    /* Update moving reference if enabled */
                    if(gRFAL.wum.cfg.indAmp.autoAvg) {
                        gRFAL.wum.cfg.indAmp.reference = rfalWakeUpModeFilter(
                            gRFAL.wum.cfg.indAmp.reference,
                            value,
                            (RFAL_WU_MIN_WEIGHT_VAL << (uint8_t)gRFAL.wum.cfg.indAmp.aaWeight));
                    }
                }

                /*******************************************************************************/
                if(gRFAL.wum.cfg.indPha.enabled) {
                    /* Perform Phase measurement */
                    st25r3916MeasurePhase(&reg);

                    /* Convert inputs to TD format */
                    value = rfalConvTDFormat(reg);
                    delta = rfalConvTDFormat(gRFAL.wum.cfg.indPha.delta);

                    /* Set first measurement as reference */
                    if(gRFAL.wum.cfg.indPha.reference == 0U) {
                        gRFAL.wum.cfg.indPha.reference = value;
                    }

                    /* Check if device should be woken */
                    if((value >= (gRFAL.wum.cfg.indPha.reference + delta)) ||
                       (value <= (gRFAL.wum.cfg.indPha.reference - delta))) {
                        gRFAL.wum.state = RFAL_WUM_STATE_ENABLED_WOKE;
                        break;
                    }

                    /* Update moving reference if enabled */
                    if(gRFAL.wum.cfg.indPha.autoAvg) {
                        gRFAL.wum.cfg.indPha.reference = rfalWakeUpModeFilter(
                            gRFAL.wum.cfg.indPha.reference,
                            value,
                            (RFAL_WU_MIN_WEIGHT_VAL << (uint8_t)gRFAL.wum.cfg.indPha.aaWeight));
                    }
                }

                /* Re-Enable low power Wake-Up mode for wto to trigger another measurement(s) */
                st25r3916ChangeRegisterBits(
                    ST25R3916_REG_OP_CONTROL,
                    (ST25R3916_REG_OP_CONTROL_en | ST25R3916_REG_OP_CONTROL_wu),
                    (ST25R3916_REG_OP_CONTROL_wu));
            }
        }
        break;

    default:
        /* MISRA 16.4: no empty default statement (a comment being enough) */
        break;
    }
}

/*******************************************************************************/
ReturnCode rfalWakeUpModeStop(void) {
    /* Check if RFAL is in Wake-up mode */
    if(gRFAL.state != RFAL_STATE_WUM) {
        return ERR_WRONG_STATE;
    }

    gRFAL.wum.state = RFAL_WUM_STATE_NOT_INIT;

    /* Disable Wake-Up Mode */
    st25r3916ClrRegisterBits(ST25R3916_REG_OP_CONTROL, ST25R3916_REG_OP_CONTROL_wu);
    st25r3916DisableInterrupts(
        (ST25R3916_IRQ_MASK_WT | ST25R3916_IRQ_MASK_WAM | ST25R3916_IRQ_MASK_WPH |
         ST25R3916_IRQ_MASK_WCAP));

    /* Re-Enable External Field Detector as: Automatics */
    st25r3916ChangeRegisterBits(
        ST25R3916_REG_OP_CONTROL,
        ST25R3916_REG_OP_CONTROL_en_fd_mask,
        ST25R3916_REG_OP_CONTROL_en_fd_auto_efd);

    /* Re-Enable the Oscillator */
    st25r3916OscOn();

    /* Set Analog configurations for Wake-up Off event */
    rfalSetAnalogConfig((RFAL_ANALOG_CONFIG_TECH_CHIP | RFAL_ANALOG_CONFIG_CHIP_WAKEUP_OFF));

    return ERR_NONE;
}

#endif /* RFAL_FEATURE_WAKEUP_MODE */

/*******************************************************************************
 *  Low-Power Mode                                                               *
 *******************************************************************************/

#if RFAL_FEATURE_LOWPOWER_MODE

/*******************************************************************************/
ReturnCode rfalLowPowerModeStart(void) {
    /* Check if RFAL is not initialized */
    if(gRFAL.state < RFAL_STATE_INIT) {
        return ERR_WRONG_STATE;
    }

    /* Stop any ongoing activity and set the device in low power by disabling oscillator, transmitter, receiver and external field detector */
    st25r3916ExecuteCommand(ST25R3916_CMD_STOP);
    st25r3916ClrRegisterBits(
        ST25R3916_REG_OP_CONTROL,
        (ST25R3916_REG_OP_CONTROL_en | ST25R3916_REG_OP_CONTROL_rx_en |
         ST25R3916_REG_OP_CONTROL_wu | ST25R3916_REG_OP_CONTROL_tx_en |
         ST25R3916_REG_OP_CONTROL_en_fd_mask));

    rfalSetAnalogConfig((RFAL_ANALOG_CONFIG_TECH_CHIP | RFAL_ANALOG_CONFIG_CHIP_LOWPOWER_ON));

    gRFAL.state = RFAL_STATE_IDLE;
    gRFAL.lpm.isRunning = true;

    platformDisableIrqCallback();

    return ERR_NONE;
}

/*******************************************************************************/
ReturnCode rfalLowPowerModeStop(void) {
    ReturnCode ret;

    platformEnableIrqCallback();

    /* Check if RFAL is on right state */
    if(!gRFAL.lpm.isRunning) {
        return ERR_WRONG_STATE;
    }

    /* Re-enable device */
    EXIT_ON_ERR(ret, st25r3916OscOn());
    st25r3916ChangeRegisterBits(
        ST25R3916_REG_OP_CONTROL,
        ST25R3916_REG_OP_CONTROL_en_fd_mask,
        ST25R3916_REG_OP_CONTROL_en_fd_auto_efd);

    rfalSetAnalogConfig((RFAL_ANALOG_CONFIG_TECH_CHIP | RFAL_ANALOG_CONFIG_CHIP_LOWPOWER_OFF));

    gRFAL.state = RFAL_STATE_INIT;
    return ERR_NONE;
}

#endif /* RFAL_FEATURE_LOWPOWER_MODE */

/*******************************************************************************
 *  RF Chip                                                                    *  
 *******************************************************************************/

/*******************************************************************************/
ReturnCode rfalChipWriteReg(uint16_t reg, const uint8_t* values, uint8_t len) {
    if(!st25r3916IsRegValid((uint8_t)reg)) {
        return ERR_PARAM;
    }

    return st25r3916WriteMultipleRegisters((uint8_t)reg, values, len);
}

/*******************************************************************************/
ReturnCode rfalChipReadReg(uint16_t reg, uint8_t* values, uint8_t len) {
    if(!st25r3916IsRegValid((uint8_t)reg)) {
        return ERR_PARAM;
    }

    return st25r3916ReadMultipleRegisters((uint8_t)reg, values, len);
}

/*******************************************************************************/
ReturnCode rfalChipExecCmd(uint16_t cmd) {
    if(!st25r3916IsCmdValid((uint8_t)cmd)) {
        return ERR_PARAM;
    }

    return st25r3916ExecuteCommand((uint8_t)cmd);
}

/*******************************************************************************/
ReturnCode rfalChipWriteTestReg(uint16_t reg, uint8_t value) {
    return st25r3916WriteTestRegister((uint8_t)reg, value);
}

/*******************************************************************************/
ReturnCode rfalChipReadTestReg(uint16_t reg, uint8_t* value) {
    return st25r3916ReadTestRegister((uint8_t)reg, value);
}

/*******************************************************************************/
ReturnCode rfalChipChangeRegBits(uint16_t reg, uint8_t valueMask, uint8_t value) {
    if(!st25r3916IsRegValid((uint8_t)reg)) {
        return ERR_PARAM;
    }

    return st25r3916ChangeRegisterBits((uint8_t)reg, valueMask, value);
}

/*******************************************************************************/
ReturnCode rfalChipChangeTestRegBits(uint16_t reg, uint8_t valueMask, uint8_t value) {
    st25r3916ChangeTestRegisterBits((uint8_t)reg, valueMask, value);
    return ERR_NONE;
}

/*******************************************************************************/
ReturnCode rfalChipSetRFO(uint8_t rfo) {
    return st25r3916ChangeRegisterBits(
        ST25R3916_REG_TX_DRIVER, ST25R3916_REG_TX_DRIVER_d_res_mask, rfo);
}

/*******************************************************************************/
ReturnCode rfalChipGetRFO(uint8_t* result) {
    ReturnCode ret;

    ret = st25r3916ReadRegister(ST25R3916_REG_TX_DRIVER, result);

    (*result) = ((*result) & ST25R3916_REG_TX_DRIVER_d_res_mask);

    return ret;
}

/*******************************************************************************/
ReturnCode rfalChipMeasureAmplitude(uint8_t* result) {
    ReturnCode err;
    uint8_t reg_opc, reg_mode, reg_conf1, reg_conf2;

    /* Save registers which will be adjusted below */
    st25r3916ReadRegister(ST25R3916_REG_OP_CONTROL, &reg_opc);
    st25r3916ReadRegister(ST25R3916_REG_MODE, &reg_mode);
    st25r3916ReadRegister(ST25R3916_REG_RX_CONF1, &reg_conf1);
    st25r3916ReadRegister(ST25R3916_REG_RX_CONF2, &reg_conf2);

    /* Set values as per defaults of DS. These regs/bits influence receiver chain and change amplitude */
    /* Doing so achieves an amplitude comparable over a complete polling cylce */
    st25r3916WriteRegister(ST25R3916_REG_OP_CONTROL, (reg_opc & ~ST25R3916_REG_OP_CONTROL_rx_chn));
    st25r3916WriteRegister(
        ST25R3916_REG_MODE,
        ST25R3916_REG_MODE_om_iso14443a | ST25R3916_REG_MODE_targ_init |
            ST25R3916_REG_MODE_tr_am_ook | ST25R3916_REG_MODE_nfc_ar_off);
    st25r3916WriteRegister(
        ST25R3916_REG_RX_CONF1, (reg_conf1 & ~ST25R3916_REG_RX_CONF1_ch_sel_AM));
    st25r3916WriteRegister(
        ST25R3916_REG_RX_CONF2,
        ((reg_conf2 & ~(ST25R3916_REG_RX_CONF2_demod_mode | ST25R3916_REG_RX_CONF2_amd_sel)) |
         ST25R3916_REG_RX_CONF2_amd_sel_peak));

    /* Perform the actual measurement */
    err = st25r3916MeasureAmplitude(result);

    /* Restore values */
    st25r3916WriteRegister(ST25R3916_REG_OP_CONTROL, reg_opc);
    st25r3916WriteRegister(ST25R3916_REG_MODE, reg_mode);
    st25r3916WriteRegister(ST25R3916_REG_RX_CONF1, reg_conf1);
    st25r3916WriteRegister(ST25R3916_REG_RX_CONF2, reg_conf2);

    return err;
}

/*******************************************************************************/
ReturnCode rfalChipMeasurePhase(uint8_t* result) {
    st25r3916MeasurePhase(result);

    return ERR_NONE;
}

/*******************************************************************************/
ReturnCode rfalChipMeasureCapacitance(uint8_t* result) {
    st25r3916MeasureCapacitance(result);

    return ERR_NONE;
}

/*******************************************************************************/
ReturnCode rfalChipMeasurePowerSupply(uint8_t param, uint8_t* result) {
    *result = st25r3916MeasurePowerSupply(param);

    return ERR_NONE;
}

/*******************************************************************************/
extern uint8_t invalid_size_of_stream_configs
    [(sizeof(struct st25r3916StreamConfig) == sizeof(struct iso15693StreamConfig)) ? 1 : (-1)];
