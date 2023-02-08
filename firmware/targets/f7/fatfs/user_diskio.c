/* USER CODE BEGIN Header */
/**
 ******************************************************************************
 * @file    user_diskio.c
 * @brief   This file includes a diskio driver skeleton to be completed by the user.
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

#ifdef USE_OBSOLETE_USER_CODE_SECTION_0
/* 
 * Warning: the user section 0 is no more in use (starting from CubeMx version 4.16.0)
 * To be suppressed in the future. 
 * Kept to ensure backward compatibility with previous CubeMx versions when 
 * migrating projects. 
 * User code previously added there should be copied in the new user sections before 
 * the section contents can be deleted.
 */
/* USER CODE BEGIN 0 */
/* USER CODE END 0 */
#endif

/* USER CODE BEGIN DECL */

/* Includes ------------------------------------------------------------------*/
#include "user_diskio.h"
#include <furi_hal.h>
/* Private typedef -----------------------------------------------------------*/
/* Private define ------------------------------------------------------------*/

/* Private variables ---------------------------------------------------------*/
/* Disk status */
static volatile DSTATUS Stat = STA_NOINIT;

static DSTATUS User_CheckStatus(BYTE lun) {
    UNUSED(lun);
    Stat = STA_NOINIT;
    if(sd_get_card_state() == SdSpiStatusOK) {
        Stat &= ~STA_NOINIT;
    }

    return Stat;
}

/* USER CODE END DECL */

/* Private function prototypes -----------------------------------------------*/
DSTATUS USER_initialize(BYTE pdrv);
DSTATUS USER_status(BYTE pdrv);
DRESULT USER_read(BYTE pdrv, BYTE* buff, DWORD sector, UINT count);
#if _USE_WRITE == 1
DRESULT USER_write(BYTE pdrv, const BYTE* buff, DWORD sector, UINT count);
#endif /* _USE_WRITE == 1 */
#if _USE_IOCTL == 1
DRESULT USER_ioctl(BYTE pdrv, BYTE cmd, void* buff);
#endif /* _USE_IOCTL == 1 */

Diskio_drvTypeDef USER_Driver = {
    USER_initialize,
    USER_status,
    USER_read,
#if _USE_WRITE
    USER_write,
#endif /* _USE_WRITE == 1 */
#if _USE_IOCTL == 1
    USER_ioctl,
#endif /* _USE_IOCTL == 1 */
};

/* Private functions ---------------------------------------------------------*/

/**
  * @brief  Initializes a Drive
  * @param  pdrv: Physical drive number (0..)
  * @retval DSTATUS: Operation status
  */
DSTATUS USER_initialize(BYTE pdrv) {
    /* USER CODE BEGIN INIT */

    furi_hal_spi_acquire(&furi_hal_spi_bus_handle_sd_fast);
    furi_hal_sd_spi_handle = &furi_hal_spi_bus_handle_sd_fast;

    DSTATUS status = User_CheckStatus(pdrv);

    furi_hal_sd_spi_handle = NULL;
    furi_hal_spi_release(&furi_hal_spi_bus_handle_sd_fast);

    return status;
    /* USER CODE END INIT */
}

/**
  * @brief  Gets Disk Status 
  * @param  pdrv: Physical drive number (0..)
  * @retval DSTATUS: Operation status
  */
DSTATUS USER_status(BYTE pdrv) {
    /* USER CODE BEGIN STATUS */
    UNUSED(pdrv);
    return Stat;
    /* USER CODE END STATUS */
}

/**
  * @brief  Reads Sector(s) 
  * @param  pdrv: Physical drive number (0..)
  * @param  *buff: Data buffer to store read data
  * @param  sector: Sector address (LBA)
  * @param  count: Number of sectors to read (1..128)
  * @retval DRESULT: Operation result
  */
DRESULT USER_read(BYTE pdrv, BYTE* buff, DWORD sector, UINT count) {
    /* USER CODE BEGIN READ */
    UNUSED(pdrv);
    DRESULT res = RES_ERROR;

    furi_hal_spi_acquire(&furi_hal_spi_bus_handle_sd_fast);
    furi_hal_sd_spi_handle = &furi_hal_spi_bus_handle_sd_fast;

    if(sd_read_blocks((uint32_t*)buff, (uint32_t)(sector), count, SD_TIMEOUT_MS) ==
       SdSpiStatusOK) {
        FuriHalCortexTimer timer = furi_hal_cortex_timer_get(SD_TIMEOUT_MS * 1000);

        /* wait until the read operation is finished */
        res = RES_OK;
        while(sd_get_card_state() != SdSpiStatusOK) {
            if(furi_hal_cortex_timer_is_expired(timer)) {
                res = RES_ERROR;
                break;
            }
        }
    }

    furi_hal_sd_spi_handle = NULL;
    furi_hal_spi_release(&furi_hal_spi_bus_handle_sd_fast);

    return res;
    /* USER CODE END READ */
}

/**
  * @brief  Writes Sector(s)  
  * @param  pdrv: Physical drive number (0..)
  * @param  *buff: Data to be written
  * @param  sector: Sector address (LBA)
  * @param  count: Number of sectors to write (1..128)
  * @retval DRESULT: Operation result
  */
#if _USE_WRITE == 1
DRESULT USER_write(BYTE pdrv, const BYTE* buff, DWORD sector, UINT count) {
    /* USER CODE BEGIN WRITE */
    /* USER CODE HERE */
    UNUSED(pdrv);
    DRESULT res = RES_ERROR;

    furi_hal_spi_acquire(&furi_hal_spi_bus_handle_sd_fast);
    furi_hal_sd_spi_handle = &furi_hal_spi_bus_handle_sd_fast;

    if(sd_write_blocks((uint32_t*)buff, (uint32_t)(sector), count, SD_TIMEOUT_MS) ==
       SdSpiStatusOK) {
        FuriHalCortexTimer timer = furi_hal_cortex_timer_get(SD_TIMEOUT_MS * 1000);

        /* wait until the Write operation is finished */
        res = RES_OK;
        while(sd_get_card_state() != SdSpiStatusOK) {
            if(furi_hal_cortex_timer_is_expired(timer)) {
                res = RES_ERROR;
                break;
            }
        }
    }

    furi_hal_sd_spi_handle = NULL;
    furi_hal_spi_release(&furi_hal_spi_bus_handle_sd_fast);

    return res;
    /* USER CODE END WRITE */
}
#endif /* _USE_WRITE == 1 */

/**
  * @brief  I/O control operation  
  * @param  pdrv: Physical drive number (0..)
  * @param  cmd: Control code
  * @param  *buff: Buffer to send/receive control data
  * @retval DRESULT: Operation result
  */
#if _USE_IOCTL == 1
DRESULT USER_ioctl(BYTE pdrv, BYTE cmd, void* buff) {
    /* USER CODE BEGIN IOCTL */
    UNUSED(pdrv);
    DRESULT res = RES_ERROR;
    SD_CardInfo CardInfo;

    if(Stat & STA_NOINIT) return RES_NOTRDY;

    furi_hal_spi_acquire(&furi_hal_spi_bus_handle_sd_fast);
    furi_hal_sd_spi_handle = &furi_hal_spi_bus_handle_sd_fast;

    switch(cmd) {
    /* Make sure that no pending write process */
    case CTRL_SYNC:
        res = RES_OK;
        break;

    /* Get number of sectors on the disk (DWORD) */
    case GET_SECTOR_COUNT:
        sd_get_card_info(&CardInfo);
        *(DWORD*)buff = CardInfo.LogBlockNbr;
        res = RES_OK;
        break;

    /* Get R/W sector size (WORD) */
    case GET_SECTOR_SIZE:
        sd_get_card_info(&CardInfo);
        *(WORD*)buff = CardInfo.LogBlockSize;
        res = RES_OK;
        break;

    /* Get erase block size in unit of sector (DWORD) */
    case GET_BLOCK_SIZE:
        sd_get_card_info(&CardInfo);
        *(DWORD*)buff = CardInfo.LogBlockSize;
        res = RES_OK;
        break;

    default:
        res = RES_PARERR;
    }

    furi_hal_sd_spi_handle = NULL;
    furi_hal_spi_release(&furi_hal_spi_bus_handle_sd_fast);

    return res;
    /* USER CODE END IOCTL */
}
#endif /* _USE_IOCTL == 1 */

/************************ (C) COPYRIGHT STMicroelectronics *****END OF FILE****/
