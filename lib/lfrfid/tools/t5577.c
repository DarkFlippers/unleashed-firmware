#include "t5577.h"
#include <furi.h>
#include <furi_hal_rfid.h>
#include <stddef.h>
#include <toolbox/manchester_decoder.h>
#include <bit_lib/bit_lib.h>

#define TAG "T5577"

// Every timing here is in field clocks: one is 8 us at 125 kHz.
#define T5577_TIMING_WAIT_TIME 400
#define T5577_TIMING_PROGRAM   700

// A long leading reference stretches one symbol by this much (Proxmark3 T55_LLR_REF).
#define T5577_LLR_REFERENCE 136

// Longest frame is a 1-of-4 write with a password: 2 lead-in + 2 opcode + 2 password padding
// + 32 password + 1 lock + 32 data + 3 address = 74. Rounded up, with slack.
#define T5577_FRAME_BITS_MAX 80

// How long the envelope detector takes to recover from the gaps a command frame cuts into the
// carrier. The same 450 ms the read worker waits from a cold field
// (LFRFID_WORKER_READ_STABILIZE_TIME_MS), and measured here rather than inherited.
//
// The edge count recovers long before the signal does, so counting edges says this could be
// 150 ms. Decoding says otherwise: at RF/32 nothing usable comes back below 250 ms, and at
// 250 ms the comparator offset is still 100 us against the 54 us it settles to - enough that
// only 26 consecutive bits decode, where 450 ms gives 219. RF/64 is more forgiving, which is
// exactly why the shorter rate sets the number.
#define T5577_READ_SETTLE_MS 450

/**************************************************************************************************/
/********************************************** FIELD *********************************************/
/**************************************************************************************************/

// Nesting depth, so a sequence of frames raises the carrier once instead of dropping it between
// them - which would reset the tag. Not thread safe, and does not need to be: a transaction owns
// the reader for its duration. The count is this module's own bookkeeping, so a caller driving
// furi_hal_rfid directly desynchronises it; t5577_field_reset() is the way back.
static uint8_t t5577_field_depth = 0;

void t5577_field_start(void) {
    furi_check(t5577_field_depth < UINT8_MAX);

    if(t5577_field_depth++ > 0) return;

    furi_hal_rfid_tim_read_start(125000, 0.5);

    // Do not ground the antenna: a gap has to be able to collapse the field.
    furi_hal_rfid_pin_pull_release();
}

void t5577_field_stop(void) {
    // Not fatal: the count is this module's bookkeeping, and a third party can desynchronise
    // it through the exported HAL. Crashing the device over its arithmetic is not warranted.
    if(t5577_field_depth == 0) {
        FURI_LOG_W(TAG, "field stopped more times than started");
        return;
    }

    if(--t5577_field_depth > 0) return;

    furi_hal_rfid_tim_read_stop();
    furi_hal_rfid_pins_reset();
}

/**************************************************************************************************/
/********************************************** SEND **********************************************/
/**************************************************************************************************/

void t5577_field_reset(void) {
    t5577_field_depth = 0;
    furi_hal_rfid_tim_read_stop();
    furi_hal_rfid_pins_reset();
}

static void t5577_write_gap(uint32_t gap_time) {
    furi_hal_rfid_tim_read_pause();
    furi_delay_us(gap_time * 8);
    furi_hal_rfid_tim_read_continue();
}

static const T5577Timing t5577_timing[T5577DownlinkModeMax] = {
    // What every write this driver has sent used.
    [T5577DownlinkFixedBitLength] =
        {.start_gap = 30, .write_gap = 18, .write_0 = 24, .write_1 = 56},
    // Proxmark3 gives its long-leading-reference row the same lengths as its fixed row, so these
    // match deliberately rather than by omission.
    [T5577DownlinkLongLeadingReference] =
        {.start_gap = 30, .write_gap = 18, .write_0 = 24, .write_1 = 56},
    [T5577DownlinkLeadingZero] = {.start_gap = 30, .write_gap = 18, .write_0 = 18, .write_1 = 40},
    [T5577Downlink1Of4] =
        {.start_gap = 30,
         .write_gap = 18,
         .write_0 = 18,
         .write_1 = 34,
         .write_2 = 50,
         .write_3 = 66},
};

const T5577Timing* t5577_timing_default(T5577DownlinkMode mode) {
    furi_check(mode < T5577DownlinkModeMax);
    return &t5577_timing[mode];
}

// Packed, as hitags.c and hitagmicro.c build their frames: a byte per bit cost 81 bytes of
// stack inside the critical section where ten will do.
typedef struct {
    uint8_t bit[(T5577_FRAME_BITS_MAX + 7) / 8];
    uint8_t count;
} T5577Bitstream;

static void t5577_bits_add(T5577Bitstream* bs, uint32_t value, uint8_t count) {
    furi_check((bs->count + count) <= T5577_FRAME_BITS_MAX);

    uint8_t be[4];
    bit_lib_num_to_bytes_be((uint32_t)(value << (32 - count)), sizeof(be), be);
    bit_lib_copy_bits(bs->bit, bs->count, count, be, 0);
    bs->count += count;
}

static void t5577_send_symbol(uint16_t length, uint16_t write_gap) {
    furi_delay_us(length * 8);
    t5577_write_gap(write_gap);
}

static void t5577_build_frame(const T5577Frame* frame, T5577Bitstream* bs) {
    const bool wide = (frame->downlink == T5577Downlink1Of4);

    // Both of these modes are told apart from a fixed-length frame by what precedes the opcode.
    if(frame->downlink == T5577DownlinkLeadingZero) {
        t5577_bits_add(bs, 0, 1);
    } else if(wide) {
        t5577_bits_add(bs, 0, 2); // one 1-of-4 symbol of zero
    }

    if(frame->command == T5577CommandReset) {
        t5577_bits_add(bs, 0, 2);
        return;
    }

    // First opcode bit says "not test mode", second selects the page.
    t5577_bits_add(bs, 1, 1);
    t5577_bits_add(bs, frame->page ? 1 : 0, 1);

    if(frame->with_pass) {
        // Those same two modes need the opcode padded out before a password follows it.
        if(frame->downlink == T5577DownlinkLeadingZero || wide) {
            t5577_bits_add(bs, 0, 2);
        }
        t5577_bits_add(bs, frame->password, 32);
    }

    // A regular read is the opcode alone: it asks the tag to go back to broadcasting its data
    // blocks, so there is nothing to address.
    if(frame->command == T5577CommandRegularRead) return;

    t5577_bits_add(bs, frame->lock ? 1 : 0, 1);

    if(frame->command == T5577CommandWrite) {
        t5577_bits_add(bs, frame->data, 32);
    }

    t5577_bits_add(bs, frame->block, 3);
}

// True for the frames that carry a block address, and so the ones a bad block number can misdirect.
static bool t5577_command_is_addressed(T5577Command command) {
    return (command == T5577CommandRead) || (command == T5577CommandWrite);
}

void t5577_send(const T5577Frame* frame) {
    furi_check(frame);
    furi_check(frame->downlink < T5577DownlinkModeMax);
    furi_check(frame->page <= 1);
    // The address is three bits wide, so an out of range block would not fail - it would quietly
    // address a different one, and on a write that is a corrupted tag.
    furi_check(
        !t5577_command_is_addressed(frame->command) ||
        frame->block <
            ((frame->page == 1) ? LFRFID_T5577_BLOCKS_IN_PAGE_1 : LFRFID_T5577_BLOCKS_IN_PAGE_0));
    furi_check(t5577_field_depth > 0);

    const T5577Timing* timing = (frame->timing != NULL) ? frame->timing :
                                                          t5577_timing_default(frame->downlink);

    const bool wide = (frame->downlink == T5577Downlink1Of4);

    // A zero length is not a short symbol, it is no symbol: the frame goes out malformed and
    // reports nothing. Worth checking on the one struct meant to be filled in by hand.
    furi_check(timing->start_gap && timing->write_gap && timing->write_0 && timing->write_1);
    furi_check(!wide || (timing->write_2 && timing->write_3));

    T5577Bitstream bs = {0};
    t5577_build_frame(frame, &bs);

    // Two bits per symbol only divides a frame that holds an even number of them. Proxmark3
    // drops the odd bit; every frame built above is even, so an odd one is a bug, not a tag.
    furi_check(!wide || (bs.count % 2) == 0);

    FURI_CRITICAL_ENTER();

    // Let the tag charge up before cutting the field, or it answers nothing.
    furi_delay_us(T5577_TIMING_WAIT_TIME * 8);

    t5577_write_gap(timing->start_gap);

    // Indexed by the symbol itself, which is what makes 1-of-4 a change of alphabet rather than
    // a change of encoder. Built once per frame, not once per symbol.
    const uint16_t length[] = {
        timing->write_0,
        timing->write_1,
        timing->write_2,
        timing->write_3,
    };
    const uint8_t step = wide ? 2 : 1;

    if(frame->downlink == T5577DownlinkLongLeadingReference) {
        t5577_send_symbol(timing->write_0 + T5577_LLR_REFERENCE, timing->write_gap);
    }

    for(uint8_t i = 0; i < bs.count; i += step) {
        const uint8_t symbol = wide ? bit_lib_get_bits(bs.bit, i, 2) : bit_lib_get_bit(bs.bit, i);
        t5577_send_symbol(length[symbol], timing->write_gap);
    }

    // The tag burns the word into its EEPROM here, and needs the field all the way through.
    if(frame->command == T5577CommandWrite) {
        furi_delay_us(T5577_TIMING_PROGRAM * 8);
    }

    FURI_CRITICAL_EXIT();
}

// Putting the tag back into regular read mode. Note this is not the chip's reset opcode: a tag
// that has been asked for one block keeps repeating it, and what stops that is being told to
// broadcast its data blocks again.
static void t5577_back_to_read_mode(T5577DownlinkMode downlink, const T5577Timing* timing) {
    t5577_send(&(T5577Frame){
        .command = T5577CommandRegularRead,
        .downlink = downlink,
        .timing = timing,
    });
}

static void t5577_write_block_pass(
    uint8_t page,
    uint8_t block,
    bool lock_bit,
    uint32_t data,
    bool with_pass,
    uint32_t password) {
    t5577_send(&(T5577Frame){
        .command = T5577CommandWrite,
        .page = page,
        .block = block,
        .lock = lock_bit,
        .data = data,
        .with_pass = with_pass,
        .password = password,
    });

    t5577_back_to_read_mode(T5577DownlinkFixedBitLength, NULL);
}

/**************************************************************************************************/
/********************************************** READ **********************************************/
/**************************************************************************************************/

// Pairs to collect before decoding. One per envelope period, and a pair is worth about one
// decoded bit, so this covers the 64 bits every alignment of a 32 bit answer needs several times
// over: 199 pairs measured in a 60 ms window at RF/32 decoded to 219 bits.
#define T5577_READ_PAIRS_MAX 256

// Consecutive decoded bits that count as "a tag is talking", for telling a wrong answer from
// no answer at all. Consecutive and decoded, not a tally of plausible runs: a raw count
// reaches any threshold on comparator chatter alone, and a quarter of a word is little
// enough to be a coincidence, so this is a full repetition.
#define T5577_READ_ANSWER_BITS 32

// Long enough for the buffer to fill at the slowest rate: 256 pairs at RF/64 is ~131 ms.
#define T5577_READ_TIMEOUT_MS 150

// Give up earlier when nothing has arrived at all. A password sweep pays this on every wrong
// password, where waiting out the whole window costs tens of seconds.
#define T5577_READ_SILENCE_MS 30

// Below this a run is noise, not envelope. Same floor the read worker uses.
#define T5577_READ_MIN_TIME_US 16

// Runs are classified against the bit period with the same +-20% the EM4100 decoder uses (100 us
// on a 512 us bit), which is what makes those timings proven rather than guessed.
#define T5577_READ_JITTER_DIV 5

// The envelope as the comparator reported it: one high run and the low run that followed it.
//
// The interrupt only stores. Everything that has to see the signal as a whole - the comparator
// offset above all - happens afterwards, where it can look at every pair at once.
typedef struct {
    uint16_t high[T5577_READ_PAIRS_MAX];
    uint16_t low[T5577_READ_PAIRS_MAX];
    uint32_t pulse;
    bool pulse_valid;
    volatile uint16_t count;
} T5577Capture;

static void t5577_read_capture(bool level, uint32_t duration, void* context) {
    T5577Capture* capture = context;
    if(capture->count >= T5577_READ_PAIRS_MAX) return;

    // A high run arrives with the level set; the whole rise-to-rise period arrives with it clear,
    // so the low run is what is left of the period. This is the pairing the read worker does with
    // its varint pairs before feeding its decoders.
    if(level) {
        capture->pulse = duration;
        capture->pulse_valid = true;
        return;
    }

    // A period no longer than its own high run means the pairing is torn, and the stored run must
    // not be subtracted from the next period either.
    if(!capture->pulse_valid || duration <= capture->pulse) {
        capture->pulse_valid = false;
        return;
    }

    const uint32_t high = capture->pulse;
    const uint32_t low = duration - capture->pulse;
    capture->pulse_valid = false;

    if(high > UINT16_MAX || low > UINT16_MAX) return;

    capture->high[capture->count] = (uint16_t)high;
    capture->low[capture->count] = (uint16_t)low;
    capture->count++;
}

// A pair worth averaging and decoding. Runs longer than two bit periods are the detector settling
// or a glitch, and letting them into the average would move it further than the offset being
// measured.
static bool t5577_pair_plausible(uint16_t high, uint16_t low, uint32_t bit_us) {
    const uint32_t limit = bit_us * 2;
    return (high > T5577_READ_MIN_TIME_US) && (low > T5577_READ_MIN_TIME_US) &&
           ((uint32_t)high < limit) && ((uint32_t)low < limit);
}

// What the comparator adds to every high run and takes off every low one.
//
// It reports a high run longer than it is, by a margin set by the detector and the coupling rather
// than by the bit rate: measured 52 us against a 128 us half-bit at RF/32, 68 us against 256 us at
// RF/64, and 141 us on a capture armed before the detector had settled. A Manchester answer spends
// equal time high and low, so whatever is left after averaging both is the comparator and not the
// tag.
//
// Subtracting it is what makes the short rates decode at all. At RF/32 a 52 us offset puts the
// two runs at 180 and 76 us, and the windows either side of a 128 us half-bit accept neither:
// uncorrected, that capture recovered 4 consecutive bits, and corrected it recovered 219.
static int32_t t5577_capture_bias(const T5577Capture* capture, uint32_t bit_us) {
    int32_t high_sum = 0;
    int32_t low_sum = 0;
    uint16_t used = 0;

    for(uint16_t i = 0; i < capture->count; i++) {
        if(!t5577_pair_plausible(capture->high[i], capture->low[i], bit_us)) continue;
        high_sum += capture->high[i];
        low_sum += capture->low[i];
        used++;
    }

    if(used == 0) return 0;
    return (high_sum - low_sum) / (2 * (int32_t)used);
}

typedef struct {
    uint16_t pairs; // plausible pairs seen, which is the evidence that a tag was talking
    uint16_t best; // longest run of consecutive decoded bits
    bool found;
} T5577Decoded;

static T5577Decoded
    t5577_capture_decode(const T5577Capture* capture, uint32_t bit_us, uint32_t expected) {
    const int32_t bias = t5577_capture_bias(capture, bit_us);
    const uint32_t jitter = bit_us / T5577_READ_JITTER_DIV;
    const int32_t short_min = (int32_t)(bit_us / 2 - jitter);
    const int32_t short_max = (int32_t)(bit_us / 2 + jitter);
    const int32_t long_min = (int32_t)(bit_us - jitter);
    const int32_t long_max = (int32_t)(bit_us + jitter);

    ManchesterState state = ManchesterStateMid1;
    uint32_t shift_reg = 0;
    uint8_t bits = 0;
    uint16_t run = 0;
    T5577Decoded out = {0};

    for(uint16_t i = 0; i < capture->count; i++) {
        if(!t5577_pair_plausible(capture->high[i], capture->low[i], bit_us)) {
            // A break in the envelope: start the window over so a comparison never straddles it.
            bits = 0;
            run = 0;
            continue;
        }
        out.pairs++;

        const int32_t corrected[] = {
            (int32_t)capture->high[i] - bias,
            (int32_t)capture->low[i] + bias,
        };

        for(uint8_t half = 0; half < 2; half++) {
            const bool from_high_run = (half == 0);
            const int32_t value = corrected[half];
            ManchesterEvent event;

            // Same convention as the protocol decoders: a high run counts as a Low event, so the
            // sense reads inverted here.
            if(value > short_min && value < short_max) {
                event = from_high_run ? ManchesterEventShortLow : ManchesterEventShortHigh;
            } else if(value > long_min && value < long_max) {
                event = from_high_run ? ManchesterEventLongLow : ManchesterEventLongHigh;
            } else {
                bits = 0;
                run = 0;
                continue;
            }

            bool data;
            if(!manchester_advance(state, event, &state, &data)) continue;

            run++;
            if(run > out.best) out.best = run;

            shift_reg = (shift_reg << 1) | (data ? 1 : 0);

            if(bits < 32) {
                bits++;
                continue;
            }

            // Which polarity the detector settled on says nothing about what the tag holds, and
            // there is no frame marker to resolve it - Proxmark3 carries an inverted flag it
            // learned during `t55xx detect`, a step this has no equivalent of. So both readings
            // count, and the answer repeating every 32 bits means every alignment gets examined.
            if(shift_reg == expected || ~shift_reg == expected) {
                out.found = true;
                return out;
            }
        }
    }

    return out;
}

T5577ReadClock t5577_read_clock_from_config(uint32_t block0) {
    // Only ASK/Manchester, and only at a rate the decoder is proven at. Direct, biphase and
    // diphase are ASK too and are excluded with it: none of them puts a transition mid-bit for
    // the decoder to lock onto. PSK and FSK are a different modulation entirely.
    if((block0 & LFRFID_T5577_MODULATION_MASK) != LFRFID_T5577_MODULATION_MANCHESTER) {
        FURI_LOG_W(
            TAG, "Modulation %08lX is not ASK/Manchester", block0 & LFRFID_T5577_MODULATION_MASK);
        return T5577ReadClockUnsupported;
    }

    switch(block0 & LFRFID_T5577_BITRATE_MASK) {
    case LFRFID_T5577_BITRATE_RF_16:
        return T5577ReadClock16;
    case LFRFID_T5577_BITRATE_RF_32:
        return T5577ReadClock32;
    case LFRFID_T5577_BITRATE_RF_64:
        return T5577ReadClock64;
    default:
        FURI_LOG_W(
            TAG, "Bit rate %08lX is not RF/16, 32 or 64", block0 & LFRFID_T5577_BITRATE_MASK);
        return T5577ReadClockUnsupported;
    }
}

// Address the block and collect whatever the tag says back. Shared by the verifier and the
// answer probe, which differ only in what they do with the capture afterwards.
static bool t5577_block_capture(const T5577ReadRequest* request, T5577Capture* capture) {
    if(request->clock == T5577ReadClockUnsupported) return false;
    furi_check(
        request->clock == T5577ReadClock16 || request->clock == T5577ReadClock32 ||
        request->clock == T5577ReadClock64);

    t5577_field_start();

    t5577_send(&(T5577Frame){
        .command = T5577CommandRead,
        .downlink = request->downlink,
        .timing = request->timing,
        .page = request->page,
        .block = request->block,
        .with_pass = request->with_pass,
        .password = request->password,
    });

    // The frame left the pull pin where a write wants it; the read path drives it the other way
    // (furi_hal_rfid_pins_read), and this is the read path now.
    furi_hal_rfid_pin_pull_pulldown();

    // Field stays up: this is the detector recovering from the frame gaps, and arming the capture
    // before it has is why a tag that was answering perfectly read as silence. Measured at RF/32:
    // nothing usable below 250 ms, and at 250 ms the offset is still twice what it settles to.
    furi_delay_ms((request->settle_ms != 0) ? request->settle_ms : T5577_READ_SETTLE_MS);

    furi_hal_rfid_tim_read_capture_start(t5577_read_capture, capture);

    const uint32_t start_tick = furi_get_tick();
    const uint32_t silence = furi_ms_to_ticks(T5577_READ_SILENCE_MS);
    const uint32_t timeout = furi_ms_to_ticks(T5577_READ_TIMEOUT_MS);

    while(capture->count < T5577_READ_PAIRS_MAX) {
        const uint32_t elapsed = furi_get_tick() - start_tick;
        if(elapsed >= timeout) break;
        if(elapsed >= silence && capture->count == 0) break;
        furi_delay_ms(1);
    }

    furi_hal_rfid_tim_read_capture_stop();

    furi_hal_rfid_pin_pull_release();
    t5577_back_to_read_mode(request->downlink, request->timing);

    t5577_field_stop();

    return true;
}

// Covers every value, so adding one is a -Wswitch warning rather than a silent "no answer".
static const char* t5577_block_result_name(T5577BlockResult result) {
    switch(result) {
    case T5577BlockHeld:
        return "held";
    case T5577BlockMismatch:
        return "mismatch";
    case T5577BlockNoAnswer:
        return "no answer";
    case T5577BlockCannotVerify:
        return "cannot verify";
    }
    return "?";
}

T5577BlockResult t5577_block_holds(const T5577ReadRequest* request, uint32_t expected) {
    furi_check(request);
    furi_check(request->page <= 1);
    furi_check(
        request->block <
        ((request->page == 1) ? LFRFID_T5577_BLOCKS_IN_PAGE_1 : LFRFID_T5577_BLOCKS_IN_PAGE_0));
    // Two ways to have nothing to say, neither of them the tag's fault: a word that is its
    // own complement cannot be told from its inverse, because polarity is unknowable here, and a
    // clock this decoder cannot follow was never going to produce bits. Both are verdicts, not
    // crashes - `expected` is data a caller may well have got from a tag, and verifying that a
    // wiped data block reads back as zero is a reasonable thing to ask for.
    if(expected == 0 || expected == UINT32_MAX) {
        FURI_LOG_W(TAG, "%08lX cannot be told from its own inverse", expected);
        return T5577BlockCannotVerify;
    }

    if(request->clock != T5577ReadClock16 && request->clock != T5577ReadClock32 &&
       request->clock != T5577ReadClock64) {
        FURI_LOG_I(
            TAG, "p%u b%u not read: no clock for that config", request->page, request->block);
        return T5577BlockCannotVerify;
    }

    T5577Capture* capture = malloc(sizeof(T5577Capture));

    if(!t5577_block_capture(request, capture)) {
        free(capture);
        return T5577BlockCannotVerify;
    }

    const uint32_t bit_us = (uint32_t)request->clock * 8;
    const int32_t bias = t5577_capture_bias(capture, bit_us);
    const T5577Decoded decoded = t5577_capture_decode(capture, bit_us, expected);

    T5577BlockResult result = T5577BlockNoAnswer;
    if(decoded.found) {
        result = T5577BlockHeld;
    } else if(decoded.best >= T5577_READ_ANSWER_BITS) {
        result = T5577BlockMismatch;
    }

    // The only diagnostic there is behind a verdict the user sees as one word.
    FURI_LOG_I(
        TAG,
        "p%u b%u rf/%u pass:%08lX expect:%08lX -> %s, %u pairs, bias %ld, longest run %u",
        request->page,
        request->block,
        request->clock,
        request->with_pass ? request->password : 0,
        expected,
        t5577_block_result_name(result),
        decoded.pairs,
        bias,
        decoded.best);

    free(capture);

    return result;
}

/**************************************************************************************************/
/********************************************** WRITE *********************************************/
/**************************************************************************************************/

// Every write entry point is this loop: the masked blocks of one page, each in its own frame,
// under one field. Interrupts are held per frame rather than across the sequence - eight blocks
// is hundreds of milliseconds, and every frame begins by charging the tag anyway, so a pause
// between them costs nothing.
static void t5577_write_blocks(
    LFRFIDT5577* data,
    uint8_t page,
    uint8_t mask,
    bool with_pass,
    uint32_t password) {
    const size_t blocks = (page == 0) ? LFRFID_T5577_BLOCKS_IN_PAGE_0 :
                                        LFRFID_T5577_BLOCKS_IN_PAGE_1;

    t5577_field_start();

    for(size_t i = 0; i < blocks; i++) {
        if(!(mask & (1u << i))) continue;
        t5577_write_block_pass(page, i, false, data->block[i], with_pass, password);
    }

    t5577_back_to_read_mode(T5577DownlinkFixedBitLength, NULL);
    t5577_field_stop();
}

// blocks_to_write counts from block 0, so the mask is that many low bits.
static uint8_t t5577_mask_from_count(uint32_t blocks_to_write) {
    if(blocks_to_write >= LFRFID_T5577_BLOCKS_IN_PAGE_0) return 0xFF;
    return (uint8_t)((1u << blocks_to_write) - 1);
}

void t5577_write(LFRFIDT5577* data) {
    t5577_write_blocks(data, 0, t5577_mask_from_count(data->blocks_to_write), false, 0);
}

void t5577_write_with_pass(LFRFIDT5577* data, uint32_t password) {
    t5577_write_blocks(data, 0, t5577_mask_from_count(data->blocks_to_write), true, password);
}

void t5577_write_with_mask(LFRFIDT5577* data, uint8_t page, bool with_pass, uint32_t password) {
    t5577_write_blocks(data, page, data->mask, with_pass, password);
}
