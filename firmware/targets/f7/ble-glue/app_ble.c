#include "main.h"

#include "app_entry.h"
#include "app_common.h"
#include "dbg_trace.h"
#include "ble.h"
#include "tl.h"
#include "app_ble.h"
#include "shci.h"
#include "cmsis_os.h"

#include <furi-hal.h>

PLACE_IN_SECTION("MB_MEM1") ALIGN(4) static TL_CmdPacket_t BleCmdBuffer;

// PLACE_IN_SECTION("TAG_OTA_END") const uint32_t MagicKeywordValue = 0x94448A29 ;
// PLACE_IN_SECTION("TAG_OTA_START") const uint32_t MagicKeywordAddress = (uint32_t)&MagicKeywordValue;

osMutexId_t MtxHciId;
osSemaphoreId_t SemHciId;
osThreadId_t HciUserEvtProcessId;

const osThreadAttr_t HciUserEvtProcess_attr = {
    .name = CFG_HCI_USER_EVT_PROCESS_NAME,
    .attr_bits = CFG_HCI_USER_EVT_PROCESS_ATTR_BITS,
    .cb_mem = CFG_HCI_USER_EVT_PROCESS_CB_MEM,
    .cb_size = CFG_HCI_USER_EVT_PROCESS_CB_SIZE,
    .stack_mem = CFG_HCI_USER_EVT_PROCESS_STACK_MEM,
    .priority = CFG_HCI_USER_EVT_PROCESS_PRIORITY,
    .stack_size = CFG_HCI_USER_EVT_PROCESS_STACK_SIZE
};

/* Private function prototypes -----------------------------------------------*/
static void HciUserEvtProcess(void *argument);
static void BLE_UserEvtRx( void * pPayload );
static void BLE_StatusNot( HCI_TL_CmdStatus_t status );
static void Ble_Tl_Init( void );

bool APP_BLE_Init() {
  SHCI_C2_Ble_Init_Cmd_Packet_t ble_init_cmd_packet = {
    {{0,0,0}},                  /**< Header unused */
    {0,                         /** pBleBufferAddress not used */
    0,                          /** BleBufferSize not used */
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
    0}
  };

  // Initialize Ble Transport Layer
  Ble_Tl_Init( );
  // Register the hci transport layer to handle BLE User Asynchronous Events
  HciUserEvtProcessId = osThreadNew(HciUserEvtProcess, NULL, &HciUserEvtProcess_attr);
  // Starts the BLE Stack on CPU2
  return (SHCI_C2_BLE_Init( &ble_init_cmd_packet ) == SHCI_Success);
}

static void Ble_Tl_Init( void ) {
  HCI_TL_HciInitConf_t Hci_Tl_Init_Conf;

  MtxHciId = osMutexNew( NULL );
  SemHciId = osSemaphoreNew( 1, 0, NULL ); /*< Create the semaphore and make it busy at initialization */

  Hci_Tl_Init_Conf.p_cmdbuffer = (uint8_t*)&BleCmdBuffer;
  Hci_Tl_Init_Conf.StatusNotCallBack = BLE_StatusNot;
  hci_init(BLE_UserEvtRx, (void*) &Hci_Tl_Init_Conf);
}

static void HciUserEvtProcess(void *argument) {
  UNUSED(argument);

  for(;;)
  {
    osThreadFlagsWait( 1, osFlagsWaitAny, osWaitForever);
    hci_user_evt_proc( );
  }
}

/*************************************************************
 *
 * WRAP FUNCTIONS
 *
 *************************************************************/
void hci_notify_asynch_evt(void* pdata) {
  UNUSED(pdata);
  osThreadFlagsSet( HciUserEvtProcessId, 1 );
}

void hci_cmd_resp_release(uint32_t flag) {
  UNUSED(flag);
  osSemaphoreRelease( SemHciId );
}

void hci_cmd_resp_wait(uint32_t timeout) {
  UNUSED(timeout);
  osSemaphoreAcquire( SemHciId, osWaitForever );
}

static void BLE_UserEvtRx( void * pPayload ) {
  SVCCTL_UserEvtFlowStatus_t svctl_return_status;
  tHCI_UserEvtRxParam *pParam;

  pParam = (tHCI_UserEvtRxParam *)pPayload;

  svctl_return_status = SVCCTL_UserEvtRx((void *)&(pParam->pckt->evtserial));
  if (svctl_return_status != SVCCTL_UserEvtFlowDisable) {
    pParam->status = HCI_TL_UserEventFlow_Enable;
  } else {
    pParam->status = HCI_TL_UserEventFlow_Disable;
  }
}

static void BLE_StatusNot( HCI_TL_CmdStatus_t status ) {
  switch (status) {
    case HCI_TL_CmdBusy:
      osMutexAcquire( MtxHciId, osWaitForever );
      break;
    case HCI_TL_CmdAvailable:
      osMutexRelease( MtxHciId );
      break;
    default:
      break;
  }
}

void SVCCTL_ResumeUserEventFlow( void ) {
  hci_resume_flow();
}
