#pragma once

#include "app_conf.h" /* required as some configuration used in dbg_trace.h are set there */
#include "dbg_trace.h"

#ifdef __cplusplus
extern "C" {
#endif

/**
 * Enable or Disable traces
 * The raw data output is the hci binary packet format as specified by the BT specification *
 */
#define TL_SHCI_CMD_DBG_EN 1 /* Reports System commands sent to CPU2 and the command response */
#define TL_SHCI_CMD_DBG_RAW_EN \
    0 /* Reports raw data System commands sent to CPU2 and the command response */
#define TL_SHCI_EVT_DBG_EN 1 /* Reports System Asynchronous Events received from CPU2 */
#define TL_SHCI_EVT_DBG_RAW_EN \
    0 /* Reports raw data System Asynchronous Events received from CPU2 */

#define TL_HCI_CMD_DBG_EN 1 /* Reports BLE command sent to CPU2 and the command response */
#define TL_HCI_CMD_DBG_RAW_EN \
    0 /* Reports raw data BLE command sent to CPU2 and the command response */
#define TL_HCI_EVT_DBG_EN     1 /* Reports BLE Asynchronous Events received from CPU2 */
#define TL_HCI_EVT_DBG_RAW_EN 0 /* Reports raw data BLE Asynchronous Events received from CPU2 */

#define TL_MM_DBG_EN 1 /* Reports the informations of the buffer released to CPU2 */

/**
 * System Transport Layer
 */
#if(TL_SHCI_CMD_DBG_EN != 0)
#define TL_SHCI_CMD_DBG_MSG PRINT_MESG_DBG
#define TL_SHCI_CMD_DBG_BUF PRINT_LOG_BUFF_DBG
#else
#define TL_SHCI_CMD_DBG_MSG(...)
#define TL_SHCI_CMD_DBG_BUF(...)
#endif

#if(TL_SHCI_CMD_DBG_RAW_EN != 0)
#define TL_SHCI_CMD_DBG_RAW(_PDATA_, _SIZE_) furi_log_tx(_PDATA_, _SIZE_)
#else
#define TL_SHCI_CMD_DBG_RAW(...)
#endif

#if(TL_SHCI_EVT_DBG_EN != 0)
#define TL_SHCI_EVT_DBG_MSG PRINT_MESG_DBG
#define TL_SHCI_EVT_DBG_BUF PRINT_LOG_BUFF_DBG
#else
#define TL_SHCI_EVT_DBG_MSG(...)
#define TL_SHCI_EVT_DBG_BUF(...)
#endif

#if(TL_SHCI_EVT_DBG_RAW_EN != 0)
#define TL_SHCI_EVT_DBG_RAW(_PDATA_, _SIZE_) furi_log_tx(_PDATA_, _SIZE_)
#else
#define TL_SHCI_EVT_DBG_RAW(...)
#endif

/**
 * BLE Transport Layer
 */
#if(TL_HCI_CMD_DBG_EN != 0)
#define TL_HCI_CMD_DBG_MSG PRINT_MESG_DBG
#define TL_HCI_CMD_DBG_BUF PRINT_LOG_BUFF_DBG
#else
#define TL_HCI_CMD_DBG_MSG(...)
#define TL_HCI_CMD_DBG_BUF(...)
#endif

#if(TL_HCI_CMD_DBG_RAW_EN != 0)
#define TL_HCI_CMD_DBG_RAW(_PDATA_, _SIZE_) furi_log_tx(_PDATA_, _SIZE_)
#else
#define TL_HCI_CMD_DBG_RAW(...)
#endif

#if(TL_HCI_EVT_DBG_EN != 0)
#define TL_HCI_EVT_DBG_MSG PRINT_MESG_DBG
#define TL_HCI_EVT_DBG_BUF PRINT_LOG_BUFF_DBG
#else
#define TL_HCI_EVT_DBG_MSG(...)
#define TL_HCI_EVT_DBG_BUF(...)
#endif

#if(TL_HCI_EVT_DBG_RAW_EN != 0)
#define TL_HCI_EVT_DBG_RAW(_PDATA_, _SIZE_) furi_log_tx(_PDATA_, _SIZE_)
#else
#define TL_HCI_EVT_DBG_RAW(...)
#endif

/**
 * Memory Manager - Released buffer tracing
 */
#if(TL_MM_DBG_EN != 0)
#define TL_MM_DBG_MSG PRINT_MESG_DBG
#else
#define TL_MM_DBG_MSG(...)
#endif

#ifdef __cplusplus
}
#endif
