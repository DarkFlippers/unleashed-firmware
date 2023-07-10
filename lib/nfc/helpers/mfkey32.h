#pragma once

#include <lib/nfc/protocols/mifare_classic.h>

#ifdef __cplusplus
extern "C" {
#endif

typedef struct Mfkey32 Mfkey32;

typedef enum {
    Mfkey32EventParamCollected,
} Mfkey32Event;

typedef void (*Mfkey32ParseDataCallback)(Mfkey32Event event, void* context);

Mfkey32* mfkey32_alloc(uint32_t cuid);

void mfkey32_free(Mfkey32* instance);

void mfkey32_process_data(
    Mfkey32* instance,
    uint8_t* data,
    uint16_t len,
    bool reader_to_tag,
    bool crc_dropped);

void mfkey32_set_callback(Mfkey32* instance, Mfkey32ParseDataCallback callback, void* context);

uint16_t mfkey32_get_auth_sectors(FuriString* string);

#ifdef __cplusplus
}
#endif