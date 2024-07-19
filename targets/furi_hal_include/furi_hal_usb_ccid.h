#pragma once

#include "hid_usage_desktop.h"
#include "hid_usage_button.h"
#include "hid_usage_keyboard.h"
#include "hid_usage_consumer.h"
#include "hid_usage_led.h"
#include <stdint.h>

#define CCID_SHORT_APDU_SIZE (0xFF)

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
    void (*xfr_datablock_callback)(
        const uint8_t* pcToReaderDataBlock,
        uint32_t pcToReaderDataBlockLen,
        uint8_t* readerToPcDataBlock,
        uint32_t* readerToPcDataBlockLen,
        void* context);
} CcidCallbacks;

/** Set CCID callbacks
 *
 * @param      cb       CcidCallbacks instance
 * @param      context  The context for callbacks
 */
void furi_hal_usb_ccid_set_callbacks(CcidCallbacks* cb, void* context);

/** Insert Smart Card */
void furi_hal_usb_ccid_insert_smartcard(void);

/** Remove Smart Card */
void furi_hal_usb_ccid_remove_smartcard(void);

#ifdef __cplusplus
}
#endif
