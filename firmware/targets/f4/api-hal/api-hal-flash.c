#include <api-hal-flash.h>
#include <api-hal-bt.h>
#include <stm32wbxx.h>

void api_hal_flash_write_dword(size_t address, uint64_t data) {
    api_hal_bt_lock_flash();
    HAL_FLASH_Program(FLASH_TYPEPROGRAM_DOUBLEWORD, address, data);
    api_hal_bt_unlock_flash();
}

void api_hal_flash_write_row(size_t address, size_t source_address) {
    api_hal_bt_lock_flash();
    HAL_FLASH_Program(FLASH_TYPEPROGRAM_DOUBLEWORD, address, source_address);
    api_hal_bt_unlock_flash();
}
