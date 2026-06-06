#pragma once
#include <stdint.h>
#include <stdbool.h>

#ifdef __cplusplus
extern "C" {
#endif

#define LFRFID_HITAGMICRO_BLOCK_SIZE 4

// Known ID82xx / Hitag micro "magic" cloning chips. They are identical apart
// from the LOGIN password, so a single writer handles all of them.
typedef enum {
    HitagMicroVariant8265, // 8265 (CN) / H5 (RU) - factory default, password 00000000
    HitagMicroVariant8210, // 8210 (CN) - password 9AC4999C
    HitagMicroVariantH55, // H5.5 (RU) - password 496B0E59

    HitagMicroVariantCount,
} HitagMicroVariant;

// Data to clone an EM4100 ID onto an ID82xx / Hitag micro magic chip.
// All fields are stored MSB-first, exactly as they are transmitted on the wire.
typedef struct {
    uint8_t block0[LFRFID_HITAGMICRO_BLOCK_SIZE]; // EM4100 frame bits 63..32 -> page 0x00
    uint8_t block1[LFRFID_HITAGMICRO_BLOCK_SIZE]; // EM4100 frame bits 31..0  -> page 0x01
    uint8_t config[LFRFID_HITAGMICRO_BLOCK_SIZE]; // TTF config word          -> page 0xFF
    uint8_t password[LFRFID_HITAGMICRO_BLOCK_SIZE]; // LOGIN credential
} LFRFIDHitagMicro;

/** Return the 4-byte LOGIN password for a given chip variant (MSB-first).
 *
 * @param      variant  The chip variant
 * @return     pointer to a static 4-byte password, or NULL if variant is invalid
 */
const uint8_t* hitagmicro_variant_password(HitagMicroVariant variant);

/** Human readable name of a chip variant, e.g. "8265 / H5". */
const char* hitagmicro_variant_name(HitagMicroVariant variant);

/** Write an EM4100 clone to an ID82xx / Hitag micro magic chip.
 *
 * This is an open-loop (transmit-only) write that mirrors how em4305_write() and
 * t5577_write() work: it modulates the field and never reads the tag back. It
 * reproduces the reader->tag frames that Proxmark3's `lf em 410x clone --htu`
 * sends (LOGIN + WRITE block0 + WRITE block1 + WRITE config), but skips the
 * closed-loop SELECT/anticollision exchange. This is sufficient for a single
 * known-password chip in the field; success is confirmed afterwards by the
 * worker re-reading the resulting EM4100 emulation.
 *
 * @param      data  The data to write (blocks, config and password)
 */
void hitagmicro_write(LFRFIDHitagMicro* data);

#ifdef __cplusplus
}
#endif
