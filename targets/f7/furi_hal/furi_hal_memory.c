#include <furi_hal.h>
#include <furi_hal_memory.h>
#include <furi_hal_rtc.h>

#define TAG "FuriHalMemory"

typedef enum {
    SRAM_A,
    SRAM_B,
    SRAM_MAX,
} SRAM;

typedef struct {
    void* start;
    uint32_t size;
} FuriHalMemoryRegion;

typedef struct {
    FuriHalMemoryRegion region[SRAM_MAX];
} FuriHalMemory;

static FuriHalMemory* furi_hal_memory = NULL;

extern const void __sram2a_start__;
extern const void __sram2a_free__;
extern const void __sram2b_start__;

void furi_hal_memory_init(void) {
    if(furi_hal_rtc_get_boot_mode() != FuriHalRtcBootModeNormal) {
        return;
    }

    FuriHalMemory* memory = malloc(sizeof(FuriHalMemory));

    uint32_t sbrsa = (FLASH->SRRVR & FLASH_SRRVR_SBRSA_Msk) >> FLASH_SRRVR_SBRSA_Pos;
    uint32_t snbrsa = (FLASH->SRRVR & FLASH_SRRVR_SNBRSA_Msk) >> FLASH_SRRVR_SNBRSA_Pos;

    // STM(TM) Copro(TM) bug(TM): SNBRSA is incorrect if stack version is higher than 1.13 and lower than 1.17.2+
    // Radio core started, but not yet ready, so we'll try to guess
    // This will be true only if BLE light radio stack used,
    // 0x0D is known to be incorrect, 0x0B is known to be correct since 1.17.2+
    // Lower value by 2 pages to match real memory layout
    if(snbrsa > 0x0B) {
        FURI_LOG_E(TAG, "SNBRSA workaround");
        snbrsa -= 2;
    }

    uint32_t sram2a_busy_size = (uint32_t)&__sram2a_free__ - (uint32_t)&__sram2a_start__;
    uint32_t sram2a_unprotected_size = (sbrsa) * 1024;
    uint32_t sram2b_unprotected_size = (snbrsa) * 1024;

    memory->region[SRAM_A].start = (uint8_t*)&__sram2a_free__;
    memory->region[SRAM_B].start = (uint8_t*)&__sram2b_start__;

    if(sram2a_unprotected_size > sram2a_busy_size) {
        memory->region[SRAM_A].size = sram2a_unprotected_size - sram2a_busy_size;
    } else {
        memory->region[SRAM_A].size = 0;
    }
    memory->region[SRAM_B].size = sram2b_unprotected_size;

    FURI_LOG_I(
        TAG, "SRAM2A: 0x%p, %lu", memory->region[SRAM_A].start, memory->region[SRAM_A].size);
    FURI_LOG_I(
        TAG, "SRAM2B: 0x%p, %lu", memory->region[SRAM_B].start, memory->region[SRAM_B].size);

    if((memory->region[SRAM_A].size > 0) || (memory->region[SRAM_B].size > 0)) {
        if(memory->region[SRAM_A].size > 0) {
            FURI_LOG_I(TAG, "SRAM2A clear");
            memset(memory->region[SRAM_A].start, 0, memory->region[SRAM_A].size);
        }
        if(memory->region[SRAM_B].size > 0) {
            FURI_LOG_I(TAG, "SRAM2B clear");
            memset(memory->region[SRAM_B].start, 0, memory->region[SRAM_B].size);
        }
        furi_hal_memory = memory;
        FURI_LOG_I(TAG, "Enabled");
    } else {
        free(memory);
        FURI_LOG_E(TAG, "No SRAM2 available");
    }
}

void* furi_hal_memory_alloc(size_t size) {
    if(FURI_IS_IRQ_MODE()) {
        furi_crash("memmgt in ISR");
    }

    if(furi_hal_memory == NULL) {
        return NULL;
    }

    void* allocated_memory = NULL;
    FURI_CRITICAL_ENTER();
    for(int i = 0; i < SRAM_MAX; i++) {
        if(furi_hal_memory->region[i].size >= size) {
            void* ptr = furi_hal_memory->region[i].start;
            furi_hal_memory->region[i].start += size;
            furi_hal_memory->region[i].size -= size;
            allocated_memory = ptr;
            break;
        }
    }
    FURI_CRITICAL_EXIT();

    return allocated_memory;
}

size_t furi_hal_memory_get_free(void) {
    if(furi_hal_memory == NULL) return 0;

    size_t free = 0;
    for(int i = 0; i < SRAM_MAX; i++) {
        free += furi_hal_memory->region[i].size;
    }
    return free;
}

size_t furi_hal_memory_max_pool_block(void) {
    if(furi_hal_memory == NULL) return 0;

    size_t max = 0;
    for(int i = 0; i < SRAM_MAX; i++) {
        if(furi_hal_memory->region[i].size > max) {
            max = furi_hal_memory->region[i].size;
        }
    }
    return max;
}
