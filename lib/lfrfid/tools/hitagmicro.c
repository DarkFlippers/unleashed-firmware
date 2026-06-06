#include "hitagmicro.h"
#include <furi.h>
#include <furi_hal_rfid.h>
#include <lib/bit_lib/bit_lib.h>

#define TAG "HitagMicro"

// --- Hitag micro / ID82xx reader->tag frame constants (from Proxmark3) ---------
// flags/commands: include/protocols.h ; frame layout: armsrc/hitagu.c
#define HITAGMICRO_FLAGS     0x04 // HITAGU_FLAG_CRCT (CRC requested on responses)
#define HITAGMICRO_CMD_LOGIN 0x28 // HITAGU_CMD_LOGIN
#define HITAGMICRO_CMD_WRITE 0x14 // HITAGU_CMD_WRITE_SINGLE_BLOCK

#define HITAGMICRO_PAGE_BLOCK0 0x00 // first EM4100 data block
#define HITAGMICRO_PAGE_BLOCK1 0x01 // second EM4100 data block
#define HITAGMICRO_PAGE_CONFIG 0xFF // HITAGU_CONFIG_PADR

// --- Reader->tag BPLM/OOK modulation timings, microseconds -----------------------
// The reader->tag link is plain on/off-keyed field-gap modulation (NOT Manchester;
// Manchester is only the tag->reader direction, unused here). Each bit cell begins
// with a fixed field gap, then the field is held on for the rest of the cell; a '1'
// keeps the field on longer than a '0'. Constants mirror Proxmark3 armsrc/hitag_common.h,
// which are in T0 units (T0 = one 125 kHz carrier cycle = 8 us): T_LOW=8 T0 (64us),
// T_0=20 T0 (160us), T_1=28 T0 (224us), T_CODE_VIOLATION=36 T0 (288us). Chip tolerances
// are wide (T_0 18..22, T_1 26..32 T0).
#define HITAGMICRO_GAP_US           64 // T_LOW: leading field-off gap of every cell
#define HITAGMICRO_BIT0_ON_US       96 // (T_0 - T_LOW): field-on tail of a '0' (160us cell)
#define HITAGMICRO_BIT1_ON_US       160 // (T_1 - T_LOW): field-on tail of a '1' (224us cell)
#define HITAGMICRO_SOF_VIOLATION_US 288 // T_CODE_VIOLATION: field-on part of the SOF
#define HITAGMICRO_CHARGE_US        3000 // field-on charge before the first frame
#define HITAGMICRO_WAIT_US          1600 // field-on hold between frames (field stays energized)

// --- Variant tables --------------------------------------------------------------
static const uint8_t hitagmicro_passwords[HitagMicroVariantCount][LFRFID_HITAGMICRO_BLOCK_SIZE] = {
    [HitagMicroVariant8265] = {0x00, 0x00, 0x00, 0x00},
    [HitagMicroVariant8210] = {0x9A, 0xC4, 0x99, 0x9C},
    [HitagMicroVariantH55] = {0x49, 0x6B, 0x0E, 0x59},
};

static const char* const hitagmicro_names[HitagMicroVariantCount] = {
    [HitagMicroVariant8265] = "8265/H5",
    [HitagMicroVariant8210] = "8210",
    [HitagMicroVariantH55] = "H5.5",
};

const uint8_t* hitagmicro_variant_password(HitagMicroVariant variant) {
    if(variant >= HitagMicroVariantCount) return NULL;
    return hitagmicro_passwords[variant];
}

const char* hitagmicro_variant_name(HitagMicroVariant variant) {
    if(variant >= HitagMicroVariantCount) return "Unknown";
    return hitagmicro_names[variant];
}

// --- Frame building --------------------------------------------------------------
// Bits are stored MSB-first (bit offset 0 == MSB of buf[0]), matching Proxmark3's
// concatbits() so the produced byte stream is identical to the firmware's.
static void hitagmicro_put_bit(uint8_t* buf, size_t* bitpos, bool bit) {
    size_t pos = *bitpos;
    if(bit) {
        buf[pos / 8] |= (1 << (7 - (pos % 8)));
    } else {
        buf[pos / 8] &= ~(1 << (7 - (pos % 8)));
    }
    *bitpos = pos + 1;
}

// Append the low nbits of a byte value, LSB-first (concatbits src_lsb=true).
static void hitagmicro_put_lsb(uint8_t* buf, size_t* bitpos, uint8_t value, uint8_t nbits) {
    for(uint8_t i = 0; i < nbits; i++) {
        hitagmicro_put_bit(buf, bitpos, (value >> i) & 1);
    }
}

// Append nbits from a byte array, MSB-first (concatbits src_lsb=false).
static void
    hitagmicro_put_msb_bytes(uint8_t* buf, size_t* bitpos, const uint8_t* src, size_t nbits) {
    for(size_t i = 0; i < nbits; i++) {
        hitagmicro_put_bit(buf, bitpos, (src[i / 8] >> (7 - (i % 8))) & 1);
    }
}

// CRC-16 with poly 0x1021, init 0x0000, refin=false, refout=true. (This refin/refout
// mix is not a standard named CCITT variant; it is what the Hitag U frames use.)
// Ported from Proxmark3 common/crc16.c Crc16(); takes a bit length because our frames
// are not byte-aligned (43-bit LOGIN, 51-bit WRITE before the CRC).
static uint16_t hitagmicro_crc16(const uint8_t* d, size_t bitlength) {
    if(bitlength == 0) return 0;

    uint16_t remainder = 0;
    // Front-pad the stream with zeros to byte-align it for the loop; the pad is 0 for an
    // already byte-aligned length. Leading zero bits do not change the CRC (init is 0).
    uint8_t offset = (8 - (bitlength % 8)) % 8;
    uint8_t prebits = 0;

    for(size_t i = 0; i < (bitlength + 7) / 8; i++) {
        uint8_t c = prebits | (uint8_t)(d[i] >> offset);
        prebits = (uint8_t)(d[i] << (8 - offset));

        remainder ^= (uint16_t)(c << 8);
        for(uint8_t j = 8; j; --j) {
            if(remainder & 0x8000) {
                remainder = (uint16_t)((remainder << 1) ^ 0x1021);
            } else {
                remainder <<= 1;
            }
        }
    }

    // refout: reflect the 16-bit remainder
    return bit_lib_reverse_16_fast(remainder);
}

// Append the 16-bit CRC, LSB-first (concatbits of the little-endian value, src_lsb=true).
static void hitagmicro_put_crc(uint8_t* buf, size_t* bitpos, uint16_t crc) {
    for(uint8_t i = 0; i < 16; i++) {
        hitagmicro_put_bit(buf, bitpos, (crc >> i) & 1);
    }
}

// LOGIN frame: flags(5) + cmd(6) + password(32) + CRC(16). Returns total bit count.
static size_t hitagmicro_build_login(uint8_t* tx, const uint8_t* password) {
    size_t bitpos = 0;
    hitagmicro_put_lsb(tx, &bitpos, HITAGMICRO_FLAGS, 5);
    hitagmicro_put_lsb(tx, &bitpos, HITAGMICRO_CMD_LOGIN, 6);
    hitagmicro_put_msb_bytes(tx, &bitpos, password, 32);
    hitagmicro_put_crc(tx, &bitpos, hitagmicro_crc16(tx, bitpos));
    return bitpos;
}

// WRITE SINGLE BLOCK frame: flags(5) + cmd(6) + page(8) + data(32) + CRC(16).
static size_t hitagmicro_build_write(uint8_t* tx, uint8_t page, const uint8_t* data) {
    size_t bitpos = 0;
    hitagmicro_put_lsb(tx, &bitpos, HITAGMICRO_FLAGS, 5);
    hitagmicro_put_lsb(tx, &bitpos, HITAGMICRO_CMD_WRITE, 6);
    hitagmicro_put_lsb(tx, &bitpos, page, 8);
    hitagmicro_put_msb_bytes(tx, &bitpos, data, 32);
    hitagmicro_put_crc(tx, &bitpos, hitagmicro_crc16(tx, bitpos));
    return bitpos;
}

// --- Modulation ------------------------------------------------------------------
static void hitagmicro_gap(void) {
    furi_hal_rfid_tim_read_pause();
    furi_delay_us(HITAGMICRO_GAP_US);
    furi_hal_rfid_tim_read_continue();
}

static void hitagmicro_send_bit(bool bit) {
    hitagmicro_gap();
    furi_delay_us(bit ? HITAGMICRO_BIT1_ON_US : HITAGMICRO_BIT0_ON_US);
}

static void hitagmicro_send_sof(void) {
    // SOF = a '0' bit followed by a code violation (gap + extended field-on).
    hitagmicro_send_bit(false);
    hitagmicro_gap();
    furi_delay_us(HITAGMICRO_SOF_VIOLATION_US);
}

static void hitagmicro_send_frame(const uint8_t* tx, size_t nbits) {
    hitagmicro_send_sof();
    for(size_t i = 0; i < nbits; i++) {
        hitagmicro_send_bit((tx[i / 8] >> (7 - (i % 8))) & 1);
    }
    // EOF = a trailing gap; the field stays energized for the next frame.
    hitagmicro_gap();
}

void hitagmicro_write(LFRFIDHitagMicro* data) {
    furi_check(data);

    // Build every frame up front: bit packing and CRC need no timing guarantees, so they
    // run with interrupts enabled. Only the bit-banged modulation below must be timed, so
    // it is all the FURI_CRITICAL section has to cover. ({0} zero-fills, which the builders
    // rely on for the unwritten tail bits of the last byte.)
    uint8_t login_tx[16] = {0};
    uint8_t block0_tx[16] = {0};
    uint8_t block1_tx[16] = {0};
    uint8_t config_tx[16] = {0};
    size_t login_bits = hitagmicro_build_login(login_tx, data->password);
    size_t block0_bits = hitagmicro_build_write(block0_tx, HITAGMICRO_PAGE_BLOCK0, data->block0);
    size_t block1_bits = hitagmicro_build_write(block1_tx, HITAGMICRO_PAGE_BLOCK1, data->block1);
    size_t config_bits = hitagmicro_build_write(config_tx, HITAGMICRO_PAGE_CONFIG, data->config);

    furi_hal_rfid_tim_read_start(125000, 0.5);
    // do not ground the antenna
    furi_hal_rfid_pin_pull_release();

    FURI_CRITICAL_ENTER();

    // Charge the tag before talking to it.
    furi_delay_us(HITAGMICRO_CHARGE_US);

    // 1. LOGIN with the chip password. The field is intentionally kept energized through
    //    the writes below so the authenticated session is not dropped between frames.
    hitagmicro_send_frame(login_tx, login_bits);
    furi_delay_us(HITAGMICRO_WAIT_US);

    // 2. WRITE EM4100 data block 0 (page 0x00).
    hitagmicro_send_frame(block0_tx, block0_bits);
    furi_delay_us(HITAGMICRO_WAIT_US);

    // 3. WRITE EM4100 data block 1 (page 0x01).
    hitagmicro_send_frame(block1_tx, block1_bits);
    furi_delay_us(HITAGMICRO_WAIT_US);

    // 4. WRITE config last, so TTF EM4100 emulation only starts once data is in place.
    hitagmicro_send_frame(config_tx, config_bits);

    FURI_CRITICAL_EXIT();

    furi_hal_rfid_tim_read_stop();
    furi_hal_rfid_pins_reset();
}
