#include "u8g2/u8g2.h"
#include "qrcode/qrcode.h"
#include "flipper.h"

void u8g2_DrawPixelSize(u8g2_t* u8g2, uint8_t x, uint8_t y, uint8_t size) {
    for(uint8_t px = 0; px < size; px++) {
        for(uint8_t py = 0; py < size; py++) {
            u8g2_DrawPixel(u8g2, x + px, y + py);
        }
    }
}

void u8g2_qrcode(void* p) {
    FuriRecordSubscriber* log = get_default_log();

    // open record
    FuriRecordSubscriber* fb_record = furi_open("u8g2_fb", false, false, NULL, NULL, NULL);

    // Allocate a chunk of memory to store the QR code
    // https://github.com/ricmoo/QRCode
    // we init version 1, 21x21 px, 16 alphanumeric chars with
    // QUARTILE error correction
    const uint8_t qr_version = 1;
    const uint8_t qr_error_correction = ECC_QUARTILE;

    const uint8_t qr_x = 32;
    const uint8_t qr_y = 0;
    const uint8_t qr_size = 3;

    // The structure to manage the QR code
    QRCode qrcode;

    // QR Code init
    uint8_t qrcodeBytes[qrcode_getBufferSize(qr_version)];
    qrcode_initText(&qrcode, qrcodeBytes, qr_version, qr_error_correction, "HELLO FLIPPER");

    if(fb_record == NULL) {
        fuprintf(log, "[widget] cannot create fb record\n");
        furiac_exit(NULL);
    }

    u8g2_t* fb = furi_take(fb_record);

    // clear display
    if(fb != NULL) {
        u8g2_ClearBuffer(fb);
    }

    while(1) {
        if(fb != NULL) {
            // draw qr code
            for(uint8_t y = 0; y < qrcode.size; y++) {
                for(uint8_t x = 0; x < qrcode.size; x++) {
                    if(qrcode_getModule(&qrcode, x, y)) {
                        u8g2_SetDrawColor(fb, 1);
                        u8g2_DrawPixelSize(fb, qr_x + x * qr_size, qr_y + y * qr_size, qr_size);
                    } else {
                        u8g2_SetDrawColor(fb, 0);
                        u8g2_DrawPixelSize(fb, qr_x + x * qr_size, qr_y + y * qr_size, qr_size);
                    }
                }
            }
        } else {
            furiac_exit(NULL);
        }

        furi_commit(fb_record);

        delay(1);
    }
}