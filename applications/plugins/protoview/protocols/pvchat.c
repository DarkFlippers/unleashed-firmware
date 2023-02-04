#include "../app.h"

/* Copyright (C) 2022-2023 Salvatore Sanfilippo -- All Rights Reserved
 * See the LICENSE file for information about the license.
 *
 * ----------------------------------------------------------
 * ProtoView chat protocol. This is just a fun test protocol
 * that can be used between two Flippers in order to send
 * and receive text messages.
 * ----------------------------------------------------------
 *
 * Protocol description
 * ====================
 *
 * The protocol works with different data rates. However here is defined
 * to use a short pulse/gap duration of 300us and a long pulse/gap
 * duration of 600us. Even with the Flipper hardware, the protocol works
 * with 100/200us, but becomes less reliable and standard presets can't
 * be used because of the higher data rate.
 *
 * In the following description we have that:
 *
 * "1" represents a pulse of one-third bit time (300us)
 * "0" represents a gap of one-third bit time (300us)
 * 
 * The message starts with a preamble + a sync pattern:
 * 
 * preamble = 1010101010101010 x 3
 * sync     = 1100110011001010
 * 
 * The a variable amount of bytes follow, where each bit
 * is encoded in the following way:
 * 
 * zero 100 (300 us pulse, 600 us gap)
 * one  110 (600 us pulse, 300 us gap)
 * 
 * Bytes are sent MSB first, so receiving, in sequence, bits
 * 11100001, means byte E1.
 * 
 * This is the data format:
 * 
 * +--+------+-------+--+--+--+
 * |SL|Sender|Message|FF|AA|CS|
 * +--+------+-------+--+--+--+
 *  |    |      |
 *  |    |      \_ N bytes of message terminated by FF AA + 1 byte of checksum
 *  |    |      
 *  |    \_ SL bytes of sender name
 *  \
 *   \_ 1 byte of sender len, 8 bit unsigned integer.
 * 
 * 
 * Checksum = sum of bytes modulo 256, with checksum set
 *            to 0 for the computation.
 *
 * Design notes
 * ============
 *
 * The protocol is designed in order to have certain properties:
 *
 * 1. Pulses and gaps can only be 100 or 200 microseconds, so the
 *    message can be described, encoded and decoded with only two
 *    fixed durations.
 *
 * 2. The preamble + sync is designed to have a well recognizable
 *    pattern that can't be reproduced just for accident inside
 *    the encoded pattern. There is no combinatio of encoded bits
 *    leading to the preamble+sync. Also the sync pattern final
 *    part can't be mistaken for actual bits of data, since it
 *    contains alternating short pulses/gaps at 100us.
 *
 * 3. Data encoding wastes some bandwidth in order to be more
 *    robust. Even so, with a 300us clock period, a single bit
 *    bit takes 900us, reaching a data transfer of 138 characters per
 *    second. More than enough for the simple chat we have here.
 */

static bool decode(uint8_t* bits, uint32_t numbytes, uint32_t numbits, ProtoViewMsgInfo* info) {
    const char* sync_pattern = "1010101010101010" // Preamble
                               "1100110011001010"; // Sync
    uint8_t sync_len = 32;

    /* This is a variable length message, however the minimum length
     * requires a sender len byte (of value zero) and the terminator
     * FF 00 plus checksum: a total of 4 bytes. */
    if(numbits - sync_len < 8 * 4) return false;

    uint64_t off = bitmap_seek_bits(bits, numbytes, 0, numbits, sync_pattern);
    if(off == BITMAP_SEEK_NOT_FOUND) return false;
    FURI_LOG_E(TAG, "Chat preamble+sync found");

    /* If there is room on the left, let's mark the start of the message
     * a bit before: we don't try to detect all the preamble, but only
     * the first part, however it is likely present. */
    if(off >= 16) {
        off -= 16;
        sync_len += 16;
    }

    info->start_off = off;
    off += sync_len; /* Skip preamble and sync. */

    uint8_t raw[64] = {(uint8_t)'.'};
    uint32_t decoded =
        convert_from_line_code(raw, sizeof(raw), bits, numbytes, off, "100", "110"); /* PWM */
    FURI_LOG_E(TAG, "Chat decoded bits: %lu", decoded);

    if(decoded < 8 * 4) return false; /* Min message len. */

    // The message needs to have a two bytes terminator before
    // the checksum.
    uint32_t j;
    for(j = 0; j < sizeof(raw) - 1; j++)
        if(raw[j] == 0xff && raw[j + 1] == 0xaa) break;

    if(j == sizeof(raw) - 1) {
        FURI_LOG_E(TAG, "Chat: terminator not found");
        return false; // No terminator found.
    }

    uint32_t datalen = j + 3; // If the terminator was found at j, then
        // we need to sum three more bytes to have
        // the len: FF itself, AA, checksum.
    info->pulses_count = sync_len + 8 * 3 * datalen;

    // Check if the control sum matches.
    if(sum_bytes(raw, datalen - 1, 0) != raw[datalen - 1]) {
        FURI_LOG_E(TAG, "Chat: checksum mismatch");
        return false;
    }

    // Check if the length of the sender looks sane
    uint8_t senderlen = raw[0];
    if(senderlen >= sizeof(raw)) {
        FURI_LOG_E(TAG, "Chat: invalid sender length");
        return false; // Overflow
    }

    fieldset_add_str(info->fieldset, "sender", (char*)raw + 1, senderlen);
    fieldset_add_str(
        info->fieldset, "message", (char*)raw + 1 + senderlen, datalen - senderlen - 4);
    return true;
}

/* Give fields and defaults for the signal creator. */
static void get_fields(ProtoViewFieldSet* fieldset) {
    fieldset_add_str(fieldset, "sender", "Carol", 5);
    fieldset_add_str(fieldset, "message", "Anyone hearing?", 15);
}

/* Create a signal. */
static void build_message(RawSamplesBuffer* samples, ProtoViewFieldSet* fs) {
    uint32_t te = 300; /* Short pulse duration in microseconds.
                          Our protocol needs three symbol times to send
                          a bit, so 300 us per bit = 3.33 kBaud. */

    // Preamble: 24 alternating 300us pulse/gap pairs.
    for(int j = 0; j < 24; j++) {
        raw_samples_add(samples, true, te);
        raw_samples_add(samples, false, te);
    }

    // Sync: 3 alternating 600 us pulse/gap pairs.
    for(int j = 0; j < 3; j++) {
        raw_samples_add(samples, true, te * 2);
        raw_samples_add(samples, false, te * 2);
    }

    // Sync: plus 2 alternating 300 us pluse/gap pairs.
    for(int j = 0; j < 2; j++) {
        raw_samples_add(samples, true, te);
        raw_samples_add(samples, false, te);
    }

    // Data: build the array.
    uint32_t datalen = 1 + fs->fields[0]->len + // Userlen + Username
                       fs->fields[1]->len + 3; // Message + FF + 00 + CRC
    uint8_t *data = malloc(datalen), *p = data;
    *p++ = fs->fields[0]->len;
    memcpy(p, fs->fields[0]->str, fs->fields[0]->len);
    p += fs->fields[0]->len;
    memcpy(p, fs->fields[1]->str, fs->fields[1]->len);
    p += fs->fields[1]->len;
    *p++ = 0xff;
    *p++ = 0xaa;
    *p = sum_bytes(data, datalen - 1, 0);

    // Emit bits
    for(uint32_t j = 0; j < datalen * 8; j++) {
        if(bitmap_get(data, datalen, j)) {
            raw_samples_add(samples, true, te * 2);
            raw_samples_add(samples, false, te);
        } else {
            raw_samples_add(samples, true, te);
            raw_samples_add(samples, false, te * 2);
        }
    }
    free(data);
}

ProtoViewDecoder ProtoViewChatDecoder = {
    .name = "ProtoView chat",
    .decode = decode,
    .get_fields = get_fields,
    .build_message = build_message};
