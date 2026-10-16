#include "t5577.h"
#include <furi.h>
#include <furi_hal_rfid.h>
#include <stddef.h>
#include <toolbox/manchester_decoder.h>

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
// carrier. This is the same 450 ms the read worker waits from a cold field
// (LFRFID_WORKER_READ_STABILIZE_TIME_MS), and it is not conservatism: measured at RF/64, the edge
// *count* is back to normal by 150 ms, but the detector's DC level keeps drifting, and a capture
// armed at 200 ms reads half-bit runs as 436 us against a 256 us nominal - far enough out that
// 203 of 208 runs classify as nothing at all. At 450 ms the same signal gives 118 clean bits with
// one unclassifiable run. Shortening this does not cost a few bits, it costs every bit.
#define T5577_READ_SETTLE_MS 450

/**************************************************************************************************/
/********************************************** FIELD *********************************************/
/**************************************************************************************************/

// Nesting depth, so a sequence of frames settles the detector once instead of per frame. Not
// thread safe, and does not need to be: a transaction owns the reader for its duration.
static uint8_t t5577_field_depth = 0;

void t5577_field_start(void) {
    furi_check(t5577_field_depth < UINT8_MAX);

    if(t5577_field_depth++ > 0) return;

    furi_hal_rfid_tim_read_start(125000, 0.5);

    // Do not ground the antenna: a gap has to be able to collapse the field.
    furi_hal_rfid_pin_pull_release();
}

void t5577_field_stop(void) {
    furi_check(t5577_field_depth > 0);

    if(--t5577_field_depth > 0) return;

    furi_hal_rfid_tim_read_stop();
    furi_hal_rfid_pins_reset();
}

/**************************************************************************************************/
/********************************************** SEND **********************************************/
/**************************************************************************************************/

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

typedef struct {
    uint8_t bit[T5577_FRAME_BITS_MAX];
    uint8_t count;
} T5577Bitstream;

static void t5577_bits_add(T5577Bitstream* bs, uint32_t value, uint8_t count) {
    furi_check((bs->count + count) <= T5577_FRAME_BITS_MAX);
    while(count-- > 0) {
        bs->bit[bs->count++] = (value >> count) & 1;
    }
}

// Symbol lengths are indexed by the symbol itself, which is what makes 1-of-4 a change of
// alphabet rather than a change of encoder.
static void t5577_send_symbol(uint8_t symbol, const T5577Timing* timing, uint16_t extra) {
    const uint16_t length[] = {
        timing->write_0,
        timing->write_1,
        timing->write_2,
        timing->write_3,
    };

    furi_delay_us((length[symbol] + extra) * 8);
    t5577_write_gap(timing->write_gap);
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
        frame->block < ((frame->page == 1) ? T5577_BLOCKS_IN_PAGE_1 : T5577_BLOCKS_IN_PAGE_0));
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

    if(frame->downlink == T5577DownlinkLongLeadingReference) {
        t5577_send_symbol(0, timing, T5577_LLR_REFERENCE);
    }

    for(uint8_t i = 0; i < bs.count; i += (wide ? 2 : 1)) {
        t5577_send_symbol(wide ? ((bs.bit[i] << 1) | bs.bit[i + 1]) : bs.bit[i], timing, 0);
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

// Bits to fill the match window before the first comparison. The answer repeats every 32 bits,
// so sliding one bit at a time from here covers every alignment, and a wrong one can only delay
// a match, never invent one.
#define T5577_READ_MIN_BITS 32

// Decoded bits that count as "a tag is talking", for telling a wrong answer from no answer.
#define T5577_READ_ANSWER_BITS 8

// 32 bits to fill plus up to 32 more to slide into alignment is ~33 ms at RF/64. The rest is
// headroom for the tag to start answering and for a glitch to cost a resync.
#define T5577_READ_TIMEOUT_MS 150

// Give up earlier when no coherent run has appeared at all. A password sweep pays this on every wrong
// password, where waiting out the whole window costs tens of seconds.
#define T5577_READ_SILENCE_MS 30

// Below this a run is noise, not envelope. Same floor the read worker uses.
#define T5577_READ_MIN_TIME_US 16

// Envelope runs are classified against the bit period with the same +-20% the EM4100 decoder
// uses (100 us on a 512 us bit), which is what makes those timings proven rather than guessed.
#define T5577_READ_JITTER_DIV 5

typedef struct {
    ManchesterState state;
    uint32_t short_min;
    uint32_t short_max;
    uint32_t long_min;
    uint32_t long_max;
    uint32_t pulse; // last high run, to recover the low one from the period that follows it
    bool pulse_valid;
    bool ignore_next_pulse;
    uint32_t shift_reg;
    uint8_t bits;
    uint32_t expected;
    uint16_t run; // consecutive bits since the last break in the envelope
    volatile uint16_t best; // longest such run, which is the evidence that a tag was talking
    volatile bool found;
} T5577Reader;

static void t5577_reader_init(T5577Reader* reader, T5577ReadClock clock, uint32_t expected) {
    // One field clock is 8 us at 125 kHz, so a bit is clock*8 us and a Manchester half-bit is
    // half of that.
    const uint32_t bit_us = (uint32_t)clock * 8;
    const uint32_t jitter = bit_us / T5577_READ_JITTER_DIV;

    *reader = (T5577Reader){
        .state = ManchesterStateMid1,
        .short_min = (bit_us / 2) - jitter,
        .short_max = (bit_us / 2) + jitter,
        .long_min = bit_us - jitter,
        .long_max = bit_us + jitter,
        .expected = expected,
    };
}

// Any break in the envelope starts the window over: a comparison must not straddle the gap, and
// the run of bits behind it is no longer evidence of anything. Counting bits across breaks
// instead reaches any threshold on noise alone, which is what makes a "no answer" verdict mean
// something here.
static void t5577_reader_restart(T5577Reader* reader) {
    reader->bits = 0;
    reader->run = 0;
    reader->pulse_valid = false;
}

// Runs in the capture interrupt, so it stays branch-light: recover the run, classify it, advance
// one Manchester step, compare.
static void t5577_read_capture(bool level, uint32_t duration, void* context) {
    T5577Reader* reader = context;
    if(reader->found) return;

    // Noise rejection, as the read worker does it: a spike goes, and so does the run it split,
    // because that width is no longer the tag's.
    if(duration <= T5577_READ_MIN_TIME_US) {
        if(level) reader->ignore_next_pulse = true;
        t5577_reader_restart(reader);
        return;
    }

    if(reader->ignore_next_pulse) {
        reader->ignore_next_pulse = false;
        t5577_reader_restart(reader);
        return;
    }

    // The capture reports a high run with the level set and the whole rise-to-rise period with it
    // clear. The windows below are envelope runs, so the low one has to be recovered from the
    // period - which is what the read worker does before feeding its decoders. Feeding the raw
    // period instead puts every low run a whole bit out, and nothing ever matches.
    if(level) {
        reader->pulse = duration;
        reader->pulse_valid = true;
    } else {
        // A period no longer than its own high run means the pairing is torn, so the stored run
        // belongs to a different period and must not be subtracted from the next one either.
        if(!reader->pulse_valid || duration <= reader->pulse) {
            t5577_reader_restart(reader);
            return;
        }
        duration -= reader->pulse;
        reader->pulse_valid = false;
    }

    ManchesterEvent event;

    // Same convention as the protocol decoders: a high run arrives with the level set and counts
    // as a Low event, so the sense reads inverted here.
    if(duration > reader->short_min && duration < reader->short_max) {
        event = level ? ManchesterEventShortLow : ManchesterEventShortHigh;
    } else if(duration > reader->long_min && duration < reader->long_max) {
        event = level ? ManchesterEventLongLow : ManchesterEventLongHigh;
    } else {
        // Unclassifiable: ignore it as the protocol decoders do, but start the window over so a
        // comparison never straddles the gap.
        t5577_reader_restart(reader);
        return;
    }

    bool data;
    if(!manchester_advance(reader->state, event, &reader->state, &data)) return;

    if(reader->run < UINT16_MAX) reader->run++;
    if(reader->run > reader->best) reader->best = reader->run;

    reader->shift_reg = (reader->shift_reg << 1) | data;

    if(reader->bits < T5577_READ_MIN_BITS) {
        reader->bits++;
        return;
    }

    // Which polarity the detector settled on says nothing about what the tag holds, and there is
    // no frame marker to resolve it - Proxmark3 carries an `inverted` flag it learned during
    // `t55xx detect`, a step this has no equivalent of. So both readings count. An inverted
    // decode is also offset, because manchester_advance resyncs to one fixed state either way,
    // but on a repeating answer an offset only costs the window a few more slides.
    if(reader->shift_reg == reader->expected || ~reader->shift_reg == reader->expected) {
        reader->found = true;
    }
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

T5577BlockResult t5577_block_holds(const T5577ReadRequest* request, uint32_t expected) {
    furi_check(request);
    furi_check(request->page <= 1);
    furi_check(
        request->block < ((request->page == 1) ? T5577_BLOCKS_IN_PAGE_1 : T5577_BLOCKS_IN_PAGE_0));
    // A word that is its own complement cannot be told from its inverse, and polarity is
    // unknowable here. Answering either way would be a guess dressed as a verdict.
    furi_check(expected != 0 && expected != UINT32_MAX);

    if(request->clock == T5577ReadClockUnsupported) {
        // Declining to look is not a verdict about the tag, and the caller can only pass it on.
        FURI_LOG_I(
            TAG, "p%u b%u not read: no clock for that config", request->page, request->block);
        return T5577BlockCannotVerify;
    }
    furi_check(
        request->clock == T5577ReadClock16 || request->clock == T5577ReadClock32 ||
        request->clock == T5577ReadClock64);

    T5577Reader reader;
    t5577_reader_init(&reader, request->clock, expected);

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

    // Field stays up: this is the detector recovering from the frame's gaps, and arming the
    // capture before it has is why a tag that was answering perfectly read as silence.
    furi_delay_ms(T5577_READ_SETTLE_MS);

    furi_hal_rfid_tim_read_capture_start(t5577_read_capture, &reader);

    const uint32_t start_tick = furi_get_tick();
    const uint32_t silence = furi_ms_to_ticks(T5577_READ_SILENCE_MS);
    const uint32_t timeout = furi_ms_to_ticks(T5577_READ_TIMEOUT_MS);

    while(!reader.found) {
        const uint32_t elapsed = furi_get_tick() - start_tick;
        if(elapsed >= timeout) break;
        if(elapsed >= silence && reader.best < T5577_READ_ANSWER_BITS) break;
        furi_delay_ms(1);
    }

    furi_hal_rfid_tim_read_capture_stop();

    furi_hal_rfid_pin_pull_release();
    t5577_back_to_read_mode(request->downlink, request->timing);

    t5577_field_stop();

    const T5577BlockResult result = reader.found                            ? T5577BlockHeld :
                                    (reader.best >= T5577_READ_ANSWER_BITS) ? T5577BlockMismatch :
                                                                              T5577BlockNoAnswer;

    // The only diagnostic there is behind a verdict the user sees as one word.
    FURI_LOG_I(
        TAG,
        "p%u b%u rf/%u pass:%08lX expect:%08lX -> %s, longest run %u bits",
        request->page,
        request->block,
        request->clock,
        request->with_pass ? request->password : 0,
        expected,
        (result == T5577BlockHeld)     ? "held" :
        (result == T5577BlockMismatch) ? "mismatch" :
                                         "no answer",
        reader.best);

    return result;
}

/**************************************************************************************************/
/********************************************** WRITE *********************************************/
/**************************************************************************************************/

static void t5577_write_block_simple(uint8_t block, bool lock_bit, uint32_t data) {
    t5577_write_block_pass(0, block, lock_bit, data, false, 0);
}

void t5577_write(LFRFIDT5577* data) {
    t5577_field_start();
    // Each frame holds interrupts off for itself rather than the whole sequence doing it once:
    // eight blocks is hundreds of milliseconds, far too long to leave the rest of the system
    // waiting, and every frame begins by charging the tag anyway so a pause between them is free.
    for(size_t i = 0; i < data->blocks_to_write; i++) {
        t5577_write_block_simple(i, false, data->block[i]);
    }
    t5577_back_to_read_mode(T5577DownlinkFixedBitLength, NULL);
    t5577_field_stop();
}

void t5577_write_with_pass(LFRFIDT5577* data, uint32_t password) {
    t5577_field_start();
    for(size_t i = 0; i < data->blocks_to_write; i++) {
        t5577_write_block_pass(0, i, false, data->block[i], true, password);
    }
    t5577_back_to_read_mode(T5577DownlinkFixedBitLength, NULL);
    t5577_field_stop();
}

void t5577_write_with_mask(LFRFIDT5577* data, uint8_t page, bool with_pass, uint32_t password) {
    t5577_field_start();

    uint8_t mask = data->mask;

    size_t pages_total = (page == 0) ? T5577_BLOCKS_IN_PAGE_0 : T5577_BLOCKS_IN_PAGE_1;

    for(size_t i = 0; i < pages_total; i++) {
        bool need_to_write = mask & 1;
        mask >>= 1;
        if(!need_to_write) continue;
        t5577_write_block_pass(page, i, false, data->block[i], with_pass, password);
    }
    t5577_back_to_read_mode(T5577DownlinkFixedBitLength, NULL);
    t5577_field_stop();
}
