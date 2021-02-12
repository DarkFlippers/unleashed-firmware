#include "main.h"

#include "app_common.h"
#include "dbg_trace.h"
#include "ble.h"
#include "tl.h"
#include "app_ble.h"

#include "cmsis_os.h"
#include "shci.h"
#include "stm32_lpm.h"
#include "otp.h"
#include "dis_app.h"
#include "hrs_app.h"

#include <api-hal.h>

typedef struct _tSecurityParams {
  uint8_t ioCapability;
  uint8_t mitm_mode;
  uint8_t bonding_mode;
  uint8_t Use_Fixed_Pin;
  uint8_t encryptionKeySizeMin;
  uint8_t encryptionKeySizeMax;
  uint32_t Fixed_Pin;
  uint8_t initiateSecurity;
} tSecurityParams;

typedef struct _tBLEProfileGlobalContext {
  tSecurityParams bleSecurityParam;
  uint16_t gapServiceHandle;
  uint16_t devNameCharHandle;
  uint16_t appearanceCharHandle;
  uint16_t connectionHandle;
  uint8_t advtServUUIDlen;
  uint8_t advtServUUID[100];
} BleGlobalContext_t;

typedef struct {
  BleGlobalContext_t BleApplicationContext_legacy;
  APP_BLE_ConnStatus_t Device_Connection_Status;
  uint8_t Advertising_mgr_timer_Id;
} BleApplicationContext_t;

#define APPBLE_GAP_DEVICE_NAME_LENGTH 7
#define FAST_ADV_TIMEOUT               (30*1000*1000/CFG_TS_TICK_VAL) /**< 30s */
#define INITIAL_ADV_TIMEOUT            (60*1000*1000/CFG_TS_TICK_VAL) /**< 60s */

#define BD_ADDR_SIZE_LOCAL    6

#define LED_ON_TIMEOUT                 (0.005*1000*1000/CFG_TS_TICK_VAL) /**< 5ms */

PLACE_IN_SECTION("MB_MEM1") ALIGN(4) static TL_CmdPacket_t BleCmdBuffer;

static const uint8_t M_bd_addr[BD_ADDR_SIZE_LOCAL] =
    {
        (uint8_t)((CFG_ADV_BD_ADDRESS & 0x0000000000FF)),
        (uint8_t)((CFG_ADV_BD_ADDRESS & 0x00000000FF00) >> 8),
        (uint8_t)((CFG_ADV_BD_ADDRESS & 0x000000FF0000) >> 16),
        (uint8_t)((CFG_ADV_BD_ADDRESS & 0x0000FF000000) >> 24),
        (uint8_t)((CFG_ADV_BD_ADDRESS & 0x00FF00000000) >> 32),
        (uint8_t)((CFG_ADV_BD_ADDRESS & 0xFF0000000000) >> 40)
    };

static uint8_t bd_addr_udn[BD_ADDR_SIZE_LOCAL];

static const uint8_t BLE_CFG_IR_VALUE[16] = CFG_BLE_IRK;
static const uint8_t BLE_CFG_ER_VALUE[16] = CFG_BLE_ERK;

PLACE_IN_SECTION("TAG_OTA_END") const uint32_t MagicKeywordValue = 0x94448A29 ;
PLACE_IN_SECTION("TAG_OTA_START") const uint32_t MagicKeywordAddress = (uint32_t)&MagicKeywordValue;

PLACE_IN_SECTION("BLE_APP_CONTEXT") static BleApplicationContext_t BleApplicationContext;
PLACE_IN_SECTION("BLE_APP_CONTEXT") static uint16_t AdvIntervalMin, AdvIntervalMax;

static const char local_name[] = { AD_TYPE_COMPLETE_LOCAL_NAME ,'F','L','I','P','P', 'E', 'R'};
uint8_t  manuf_data[14] = {
    sizeof(manuf_data)-1, AD_TYPE_MANUFACTURER_SPECIFIC_DATA,
    0x01/*SKD version */,
    0x00 /* Generic*/,
    0x00 /* GROUP A Feature  */,
    0x00 /* GROUP A Feature */,
    0x00 /* GROUP B Feature */,
    0x00 /* GROUP B Feature */,
    0x00, /* BLE MAC start -MSB */
    0x00,
    0x00,
    0x00,
    0x00,
    0x00, /* BLE MAC stop */

};

osMutexId_t MtxHciId;
osSemaphoreId_t SemHciId;
osThreadId_t AdvUpdateProcessId;
osThreadId_t HciUserEvtProcessId;

const osThreadAttr_t AdvUpdateProcess_attr = {
    .name = CFG_ADV_UPDATE_PROCESS_NAME,
    .attr_bits = CFG_ADV_UPDATE_PROCESS_ATTR_BITS,
    .cb_mem = CFG_ADV_UPDATE_PROCESS_CB_MEM,
    .cb_size = CFG_ADV_UPDATE_PROCESS_CB_SIZE,
    .stack_mem = CFG_ADV_UPDATE_PROCESS_STACK_MEM,
    .priority = CFG_ADV_UPDATE_PROCESS_PRIORITY,
    .stack_size = CFG_ADV_UPDATE_PROCESS_STACK_SIZE
};

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
static void Ble_Hci_Gap_Gatt_Init();
static const uint8_t* BleGetBdAddress( void );
static void Adv_Request( APP_BLE_ConnStatus_t New_Status );
static void Add_Advertisment_Service_UUID( uint16_t servUUID );
static void Adv_Mgr( void );
static void AdvUpdateProcess(void *argument);
static void Adv_Update( void );


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
  if (SHCI_C2_BLE_Init( &ble_init_cmd_packet ) != SHCI_Success) {
    return false;
  }
  // Initialization of HCI & GATT & GAP layer
  Ble_Hci_Gap_Gatt_Init();
  // Initialization of the BLE Services
  SVCCTL_Init();
  // Initialization of the BLE App Context
  BleApplicationContext.Device_Connection_Status = APP_BLE_IDLE;
  BleApplicationContext.BleApplicationContext_legacy.connectionHandle = 0xFFFF;
  // From here, all initialization are BLE application specific
  AdvUpdateProcessId = osThreadNew(AdvUpdateProcess, NULL, &AdvUpdateProcess_attr);

  // Initialization of ADV - Ad Manufacturer Element - Support OTA Bit Masks
#if(BLE_CFG_OTA_REBOOT_CHAR != 0)
  manuf_data[sizeof(manuf_data)-8] = CFG_FEATURE_OTA_REBOOT;
#endif

  // Initialize DIS Application
  DISAPP_Init();
  // Initialize HRS Application
  HRSAPP_Init();
  // Create timer to handle the connection state machine
  HW_TS_Create(CFG_TIM_PROC_ID_ISR, &(BleApplicationContext.Advertising_mgr_timer_Id), hw_ts_SingleShot, Adv_Mgr);

  // Make device discoverable
  BleApplicationContext.BleApplicationContext_legacy.advtServUUID[0] = AD_TYPE_16_BIT_SERV_UUID;
  BleApplicationContext.BleApplicationContext_legacy.advtServUUIDlen = 1;
  Add_Advertisment_Service_UUID(HEART_RATE_SERVICE_UUID);
  /* Initialize intervals for reconnexion without intervals update */
  AdvIntervalMin = CFG_FAST_CONN_ADV_INTERVAL_MIN;
  AdvIntervalMax = CFG_FAST_CONN_ADV_INTERVAL_MAX;

  Adv_Request(APP_BLE_FAST_ADV);
  return true;
}

SVCCTL_UserEvtFlowStatus_t SVCCTL_App_Notification( void *pckt )
{
  hci_event_pckt *event_pckt;
  evt_le_meta_event *meta_evt;
  evt_blue_aci *blue_evt;
  hci_le_phy_update_complete_event_rp0 *evt_le_phy_update_complete;
  uint8_t TX_PHY, RX_PHY;
  tBleStatus ret = BLE_STATUS_INVALID_PARAMS;

  event_pckt = (hci_event_pckt*) ((hci_uart_pckt *) pckt)->data;

  switch (event_pckt->evt) {
    case EVT_DISCONN_COMPLETE:
    {
      hci_disconnection_complete_event_rp0 *disconnection_complete_event;
      disconnection_complete_event = (hci_disconnection_complete_event_rp0 *) event_pckt->data;

      if (disconnection_complete_event->Connection_Handle == BleApplicationContext.BleApplicationContext_legacy.connectionHandle) {
        BleApplicationContext.BleApplicationContext_legacy.connectionHandle = 0;
        BleApplicationContext.Device_Connection_Status = APP_BLE_IDLE;
        APP_DBG_MSG("\r\n\r** DISCONNECTION EVENT WITH CLIENT \r\n");
      }
      /* restart advertising */
      Adv_Request(APP_BLE_FAST_ADV);
      api_hal_power_insomnia_exit();
    }
    break; /* EVT_DISCONN_COMPLETE */

    case EVT_LE_META_EVENT:
    {
      meta_evt = (evt_le_meta_event*) event_pckt->data;
      switch (meta_evt->subevent)
      {
        case EVT_LE_CONN_UPDATE_COMPLETE:
          APP_DBG_MSG("\r\n\r** CONNECTION UPDATE EVENT WITH CLIENT \r\n");

          /* USER CODE BEGIN EVT_LE_CONN_UPDATE_COMPLETE */

          /* USER CODE END EVT_LE_CONN_UPDATE_COMPLETE */
          break;
        case EVT_LE_PHY_UPDATE_COMPLETE:
          APP_DBG_MSG("EVT_UPDATE_PHY_COMPLETE \r\n");
          evt_le_phy_update_complete = (hci_le_phy_update_complete_event_rp0*)meta_evt->data;
          if (evt_le_phy_update_complete->Status == 0)
          {
            APP_DBG_MSG("EVT_UPDATE_PHY_COMPLETE, status ok \r\n");
          }
          else
          {
            APP_DBG_MSG("EVT_UPDATE_PHY_COMPLETE, status nok \r\n");
          }

          ret = hci_le_read_phy(BleApplicationContext.BleApplicationContext_legacy.connectionHandle,&TX_PHY,&RX_PHY);
          if (ret == BLE_STATUS_SUCCESS)
          {
            APP_DBG_MSG("Read_PHY success \r\n");

            if ((TX_PHY == TX_2M) && (RX_PHY == RX_2M))
            {
              APP_DBG_MSG("PHY Param  TX= %d, RX= %d \r\n", TX_PHY, RX_PHY);
            }
            else
            {
              APP_DBG_MSG("PHY Param  TX= %d, RX= %d \r\n", TX_PHY, RX_PHY);
            }
          }
          else
          {
            APP_DBG_MSG("Read conf not succeess \r\n");
          }
          break;
        case EVT_LE_CONN_COMPLETE:
        {
          api_hal_power_insomnia_enter();
          hci_le_connection_complete_event_rp0 *connection_complete_event;

          /**
           * The connection is done, there is no need anymore to schedule the LP ADV
           */
          connection_complete_event = (hci_le_connection_complete_event_rp0 *) meta_evt->data;

          HW_TS_Stop(BleApplicationContext.Advertising_mgr_timer_Id);

          APP_DBG_MSG("EVT_LE_CONN_COMPLETE for connection handle 0x%x\r\n", connection_complete_event->Connection_Handle);
          if (BleApplicationContext.Device_Connection_Status == APP_BLE_LP_CONNECTING)
          {
            /* Connection as client */
            BleApplicationContext.Device_Connection_Status = APP_BLE_CONNECTED_CLIENT;
          }
          else
          {
            /* Connection as server */
            BleApplicationContext.Device_Connection_Status = APP_BLE_CONNECTED_SERVER;
          }
          BleApplicationContext.BleApplicationContext_legacy.connectionHandle = connection_complete_event->Connection_Handle;
        }
        break; /* HCI_EVT_LE_CONN_COMPLETE */
        default:
          break;
      }
    }
    break; /* HCI_EVT_LE_META_EVENT */

    case EVT_VENDOR:
      blue_evt = (evt_blue_aci*) event_pckt->data;
      switch (blue_evt->ecode) {
        aci_gap_pairing_complete_event_rp0 *pairing_complete;

      case EVT_BLUE_GAP_LIMITED_DISCOVERABLE: 
        APP_DBG_MSG("\r\n\r** EVT_BLUE_GAP_LIMITED_DISCOVERABLE \r\n");
          break; /* EVT_BLUE_GAP_LIMITED_DISCOVERABLE */
          
      case EVT_BLUE_GAP_PASS_KEY_REQUEST:  
        APP_DBG_MSG("\r\n\r** EVT_BLUE_GAP_PASS_KEY_REQUEST \r\n");

        aci_gap_pass_key_resp(BleApplicationContext.BleApplicationContext_legacy.connectionHandle,123456);

        APP_DBG_MSG("\r\n\r** aci_gap_pass_key_resp \r\n");
          break; /* EVT_BLUE_GAP_PASS_KEY_REQUEST */

      case EVT_BLUE_GAP_AUTHORIZATION_REQUEST:    
        APP_DBG_MSG("\r\n\r** EVT_BLUE_GAP_AUTHORIZATION_REQUEST \r\n");
          break; /* EVT_BLUE_GAP_AUTHORIZATION_REQUEST */

      case EVT_BLUE_GAP_SLAVE_SECURITY_INITIATED:   
        APP_DBG_MSG("\r\n\r** EVT_BLUE_GAP_SLAVE_SECURITY_INITIATED \r\n");
          break; /* EVT_BLUE_GAP_SLAVE_SECURITY_INITIATED */

      case EVT_BLUE_GAP_BOND_LOST:    
        APP_DBG_MSG("\r\n\r** EVT_BLUE_GAP_BOND_LOST \r\n");
          aci_gap_allow_rebond(BleApplicationContext.BleApplicationContext_legacy.connectionHandle);
        APP_DBG_MSG("\r\n\r** Send allow rebond \r\n");
          break; /* EVT_BLUE_GAP_BOND_LOST */

      case EVT_BLUE_GAP_DEVICE_FOUND:  
        APP_DBG_MSG("\r\n\r** EVT_BLUE_GAP_DEVICE_FOUND \r\n");
          break; /* EVT_BLUE_GAP_DEVICE_FOUND */

      case EVT_BLUE_GAP_ADDR_NOT_RESOLVED:
         APP_DBG_MSG("\r\n\r** EVT_BLUE_GAP_DEVICE_FOUND \r\n");
          break; /* EVT_BLUE_GAP_DEVICE_FOUND */
      
      case (EVT_BLUE_GAP_KEYPRESS_NOTIFICATION):
         APP_DBG_MSG("\r\n\r** EVT_BLUE_GAP_KEYPRESS_NOTIFICATION \r\n");
          break; /* EVT_BLUE_GAP_KEY_PRESS_NOTIFICATION */    

       case (EVT_BLUE_GAP_NUMERIC_COMPARISON_VALUE):
          APP_DBG_MSG("numeric_value = %ld\r\n",
                      ((aci_gap_numeric_comparison_value_event_rp0 *)(blue_evt->data))->Numeric_Value);

          APP_DBG_MSG("Hex_value = %lx\r\n",
                      ((aci_gap_numeric_comparison_value_event_rp0 *)(blue_evt->data))->Numeric_Value);

          aci_gap_numeric_comparison_value_confirm_yesno(BleApplicationContext.BleApplicationContext_legacy.connectionHandle, 1); /* CONFIRM_YES = 1 */

          APP_DBG_MSG("\r\n\r** aci_gap_numeric_comparison_value_confirm_yesno-->YES \r\n");
          break;

        case (EVT_BLUE_GAP_PAIRING_CMPLT):
          {
            pairing_complete = (aci_gap_pairing_complete_event_rp0*)blue_evt->data;

            APP_DBG_MSG("BLE_CTRL_App_Notification: EVT_BLUE_GAP_PAIRING_CMPLT, pairing_complete->Status = %d\r\n",pairing_complete->Status);
            if (pairing_complete->Status == 0) {
              APP_DBG_MSG("\r\n\r** Pairing OK \r\n");
            } else {
              APP_DBG_MSG("\r\n\r** Pairing KO \r\n");
            }
          }
          break;

      /* USER CODE END ecode */
        case EVT_BLUE_GAP_PROCEDURE_COMPLETE:
          APP_DBG_MSG("\r\n\r** EVT_BLUE_GAP_PROCEDURE_COMPLETE \r\n");
          break;
      }
      break; /* EVT_VENDOR */
      default:
        break;
  }

  return (SVCCTL_UserEvtFlowEnable);
}

APP_BLE_ConnStatus_t APP_BLE_Get_Server_Connection_Status() {
    return BleApplicationContext.Device_Connection_Status;
}

/* USER CODE BEGIN FD*/
void APP_BLE_Key_Button1_Action() {
  tBleStatus ret = BLE_STATUS_INVALID_PARAMS;
  ret = aci_gap_clear_security_db();
  if (ret == BLE_STATUS_SUCCESS) {
    APP_DBG_MSG("Successfully aci_gap_clear_security_db()\r\n");
  } else {
    APP_DBG_MSG("aci_gap_clear_security_db() Failed , result: %d \r\n", ret);
  }
}

void APP_BLE_Key_Button2_Action() {
  tBleStatus ret = BLE_STATUS_INVALID_PARAMS;
  ret = aci_gap_slave_security_req(BleApplicationContext.BleApplicationContext_legacy.connectionHandle); 
  if (ret == BLE_STATUS_SUCCESS) {
    APP_DBG_MSG("Successfully aci_gap_slave_security_req()");
  } else {
    APP_DBG_MSG("aci_gap_slave_security_req() Failed , result: %d \r\n", ret);
  }
}
  
void APP_BLE_Key_Button3_Action() {
  uint8_t TX_PHY, RX_PHY;
  tBleStatus ret = BLE_STATUS_INVALID_PARAMS;
  ret = hci_le_read_phy(BleApplicationContext.BleApplicationContext_legacy.connectionHandle,&TX_PHY,&RX_PHY);
  if (ret == BLE_STATUS_SUCCESS) {
    APP_DBG_MSG("Read_PHY success \r\n");
    APP_DBG_MSG("PHY Param  TX= %d, RX= %d \r\n", TX_PHY, RX_PHY);
    if ((TX_PHY == TX_2M) && (RX_PHY == RX_2M)) {
      APP_DBG_MSG("hci_le_set_phy PHY Param  TX= %d, RX= %d \r\n", TX_1M, RX_1M);
      ret = hci_le_set_phy(BleApplicationContext.BleApplicationContext_legacy.connectionHandle,ALL_PHYS_PREFERENCE,TX_1M,RX_1M,0);
    } else {
      APP_DBG_MSG("hci_le_set_phy PHY Param  TX= %d, RX= %d \r\n", TX_2M_PREFERRED, RX_2M_PREFERRED);
      ret = hci_le_set_phy(BleApplicationContext.BleApplicationContext_legacy.connectionHandle,ALL_PHYS_PREFERENCE,TX_2M_PREFERRED,RX_2M_PREFERRED,0);
    } 
  } else {
    APP_DBG_MSG("Read conf not succeess \r\n");
  }

  if (ret == BLE_STATUS_SUCCESS) {
    APP_DBG_MSG("set PHY cmd ok\r\n");
  } else {
    APP_DBG_MSG("set PHY cmd NOK\r\n");
  }
}

static void Ble_Tl_Init( void ) {
  HCI_TL_HciInitConf_t Hci_Tl_Init_Conf;

  MtxHciId = osMutexNew( NULL );
  SemHciId = osSemaphoreNew( 1, 0, NULL ); /*< Create the semaphore and make it busy at initialization */

  Hci_Tl_Init_Conf.p_cmdbuffer = (uint8_t*)&BleCmdBuffer;
  Hci_Tl_Init_Conf.StatusNotCallBack = BLE_StatusNot;
  hci_init(BLE_UserEvtRx, (void*) &Hci_Tl_Init_Conf);
}

static void Ble_Hci_Gap_Gatt_Init() {
  uint8_t role;
  uint16_t gap_service_handle, gap_dev_name_char_handle, gap_appearance_char_handle;
  const uint8_t *bd_addr;
  uint32_t srd_bd_addr[2];
  uint16_t appearance[1] = { BLE_CFG_GAP_APPEARANCE };

  /*HCI Reset to synchronise BLE Stack*/
  hci_reset();

  /**
   * Write the BD Address
   */
  bd_addr = BleGetBdAddress();
  aci_hal_write_config_data(CONFIG_DATA_PUBADDR_OFFSET,
                            CONFIG_DATA_PUBADDR_LEN,
                            (uint8_t*) bd_addr);

  /* BLE MAC in ADV Packet */
  manuf_data[ sizeof(manuf_data)-6] = bd_addr[5];
  manuf_data[ sizeof(manuf_data)-5] = bd_addr[4];
  manuf_data[ sizeof(manuf_data)-4] = bd_addr[3];
  manuf_data[ sizeof(manuf_data)-3] = bd_addr[2];
  manuf_data[ sizeof(manuf_data)-2] = bd_addr[1];
  manuf_data[ sizeof(manuf_data)-1] = bd_addr[0];

  /**
   * Write Identity root key used to derive LTK and CSRK
   */
    aci_hal_write_config_data(CONFIG_DATA_IR_OFFSET,
    CONFIG_DATA_IR_LEN,
                            (uint8_t*) BLE_CFG_IR_VALUE);

   /**
   * Write Encryption root key used to derive LTK and CSRK
   */
    aci_hal_write_config_data(CONFIG_DATA_ER_OFFSET,
    CONFIG_DATA_ER_LEN,
                            (uint8_t*) BLE_CFG_ER_VALUE);

   /**
   * Write random bd_address
   */
   /* random_bd_address = R_bd_address;
    aci_hal_write_config_data(CONFIG_DATA_RANDOM_ADDRESS_WR,
    CONFIG_DATA_RANDOM_ADDRESS_LEN,
                            (uint8_t*) random_bd_address);
  */

  /**
   * Static random Address
   * The two upper bits shall be set to 1
   * The lowest 32bits is read from the UDN to differentiate between devices
   * The RNG may be used to provide a random number on each power on
   */
  srd_bd_addr[1] =  0x0000ED6E;
  srd_bd_addr[0] =  LL_FLASH_GetUDN( );
  aci_hal_write_config_data( CONFIG_DATA_RANDOM_ADDRESS_OFFSET, CONFIG_DATA_RANDOM_ADDRESS_LEN, (uint8_t*)srd_bd_addr );

  /**
   * Write Identity root key used to derive LTK and CSRK
   */
    aci_hal_write_config_data( CONFIG_DATA_IR_OFFSET, CONFIG_DATA_IR_LEN, (uint8_t*)BLE_CFG_IR_VALUE );

   /**
   * Write Encryption root key used to derive LTK and CSRK
   */
    aci_hal_write_config_data( CONFIG_DATA_ER_OFFSET, CONFIG_DATA_ER_LEN, (uint8_t*)BLE_CFG_ER_VALUE );

  /**
   * Set TX Power to 0dBm.
   */
  aci_hal_set_tx_power_level(1, CFG_TX_POWER);

  /**
   * Initialize GATT interface
   */
  aci_gatt_init();

  /**
   * Initialize GAP interface
   */
  role = 0;

#if (BLE_CFG_PERIPHERAL == 1)
  role |= GAP_PERIPHERAL_ROLE;
#endif

#if (BLE_CFG_CENTRAL == 1)
  role |= GAP_CENTRAL_ROLE;
#endif

  if (role > 0)
  {
    const char *name = "Flipper";
    aci_gap_init(role, 0,
                 APPBLE_GAP_DEVICE_NAME_LENGTH,
                 &gap_service_handle, &gap_dev_name_char_handle, &gap_appearance_char_handle);

    if (aci_gatt_update_char_value(gap_service_handle, gap_dev_name_char_handle, 0, strlen(name), (uint8_t *) name))
    {
      BLE_DBG_SVCCTL_MSG("Device Name aci_gatt_update_char_value failed.\r\n");
    }
  }

  if(aci_gatt_update_char_value(gap_service_handle,
                                gap_appearance_char_handle,
                                0,
                                2,
                                (uint8_t *)&appearance))
  {
    BLE_DBG_SVCCTL_MSG("Appearance aci_gatt_update_char_value failed.\r\n");
  }
  /**
   * Initialize Default PHY
   */
  hci_le_set_default_phy(ALL_PHYS_PREFERENCE,TX_2M_PREFERRED,RX_2M_PREFERRED);

  /**
   * Initialize IO capability
   */
  BleApplicationContext.BleApplicationContext_legacy.bleSecurityParam.ioCapability = CFG_IO_CAPABILITY;
  aci_gap_set_io_capability(BleApplicationContext.BleApplicationContext_legacy.bleSecurityParam.ioCapability);

  /**
   * Initialize authentication
   */
  BleApplicationContext.BleApplicationContext_legacy.bleSecurityParam.mitm_mode = CFG_MITM_PROTECTION;
  BleApplicationContext.BleApplicationContext_legacy.bleSecurityParam.encryptionKeySizeMin = CFG_ENCRYPTION_KEY_SIZE_MIN;
  BleApplicationContext.BleApplicationContext_legacy.bleSecurityParam.encryptionKeySizeMax = CFG_ENCRYPTION_KEY_SIZE_MAX;
  BleApplicationContext.BleApplicationContext_legacy.bleSecurityParam.Use_Fixed_Pin = CFG_USED_FIXED_PIN;
  BleApplicationContext.BleApplicationContext_legacy.bleSecurityParam.Fixed_Pin = CFG_FIXED_PIN;
  BleApplicationContext.BleApplicationContext_legacy.bleSecurityParam.bonding_mode = CFG_BONDING_MODE;

  aci_gap_set_authentication_requirement(BleApplicationContext.BleApplicationContext_legacy.bleSecurityParam.bonding_mode,
                                         BleApplicationContext.BleApplicationContext_legacy.bleSecurityParam.mitm_mode,
                                         CFG_SC_SUPPORT,
                                         CFG_KEYPRESS_NOTIFICATION_SUPPORT,
                                         BleApplicationContext.BleApplicationContext_legacy.bleSecurityParam.encryptionKeySizeMin,
                                         BleApplicationContext.BleApplicationContext_legacy.bleSecurityParam.encryptionKeySizeMax,
                                         BleApplicationContext.BleApplicationContext_legacy.bleSecurityParam.Use_Fixed_Pin,
                                         BleApplicationContext.BleApplicationContext_legacy.bleSecurityParam.Fixed_Pin,
                                         PUBLIC_ADDR
                                         );

  /**
   * Initialize whitelist
   */
   if (BleApplicationContext.BleApplicationContext_legacy.bleSecurityParam.bonding_mode)
   {
     aci_gap_configure_whitelist();
   }
}

static void Adv_Request(APP_BLE_ConnStatus_t New_Status)
{
  tBleStatus ret = BLE_STATUS_INVALID_PARAMS;
  uint16_t Min_Inter, Max_Inter;

  if (New_Status == APP_BLE_FAST_ADV)
  {
    Min_Inter = AdvIntervalMin;
    Max_Inter = AdvIntervalMax;
  }
  else
  {
    Min_Inter = CFG_LP_CONN_ADV_INTERVAL_MIN;
    Max_Inter = CFG_LP_CONN_ADV_INTERVAL_MAX;
  }

    /**
     * Stop the timer, it will be restarted for a new shot
     * It does not hurt if the timer was not running
     */
    HW_TS_Stop(BleApplicationContext.Advertising_mgr_timer_Id);

    APP_DBG_MSG("First index in %d state \r\n", BleApplicationContext.Device_Connection_Status);

    if ((New_Status == APP_BLE_LP_ADV)
        && ((BleApplicationContext.Device_Connection_Status == APP_BLE_FAST_ADV)
            || (BleApplicationContext.Device_Connection_Status == APP_BLE_LP_ADV)))
    {
      /* Connection in ADVERTISE mode have to stop the current advertising */
      ret = aci_gap_set_non_discoverable();
      if (ret == BLE_STATUS_SUCCESS)
      {
        APP_DBG_MSG("Successfully Stopped Advertising \r\n");
      }
      else
      {
        APP_DBG_MSG("Stop Advertising Failed , result: %d \r\n", ret);
      }
    }

    BleApplicationContext.Device_Connection_Status = New_Status;
    /* Start Fast or Low Power Advertising */
    ret = aci_gap_set_discoverable(
        ADV_IND,
        Min_Inter,
        Max_Inter,
        PUBLIC_ADDR,
        NO_WHITE_LIST_USE, /* use white list */
        sizeof(local_name),
        (uint8_t*) &local_name,
        BleApplicationContext.BleApplicationContext_legacy.advtServUUIDlen,
        BleApplicationContext.BleApplicationContext_legacy.advtServUUID,
        0,
        0);

    /* Update Advertising data */
    ret = aci_gap_update_adv_data(sizeof(manuf_data), (uint8_t*) manuf_data);
    if (ret == BLE_STATUS_SUCCESS) {
      if (New_Status == APP_BLE_FAST_ADV) {
        APP_DBG_MSG("Successfully Start Fast Advertising \r\n" );
        /* Start Timer to STOP ADV - TIMEOUT */
        HW_TS_Start(BleApplicationContext.Advertising_mgr_timer_Id, INITIAL_ADV_TIMEOUT);
      } else {
        APP_DBG_MSG("Successfully Start Low Power Advertising \r\n");
      }
    } else {
      if (New_Status == APP_BLE_FAST_ADV) {
        APP_DBG_MSG("Start Fast Advertising Failed , result: %d \r\n", ret);
      } else {
        APP_DBG_MSG("Start Low Power Advertising Failed , result: %d \r\n", ret);
      }
    }
}

const uint8_t* BleGetBdAddress( void ) {
  uint8_t *otp_addr;
  const uint8_t *bd_addr;
  uint32_t udn;
  uint32_t company_id;
  uint32_t device_id;

  udn = LL_FLASH_GetUDN();

  if(udn != 0xFFFFFFFF) {
    company_id = LL_FLASH_GetSTCompanyID();
    device_id = LL_FLASH_GetDeviceID();

    bd_addr_udn[0] = (uint8_t)(udn & 0x000000FF);
    bd_addr_udn[1] = (uint8_t)( (udn & 0x0000FF00) >> 8 );
    bd_addr_udn[2] = (uint8_t)( (udn & 0x00FF0000) >> 16 );
    bd_addr_udn[3] = (uint8_t)device_id;
    bd_addr_udn[4] = (uint8_t)(company_id & 0x000000FF);;
    bd_addr_udn[5] = (uint8_t)( (company_id & 0x0000FF00) >> 8 );

    bd_addr = (const uint8_t *)bd_addr_udn;
  } else {
    otp_addr = OTP_Read(0);
    if(otp_addr) {
      bd_addr = ((OTP_ID0_t*)otp_addr)->bd_address;
    } else {
      bd_addr = M_bd_addr;
    }
  }

  return bd_addr;
}

/*************************************************************
 *
 *SPECIFIC FUNCTIONS
 *
 *************************************************************/
static void Add_Advertisment_Service_UUID( uint16_t servUUID ) {
  BleApplicationContext.BleApplicationContext_legacy.advtServUUID[BleApplicationContext.BleApplicationContext_legacy.advtServUUIDlen] =
      (uint8_t) (servUUID & 0xFF);
  BleApplicationContext.BleApplicationContext_legacy.advtServUUIDlen++;
  BleApplicationContext.BleApplicationContext_legacy.advtServUUID[BleApplicationContext.BleApplicationContext_legacy.advtServUUIDlen] =
      (uint8_t) (servUUID >> 8) & 0xFF;
  BleApplicationContext.BleApplicationContext_legacy.advtServUUIDlen++;

}

static void Adv_Mgr( void ) {
  /**
   * The code shall be executed in the background as an aci command may be sent
   * The background is the only place where the application can make sure a new aci command
   * is not sent if there is a pending one
   */
  osThreadFlagsSet( AdvUpdateProcessId, 1 );
}

static void AdvUpdateProcess(void *argument) {
  UNUSED(argument);

  for(;;) {
    osThreadFlagsWait( 1, osFlagsWaitAny, osWaitForever);
    Adv_Update( );
  }
}

static void Adv_Update( void ) {
  Adv_Request(APP_BLE_LP_ADV);

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
