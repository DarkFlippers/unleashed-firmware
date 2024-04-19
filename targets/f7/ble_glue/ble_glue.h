#pragma once

#include <stdint.h>
#include <stdbool.h>

#ifdef __cplusplus
extern "C" {
#endif

/* 
 * Low-level interface to Core2 - startup, shutdown, mode switching, FUS commands.
 */

typedef enum {
    BleGlueC2ModeUnknown = 0,
    BleGlueC2ModeFUS,
    BleGlueC2ModeStack,
} BleGlueC2Mode;

#define BLE_MAX_VERSION_STRING_LEN (20)
typedef struct {
    BleGlueC2Mode mode;
    /**
     * Wireless Info
     */
    uint8_t VersionMajor;
    uint8_t VersionMinor;
    uint8_t VersionSub;
    uint8_t VersionBranch;
    uint8_t VersionReleaseType;
    uint8_t MemorySizeSram2B; /*< Multiple of 1K */
    uint8_t MemorySizeSram2A; /*< Multiple of 1K */
    uint8_t MemorySizeSram1; /*< Multiple of 1K */
    uint8_t MemorySizeFlash; /*< Multiple of 4K */
    uint8_t StackType;
    char StackTypeString[BLE_MAX_VERSION_STRING_LEN];
    /**
     * Fus Info
     */
    uint8_t FusVersionMajor;
    uint8_t FusVersionMinor;
    uint8_t FusVersionSub;
    uint8_t FusMemorySizeSram2B; /*< Multiple of 1K */
    uint8_t FusMemorySizeSram2A; /*< Multiple of 1K */
    uint8_t FusMemorySizeFlash; /*< Multiple of 4K */
} BleGlueC2Info;

typedef enum {
    // Stage 1: core2 startup and FUS
    BleGlueStatusStartup,
    BleGlueStatusBroken,
    BleGlueStatusC2Started,
    // Stage 2: radio stack
    BleGlueStatusRadioStackRunning,
    BleGlueStatusRadioStackMissing
} BleGlueStatus;

typedef void (
    *BleGlueKeyStorageChangedCallback)(uint8_t* change_addr_start, uint16_t size, void* context);

/** Initialize start core2 and initialize transport */
void ble_glue_init(void);

/** Start Core2 Radio stack
 *
 * @return     true on success
 */
bool ble_glue_start(void);

void ble_glue_stop(void);

/** Is core2 alive and at least FUS is running
 * 
 * @return     true if core2 is alive
 */
bool ble_glue_is_alive(void);

/** Waits for C2 to reports its mode to callback
 *
 * @return     true if it reported before reaching timeout
 */
bool ble_glue_wait_for_c2_start(int32_t timeout_ms);

BleGlueStatus ble_glue_get_c2_status(void);

const BleGlueC2Info* ble_glue_get_c2_info(void);

/** Is core2 radio stack present and ready
 *
 * @return     true if present and ready
 */
bool ble_glue_is_radio_stack_ready(void);

/** Set callback for NVM in RAM changes
 *
 * @param[in]  callback  The callback to call on NVM change
 * @param      context   The context for callback
 */
void ble_glue_set_key_storage_changed_callback(
    BleGlueKeyStorageChangedCallback callback,
    void* context);

bool ble_glue_reinit_c2(void);

typedef enum {
    BleGlueCommandResultUnknown,
    BleGlueCommandResultOK,
    BleGlueCommandResultError,
    BleGlueCommandResultRestartPending,
    BleGlueCommandResultOperationOngoing,
} BleGlueCommandResult;

/** Restart MCU to launch radio stack firmware if necessary
 *
 * @return      true on radio stack start command
 */
BleGlueCommandResult ble_glue_force_c2_mode(BleGlueC2Mode mode);

BleGlueCommandResult ble_glue_fus_stack_delete(void);

BleGlueCommandResult ble_glue_fus_stack_install(uint32_t src_addr, uint32_t dst_addr);

BleGlueCommandResult ble_glue_fus_get_status(void);

BleGlueCommandResult ble_glue_fus_wait_operation(void);

typedef struct {
    uint32_t magic;
    uint32_t source_pc;
    uint32_t source_lr;
    uint32_t source_sp;
} BleGlueHardfaultInfo;

/** Get hardfault info
 *
 * @return     hardfault info. NULL if no hardfault
 */
const BleGlueHardfaultInfo* ble_glue_get_hardfault_info(void);

#ifdef __cplusplus
}
#endif
