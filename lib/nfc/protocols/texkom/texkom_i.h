/**
 * @file texkom_i.h
 * @brief Texkom protocol internals.
 *
 * This file is an implementation detail. It must not be included in
 * any public API-related headers.
 */
#pragma once

#include "texkom.h"

#ifdef __cplusplus
extern "C" {
#endif

/** Type byte values that name a variant. MMBIT instead carries FF here and in the byte after. */
#define TEXKOM_TYPE_BYTE_TK13 (0x63U)
#define TEXKOM_TYPE_BYTE_TK17 (0xCAU)

/**
 * Gaps in one frame, which is one fewer than the impulse count: the gap after the last impulse
 * falls in the pause between repetitions and is never measured.
 *
 * TK13 sends 128 impulses, TK17 sends 65.
 */
#define TEXKOM_TK13_INTERVALS (127U)
#define TEXKOM_TK17_INTERVALS (64U)

/**
 * @brief Line coding a variant uses on the air.
 *
 * TK13 and TK15 share one coding and differ only in how sloppy their timings are, and MMBIT rides
 * on the TK13 coding as well, so there are only ever these two.
 *
 * A decoded frame always agrees with the coding it arrived in - texkom_decode_intervals() rejects
 * frames where the two disagree - so the coding can be recovered from the type byte afterwards.
 */
typedef enum {
    TexkomCodingTk13,
    TexkomCodingTk17,
} TexkomCoding;

/**
 * @brief Recompute and store the frame's checksum.
 *
 * Does nothing for a variant whose checksum algorithm is not known.
 *
 * @param[in,out] data pointer to the instance to be modified.
 */
void texkom_update_crc(TexkomData* data);

/**
 * @brief Rewrite the frame as a blank key of the given variant.
 *
 * Lays down the markers that name the variant, clears the UID and sets a matching checksum.
 *
 * TexkomTypeTk15 is accepted but pointless: it is byte-identical to TK13, because the two differ
 * only in how ragged a real key's timings are, and anything emitted from here is clean. It reads
 * back as TK13 on any reader, which is why Add Manually does not offer it.
 *
 * @param[in,out] data pointer to the instance to be rewritten.
 * @param[in] type variant to write. TexkomTypeUnknown names no variant and is refused.
 * @returns true on success, false if the variant is not one that can be written.
 */
bool texkom_make_blank(TexkomData* data, TexkomType type);

/**
 * @brief Decode one captured impulse train into a frame.
 *
 * Takes the gaps between consecutive impulses as the HAL hands them over, in capture samples. All
 * the decoders judge the gaps against each other rather than against absolute times, so the
 * sample rate does not matter as long as it is the same for every interval and keeps the longest
 * gap inside a byte.
 *
 * A frame is accepted only if its preamble is intact and its type byte names a variant that uses
 * the coding the frame arrived in, which keeps noise out and lets everything downstream trust the
 * type byte.
 *
 * @param[out] data pointer to the instance to be filled in on success.
 * @param[in] intervals pointer to the array of inter-impulse gaps.
 * @param[in] interval_count number of gaps in the array.
 * @returns true if a frame was decoded, false otherwise.
 */
bool texkom_decode_intervals(TexkomData* data, const uint8_t* intervals, size_t interval_count);

#ifdef __cplusplus
}
#endif
