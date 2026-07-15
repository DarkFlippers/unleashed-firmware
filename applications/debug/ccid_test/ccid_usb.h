#pragma once

#include <stdint.h>
#include <furi_hal_usb.h>

#define CCID_SHORT_APDU_SIZE (0xFF)

#ifdef __cplusplus
extern "C" {
#endif

/** App-local USB CCID device interface.
 *
 * This used to live in the firmware HAL (furi_hal_usb_ccid). It now lives
 * inside the ccid_test app: the app defines its own FuriHalUsbInterface and
 * drives the libusb_stm32 device stack directly through the usbd_* inline
 * helpers (usbd_core.h), which are shipped in the app SDK.
 */
extern FuriHalUsbInterface ccid_usb_interface;

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
void ccid_usb_set_callbacks(CcidCallbacks* cb, void* context);

/** Insert Smart Card */
void ccid_usb_insert_smartcard(void);

/** Remove Smart Card */
void ccid_usb_remove_smartcard(void);

#ifdef __cplusplus
}
#endif
