#include "app_common.h"
#include "main.h"
#include "app_entry.h"
#include "app_ble.h"
#include "ble.h"
#include "tl.h"
#include "cmsis_os.h"
#include "shci_tl.h"
#include <furi-hal.h>

extern RTC_HandleTypeDef hrtc;

#define POOL_SIZE (CFG_TLBLE_EVT_QUEUE_LENGTH*4U*DIVC(( sizeof(TL_PacketHeader_t) + TL_BLE_EVENT_FRAME_SIZE ), 4U))

PLACE_IN_SECTION("MB_MEM2") ALIGN(4) static uint8_t EvtPool[POOL_SIZE];
PLACE_IN_SECTION("MB_MEM2") ALIGN(4) static TL_CmdPacket_t SystemCmdBuffer;
PLACE_IN_SECTION("MB_MEM2") ALIGN(4) static uint8_t SystemSpareEvtBuffer[sizeof(TL_PacketHeader_t) + TL_EVT_HDR_SIZE + 255U];
PLACE_IN_SECTION("MB_MEM2") ALIGN(4) static uint8_t BleSpareEvtBuffer[sizeof(TL_PacketHeader_t) + TL_EVT_HDR_SIZE + 255];

osMutexId_t MtxShciId;
osSemaphoreId_t SemShciId;
osThreadId_t ShciUserEvtProcessId;

volatile static BleGlueStatus ble_glue_status = BleGlueStatusUninitialized;

const osThreadAttr_t ShciUserEvtProcess_attr = {
    .name = CFG_SHCI_USER_EVT_PROCESS_NAME,
    .attr_bits = CFG_SHCI_USER_EVT_PROCESS_ATTR_BITS,
    .cb_mem = CFG_SHCI_USER_EVT_PROCESS_CB_MEM,
    .cb_size = CFG_SHCI_USER_EVT_PROCESS_CB_SIZE,
    .stack_mem = CFG_SHCI_USER_EVT_PROCESS_STACK_MEM,
    .priority = CFG_SHCI_USER_EVT_PROCESS_PRIORITY,
    .stack_size = CFG_SHCI_USER_EVT_PROCESS_STACK_SIZE
};

static void ShciUserEvtProcess(void *argument);
static void SystemPower_Config( void );
static void appe_Tl_Init( void );
static void APPE_SysStatusNot( SHCI_TL_CmdStatus_t status );
static void APPE_SysUserEvtRx( void * pPayload );

BleGlueStatus APPE_Status() {
  return ble_glue_status;
}

void APPE_Init() {
  ble_glue_status = BleGlueStatusStartup;
  SystemPower_Config(); /**< Configure the system Power Mode */

  HW_TS_Init(hw_ts_InitMode_Full, &hrtc); /**< Initialize the TimerServer */

  // APPD_Init();
  furi_hal_power_insomnia_enter();

  appe_Tl_Init();	/* Initialize all transport layers */

  /**
   * From now, the application is waiting for the ready event ( VS_HCI_C2_Ready )
   * received on the system channel before starting the Stack
   * This system event is received with APPE_SysUserEvtRx()
   */
}

/*************************************************************
 *
 * LOCAL FUNCTIONS
 *
 *************************************************************/

/**
 * @brief  Configure the system for power optimization
 *
 * @note  This API configures the system to be ready for low power mode
 *
 * @param  None
 * @retval None
 */
static void SystemPower_Config(void) {
  // Select HSI as system clock source after Wake Up from Stop mode
  LL_RCC_SetClkAfterWakeFromStop(LL_RCC_STOP_WAKEUPCLOCK_HSI);

  /* Initialize the CPU2 reset value before starting CPU2 with C2BOOT */
  LL_C2_PWR_SetPowerMode(LL_PWR_MODE_SHUTDOWN);
}

static void appe_Tl_Init( void ) {
  TL_MM_Config_t tl_mm_config;
  SHCI_TL_HciInitConf_t SHci_Tl_Init_Conf;
  /**< Reference table initialization */
  TL_Init();

  MtxShciId = osMutexNew( NULL );
  SemShciId = osSemaphoreNew( 1, 0, NULL ); /*< Create the semaphore and make it busy at initialization */

  /** FreeRTOS system task creation */
  ShciUserEvtProcessId = osThreadNew(ShciUserEvtProcess, NULL, &ShciUserEvtProcess_attr);

  /**< System channel initialization */
  SHci_Tl_Init_Conf.p_cmdbuffer = (uint8_t*)&SystemCmdBuffer;
  SHci_Tl_Init_Conf.StatusNotCallBack = APPE_SysStatusNot;
  shci_init(APPE_SysUserEvtRx, (void*) &SHci_Tl_Init_Conf);

  /**< Memory Manager channel initialization */
  tl_mm_config.p_BleSpareEvtBuffer = BleSpareEvtBuffer;
  tl_mm_config.p_SystemSpareEvtBuffer = SystemSpareEvtBuffer;
  tl_mm_config.p_AsynchEvtPool = EvtPool;
  tl_mm_config.AsynchEvtPoolSize = POOL_SIZE;
  TL_MM_Init( &tl_mm_config );

  TL_Enable();
}

static void APPE_SysStatusNot( SHCI_TL_CmdStatus_t status ) {
  switch (status) {
    case SHCI_TL_CmdBusy:
      osMutexAcquire( MtxShciId, osWaitForever );
      break;
    case SHCI_TL_CmdAvailable:
      osMutexRelease( MtxShciId );
      break;
    default:
      break;
  }
}

/**
 * The type of the payload for a system user event is tSHCI_UserEvtRxParam
 * When the system event is both :
 *    - a ready event (subevtcode = SHCI_SUB_EVT_CODE_READY)
 *    - reported by the FUS (sysevt_ready_rsp == FUS_FW_RUNNING)
 * The buffer shall not be released
 * ( eg ((tSHCI_UserEvtRxParam*)pPayload)->status shall be set to SHCI_TL_UserEventFlow_Disable )
 * When the status is not filled, the buffer is released by default
 */
static void APPE_SysUserEvtRx( void * pPayload ) {
  UNUSED(pPayload);
  /* Traces channel initialization */
  // APPD_EnableCPU2( );
  
  if (APP_BLE_Init()) {
    FURI_LOG_I("Core2", "BLE stack started");
    ble_glue_status = BleGlueStatusStarted;
  } else {
    FURI_LOG_E("Core2", "BLE stack startup failed");
    ble_glue_status = BleGlueStatusBroken;
  }
  furi_hal_power_insomnia_exit();
}

/*************************************************************
 *
 * FREERTOS WRAPPER FUNCTIONS
 *
*************************************************************/
static void ShciUserEvtProcess(void *argument) {
  UNUSED(argument);
  for(;;) {
    osThreadFlagsWait(1, osFlagsWaitAny, osWaitForever);
    shci_user_evt_proc();
  }
}

/*************************************************************
 *
 * WRAP FUNCTIONS
 *
 *************************************************************/
void shci_notify_asynch_evt(void* pdata) {
  UNUSED(pdata);
  osThreadFlagsSet( ShciUserEvtProcessId, 1 );
}

void shci_cmd_resp_release(uint32_t flag) {
  UNUSED(flag);
  osSemaphoreRelease( SemShciId );
}

void shci_cmd_resp_wait(uint32_t timeout) {
  UNUSED(timeout);
  osSemaphoreAcquire( SemShciId, osWaitForever );
}

#if(CFG_DEBUG_TRACE != 0)
void DbgOutputInit( void )
{
}

void DbgOutputTraces(  uint8_t *p_data, uint16_t size, void (*cb)(void) )
{
  furi_hal_console_tx(p_data, size);
  cb();
}
#endif

