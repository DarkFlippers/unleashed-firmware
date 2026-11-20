#include "hybridus_color.h"
#include "saved_struct.h"
#include <storage/storage.h>
#include <stdatomic.h>

#define HYBRIDUS_COLOR_PATH INT_PATH(".hybridus-color")
#define HYBRIDUS_COLOR_MAGIC 0xC7
#define HYBRIDUS_COLOR_VERSION 1

// Readers include Bluetooth and RPC tasks. One byte is published atomically.
static _Atomic uint8_t hybridus_color;

void hybridus_color_load(void) {
    uint8_t color = 0;
    if(!saved_struct_load(
           HYBRIDUS_COLOR_PATH, &color, sizeof(color),
           HYBRIDUS_COLOR_MAGIC, HYBRIDUS_COLOR_VERSION) || color > 3) {
        color = 0;
    }
    atomic_store(&hybridus_color, color);
}

uint8_t hybridus_color_get(void) {
    return atomic_load(&hybridus_color);
}

bool hybridus_color_set(uint8_t color) {
    if(color > 3) return false;
    if(!saved_struct_save(
           HYBRIDUS_COLOR_PATH, &color, sizeof(color),
           HYBRIDUS_COLOR_MAGIC, HYBRIDUS_COLOR_VERSION)) return false;
    atomic_store(&hybridus_color, color);
    return true;
}
