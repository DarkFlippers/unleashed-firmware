#include "user_diskio.h"
#include <furi_hal.h>
#include "sector_cache.h"

static volatile DSTATUS Stat = STA_NOINIT;

static DSTATUS driver_check_status(BYTE lun) {
    UNUSED(lun);
    Stat = STA_NOINIT;
    if(sd_get_card_state() == SdSpiStatusOK) {
        Stat &= ~STA_NOINIT;
    }

    return Stat;
}

static DSTATUS driver_initialize(BYTE pdrv);
static DSTATUS driver_status(BYTE pdrv);
static DRESULT driver_read(BYTE pdrv, BYTE* buff, DWORD sector, UINT count);
static DRESULT driver_write(BYTE pdrv, const BYTE* buff, DWORD sector, UINT count);
static DRESULT driver_ioctl(BYTE pdrv, BYTE cmd, void* buff);

Diskio_drvTypeDef sd_fatfs_driver = {
    driver_initialize,
    driver_status,
    driver_read,
    driver_write,
    driver_ioctl,
};

static inline bool sd_cache_get(uint32_t address, uint32_t* data) {
    uint8_t* cached_data = sector_cache_get(address);
    if(cached_data) {
        memcpy(data, cached_data, SD_BLOCK_SIZE);
        return true;
    }
    return false;
}

static inline void sd_cache_put(uint32_t address, uint32_t* data) {
    sector_cache_put(address, (uint8_t*)data);
}

static inline void sd_cache_invalidate_range(uint32_t start_sector, uint32_t end_sector) {
    sector_cache_invalidate_range(start_sector, end_sector);
}

static inline void sd_cache_invalidate_all() {
    sector_cache_init();
}

static bool sd_device_read(uint32_t* buff, uint32_t sector, uint32_t count) {
    bool result = false;

    furi_hal_spi_acquire(&furi_hal_spi_bus_handle_sd_fast);
    furi_hal_sd_spi_handle = &furi_hal_spi_bus_handle_sd_fast;

    if(sd_read_blocks(buff, sector, count, SD_TIMEOUT_MS) == SdSpiStatusOK) {
        FuriHalCortexTimer timer = furi_hal_cortex_timer_get(SD_TIMEOUT_MS * 1000);

        /* wait until the read operation is finished */
        result = true;
        while(sd_get_card_state() != SdSpiStatusOK) {
            if(furi_hal_cortex_timer_is_expired(timer)) {
                result = false;
                break;
            }
        }
    }

    furi_hal_sd_spi_handle = NULL;
    furi_hal_spi_release(&furi_hal_spi_bus_handle_sd_fast);

    return result;
}

static bool sd_device_write(uint32_t* buff, uint32_t sector, uint32_t count) {
    bool result = false;

    furi_hal_spi_acquire(&furi_hal_spi_bus_handle_sd_fast);
    furi_hal_sd_spi_handle = &furi_hal_spi_bus_handle_sd_fast;

    if(sd_write_blocks(buff, sector, count, SD_TIMEOUT_MS) == SdSpiStatusOK) {
        FuriHalCortexTimer timer = furi_hal_cortex_timer_get(SD_TIMEOUT_MS * 1000);

        /* wait until the Write operation is finished */
        result = true;
        while(sd_get_card_state() != SdSpiStatusOK) {
            if(furi_hal_cortex_timer_is_expired(timer)) {
                sd_cache_invalidate_all();

                result = false;
                break;
            }
        }
    }

    furi_hal_sd_spi_handle = NULL;
    furi_hal_spi_release(&furi_hal_spi_bus_handle_sd_fast);

    return result;
}

/**
  * @brief  Initializes a Drive
  * @param  pdrv: Physical drive number (0..)
  * @retval DSTATUS: Operation status
  */
static DSTATUS driver_initialize(BYTE pdrv) {
    furi_hal_spi_acquire(&furi_hal_spi_bus_handle_sd_fast);
    furi_hal_sd_spi_handle = &furi_hal_spi_bus_handle_sd_fast;

    DSTATUS status = driver_check_status(pdrv);

    furi_hal_sd_spi_handle = NULL;
    furi_hal_spi_release(&furi_hal_spi_bus_handle_sd_fast);

    return status;
}

/**
  * @brief  Gets Disk Status 
  * @param  pdrv: Physical drive number (0..)
  * @retval DSTATUS: Operation status
  */
static DSTATUS driver_status(BYTE pdrv) {
    UNUSED(pdrv);
    return Stat;
}

/**
  * @brief  Reads Sector(s) 
  * @param  pdrv: Physical drive number (0..)
  * @param  *buff: Data buffer to store read data
  * @param  sector: Sector address (LBA)
  * @param  count: Number of sectors to read (1..128)
  * @retval DRESULT: Operation result
  */
static DRESULT driver_read(BYTE pdrv, BYTE* buff, DWORD sector, UINT count) {
    UNUSED(pdrv);

    bool result;
    bool single_sector = count == 1;

    if(single_sector) {
        if(sd_cache_get(sector, (uint32_t*)buff)) {
            return RES_OK;
        }
    }

    result = sd_device_read((uint32_t*)buff, (uint32_t)(sector), count);

    if(!result) {
        uint8_t counter = sd_max_mount_retry_count();

        while(result == false && counter > 0 && hal_sd_detect()) {
            SdSpiStatus status;

            if((counter % 2) == 0) {
                // power reset sd card
                status = sd_init(true);
            } else {
                status = sd_init(false);
            }

            if(status == SdSpiStatusOK) {
                result = sd_device_read((uint32_t*)buff, (uint32_t)(sector), count);
            }
            counter--;
        }
    }

    if(single_sector && result == true) {
        sd_cache_put(sector, (uint32_t*)buff);
    }

    return result ? RES_OK : RES_ERROR;
}

/**
  * @brief  Writes Sector(s)  
  * @param  pdrv: Physical drive number (0..)
  * @param  *buff: Data to be written
  * @param  sector: Sector address (LBA)
  * @param  count: Number of sectors to write (1..128)
  * @retval DRESULT: Operation result
  */
static DRESULT driver_write(BYTE pdrv, const BYTE* buff, DWORD sector, UINT count) {
    UNUSED(pdrv);
    bool result;

    sd_cache_invalidate_range(sector, sector + count);

    result = sd_device_write((uint32_t*)buff, (uint32_t)(sector), count);

    if(!result) {
        uint8_t counter = sd_max_mount_retry_count();

        while(result == false && counter > 0 && hal_sd_detect()) {
            SdSpiStatus status;

            if((counter % 2) == 0) {
                // power reset sd card
                status = sd_init(true);
            } else {
                status = sd_init(false);
            }

            if(status == SdSpiStatusOK) {
                result = sd_device_write((uint32_t*)buff, (uint32_t)(sector), count);
            }
            counter--;
        }
    }

    return result ? RES_OK : RES_ERROR;
}

/**
  * @brief  I/O control operation  
  * @param  pdrv: Physical drive number (0..)
  * @param  cmd: Control code
  * @param  *buff: Buffer to send/receive control data
  * @retval DRESULT: Operation result
  */
static DRESULT driver_ioctl(BYTE pdrv, BYTE cmd, void* buff) {
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
}
