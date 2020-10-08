/**
  ******************************************************************************
  * @file    ff_gen_drv.h
  * @author  MCD Application Team
  * @brief   Header for ff_gen_drv.c module.
  *****************************************************************************
  * @attention
  *
  * Copyright (c) 2017 STMicroelectronics. All rights reserved.
  *
  * This software component is licensed by ST under BSD 3-Clause license,
  * the "License"; You may not use this file except in compliance with the
  * License. You may obtain a copy of the License at:
  *                       opensource.org/licenses/BSD-3-Clause
  *
  ******************************************************************************
**/

/* Define to prevent recursive inclusion -------------------------------------*/
#ifndef __FF_GEN_DRV_H
#define __FF_GEN_DRV_H

#ifdef __cplusplus
 extern "C" {
#endif

/* Includes ------------------------------------------------------------------*/
#include "diskio.h"
#include "ff.h"
#include "stdint.h"


/* Exported types ------------------------------------------------------------*/

/**
  * @brief  Disk IO Driver structure definition
  */
typedef struct
{
  DSTATUS (*disk_initialize) (BYTE);                     /*!< Initialize Disk Drive                     */
  DSTATUS (*disk_status)     (BYTE);                     /*!< Get Disk Status                           */
  DRESULT (*disk_read)       (BYTE, BYTE*, DWORD, UINT);       /*!< Read Sector(s)                            */
#if _USE_WRITE == 1
  DRESULT (*disk_write)      (BYTE, const BYTE*, DWORD, UINT); /*!< Write Sector(s) when _USE_WRITE = 0       */
#endif /* _USE_WRITE == 1 */
#if _USE_IOCTL == 1
  DRESULT (*disk_ioctl)      (BYTE, BYTE, void*);              /*!< I/O control operation when _USE_IOCTL = 1 */
#endif /* _USE_IOCTL == 1 */

}Diskio_drvTypeDef;

/**
  * @brief  Global Disk IO Drivers structure definition
  */
typedef struct
{
  uint8_t                 is_initialized[_VOLUMES];
  const Diskio_drvTypeDef *drv[_VOLUMES];
  uint8_t                 lun[_VOLUMES];
  volatile uint8_t        nbr;

}Disk_drvTypeDef;

/* Exported constants --------------------------------------------------------*/
/* Exported macro ------------------------------------------------------------*/
/* Exported functions ------------------------------------------------------- */
uint8_t FATFS_LinkDriver(const Diskio_drvTypeDef *drv, char *path);
uint8_t FATFS_UnLinkDriver(char *path);
uint8_t FATFS_LinkDriverEx(const Diskio_drvTypeDef *drv, char *path, BYTE lun);
uint8_t FATFS_UnLinkDriverEx(char *path, BYTE lun);
uint8_t FATFS_GetAttachedDriversNbr(void);

#ifdef __cplusplus
}
#endif

#endif /* __FF_GEN_DRV_H */

/************************ (C) COPYRIGHT STMicroelectronics *****END OF FILE****/

