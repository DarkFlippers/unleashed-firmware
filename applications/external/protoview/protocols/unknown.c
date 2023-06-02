#include "../app.h"

/* Copyright (C) 2023 Salvatore Sanfilippo -- All Rights Reserved
 * See the LICENSE file for information about the license.
 *
 * ----------------------------------------------------------------------------
 * The "unknown" decoder fires as the last one, once we are sure no other
 * decoder was able to identify the signal. The goal is to detect the
 * preamble and line code used in the received signal, then turn the
 * decoded bits into bytes.
 *
 * The techniques used for the detection are described in the comments
 * below.
 * ----------------------------------------------------------------------------
 */

/* Scan the signal bitmap looking for a PWM modulation. In this case
 * for PWM we are referring to two exact patterns of high and low
 * signal (each bit in the bitmap is worth the smallest gap/pulse duration
 * we detected) that repeat each other in a given segment of the message.
 *
 * This modulation is quite common, for instance sometimes zero and
 * one are rappresented by a 700us pulse followed by 350 gap,
 * and 350us pulse followed by a 700us gap. So the signal bitmap received
 * by the decoder would contain 110 and 100 symbols.
 *
 * The way this function work is commented inline.
 *
 * The function returns the number of consecutive symbols found, having
 * a symbol length of 'symlen' (3 in the above example), and stores
 * in *s1i the offset of the first symbol found, and in *s2i the offset
 * of the second symbol. The function can't tell which is one and which
 * zero. */
static uint32_t find_pwm(
    uint8_t* bits,
    uint32_t numbytes,
    uint32_t numbits,
    uint32_t symlen,
    uint32_t* s1i,
    uint32_t* s2i) {
    uint32_t best_count = 0; /* Max number of symbols found in this try. */
    uint32_t best_idx1 = 0; /* First symbol offset of longest sequence found.
                              * This is also the start sequence offset. */
    uint32_t best_idx2 = 0; /* Second symbol offset. */

    /* Try all the possible symbol offsets that are less of our
     * symbol len. This is likely not really useful but we take
     * a conservative approach. Because if have have, for instance,
     * repeating symbols "100" and "110", they will form a sequence
     * that is choerent at different offsets, but out-of-sync.
     *
     * Anyway at the end of the function we try to fix the sync. */
    for(uint32_t off = 0; off < symlen; off++) {
        uint32_t c = 0; // Number of contiguous symbols found.
        uint32_t c1 = 0, c2 = 0; // Occurrences of first/second symbol.
        *s1i = off; // Assume we start at one symbol boundaty.
        *s2i = UINT32_MAX; // Second symbol first index still unknown.
        uint32_t next = off;

        /* We scan the whole bitmap in one pass, resetting the state
         * each time we find a pattern that is not one of the two
         * symbols we found so far. */
        while(next < numbits - symlen) {
            bool match1 = bitmap_match_bitmap(bits, numbytes, next, bits, numbytes, *s1i, symlen);
            if(!match1 && *s2i == UINT32_MAX) {
                /* It's not the first sybol. We don't know how the
                 * second look like. Assume we found an occurrence of
                 * the second symbol. */
                *s2i = next;
            }

            bool match2 = bitmap_match_bitmap(bits, numbytes, next, bits, numbytes, *s2i, symlen);

            /* One or the other should match. */
            if(match1 || match2) {
                c++;
                if(match1) c1++;
                if(match2) c2++;
                if(c > best_count && c1 >= best_count / 5 && // Require enough presence of both
                   c2 >= best_count / 5) // zero and one.
                {
                    best_count = c;
                    best_idx1 = *s1i;
                    best_idx2 = *s2i;
                }
                next += symlen;
            } else {
                /* No match. Continue resetting the signal info. */
                c = 0; // Start again to count contiguous symbols.
                c1 = 0;
                c2 = 0;
                *s1i = next; // First symbol always at start.
                *s2i = UINT32_MAX; // Second symbol unknown.
            }
        }
    }

    /* We don't know if we are really synchronized with the bits at this point.
     * For example if zero bit is 100 and one bit is 110 in a specific
     * line code, our detector could randomly believe it's 001 and 101.
     * However PWD line codes normally start with a pulse in both symbols.
     * If that is the case, let's align. */
    uint32_t shift;
    for(shift = 0; shift < symlen; shift++) {
        if(bitmap_get(bits, numbytes, best_idx1 + shift) &&
           bitmap_get(bits, numbytes, best_idx2 + shift))
            break;
    }
    if(shift != symlen) {
        best_idx1 += shift;
        best_idx2 += shift;
    }

    *s1i = best_idx1;
    *s2i = best_idx2;
    return best_count;
}

/* Find the longest sequence that looks like Manchester coding.
 *
 * Manchester coding requires each pairs of bits to be either
 * 01 or 10. We'll have to try odd and even offsets to be
 * sure to find it.
 *
 * Note that this will also detect differential Manchester, but
 * will report it as Manchester. I can't think of any way to
 * distinguish between the two line codes, because shifting them
 * one symbol will make one to look like the other.
 *
 * Only option could be to decode the message with both line
 * codes and use statistical properties (common byte values)
 * to determine what's more likely, but this looks very fragile.
 *
 * Fortunately differential Manchester is more rarely used,
 * so we can assume Manchester most of the times. Yet we are left
 * with the indetermination about zero being pulse-gap or gap-pulse
 * or the other way around.
 *
 * If the 'only_raising' parameter is true, the function detects
 * only sequences going from gap to pulse: this is useful in order
 * to locate preambles of alternating gaps and pulses. */
static uint32_t find_alternating_bits(
    uint8_t* bits,
    uint32_t numbytes,
    uint32_t numbits,
    uint32_t* start,
    bool only_raising) {
    uint32_t best_count = 0; // Max number of symbols found
    uint32_t best_off = 0; // Max symbols start offset.
    for(int odd = 0; odd < 2; odd++) {
        uint32_t count = 0; // Symbols found so far
        uint32_t start_off = odd;
        uint32_t j = odd;
        while(j < numbits - 1) {
            bool bit1 = bitmap_get(bits, numbytes, j);
            bool bit2 = bitmap_get(bits, numbytes, j + 1);
            if((!only_raising && bit1 != bit2) || (only_raising && !bit1 && bit2)) {
                count++;
                if(count > best_count) {
                    best_count = count;
                    best_off = start_off;
                }
            } else {
                /* End of sequence. Continue with the next
                 * part of the signal. */
                count = 0;
                start_off = j + 2;
            }
            j += 2;
        }
    }
    *start = best_off;
    return best_count;
}

/* Wrapper to find Manchester code. */
static uint32_t
    find_manchester(uint8_t* bits, uint32_t numbytes, uint32_t numbits, uint32_t* start) {
    return find_alternating_bits(bits, numbytes, numbits, start, false);
}

/* Wrapper to find preamble sections. */
static uint32_t
    find_preamble(uint8_t* bits, uint32_t numbytes, uint32_t numbits, uint32_t* start) {
    return find_alternating_bits(bits, numbytes, numbits, start, true);
}

typedef enum {
    LineCodeNone,
    LineCodeManchester,
    LineCodePWM3,
    LineCodePWM4,
} LineCodeGuess;

static char* get_linecode_name(LineCodeGuess lc) {
    switch(lc) {
    case LineCodeNone:
        return "none";
    case LineCodeManchester:
        return "Manchester";
    case LineCodePWM3:
        return "PWM3";
    case LineCodePWM4:
        return "PWM4";
    }
    return "unknown";
}

static bool decode(uint8_t* bits, uint32_t numbytes, uint32_t numbits, ProtoViewMsgInfo* info) {
    /* No decoder was able to detect this message. Let's try if we can
     * find some structure. To start, we'll see if it looks like is
     * manchester coded, or PWM with symbol len of 3 or 4. */

    /* For PWM, start1 and start2 are the offsets at which the two
     * sequences composing the message appear the first time.
     * So start1 is also the message start offset. Start2 is not used
     * for Manchester, that does not have two separated symbols like
     * PWM. */
    uint32_t start1 = 0, start2 = 0;
    uint32_t msgbits; // Number of message bits in the bitmap, so
        // this will be the number of symbols, not actual
        // bits after the message is decoded.
    uint32_t tmp1, tmp2; // Temp vars to store the start.
    uint32_t minbits = 16; // Less than that gets undetected.
    uint32_t pwm_len; // Bits per symbol, in the case of PWM.
    LineCodeGuess linecode = LineCodeNone;

    // Try PWM3
    uint32_t pwm3_bits = find_pwm(bits, numbytes, numbits, 3, &tmp1, &tmp2);
    if(pwm3_bits >= minbits) {
        linecode = LineCodePWM3;
        start1 = tmp1;
        start2 = tmp2;
        pwm_len = 3;
        msgbits = pwm3_bits * pwm_len;
    }

    // Try PWM4
    uint32_t pwm4_bits = find_pwm(bits, numbytes, numbits, 4, &tmp1, &tmp2);
    if(pwm4_bits >= minbits && pwm4_bits > pwm3_bits) {
        linecode = LineCodePWM4;
        start1 = tmp1;
        start2 = tmp2;
        pwm_len = 4;
        msgbits = pwm3_bits * pwm_len;
    }

    // Try Manchester
    uint32_t manchester_bits = find_manchester(bits, numbytes, numbits, &tmp1);
    if(manchester_bits > minbits && manchester_bits > pwm3_bits && manchester_bits > pwm4_bits) {
        linecode = LineCodeManchester;
        start1 = tmp1;
        msgbits = manchester_bits * 2;
        //FURI_LOG_T(TAG, "MANCHESTER START: %lu", tmp1);
    }

    if(linecode == LineCodeNone) return false;

    /* Often there is a preamble before the signal. We'll try to find
     * it, and if it is not too far away from our signal, we'll claim
     * our signal starts at the preamble. */
    uint32_t preamble_len = find_preamble(bits, numbytes, numbits, &tmp1);
    uint32_t min_preamble_len = 10;
    uint32_t max_preamble_distance = 32;
    uint32_t preamble_start = 0;
    bool preamble_found = false;

    /* Note that because of the following checks, if the Manchester detector
     * detected the preamble bits as data, we are ok with that, since it
     * means that the synchronization is not designed to "break" the bits
     * flow. In this case we ignore the preamble and return all as data. */
    if(preamble_len >= min_preamble_len && // Not too short.
       tmp1 < start1 && // Should be before the data.
       start1 - tmp1 <= max_preamble_distance) // Not too far.
    {
        preamble_start = tmp1;
        preamble_found = true;
    }

    info->start_off = preamble_found ? preamble_start : start1;
    info->pulses_count = (start1 + msgbits) - info->start_off;
    info->pulses_count += 20; /* Add a few more, so that if the user resends
                               * the message, it is more likely we will
                               * transfer all that is needed, like a message
                               * terminator (that we don't detect). */

    /*if(preamble_found) FURI_LOG_T(TAG, "PREAMBLE AT: %lu", preamble_start);
    FURI_LOG_T(TAG, "START: %lu", info->start_off);
    FURI_LOG_T(TAG, "MSGBITS: %lu", msgbits);
    FURI_LOG_T(TAG, "DATASTART: %lu", start1);
    FURI_LOG_T(TAG, "PULSES: %lu", info->pulses_count);*/

    /* We think there is a message and we know where it starts and the
     * line code used. We can turn it into bits and bytes. */
    uint32_t decoded;
    uint8_t data[32];
    uint32_t datalen;

    char symbol1[5], symbol2[5];
    if(linecode == LineCodePWM3 || linecode == LineCodePWM4) {
        bitmap_to_string(symbol1, bits, numbytes, start1, pwm_len);
        bitmap_to_string(symbol2, bits, numbytes, start2, pwm_len);
    } else if(linecode == LineCodeManchester) {
        memcpy(symbol1, "01", 3);
        memcpy(symbol2, "10", 3);
    }

    decoded = convert_from_line_code(data, sizeof(data), bits, numbytes, start1, symbol1, symbol2);
    datalen = (decoded + 7) / 8;

    char* linecode_name = get_linecode_name(linecode);
    fieldset_add_str(info->fieldset, "line code", linecode_name, strlen(linecode_name));
    fieldset_add_uint(info->fieldset, "data bits", decoded, 8);
    if(preamble_found) fieldset_add_uint(info->fieldset, "preamble len", preamble_len, 8);
    fieldset_add_str(info->fieldset, "first symbol", symbol1, strlen(symbol1));
    fieldset_add_str(info->fieldset, "second symbol", symbol2, strlen(symbol2));
    for(uint32_t j = 0; j < datalen; j++) {
        char label[16];
        snprintf(label, sizeof(label), "data[%lu]", j);
        fieldset_add_bytes(info->fieldset, label, data + j, 2);
    }
    return true;
}

ProtoViewDecoder UnknownDecoder =
    {.name = "Unknown", .decode = decode, .get_fields = NULL, .build_message = NULL};
