#include "hitagmicro.h"
#include <furi.h>
#include <furi_hal_rfid.h>
#include <lib/bit_lib/bit_lib.h>
#include <stdio.h>

#define TAG "HitagMicro"

// --- Hitag micro / ID82xx reader->tag frame constants (from Proxmark3) ---------
// flags/commands: include/protocols.h ; frame layout: armsrc/hitagu.c
#define HITAGMICRO_FLAGS        0x04 // HITAGU_FLAG_CRCT (CRC requested on responses)
#define HITAGMICRO_CMD_READ_UID 0x02 // HITAGU_CMD_READ_UID
#define HITAGMICRO_CMD_SYSINFO  0x17 // HITAGU_CMD_SYSINFO
#define HITAGMICRO_CMD_READ     0x12 // HITAGU_CMD_READ_MULTIPLE_BLOCK
#define HITAGMICRO_CMD_LOGIN    0x28 // HITAGU_CMD_LOGIN
#define HITAGMICRO_CMD_WRITE    0x14 // HITAGU_CMD_WRITE_SINGLE_BLOCK

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

// Field-on hold after each command, long enough for the tag to finish its (unread)
// response before the next command. Derived from a Proxmark3 `lf hitag htu` clone trace
// (timings in 8us ETU): READ UID response is the longest (~18ms).
#define HITAGMICRO_WAIT_UID_US   21000 // after READ UID (48-bit UID response)
#define HITAGMICRO_WAIT_SYS_US   13000 // after GET SYSTEM INFO
#define HITAGMICRO_WAIT_READ_US  17000 // after READ config block
#define HITAGMICRO_WAIT_LOGIN_US 9000 // after LOGIN (auth ack)
#define HITAGMICRO_WAIT_WRITE_US 30000 // field-on after WRITE so the EEPROM finishes programming
#define HITAGMICRO_WRITE_REPEATS 2 // re-send each WRITE per session (8265 commits intermittently)
#define HITAGMICRO_POWERDOWN_US  20000 // field off between blocks so the tag resets cleanly
#define HITAGMICRO_LATCH_CYCLES  6 // clean power cycles after writing to start TTF emission
#define HITAGMICRO_LATCH_HOLD_US 50000 // field-on per latch cycle (long enough to re-read config)
#define HITAGMICRO_COLD_RESET_US 100000 // leading field-off so a warm/emitting tag fully resets

// --- Variant table ---------------------------------------------------------------
// One row per chip: password and display name kept together so the two cannot drift apart.
static const struct {
    uint8_t password[LFRFID_HITAGMICRO_BLOCK_SIZE];
    const char* name;
} hitagmicro_variants[HitagMicroVariantCount] = {
    [HitagMicroVariant8265] = {{0x00, 0x00, 0x00, 0x00}, "8265"},
    [HitagMicroVariant8210] = {{0x9A, 0xC4, 0x99, 0x9C}, "8210"},
    [HitagMicroVariantH55] = {{0x49, 0x6B, 0x0E, 0x59}, "H5.5"},
};

const uint8_t* hitagmicro_variant_password(HitagMicroVariant variant) {
    if(variant >= HitagMicroVariantCount) return NULL;
    return hitagmicro_variants[variant].password;
}

const char* hitagmicro_variant_name(HitagMicroVariant variant) {
    if(variant >= HitagMicroVariantCount) return "Unknown";
    return hitagmicro_variants[variant].name;
}

// --- Frame building --------------------------------------------------------------
// Bits are stored MSB-first (bit offset 0 == MSB of buf[0]), matching Proxmark3's
// concatbits() so the produced byte stream is identical to the firmware's.
static void hitagmicro_put_bit(uint8_t* buf, size_t* bitpos, bool bit) {
    bit_lib_set_bit(buf, *bitpos, bit);
    (*bitpos)++;
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
        hitagmicro_put_bit(buf, bitpos, bit_lib_get_bit(src, i));
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

// Payload-less command frame: flags(5) + cmd(6) + CRC(16). Used for READ UID and SYSINFO.
static size_t hitagmicro_build_cmd(uint8_t* tx, uint8_t cmd) {
    size_t bitpos = 0;
    hitagmicro_put_lsb(tx, &bitpos, HITAGMICRO_FLAGS, 5);
    hitagmicro_put_lsb(tx, &bitpos, cmd, 6);
    hitagmicro_put_crc(tx, &bitpos, hitagmicro_crc16(tx, bitpos));
    return bitpos;
}

// READ MULTIPLE BLOCK frame: flags(5) + cmd(6) + page(8) + count(8) + CRC(16).
static size_t hitagmicro_build_read(uint8_t* tx, uint8_t page, uint8_t count) {
    size_t bitpos = 0;
    hitagmicro_put_lsb(tx, &bitpos, HITAGMICRO_FLAGS, 5);
    hitagmicro_put_lsb(tx, &bitpos, HITAGMICRO_CMD_READ, 6);
    hitagmicro_put_lsb(tx, &bitpos, page, 8);
    hitagmicro_put_lsb(tx, &bitpos, count, 8);
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

// Log a built frame as hex (debug level). Open-loop has no RX, so the transmitted frames
// are the main thing we can trace - compare them against a Proxmark3 `lf hitag htu` capture.
static void hitagmicro_log_frame(const char* label, const uint8_t* tx, size_t nbits) {
#ifndef LOGS_RELEASE_BUILD
    if(furi_log_get_level() < FuriLogLevelDebug) return; // skip the hex build when it is discarded
    char hex[3 * 9 + 1] = {0}; // up to 9 bytes (67-bit max frame)
    size_t pos = 0;
    for(size_t i = 0; i < (nbits + 7) / 8 && pos + 3 < sizeof(hex); i++) {
        pos += snprintf(hex + pos, sizeof(hex) - pos, "%02X ", tx[i]);
    }
    FURI_LOG_D(TAG, "tx %s (%u bits): %s", label, (unsigned)nbits, hex);
#else
    UNUSED(label);
    UNUSED(tx);
    UNUSED(nbits);
#endif
}

// Transmit one frame (timing-critical, interrupts off), then hold the field on for wait_us
// with interrupts enabled so the tag can finish its (unread) response before the next
// command. Keeping the long waits outside FURI_CRITICAL bounds the interrupts-off window to
// a single frame (~13ms typical, up to ~16ms for an all-ones WRITE).
static void hitagmicro_send(const uint8_t* tx, size_t nbits, uint32_t wait_us) {
    FURI_CRITICAL_ENTER();
    hitagmicro_send_frame(tx, nbits);
    FURI_CRITICAL_EXIT();
    furi_delay_us(wait_us);
}

// Energize / de-energize the 125 kHz field. pin_pull_release keeps the antenna from being
// grounded; field_off waits POWERDOWN so the tag powers down and re-selects cleanly between
// blocks. Mirrors t5577_start / t5577_stop in the sibling writers.
static void hitagmicro_field_on(void) {
    furi_hal_rfid_tim_read_start(125000, 0.5);
    furi_hal_rfid_pin_pull_release();
}

static void hitagmicro_field_off(void) {
    furi_hal_rfid_tim_read_stop();
    furi_hal_rfid_pins_reset();
    furi_delay_us(HITAGMICRO_POWERDOWN_US);
}

void hitagmicro_write(const LFRFIDHitagMicro* data, const uint8_t* password) {
    furi_check(data);
    furi_check(password);

    // Cold power-on-reset before the select chain. A preceding successful write leaves this tag
    // warm and actively emitting (its verify read held the field on for up to ~2s, and the latch
    // cycles below drive it into TTF emission). The open-loop chain assumes a cold chip, so
    // without a guaranteed long field-off a 2nd consecutive write to the same tag lands on a chip
    // that never reset and is silently ignored. The read timer is already stopped on entry (the
    // worker's verify read and any prior session leave the field off), so just hold it off - do
    // NOT call tim_read_stop() here: it furi_check-asserts the timer bus is currently running.
    furi_delay_us(HITAGMICRO_COLD_RESET_US);

    // The select-chain frames are identical for every block, so build them once. The {0}
    // zero-fills the buffers as a safety measure; only the first N bits are ever transmitted
    // or fed to the CRC, so the unwritten tail bits never affect the wire output.
    uint8_t read_uid_tx[16] = {0};
    uint8_t sysinfo_tx[16] = {0};
    uint8_t read_cfg_tx[16] = {0};
    uint8_t login_tx[16] = {0};
    size_t read_uid_bits = hitagmicro_build_cmd(read_uid_tx, HITAGMICRO_CMD_READ_UID);
    size_t sysinfo_bits = hitagmicro_build_cmd(sysinfo_tx, HITAGMICRO_CMD_SYSINFO);
    size_t read_cfg_bits = hitagmicro_build_read(read_cfg_tx, HITAGMICRO_PAGE_CONFIG, 0x00);
    size_t login_bits = hitagmicro_build_login(login_tx, password);

    hitagmicro_log_frame("READ UID", read_uid_tx, read_uid_bits);
    hitagmicro_log_frame("SYSINFO", sysinfo_tx, sysinfo_bits);
    hitagmicro_log_frame("READ config", read_cfg_tx, read_cfg_bits);
    hitagmicro_log_frame("LOGIN", login_tx, login_bits);

    // Each block is written in its own power-cycled session (power up, full open-loop
    // select+login, write, power down): the chip drops the session after a write and must be
    // re-selected per block, and a blind LOGIN+WRITE without the select does nothing.
    //
    // Config is written LAST (the PM3 client writes it first). These magic chips are normally
    // already EM4100/TTF-configured, so the data blocks alone change the emitted ID. Writing
    // config first and then having the data writes not land - open-loop has no way to confirm
    // the select/login actually took - leaves the tag reconfigured-but-dataless, i.e. emitting
    // nothing (a "brick"). Config-last means a failed/partial run keeps the previous working
    // config, while a fresh tag still gets configured once its data is in place.
    // One row per block (page + data + label kept together so they cannot drift apart).
    const struct {
        uint8_t page;
        const uint8_t* block;
        const char* label;
    } steps[3] = {
        {HITAGMICRO_PAGE_BLOCK0, data->block0, "WRITE block0"},
        {HITAGMICRO_PAGE_BLOCK1, data->block1, "WRITE block1"},
        {HITAGMICRO_PAGE_CONFIG, data->config, "WRITE config"},
    };

    for(uint8_t i = 0; i < 3; i++) {
        uint8_t write_tx[16] = {0};
        size_t write_bits = hitagmicro_build_write(write_tx, steps[i].page, steps[i].block);
        hitagmicro_log_frame(steps[i].label, write_tx, write_bits);

        hitagmicro_field_on();
        furi_delay_us(HITAGMICRO_CHARGE_US);
        hitagmicro_send(read_uid_tx, read_uid_bits, HITAGMICRO_WAIT_UID_US);
        hitagmicro_send(sysinfo_tx, sysinfo_bits, HITAGMICRO_WAIT_SYS_US);
        hitagmicro_send(read_cfg_tx, read_cfg_bits, HITAGMICRO_WAIT_READ_US);
        hitagmicro_send(login_tx, login_bits, HITAGMICRO_WAIT_LOGIN_US);
        // The 8265 commits a WRITE only intermittently; re-send it a few times within the
        // session (idempotent - same data) to raise the odds it lands this pass.
        for(uint8_t w = 0; w < HITAGMICRO_WRITE_REPEATS; w++) {
            hitagmicro_send(write_tx, write_bits, HITAGMICRO_WAIT_WRITE_US);
        }

        hitagmicro_field_off();
    }

    // The 8265 starts TTF emission only after the written config is latched by a clean
    // power cycle clear of the write session; a single cycle right after writing can be
    // missed by the read-back verify (the emission then shows up a variant later, causing a
    // mislabel). Cycle the field a few more times here so the verify that follows sees the
    // cloned ID promptly and attributes it to the variant that actually wrote it.
    for(uint8_t i = 0; i < HITAGMICRO_LATCH_CYCLES; i++) {
        hitagmicro_field_on();
        // Hold the field on long enough for the tag to power up and re-read the new config
        // (a brief charge-only pulse is not counted as a real power cycle by the chip).
        furi_delay_us(HITAGMICRO_LATCH_HOLD_US);
        hitagmicro_field_off();
    }
}
