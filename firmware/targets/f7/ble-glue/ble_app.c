#include "ble_app.h"

#include "hci_tl.h"
#include "ble.h"
#include "shci.h"
#include "cmsis_os.h"
#include "gap.h"

#include <furi-hal.h>

#define TAG "Bt"

PLACE_IN_SECTION("MB_MEM1") ALIGN(4) static TL_CmdPacket_t ble_app_cmd_buffer;
PLACE_IN_SECTION("MB_MEM2") ALIGN(4) static uint32_t ble_app_nvm[BLE_NVM_SRAM_SIZE];

typedef struct {
    osMutexId_t hci_mtx;
    osSemaphoreId_t hci_sem;
    osThreadId_t hci_thread_id;
    osThreadAttr_t hci_thread_attr;
} BleApp;

static BleApp* ble_app;

static void ble_app_hci_thread(void *arg);
static void ble_app_hci_event_handler(void * pPayload);
static void ble_app_hci_status_not_handler(HCI_TL_CmdStatus_t status);

bool ble_app_init() {
    SHCI_CmdStatus_t status;
    ble_app = furi_alloc(sizeof(BleApp));
    // Allocate semafore and mutex for ble command buffer access
    ble_app->hci_mtx = osMutexNew(NULL);
    ble_app->hci_sem = osSemaphoreNew(1, 0, NULL);
    // HCI transport layer thread to handle user asynch events
    ble_app->hci_thread_attr.name = "BleHciWorker";
    ble_app->hci_thread_attr.stack_size = 1024;
    ble_app->hci_thread_id = osThreadNew(ble_app_hci_thread, NULL, &ble_app->hci_thread_attr);

    // Initialize Ble Transport Layer
    HCI_TL_HciInitConf_t hci_tl_config = {
        .p_cmdbuffer = (uint8_t*)&ble_app_cmd_buffer,
        .StatusNotCallBack = ble_app_hci_status_not_handler,
    };
    hci_init(ble_app_hci_event_handler, (void*)&hci_tl_config);

    // Configure NVM store for pairing data
    SHCI_C2_CONFIG_Cmd_Param_t config_param = {
        .PayloadCmdSize = SHCI_C2_CONFIG_PAYLOAD_CMD_SIZE,
        .Config1 =SHCI_C2_CONFIG_CONFIG1_BIT0_BLE_NVM_DATA_TO_SRAM,
        .BleNvmRamAddress = (uint32_t)ble_app_nvm,
        .EvtMask1 = SHCI_C2_CONFIG_EVTMASK1_BIT1_BLE_NVM_RAM_UPDATE_ENABLE,
    };
    status = SHCI_C2_Config(&config_param);
    if(status) {
        FURI_LOG_E(TAG, "Failed to configure 2nd core: %d", status);
    }

    // Start ble stack on 2nd core
    SHCI_C2_Ble_Init_Cmd_Packet_t ble_init_cmd_packet = {
        .Header = {{0,0,0}}, // Header unused
        .Param = {
            0, // pBleBufferAddress not used
            0, // BleBufferSize not used
            CFG_BLE_NUM_GATT_ATTRIBUTES,
            CFG_BLE_NUM_GATT_SERVICES,
            CFG_BLE_ATT_VALUE_ARRAY_SIZE,
            CFG_BLE_NUM_LINK,
            CFG_BLE_DATA_LENGTH_EXTENSION,
            CFG_BLE_PREPARE_WRITE_LIST_SIZE,
            CFG_BLE_MBLOCK_COUNT,
            CFG_BLE_MAX_ATT_MTU,
            CFG_BLE_SLAVE_SCA,
            CFG_BLE_MASTER_SCA,
            CFG_BLE_LSE_SOURCE,
            CFG_BLE_MAX_CONN_EVENT_LENGTH,
            CFG_BLE_HSE_STARTUP_TIME,
            CFG_BLE_VITERBI_MODE,
            CFG_BLE_LL_ONLY,
            0,
        }
    };
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

static void ble_app_hci_thread(void *arg) {
    while(1) {
        osThreadFlagsWait(1, osFlagsWaitAny, osWaitForever);
        hci_user_evt_proc();
    }
}

// Called by WPAN lib
void hci_notify_asynch_evt(void* pdata) {
    osThreadFlagsSet(ble_app->hci_thread_id, 1);
}

void hci_cmd_resp_release(uint32_t flag) {
    osSemaphoreRelease(ble_app->hci_sem);
}

void hci_cmd_resp_wait(uint32_t timeout) {
    osSemaphoreAcquire(ble_app->hci_sem, osWaitForever);
}

static void ble_app_hci_event_handler( void * pPayload ) {
    SVCCTL_UserEvtFlowStatus_t svctl_return_status;
    tHCI_UserEvtRxParam *pParam = (tHCI_UserEvtRxParam *)pPayload;

    svctl_return_status = SVCCTL_UserEvtRx((void *)&(pParam->pckt->evtserial));
    if (svctl_return_status != SVCCTL_UserEvtFlowDisable) {
        pParam->status = HCI_TL_UserEventFlow_Enable;
    } else {
        pParam->status = HCI_TL_UserEventFlow_Disable;
    }
}

static void ble_app_hci_status_not_handler( HCI_TL_CmdStatus_t status ) {
    if(status == HCI_TL_CmdBusy) {
        osMutexAcquire(ble_app->hci_mtx, osWaitForever );
    } else if(status == HCI_TL_CmdAvailable) {
        osMutexRelease(ble_app->hci_mtx);
    }
}

void SVCCTL_ResumeUserEventFlow( void ) {
    hci_resume_flow();
}
