/* Copyright (C) 2022-2023 Salvatore Sanfilippo -- All Rights Reserved
 * See the LICENSE file for information about the license. */

#include "app.h"

bool decode_signal(RawSamplesBuffer* s, uint64_t len, ProtoViewMsgInfo* info);

/* =============================================================================
 * Protocols table.
 *
 * Supported protocols go here, with the relevant implementation inside
 * protocols/<name>.c
 * ===========================================================================*/

extern ProtoViewDecoder Oregon2Decoder;
extern ProtoViewDecoder B4B1Decoder;
extern ProtoViewDecoder RenaultTPMSDecoder;
extern ProtoViewDecoder ToyotaTPMSDecoder;
extern ProtoViewDecoder SchraderTPMSDecoder;
extern ProtoViewDecoder SchraderEG53MA4TPMSDecoder;
extern ProtoViewDecoder CitroenTPMSDecoder;
extern ProtoViewDecoder FordTPMSDecoder;
extern ProtoViewDecoder KeeloqDecoder;
extern ProtoViewDecoder ProtoViewChatDecoder;
extern ProtoViewDecoder UnknownDecoder;

ProtoViewDecoder* Decoders[] = {
    &Oregon2Decoder, /* Oregon sensors v2.1 protocol. */
    &B4B1Decoder, /* PT, SC, ... 24 bits remotes. */
    &RenaultTPMSDecoder, /* Renault TPMS. */
    &ToyotaTPMSDecoder, /* Toyota TPMS. */
    &SchraderTPMSDecoder, /* Schrader TPMS. */
    &SchraderEG53MA4TPMSDecoder, /* Schrader EG53MA4 TPMS. */
    &CitroenTPMSDecoder, /* Citroen TPMS. */
    &FordTPMSDecoder, /* Ford TPMS. */
    &KeeloqDecoder, /* Keeloq remote. */
    &ProtoViewChatDecoder, /* Protoview simple text messages. */

    /* Warning: the following decoder must stay at the end of the
     * list. Otherwise would detect most signals and prevent the actaul
     * decoders from handling them. */
    &UnknownDecoder, /* General protocol detector. */
    NULL};

/* =============================================================================
 * Raw signal detection
 * ===========================================================================*/

/* Return the time difference between a and b, always >= 0 since
 * the absolute value is returned. */
uint32_t duration_delta(uint32_t a, uint32_t b) {
    return a > b ? a - b : b - a;
}

/* Reset the current signal, so that a new one can be detected. */
void reset_current_signal(ProtoViewApp* app) {
    app->signal_bestlen = 0;
    app->signal_offset = 0;
    app->signal_decoded = false;
    raw_samples_reset(DetectedSamples);
    raw_samples_reset(RawSamples);
    free_msg_info(app->msg_info);
    app->msg_info = NULL;
}

/* This function starts scanning samples at offset idx looking for the
 * longest run of pulses, either high or low, that are not much different
 * from each other, for a maximum of three duration classes.
 * So for instance 50 successive pulses that are roughly long 340us or 670us
 * will be sensed as a coherent signal (example: 312, 361, 700, 334, 667, ...)
 *
 * The classes are counted separtely for high and low signals (RF on / off)
 * because many devices tend to have different pulse lenghts depending on
 * the level of the pulse.
 *
 * For instance Oregon2 sensors, in the case of protocol 2.1 will send
 * pulses of ~400us (RF on) VS ~580us (RF off). */
#define SEARCH_CLASSES 3
uint32_t search_coherent_signal(RawSamplesBuffer* s, uint32_t idx, uint32_t min_duration) {
    struct {
        uint32_t dur[2]; /* dur[0] = low, dur[1] = high */
        uint32_t count[2]; /* Associated observed frequency. */
    } classes[SEARCH_CLASSES];

    memset(classes, 0, sizeof(classes));

    // Set a min/max duration limit for samples to be considered part of a
    // coherent signal. The maximum length is fixed while the minimum
    // is passed as argument, as depends on the data rate and in general
    // on the signal to analyze.
    uint32_t max_duration = 4000;

    uint32_t len = 0; /* Observed len of coherent samples. */
    s->short_pulse_dur = 0;
    for(uint32_t j = idx; j < idx + s->total; j++) {
        bool level;
        uint32_t dur;
        raw_samples_get(s, j, &level, &dur);

        if(dur < min_duration || dur > max_duration) break; /* return. */

        /* Let's see if it matches a class we already have or if we
         * can populate a new (yet empty) class. */
        uint32_t k;
        for(k = 0; k < SEARCH_CLASSES; k++) {
            if(classes[k].count[level] == 0) {
                classes[k].dur[level] = dur;
                classes[k].count[level] = 1;
                break; /* Sample accepted. */
            } else {
                uint32_t classavg = classes[k].dur[level];
                uint32_t count = classes[k].count[level];
                uint32_t delta = duration_delta(dur, classavg);
                /* Is the difference in duration between this signal and
                 * the class we are inspecting less than a given percentage?
                 * If so, accept this signal. */
                if(delta < classavg / 5) { /* 100%/5 = 20%. */
                    /* It is useful to compute the average of the class
                     * we are observing. We know how many samples we got so
                     * far, so we can recompute the average easily.
                     * By always having a better estimate of the pulse len
                     * we can avoid missing next samples in case the first
                     * observed samples are too off. */
                    classavg = ((classavg * count) + dur) / (count + 1);
                    classes[k].dur[level] = classavg;
                    classes[k].count[level]++;
                    break; /* Sample accepted. */
                }
            }
        }

        if(k == SEARCH_CLASSES) break; /* No match, return. */

        /* If we are here, we accepted this sample. Try with the next
         * one. */
        len++;
    }

    /* Update the buffer setting the shortest pulse we found
     * among the three classes. This will be used when scaling
     * for visualization. */
    uint32_t short_dur[2] = {0, 0};
    for(int j = 0; j < SEARCH_CLASSES; j++) {
        for(int level = 0; level < 2; level++) {
            if(classes[j].dur[level] == 0) continue;
            if(classes[j].count[level] < 3) continue;
            if(short_dur[level] == 0 || short_dur[level] > classes[j].dur[level]) {
                short_dur[level] = classes[j].dur[level];
            }
        }
    }

    /* Use the average between high and low short pulses duration.
     * Often they are a bit different, and using the average is more robust
     * when we do decoding sampling at short_pulse_dur intervals. */
    if(short_dur[0] == 0) short_dur[0] = short_dur[1];
    if(short_dur[1] == 0) short_dur[1] = short_dur[0];
    s->short_pulse_dur = (short_dur[0] + short_dur[1]) / 2;

    return len;
}

/* Called when we detect a message. Just blinks when the message was
 * not decoded. Vibrates, too, when the message was correctly decoded. */
void notify_signal_detected(ProtoViewApp* app, bool decoded) {
    static const NotificationSequence decoded_seq = {
        &message_vibro_on,
        &message_green_255,
        &message_delay_50,
        &message_green_0,
        &message_vibro_off,
        NULL};

    static const NotificationSequence unknown_seq = {
        &message_red_255, &message_delay_50, &message_red_0, NULL};

    if(decoded)
        notification_message(app->notification, &decoded_seq);
    else
        notification_message(app->notification, &unknown_seq);
}

/* Search the source buffer with the stored signal (last N samples received)
 * in order to find a coherent signal. If a signal that does not appear to
 * be just noise is found, it is set in DetectedSamples global signal
 * buffer, that is what is rendered on the screen. */
void scan_for_signal(ProtoViewApp* app, RawSamplesBuffer* source, uint32_t min_duration) {
    /* We need to work on a copy: the source buffer may be populated
     * by the background thread receiving data. */
    RawSamplesBuffer* copy = raw_samples_alloc();
    raw_samples_copy(copy, source);

    /* Try to seek on data that looks to have a regular high low high low
     * pattern. */
    uint32_t minlen = 18; /* Min run of coherent samples. With less
                                       than a few samples it's very easy to
                                       mistake noise for signal. */

    uint32_t i = 0;

    while(i < copy->total - 1) {
        uint32_t thislen = search_coherent_signal(copy, i, min_duration);

        /* For messages that are long enough, attempt decoding. */
        if(thislen > minlen) {
            /* Allocate the message information that some decoder may
             * fill, in case it is able to decode a message. */
            ProtoViewMsgInfo* info = malloc(sizeof(ProtoViewMsgInfo));
            init_msg_info(info, app);
            info->short_pulse_dur = copy->short_pulse_dur;

            uint32_t saved_idx = copy->idx; /* Save index, see later. */

            /* decode_signal() expects the detected signal to start
             * from index zero .*/
            raw_samples_center(copy, i);
            bool decoded = decode_signal(copy, thislen, info);
            copy->idx = saved_idx; /* Restore the index as we are scanning
                                      the signal in the loop. */

            /* Accept this signal as the new signal if either it's longer
             * than the previous undecoded one, or the previous one was
             * unknown and this is decoded. */
            bool oldsignal_not_decoded = app->signal_decoded == false ||
                                         app->msg_info->decoder == &UnknownDecoder;

            if(oldsignal_not_decoded &&
               (thislen > app->signal_bestlen || (decoded && info->decoder != &UnknownDecoder))) {
                free_msg_info(app->msg_info);
                app->msg_info = info;
                app->signal_bestlen = thislen;
                app->signal_decoded = decoded;
                raw_samples_copy(DetectedSamples, copy);
                raw_samples_center(DetectedSamples, i);
                FURI_LOG_E(
                    TAG,
                    "===> Displayed sample updated (%d samples %lu us)",
                    (int)thislen,
                    DetectedSamples->short_pulse_dur);

                adjust_raw_view_scale(app, DetectedSamples->short_pulse_dur);
                if(app->msg_info->decoder != &UnknownDecoder) notify_signal_detected(app, decoded);
            } else {
                /* If the structure was not filled, discard it. Otherwise
                 * now the owner is app->msg_info. */
                free_msg_info(info);
            }
        }
        i += thislen ? thislen : 1;
    }
    raw_samples_free(copy);
}

/* =============================================================================
 * Decoding
 *
 * The following code will translates the raw singals as received by
 * the CC1101 into logical signals: a bitmap of 0s and 1s sampled at
 * the detected data clock interval.
 *
 * Then the converted signal is passed to the protocols decoders, that look
 * for protocol-specific information. We stop at the first decoder that is
 * able to decode the data, so protocols here should be registered in
 * order of complexity and specificity, with the generic ones at the end.
 * ===========================================================================*/

/* Set the 'bitpos' bit to value 'val', in the specified bitmap
 * 'b' of len 'blen'.
 * Out of range bits will silently be discarded. */
void bitmap_set(uint8_t* b, uint32_t blen, uint32_t bitpos, bool val) {
    uint32_t byte = bitpos / 8;
    uint32_t bit = 7 - (bitpos & 7);
    if(byte >= blen) return;
    if(val)
        b[byte] |= 1 << bit;
    else
        b[byte] &= ~(1 << bit);
}

/* Get the bit 'bitpos' of the bitmap 'b' of 'blen' bytes.
 * Out of range bits return false (not bit set). */
bool bitmap_get(uint8_t* b, uint32_t blen, uint32_t bitpos) {
    uint32_t byte = bitpos / 8;
    uint32_t bit = 7 - (bitpos & 7);
    if(byte >= blen) return 0;
    return (b[byte] & (1 << bit)) != 0;
}

/* Copy 'count' bits from the bitmap 's' of 'slen' total bytes, to the
 * bitmap 'd' of 'dlen' total bytes. The bits are copied starting from
 * offset 'soff' of the source bitmap to the offset 'doff' of the
 * destination bitmap. */
void bitmap_copy(
    uint8_t* d,
    uint32_t dlen,
    uint32_t doff,
    uint8_t* s,
    uint32_t slen,
    uint32_t soff,
    uint32_t count) {
    /* If we are byte-aligned in both source and destination, use a fast
     * path for the number of bytes we can consume this way. */
    if((doff & 7) == 0 && (soff & 7) == 0) {
        uint32_t didx = doff / 8;
        uint32_t sidx = soff / 8;
        while(count > 8 && didx < dlen && sidx < slen) {
            d[didx++] = s[sidx++];
            count -= 8;
        }
        doff = didx * 8;
        soff = sidx * 8;
        /* Note that if we entered this path, the count at the end
         * of the loop will be < 8. */
    }

    /* Copy the bits needed to reach an offset where we can copy
     * two half bytes of src to a full byte of destination. */
    while(count > 8 && (doff & 7) != 0) {
        bool bit = bitmap_get(s, slen, soff++);
        bitmap_set(d, dlen, doff++, bit);
        count--;
    }

    /* If we are here and count > 8, we have an offset that is byte aligned
     * to the destination bitmap, but not aligned to the source bitmap.
     * We can copy fast enough by shifting each two bytes of the original
     * bitmap.
     *
     * This is how it works:
     *
     *  dst:
     *  +--------+--------+--------+
     *  | 0      | 1      | 2      |
     *  |        |        |        | <- data to fill
     *  +--------+--------+--------+
     *            ^
     *            |
     *            doff = 8
     *
     *  src:
     *  +--------+--------+--------+
     *  | 0      | 1      | 2      |
     *  |hellowor|ld!HELLO|WORLDS!!| <- data to copy
     *  +--------+--------+--------+
     *               ^
     *               |
     *               soff = 11
     *
     *  skew = 11%8 = 3
     *  each destination byte in dst will receive:
     *
     *  dst[doff/8] = (src[soff/8] << skew) | (src[soff/8+1] >> (8-skew))
     *
     *  dstbyte = doff/8 = 8/8 = 1
     *  srcbyte = soff/8 = 11/8 = 1
     *
     *  so dst[1] will get:
     *  src[1] << 3, that is "ld!HELLO" << 3 = "HELLO..."
     *      xored with
     *  src[2] << 5, that is "WORLDS!!" >> 5 = ".....WOR"
     *  That is "HELLOWOR"
     */
    if(count > 8) {
        uint8_t skew = soff % 8; /* Don't worry, compiler will optimize. */
        uint32_t didx = doff / 8;
        uint32_t sidx = soff / 8;
        while(count > 8 && didx < dlen && sidx < slen) {
            d[didx] = ((s[sidx] << skew) | (s[sidx + 1] >> (8 - skew)));
            sidx++;
            didx++;
            soff += 8;
            doff += 8;
            count -= 8;
        }
    }

    /* Here count is guaranteed to be < 8.
     * Copy the final bits bit by bit. */
    while(count) {
        bool bit = bitmap_get(s, slen, soff++);
        bitmap_set(d, dlen, doff++, bit);
        count--;
    }
}

/* We decode bits assuming the first bit we receive is the MSB
 * (see bitmap_set/get functions). Certain devices send data
 * encoded in the reverse way. */
void bitmap_reverse_bytes_bits(uint8_t* p, uint32_t len) {
    for(uint32_t j = 0; j < len; j++) {
        uint32_t b = p[j];
        /* Step 1: swap the two nibbles: 12345678 -> 56781234 */
        b = (b & 0xf0) >> 4 | (b & 0x0f) << 4;
        /* Step 2: swap adjacent pairs : 56781234 -> 78563412 */
        b = (b & 0xcc) >> 2 | (b & 0x33) << 2;
        /* Step 3: swap adjacent bits  : 78563412 -> 87654321 */
        b = (b & 0xaa) >> 1 | (b & 0x55) << 1;
        p[j] = b;
    }
}

/* Return true if the specified sequence of bits, provided as a string in the
 * form "11010110..." is found in the 'b' bitmap of 'blen' bits at 'bitpos'
 * position. */
bool bitmap_match_bits(uint8_t* b, uint32_t blen, uint32_t bitpos, const char* bits) {
    for(size_t j = 0; bits[j]; j++) {
        bool expected = (bits[j] == '1') ? true : false;
        if(bitmap_get(b, blen, bitpos + j) != expected) return false;
    }
    return true;
}

/* Search for the specified bit sequence (see bitmap_match_bits() for details)
 * in the bitmap 'b' of 'blen' bytes, looking forward at most 'maxbits' ahead.
 * Returns the offset (in bits) of the match, or BITMAP_SEEK_NOT_FOUND if not
 * found.
 *
 * Note: there are better algorithms, such as Boyer-Moore. Here we hope that
 * for the kind of patterns we search we'll have a lot of early stops so
 * we use a vanilla approach. */
uint32_t bitmap_seek_bits(
    uint8_t* b,
    uint32_t blen,
    uint32_t startpos,
    uint32_t maxbits,
    const char* bits) {
    uint32_t endpos = startpos + blen * 8;
    uint32_t end2 = startpos + maxbits;
    if(end2 < endpos) endpos = end2;
    for(uint32_t j = startpos; j < endpos; j++)
        if(bitmap_match_bits(b, blen, j, bits)) return j;
    return BITMAP_SEEK_NOT_FOUND;
}

/* Compare bitmaps b1 and b2 (possibly overlapping or the same bitmap),
 * at the specified offsets, for cmplen bits. Returns true if the
 * exact same bits are found, otherwise false. */
bool bitmap_match_bitmap(
    uint8_t* b1,
    uint32_t b1len,
    uint32_t b1off,
    uint8_t* b2,
    uint32_t b2len,
    uint32_t b2off,
    uint32_t cmplen) {
    for(uint32_t j = 0; j < cmplen; j++) {
        bool bit1 = bitmap_get(b1, b1len, b1off + j);
        bool bit2 = bitmap_get(b2, b2len, b2off + j);
        if(bit1 != bit2) return false;
    }
    return true;
}

/* Convert 'len' bitmap bits of the bitmap 'bitmap' into a null terminated
 * string, stored at 'dst', that must have space at least for len+1 bytes.
 * The bits are extracted from the specified offset. */
void bitmap_to_string(char* dst, uint8_t* b, uint32_t blen, uint32_t off, uint32_t len) {
    for(uint32_t j = 0; j < len; j++) dst[j] = bitmap_get(b, blen, off + j) ? '1' : '0';
    dst[len] = 0;
}

/* Set the pattern 'pat' into the bitmap 'b' of max length 'blen' bytes,
 * starting from the specified offset.
 *
 * The pattern is given as a string of 0s and 1s characters, like "01101001".
 * This function is useful in order to set the test vectors in the protocol
 * decoders, to see if the decoding works regardless of the fact we are able
 * to actually receive a given signal. */
void bitmap_set_pattern(uint8_t* b, uint32_t blen, uint32_t off, const char* pat) {
    uint32_t i = 0;
    while(pat[i]) {
        bitmap_set(b, blen, i + off, pat[i] == '1');
        i++;
    }
}

/* Take the raw signal and turn it into a sequence of bits inside the
 * buffer 'b'. Note that such 0s and 1s are NOT the actual data in the
 * signal, but is just a low level representation of the line code. Basically
 * if the short pulse we find in the signal is 320us, we convert high and
 * low levels in the raw sample in this way:
 *
 * If for instance we see a high level lasting ~600 us, we will add
 * two 1s bit. If then the signal goes down for 330us, we will add one zero,
 * and so forth. So for each period of high and low we find the closest
 * multiple and set the relevant number of bits.
 * 
 * In case of a short pulse of 320us detected, 320*2 is the closest to a
 * high pulse of 600us, so 2 bits will be set.
 *
 * In other terms what this function does is sampling the signal at
 * fixed 'rate' intervals.
 *
 * This representation makes it simple to decode the signal at a higher
 * level later, translating it from Marshal coding or other line codes
 * to the actual bits/bytes.
 *
 * The 'idx' argument marks the detected signal start index into the
 * raw samples buffer. The 'count' tells the function how many raw
 * samples to convert into bits. The function returns the number of
 * bits set into the buffer 'b'. The 'rate' argument, in microseconds, is
 * the detected short-pulse duration. We expect the line code to be
 * meaningful when interpreted at multiples of 'rate'. */
uint32_t convert_signal_to_bits(
    uint8_t* b,
    uint32_t blen,
    RawSamplesBuffer* s,
    uint32_t idx,
    uint32_t count,
    uint32_t rate) {
    if(rate == 0) return 0; /* We can't perform the conversion. */
    uint32_t bitpos = 0;
    for(uint32_t j = 0; j < count; j++) {
        uint32_t dur;
        bool level;
        raw_samples_get(s, j + idx, &level, &dur);

        uint32_t numbits = dur / rate; /* full bits that surely fit. */
        uint32_t rest = dur % rate; /* How much we are left with. */
        if(rest > rate / 2) numbits++; /* There is another one. */

        /* Limit how much a single sample can spawn. There are likely no
         * protocols doing such long pulses when the rate is low. */
        if(numbits > 1024) numbits = 1024;

        if(0) /* Super verbose, so not under the DEBUG_MSG define. */
            FURI_LOG_E(TAG, "%lu converted into %lu (%d) bits", dur, numbits, (int)level);

        /* If the signal is too short, let's claim it an interference
         * and ignore it completely. */
        if(numbits == 0) continue;

        while(numbits--) bitmap_set(b, blen, bitpos++, level);
    }
    return bitpos;
}

/* This function converts the line code used to the final data representation.
 * The representation is put inside 'buf', for up to 'buflen' bytes of total
 * data. For instance in order to convert manchester you can use "10" and "01"
 * as zero and one patterns. However this function does not handle differential
 * encodings. See below for convert_from_diff_manchester().
 *
 * The function returns the number of bits converted. It will stop as soon
 * as it finds a pattern that does not match zero or one patterns, or when
 * the end of the bitmap pointed by 'bits' is reached (the length is
 * specified in bytes by the caller, via the 'len' parameters).
 *
 * The decoding starts at the specified offset (in bits) 'off'. */
uint32_t convert_from_line_code(
    uint8_t* buf,
    uint64_t buflen,
    uint8_t* bits,
    uint32_t len,
    uint32_t off,
    const char* zero_pattern,
    const char* one_pattern) {
    uint32_t decoded = 0; /* Number of bits extracted. */
    len *= 8; /* Convert bytes to bits. */
    while(off < len) {
        bool bitval;
        if(bitmap_match_bits(bits, len, off, zero_pattern)) {
            bitval = false;
            off += strlen(zero_pattern);
        } else if(bitmap_match_bits(bits, len, off, one_pattern)) {
            bitval = true;
            off += strlen(one_pattern);
        } else {
            break;
        }
        bitmap_set(buf, buflen, decoded++, bitval);
        if(decoded / 8 == buflen) break; /* No space left on target buffer. */
    }
    return decoded;
}

/* Convert the differential Manchester code to bits. This is similar to
 * convert_from_line_code() but specific for diff-Manchester. The user must
 * supply the value of the previous symbol before this stream, since
 * in differential codings the next bits depend on the previous one.
 *
 * Parameters and return values are like convert_from_line_code(). */
uint32_t convert_from_diff_manchester(
    uint8_t* buf,
    uint64_t buflen,
    uint8_t* bits,
    uint32_t len,
    uint32_t off,
    bool previous) {
    uint32_t decoded = 0;
    len *= 8; /* Conver to bits. */
    for(uint32_t j = off; j < len; j += 2) {
        bool b0 = bitmap_get(bits, len, j);
        bool b1 = bitmap_get(bits, len, j + 1);
        if(b0 == previous) break; /* Each new bit must switch value. */
        bitmap_set(buf, buflen, decoded++, b0 == b1);
        previous = b1;
        if(decoded / 8 == buflen) break; /* No space left on target buffer. */
    }
    return decoded;
}

/* Free the message info and allocated data. */
void free_msg_info(ProtoViewMsgInfo* i) {
    if(i == NULL) return;
    fieldset_free(i->fieldset);
    free(i->bits);
    free(i);
}

/* Reset the message info structure before passing it to the decoding
 * functions. */
void init_msg_info(ProtoViewMsgInfo* i, ProtoViewApp* app) {
    UNUSED(app);
    memset(i, 0, sizeof(ProtoViewMsgInfo));
    i->bits = NULL;
    i->fieldset = fieldset_new();
}

/* This function is called when a new signal is detected. It converts it
 * to a bitstream, and the calls the protocol specific functions for
 * decoding. If the signal was decoded correctly by some protocol, true
 * is returned. Otherwise false is returned. */
bool decode_signal(RawSamplesBuffer* s, uint64_t len, ProtoViewMsgInfo* info) {
    uint32_t bitmap_bits_size = 4096 * 8;
    uint32_t bitmap_size = bitmap_bits_size / 8;

    /* We call the decoders with an offset a few samples before the actual
     * signal detected and for a len of a few bits after its end. */
    uint32_t before_samples = 32;
    uint32_t after_samples = 100;

    uint8_t* bitmap = malloc(bitmap_size);
    uint32_t bits = convert_signal_to_bits(
        bitmap,
        bitmap_size,
        s,
        -before_samples,
        len + before_samples + after_samples,
        s->short_pulse_dur);

    if(DEBUG_MSG) { /* Useful for debugging purposes. Don't remove. */
        char* str = malloc(1024);
        uint32_t j;
        for(j = 0; j < bits && j < 1023; j++) {
            str[j] = bitmap_get(bitmap, bitmap_size, j) ? '1' : '0';
        }
        str[j] = 0;
        FURI_LOG_E(TAG, "%lu bits sampled: %s", bits, str);
        free(str);
    }

    /* Try all the decoders available. */
    int j = 0;

    bool decoded = false;
    while(Decoders[j]) {
        uint32_t start_time = furi_get_tick();
        decoded = Decoders[j]->decode(bitmap, bitmap_size, bits, info);
        uint32_t delta = furi_get_tick() - start_time;
        FURI_LOG_E(TAG, "Decoder %s took %lu ms", Decoders[j]->name, (unsigned long)delta);
        if(decoded) {
            info->decoder = Decoders[j];
            break;
        }
        j++;
    }

    if(!decoded) {
        FURI_LOG_E(TAG, "No decoding possible");
    } else {
        FURI_LOG_E(TAG, "+++ Decoded %s", info->decoder->name);
        /* The message was correctly decoded: fill the info structure
         * with the decoded signal. The decoder may not implement offset/len
         * filling of the structure. In such case we have no info and
         * pulses_count will be set to zero. */
        if(info->pulses_count) {
            info->bits_bytes = (info->pulses_count + 7) / 8; // Round to full byte.
            info->bits = malloc(info->bits_bytes);
            bitmap_copy(
                info->bits,
                info->bits_bytes,
                0,
                bitmap,
                bitmap_size,
                info->start_off,
                info->pulses_count);
        }
    }
    free(bitmap);
    return decoded;
}
