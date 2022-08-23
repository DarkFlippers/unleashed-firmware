#pragma once

#include "hw_conf.h"
#include "hw_if.h"

#include <interface/patterns/ble_thread/hw.h>
#include <ble/core/ble_bufsize.h>

#define CFG_TX_POWER (0x19) /* +0dBm */

/**
 * Define Advertising parameters
 */
#define CFG_ADV_BD_ADDRESS (0x7257acd87a6c)
#define CFG_FAST_CONN_ADV_INTERVAL_MIN (0x80) /**< 80ms */
#define CFG_FAST_CONN_ADV_INTERVAL_MAX (0xa0) /**< 100ms */
#define CFG_LP_CONN_ADV_INTERVAL_MIN (0x640) /**< 1s */
#define CFG_LP_CONN_ADV_INTERVAL_MAX (0xfa0) /**< 2.5s */

/**
 * Define IO Authentication
 */
#define CFG_BONDING_MODE (1)
#define CFG_FIXED_PIN (111111)
#define CFG_USED_FIXED_PIN (1)
#define CFG_ENCRYPTION_KEY_SIZE_MAX (16)
#define CFG_ENCRYPTION_KEY_SIZE_MIN (8)

/**
 * Define IO capabilities
 */
#define CFG_IO_CAPABILITY_DISPLAY_ONLY (0x00)
#define CFG_IO_CAPABILITY_DISPLAY_YES_NO (0x01)
#define CFG_IO_CAPABILITY_KEYBOARD_ONLY (0x02)
#define CFG_IO_CAPABILITY_NO_INPUT_NO_OUTPUT (0x03)
#define CFG_IO_CAPABILITY_KEYBOARD_DISPLAY (0x04)

#define CFG_IO_CAPABILITY CFG_IO_CAPABILITY_DISPLAY_YES_NO

/**
 * Define MITM modes
 */
#define CFG_MITM_PROTECTION_NOT_REQUIRED (0x00)
#define CFG_MITM_PROTECTION_REQUIRED (0x01)

#define CFG_MITM_PROTECTION CFG_MITM_PROTECTION_REQUIRED

/**
 * Define Secure Connections Support
 */
#define CFG_SECURE_NOT_SUPPORTED (0x00)
#define CFG_SECURE_OPTIONAL (0x01)
#define CFG_SECURE_MANDATORY (0x02)

#define CFG_SC_SUPPORT CFG_SECURE_OPTIONAL

/**
 * Define Keypress Notification Support
 */
#define CFG_KEYPRESS_NOT_SUPPORTED (0x00)
#define CFG_KEYPRESS_SUPPORTED (0x01)

#define CFG_KEYPRESS_NOTIFICATION_SUPPORT CFG_KEYPRESS_NOT_SUPPORTED

/**
 * Numeric Comparison Answers
 */
#define YES (0x01)
#define NO (0x00)

/**
 * Device name configuration for Generic Access Service
 */
#define CFG_GAP_DEVICE_NAME "TEMPLATE"
#define CFG_GAP_DEVICE_NAME_LENGTH (8)

/**
 * Define PHY
 */
#define ALL_PHYS_PREFERENCE 0x00
#define RX_2M_PREFERRED 0x02
#define TX_2M_PREFERRED 0x02
#define TX_1M 0x01
#define TX_2M 0x02
#define RX_1M 0x01
#define RX_2M 0x02

/**
*   Identity root key used to derive LTK and CSRK
*/
#define CFG_BLE_IRK                                                                               \
    {                                                                                             \
        0x12, 0x34, 0x56, 0x78, 0x9a, 0xbc, 0xde, 0xf0, 0x12, 0x34, 0x56, 0x78, 0x9a, 0xbc, 0xde, \
            0xf0                                                                                  \
    }

/**
* Encryption root key used to derive LTK and CSRK
*/
#define CFG_BLE_ERK                                                                               \
    {                                                                                             \
        0xfe, 0xdc, 0xba, 0x09, 0x87, 0x65, 0x43, 0x21, 0xfe, 0xdc, 0xba, 0x09, 0x87, 0x65, 0x43, \
            0x21                                                                                  \
    }

/* USER CODE BEGIN Generic_Parameters */
/**
 * SMPS supply
 * SMPS not used when Set to 0
 * SMPS used when Set to 1
 */
#define CFG_USE_SMPS 1
/* USER CODE END Generic_Parameters */

/**< specific parameters */
/*****************************************************/

/**
* AD Element - Group B Feature
*/
/* LSB - Second Byte */
#define CFG_FEATURE_OTA_REBOOT (0x20)

/******************************************************************************
 * BLE Stack
 ******************************************************************************/
/**
 * Maximum number of simultaneous connections that the device will support.
 * Valid values are from 1 to 8
 */
#define CFG_BLE_NUM_LINK 1

/**
 * Maximum number of Services that can be stored in the GATT database.
 * Note that the GAP and GATT services are automatically added so this parameter should be 2 plus the number of user services
 */
#define CFG_BLE_NUM_GATT_SERVICES 8

/**
 * Maximum number of Attributes
 * (i.e. the number of characteristic + the number of characteristic values + the number of descriptors, excluding the services)
 * that can be stored in the GATT database.
 * Note that certain characteristics and relative descriptors are added automatically during device initialization
 * so this parameters should be 9 plus the number of user Attributes
 */
#define CFG_BLE_NUM_GATT_ATTRIBUTES 68

/**
 * Maximum supported ATT_MTU size
 */
#define CFG_BLE_MAX_ATT_MTU (256 + 128 + 16 + 8 + 4 + 2)

/**
 * Size of the storage area for Attribute values
 *  This value depends on the number of attributes used by application. In particular the sum of the following quantities (in octets) should be made for each attribute:
 *  - attribute value length
 *  - 5, if UUID is 16 bit; 19, if UUID is 128 bit
 *  - 2, if server configuration descriptor is used
 *  - 2*DTM_NUM_LINK, if client configuration descriptor is used
 *  - 2, if extended properties is used
 *  The total amount of memory needed is the sum of the above quantities for each attribute.
 */
#define CFG_BLE_ATT_VALUE_ARRAY_SIZE (1344)

/**
 * Prepare Write List size in terms of number of packet
 */
#define CFG_BLE_PREPARE_WRITE_LIST_SIZE BLE_PREP_WRITE_X_ATT(CFG_BLE_MAX_ATT_MTU)

/**
 * Number of allocated memory blocks
 */
#define CFG_BLE_MBLOCK_COUNT \
    (BLE_MBLOCKS_CALC(CFG_BLE_PREPARE_WRITE_LIST_SIZE, CFG_BLE_MAX_ATT_MTU, CFG_BLE_NUM_LINK))

/**
 * Enable or disable the Extended Packet length feature. Valid values are 0 or 1.
 */
#define CFG_BLE_DATA_LENGTH_EXTENSION 1

/**
 * Sleep clock accuracy in Slave mode (ppm value)
 */
#define CFG_BLE_SLAVE_SCA 500

/**
 * Sleep clock accuracy in Master mode
 * 0 : 251 ppm to 500 ppm
 * 1 : 151 ppm to 250 ppm
 * 2 : 101 ppm to 150 ppm
 * 3 : 76 ppm to 100 ppm
 * 4 : 51 ppm to 75 ppm
 * 5 : 31 ppm to 50 ppm
 * 6 : 21 ppm to 30 ppm
 * 7 : 0 ppm to 20 ppm
 */
#define CFG_BLE_MASTER_SCA 0

/**
 *  Source for the low speed clock for RF wake-up
 *  1 : external high speed crystal HSE/32/32
 *  0 : external low speed crystal ( no calibration )
 */
#define CFG_BLE_LSE_SOURCE 0

/**
 * Start up time of the high speed (16 or 32 MHz) crystal oscillator in units of 625/256 us (~2.44 us)
 */
#define CFG_BLE_HSE_STARTUP_TIME 0x148

/**
 * Maximum duration of the connection event when the device is in Slave mode in units of 625/256 us (~2.44 us)
 */
#define CFG_BLE_MAX_CONN_EVENT_LENGTH (0xFFFFFFFF)

/**
 * Viterbi Mode
 * 1 : enabled
 * 0 : disabled
 */
#define CFG_BLE_VITERBI_MODE 1

/**
 * BLE stack Options flags to be configured with:
 * - SHCI_C2_BLE_INIT_OPTIONS_LL_ONLY
 * - SHCI_C2_BLE_INIT_OPTIONS_LL_HOST
 * - SHCI_C2_BLE_INIT_OPTIONS_NO_SVC_CHANGE_DESC
 * - SHCI_C2_BLE_INIT_OPTIONS_WITH_SVC_CHANGE_DESC
 * - SHCI_C2_BLE_INIT_OPTIONS_DEVICE_NAME_RO
 * - SHCI_C2_BLE_INIT_OPTIONS_DEVICE_NAME_RW
 * - SHCI_C2_BLE_INIT_OPTIONS_EXT_ADV
 * - SHCI_C2_BLE_INIT_OPTIONS_NO_EXT_ADV
 * - SHCI_C2_BLE_INIT_OPTIONS_CS_ALGO2
 * - SHCI_C2_BLE_INIT_OPTIONS_NO_CS_ALGO2
 * - SHCI_C2_BLE_INIT_OPTIONS_POWER_CLASS_1
 * - SHCI_C2_BLE_INIT_OPTIONS_POWER_CLASS_2_3
 * which are used to set following configuration bits:
 * (bit 0): 1: LL only
 *          0: LL + host
 * (bit 1): 1: no service change desc.
 *          0: with service change desc.
 * (bit 2): 1: device name Read-Only
 *          0: device name R/W
 * (bit 3): 1: extended advertizing supported       [NOT SUPPORTED]
 *          0: extended advertizing not supported   [NOT SUPPORTED]
 * (bit 4): 1: CS Algo #2 supported
 *          0: CS Algo #2 not supported
 * (bit 7): 1: LE Power Class 1
 *          0: LE Power Class 2-3
 * other bits: reserved (shall be set to 0)
 */
#define CFG_BLE_OPTIONS                                                                 \
    (SHCI_C2_BLE_INIT_OPTIONS_LL_HOST | SHCI_C2_BLE_INIT_OPTIONS_WITH_SVC_CHANGE_DESC | \
     SHCI_C2_BLE_INIT_OPTIONS_DEVICE_NAME_RW | SHCI_C2_BLE_INIT_OPTIONS_NO_EXT_ADV |    \
     SHCI_C2_BLE_INIT_OPTIONS_NO_CS_ALGO2 | SHCI_C2_BLE_INIT_OPTIONS_POWER_CLASS_2_3)

/**
 * Queue length of BLE Event
 * This parameter defines the number of asynchronous events that can be stored in the HCI layer before
 * being reported to the application. When a command is sent to the BLE core coprocessor, the HCI layer
 * is waiting for the event with the Num_HCI_Command_Packets set to 1. The receive queue shall be large
 * enough to store all asynchronous events received in between.
 * When CFG_TLBLE_MOST_EVENT_PAYLOAD_SIZE is set to 27, this allow to store three 255 bytes long asynchronous events
 * between the HCI command and its event.
 * This parameter depends on the value given to CFG_TLBLE_MOST_EVENT_PAYLOAD_SIZE. When the queue size is to small,
 * the system may hang if the queue is full with asynchronous events and the HCI layer is still waiting
 * for a CC/CS event, In that case, the notification TL_BLE_HCI_ToNot() is called to indicate
 * to the application a HCI command did not receive its command event within 30s (Default HCI Timeout).
 */
#define CFG_TLBLE_EVT_QUEUE_LENGTH 5
/**
 * This parameter should be set to fit most events received by the HCI layer. It defines the buffer size of each element
 * allocated in the queue of received events and can be used to optimize the amount of RAM allocated by the Memory Manager.
 * It should not exceed 255 which is the maximum HCI packet payload size (a greater value is a lost of memory as it will
 * never be used)
 * With the current wireless firmware implementation, this parameter shall be kept to 255
 *
 */
#define CFG_TLBLE_MOST_EVENT_PAYLOAD_SIZE \
    255 /**< Set to 255 with the memory manager and the mailbox */

#define TL_BLE_EVENT_FRAME_SIZE (TL_EVT_HDR_SIZE + CFG_TLBLE_MOST_EVENT_PAYLOAD_SIZE)
/******************************************************************************
 * UART interfaces
 ******************************************************************************/

/**
 * Select UART interfaces
 */
#define CFG_DEBUG_TRACE_UART hw_uart1
#define CFG_CONSOLE_MENU 0

/******************************************************************************
 * Low Power
 ******************************************************************************/
/**
 *  When set to 1, the low power mode is enable
 *  When set to 0, the device stays in RUN mode
 */
#define CFG_LPM_SUPPORTED 1

/******************************************************************************
 * Timer Server
 ******************************************************************************/
/**
 *  CFG_RTC_WUCKSEL_DIVIDER:  This sets the RTCCLK divider to the wakeup timer.
 *  The lower is the value, the better is the power consumption and the accuracy of the timerserver
 *  The higher is the value, the finest is the granularity
 *
 *  CFG_RTC_ASYNCH_PRESCALER: This sets the asynchronous prescaler of the RTC. It should as high as possible ( to ouput
 *  clock as low as possible) but the output clock should be equal or higher frequency compare to the clock feeding
 *  the wakeup timer. A lower clock speed would impact the accuracy of the timer server.
 *
 *  CFG_RTC_SYNCH_PRESCALER: This sets the synchronous prescaler of the RTC.
 *  When the 1Hz calendar clock is required, it shall be sets according to other settings
 *  When the 1Hz calendar clock is not needed, CFG_RTC_SYNCH_PRESCALER should be set to 0x7FFF (MAX VALUE)
 *
 *  CFG_RTCCLK_DIVIDER_CONF:
 *  Shall be set to either 0,2,4,8,16
 *  When set to either 2,4,8,16, the 1Hhz calendar is supported
 *  When set to 0, the user sets its own configuration
 *
 *  The following settings are computed with LSI as input to the RTC
 */
#define CFG_RTCCLK_DIVIDER_CONF 0

#if(CFG_RTCCLK_DIVIDER_CONF == 0)
/**
 * Custom configuration
 * It does not support 1Hz calendar
 * It divides the RTC CLK by 16
 */
#define CFG_RTCCLK_DIV (16)
#define CFG_RTC_WUCKSEL_DIVIDER (0)
#define CFG_RTC_ASYNCH_PRESCALER (CFG_RTCCLK_DIV - 1)
#define CFG_RTC_SYNCH_PRESCALER (0x7FFF)

#else

#if(CFG_RTCCLK_DIVIDER_CONF == 2)
/**
 * It divides the RTC CLK by 2
 */
#define CFG_RTC_WUCKSEL_DIVIDER (3)
#endif

#if(CFG_RTCCLK_DIVIDER_CONF == 4)
/**
 * It divides the RTC CLK by 4
 */
#define CFG_RTC_WUCKSEL_DIVIDER (2)
#endif

#if(CFG_RTCCLK_DIVIDER_CONF == 8)
/**
 * It divides the RTC CLK by 8
 */
#define CFG_RTC_WUCKSEL_DIVIDER (1)
#endif

#if(CFG_RTCCLK_DIVIDER_CONF == 16)
/**
 * It divides the RTC CLK by 16
 */
#define CFG_RTC_WUCKSEL_DIVIDER (0)
#endif

#define CFG_RTCCLK_DIV CFG_RTCCLK_DIVIDER_CONF
#define CFG_RTC_ASYNCH_PRESCALER (CFG_RTCCLK_DIV - 1)
#define CFG_RTC_SYNCH_PRESCALER (DIVR(LSE_VALUE, (CFG_RTC_ASYNCH_PRESCALER + 1)) - 1)

#endif

/** tick timer value in us */
#define CFG_TS_TICK_VAL DIVR((CFG_RTCCLK_DIV * 1000000), LSE_VALUE)

typedef enum {
    CFG_TIM_PROC_ID_ISR,
    /* USER CODE BEGIN CFG_TimProcID_t */

    /* USER CODE END CFG_TimProcID_t */
} CFG_TimProcID_t;

/******************************************************************************
 * Debug
 ******************************************************************************/
/**
 * When set, this resets some hw resources to set the device in the same state than the power up
 * The FW resets only register that may prevent the FW to run properly
 *
 * This shall be set to 0 in a final product
 *
 */
#define CFG_HW_RESET_BY_FW 0

/**
 * keep debugger enabled while in any low power mode when set to 1
 * should be set to 0 in production
 */
#define CFG_DEBUGGER_SUPPORTED 1

/**
 * When set to 1, the traces are enabled in the BLE services
 */
#define CFG_DEBUG_BLE_TRACE 0

/**
 * Enable or Disable traces in application
 */
#define CFG_DEBUG_APP_TRACE 0

#if(CFG_DEBUG_APP_TRACE != 0)
#define APP_DBG_MSG PRINT_MESG_DBG
#else
#define APP_DBG_MSG PRINT_NO_MESG
#endif

#if((CFG_DEBUG_BLE_TRACE != 0) || (CFG_DEBUG_APP_TRACE != 0))
#define CFG_DEBUG_TRACE 1
#endif

#if(CFG_DEBUG_TRACE != 0)
#undef CFG_LPM_SUPPORTED
#undef CFG_DEBUGGER_SUPPORTED
#define CFG_LPM_SUPPORTED 0
#define CFG_DEBUGGER_SUPPORTED 1
#endif

/**
 * When CFG_DEBUG_TRACE_FULL is set to 1, the trace are output with the API name, the file name and the line number
 * When CFG_DEBUG_TRACE_LIGHT is set to 1, only the debug message is output
 *
 * When both are set to 0, no trace are output
 * When both are set to 1,  CFG_DEBUG_TRACE_FULL is selected
 */
#define CFG_DEBUG_TRACE_LIGHT 0
#define CFG_DEBUG_TRACE_FULL 0

#if((CFG_DEBUG_TRACE != 0) && (CFG_DEBUG_TRACE_LIGHT == 0) && (CFG_DEBUG_TRACE_FULL == 0))
#undef CFG_DEBUG_TRACE_FULL
#undef CFG_DEBUG_TRACE_LIGHT
#define CFG_DEBUG_TRACE_FULL 0
#define CFG_DEBUG_TRACE_LIGHT 1
#endif

#if(CFG_DEBUG_TRACE == 0)
#undef CFG_DEBUG_TRACE_FULL
#undef CFG_DEBUG_TRACE_LIGHT
#define CFG_DEBUG_TRACE_FULL 0
#define CFG_DEBUG_TRACE_LIGHT 0
#endif

/**
 * When not set, the traces is looping on sending the trace over UART
 */
#define DBG_TRACE_USE_CIRCULAR_QUEUE 0

/**
 * max buffer Size to queue data traces and max data trace allowed.
 * Only Used if DBG_TRACE_USE_CIRCULAR_QUEUE is defined
 */
#define DBG_TRACE_MSG_QUEUE_SIZE 4096
#define MAX_DBG_TRACE_MSG_SIZE 1024

#define CFG_OTP_BASE_ADDRESS OTP_AREA_BASE
#define CFG_OTP_END_ADRESS OTP_AREA_END_ADDR
