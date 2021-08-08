#include "app_common.h"
#include "ble.h"
#include "dis_app.h"
#include <furi-hal-version.h>

#if ((BLE_CFG_DIS_SYSTEM_ID != 0) || (CFG_MENU_DEVICE_INFORMATION != 0))
static const uint8_t system_id[BLE_CFG_DIS_SYSTEM_ID_LEN_MAX] = {
  (uint8_t)((DISAPP_MANUFACTURER_ID & 0xFF0000) >> 16),
  (uint8_t)((DISAPP_MANUFACTURER_ID & 0x00FF00) >> 8),
  (uint8_t)(DISAPP_MANUFACTURER_ID & 0x0000FF),
  0xFE,
  0xFF,
  (uint8_t)((DISAPP_OUI & 0xFF0000) >> 16),
  (uint8_t)((DISAPP_OUI & 0x00FF00) >> 8),
  (uint8_t)(DISAPP_OUI & 0x0000FF)
};
#endif

#if ((BLE_CFG_DIS_IEEE_CERTIFICATION != 0) || (CFG_MENU_DEVICE_INFORMATION != 0))
static const uint8_t ieee_id[BLE_CFG_DIS_IEEE_CERTIFICATION_LEN_MAX] = {
  0xFE, 0xCA, 0xFE, 0xCA, 0xFE, 0xCA, 0xFE, 0xCA,
  0xFE, 0xCA, 0xFE, 0xCA, 0xFE, 0xCA, 0xFE, 0xCA,
  0xFE, 0xCA, 0xFE, 0xCA, 0xFE, 0xCA, 0xFE, 0xCA,
  0xFE, 0xCA, 0xFE, 0xCA, 0xFE, 0xCA, 0xFE, 0xCA,
};
#endif

#if ((BLE_CFG_DIS_PNP_ID != 0) || (CFG_MENU_DEVICE_INFORMATION != 0))
static const uint8_t pnp_id[BLE_CFG_DIS_PNP_ID_LEN_MAX] = {
  0x1,
  0xAD, 0xDE,
  0xDE, 0xDA,
  0x01, 0x00
};
#endif

void DISAPP_Init(void) {
  DIS_Data_t dis_information_data;

#if ((BLE_CFG_DIS_MANUFACTURER_NAME_STRING != 0) || (CFG_MENU_DEVICE_INFORMATION != 0))
  /**
   * Update MANUFACTURER NAME Information
   *
   * @param UUID
   * @param pPData
   * @return
   */
  dis_information_data.pPayload = (uint8_t*)DISAPP_MANUFACTURER_NAME;
  dis_information_data.Length = sizeof(DISAPP_MANUFACTURER_NAME);
  DIS_UpdateChar(MANUFACTURER_NAME_UUID, &dis_information_data);
#endif

#if ((BLE_CFG_DIS_MODEL_NUMBER_STRING != 0) || (CFG_MENU_DEVICE_INFORMATION != 0))
  /**
   * Update MODEL NUMBERInformation
   *
   * @param UUID
   * @param pPData
   * @return
   */
  const char* name = furi_hal_version_get_device_name_ptr();
  dis_information_data.pPayload = (uint8_t*)name;
  dis_information_data.Length = strlen(name) + 1;
  DIS_UpdateChar(MODEL_NUMBER_UUID, &dis_information_data);
#endif

#if ((BLE_CFG_DIS_SERIAL_NUMBER_STRING != 0) || (CFG_MENU_DEVICE_INFORMATION != 0))
  /**
   * Update SERIAL NUMBERInformation
   *
   * @param UUID
   * @param pPData
   * @return
   */
  dis_information_data.pPayload = (uint8_t*)DISAPP_SERIAL_NUMBER;
  dis_information_data.Length = sizeof(DISAPP_SERIAL_NUMBER);
  DIS_UpdateChar(SERIAL_NUMBER_UUID, &dis_information_data);
#endif

#if ((BLE_CFG_DIS_HARDWARE_REVISION_STRING != 0) || (CFG_MENU_DEVICE_INFORMATION != 0))
  /**
   * Update HARDWARE REVISION NUMBERInformation
   *
   * @param UUID
   * @param pPData
   * @return
   */
  dis_information_data.pPayload = (uint8_t*)DISAPP_HARDWARE_REVISION_NUMBER;
  dis_information_data.Length = sizeof(DISAPP_HARDWARE_REVISION_NUMBER);
  DIS_UpdateChar(HARDWARE_REVISION_UUID, &dis_information_data);
#endif

#if ((BLE_CFG_DIS_FIRMWARE_REVISION_STRING != 0) || (CFG_MENU_DEVICE_INFORMATION != 0))
  /**
   * Update FIRMWARE REVISION NUMBERInformation
   *
   * @param UUID
   * @param pPData
   * @return
   */
  dis_information_data.pPayload = (uint8_t*)DISAPP_FIRMWARE_REVISION_NUMBER;
  dis_information_data.Length = sizeof(DISAPP_FIRMWARE_REVISION_NUMBER);
  DIS_UpdateChar(FIRMWARE_REVISION_UUID, &dis_information_data);
#endif

#if ((BLE_CFG_DIS_SOFTWARE_REVISION_STRING != 0) || (CFG_MENU_DEVICE_INFORMATION != 0))
  /**
   * Update SOFTWARE REVISION NUMBERInformation
   *
   * @param UUID
   * @param pPData
   * @return
   */
  dis_information_data.pPayload = (uint8_t*)DISAPP_SOFTWARE_REVISION_NUMBER;
  dis_information_data.Length = sizeof(DISAPP_SOFTWARE_REVISION_NUMBER);
  DIS_UpdateChar(SOFTWARE_REVISION_UUID, &dis_information_data);
#endif

#if ((BLE_CFG_DIS_SYSTEM_ID != 0) || (CFG_MENU_DEVICE_INFORMATION != 0))
  /**
   * Update SYSTEM ID Information
   *
   * @param UUID
   * @param pPData
   * @return
   */
  dis_information_data.pPayload = (uint8_t *)system_id;
  dis_information_data.Length = BLE_CFG_DIS_SYSTEM_ID_LEN_MAX;
  DIS_UpdateChar(SYSTEM_ID_UUID, &dis_information_data);
#endif

#if ((BLE_CFG_DIS_IEEE_CERTIFICATION != 0) || (CFG_MENU_DEVICE_INFORMATION != 0))
  /**
   * Update IEEE CERTIFICATION ID Information
   *
   * @param UUID
   * @param pPData
   * @return
   */
  dis_information_data.pPayload = (uint8_t *)ieee_id;
  dis_information_data.Length = BLE_CFG_DIS_IEEE_CERTIFICATION_LEN_MAX;
  DIS_UpdateChar(IEEE_CERTIFICATION_UUID, &dis_information_data);
#endif

#if ((BLE_CFG_DIS_PNP_ID != 0) || (CFG_MENU_DEVICE_INFORMATION != 0))
  /**
   * Update PNP ID Information
   *
   * @param UUID
   * @param pPData
   * @return
   */
  dis_information_data.pPayload = (uint8_t *)pnp_id;
  dis_information_data.Length = BLE_CFG_DIS_PNP_ID_LEN_MAX;
  DIS_UpdateChar(PNP_ID_UUID, &dis_information_data);
#endif
}
