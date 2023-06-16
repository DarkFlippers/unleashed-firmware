#include "ble_app.h"

#include <ble/ble.h>
#include <interface/patterns/ble_thread/tl/hci_tl.h>
#include <interface/patterns/ble_thread/shci/shci.h>
#include "gap.h"

#include <furi_hal.h>
#include <furi.h>

#define TAG "Bt"

#define BLE_APP_FLAG_HCI_EVENT (1UL << 0)
#define BLE_APP_FLAG_KILL_THREAD (1UL << 1)
#define BLE_APP_FLAG_ALL (BLE_APP_FLAG_HCI_EVENT | BLE_APP_FLAG_KILL_THREAD)

PLACE_IN_SECTION("MB_MEM1") ALIGN(4) static TL_CmdPacket_t ble_app_cmd_buffer;
PLACE_IN_SECTION("MB_MEM2") ALIGN(4) static uint32_t ble_app_nvm[BLE_NVM_SRAM_SIZE];

_Static_assert(
    sizeof(SHCI_C2_Ble_Init_Cmd_Packet_t) == 57,
    "Ble stack config structure size mismatch (check new config options - last updated for v.1.15.0)");

typedef struct {
    FuriMutex* hci_mtx;
    FuriSemaphore* hci_sem;
    FuriThread* thread;
} BleApp;

static BleApp* ble_app = NULL;

static int32_t ble_app_hci_thread(void* context);
static void ble_app_hci_event_handler(void* pPayload);
static void ble_app_hci_status_not_handler(HCI_TL_CmdStatus_t status);

bool ble_app_init() {
    SHCI_CmdStatus_t status;
    ble_app = malloc(sizeof(BleApp));
    // Allocate semafore and mutex for ble command buffer access
    ble_app->hci_mtx = furi_mutex_alloc(FuriMutexTypeNormal);
    ble_app->hci_sem = furi_semaphore_alloc(1, 0);
    // HCI transport layer thread to handle user asynch events
    ble_app->thread = furi_thread_alloc_ex("BleHciDriver", 1024, ble_app_hci_thread, ble_app);
    furi_thread_start(ble_app->thread);

    // Initialize Ble Transport Layer
    HCI_TL_HciInitConf_t hci_tl_config = {
        .p_cmdbuffer = (uint8_t*)&ble_app_cmd_buffer,
        .StatusNotCallBack = ble_app_hci_status_not_handler,
    };
    hci_init(ble_app_hci_event_handler, (void*)&hci_tl_config);

    // Configure NVM store for pairing data
    SHCI_C2_CONFIG_Cmd_Param_t config_param = {
        .PayloadCmdSize = SHCI_C2_CONFIG_PAYLOAD_CMD_SIZE,
        .Config1 = SHCI_C2_CONFIG_CONFIG1_BIT0_BLE_NVM_DATA_TO_SRAM,
        .BleNvmRamAddress = (uint32_t)ble_app_nvm,
        .EvtMask1 = SHCI_C2_CONFIG_EVTMASK1_BIT1_BLE_NVM_RAM_UPDATE_ENABLE,
    };
    status = SHCI_C2_Config(&config_param);
    if(status) {
        FURI_LOG_E(TAG, "Failed to configure 2nd core: %d", status);
    }

    // Start ble stack on 2nd core
    SHCI_C2_Ble_Init_Cmd_Packet_t ble_init_cmd_packet = {
        .Header = {{0, 0, 0}}, // Header unused
        .Param = {
            .pBleBufferAddress = 0, // pBleBufferAddress not used
            .BleBufferSize = 0, // BleBufferSize not used
            .NumAttrRecord = CFG_BLE_NUM_GATT_ATTRIBUTES,
            .NumAttrServ = CFG_BLE_NUM_GATT_SERVICES,
            .AttrValueArrSize = CFG_BLE_ATT_VALUE_ARRAY_SIZE,
            .NumOfLinks = CFG_BLE_NUM_LINK,
            .ExtendedPacketLengthEnable = CFG_BLE_DATA_LENGTH_EXTENSION,
            .PrWriteListSize = CFG_BLE_PREPARE_WRITE_LIST_SIZE,
            .MblockCount = CFG_BLE_MBLOCK_COUNT,
            .AttMtu = CFG_BLE_MAX_ATT_MTU,
            .SlaveSca = CFG_BLE_SLAVE_SCA,
            .MasterSca = CFG_BLE_MASTER_SCA,
            .LsSource = CFG_BLE_LSE_SOURCE,
            .MaxConnEventLength = CFG_BLE_MAX_CONN_EVENT_LENGTH,
            .HsStartupTime = CFG_BLE_HSE_STARTUP_TIME,
            .ViterbiEnable = CFG_BLE_VITERBI_MODE,
            .Options = CFG_BLE_OPTIONS,
            .HwVersion = 0,
            .max_coc_initiator_nbr = 32,
            .min_tx_power = 0,
            .max_tx_power = 0,
            .rx_model_config = 1,
            /* New stack (13.3->15.0) */
            .max_adv_set_nbr = 1, // Only used if SHCI_C2_BLE_INIT_OPTIONS_EXT_ADV is set
            .max_adv_data_len = 31, // Only used if SHCI_C2_BLE_INIT_OPTIONS_EXT_ADV is set
            .tx_path_compens = 0, // RF TX Path Compensation, * 0.1 dB
            .rx_path_compens = 0, // RF RX Path Compensation, * 0.1 dB
            .ble_core_version = 11, // BLE Core Version: 11(5.2), 12(5.3)
        }};
    status = SHCI_C2_BLE_Init(&ble_init_cmd_packet);
    if(status) {
        FURI_LOG_E(TAG, "Failed to start ble stack: %d", status);
    }
    return status == SHCI_Success;
}

void ble_app_get_key_storage_buff(uint8_t** addr, uint16_t* size) {
    *addr = (uint8_t*)ble_app_nvm;
    *size = sizeof(ble_app_nvm);
}

void ble_app_thread_stop() {
    if(ble_app) {
        FuriThreadId thread_id = furi_thread_get_id(ble_app->thread);
        furi_assert(thread_id);
        furi_thread_flags_set(thread_id, BLE_APP_FLAG_KILL_THREAD);
        furi_thread_join(ble_app->thread);
        furi_thread_free(ble_app->thread);
        // Free resources
        furi_mutex_free(ble_app->hci_mtx);
        furi_semaphore_free(ble_app->hci_sem);
        free(ble_app);
        ble_app = NULL;
        memset(&ble_app_cmd_buffer, 0, sizeof(ble_app_cmd_buffer));
    }
}

static int32_t ble_app_hci_thread(void* arg) {
    UNUSED(arg);
    uint32_t flags = 0;

    while(1) {
        flags = furi_thread_flags_wait(BLE_APP_FLAG_ALL, FuriFlagWaitAny, FuriWaitForever);
        if(flags & BLE_APP_FLAG_KILL_THREAD) {
            break;
        }
        if(flags & BLE_APP_FLAG_HCI_EVENT) {
            hci_user_evt_proc();
        }
    }

    return 0;
}

// Called by WPAN lib
void hci_notify_asynch_evt(void* pdata) {
    UNUSED(pdata);
    furi_check(ble_app);
    FuriThreadId thread_id = furi_thread_get_id(ble_app->thread);
    furi_assert(thread_id);
    furi_thread_flags_set(thread_id, BLE_APP_FLAG_HCI_EVENT);
}

void hci_cmd_resp_release(uint32_t flag) {
    UNUSED(flag);
    furi_check(ble_app);
    furi_check(furi_semaphore_release(ble_app->hci_sem) == FuriStatusOk);
}

void hci_cmd_resp_wait(uint32_t timeout) {
    furi_check(ble_app);
    furi_check(furi_semaphore_acquire(ble_app->hci_sem, timeout) == FuriStatusOk);
}

static void ble_app_hci_event_handler(void* pPayload) {
    SVCCTL_UserEvtFlowStatus_t svctl_return_status;
    tHCI_UserEvtRxParam* pParam = (tHCI_UserEvtRxParam*)pPayload;

    furi_check(ble_app);
    svctl_return_status = SVCCTL_UserEvtRx((void*)&(pParam->pckt->evtserial));
    if(svctl_return_status != SVCCTL_UserEvtFlowDisable) {
        pParam->status = HCI_TL_UserEventFlow_Enable;
    } else {
        pParam->status = HCI_TL_UserEventFlow_Disable;
    }
}

static void ble_app_hci_status_not_handler(HCI_TL_CmdStatus_t status) {
    if(status == HCI_TL_CmdBusy) {
        furi_mutex_acquire(ble_app->hci_mtx, FuriWaitForever);
    } else if(status == HCI_TL_CmdAvailable) {
        furi_mutex_release(ble_app->hci_mtx);
    }
}

void SVCCTL_ResumeUserEventFlow(void) {
    hci_resume_flow();
}
