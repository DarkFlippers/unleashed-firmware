/**
  ******************************************************************************
  * @file   fatfs.c
  * @brief  Code for fatfs applications
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

#include "fatfs.h"

/** logical drive path */
char fatfs_path[4];
/** File system object */
FATFS fatfs_object;

void fatfs_init(void) {
    FATFS_LinkDriver(&sd_fatfs_driver, fatfs_path);
}

/**
  * @brief  Gets Time from RTC 
  * @param  None
  * @retval Time in DWORD
  */
DWORD get_fattime(void) {
    return 0;
}

/************************ (C) COPYRIGHT STMicroelectronics *****END OF FILE****/
