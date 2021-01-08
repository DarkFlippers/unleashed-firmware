#include <api-hal-flash.h>
#include <api-hal-bt.h>
#include <stm32wbxx.h>

bool api_hal_flash_erase(uint8_t page, uint8_t count) {
    api_hal_bt_lock_flash();
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
    api_hal_bt_lock_flash();
    HAL_StatusTypeDef status = HAL_FLASH_Program(FLASH_TYPEPROGRAM_DOUBLEWORD, address, data);
    api_hal_bt_unlock_flash();
    return status == HAL_OK;
}

bool api_hal_flash_write_row(size_t address, size_t source_address) {
    api_hal_bt_lock_flash();
    HAL_StatusTypeDef status = HAL_FLASH_Program(FLASH_TYPEPROGRAM_FAST, address, source_address);
    api_hal_bt_unlock_flash();
    return status == HAL_OK;
}
