#include "ble_glue.h"
#include "app_common.h"
#include "ble_app.h"
#include "ble.h"
#include "tl.h"
#include "shci.h"
#include "shci_tl.h"
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

typedef enum {
    // Stage 1: core2 startup and FUS
    BleGlueStatusStartup,
    BleGlueStatusBroken,
    BleGlueStatusFusStarted,
    // Stage 2: radio stack
    BleGlueStatusRadioStackStarted,
    BleGlueStatusRadioStackMissing
} BleGlueStatus;

typedef struct {
    osMutexId_t shci_mtx;
    osSemaphoreId_t shci_sem;
    FuriThread* thread;
    BleGlueStatus status;
    BleGlueKeyStorageChangedCallback callback;
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

void ble_glue_init() {
    ble_glue = malloc(sizeof(BleGlue));
    ble_glue->status = BleGlueStatusStartup;

    // Configure the system Power Mode
    // Select HSI as system clock source after Wake Up from Stop mode
    LL_RCC_SetClkAfterWakeFromStop(LL_RCC_STOP_WAKEUPCLOCK_HSI);
    /* Initialize the CPU2 reset value before starting CPU2 with C2BOOT */
    LL_C2_PWR_SetPowerMode(LL_PWR_MODE_SHUTDOWN);
    furi_hal_power_insomnia_enter();

    // APPD_Init();

    // Initialize all transport layers
    TL_MM_Config_t tl_mm_config;
    SHCI_TL_HciInitConf_t SHci_Tl_Init_Conf;
    // Reference table initialization
    TL_Init();

    ble_glue->shci_mtx = osMutexNew(NULL);
    ble_glue->shci_sem = osSemaphoreNew(1, 0, NULL);

    // FreeRTOS system task creation
    ble_glue->thread = furi_thread_alloc();
    furi_thread_set_name(ble_glue->thread, "BleShciDriver");
    furi_thread_set_stack_size(ble_glue->thread, 1024);
    furi_thread_set_context(ble_glue->thread, ble_glue);
    furi_thread_set_callback(ble_glue->thread, ble_glue_shci_thread);
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

bool ble_glue_wait_for_fus_start(WirelessFwInfo_t* info) {
    bool ret = false;

    size_t countdown = 1000;
    while(countdown > 0) {
        if(ble_glue->status == BleGlueStatusFusStarted) {
            ret = true;
            break;
        }
        countdown--;
        osDelay(1);
    }

    if(ble_glue->status == BleGlueStatusFusStarted) {
        SHCI_GetWirelessFwInfo(info);
    } else {
        FURI_LOG_E(TAG, "Failed to start FUS");
        ble_glue->status = BleGlueStatusBroken;
    }

    return ret;
}

bool ble_glue_start() {
    furi_assert(ble_glue);

    if(ble_glue->status != BleGlueStatusFusStarted) {
        return false;
    }

    bool ret = false;
    furi_hal_power_insomnia_enter();
    if(ble_app_init()) {
        FURI_LOG_I(TAG, "Radio stack started");
        ble_glue->status = BleGlueStatusRadioStackStarted;
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
    furi_hal_power_insomnia_exit();

    return ret;
}

bool ble_glue_is_alive() {
    if(!ble_glue) {
        return false;
    }

    return ble_glue->status >= BleGlueStatusFusStarted;
}

bool ble_glue_is_radio_stack_ready() {
    if(!ble_glue) {
        return false;
    }

    return ble_glue->status == BleGlueStatusRadioStackStarted;
}

bool ble_glue_radio_stack_fw_launch_started() {
    bool ret = false;
    // Get FUS status
    SHCI_FUS_GetState_ErrorCode_t err_code = 0;
    uint8_t state = SHCI_C2_FUS_GetState(&err_code);
    if(state == FUS_STATE_VALUE_IDLE) {
        // When FUS is running we can't read radio stack version correctly
        // Trying to start radio stack fw, which leads to reset
        FURI_LOG_W(TAG, "FUS is running. Restart to launch Radio Stack");
        SHCI_CmdStatus_t status = SHCI_C2_FUS_StartWs();
        if(status) {
            FURI_LOG_E(TAG, "Failed to start Radio Stack with status: %02X", status);
        } else {
            ret = true;
        }
    }
    return ret;
}

static void ble_glue_sys_status_not_callback(SHCI_TL_CmdStatus_t status) {
    switch(status) {
    case SHCI_TL_CmdBusy:
        osMutexAcquire(ble_glue->shci_mtx, osWaitForever);
        break;
    case SHCI_TL_CmdAvailable:
        osMutexRelease(ble_glue->shci_mtx);
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
    /* Traces channel initialization */
    // APPD_EnableCPU2( );

    TL_AsynchEvt_t* p_sys_event =
        (TL_AsynchEvt_t*)(((tSHCI_UserEvtRxParam*)pPayload)->pckt->evtserial.evt.payload);

    if(p_sys_event->subevtcode == SHCI_SUB_EVT_CODE_READY) {
        FURI_LOG_I(TAG, "Fus started");
        ble_glue->status = BleGlueStatusFusStarted;
        furi_hal_power_insomnia_exit();
    } else if(p_sys_event->subevtcode == SHCI_SUB_EVT_ERROR_NOTIF) {
        FURI_LOG_E(TAG, "Error during initialization");
        furi_hal_power_insomnia_exit();
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
        osThreadId_t thread_id = furi_thread_get_thread_id(ble_glue->thread);
        furi_assert(thread_id);
        osThreadFlagsSet(thread_id, BLE_GLUE_FLAG_KILL_THREAD);
        furi_thread_join(ble_glue->thread);
        furi_thread_free(ble_glue->thread);
        // Free resources
        osMutexDelete(ble_glue->shci_mtx);
        osSemaphoreDelete(ble_glue->shci_sem);
        ble_glue_clear_shared_memory();
        free(ble_glue);
        ble_glue = NULL;
    }
}

// Wrap functions
static int32_t ble_glue_shci_thread(void* context) {
    uint32_t flags = 0;

    while(true) {
        flags = osThreadFlagsWait(BLE_GLUE_FLAG_ALL, osFlagsWaitAny, osWaitForever);
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
        osThreadId_t thread_id = furi_thread_get_thread_id(ble_glue->thread);
        furi_assert(thread_id);
        osThreadFlagsSet(thread_id, BLE_GLUE_FLAG_SHCI_EVENT);
    }
}

void shci_cmd_resp_release(uint32_t flag) {
    UNUSED(flag);
    if(ble_glue) {
        osSemaphoreRelease(ble_glue->shci_sem);
    }
}

void shci_cmd_resp_wait(uint32_t timeout) {
    UNUSED(timeout);
    if(ble_glue) {
        osSemaphoreAcquire(ble_glue->shci_sem, osWaitForever);
    }
}
