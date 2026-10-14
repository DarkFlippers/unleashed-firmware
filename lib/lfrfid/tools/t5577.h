#pragma once
#include <stdint.h>
#include <stdbool.h>

#ifdef __cplusplus
extern "C" {
#endif

#define LFRFID_T5577_BLOCK_COUNT 8
#define T5577_BLOCKS_IN_PAGE_0   8
#define T5577_BLOCKS_IN_PAGE_1   4

// T5577 block 0 definitions, thanks proxmark3!
#define LFRFID_T5577_POR_DELAY             0x00000001
#define LFRFID_T5577_ST_TERMINATOR         0x00000008
#define LFRFID_T5577_PWD                   0x00000010
#define LFRFID_T5577_MAXBLOCK_SHIFT        5
#define LFRFID_T5577_AOR                   0x00000200
#define LFRFID_T5577_PSKCF_RF_2            0
#define LFRFID_T5577_PSKCF_RF_4            0x00000400
#define LFRFID_T5577_PSKCF_RF_8            0x00000800
// Modulation occupies bits 12..16 of block 0 and the bit rate bits 18..20; the named values
// below are field values, so isolating either field needs its mask.
#define LFRFID_T5577_MODULATION_MASK       0x0001F000
#define LFRFID_T5577_BITRATE_MASK          0x001C0000
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

typedef struct {
    uint8_t page; // 0 or 1
    uint8_t block; // 0..7 on page 0, 0..3 on page 1
    bool with_pass;
    uint32_t password;
    T5577ReadClock clock;
} T5577ReadRequest;

/** Bit rate and modulation a configuration word asks for, as a read clock.
 *
 * @param      block0  the configuration word the caller wrote to block 0
 * @return     the clock to read at, or T5577ReadClockUnsupported when the tag
 *             was configured to answer in a modulation this decoder cannot read
 */
T5577ReadClock t5577_read_clock_from_config(uint32_t block0);

/** Confirm a T5577 block holds an expected word.
 *
 * Addresses the block, then listens: the tag repeats the addressed word for as
 * long as the field is up, so the expected value turning up anywhere in the
 * stream is the proof. This deliberately verifies rather than reads - the answer
 * carries no frame marker, so recovering an *unknown* word needs the
 * Proxmark3 offset/sequence-terminator machinery, which does not belong in the
 * firmware.
 *
 * Only ASK/Manchester at RF/16, RF/32 and RF/64 is decoded. A tag configured for
 * PSK or FSK answers in a modulation this cannot read, so pass a clock from
 * t5577_read_clock_from_config() and treat T5577ReadClockUnsupported as "cannot
 * verify" rather than as a failed write.
 *
 * @param      request   block to address and the clock to read it at
 * @param      expected  the word the block should hold
 * @return     true if the block answered with the expected word
 */
bool t5577_block_holds(const T5577ReadRequest* request, uint32_t expected);

#ifdef __cplusplus
}
#endif
