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

void furi_hal_memory_init() {
    if(furi_hal_rtc_get_boot_mode() != FuriHalRtcBootModeNormal) {
        return;
    }

    if(!ble_glue_wait_for_c2_start(FURI_HAL_BT_C2_START_TIMEOUT)) {
        FURI_LOG_E(TAG, "C2 start timeout");
        return;
    }

    FuriHalMemory* memory = malloc(sizeof(FuriHalMemory));

    const BleGlueC2Info* c2_ver = ble_glue_get_c2_info();

    if(c2_ver->mode == BleGlueC2ModeStack) {
        uint32_t sram2a_busy_size = (uint32_t)&__sram2a_free__ - (uint32_t)&__sram2a_start__;
        uint32_t sram2a_unprotected_size = (32 - c2_ver->MemorySizeSram2A) * 1024;
        uint32_t sram2b_unprotected_size = (32 - c2_ver->MemorySizeSram2B) * 1024;

        memory->region[SRAM_A].start = (uint8_t*)&__sram2a_free__;
        memory->region[SRAM_B].start = (uint8_t*)&__sram2b_start__;

        if(sram2a_unprotected_size > sram2a_busy_size) {
            memory->region[SRAM_A].size = sram2a_unprotected_size - sram2a_busy_size;
        } else {
            memory->region[SRAM_A].size = 0;
        }
        memory->region[SRAM_B].size = sram2b_unprotected_size;

        FURI_LOG_I(
            TAG, "SRAM2A: 0x%p, %d", memory->region[SRAM_A].start, memory->region[SRAM_A].size);
        FURI_LOG_I(
            TAG, "SRAM2B: 0x%p, %d", memory->region[SRAM_B].start, memory->region[SRAM_B].size);

        if((memory->region[SRAM_A].size > 0) || (memory->region[SRAM_B].size > 0)) {
            if((memory->region[SRAM_A].size > 0)) {
                FURI_LOG_I(TAG, "SRAM2A clear");
                memset(memory->region[SRAM_A].start, 0, memory->region[SRAM_A].size);
            }
            if((memory->region[SRAM_B].size > 0)) {
                FURI_LOG_I(TAG, "SRAM2B clear");
                memset(memory->region[SRAM_B].start, 0, memory->region[SRAM_B].size);
            }
            furi_hal_memory = memory;
            FURI_LOG_I(TAG, "Enabled");
        } else {
            free(memory);
            FURI_LOG_E(TAG, "No SRAM2 available");
        }
    } else {
        free(memory);
        FURI_LOG_E(TAG, "No Core2 available");
    }
}

void* furi_hal_memory_alloc(size_t size) {
    if(furi_hal_memory == NULL) {
        return NULL;
    }

    for(int i = 0; i < SRAM_MAX; i++) {
        if(furi_hal_memory->region[i].size >= size) {
            void* ptr = furi_hal_memory->region[i].start;
            furi_hal_memory->region[i].start += size;
            furi_hal_memory->region[i].size -= size;
            return ptr;
        }
    }
    return NULL;
}

size_t furi_hal_memory_get_free() {
    if(furi_hal_memory == NULL) return 0;

    size_t free = 0;
    for(int i = 0; i < SRAM_MAX; i++) {
        free += furi_hal_memory->region[i].size;
    }
    return free;
}

size_t furi_hal_memory_max_pool_block() {
    if(furi_hal_memory == NULL) return 0;

    size_t max = 0;
    for(int i = 0; i < SRAM_MAX; i++) {
        if(furi_hal_memory->region[i].size > max) {
            max = furi_hal_memory->region[i].size;
        }
    }
    return max;
}