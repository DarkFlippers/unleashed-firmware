#include "ble_glue.h"
#include "app_common.h"
#include "ble_app.h"
#include <ble/ble.h>
#include <hci_tl.h>

#include <interface/patterns/ble_thread/tl/tl.h>
#include <interface/patterns/ble_thread/shci/shci.h>
#include <interface/patterns/ble_thread/tl/shci_tl.h>
#include "app_debug.h"

#include <furi_hal.h>

#define TAG "Core2"

#define BLE_GLUE_FLAG_SHCI_EVENT (1UL << 0)
#define BLE_GLUE_FLAG_KILL_THREAD (1UL << 1)
#define BLE_GLUE_FLAG_ALL (BLE_GLUE_FLAG_SHCI_EVENT | BLE_GLUE_FLAG_KILL_THREAD)

#define POOL_SIZE                      \
    (CFG_TLBLE_EVT_QUEUE_LENGTH * 4U * \
     DIVC((sizeof(TL_PacketHeader_t) + TL_BLE_EVENT_FRAME_SIZE), 4U))

PLACE_IN_SECTION("MB_MEM2") ALIGN(4) static uint8_t ble_glue_event_pool[POOL_SIZE];
PLACE_IN_SECTION("MB_MEM2") ALIGN(4) static TL_CmdPacket_t ble_glue_system_cmd_buff;
PLACE_IN_SECTION("MB_MEM2")
ALIGN(4)
static uint8_t ble_glue_system_spare_event_buff[sizeof(TL_PacketHeader_t) + TL_EVT_HDR_SIZE + 255U];
PLACE_IN_SECTION("MB_MEM2")
ALIGN(4)
static uint8_t ble_glue_ble_spare_event_buff[sizeof(TL_PacketHeader_t) + TL_EVT_HDR_SIZE + 255];

typedef struct {
    FuriMutex* shci_mtx;
    FuriSemaphore* shci_sem;
    FuriThread* thread;
    BleGlueStatus status;
    BleGlueKeyStorageChangedCallback callback;
    BleGlueC2Info c2_info;
    void* context;
} BleGlue;

static BleGlue* ble_glue = NULL;

static int32_t ble_glue_shci_thread(void* argument);
static void ble_glue_sys_status_not_callback(SHCI_TL_CmdStatus_t status);
static void ble_glue_sys_user_event_callback(void* pPayload);

void ble_glue_set_key_storage_changed_callback(
    BleGlueKeyStorageChangedCallback callback,
    void* context) {
    furi_assert(ble_glue);
    furi_assert(callback);
    ble_glue->callback = callback;
    ble_glue->context = context;
}

///////////////////////////////////////////////////////////////////////////////

/* TL hook to catch hardfaults */

int32_t ble_glue_TL_SYS_SendCmd(uint8_t* buffer, uint16_t size) {
    if(furi_hal_bt_get_hardfault_info()) {
        furi_crash("ST(R) Copro(R) HardFault");
    }

    return TL_SYS_SendCmd(buffer, size);
}

void shci_register_io_bus(tSHciIO* fops) {
    /* Register IO bus services */
    fops->Init = TL_SYS_Init;
    fops->Send = ble_glue_TL_SYS_SendCmd;
}

static int32_t ble_glue_TL_BLE_SendCmd(uint8_t* buffer, uint16_t size) {
    if(furi_hal_bt_get_hardfault_info()) {
        furi_crash("ST(R) Copro(R) HardFault");
    }

    return TL_BLE_SendCmd(buffer, size);
}

void hci_register_io_bus(tHciIO* fops) {
    /* Register IO bus services */
    fops->Init = TL_BLE_Init;
    fops->Send = ble_glue_TL_BLE_SendCmd;
}

///////////////////////////////////////////////////////////////////////////////

void ble_glue_init() {
    ble_glue = malloc(sizeof(BleGlue));
    ble_glue->status = BleGlueStatusStartup;

#ifdef BLE_GLUE_DEBUG
    APPD_Init();
#endif

    // Initialize all transport layers
    TL_MM_Config_t tl_mm_config;
    SHCI_TL_HciInitConf_t SHci_Tl_Init_Conf;
    // Reference table initialization
    TL_Init();

    ble_glue->shci_mtx = furi_mutex_alloc(FuriMutexTypeNormal);
    ble_glue->shci_sem = furi_semaphore_alloc(1, 0);

    // FreeRTOS system task creation
    ble_glue->thread = furi_thread_alloc_ex("BleShciDriver", 1024, ble_glue_shci_thread, ble_glue);
    furi_thread_start(ble_glue->thread);

    // System channel initialization
    SHci_Tl_Init_Conf.p_cmdbuffer = (uint8_t*)&ble_glue_system_cmd_buff;
    SHci_Tl_Init_Conf.StatusNotCallBack = ble_glue_sys_status_not_callback;
    shci_init(ble_glue_sys_user_event_callback, (void*)&SHci_Tl_Init_Conf);

    /**< Memory Manager channel initialization */
    tl_mm_config.p_BleSpareEvtBuffer = ble_glue_ble_spare_event_buff;
    tl_mm_config.p_SystemSpareEvtBuffer = ble_glue_system_spare_event_buff;
    tl_mm_config.p_AsynchEvtPool = ble_glue_event_pool;
    tl_mm_config.AsynchEvtPoolSize = POOL_SIZE;
    TL_MM_Init(&tl_mm_config);
    TL_Enable();

    /*
     * From now, the application is waiting for the ready event ( VS_HCI_C2_Ready )
     * received on the system channel before starting the Stack
     * This system event is received with ble_glue_sys_user_event_callback()
     */
}

const BleGlueC2Info* ble_glue_get_c2_info() {
    return &ble_glue->c2_info;
}

BleGlueStatus ble_glue_get_c2_status() {
    return ble_glue->status;
}

static const char* ble_glue_get_reltype_str(const uint8_t reltype) {
    static char relcode[3] = {0};
    switch(reltype) {
    case INFO_STACK_TYPE_BLE_FULL:
        return "F";
    case INFO_STACK_TYPE_BLE_HCI:
        return "H";
    case INFO_STACK_TYPE_BLE_LIGHT:
        return "L";
    case INFO_STACK_TYPE_BLE_BEACON:
        return "Be";
    case INFO_STACK_TYPE_BLE_BASIC:
        return "Ba";
    case INFO_STACK_TYPE_BLE_FULL_EXT_ADV:
        return "F+";
    case INFO_STACK_TYPE_BLE_HCI_EXT_ADV:
        return "H+";
    default:
        snprintf(relcode, sizeof(relcode), "%X", reltype);
        return relcode;
    }
}

static void ble_glue_update_c2_fw_info() {
    WirelessFwInfo_t wireless_info;
    SHCI_GetWirelessFwInfo(&wireless_info);
    BleGlueC2Info* local_info = &ble_glue->c2_info;

    local_info->VersionMajor = wireless_info.VersionMajor;
    local_info->VersionMinor = wireless_info.VersionMinor;
    local_info->VersionSub = wireless_info.VersionSub;
    local_info->VersionBranch = wireless_info.VersionBranch;
    local_info->VersionReleaseType = wireless_info.VersionReleaseType;

    local_info->MemorySizeSram2B = wireless_info.MemorySizeSram2B;
    local_info->MemorySizeSram2A = wireless_info.MemorySizeSram2A;
    local_info->MemorySizeSram1 = wireless_info.MemorySizeSram1;
    local_info->MemorySizeFlash = wireless_info.MemorySizeFlash;

    local_info->StackType = wireless_info.StackType;
    snprintf(
        local_info->StackTypeString,
        BLE_GLUE_MAX_VERSION_STRING_LEN,
        "%d.%d.%d:%s",
        local_info->VersionMajor,
        local_info->VersionMinor,
        local_info->VersionSub,
        ble_glue_get_reltype_str(local_info->StackType));

    local_info->FusVersionMajor = wireless_info.FusVersionMajor;
    local_info->FusVersionMinor = wireless_info.FusVersionMinor;
    local_info->FusVersionSub = wireless_info.FusVersionSub;
    local_info->FusMemorySizeSram2B = wireless_info.FusMemorySizeSram2B;
    local_info->FusMemorySizeSram2A = wireless_info.FusMemorySizeSram2A;
    local_info->FusMemorySizeFlash = wireless_info.FusMemorySizeFlash;
}

static void ble_glue_dump_stack_info() {
    const BleGlueC2Info* c2_info = &ble_glue->c2_info;
    FURI_LOG_I(
        TAG,
        "Core2: FUS: %d.%d.%d, mem %d/%d, flash %d pages",
        c2_info->FusVersionMajor,
        c2_info->FusVersionMinor,
        c2_info->FusVersionSub,
        c2_info->FusMemorySizeSram2B,
        c2_info->FusMemorySizeSram2A,
        c2_info->FusMemorySizeFlash);
    FURI_LOG_I(
        TAG,
        "Core2: Stack: %d.%d.%d, branch %d, reltype %d, stacktype %d, flash %d pages",
        c2_info->VersionMajor,
        c2_info->VersionMinor,
        c2_info->VersionSub,
        c2_info->VersionBranch,
        c2_info->VersionReleaseType,
        c2_info->StackType,
        c2_info->MemorySizeFlash);
}

bool ble_glue_wait_for_c2_start(int32_t timeout) {
    bool started = false;

    do {
        // TODO FL-3505: use mutex?
        started = ble_glue->status == BleGlueStatusC2Started;
        if(!started) {
            timeout--;
            furi_delay_tick(1);
        }
    } while(!started && (timeout > 0));

    if(started) {
        FURI_LOG_I(
            TAG,
            "C2 boot completed, mode: %s",
            ble_glue->c2_info.mode == BleGlueC2ModeFUS ? "FUS" : "Stack");
        ble_glue_update_c2_fw_info();
        ble_glue_dump_stack_info();
    } else {
        FURI_LOG_E(TAG, "C2 startup failed");
        ble_glue->status = BleGlueStatusBroken;
    }

    return started;
}

bool ble_glue_start() {
    furi_assert(ble_glue);

    if(ble_glue->status != BleGlueStatusC2Started) {
        return false;
    }

    bool ret = false;
    if(ble_app_init()) {
        FURI_LOG_I(TAG, "Radio stack started");
        ble_glue->status = BleGlueStatusRadioStackRunning;
        ret = true;
        if(SHCI_C2_SetFlashActivityControl(FLASH_ACTIVITY_CONTROL_SEM7) == SHCI_Success) {
            FURI_LOG_I(TAG, "Flash activity control switched to SEM7");
        } else {
            FURI_LOG_E(TAG, "Failed to switch flash activity control to SEM7");
        }
    } else {
        FURI_LOG_E(TAG, "Radio stack startup failed");
        ble_glue->status = BleGlueStatusRadioStackMissing;
        ble_app_thread_stop();
    }

    return ret;
}

bool ble_glue_is_alive() {
    if(!ble_glue) {
        return false;
    }

    return ble_glue->status >= BleGlueStatusC2Started;
}

bool ble_glue_is_radio_stack_ready() {
    if(!ble_glue) {
        return false;
    }

    return ble_glue->status == BleGlueStatusRadioStackRunning;
}

BleGlueCommandResult ble_glue_force_c2_mode(BleGlueC2Mode desired_mode) {
    furi_check(desired_mode > BleGlueC2ModeUnknown);

    if(desired_mode == ble_glue->c2_info.mode) {
        return BleGlueCommandResultOK;
    }

    if((ble_glue->c2_info.mode == BleGlueC2ModeFUS) && (desired_mode == BleGlueC2ModeStack)) {
        if((ble_glue->c2_info.VersionMajor == 0) && (ble_glue->c2_info.VersionMinor == 0)) {
            FURI_LOG_W(TAG, "Stack isn't installed!");
            return BleGlueCommandResultError;
        }
        SHCI_CmdStatus_t status = SHCI_C2_FUS_StartWs();
        if(status) {
            FURI_LOG_E(TAG, "Failed to start Radio Stack with status: %02X", status);
            return BleGlueCommandResultError;
        }
        return BleGlueCommandResultRestartPending;
    }
    if((ble_glue->c2_info.mode == BleGlueC2ModeStack) && (desired_mode == BleGlueC2ModeFUS)) {
        SHCI_FUS_GetState_ErrorCode_t error_code = 0;
        uint8_t fus_state = SHCI_C2_FUS_GetState(&error_code);
        FURI_LOG_D(TAG, "FUS state: %X, error = %x", fus_state, error_code);
        if(fus_state == SHCI_FUS_CMD_NOT_SUPPORTED) {
            // Second call to SHCI_C2_FUS_GetState() restarts whole MCU & boots FUS
            fus_state = SHCI_C2_FUS_GetState(&error_code);
            FURI_LOG_D(TAG, "FUS state#2: %X, error = %x", fus_state, error_code);
            return BleGlueCommandResultRestartPending;
        }
        return BleGlueCommandResultOK;
    }

    return BleGlueCommandResultError;
}

static void ble_glue_sys_status_not_callback(SHCI_TL_CmdStatus_t status) {
    switch(status) {
    case SHCI_TL_CmdBusy:
        furi_mutex_acquire(ble_glue->shci_mtx, FuriWaitForever);
        break;
    case SHCI_TL_CmdAvailable:
        furi_mutex_release(ble_glue->shci_mtx);
        break;
    default:
        break;
    }
}

/*
 * The type of the payload for a system user event is tSHCI_UserEvtRxParam
 * When the system event is both :
 *    - a ready event (subevtcode = SHCI_SUB_EVT_CODE_READY)
 *    - reported by the FUS (sysevt_ready_rsp == FUS_FW_RUNNING)
 * The buffer shall not be released
 * ( eg ((tSHCI_UserEvtRxParam*)pPayload)->status shall be set to SHCI_TL_UserEventFlow_Disable )
 * When the status is not filled, the buffer is released by default
 */
static void ble_glue_sys_user_event_callback(void* pPayload) {
    UNUSED(pPayload);

#ifdef BLE_GLUE_DEBUG
    APPD_EnableCPU2();
#endif

    TL_AsynchEvt_t* p_sys_event =
        (TL_AsynchEvt_t*)(((tSHCI_UserEvtRxParam*)pPayload)->pckt->evtserial.evt.payload);

    if(p_sys_event->subevtcode == SHCI_SUB_EVT_CODE_READY) {
        FURI_LOG_I(TAG, "Core2 started");
        SHCI_C2_Ready_Evt_t* p_c2_ready_evt = (SHCI_C2_Ready_Evt_t*)p_sys_event->payload;
        if(p_c2_ready_evt->sysevt_ready_rsp == WIRELESS_FW_RUNNING) {
            ble_glue->c2_info.mode = BleGlueC2ModeStack;
        } else if(p_c2_ready_evt->sysevt_ready_rsp == FUS_FW_RUNNING) {
            ble_glue->c2_info.mode = BleGlueC2ModeFUS;
        }

        ble_glue->status = BleGlueStatusC2Started;
    } else if(p_sys_event->subevtcode == SHCI_SUB_EVT_ERROR_NOTIF) {
        FURI_LOG_E(TAG, "Error during initialization");
    } else if(p_sys_event->subevtcode == SHCI_SUB_EVT_BLE_NVM_RAM_UPDATE) {
        SHCI_C2_BleNvmRamUpdate_Evt_t* p_sys_ble_nvm_ram_update_event =
            (SHCI_C2_BleNvmRamUpdate_Evt_t*)p_sys_event->payload;
        if(ble_glue->callback) {
            ble_glue->callback(
                (uint8_t*)p_sys_ble_nvm_ram_update_event->StartAddress,
                p_sys_ble_nvm_ram_update_event->Size,
                ble_glue->context);
        }
    }
}

static void ble_glue_clear_shared_memory() {
    memset(ble_glue_event_pool, 0, sizeof(ble_glue_event_pool));
    memset(&ble_glue_system_cmd_buff, 0, sizeof(ble_glue_system_cmd_buff));
    memset(ble_glue_system_spare_event_buff, 0, sizeof(ble_glue_system_spare_event_buff));
    memset(ble_glue_ble_spare_event_buff, 0, sizeof(ble_glue_ble_spare_event_buff));
}

void ble_glue_thread_stop() {
    if(ble_glue) {
        FuriThreadId thread_id = furi_thread_get_id(ble_glue->thread);
        furi_assert(thread_id);
        furi_thread_flags_set(thread_id, BLE_GLUE_FLAG_KILL_THREAD);
        furi_thread_join(ble_glue->thread);
        furi_thread_free(ble_glue->thread);
        // Free resources
        furi_mutex_free(ble_glue->shci_mtx);
        furi_semaphore_free(ble_glue->shci_sem);
        ble_glue_clear_shared_memory();
        free(ble_glue);
        ble_glue = NULL;
    }
}

// Wrap functions
static int32_t ble_glue_shci_thread(void* context) {
    UNUSED(context);
    uint32_t flags = 0;

    while(true) {
        flags = furi_thread_flags_wait(BLE_GLUE_FLAG_ALL, FuriFlagWaitAny, FuriWaitForever);
        if(flags & BLE_GLUE_FLAG_SHCI_EVENT) {
            shci_user_evt_proc();
        }
        if(flags & BLE_GLUE_FLAG_KILL_THREAD) {
            break;
        }
    }

    return 0;
}

void shci_notify_asynch_evt(void* pdata) {
    UNUSED(pdata);
    if(ble_glue) {
        FuriThreadId thread_id = furi_thread_get_id(ble_glue->thread);
        furi_assert(thread_id);
        furi_thread_flags_set(thread_id, BLE_GLUE_FLAG_SHCI_EVENT);
    }
}

void shci_cmd_resp_release(uint32_t flag) {
    UNUSED(flag);
    if(ble_glue) {
        furi_semaphore_release(ble_glue->shci_sem);
    }
}

void shci_cmd_resp_wait(uint32_t timeout) {
    UNUSED(timeout);
    if(ble_glue) {
        furi_hal_power_insomnia_enter();
        furi_semaphore_acquire(ble_glue->shci_sem, FuriWaitForever);
        furi_hal_power_insomnia_exit();
    }
}

bool ble_glue_reinit_c2() {
    return SHCI_C2_Reinit() == SHCI_Success;
}

BleGlueCommandResult ble_glue_fus_stack_delete() {
    FURI_LOG_I(TAG, "Erasing stack");
    SHCI_CmdStatus_t erase_stat = SHCI_C2_FUS_FwDelete();
    FURI_LOG_I(TAG, "Cmd res = %x", erase_stat);
    if(erase_stat == SHCI_Success) {
        return BleGlueCommandResultOperationOngoing;
    }
    ble_glue_fus_get_status();
    return BleGlueCommandResultError;
}

BleGlueCommandResult ble_glue_fus_stack_install(uint32_t src_addr, uint32_t dst_addr) {
    FURI_LOG_I(TAG, "Installing stack");
    SHCI_CmdStatus_t write_stat = SHCI_C2_FUS_FwUpgrade(src_addr, dst_addr);
    FURI_LOG_I(TAG, "Cmd res = %x", write_stat);
    if(write_stat == SHCI_Success) {
        return BleGlueCommandResultOperationOngoing;
    }
    ble_glue_fus_get_status();
    return BleGlueCommandResultError;
}

BleGlueCommandResult ble_glue_fus_get_status() {
    furi_check(ble_glue->c2_info.mode == BleGlueC2ModeFUS);
    SHCI_FUS_GetState_ErrorCode_t error_code = 0;
    uint8_t fus_state = SHCI_C2_FUS_GetState(&error_code);
    FURI_LOG_I(TAG, "FUS state: %x, error: %x", fus_state, error_code);
    if((error_code != 0) || (fus_state == FUS_STATE_VALUE_ERROR)) {
        return BleGlueCommandResultError;
    } else if(
        (fus_state >= FUS_STATE_VALUE_FW_UPGRD_ONGOING) &&
        (fus_state <= FUS_STATE_VALUE_SERVICE_ONGOING_END)) {
        return BleGlueCommandResultOperationOngoing;
    }
    return BleGlueCommandResultOK;
}

BleGlueCommandResult ble_glue_fus_wait_operation() {
    furi_check(ble_glue->c2_info.mode == BleGlueC2ModeFUS);

    while(true) {
        BleGlueCommandResult fus_status = ble_glue_fus_get_status();
        if(fus_status == BleGlueCommandResultOperationOngoing) {
            furi_delay_ms(20);
        } else if(fus_status == BleGlueCommandResultError) {
            return BleGlueCommandResultError;
        } else {
            return BleGlueCommandResultOK;
        }
    }
}
