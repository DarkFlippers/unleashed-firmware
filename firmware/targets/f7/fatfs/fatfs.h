/**
  ******************************************************************************
  * @file   fatfs.h
  * @brief  Header for fatfs applications
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

#pragma once

#include "fatfs/ff.h"
#include "fatfs/ff_gen_drv.h"
#include "user_diskio.h"

#ifdef __cplusplus
extern "C" {
#endif

/** File system object */
extern FATFS fatfs_object;

/** Init file system driver */
void fatfs_init(void);

#ifdef __cplusplus
}
#endif

/************************ (C) COPYRIGHT STMicroelectronics *****END OF FILE****/
