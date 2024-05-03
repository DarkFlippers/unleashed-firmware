#pragma once
/**
 * @file furi_hal_sd.h
 * SD Card HAL API
 */

#include <furi.h>

#ifdef __cplusplus
extern "C" {
#endif

typedef struct {
    uint64_t capacity; /*!< total capacity in bytes */
    uint32_t block_size; /*!< block size */
    uint32_t logical_block_count; /*!< logical capacity in blocks */
    uint32_t logical_block_size; /*!< logical block size in bytes */

    uint8_t manufacturer_id; /*!< manufacturer ID */
    char oem_id[3]; /*!< OEM ID, 2 characters + null terminator */
    char product_name[6]; /*!< product name, 5 characters + null terminator */
    uint8_t product_revision_major; /*!< product revision major */
    uint8_t product_revision_minor; /*!< product revision minor */
    uint32_t product_serial_number; /*!< product serial number */
    uint8_t manufacturing_month; /*!< manufacturing month */
    uint16_t manufacturing_year; /*!< manufacturing year */
} FuriHalSdInfo;

/** 
 * @brief Init SD card presence detection
 */
void furi_hal_sd_presence_init(void);

/** 
 * @brief Get SD card status
 * @return true if SD card is present
 */
bool furi_hal_sd_is_present(void);

/**
 * @brief SD card max mount retry count
 * @return uint8_t 
 */
uint8_t furi_hal_sd_max_mount_retry_count(void);

/**
 * @brief Init SD card
 * @param power_reset reset card power
 * @return FuriStatus 
 */
FuriStatus furi_hal_sd_init(bool power_reset);

/**
 * @brief Read blocks from SD card
 * @param buff 
 * @param sector 
 * @param count 
 * @return FuriStatus 
 */
FuriStatus furi_hal_sd_read_blocks(uint32_t* buff, uint32_t sector, uint32_t count);

/**
 * @brief Write blocks to SD card
 * @param buff 
 * @param sector 
 * @param count 
 * @return FuriStatus 
 */
FuriStatus furi_hal_sd_write_blocks(const uint32_t* buff, uint32_t sector, uint32_t count);

/**
 * @brief Get SD card info
 * @param info 
 * @return FuriStatus 
 */
FuriStatus furi_hal_sd_info(FuriHalSdInfo* info);

/**
 * @brief Get SD card state
 * @return FuriStatus 
 */
FuriStatus furi_hal_sd_get_card_state(void);

#ifdef __cplusplus
}
#endif
