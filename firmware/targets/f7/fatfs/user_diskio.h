/* USER CODE BEGIN Header */
/**
 ******************************************************************************
  * @file    user_diskio.h
  * @brief   This file contains the common defines and functions prototypes for  
  *          the user_diskio driver.
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
/* USER CODE END Header */

/* Define to prevent recursive inclusion -------------------------------------*/
#ifndef __USER_DISKIO_H
#define __USER_DISKIO_H

#ifdef __cplusplus
extern "C" {
#endif

/* USER CODE BEGIN 0 */

/* Includes ------------------------------------------------------------------*/
#include "sd_spi_io.h"
#include "fatfs/ff_gen_drv.h"
/* Exported types ------------------------------------------------------------*/
/* Exported constants --------------------------------------------------------*/
/* Exported functions ------------------------------------------------------- */
extern Diskio_drvTypeDef USER_Driver;

/* USER CODE END 0 */

#ifdef __cplusplus
}
#endif

#endif /* __USER_DISKIO_H */

/************************ (C) COPYRIGHT STMicroelectronics *****END OF FILE****/
