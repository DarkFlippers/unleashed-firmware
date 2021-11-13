#include "ble_glue.h"
#include "app_common.h"
#include "main.h"
#include "ble_app.h"
#include "ble.h"
#include "tl.h"
#include "shci.h"
#include "cmsis_os.h"
#include "shci_tl.h"
#include "app_debug.h"
#include <furi-hal.h>

#define TAG "Core2"

#define POOL_SIZE (CFG_TLBLE_EVT_QUEUE_LENGTH*4U*DIVC(( sizeof(TL_PacketHeader_t) + TL_BLE_EVENT_FRAME_SIZE ), 4U))

PLACE_IN_SECTION("MB_MEM2") ALIGN(4) static uint8_t ble_glue_event_pool[POOL_SIZE];
PLACE_IN_SECTION("MB_MEM2") ALIGN(4) static TL_CmdPacket_t ble_glue_system_cmd_buff;
PLACE_IN_SECTION("MB_MEM2") ALIGN(4) static uint8_t ble_glue_system_spare_event_buff[sizeof(TL_PacketHeader_t) + TL_EVT_HDR_SIZE + 255U];
PLACE_IN_SECTION("MB_MEM2") ALIGN(4) static uint8_t ble_glue_ble_spare_event_buff[sizeof(TL_PacketHeader_t) + TL_EVT_HDR_SIZE + 255];

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
    osThreadId_t shci_user_event_thread_id;
    osThreadAttr_t shci_user_event_thread_attr;
    BleGlueStatus status;
    BleGlueKeyStorageChangedCallback callback;
    void* context;
} BleGlue;

static BleGlue* ble_glue = NULL;

static void ble_glue_user_event_thread(void *argument);
static void ble_glue_sys_status_not_callback(SHCI_TL_CmdStatus_t status);
static void ble_glue_sys_user_event_callback(void* pPayload);

void ble_glue_set_key_storage_changed_callback(BleGlueKeyStorageChangedCallback callback, void* context) {
    furi_assert(ble_glue);
    furi_assert(callback);
    ble_glue->callback = callback;
    ble_glue->context = context;
}

void ble_glue_init() {
    ble_glue = furi_alloc(sizeof(BleGlue));
    ble_glue->status = BleGlueStatusStartup;
    ble_glue->shci_user_event_thread_attr.name = "BleShciWorker";
    ble_glue->shci_user_event_thread_attr.stack_size = 1024;

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
    ble_glue->shci_user_event_thread_id = osThreadNew(ble_glue_user_event_thread, NULL, &ble_glue->shci_user_event_thread_attr);

    // System channel initialization
    SHci_Tl_Init_Conf.p_cmdbuffer = (uint8_t*)&ble_glue_system_cmd_buff;
    SHci_Tl_Init_Conf.StatusNotCallBack = ble_glue_sys_status_not_callback;
    shci_init(ble_glue_sys_user_event_callback, (void*) &SHci_Tl_Init_Conf);

    /**< Memory Manager channel initialization */
    tl_mm_config.p_BleSpareEvtBuffer = ble_glue_ble_spare_event_buff;
    tl_mm_config.p_SystemSpareEvtBuffer = ble_glue_system_spare_event_buff;
    tl_mm_config.p_AsynchEvtPool = ble_glue_event_pool;
    tl_mm_config.AsynchEvtPoolSize = POOL_SIZE;
    TL_MM_Init( &tl_mm_config );
    TL_Enable();

    /*
     * From now, the application is waiting for the ready event ( VS_HCI_C2_Ready )
     * received on the system channel before starting the Stack
     * This system event is received with ble_glue_sys_user_event_callback()
     */
}

static bool ble_glue_wait_status(BleGlueStatus status) {
    bool ret = false;
    size_t countdown = 1000;
    while (countdown > 0) {
        if (ble_glue->status == status) {
            ret = true;
            break;
        }
        countdown--;
        osDelay(1);
    }
    return ret;
}

bool ble_glue_start() {
    furi_assert(ble_glue);

    if (!ble_glue_wait_status(BleGlueStatusFusStarted)) {
        // shutdown core2 power
        FURI_LOG_E(TAG, "Core2 catastrophic failure, cutting its power");
        LL_C2_PWR_SetPowerMode(LL_PWR_MODE_SHUTDOWN);
        ble_glue->status = BleGlueStatusBroken;
        furi_hal_power_insomnia_exit();
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

static void ble_glue_sys_status_not_callback(SHCI_TL_CmdStatus_t status) {
    switch (status) {
    case SHCI_TL_CmdBusy:
        osMutexAcquire( ble_glue->shci_mtx, osWaitForever );
        break;
    case SHCI_TL_CmdAvailable:
        osMutexRelease( ble_glue->shci_mtx );
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
static void ble_glue_sys_user_event_callback( void * pPayload ) {
    UNUSED(pPayload);
    /* Traces channel initialization */
    // APPD_EnableCPU2( );

    TL_AsynchEvt_t *p_sys_event = (TL_AsynchEvt_t*)(((tSHCI_UserEvtRxParam*)pPayload)->pckt->evtserial.evt.payload);
    
    if(p_sys_event->subevtcode == SHCI_SUB_EVT_CODE_READY) {
        FURI_LOG_I(TAG, "Fus started");
        ble_glue->status = BleGlueStatusFusStarted;
        furi_hal_power_insomnia_exit();
    } else if(p_sys_event->subevtcode == SHCI_SUB_EVT_ERROR_NOTIF) {
        FURI_LOG_E(TAG, "Error during initialization");
        furi_hal_power_insomnia_exit();
    } else if(p_sys_event->subevtcode == SHCI_SUB_EVT_BLE_NVM_RAM_UPDATE) {
        SHCI_C2_BleNvmRamUpdate_Evt_t* p_sys_ble_nvm_ram_update_event = (SHCI_C2_BleNvmRamUpdate_Evt_t*)p_sys_event->payload;
        if(ble_glue->callback) {
            ble_glue->callback((uint8_t*)p_sys_ble_nvm_ram_update_event->StartAddress, p_sys_ble_nvm_ram_update_event->Size, ble_glue->context);
        }
    }
}

// Wrap functions
static void ble_glue_user_event_thread(void *argument) {
    UNUSED(argument);
    for(;;) {
        osThreadFlagsWait(1, osFlagsWaitAny, osWaitForever);
        shci_user_evt_proc();
    }
}

void shci_notify_asynch_evt(void* pdata) {
    UNUSED(pdata);
    osThreadFlagsSet(ble_glue->shci_user_event_thread_id, 1);
}

void shci_cmd_resp_release(uint32_t flag) {
    UNUSED(flag);
    osSemaphoreRelease(ble_glue->shci_sem);
}

void shci_cmd_resp_wait(uint32_t timeout) {
    UNUSED(timeout);
    osSemaphoreAcquire(ble_glue->shci_sem, osWaitForever);
}
