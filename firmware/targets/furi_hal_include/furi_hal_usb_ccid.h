#pragma once
#include "hid_usage_desktop.h"
#include "hid_usage_button.h"
#include "hid_usage_keyboard.h"
#include "hid_usage_consumer.h"
#include "hid_usage_led.h"

#ifdef __cplusplus
extern "C" {
#endif

typedef struct {
    uint16_t vid;
    uint16_t pid;
    char manuf[32];
    char product[32];
} FuriHalUsbCcidConfig;

typedef struct {
    void (*icc_power_on_callback)(uint8_t* dataBlock, uint32_t* dataBlockLen, void* context);
    void (*xfr_datablock_callback)(uint8_t* dataBlock, uint32_t* dataBlockLen, void* context);
} CcidCallbacks;

void furi_hal_ccid_set_callbacks(CcidCallbacks* cb);

void furi_hal_ccid_ccid_insert_smartcard();
void furi_hal_ccid_ccid_remove_smartcard();

#ifdef __cplusplus
}
#endif
