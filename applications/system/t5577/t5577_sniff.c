#include "t5577_sniff.h"

#include <furi_hal.h>
#include <stm32wbxx_ll_tim.h>

/** A stall longer than this is a gap rather than the time between two carrier edges.
 *
 * One carrier period is 8 us and the shortest downlink gap in the band below is five of them, so
 * anything from about 16 us up separates a gap from the quiet between two edges. 20 leaves room
 * for a carrier that is off frequency.
 */
#define T5577_SNIFF_THRESHOLD_US 20

/** The width band a downlink gap falls in.
 *
 * Wider than the band the Varta sniffer uses, and deliberately: that one only had to see write
 * gaps, because the reader it listens to repeats one frame and the start gap can be missed
 * without losing the command. Here the start gap is the frame delimiter and must be kept. This
 * firmware sends 18 field clocks for a write gap and 30 for a start gap - 144 and 240 us - and
 * a start gap may be configured longer, so the top of the band is set past any of them rather
 * than at what one reader happened to measure.
 */
#define T5577_SNIFF_BAND_MIN_US 40
#define T5577_SNIFF_BAND_MAX_US 500

/** Two stalls closer than this are one gap counted twice.
 *
 * The carrier does not return cleanly - on the way up it wobbles enough to stall the counter a
 * second time, which showed up as roughly one spurious gap per bit. The shortest real symbol is
 * several times this, so folding them cannot swallow a bit.
 */
#define T5577_SNIFF_DEBOUNCE_US 50

/** A silence this long ends a frame.
 *
 * Bit periods stop below 1 ms - the longest this firmware sends is 56 field clocks plus the gap,
 * 592 us - and the shortest pause between two frames is the 400-clock charge-up before the next
 * start gap, 3.2 ms. This sits in the empty space between them.
 */
#define T5577_SNIFF_PAUSE_US 2000

/** How finely the class histogram bins a symbol period, in microseconds.
 *
 * Far below the 120 us the two classes must be apart to be believed, and far above the jitter
 * within one class, so the bins never merge two classes and never split one into a shape the
 * means would chase. Every period below T5577_SNIFF_PAUSE_US has a bin, which is 62 of them.
 */
#define T5577_SNIFF_BUCKET_US 32
#define T5577_SNIFF_BUCKETS   (T5577_SNIFF_PAUSE_US / T5577_SNIFF_BUCKET_US)
#define T5577_SNIFF_BUCKET_MID(b) \
    ((uint32_t)(b) * T5577_SNIFF_BUCKET_US + T5577_SNIFF_BUCKET_US / 2)

/** A long leading reference symbol, which precedes the frame in one downlink mode.
 *
 * It is a zero symbol stretched by 136 field clocks, so about 1.4 ms against the 592 us a one
 * reaches in this firmware's fixed mode. Nothing inside a frame comes near this.
 */
#define T5577_SNIFF_REFERENCE_US 900

/** Field-on time a write needs after its frame to burn the word into EEPROM.
 *
 * This firmware holds the carrier up for 700 field clocks, 5.6 ms. The threshold sits below that
 * because a pause is measured gap-start to gap-start and so carries a gap width of slack, and
 * because another sender may hold it for less.
 *
 * Used in one direction only: a frame followed by LESS than this cannot have been a write,
 * because the word would never have programmed. The converse does not hold - a reader idling
 * between polls leaves far more than this - so a long pause resolves nothing. This is the one
 * statement of that asymmetry; t5577_sniff_resolve() points here rather than repeating it.
 */
#define T5577_SNIFF_PROGRAM_US 4000

typedef struct {
    uint16_t width_us; // how long the carrier was down
    uint16_t since_us; // from the previous kept gap's start to this one's, so the symbol before it
} T5577SniffEntry;

struct T5577Sniff {
    // Volatile throughout, including the fields only the handler touches. Narrowing it to the
    // fields that cross the boundary is defensible on paper and was tried in the app this
    // detector came from; it stopped behaving and reading the code did not say why.
    volatile uint32_t gap_began_at; // raw DWT count when the stall was declared
    volatile uint32_t last_began_at; // raw DWT count of the previous kept gap
    volatile bool have_previous;
    volatile bool in_gap;
    volatile bool running;

    volatile T5577SniffEntry entry[T5577_SNIFF_ENTRIES];
    // Every gap ever recorded, not the ring's fill: the ring keeps the last
    // T5577_SNIFF_ENTRIES of them and this is what says which those are.
    volatile uint32_t total;

    volatile uint32_t stalls;
    volatile uint32_t gaps;
    // Runs of symbols long enough to be a frame - T5577_SNIFF_FRAME_MIN, which admits the
    // two-symbol commands and therefore the occasional noise pair. Counted here rather than in
    // the decode so a listening screen can tell "a burst has been and gone" from "nothing yet"
    // on a UI tick; what counts as a command is T5577_SNIFF_COMMAND_MIN and the decode's job.
    volatile uint32_t bursts;
    volatile uint32_t run;
    // Every stall's width, in band or not. Evidence only - nothing is gated on it - but it is
    // the only thing that can say a sender's gaps fall outside the band rather than that the
    // coil was quiet.
    volatile uint16_t width[T5577_SNIFF_WIDTH_BUCKETS];

    uint32_t cycles_per_us;
};

// ---------------------------------------------------------------- the measurement

static void t5577_sniff_account(T5577Sniff* sniff, uint32_t began_at, uint32_t ended_at) {
    const uint32_t cycles_per_us = sniff->cycles_per_us;

    // Before the band test, so a reader this cannot measure is still visible as something.
    sniff->stalls++;

    // Plus the threshold: the stall was already that old when the timer reached reload and told
    // us about it, so without it every width would read 20 us short of the band it is tested on.
    const uint32_t width_us = ((ended_at - began_at) / cycles_per_us) + T5577_SNIFF_THRESHOLD_US;

    // Binned before the band test, for the same reason stalls are counted before it.
    const uint32_t bucket = width_us / T5577_SNIFF_WIDTH_BUCKET_US;
    const uint32_t bin = bucket < T5577_SNIFF_WIDTH_BUCKETS ? bucket :
                                                              T5577_SNIFF_WIDTH_BUCKETS - 1;

    if(sniff->width[bin] < UINT16_MAX) sniff->width[bin]++;

    if(width_us < T5577_SNIFF_BAND_MIN_US || width_us > T5577_SNIFF_BAND_MAX_US) return;

    sniff->gaps++;

    // The first gap has no predecessor, so there is no period to measure from it yet. Recorded
    // all the same, with a period that says "longer than any frame": it is a boundary as far as
    // the decode is concerned, which is exactly what the start of a capture is.
    uint32_t since_us = UINT16_MAX;

    if(sniff->have_previous) {
        const uint32_t since = began_at - sniff->last_began_at;

        // A second stall inside the debounce is the same gap: it must not become a symbol and
        // must not move the anchor, since the next real period is still measured from the gap
        // that began. Tested on raw cycles, before the divide: about half of all in-band stalls
        // are these, and `delta / cpu < 50` is exactly `delta < 50 * cpu` for unsigned
        // truncating division.
        if(since < T5577_SNIFF_DEBOUNCE_US * cycles_per_us) return;

        const uint32_t us = since / cycles_per_us;
        since_us = us < UINT16_MAX ? us : UINT16_MAX;
    }

    sniff->last_began_at = began_at;
    sniff->have_previous = true;

    // The run in progress, for the burst count a listening screen watches. A pause closes it;
    // anything shorter is one more symbol.
    if(since_us >= T5577_SNIFF_PAUSE_US) {
        if(sniff->run >= T5577_SNIFF_FRAME_MIN) sniff->bursts++;
        sniff->run = 0;
    } else {
        sniff->run++;
    }

    const uint32_t total = sniff->total;

    sniff->entry[total % T5577_SNIFF_ENTRIES].width_us = (uint16_t)width_us;
    sniff->entry[total % T5577_SNIFF_ENTRIES].since_us = (uint16_t)since_us;
    sniff->total = total + 1;
}

static void t5577_sniff_isr(void* context) {
    T5577Sniff* sniff = context;

    // The carrier stopped: the reload was reached with no edge to reset it.
    if(LL_TIM_IsActiveFlag_UPDATE(TIM2) && LL_TIM_IsEnabledIT_UPDATE(TIM2)) {
        LL_TIM_ClearFlag_UPDATE(TIM2);

        sniff->gap_began_at = DWT->CYCCNT;
        sniff->in_gap = true;

        // Hand over to the trigger, which is the edge that ends this gap. Both armed at once
        // would be an interrupt per carrier edge - 125,000 a second.
        LL_TIM_DisableIT_UPDATE(TIM2);
        LL_TIM_ClearFlag_TRIG(TIM2);
        LL_TIM_EnableIT_TRIG(TIM2);
    }

    // The carrier came back, resetting the timer and raising the trigger flag.
    if(LL_TIM_IsActiveFlag_TRIG(TIM2) && LL_TIM_IsEnabledIT_TRIG(TIM2)) {
        LL_TIM_ClearFlag_TRIG(TIM2);

        if(sniff->in_gap) {
            t5577_sniff_account(sniff, sniff->gap_began_at, DWT->CYCCNT);
            sniff->in_gap = false;
        }

        LL_TIM_DisableIT_TRIG(TIM2);
        // The reset that raised the trigger can raise the update flag with it; starting the next
        // stall from a flag this gap set would report a gap that never happened.
        LL_TIM_ClearFlag_UPDATE(TIM2);
        LL_TIM_EnableIT_UPDATE(TIM2);
    }
}

// Carrier onto TIM2's external trigger, and everything that could drive the antenna held low -
// the pin setup a field detect uses, which is not exported to an app.
static void t5577_sniff_pins(void) {
    furi_hal_ibutton_pin_configure();
    furi_hal_ibutton_pin_write(false);

    furi_hal_gpio_init(&gpio_nfc_irq_rfid_pull, GpioModeOutputPushPull, GpioPullNo, GpioSpeedLow);
    furi_hal_gpio_write(&gpio_nfc_irq_rfid_pull, false);

    furi_hal_gpio_init(&gpio_rfid_carrier_out, GpioModeOutputPushPull, GpioPullNo, GpioSpeedLow);
    furi_hal_gpio_write(&gpio_rfid_carrier_out, false);

    furi_hal_gpio_init_ex(
        &gpio_rfid_carrier, GpioModeAltFunctionPushPull, GpioPullNo, GpioSpeedLow, GpioAltFn2TIM2);
}

// ---------------------------------------------------------------- the decode

// One symbol, against the threshold the capture settled on.
static inline bool t5577_sniff_bit(const uint16_t* symbol, uint16_t split_us, uint8_t at) {
    return symbol[at] > split_us;
}

// MSB first, which is the order a T5577 downlink sends. Long symbol means one.
static uint32_t
    t5577_sniff_word(const uint16_t* symbol, uint16_t split_us, uint8_t at, uint8_t count) {
    uint32_t word = 0;

    for(uint8_t i = 0; i < count; i++) {
        word = (word << 1) | (t5577_sniff_bit(symbol, split_us, at + i) ? 1u : 0u);
    }

    return word;
}

// Every burst length a downlink frame can have.
//
// Two for a reset or a read-mode command, six for a plain read, 38 for a plain write or the
// passworded read it cannot be told from, 40 and 72 for the two commands that only exist with
// leading-zero padding, 70 for a passworded write - each also one longer, for the single symbol
// that a leading zero or a long leading reference puts in front of it.
//
// Shared by the class measurement and the decode so the two cannot disagree about what a frame
// can be, and safe for the former because a burst's length is set by the pause boundaries either
// side of it and owes nothing to the threshold the classes produce.
static bool t5577_sniff_length_known(uint32_t symbols) {
    switch(symbols) {
    case 2:
    case 3:
    case 6:
    case 7:
    case 38:
    case 39:
    case 40:
    case 41:
    case 70:
    case 71:
    case 72:
    case 73:
        return true;
    default:
        return false;
    }
}

// Where the capture's two symbol classes separate, or the measured fallback where it has only
// one.
//
// Measured over the whole capture rather than one burst, because one sender is talking in one
// mode and the capture holds hundreds of symbols where a burst holds tens. That is what makes it
// survive a missed gap: a lost gap merges two symbols into a period longer than either class -
// 336 + 592 = 928 us in this firmware - and an estimate taken from a burst's extremes would put
// the split above every real symbol and decode the whole frame as zeroes.
//
// Only bursts whose LENGTH could be a frame are counted. A run of some other length is not a
// command, and its periods - which run up to the pause threshold - would otherwise outvote the
// sender's: measured on a blue cloner, junk bursts of 26, 16, 13 and 28 symbols pushed the
// estimate to 987 us against real classes of 381 and 762, so every symbol read zero and nine
// clean passworded writes decoded as nothing.
//
// Each burst's first symbol is skipped too: it is measured from the start gap and so reads long
// by a frame-boundary artefact rather than by its class.
static uint16_t t5577_sniff_classes(const T5577Sniff* sniff, uint32_t oldest, uint32_t count) {
    uint16_t bucket[T5577_SNIFF_BUCKETS];
    uint32_t run_start = 0;
    uint32_t run = 0;

    memset(bucket, 0, sizeof(bucket));

    for(uint32_t i = 0; i < count; i++) {
        if(sniff->entry[(oldest + i) % T5577_SNIFF_ENTRIES].since_us < T5577_SNIFF_PAUSE_US) {
            run++;
            continue;
        }

        if(t5577_sniff_length_known(run)) {
            // run_start is the pause that opened the burst, so its first symbol is the
            // entry after it - and that is the one skipped.
            for(uint32_t at = run_start + 2; at < i; at++) {
                const uint16_t us = sniff->entry[(oldest + at) % T5577_SNIFF_ENTRIES].since_us;

                bucket[us / T5577_SNIFF_BUCKET_US]++;
            }
        }

        run_start = i;
        run = 0;
    }

    // The two most populated bins. Two symbol classes make two peaks and nothing else does:
    // a merged symbol, a split one, a stray period out of a burst that happened to be a frame
    // length - each is a few samples spread thin, and none can outweigh a class.
    //
    // Two means seeded at the occupied extremes was tried first and is the wrong tool. Two
    // clusters over this data separate the data from its OUTLIERS rather than one class from the
    // other, so a single period near the pause threshold puts the boundary above both. Measured
    // on a blue cloner over four runs of the same signal: classes at 379/733, 379/750, 380/720
    // and 381/717, and splits of 1176, 510, 1145 and 985. Only one of the four decoded.
    uint8_t first = 0;

    for(uint8_t b = 1; b < T5577_SNIFF_BUCKETS; b++) {
        if(bucket[b] > bucket[first]) first = b;
    }

    if(bucket[first] == 0) return T5577_SNIFF_SPLIT_US;

    uint8_t second = 0;
    bool found = false;

    for(uint8_t b = 0; b < T5577_SNIFF_BUCKETS; b++) {
        if(bucket[b] == 0) continue;

        const uint32_t apart =
            (uint32_t)(b > first ? b - first : first - b) * T5577_SNIFF_BUCKET_US;

        // Near neighbours are one class's own spread, not the other class.
        if(apart < T5577_SNIFF_CLASSES_MIN_US) continue;
        if(found && bucket[b] <= bucket[second]) continue;

        second = b;
        found = true;
    }

    // One class only - every symbol the same - so there is no boundary to find and the fallback
    // is the honest answer.
    if(!found) return T5577_SNIFF_SPLIT_US;

    const uint32_t low = T5577_SNIFF_BUCKET_MID(first < second ? first : second);
    const uint32_t high = T5577_SNIFF_BUCKET_MID(first < second ? second : first);

    return (uint16_t)((low + high) / 2);
}

// Everything the layout of a frame depends on, once the mode's preamble has been accounted for:
// where the fields start and how many there are.
//
// The block address is the low three bits of every frame that carries one, whichever reading is
// taken, so it is read from the end - but only inside the cases that have one. A two-symbol
// command has no address, and reading one from the end of it walks off the front of the array.
static void
    t5577_sniff_fields(T5577SniffFrame* frame, const uint16_t* symbol, uint16_t split, uint8_t at) {
    const uint8_t bits = frame->symbols - at;
    const uint8_t last = frame->symbols;
    // Two bits read once: the opcode's first says "not test mode" and its second selects the
    // page, and the reset is the one command where both are clear.
    const uint32_t opcode = t5577_sniff_word(symbol, split, at, 2);

    frame->page = (opcode & 0x1) != 0;

    switch(bits) {
    case 2:
        frame->kind = opcode == 0 ? T5577SniffFrameReset : T5577SniffFrameRegularRead;
        break;

    case 6:
        frame->kind = T5577SniffFramePlainRead;
        frame->lock = t5577_sniff_bit(symbol, split, at + 2);
        frame->block = (uint8_t)t5577_sniff_word(symbol, split, last - 3, 3);
        break;

    case 38:
        // Both readings filled, because this length admits both and nothing in the frame itself
        // chooses - unless it arrived in leading-zero mode, where the two padding bits before a
        // password make a passworded read 40 and leave this length to a write alone.
        //
        // Plain write: opcode, lock, data, block.
        frame->lock = t5577_sniff_bit(symbol, split, at + 2);
        frame->data = t5577_sniff_word(symbol, split, at + 3, 32);
        frame->block = (uint8_t)t5577_sniff_word(symbol, split, last - 3, 3);
        // And where the passworded read puts its lock bit, kept so a resolution that lands on
        // that reading does not report the password's top bit as a lock bit.
        frame->lock_alt = t5577_sniff_bit(symbol, split, last - 4);

        if(frame->leading_zero) {
            frame->kind = T5577SniffFramePlainWrite;
            frame->why = T5577SniffWhyLength;
            break;
        }

        frame->kind = T5577SniffFrameAmbiguous;
        // Passworded read: opcode, password, lock, block.
        frame->password = t5577_sniff_word(symbol, split, at + 2, 32);
        break;

    case 40:
        // Leading-zero mode only, where the two padding bits before a password push a passworded
        // read past a plain write. Without that check this length is also reachable from a
        // FIXED-mode frame that picked up two spurious gaps - the carrier produces about one per
        // bit, which is what the debounce is for - and then the password is read two bits off
        // and reported as the one length that admits no other reading.
        if(!frame->leading_zero) {
            frame->kind = T5577SniffFrameUnknown;
            break;
        }

        frame->kind = T5577SniffFramePasswordedRead;
        frame->why = T5577SniffWhyLength;
        frame->password = t5577_sniff_word(symbol, split, at + 4, 32);
        frame->lock = t5577_sniff_bit(symbol, split, last - 4);
        frame->block = (uint8_t)t5577_sniff_word(symbol, split, last - 3, 3);
        break;

    case 70:
    case 72: {
        // Opcode, [two padding bits in leading-zero mode], password, lock, data, block. 72 is
        // the same command in that mode: the padding sits between the opcode and the password,
        // and everything after the password is read from the end and so is unmoved.
        //
        // The mode has to agree with the length for the same reason case 40 checks it - measured
        // before that guard, a passworded write of 50415353 came back as 40415353, with its
        // data, block and lock all still correct and self-consistent.
        const bool padded = (bits == 72);

        if(frame->leading_zero != padded) {
            frame->kind = T5577SniffFrameUnknown;
            break;
        }

        frame->kind = T5577SniffFramePasswordedWrite;
        frame->why = T5577SniffWhyLength;
        frame->password = t5577_sniff_word(symbol, split, at + (padded ? 4 : 2), 32);
        frame->lock = t5577_sniff_bit(symbol, split, last - 36);
        frame->data = t5577_sniff_word(symbol, split, last - 35, 32);
        frame->block = (uint8_t)t5577_sniff_word(symbol, split, last - 3, 3);
        break;
    }

    default:
        frame->kind = T5577SniffFrameUnknown;
        break;
    }
}

// A whole burst, from its start gap to the pause that ended it. The caller has already
// established `symbols >= T5577_SNIFF_FRAME_MIN`, which is this function's precondition: the
// fields below read a two-symbol opcode before anything else.
static void t5577_sniff_frame(
    T5577SniffFrame* frame,
    const uint16_t* symbol,
    uint16_t split,
    uint8_t symbols) {
    memset(frame, 0, sizeof(*frame));
    frame->symbols = symbols;

    // What precedes the opcode is what tells the downlink modes apart. A long leading reference
    // is a stretched zero symbol an order of magnitude past anything inside a frame; a leading
    // zero is an ordinary short symbol, and is only read as one where the length that follows it
    // is a length a frame can be.
    const bool reference = symbol[0] >= T5577_SNIFF_REFERENCE_US && symbols >= 3;
    uint8_t at = reference ? 1 : 0;

    if(!reference && symbol[0] <= split) {
        const uint8_t rest = symbols - 1;

        // Deliberately not the same set as the switch in t5577_sniff_fields(): 70 is absent,
        // because a passworded write in leading-zero mode is 73 symbols and not 71.
        if(rest == 2 || rest == 6 || rest == 38 || rest == 40 || rest == 72) {
            frame->leading_zero = true;
            at = 1;
        }
    }

    // A fixed-length frame starts at its opcode, so the opcode's leading one is the check that
    // the alignment is right. Only a reset has it clear, and a reset is two symbols.
    if(!t5577_sniff_bit(symbol, split, at) && (symbols - at) != 2) {
        frame->kind = T5577SniffFrameUnknown;
        return;
    }

    t5577_sniff_fields(frame, symbol, split, at);
}

// One burst, closed. `pause_us` is the carrier-on time that ended it, or zero where the capture
// stopped first and the question of what followed has no answer.
static void t5577_sniff_close(
    T5577SniffResult* result,
    const uint16_t* symbol,
    uint8_t symbols,
    uint32_t pause_us) {
    // A lone gap with quiet either side is not a command, and open air produces those by the
    // hundred: 826 in-band gaps in thirty seconds on a desk, almost every one of them alone.
    // Reporting each as a frame filled the list with nothing and drowned the one burst that
    // was real.
    if(symbols < T5577_SNIFF_FRAME_MIN) return;

    if(result->frames >= T5577_SNIFF_FRAMES_MAX) {
        // Saturating, as the quiet counter one file over does: a noisy capture can close more
        // bursts than a byte holds, and a wrapped count reads as a tidier capture than it was.
        if(result->dropped < UINT8_MAX) result->dropped++;
        return;
    }

    T5577SniffFrame* frame = &result->frame[result->frames++];

    t5577_sniff_frame(frame, symbol, result->split_us, symbols);
    frame->pause_us = pause_us;
    // A gap period is held in a uint16_t, so anything past its ceiling arrived as the ceiling.
    frame->pause_capped = pause_us >= UINT16_MAX;
}

// Which of an ambiguous frame's two readings the frames around it rule out. Done in a second
// pass because the deciding evidence is what FOLLOWS the frame, which is not known while it is
// being decoded.
static void t5577_sniff_resolve(T5577SniffResult* result) {
    for(uint8_t i = 0; i < result->frames; i++) {
        T5577SniffFrame* frame = &result->frame[i];

        if(frame->kind != T5577SniffFrameAmbiguous) continue;

        // Not a write, whatever followed it - see T5577_SNIFF_PROGRAM_US for why this holds in
        // one direction only. Physics first and convention second: this test used to run after
        // the marker below, and when the two disagreed the marker won in silence.
        if(frame->pause_us > 0 && frame->pause_us < T5577_SNIFF_PROGRAM_US) {
            frame->kind = T5577SniffFramePasswordedRead;
            frame->why = T5577SniffWhyPause;
            // The two readings put their lock bits in different places.
            frame->lock = frame->lock_alt;
            continue;
        }

        // A read-mode command follows every write, to stop the tag repeating the block that was
        // addressed - but it follows every READ in this firmware too (t5577_block_read_bits), so
        // on its own it proves nothing and once reported a captured password as a write of that
        // password into a block. What separates them is the wait before it: 8.8 ms for a write,
        // hundreds of milliseconds for a read waiting on a tag to answer. The upper bound covers
        // the saturating case too, since a pause at the uint16 ceiling is far past it.
        const bool followed_by_read = i + 1 < result->frames &&
                                      result->frame[i + 1].kind == T5577SniffFrameRegularRead;
        const bool programming_wait = frame->pause_us >= T5577_SNIFF_PROGRAM_US &&
                                      frame->pause_us < T5577_SNIFF_MARKER_MAX_US;

        if(followed_by_read && programming_wait) {
            frame->kind = T5577SniffFramePlainWrite;
            frame->why = T5577SniffWhyMarker;
        }
    }
}

// ---------------------------------------------------------------- the handle

T5577Sniff* t5577_sniff_alloc(void) {
    T5577Sniff* sniff = malloc(sizeof(T5577Sniff));

    // Zeroed rather than left to the allocator: the heap only zeroes because this target happens
    // to configure it to, and a decode before the first start() would otherwise frame whatever
    // the last owner of this memory left behind.
    memset(sniff, 0, sizeof(*sniff));
    sniff->cycles_per_us = furi_hal_cortex_instructions_per_microsecond();

    return sniff;
}

void t5577_sniff_free(T5577Sniff* sniff) {
    furi_check(sniff);

    t5577_sniff_stop(sniff);
    free(sniff);
}

void t5577_sniff_start(T5577Sniff* sniff) {
    furi_check(sniff);
    furi_check(!sniff->running);

    t5577_sniff_pins();
    furi_hal_bus_enable(FuriHalBusTIM2);

    LL_TIM_SetPrescaler(TIM2, sniff->cycles_per_us - 1);
    LL_TIM_SetCounterMode(TIM2, LL_TIM_COUNTERMODE_UP);
    LL_TIM_SetAutoReload(TIM2, T5577_SNIFF_THRESHOLD_US - 1);
    LL_TIM_SetClockSource(TIM2, LL_TIM_CLOCKSOURCE_INTERNAL);

    // Only an overflow counts as an update. The reset default also raises UPDATE whenever the
    // slave mode controller reloads the counter - which here is every carrier edge, so the
    // detector read a 125 kHz carrier as 113k stalls a second. The flag that must mean "the
    // carrier stopped" cannot also mean "the carrier arrived".
    LL_TIM_SetUpdateSource(TIM2, LL_TIM_UPDATESOURCE_COUNTER);

    // The carrier resets the count. Inverted polarity and no filter, exactly as the field detect
    // configures the same pin - a filter would swallow the edges this depends on.
    LL_TIM_ConfigETR(
        TIM2, LL_TIM_ETR_POLARITY_INVERTED, LL_TIM_ETR_PRESCALER_DIV1, LL_TIM_ETR_FILTER_FDIV1);
    LL_TIM_SetTriggerInput(TIM2, LL_TIM_TS_ETRF);
    LL_TIM_SetSlaveMode(TIM2, LL_TIM_SLAVEMODE_RESET);

    FURI_CRITICAL_ENTER();
    sniff->in_gap = false;
    sniff->have_previous = false;
    sniff->total = 0;
    sniff->stalls = 0;
    sniff->gaps = 0;
    sniff->bursts = 0;
    sniff->run = 0;
    for(uint8_t i = 0; i < T5577_SNIFF_WIDTH_BUCKETS; i++) {
        sniff->width[i] = 0;
    }
    FURI_CRITICAL_EXIT();

    furi_hal_interrupt_set_isr(FuriHalInterruptIdTIM2, t5577_sniff_isr, sniff);

    LL_TIM_ClearFlag_UPDATE(TIM2);
    LL_TIM_ClearFlag_TRIG(TIM2);
    LL_TIM_EnableIT_UPDATE(TIM2);
    LL_TIM_EnableCounter(TIM2);

    sniff->running = true;
}

void t5577_sniff_stop(T5577Sniff* sniff) {
    furi_check(sniff);
    if(!sniff->running) return;

    LL_TIM_DisableIT_UPDATE(TIM2);
    LL_TIM_DisableIT_TRIG(TIM2);
    LL_TIM_DisableCounter(TIM2);

    furi_hal_interrupt_set_isr(FuriHalInterruptIdTIM2, NULL, NULL);
    furi_hal_bus_disable(FuriHalBusTIM2);

    // All four pins, not just the carrier input: start() drives the iButton pad, the NFC pull and
    // the carrier output low, and leaving the scene without this leaves them that way - a 1-Wire
    // pad shorted to ground for as long as the app stays open.
    furi_hal_rfid_pins_reset();

    sniff->running = false;
}

void t5577_sniff_poll(const T5577Sniff* sniff, T5577SniffProgress* progress) {
    furi_check(sniff);
    furi_check(progress);

    // One copy under a critical section: counters read either side of an interrupt would
    // describe two different moments, and the screen decides when to stop from them.
    FURI_CRITICAL_ENTER();
    progress->stalls = sniff->stalls;
    progress->gaps = sniff->gaps;
    progress->bursts = sniff->bursts;
    progress->overflowed = sniff->total > T5577_SNIFF_ENTRIES;
    FURI_CRITICAL_EXIT();
}

void t5577_sniff_decode(const T5577Sniff* sniff, T5577SniffResult* result) {
    furi_check(sniff);
    furi_check(result);
    // Not a style check: the ring is read in place rather than copied - 4 KiB does not go on a
    // 4 KiB stack - so a gap arriving mid-walk would be framed into a sequence already half read.
    furi_check(!sniff->running);

    memset(result, 0, sizeof(*result));
    t5577_sniff_poll(sniff, &result->progress);

    const uint32_t total = sniff->total;
    const uint32_t count = total < T5577_SNIFF_ENTRIES ? total : T5577_SNIFF_ENTRIES;
    // Where the oldest surviving entry sits. Once the ring has wrapped the oldest is the one
    // about to be overwritten, which is the slot the next write would take.
    const uint32_t oldest = total > T5577_SNIFF_ENTRIES ? total % T5577_SNIFF_ENTRIES : 0;

    for(uint8_t i = 0; i < T5577_SNIFF_WIDTH_BUCKETS; i++) {
        result->widths.bucket[i] = sniff->width[i];
    }

    result->split_us = t5577_sniff_classes(sniff, oldest, count);

    uint16_t symbol[T5577_SNIFF_SYMBOLS_MAX];
    uint8_t symbols = 0;
    uint32_t short_count = 0;
    uint32_t short_sum = 0;
    uint32_t long_count = 0;
    uint32_t long_sum = 0;
    // Everything before the first pause is the tail of a command already in flight when
    // listening began, and a partial frame decoded as whole is how a wrong model gets confirmed.
    bool aligned = false;
    uint16_t start_gap_us = 0;

    for(uint32_t i = 0; i < count; i++) {
        // Field by field, not a struct copy: a struct assignment out of a volatile array is an
        // atomic load to the compiler, which on a 32-bit load of two 16-bit fields means a call
        // into a runtime that is not linked.
        const uint32_t at = (oldest + i) % T5577_SNIFF_ENTRIES;
        const uint16_t width_us = sniff->entry[at].width_us;
        uint16_t since_us = sniff->entry[at].since_us;

        if(since_us < T5577_SNIFF_PAUSE_US) {
            if(!aligned) continue;

            // Symbol one is measured from the START gap, which is wider than a write gap, so it
            // reads that much long. Corrected here, where both widths are in hand - this entry's
            // own gap is the one that ENDED the symbol, so on a burst's first symbol it is the
            // first write gap. From the two measured widths rather than a constant, because the
            // amount varies by sender; uncorrected it pushes symbol one UP, so a zero crosses
            // the split and reads as a one, which turns a reset into a regular read and costs a
            // leading-zero frame the short first symbol its mode is recognised by.
            if(symbols == 0 && start_gap_us > width_us && since_us > (start_gap_us - width_us)) {
                since_us = since_us - start_gap_us + width_us;
            }

            // Against the fixed split, not a burst's own: these are a whole-capture sanity
            // check on the fallback, and are the figures that reveal a sender whose two classes
            // straddle it.
            if(since_us > T5577_SNIFF_SPLIT_US) {
                long_count++;
                long_sum += since_us;
            } else {
                short_count++;
                short_sum += since_us;
            }

            // Past the longest frame any mode sends means the boundary is not where we think it
            // is, so there is nothing to gain by storing more of it. The burst is still closed
            // and still reported, at a length no layout matches - dropping it would hide
            // exactly the capture that needs looking at.
            if(symbols < T5577_SNIFF_SYMBOLS_MAX) symbol[symbols++] = since_us;
            continue;
        }

        // A pause. It closes the burst before it, and this entry's own gap is the start gap of
        // the burst after it.
        if(aligned) t5577_sniff_close(result, symbol, symbols, since_us);

        aligned = true;
        symbols = 0;
        start_gap_us = width_us;
    }

    // The burst still open when the capture stopped, reported with no pause: a cloner need not
    // follow its last command with anything, and dropping the frame would lose the last write of
    // every sequence that does not. Zero reads as "nothing followed" to the resolution below,
    // which is the truth about it.
    if(aligned) t5577_sniff_close(result, symbol, symbols, 0);

    result->short_us = short_count ? short_sum / short_count : 0;
    result->long_us = long_count ? long_sum / long_count : 0;

    t5577_sniff_resolve(result);
}
