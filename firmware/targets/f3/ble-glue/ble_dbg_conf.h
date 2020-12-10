/**
 ******************************************************************************
  * File Name          : App/ble_dbg_conf.h
  * Description        : Debug configuration file for BLE Middleware.
  *
 ******************************************************************************
  * @attention
  *
  * <h2><center>&copy; Copyright (c) 2020 STMicroelectronics.
  * All rights reserved.</center></h2>
  *
  * This software component is licensed by ST under Ultimate Liberty license
  * SLA0044, the "License"; You may not use this file except in compliance with
  * the License. You may obtain a copy of the License at:
  *                             www.st.com/SLA0044
  *
  ******************************************************************************
  */

/* Define to prevent recursive inclusion -------------------------------------*/
#ifndef __BLE_DBG_CONF_H
#define __BLE_DBG_CONF_H

/**
 * Enable or Disable traces from BLE
 */

#define BLE_DBG_APP_EN             0
#define BLE_DBG_DIS_EN             0
#define BLE_DBG_HRS_EN             0
#define BLE_DBG_SVCCTL_EN          0
#define BLE_DBG_BLS_EN             0
#define BLE_DBG_HTS_EN             0
#define BLE_DBG_P2P_STM_EN         0

/**
 * Macro definition
 */
#if ( BLE_DBG_APP_EN != 0 )
#define BLE_DBG_APP_MSG             PRINT_MESG_DBG
#else
#define BLE_DBG_APP_MSG             PRINT_NO_MESG
#endif

#if ( BLE_DBG_DIS_EN != 0 )
#define BLE_DBG_DIS_MSG             PRINT_MESG_DBG
#else
#define BLE_DBG_DIS_MSG             PRINT_NO_MESG
#endif

#if ( BLE_DBG_HRS_EN != 0 )
#define BLE_DBG_HRS_MSG             PRINT_MESG_DBG
#else
#define BLE_DBG_HRS_MSG             PRINT_NO_MESG
#endif

#if ( BLE_DBG_P2P_STM_EN != 0 )
#define BLE_DBG_P2P_STM_MSG         PRINT_MESG_DBG
#else
#define BLE_DBG_P2P_STM_MSG         PRINT_NO_MESG
#endif

#if ( BLE_DBG_TEMPLATE_STM_EN != 0 )
#define BLE_DBG_TEMPLATE_STM_MSG         PRINT_MESG_DBG
#else
#define BLE_DBG_TEMPLATE_STM_MSG         PRINT_NO_MESG
#endif

#if ( BLE_DBG_EDS_STM_EN != 0 )
#define BLE_DBG_EDS_STM_MSG         PRINT_MESG_DBG
#else
#define BLE_DBG_EDS_STM_MSG         PRINT_NO_MESG
#endif

#if ( BLE_DBG_LBS_STM_EN != 0 )
#define BLE_DBG_LBS_STM_MSG         PRINT_MESG_DBG
#else
#define BLE_DBG_LBS_STM_MSG         PRINT_NO_MESG
#endif

#if ( BLE_DBG_SVCCTL_EN != 0 )
#define BLE_DBG_SVCCTL_MSG          PRINT_MESG_DBG
#else
#define BLE_DBG_SVCCTL_MSG          PRINT_NO_MESG
#endif

#if (BLE_DBG_CTS_EN != 0)
#define BLE_DBG_CTS_MSG             PRINT_MESG_DBG
#else
#define BLE_DBG_CTS_MSG             PRINT_NO_MESG
#endif

#if (BLE_DBG_HIDS_EN != 0)
#define BLE_DBG_HIDS_MSG            PRINT_MESG_DBG
#else
#define BLE_DBG_HIDS_MSG            PRINT_NO_MESG
#endif

#if (BLE_DBG_PASS_EN != 0)
#define BLE_DBG_PASS_MSG            PRINT_MESG_DBG
#else
#define BLE_DBG_PASS_MSG            PRINT_NO_MESG
#endif

#if (BLE_DBG_BLS_EN != 0)
#define BLE_DBG_BLS_MSG             PRINT_MESG_DBG
#else
#define BLE_DBG_BLS_MSG             PRINT_NO_MESG
#endif

#if (BLE_DBG_HTS_EN != 0)
#define BLE_DBG_HTS_MSG             PRINT_MESG_DBG
#else
#define BLE_DBG_HTS_MSG             PRINT_NO_MESG
#endif

#if (BLE_DBG_ANS_EN != 0)
#define BLE_DBG_ANS_MSG             PRINT_MESG_DBG
#else
#define BLE_DBG_ANS_MSG             PRINT_NO_MESG
#endif

#if (BLE_DBG_ESS_EN != 0)
#define BLE_DBG_ESS_MSG             PRINT_MESG_DBG
#else
#define BLE_DBG_ESS_MSG             PRINT_NO_MESG
#endif

#if (BLE_DBG_GLS_EN != 0)
#define BLE_DBG_GLS_MSG             PRINT_MESG_DBG
#else
#define BLE_DBG_GLS_MSG             PRINT_NO_MESG
#endif

#if (BLE_DBG_BAS_EN != 0)
#define BLE_DBG_BAS_MSG             PRINT_MESG_DBG
#else
#define BLE_DBG_BAS_MSG             PRINT_NO_MESG
#endif

#if (BLE_DBG_RTUS_EN != 0)
#define BLE_DBG_RTUS_MSG            PRINT_MESG_DBG
#else
#define BLE_DBG_RTUS_MSG            PRINT_NO_MESG
#endif

#if (BLE_DBG_HPS_EN != 0)
#define BLE_DBG_HPS_MSG             PRINT_MESG_DBG
#else
#define BLE_DBG_HPS_MSG             PRINT_NO_MESG
#endif

#if (BLE_DBG_TPS_EN != 0)
#define BLE_DBG_TPS_MSG             PRINT_MESG_DBG
#else
#define BLE_DBG_TPS_MSG             PRINT_NO_MESG
#endif

#if (BLE_DBG_LLS_EN != 0)
#define BLE_DBG_LLS_MSG             PRINT_MESG_DBG
#else
#define BLE_DBG_LLS_MSG             PRINT_NO_MESG
#endif

#if (BLE_DBG_IAS_EN != 0)
#define BLE_DBG_IAS_MSG             PRINT_MESG_DBG
#else
#define BLE_DBG_IAS_MSG             PRINT_NO_MESG
#endif

#if (BLE_DBG_WSS_EN != 0)
#define BLE_DBG_WSS_MSG             PRINT_MESG_DBG
#else
#define BLE_DBG_WSS_MSG             PRINT_NO_MESG
#endif

#if (BLE_DBG_LNS_EN != 0)
#define BLE_DBG_LNS_MSG             PRINT_MESG_DBG
#else
#define BLE_DBG_LNS_MSG             PRINT_NO_MESG
#endif

#if (BLE_DBG_SCPS_EN != 0)
#define BLE_DBG_SCPS_MSG            PRINT_MESG_DBG
#else
#define BLE_DBG_SCPS_MSG            PRINT_NO_MESG
#endif

#if (BLE_DBG_DTS_EN != 0)
#define BLE_DBG_DTS_MSG             PRINT_MESG_DBG
#define BLE_DBG_DTS_BUF             PRINT_LOG_BUFF_DBG
#else
#define BLE_DBG_DTS_MSG             PRINT_NO_MESG
#define BLE_DBG_DTS_BUF             PRINT_NO_MESG
#endif

#endif /*__BLE_DBG_CONF_H */

/************************ (C) COPYRIGHT STMicroelectronics *****END OF FILE****/
