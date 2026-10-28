#pragma once
#include <storage/storage.h> // EXT_PATH, for the dictionary below
#include <stdint.h>
#include <stdbool.h>
#include <stddef.h>

#ifdef __cplusplus
extern "C" {
#endif

#define LFRFID_T5577_BLOCK_COUNT      8
#define LFRFID_T5577_BLOCKS_IN_PAGE_0 LFRFID_T5577_BLOCK_COUNT
#define LFRFID_T5577_BLOCKS_IN_PAGE_1 4

/** Blocks a page of a T5577 has. Page 1 is shorter than page 0, so every bound over a page has
 * to ask - and the answer is chip geometry, which is why it lives here rather than beside the
 * file format. Pages other than 0 and 1 are answered as page 1; a caller holding a page number
 * it did not write down should check it before asking.
 */
static inline uint8_t t5577_page_blocks(uint8_t page) {
    return (page == 0) ? LFRFID_T5577_BLOCKS_IN_PAGE_0 : LFRFID_T5577_BLOCKS_IN_PAGE_1;
}

// T5577 block 0 definitions, thanks proxmark3!
#define LFRFID_T5577_POR_DELAY             0x00000001
#define LFRFID_T5577_ST_TERMINATOR         0x00000008
#define LFRFID_T5577_PWD                   0x00000010
#define LFRFID_T5577_MAXBLOCK_SHIFT        5
#define LFRFID_T5577_MAXBLOCK_MASK         0x7
#define LFRFID_T5577_AOR                   0x00000200
#define LFRFID_T5577_PSKCF_RF_2            0
#define LFRFID_T5577_PSKCF_RF_4            0x00000400
#define LFRFID_T5577_PSKCF_RF_8            0x00000800
// PSK carrier is a two bit field, so RF/2 is the value zero rather than a bit, and
// both bits set is a combination the chip leaves reserved.
#define LFRFID_T5577_PSKCF_MASK            0x00000C00
// Modulation occupies bits 12..16 of block 0 and the bit rate bits 18..20; the named values
// below are field values, so isolating either field needs its mask.
#define LFRFID_T5577_MODULATION_MASK       0x0001F000
#define LFRFID_T5577_BITRATE_MASK          0x001C0000
#define LFRFID_T5577_BITRATE_SHIFT         18
#define LFRFID_T5577_MODULATION_DIRECT     0
#define LFRFID_T5577_MODULATION_PSK1       0x00001000
#define LFRFID_T5577_MODULATION_PSK2       0x00002000
#define LFRFID_T5577_MODULATION_PSK3       0x00003000
#define LFRFID_T5577_MODULATION_FSK1       0x00004000
#define LFRFID_T5577_MODULATION_FSK2       0x00005000
#define LFRFID_T5577_MODULATION_FSK1a      0x00006000
#define LFRFID_T5577_MODULATION_FSK2a      0x00007000
#define LFRFID_T5577_MODULATION_MANCHESTER 0x00008000
#define LFRFID_T5577_MODULATION_BIPHASE    0x00010000
#define LFRFID_T5577_MODULATION_DIPHASE    0x00018000
#define LFRFID_T5577_X_MODE                0x00020000
#define LFRFID_T5577_BITRATE_RF_8          0
#define LFRFID_T5577_BITRATE_RF_16         0x00040000
#define LFRFID_T5577_BITRATE_RF_32         0x00080000
#define LFRFID_T5577_BITRATE_RF_40         0x000C0000
#define LFRFID_T5577_BITRATE_RF_50         0x00100000
#define LFRFID_T5577_BITRATE_RF_64         0x00140000
#define LFRFID_T5577_BITRATE_RF_100        0x00180000
#define LFRFID_T5577_BITRATE_RF_128        0x001C0000
#define LFRFID_T5577_TESTMODE_DISABLED     0x60000000

/** Modulation a configuration word asks for, as a field rather than a bit.
 *
 * Has to be compared whole: DIPHASE is MANCHESTER|BIPHASE, so testing the Manchester bit
 * on its own accepts the two protocols that write diphase.
 */
static inline uint32_t t5577_config_modulation(uint32_t block0) {
    return block0 & LFRFID_T5577_MODULATION_MASK;
}

/** Bit rate a configuration word asks for, as a field. */
static inline uint32_t t5577_config_bitrate(uint32_t block0) {
    return block0 & LFRFID_T5577_BITRATE_MASK;
}

/** PSK carrier a configuration word asks for. RF/2 is the field's zero. */
static inline uint32_t t5577_config_psk_carrier(uint32_t block0) {
    return block0 & LFRFID_T5577_PSKCF_MASK;
}

/** Blocks a configuration word says the chip broadcasts. */
static inline uint8_t t5577_config_max_block(uint32_t block0) {
    return (uint8_t)((block0 >> LFRFID_T5577_MAXBLOCK_SHIFT) & LFRFID_T5577_MAXBLOCK_MASK);
}

/** Whether a configuration word asks for a password.
 *
 * Answer-on-request counts. The bit that protects writes is LFRFID_T5577_PWD, but a chip with
 * LFRFID_T5577_AOR set says nothing at all until a password frame wakes it, so a reader without
 * one gets no further than a reader turned away - and t5577_detect_both() already keeps a
 * password on exactly this test.
 */
static inline bool t5577_config_needs_password(uint32_t block0) {
    return (block0 & (LFRFID_T5577_PWD | LFRFID_T5577_AOR)) != 0;
}

/** Whether a block holds the factory traceability data.
 *
 * Page 1 blocks 1 and 2 are programmed at manufacture and locked, so a genuine chip refuses to
 * change them: the year and quarter, the wafer and the die the chip came off. Some clones do
 * allow the write, which is the only case in which putting them back means anything - and the
 * reason a refusal there is the chip behaving correctly rather than a write that failed.
 */
static inline bool t5577_block_is_trace(uint8_t page, uint8_t block) {
    return page == 1 && (block == 1 || block == 2);
}

/** The block the chip takes its password from.
 *
 * Writing it replaces the word the chip will ask for, so every frame sent after it has to carry
 * the new one - which is why a write order puts it back before block 0 turns protection on,
 * rather than anywhere in particular among the data blocks.
 */
#define LFRFID_T5577_PASSWORD_BLOCK 7

typedef struct {
    uint32_t block[LFRFID_T5577_BLOCK_COUNT];
    uint32_t blocks_to_write;
    uint8_t mask;
} LFRFIDT5577;

/**
 * @brief Write T5577 tag data to tag
 * 
 * @param data 
 */
void t5577_write(LFRFIDT5577* data);

void t5577_write_with_pass(LFRFIDT5577* data, uint32_t password);

void t5577_write_with_mask(LFRFIDT5577* data, uint8_t page, bool with_pass, uint32_t password);

/** Write one block, which is what every caller outside a full restore actually wants.
 *
 * Inline so that it adds no API row: it is the mask arithmetic and the zeroed frame that were
 * being written out four times over, not anything the library had to export.
 */
static inline void t5577_write_block(
    uint8_t page,
    uint8_t block,
    uint32_t value,
    bool with_pass,
    uint32_t password) {
    // Every member named: this header is also compiled as C++, where a partial initialiser is
    // -Werror=missing-field-initializers.
    LFRFIDT5577 frame = {{0}, 0, (uint8_t)(1u << block)};

    frame.block[block] = value;

    t5577_write_with_mask(&frame, page, with_pass, password);
}

/** How a command is put on the wire.
 *
 * Fixed bit length is what a tag answers to out of the factory and the only
 * mode this driver's own callers use. The other three exist because a tag can
 * be configured to expect them, and then nothing else gets through.
 *
 * The fixed row carries this driver's own long-standing timings. The other
 * three keep those gaps and take their symbol lengths from Proxmark3's
 * `T55xx_Timing` table in `armsrc/lfops.c`; none of the three has ever driven a
 * tag from here.
 */
typedef enum {
    T5577DownlinkFixedBitLength,
    T5577DownlinkLongLeadingReference,
    T5577DownlinkLeadingZero,
    T5577Downlink1Of4,
    /** Count of the modes this firmware knows, for iteration and bounds.
     *
     * It is this build's count, not a property of the chip: a later firmware
     * may add modes, so an app must not use it to size anything it stores or
     * exchanges.
     */
    T5577DownlinkModeMax,
} T5577DownlinkMode;

/** The frames a T5577 takes.
 *
 * Not one-to-one with the chip's opcodes: the page bit is a field of
 * T5577Frame, so one command covers both pages. There is no wake-up frame
 * either, because an AOR wake-up is T5577CommandRegularRead carrying a
 * password, which is what Proxmark3's `lf t55xx wakeup` sends.
 */
typedef enum {
    T5577CommandReset, // opcode 00
    T5577CommandRegularRead, // opcode + password; no lock, data or address
    T5577CommandRead, // opcode + password + lock + address
    T5577CommandWrite, // opcode + password + lock + data + address
} T5577Command;

/** Downlink timings, in field clocks - one field clock is 8 us at 125 kHz.
 *
 * Which gap and symbol lengths a given tag and antenna will take is the thing
 * that has to be tuned in the field (Proxmark3 keeps a whole command for it), so
 * these come from the caller rather than being baked in. write_2 and write_3
 * carry the extra two symbols of 1-of-4 coding and are required only by that
 * mode. There is no read gap: the caller owns the field and the capture after a
 * frame, so how long to listen is its business.
 */
typedef struct {
    uint16_t start_gap;
    uint16_t write_gap;
    uint16_t write_0;
    uint16_t write_1;
    uint16_t write_2;
    uint16_t write_3;
} T5577Timing;

typedef struct {
    T5577Command command;
    T5577DownlinkMode downlink;
    const T5577Timing* timing; // NULL for t5577_timing_default(); read during the call only
    uint32_t password; // with_pass only
    uint32_t data; // T5577CommandWrite only
    uint8_t page; // 0 or 1; not T5577CommandReset
    uint8_t block; // T5577CommandRead and T5577CommandWrite only
    bool with_pass;
    bool lock; // T5577CommandRead and T5577CommandWrite only
} T5577Frame;

/** Timings this driver uses when a frame does not bring its own.
 *
 * Points into static storage and stays valid for the life of the firmware.
 */
const T5577Timing* t5577_timing_default(T5577DownlinkMode mode);

/** Raise the carrier that a frame is cut into.
 *
 * A frame is gaps in a carrier that is already up, so t5577_send() cannot do
 * this for itself: bringing the field down between frames would reset the tag.
 *
 * This returns as soon as the carrier is running and does not wait for the
 * envelope detector - that wait belongs after a command frame, because what the
 * detector has to recover from is the gaps the frame cuts, and a caller that
 * only writes never listens at all. t5577_block_holds() does it for itself.
 */
void t5577_field_start(void);

/** Drop the carrier and release the pins.
 *
 * Nests: the carrier only goes down when the last holder stops. An unbalanced stop is
 * logged and ignored rather than fatal, because the count is this module's own bookkeeping
 * and a caller that drives furi_hal_rfid directly can desynchronise it.
 */
void t5577_field_stop(void);

/** Force the carrier down and the nesting count to zero.
 *
 * The way back when a caller has exited without stopping: without it the count never
 * returns to zero, every later t5577_field_start() returns early without raising the
 * carrier, and frames go out into a dead field.
 */
void t5577_field_reset(void);

/** Put one frame on the wire.
 *
 * Needs the field up (t5577_field_start) and leaves it up. Runs with interrupts
 * off, because the bits are timed by busy waiting, and a write frame holds them
 * off for its programming time as well.
 *
 * @param      frame  what to send
 */
void t5577_send(const T5577Frame* frame);

/** The blank Proxmark3's `lf t55xx wipe` leaves: ASK/Manchester, RF/32, seven data blocks and
 * no password. Writing the same word is what makes a tag wiped by either tool look the same to
 * the other.
 */
#define LFRFID_T5577_CONFIG_BLANK                                      \
    (LFRFID_T5577_BITRATE_RF_32 | LFRFID_T5577_MODULATION_MANCHESTER | \
     (7UL << LFRFID_T5577_MAXBLOCK_SHIFT)) // == 0x000880E0

/** A configuration with the password bit clear: ASK/Manchester, RF/64, two data blocks. What
 * Proxmark3 writes to block 0 to drop a password, and what an EM4100 asks for anyway.
 *
 * Clearing LFRFID_T5577_PWD is the whole of removing a password - block 7 goes on holding the
 * old word and simply stops being consulted.
 */
#define LFRFID_T5577_CONFIG_OPEN                                       \
    (LFRFID_T5577_BITRATE_RF_64 | LFRFID_T5577_MODULATION_MANCHESTER | \
     (2UL << LFRFID_T5577_MAXBLOCK_SHIFT)) // == 0x00148040

/** The passwords to try on a tag whose own is not known, as Proxmark3 ships them: one eight
 * digit hex word per line, `#` for a comment, read with keys_dict from lib/toolbox the same way
 * the NFC key dictionaries are.
 *
 * A file rather than a table in flash because the list is data that outgrows a release: an
 * update overwrites it, and a word found next year should not need new firmware to try. It is
 * also 508 bytes of internal flash that a dictionary on the card does not spend.
 *
 * Worth knowing before sweeping it: 44 of the shipped words have bit 31 set, and a password
 * frame that reaches a tag with no password at all is read off the wrong bit positions - where
 * that bit lands on block 0's lock bit. So a sweep has to stop at the first word that opens the
 * tag.
 */
#define T5577_DICT_PATH EXT_PATH("lfrfid/assets/t5577_dict.lfrfid")

/** Where a password the user found themselves is kept.
 *
 * Separate from the shipped list because that one carries "Do not edit, this file will be
 * overwritten after firmware update" in its own header, and it means it. A word recovered off a
 * cloner, or typed in from a note, has to outlive an update - so it goes here, and the shipped
 * file stays the vendor's to replace. The same split the NFC key dictionaries use.
 *
 * Opened with KeysDictModeOpenExisting by anything that reads it: a missing file is simply an
 * empty list, and nothing should create one just by looking.
 */
#define T5577_DICT_USER_PATH EXT_PATH("lfrfid/assets/t5577_dict_user.lfrfid")
#define T5577_DICT_KEY_SIZE  4

/** Bit rate a block read's answer is clocked at, in field clocks per bit.
 *
 * The tag answers in whatever block 0 configured, so this mirrors the
 * LFRFID_T5577_BITRATE_RF_* the caller wrote. Only the rates the ASK/Manchester
 * decoder is proven at are offered; t5577_read_clock_from_config() maps a
 * configuration word onto them.
 */
typedef enum {
    T5577ReadClockUnsupported = 0,
    T5577ReadClock16 = 16,
    T5577ReadClock32 = 32,
    T5577ReadClock64 = 64,
} T5577ReadClock;

/** A block to address and how to read its answer. */
typedef struct {
    T5577DownlinkMode downlink;
    const T5577Timing* timing; // NULL for t5577_timing_default(); read during the call only
    T5577ReadClock clock; // the rate the tag was configured to answer at
    // How long to let the envelope detector recover from the frame before listening, in
    // milliseconds; 0 takes the measured default. Like the timings, this is antenna-dependent
    // and the sort of thing a tool has to be able to tune without a new firmware.
    uint16_t settle_ms;
    uint32_t password; // with_pass only
    uint8_t page; // 0 or 1
    uint8_t block; // 0..7 on page 0, 0..3 on page 1
    bool with_pass;
} T5577ReadRequest;

/** What a block read came back with. */
typedef enum {
    T5577BlockHeld, // the block answered with the expected word
    T5577BlockMismatch, // the tag answered, but with something else
    T5577BlockNoAnswer, // nothing decodable came back at all
    T5577BlockCannotVerify, // nothing was decided: an unreadable clock, or a word that is
        // its own complement
} T5577BlockResult;

/** Bit rate and modulation a configuration word asks for, as a read clock.
 *
 * @param      block0  the configuration word the caller wrote to block 0
 * @return     the clock to read at, or T5577ReadClockUnsupported when the tag
 *             was configured to answer in a modulation or at a rate this
 *             decoder cannot follow
 */
T5577ReadClock t5577_read_clock_from_config(uint32_t block0);

/** Whether this reader can follow a tag configured by this word.
 *
 * It decodes ASK/Manchester at three rates and nothing else, so a tag given any other
 * configuration still takes writes and still answers in regular read mode - but no block of it
 * can be read back, which is the half a caller has to tell the user about.
 */
static inline bool t5577_config_readable(uint32_t block0) {
    return t5577_read_clock_from_config(block0) != T5577ReadClockUnsupported;
}

/** When a decoded run arrived, which is what lets two separate reads be lined up.
 *
 * The capture counter is zeroed when the read window opens, so these are times since that
 * instant - and comparable between reads, because the window opens a fixed interval after the
 * command frame and the tag's bit clock is divided from the field the Flipper itself generates.
 * Two reads of different blocks therefore put the same tag bit at the same time.
 *
 * This is the only thing that can line up blocks whose contents are unknown. A read hands back
 * the longest unbroken run it decoded, and where that run starts in the tag's stream varies from
 * read to read, so the bit offset a word was found at is not reusable; the time it arrived at is.
 */
typedef struct {
    uint32_t first_us; // transition that produced bits[0]
    uint32_t last_us; // and the one that produced the last bit
} T5577ReadMark;

/** Longest bit stream one block read can hand back, which bounds the buffer a caller needs.
 *
 * A read collects a bounded number of envelope periods and each yields at most two Manchester
 * events, so this is the ceiling regardless of bit rate.
 */
#define LFRFID_T5577_READ_BITS_MAX 512

/** Bytes a caller needs for LFRFID_T5577_READ_BITS_MAX packed bits. */
#define LFRFID_T5577_READ_BITS_BYTES ((LFRFID_T5577_READ_BITS_MAX + 7) / 8)

/** Address a block and decode its answer into a bit stream.
 *
 * For a caller that has to recover an *unknown* word, where t5577_block_holds() only confirms a
 * known one. The comparator offset is already subtracted and the Manchester already resolved;
 * what is left is the part no block read can settle by itself.
 *
 * Where the word starts is unknown, because the answer repeats with no frame marker. Block 0 is
 * the one block that can settle it on its own: it describes the modulation and bit rate the
 * answer arrived in, so the offset whose word agrees with how it was decoded - and which repeats
 * 32 bits later - is the real one. Every other block then reads at that same offset.
 *
 * Polarity is unresolved for the same reason: the stream may be the tag's bits or their
 * complement, and a caller testing a candidate has to try both.
 *
 * Only the longest unbroken run is returned, so a window taken anywhere inside it never
 * straddles a gap in the envelope.
 *
 * @param      request   block to address and the clock to read it at
 * @param[out] bits      packed most significant bit first, so a 32 bit window at `offset` is
 *                       bit_lib_get_bits_32(bits, offset, 32)
 * @param      max_bits  capacity of `bits`, at most LFRFID_T5577_READ_BITS_MAX
 * @param[out] mark      when the run arrived, for lining it up against another read. Optional
 * @return     bits written, 0 when nothing decodable came back
 */
size_t t5577_block_read_bits(
    const T5577ReadRequest* request,
    uint8_t* bits,
    size_t max_bits,
    T5577ReadMark* mark);

/** Bits in one block, which is also the period the answer repeats with. */
#define LFRFID_T5577_WORD_BITS 32

/** How long one tag bit lasts, which is eight field clocks whatever the rate. */
static inline uint32_t t5577_bit_us(T5577ReadClock clock) {
    return (uint32_t)clock * 8;
}

/** How a tag reads, worked out from the tag itself.
 *
 * Everything needed to turn a later read of any block into its word, and nothing a caller could
 * have known in advance: the rate the answer arrives at, which way round it arrives, and where a
 * word begins. All three come from block 0, which is the only block that describes itself.
 */
typedef struct {
    uint32_t block0; // configuration word as the tag holds it
    T5577ReadClock clock; // rate its answer decoded at
    uint32_t word_us; // when a word begins, in us into a word since the read window opened
    bool inverted; // the answer arrives as the complement of the tag's bits
    uint32_t password; // the password that was needed, and will be needed again
    bool with_pass; // whether one was needed at all
} T5577Config;

/** True when a T5577Config holds a tag.
 *
 * A detect that fails leaves the struct zeroed, and a zeroed clock is not one of the
 * T5577ReadClock values, so this is the one question to ask of a config a caller kept.
 */
static inline bool t5577_config_found(const T5577Config* config) {
    return config->clock != T5577ReadClockUnsupported;
}

/** Work out how a tag reads, by reading the block that describes itself.
 *
 * Sweeps the supported rates and accepts a word that agrees with how it was decoded - it claims
 * the modulation and bit rate it actually arrived in, its reserved bits are zero, and it repeats
 * at the one word stride the answer has. That agreement is the test; presence is not, because a
 * tag goes on modulating whether or not it answered the command.
 *
 * @param[out] config  how to read this tag, zeroed when nothing was found - so
 *                     t5577_config_found() answers for a config a caller is holding on to
 * @return     true when a configuration was recovered
 */
bool t5577_detect(T5577Config* config);

/** As t5577_detect(), for a tag that will not answer without its password.
 *
 * The recovered configuration decides whether the password is kept: a chip that asks for
 * neither a password nor answer-on-request did not need one, and the config comes back as
 * though it had been opened without one - so later reads taken with it send no password
 * frames.
 *
 * @warning A passworded read and a plain write are both 38 bits - see t5577_build_frame() -
 * and the tag tells them apart by length alone. Sent to a tag with no password set, this is
 * parsed as a write of `password << 1` into block 0, the block it addressed, with the
 * password's bit 31 as block 0's lock bit, which is one-way. Only call it where a password
 * is genuinely expected.
 */
bool t5577_detect_with_pass(T5577Config* config, uint32_t password);

/** Read one block and return its word, lined up with what t5577_detect() found.
 *
 * The bit offset a word sat at in one read means nothing in the next, so the alignment is carried
 * in time instead: the read window opens a fixed interval after the command and the tag's bit
 * clock is divided from the Flipper's own carrier, which puts the same tag bit at the same time
 * in every read.
 *
 * Rejects an answer rather than guessing at it - a read that locked onto the wrong half of the
 * bit grid, or whose word does not still repeat, is read again.
 *
 * @param      config  from t5577_detect()
 * @param[out] value   the block's word
 * @return     true when a word was recovered
 */
bool t5577_block_value(const T5577Config* config, uint8_t page, uint8_t block, uint32_t* value);

/** Confirm a T5577 block holds an expected word.
 *
 * Addresses the block, then listens: the tag repeats the addressed word for as
 * long as the field is up, so the expected value turning up anywhere in the
 * stream is the proof. This deliberately verifies rather than reads - the answer
 * carries no frame marker, so recovering an *unknown* word needs the
 * Proxmark3 offset/sequence-terminator machinery, which does not belong in the
 * firmware.
 *
 * Only ASK/Manchester at the rates T5577ReadClock names is decoded, and which
 * polarity the detector settled on is unknowable without a frame marker, so
 * `expected` is matched against the stream and against its complement. A word
 * that is its own complement therefore cannot be told from its inverse, and
 * 0x00000000 and 0xFFFFFFFF are each other's - neither can be verified, and
 * both come back as T5577BlockCannotVerify. Asking is reasonable: a restore
 * checks the words a dump holds, and a dump legitimately holds zero.
 *
 * Raises the field if it is not already up, so it works standalone or inside a field a
 * caller is holding across several operations.
 *
 * @param      request   block to address and the clock to read it at
 * @param      expected  the word the block should hold. 0 and 0xFFFFFFFF are each other's
 *                       complement, so neither can be told from its own inverse and both
 *                       come back as T5577BlockCannotVerify rather than a guess
 * @return     which of the four outcomes happened
 */
T5577BlockResult t5577_block_holds(const T5577ReadRequest* request, uint32_t expected);

#ifdef __cplusplus
}
#endif
