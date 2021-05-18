#include <api-hal-flash.h>
#include <api-hal-bt.h>
#include <stm32wbxx.h>
#include <furi.h>

/* Free flash space borders, exported by linker */
extern const void __free_flash_start__;
extern const void __free_flash_end__;

#define API_HAL_FLASH_READ_BLOCK 8
#define API_HAL_FLASH_WRITE_BLOCK 8
#define API_HAL_FLASH_PAGE_SIZE 4096
#define API_HAL_FLASH_CYCLES_COUNT 10000

size_t api_hal_flash_get_base() {
    return FLASH_BASE;
}

size_t api_hal_flash_get_read_block_size() {
    return API_HAL_FLASH_READ_BLOCK;
}

size_t api_hal_flash_get_write_block_size() {
    return API_HAL_FLASH_WRITE_BLOCK;
}

size_t api_hal_flash_get_page_size() {
    return API_HAL_FLASH_PAGE_SIZE;
}

size_t api_hal_flash_get_cycles_count() {
    return API_HAL_FLASH_CYCLES_COUNT;
}

const void* api_hal_flash_get_free_start_address() {
    return &__free_flash_start__;
}

const void* api_hal_flash_get_free_end_address() {
    return &__free_flash_end__;
}

size_t api_hal_flash_get_free_page_start_address() {
    size_t start = (size_t)api_hal_flash_get_free_start_address();
    size_t page_start = start - start % API_HAL_FLASH_PAGE_SIZE;
    if (page_start != start) {
        page_start += API_HAL_FLASH_PAGE_SIZE;
    }
    return page_start;
}

size_t api_hal_flash_get_free_page_count() {
    size_t end = (size_t)api_hal_flash_get_free_end_address();
    size_t page_start = (size_t)api_hal_flash_get_free_page_start_address();
    return (end-page_start) / API_HAL_FLASH_PAGE_SIZE;
}

bool api_hal_flash_erase(uint8_t page, uint8_t count) {
    if (!api_hal_bt_lock_flash()) {
        return false;
    }
    FLASH_EraseInitTypeDef erase;
    erase.TypeErase = FLASH_TYPEERASE_PAGES;
    erase.Page = page;
    erase.NbPages = count;
    uint32_t error;
    HAL_StatusTypeDef status = HAL_FLASHEx_Erase(&erase, &error);
    api_hal_bt_unlock_flash();
    return status == HAL_OK;
}

bool api_hal_flash_write_dword(size_t address, uint64_t data) {
    if (!api_hal_bt_lock_flash()) {
        return false;
    }
    HAL_StatusTypeDef status = HAL_FLASH_Program(FLASH_TYPEPROGRAM_DOUBLEWORD, address, data);
    api_hal_bt_unlock_flash();
    return status == HAL_OK;
}

bool api_hal_flash_write_dword_from(size_t address, size_t source_address) {
    if (!api_hal_bt_lock_flash()) {
        return false;
    }
    HAL_StatusTypeDef status = HAL_FLASH_Program(FLASH_TYPEPROGRAM_FAST, address, source_address);
    api_hal_bt_unlock_flash();
    return status == HAL_OK;
}
