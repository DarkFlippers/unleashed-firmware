#include "sector_cache.h"

#include <stddef.h>
#include <stdio.h>
#include <string.h>
#include <furi.h>
#include <furi_hal_memory.h>

#define SECTOR_SIZE 512
#define N_SECTORS 8

typedef struct {
    uint32_t itr;
    uint32_t sectors[N_SECTORS];
    uint8_t sector_data[N_SECTORS][SECTOR_SIZE];
} SectorCache;

static SectorCache* cache = NULL;

void sector_cache_init() {
    if(cache == NULL) {
        cache = memmgr_alloc_from_pool(sizeof(SectorCache));
    }

    if(cache != NULL) {
        memset(cache, 0, sizeof(SectorCache));
    }
}

uint8_t* sector_cache_get(uint32_t n_sector) {
    if(cache != NULL && n_sector != 0) {
        for(int sector_i = 0; sector_i < N_SECTORS; ++sector_i) {
            if(cache->sectors[sector_i] == n_sector) {
                return cache->sector_data[sector_i];
            }
        }
    }
    return NULL;
}

void sector_cache_put(uint32_t n_sector, uint8_t* data) {
    if(cache == NULL) return;
    cache->sectors[cache->itr % N_SECTORS] = n_sector;
    memcpy(cache->sector_data[cache->itr % N_SECTORS], data, SECTOR_SIZE);
    cache->itr++;
}

void sector_cache_invalidate_range(uint32_t start_sector, uint32_t end_sector) {
    if(cache == NULL) return;
    for(int sector_i = 0; sector_i < N_SECTORS; ++sector_i) {
        if((cache->sectors[sector_i] >= start_sector) &&
           (cache->sectors[sector_i] <= end_sector)) {
            cache->sectors[sector_i] = 0;
        }
    }
}