#include <furi_hal_flash.h>
#include <furi_hal_bt.h>
#include <furi.h>
#include <ble.h>
#include <shci.h>

#include <stm32wbxx.h>

#define FURI_HAL_TAG "FuriHalFlash"
#define FURI_HAL_CRITICAL_MSG "Critical flash operation fail"
#define FURI_HAL_FLASH_READ_BLOCK 8
#define FURI_HAL_FLASH_WRITE_BLOCK 8
#define FURI_HAL_FLASH_PAGE_SIZE 4096
#define FURI_HAL_FLASH_CYCLES_COUNT 10000

/* Free flash space borders, exported by linker */
extern const void __free_flash_start__;

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
    uint32_t sfr_reg_val = READ_REG(FLASH->SFR);
    uint32_t sfsa = (READ_BIT(sfr_reg_val, FLASH_SFR_SFSA) >> FLASH_SFR_SFSA_Pos);
    return (const void*)((sfsa * FLASH_PAGE_SIZE) + FLASH_BASE);
}

size_t furi_hal_flash_get_free_page_start_address() {
    size_t start = (size_t)furi_hal_flash_get_free_start_address();
    size_t page_start = start - start % FURI_HAL_FLASH_PAGE_SIZE;
    if(page_start != start) {
        page_start += FURI_HAL_FLASH_PAGE_SIZE;
    }
    return page_start;
}

size_t furi_hal_flash_get_free_page_count() {
    size_t end = (size_t)furi_hal_flash_get_free_end_address();
    size_t page_start = (size_t)furi_hal_flash_get_free_page_start_address();
    return (end - page_start) / FURI_HAL_FLASH_PAGE_SIZE;
}

static void furi_hal_flash_unlock() {
    /* verify Flash is locked */
    furi_check(READ_BIT(FLASH->CR, FLASH_CR_LOCK) != 0U);

    /* Authorize the FLASH Registers access */
    WRITE_REG(FLASH->KEYR, FLASH_KEY1);
    WRITE_REG(FLASH->KEYR, FLASH_KEY2);

    /* verify Flash is unlock */
    furi_check(READ_BIT(FLASH->CR, FLASH_CR_LOCK) == 0U);
}

static void furi_hal_flash_lock(void) {
    /* verify Flash is unlocked */
    furi_check(READ_BIT(FLASH->CR, FLASH_CR_LOCK) == 0U);

    /* Set the LOCK Bit to lock the FLASH Registers access */
    /* @Note  The lock and unlock procedure is done only using CR registers even from CPU2 */
    SET_BIT(FLASH->CR, FLASH_CR_LOCK);

    /* verify Flash is locked */
    furi_check(READ_BIT(FLASH->CR, FLASH_CR_LOCK) != 0U);
}

static void furi_hal_flash_begin_with_core2(bool erase_flag) {
    // Take flash controller ownership
    while(HAL_HSEM_FastTake(CFG_HW_FLASH_SEMID) != HAL_OK) {
        taskYIELD();
    }

    // Unlock flash operation
    furi_hal_flash_unlock();

    // Erase activity notification
    if(erase_flag) SHCI_C2_FLASH_EraseActivity(ERASE_ACTIVITY_ON);

    while(true) {
        // Wait till flash controller become usable
        while(LL_FLASH_IsActiveFlag_OperationSuspended()) {
            taskYIELD();
        };

        // Just a little more love
        taskENTER_CRITICAL();

        // Actually we already have mutex for it, but specification is specification
        if(HAL_HSEM_IsSemTaken(CFG_HW_BLOCK_FLASH_REQ_BY_CPU1_SEMID)) {
            taskEXIT_CRITICAL();
            continue;
        }

        // Take sempahopre and prevent core2 from anyting funky
        if(!HAL_HSEM_IsSemTaken(CFG_HW_BLOCK_FLASH_REQ_BY_CPU2_SEMID)) {
            if(HAL_HSEM_FastTake(CFG_HW_BLOCK_FLASH_REQ_BY_CPU2_SEMID) != HAL_OK) {
                taskEXIT_CRITICAL();
                continue;
            }
        }

        break;
    }
}

static void furi_hal_flash_begin(bool erase_flag) {
    // Acquire dangerous ops mutex
    furi_hal_bt_lock_core2();

    // If Core2 is running use IPC locking
    if(furi_hal_bt_is_alive()) {
        furi_hal_flash_begin_with_core2(erase_flag);
    } else {
        furi_hal_flash_unlock();
    }
}

static void furi_hal_flash_end_with_core2(bool erase_flag) {
    // Funky ops are ok at this point
    HAL_HSEM_Release(CFG_HW_BLOCK_FLASH_REQ_BY_CPU2_SEMID, 0);

    // Task switching is ok
    taskEXIT_CRITICAL();

    // Doesn't make much sense, does it?
    while(__HAL_FLASH_GET_FLAG(FLASH_FLAG_BSY)) {
        taskYIELD();
    }

    // Erase activity over, core2 can continue
    if(erase_flag) SHCI_C2_FLASH_EraseActivity(ERASE_ACTIVITY_OFF);

    // Lock flash controller
    furi_hal_flash_lock();

    // Release flash controller ownership
    HAL_HSEM_Release(CFG_HW_FLASH_SEMID, 0);
}

static void furi_hal_flash_end(bool erase_flag) {
    // If Core2 is running use IPC locking
    if(furi_hal_bt_is_alive()) {
        furi_hal_flash_end_with_core2(erase_flag);
    } else {
        furi_hal_flash_lock();
    }

    // Release dangerous ops mutex
    furi_hal_bt_unlock_core2();
}

static void furi_hal_flush_cache(void) {
    /* Flush instruction cache  */
    if(READ_BIT(FLASH->ACR, FLASH_ACR_ICEN) == FLASH_ACR_ICEN) {
        /* Disable instruction cache  */
        __HAL_FLASH_INSTRUCTION_CACHE_DISABLE();
        /* Reset instruction cache */
        __HAL_FLASH_INSTRUCTION_CACHE_RESET();
        /* Enable instruction cache */
        __HAL_FLASH_INSTRUCTION_CACHE_ENABLE();
    }

    /* Flush data cache */
    if(READ_BIT(FLASH->ACR, FLASH_ACR_DCEN) == FLASH_ACR_DCEN) {
        /* Disable data cache  */
        __HAL_FLASH_DATA_CACHE_DISABLE();
        /* Reset data cache */
        __HAL_FLASH_DATA_CACHE_RESET();
        /* Enable data cache */
        __HAL_FLASH_DATA_CACHE_ENABLE();
    }
}

HAL_StatusTypeDef furi_hal_flash_wait_last_operation(uint32_t timeout) {
    uint32_t error = 0;
    uint32_t countdown = 0;

    // Wait for the FLASH operation to complete by polling on BUSY flag to be reset.
    // Even if the FLASH operation fails, the BUSY flag will be reset and an error
    // flag will be set
    countdown = timeout;
    while(__HAL_FLASH_GET_FLAG(FLASH_FLAG_BSY)) {
        if(LL_SYSTICK_IsActiveCounterFlag()) {
            countdown--;
        }
        if(countdown == 0) {
            return HAL_TIMEOUT;
        }
    }

    /* Check FLASH operation error flags */
    error = FLASH->SR;

    /* Check FLASH End of Operation flag */
    if((error & FLASH_FLAG_EOP) != 0U) {
        /* Clear FLASH End of Operation pending bit */
        __HAL_FLASH_CLEAR_FLAG(FLASH_FLAG_EOP);
    }

    /* Now update error variable to only error value */
    error &= FLASH_FLAG_SR_ERRORS;

    furi_check(error == 0);

    /* clear error flags */
    __HAL_FLASH_CLEAR_FLAG(error);

    /* Wait for control register to be written */
    countdown = timeout;
    while(__HAL_FLASH_GET_FLAG(FLASH_FLAG_CFGBSY)) {
        if(LL_SYSTICK_IsActiveCounterFlag()) {
            countdown--;
        }
        if(countdown == 0) {
            return HAL_TIMEOUT;
        }
    }

    return HAL_OK;
}

bool furi_hal_flash_erase(uint8_t page) {
    furi_hal_flash_begin(true);

    // Ensure that controller state is valid
    furi_check(FLASH->SR == 0);

    /* Verify that next operation can be proceed */
    furi_check(furi_hal_flash_wait_last_operation(FLASH_TIMEOUT_VALUE) == HAL_OK);

    /* Select page and start operation */
    MODIFY_REG(
        FLASH->CR, FLASH_CR_PNB, ((page << FLASH_CR_PNB_Pos) | FLASH_CR_PER | FLASH_CR_STRT));

    /* Wait for last operation to be completed */
    furi_check(furi_hal_flash_wait_last_operation(FLASH_TIMEOUT_VALUE) == HAL_OK);

    /* If operation is completed or interrupted, disable the Page Erase Bit */
    CLEAR_BIT(FLASH->CR, (FLASH_CR_PER | FLASH_CR_PNB));

    /* Flush the caches to be sure of the data consistency */
    furi_hal_flush_cache();

    furi_hal_flash_end(true);

    return true;
}

bool furi_hal_flash_write_dword(size_t address, uint64_t data) {
    furi_hal_flash_begin(false);

    // Ensure that controller state is valid
    furi_check(FLASH->SR == 0);

    /* Check the parameters */
    furi_check(IS_ADDR_ALIGNED_64BITS(address));
    furi_check(IS_FLASH_PROGRAM_ADDRESS(address));

    /* Set PG bit */
    SET_BIT(FLASH->CR, FLASH_CR_PG);

    /* Program first word */
    *(uint32_t*)address = (uint32_t)data;

    // Barrier to ensure programming is performed in 2 steps, in right order
    // (independently of compiler optimization behavior)
    __ISB();

    /* Program second word */
    *(uint32_t*)(address + 4U) = (uint32_t)(data >> 32U);

    /* Wait for last operation to be completed */
    furi_check(furi_hal_flash_wait_last_operation(FLASH_TIMEOUT_VALUE) == HAL_OK);

    /* If the program operation is completed, disable the PG or FSTPG Bit */
    CLEAR_BIT(FLASH->CR, FLASH_CR_PG);

    furi_hal_flash_end(false);

    return true;
}
