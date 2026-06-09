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
} LFRFIDHitagMicro;

/** Return the 4-byte LOGIN password for a given chip variant (MSB-first).
 *
 * @param      variant  The chip variant
 * @return     pointer to a static 4-byte password, or NULL if variant is invalid
 */
const uint8_t* hitagmicro_variant_password(HitagMicroVariant variant);

/** Human readable name of a chip variant, e.g. "8265". */
const char* hitagmicro_variant_name(HitagMicroVariant variant);

/** Write an EM4100 clone to an ID82xx / Hitag micro magic chip.
 *
 * This is an open-loop (transmit-only) write that mirrors how em4305_write() and
 * t5577_write() work: it modulates the field and never reads the tag back. Each
 * block is written in its own power-cycled session that replays the reader->tag
 * frames Proxmark3's `lf em 410x clone --htu` sends - an open-loop select chain
 * (READ UID -> GET SYSTEM INFO -> READ config) followed by LOGIN and the WRITE -
 * but without the closed-loop RX/anticollision (no tag responses are read). Config
 * is written last so a failed run cannot leave the tag reconfigured-but-dataless.
 * Success is confirmed afterwards by the worker re-reading the EM4100 emulation.
 *
 * @param      data      The payload to clone; the caller fully populates block0/block1/config
 * @param      password  The 4-byte LOGIN password (MSB-first) for the target chip variant
 */
void hitagmicro_write(const LFRFIDHitagMicro* data, const uint8_t* password);

#ifdef __cplusplus
}
#endif
