#include <furi-hal-flash.h>
#include <furi-hal-bt.h>
#include <furi.h>

#include <stm32wbxx.h>

/* Free flash space borders, exported by linker */
extern const void __free_flash_start__;

#define FURI_HAL_TAG "FuriHalFlash"
#define FURI_HAL_CRITICAL_MSG "Critical flash operation fail"
#define FURI_HAL_FLASH_READ_BLOCK 8
#define FURI_HAL_FLASH_WRITE_BLOCK 8
#define FURI_HAL_FLASH_PAGE_SIZE 4096
#define FURI_HAL_FLASH_CYCLES_COUNT 10000

size_t furi_hal_flash_get_base() {
    return FLASH_BASE;
}

size_t furi_hal_flash_get_read_block_size() {
    return FURI_HAL_FLASH_READ_BLOCK;
}

size_t furi_hal_flash_get_write_block_size() {
    return FURI_HAL_FLASH_WRITE_BLOCK;
}

size_t furi_hal_flash_get_page_size() {
    return FURI_HAL_FLASH_PAGE_SIZE;
}

size_t furi_hal_flash_get_cycles_count() {
    return FURI_HAL_FLASH_CYCLES_COUNT;
}

const void* furi_hal_flash_get_free_start_address() {
    return &__free_flash_start__;
}

const void* furi_hal_flash_get_free_end_address() {
    FLASH_OBProgramInitTypeDef pOBInit;
    HAL_FLASHEx_OBGetConfig(&pOBInit);
    return (const void *)pOBInit.SecureFlashStartAddr;
}

size_t furi_hal_flash_get_free_page_start_address() {
    size_t start = (size_t)furi_hal_flash_get_free_start_address();
    size_t page_start = start - start % FURI_HAL_FLASH_PAGE_SIZE;
    if (page_start != start) {
        page_start += FURI_HAL_FLASH_PAGE_SIZE;
    }
    return page_start;
}

size_t furi_hal_flash_get_free_page_count() {
    size_t end = (size_t)furi_hal_flash_get_free_end_address();
    size_t page_start = (size_t)furi_hal_flash_get_free_page_start_address();
    return (end-page_start) / FURI_HAL_FLASH_PAGE_SIZE;
}

bool furi_hal_flash_erase(uint8_t page, uint8_t count) {
    furi_hal_bt_lock_flash(true);

    FLASH_EraseInitTypeDef erase;
    erase.TypeErase = FLASH_TYPEERASE_PAGES;
    erase.Page = page;
    erase.NbPages = count;

    uint32_t error_page = 0;
    HAL_StatusTypeDef status = HAL_FLASHEx_Erase(&erase, &error_page);
    if (status != HAL_OK) {
        FURI_LOG_E(FURI_HAL_TAG, "Erase failed, ret: %d, page: %d", status, error_page);
        furi_crash(FURI_HAL_CRITICAL_MSG);
    }

    furi_hal_bt_unlock_flash(true);

    return true;
}

bool furi_hal_flash_write_dword(size_t address, uint64_t data) {
    furi_hal_bt_lock_flash(false);

    HAL_StatusTypeDef status = HAL_FLASH_Program(FLASH_TYPEPROGRAM_DOUBLEWORD, address, data);
    if (status != HAL_OK) {
        FURI_LOG_E(FURI_HAL_TAG, "Programming failed, ret: %d, address: %p", status, address);
        furi_crash(FURI_HAL_CRITICAL_MSG);
    }

    furi_hal_bt_unlock_flash(false);

    return true;
}

bool furi_hal_flash_write_row(size_t address, size_t source_address) {
    furi_hal_bt_lock_flash(false);

    HAL_StatusTypeDef status = HAL_FLASH_Program(FLASH_TYPEPROGRAM_FAST, address, source_address);
    furi_check(status == HAL_OK);

    furi_hal_bt_unlock_flash(false);

    return true;
}
