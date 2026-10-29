// Host test for the T5577 downlink sniffer's framing and decode. Builds synthetic rings with
// this firmware's own write timings, so the layout arithmetic is checked against a sequence
// whose answer is known before any of it meets a cloner.
#include "t5577_sniff.c"

// This firmware's fixed-bit-length timings, in microseconds: 24, 56, 18 and 30 field clocks.
#define ON0       192
#define ON1       448
#define WGAP      144
#define SGAP      240
// And its leading-zero row: 18 and 40 field clocks. Both periods - 288 and 464 us - fall BELOW
// the 480 us fixed split, which is why a burst has to be allowed to split itself.
#define LZ_ON0    144
#define LZ_ON1    320
// 700 clocks of programming plus the 400-clock charge-up before the next frame, and the
// charge-up alone after a read.
#define PAUSE_PGM 8800
#define PAUSE_RD  3200

static T5577Sniff* sniff;
static int failures;

// What emit() sends for a zero and a one. Switched per case, because the modes differ and a test
// that sends fixed-mode symbols while claiming leading-zero proves nothing about leading-zero.
static uint16_t on_0 = ON0;
static uint16_t on_1 = ON1;

static void mode_fixed(void) {
    on_0 = ON0;
    on_1 = ON1;
}

static void mode_leading_zero(void) {
    on_0 = LZ_ON0;
    on_1 = LZ_ON1;
}

static void push(uint16_t width_us, uint32_t since_us) {
    const uint32_t total = sniff->total;
    const uint32_t since = since_us < UINT16_MAX ? since_us : UINT16_MAX;

    sniff->entry[total % T5577_SNIFF_ENTRIES].width_us = width_us;
    sniff->entry[total % T5577_SNIFF_ENTRIES].since_us = (uint16_t)since;
    sniff->total = total + 1;
}

// One frame on the air: a start gap, then a gap per bit. `pause_us` is the carrier-on time since
// whatever came before, which is what makes the start gap a frame boundary.
static void emit(const char* bits, uint32_t pause_us, uint16_t start_gap) {
    push(start_gap, pause_us);

    uint16_t previous_gap = start_gap;

    for(const char* at = bits; *at; at++) {
        const uint16_t on = (*at == '1') ? on_1 : on_0;
        push(WGAP, previous_gap + on);
        previous_gap = WGAP;
    }
}

static void begin(void) {
    memset((void*)sniff->entry, 0, sizeof(sniff->entry));
    sniff->total = 0;
    sniff->running = false;
    sniff->stalls = 0;
    sniff->gaps = 0;
    sniff->bursts = 0;
    sniff->run = 0;
    sniff->have_previous = false;
    mode_fixed();
}

// A physical gap, fed through the real entry point instead of straight into the ring, so the
// band filter, the debounce and the counters are exercised. The handler learns of a stall
// THRESHOLD_US after it began, which is the correction account() applies - so starting the call
// that late makes the width it records equal the width asked for here.
static uint32_t now_us;

static void gap(uint16_t width_us, uint32_t quiet_us) {
    const uint32_t cpu = sniff->cycles_per_us;

    now_us += quiet_us;
    t5577_sniff_account(
        sniff, (now_us + T5577_SNIFF_THRESHOLD_US) * cpu, (now_us + width_us) * cpu);
    now_us += width_us;
}

static void bits_of(char* out, uint32_t value, uint8_t count) {
    for(uint8_t i = 0; i < count; i++) {
        out[i] = ((value >> (count - 1 - i)) & 1) ? '1' : '0';
    }
    out[count] = 0;
}

// opcode(2) [password(32)] lock(1) [data(32)] block(3), as t5577_build_frame() assembles it.
static void frame_bits_locked(
    char* out,
    bool page,
    bool with_pass,
    uint32_t password,
    bool write,
    uint32_t data,
    uint8_t block,
    bool leading_zero,
    bool lock) {
    char* at = out;

    if(leading_zero) *at++ = '0';

    *at++ = '1';
    *at++ = page ? '1' : '0';

    if(with_pass) {
        // Those modes pad the opcode out before a password follows it.
        if(leading_zero) {
            *at++ = '0';
            *at++ = '0';
        }
        bits_of(at, password, 32);
        at += 32;
    }

    *at++ = lock ? '1' : '0';

    if(write) {
        bits_of(at, data, 32);
        at += 32;
    }

    bits_of(at, block, 3);
}

static void frame_bits(
    char* out,
    bool page,
    bool with_pass,
    uint32_t password,
    bool write,
    uint32_t data,
    uint8_t block,
    bool leading_zero) {
    frame_bits_locked(out, page, with_pass, password, write, data, block, leading_zero, false);
}

#define CHECK(what, got, want)                                                              \
    do {                                                                                    \
        if((unsigned long)(got) != (unsigned long)(want)) {                                 \
            printf("  FAIL %-22s got %lu (0x%lX) want %lu (0x%lX)\n", what,                 \
                   (unsigned long)(got), (unsigned long)(got), (unsigned long)(want),       \
                   (unsigned long)(want));                                                  \
            failures++;                                                                     \
        }                                                                                   \
    } while(0)

static T5577SniffResult result;

static void decode(const char* name) {
    t5577_sniff_decode(sniff, &result);
    printf("%s: %u frames, %u dropped, symbols %lu/%lu us\n", name, result.frames,
           result.dropped, (unsigned long)result.short_us, (unsigned long)result.long_us);

    for(uint8_t i = 0; i < result.frames; i++) {
        const T5577SniffFrame* f = &result.frame[i];
        printf("  [%u] kind %u why %u sym %u lz %u p%u b%u lock %u", i, f->kind, f->why,
               f->symbols, f->leading_zero, f->page, f->block, f->lock);
        if(t5577_sniff_frame_has_data(f)) printf(" data %08lX", (unsigned long)f->data);
        if(t5577_sniff_frame_tells_password(f)) printf(" pass %08lX", (unsigned long)f->password);
        printf(" pause %lu\n", (unsigned long)f->pause_us);
    }
}

// ------------------------------------------------------------------ the cases

// What this firmware does when it writes one block: the write, then a regular read to stop the
// tag repeating the block that was addressed.
static void test_plain_write(void) {
    char bits[128];

    begin();
    frame_bits(bits, false, false, 0, true, 0xDEADBEEF, 7, false);
    CHECK("plain write length", strlen(bits), 38);
    emit(bits, 20000, SGAP);
    emit("10", PAUSE_PGM, SGAP); // back to read mode
    push(SGAP, PAUSE_RD); // and the next command's boundary, so the pair is closed

    decode("plain write to block 7");
    CHECK("frames", result.frames, 2);
    CHECK("kind", result.frame[0].kind, T5577SniffFramePlainWrite);
    CHECK("why", result.frame[0].why, T5577SniffWhyMarker);
    CHECK("data", result.frame[0].data, 0xDEADBEEF);
    CHECK("block", result.frame[0].block, 7);
    CHECK("no password claimed", t5577_sniff_frame_tells_password(&result.frame[0]), false);
    CHECK("marker kind", result.frame[1].kind, T5577SniffFrameRegularRead);
}

// A reader polling one block with a password. Nothing rules out the write reading, so the frame
// stays ambiguous and both words are reported - which is the honest answer, not a failure.
static void test_passworded_read(void) {
    char bits[128];

    begin();
    frame_bits(bits, false, true, 0xFFA94A50, false, 0, 6, false);
    CHECK("passworded read length", strlen(bits), 38);
    emit(bits, 20000, SGAP);
    emit(bits, 13760, SGAP);
    push(SGAP, 13760);

    decode("passworded read of block 6");
    CHECK("frames", result.frames, 2);
    CHECK("kind", result.frame[0].kind, T5577SniffFrameAmbiguous);
    CHECK("password reading", result.frame[0].password, 0xFFA94A50);
    CHECK("block", result.frame[0].block, 6);
    CHECK("data reading offered", t5577_sniff_frame_has_data(&result.frame[0]), true);
}

// The same read, but with a pause too short for anything to have programmed. That rules out the
// write reading on its own.
static void test_read_short_pause(void) {
    char bits[128];

    begin();
    frame_bits(bits, false, true, 0x11223344, false, 0, 1, false);
    emit(bits, 20000, SGAP);
    emit(bits, PAUSE_RD, SGAP);
    push(SGAP, PAUSE_RD);

    decode("passworded read, 3.2 ms pause");
    CHECK("kind", result.frame[0].kind, T5577SniffFramePasswordedRead);
    CHECK("why", result.frame[0].why, T5577SniffWhyPause);
    CHECK("password", result.frame[0].password, 0x11223344);
    CHECK("no data claimed", t5577_sniff_frame_has_data(&result.frame[0]), false);
}

// 70 bits is a passworded write and nothing else, so both words are known: the password the
// sender already holds, and what it put in the block.
static void test_passworded_write(void) {
    char bits[128];

    begin();
    frame_bits(bits, false, true, 0x50415353, true, 0x00148040, 0, false);
    CHECK("passworded write length", strlen(bits), 70);
    emit(bits, 20000, SGAP);
    push(SGAP, PAUSE_PGM);

    decode("passworded write of block 0");
    CHECK("kind", result.frame[0].kind, T5577SniffFramePasswordedWrite);
    CHECK("why", result.frame[0].why, T5577SniffWhyLength);
    CHECK("password", result.frame[0].password, 0x50415353);
    CHECK("data", result.frame[0].data, 0x00148040);
    CHECK("block", result.frame[0].block, 0);
}

// In leading-zero mode the two padding bits before a password make a passworded read 41 symbols
// and leave 39 to a plain write, so the ambiguity this app is otherwise careful about disappears.
static void test_leading_zero(void) {
    char bits[128];

    begin();
    mode_leading_zero();
    frame_bits(bits, false, false, 0, true, 0xCAFEF00D, 7, true);
    CHECK("lz plain write length", strlen(bits), 39);
    emit(bits, 20000, SGAP);
    push(SGAP, PAUSE_PGM);

    decode("leading-zero plain write");
    CHECK("kind", result.frame[0].kind, T5577SniffFramePlainWrite);
    CHECK("why", result.frame[0].why, T5577SniffWhyLength);
    CHECK("leading zero", result.frame[0].leading_zero, true);
    CHECK("data", result.frame[0].data, 0xCAFEF00D);
    CHECK("block", result.frame[0].block, 7);

    begin();
    mode_leading_zero();
    frame_bits(bits, true, true, 0x87654321, false, 0, 3, true);
    CHECK("lz passworded read length", strlen(bits), 41);
    emit(bits, 20000, SGAP);
    push(SGAP, 30000);

    decode("leading-zero passworded read");
    CHECK("kind", result.frame[0].kind, T5577SniffFramePasswordedRead);
    CHECK("why", result.frame[0].why, T5577SniffWhyLength);
    CHECK("password", result.frame[0].password, 0x87654321);
    CHECK("page", result.frame[0].page, true);
    CHECK("block", result.frame[0].block, 3);

    begin();
    mode_leading_zero();
    frame_bits(bits, false, true, 0x0BADC0DE, true, 0x12345678, 5, true);
    CHECK("lz passworded write length", strlen(bits), 73);
    emit(bits, 20000, SGAP);
    push(SGAP, PAUSE_PGM);

    decode("leading-zero passworded write");
    // Both leading-zero classes sit below the fixed fallback, so this case only decodes at all
    // because the burst was allowed to split itself.
    CHECK("split is the capture's own", result.split_us < T5577_SNIFF_SPLIT_US, true);
    CHECK("kind", result.frame[0].kind, T5577SniffFramePasswordedWrite);
    CHECK("password", result.frame[0].password, 0x0BADC0DE);
    CHECK("data", result.frame[0].data, 0x12345678);
    CHECK("block", result.frame[0].block, 5);
}

static void test_short_commands(void) {
    begin();
    emit("00", 20000, SGAP); // reset
    emit("10", 5000, SGAP); // regular read, page 0
    emit("000", 5000, SGAP); // leading-zero reset
    emit("011", 5000, SGAP); // leading-zero regular read, page 1
    emit("100", 5000, SGAP); // plain read is six bits, so three is nothing
    push(SGAP, 5000);

    decode("short commands");
    // And the other side of the boundary: three symbols is below it.
    CHECK("not commanded", t5577_sniff_commanded(&result), false);
    CHECK("frames", result.frames, 5);
    CHECK("reset", result.frame[0].kind, T5577SniffFrameReset);
    CHECK("regular read", result.frame[1].kind, T5577SniffFrameRegularRead);
    CHECK("lz reset", result.frame[2].kind, T5577SniffFrameReset);
    CHECK("lz regular read", result.frame[3].kind, T5577SniffFrameRegularRead);
    CHECK("lz regular read page", result.frame[3].page, true);
    CHECK("three bits is nothing", result.frame[4].kind, T5577SniffFrameUnknown);
}

static void test_plain_read(void) {
    char bits[128];

    begin();
    frame_bits(bits, true, false, 0, false, 0, 2, false);
    CHECK("plain read length", strlen(bits), 6);
    emit(bits, 20000, SGAP);
    push(SGAP, PAUSE_RD);

    decode("plain read of page 1 block 2");
    // Six symbols is exactly T5577_SNIFF_COMMAND_MIN. Without this the suite only ever asserts
    // commanded() false, and a version returning false unconditionally passes all of it.
    CHECK("commanded", t5577_sniff_commanded(&result), true);
    CHECK("kind", result.frame[0].kind, T5577SniffFramePlainRead);
    CHECK("page", result.frame[0].page, true);
    CHECK("block", result.frame[0].block, 2);
    CHECK("nothing claimed", t5577_sniff_frame_tells_password(&result.frame[0]), false);
}

// A wide start gap inflates symbol one past the split, so without the correction the opcode's
// leading one reads as a zero and the whole frame is thrown away as a reset.
static void test_wide_start_gap(void) {
    char bits[128];

    begin();
    frame_bits(bits, false, false, 0, true, 0x0F0F0F0F, 4, false);
    emit(bits, 20000, 560); // 70 field clocks, well past this firmware's 30
    push(560, PAUSE_PGM);

    decode("wide start gap");
    CHECK("kind", result.frame[0].kind, T5577SniffFrameAmbiguous);
    CHECK("data", result.frame[0].data, 0x0F0F0F0F);
    CHECK("block", result.frame[0].block, 4);
}

// A whole restore, as t5577_write_blocks() sends it: block 0 first, then ascending, each write
// followed by its regular read. The last block written is 7, which is where a password goes.
static void test_restore_sequence(void) {
    char bits[128];
    const uint32_t word[8] = {
        0x00148040, 0x11111111, 0x22222222, 0x33333333, 0x44444444, 0x55555555, 0x66666666,
        0xA1B2C3D4};

    begin();

    for(uint8_t block = 0; block < 8; block++) {
        frame_bits(bits, false, false, 0, true, word[block], block, false);
        emit(bits, block == 0 ? 20000 : PAUSE_RD, SGAP);
        emit("10", PAUSE_PGM, SGAP);
    }

    push(SGAP, PAUSE_RD);

    decode("eight-block restore");
    CHECK("frames", result.frames, 16);

    for(uint8_t block = 0; block < 8; block++) {
        const T5577SniffFrame* f = &result.frame[block * 2];
        CHECK("write kind", f->kind, T5577SniffFramePlainWrite);
        CHECK("write block", f->block, block);
        CHECK("write data", f->data, word[block]);
        CHECK("marker", result.frame[block * 2 + 1].kind, T5577SniffFrameRegularRead);
    }

    CHECK("password word", result.frame[14].data, 0xA1B2C3D4);
}

// The ring keeps the last T5577_SNIFF_ENTRIES gaps, so a burst that arrives after a long spell
// of noise must survive and the noise must not.
static void test_ring_wrap(void) {
    char bits[128];

    begin();

    // Well over the ring: in-band stalls at a width and spacing no frame has.
    for(uint32_t i = 0; i < T5577_SNIFF_ENTRIES + 500; i++) push(60, 900);

    frame_bits(bits, false, true, 0xFEEDFACE, true, 0x76543210, 2, false);
    emit(bits, 20000, SGAP);
    push(SGAP, PAUSE_PGM);

    decode("burst after a full ring of noise");
    CHECK("overflowed", result.progress.overflowed, true);
    CHECK("frames", result.frames, 1);
    CHECK("kind", result.frame[0].kind, T5577SniffFramePasswordedWrite);
    CHECK("password", result.frame[0].password, 0xFEEDFACE);
    CHECK("data", result.frame[0].data, 0x76543210);
}

// More frames than the result holds. What is kept has to be the frames themselves plus an
// honest count of what was not.
static void test_overflow_frames(void) {
    begin();

    for(uint32_t i = 0; i < T5577_SNIFF_FRAMES_MAX + 6; i++) emit("10", 5000, SGAP);

    push(SGAP, 5000);

    decode("more frames than fit");
    CHECK("frames", result.frames, T5577_SNIFF_FRAMES_MAX);
    CHECK("dropped", result.dropped, 6);
}

// What a Flipper on a desk actually records: isolated in-band gaps with long quiet either side.
// Measured over thirty seconds - 11,949 stalls, 826 in-band gaps, one burst of two symbols - and
// every one of those lone gaps was reported as a frame until close() grew a floor.
static void test_open_air(void) {
    begin();

    for(uint32_t i = 0; i < 400; i++) push(60, 5000 + (i % 7) * 3000);

    decode("open air");
    CHECK("no frames", result.frames, 0);
    CHECK("nothing commanded", t5577_sniff_commanded(&result), false);

    // The one two-symbol burst that thirty seconds of it produced. Reported, because it is the
    // only evidence there is, but not enough to claim a reader.
    begin();
    for(uint32_t i = 0; i < 20; i++) push(60, 9000);
    push(60, 1408); // two gaps closer together than a pause, so two symbols between three gaps
    push(60, 1408);
    for(uint32_t i = 0; i < 20; i++) push(60, 9000);

    decode("open air with one pair");
    CHECK("the pair is kept", result.frames, 1);
    CHECK("still nothing commanded", t5577_sniff_commanded(&result), false);
}

// Two spurious gaps inside a fixed-mode frame push it onto a length that exists only in
// leading-zero mode. Before cases 40 and 72 required that mode, the password came back shifted
// two bits and labelled "only this length fits" - 50415353 read as 40415353, with the data,
// block and lock all still correct and self-consistent.
static void test_spurious_gaps(void) {
    char bits[128];

    begin();
    frame_bits(bits, false, true, 0xFFA94A50, false, 0, 6, false);
    emit(bits, 20000, SGAP);
    // Two more gaps at a symbol's spacing, as a rippling carrier produces.
    push(WGAP, 336);
    push(WGAP, 336);
    push(SGAP, 20000);

    decode("fixed passworded read + 2 spurious gaps");
    CHECK("symbols", result.frame[0].symbols, 40);
    CHECK("not claimed", result.frame[0].kind, T5577SniffFrameUnknown);
    CHECK("no password", t5577_sniff_frame_tells_password(&result.frame[0]), false);

    begin();
    frame_bits(bits, false, true, 0x50415353, true, 0x00148040, 5, false);
    emit(bits, 20000, SGAP);
    push(WGAP, 336);
    push(WGAP, 336);
    push(SGAP, 20000);

    decode("fixed passworded write + 2 spurious gaps");
    CHECK("symbols", result.frame[0].symbols, 72);
    CHECK("not claimed", result.frame[0].kind, T5577SniffFrameUnknown);
    CHECK("no password", t5577_sniff_frame_tells_password(&result.frame[0]), false);
}

// t5577_block_read_bits() ends every read with the same read-mode command a write ends with, so
// the marker alone cannot tell them apart. What does is the wait before it: a write resumes in
// 8.8 ms, a read after hundreds of milliseconds of settling and capture.
static void test_marker_needs_the_pause(void) {
    char bits[128];

    begin();
    frame_bits(bits, false, true, 0xFFA94A50, false, 0, 6, false);
    emit(bits, 20000, SGAP);
    emit("10", 65535, SGAP); // the read path's ~600 ms, arriving at the uint16 ceiling
    push(SGAP, 20000);

    decode("passworded read, then a read-mode command");
    CHECK("stays ambiguous", result.frame[0].kind, T5577SniffFrameAmbiguous);
    CHECK("pause was capped", result.frame[0].pause_capped, true);
    CHECK("password still offered", result.frame[0].password, 0xFFA94A50);

    // The same marker at a write's programming time is a write.
    begin();
    frame_bits(bits, false, false, 0, true, 0x0F0F0F0F, 4, false);
    emit(bits, 20000, SGAP);
    emit("10", PAUSE_PGM, SGAP);
    push(SGAP, PAUSE_RD);

    decode("write, then a read-mode command");
    CHECK("kind", result.frame[0].kind, T5577SniffFramePlainWrite);
    CHECK("why", result.frame[0].why, T5577SniffWhyMarker);

    // And when the two disagree, physics wins: nothing can have programmed in 3.2 ms.
    begin();
    frame_bits(bits, false, true, 0x11223344, false, 0, 1, false);
    emit(bits, 20000, SGAP);
    emit("10", PAUSE_RD, SGAP);
    push(SGAP, PAUSE_RD);

    decode("short pause, then a read-mode command");
    CHECK("kind", result.frame[0].kind, T5577SniffFramePasswordedRead);
    CHECK("why", result.frame[0].why, T5577SniffWhyPause);
    CHECK("password", result.frame[0].password, 0x11223344);
}

// Every other case appends a trailing boundary, so the path where a capture simply stops was
// never taken - and that is the shape of every real capture.
static void test_capture_ends(void) {
    char bits[128];

    begin();
    frame_bits(bits, false, true, 0x50415353, true, 0x00148040, 7, false);
    emit(bits, 20000, SGAP);

    decode("passworded write, then the capture ends");
    CHECK("frames", result.frames, 1);
    CHECK("kind", result.frame[0].kind, T5577SniffFramePasswordedWrite);
    CHECK("nothing followed", result.frame[0].pause_us, 0);
    CHECK("not capped", result.frame[0].pause_capped, false);

    begin();
    frame_bits(bits, false, true, 0xFEEDFACE, false, 0, 2, false);
    emit(bits, 20000, SGAP);

    decode("ambiguous frame, then the capture ends");
    // A zero pause is the capture stopping, not a short one, so it must resolve nothing.
    CHECK("stays ambiguous", result.frame[0].kind, T5577SniffFrameAmbiguous);
    CHECK("why", result.frame[0].why, T5577SniffWhyNone);
}

// The lock bit was hardcoded zero in every frame the suite built, so every lock assertion in it
// would have passed against a decode that returned false always. It also sits in a different
// place in each of the two 38-bit readings.
static void test_lock_bit(void) {
    char bits[128];

    begin();
    frame_bits_locked(bits, false, false, 0, false, 0, 2, false, true);
    emit(bits, 20000, SGAP);
    push(SGAP, PAUSE_RD);

    decode("plain read, lock set");
    CHECK("kind", result.frame[0].kind, T5577SniffFramePlainRead);
    CHECK("lock", result.frame[0].lock, true);

    begin();
    frame_bits_locked(bits, false, true, 0x50415353, true, 0x00148040, 5, false, true);
    emit(bits, 20000, SGAP);
    push(SGAP, PAUSE_PGM);

    decode("passworded write, lock set");
    CHECK("kind", result.frame[0].kind, T5577SniffFramePasswordedWrite);
    CHECK("lock", result.frame[0].lock, true);
    CHECK("password", result.frame[0].password, 0x50415353);
    CHECK("data", result.frame[0].data, 0x00148040);

    // A passworded read resolved by its pause must report ITS lock bit, not the top bit of the
    // password the write reading would have found there.
    begin();
    frame_bits_locked(bits, false, true, 0x11223344, false, 0, 1, false, true);
    emit(bits, 20000, SGAP);
    emit(bits, PAUSE_RD, SGAP);
    push(SGAP, PAUSE_RD);

    decode("passworded read, lock set, resolved by pause");
    CHECK("kind", result.frame[0].kind, T5577SniffFramePasswordedRead);
    CHECK("password", result.frame[0].password, 0x11223344);
    CHECK("lock is the read reading's", result.frame[0].lock, true);
}

// A long leading reference is a stretched zero before the frame. Nothing else exercises at = 1.
static void test_long_leading_reference(void) {
    char bits[128];

    begin();
    frame_bits(bits, false, true, 0x0BADC0DE, true, 0x76543210, 3, false);
    push(SGAP, 20000);
    push(WGAP, SGAP + 1280); // the reference symbol: a zero stretched by 136 field clocks
    for(const char* at = bits; *at; at++) {
        push(WGAP, WGAP + ((*at == '1') ? ON1 : ON0));
    }
    push(SGAP, PAUSE_PGM);

    decode("long leading reference, passworded write");
    CHECK("symbols", result.frame[0].symbols, 71);
    CHECK("kind", result.frame[0].kind, T5577SniffFramePasswordedWrite);
    CHECK("password", result.frame[0].password, 0x0BADC0DE);
    CHECK("data", result.frame[0].data, 0x76543210);
    CHECK("block", result.frame[0].block, 3);
    CHECK("not leading zero", result.frame[0].leading_zero, false);
}

// The band filter, the debounce and the counters live in t5577_sniff_account(), which pushing
// into the ring bypasses entirely - so none of it had any coverage.
static void test_account(void) {
    char bits[128];

    begin();
    now_us = 100000;
    gap(39, 9000); // under the band
    gap(501, 9000); // over it
    gap(40, 9000); // the edges are kept
    gap(500, 9000);

    T5577SniffProgress progress;
    t5577_sniff_poll(sniff, &progress);
    printf("account band: %lu stalls %lu gaps\n", (unsigned long)progress.stalls,
           (unsigned long)progress.gaps);
    CHECK("every stall counted", progress.stalls, 4);
    CHECK("only the in-band ones", progress.gaps, 2);

    // A whole passworded write through the real entry point, with a ripple stall on every bit -
    // the debounce has to fold those without moving the anchor, or every period after one reads
    // short and the password comes back wrong.
    begin();
    now_us = 100000;
    frame_bits(bits, false, true, 0xA1B2C3D4, true, 0xDEADBEEF, 6, false);

    gap(SGAP, 20000);
    for(const char* at = bits; *at; at++) {
        const uint16_t on = (*at == '1') ? ON1 : ON0;
        gap(30, on - 30 - 40); // ripple, inside the debounce window
        gap(WGAP, 10);
    }
    gap(SGAP, 20000);

    decode("passworded write through account(), with ripple");
    CHECK("frames", result.frames, 1);
    CHECK("kind", result.frame[0].kind, T5577SniffFramePasswordedWrite);
    CHECK("password", result.frame[0].password, 0xA1B2C3D4);
    CHECK("data", result.frame[0].data, 0xDEADBEEF);
    CHECK("block", result.frame[0].block, 6);
    CHECK("a burst was counted", result.progress.bursts >= 1, true);
}

// A missed gap merges two symbols into a period longer than either class - 336 + 592 = 928 us.
// Under the old per-burst midpoint-of-extremes that one sample set the burst's maximum, dragged
// the split to 632 and decoded the WHOLE frame as zeroes. Measured across the capture it is one
// sample among hundreds, and the frames around it are untouched.
static void test_merged_symbol(void) {
    char bits[128];

    begin();
    frame_bits(bits, false, true, 0x50415353, true, 0x00148040, 7, false);
    emit(bits, 20000, SGAP);

    // The same command again, with one gap lost in the middle: two symbols become one.
    push(SGAP, PAUSE_PGM);
    uint16_t previous = SGAP;
    for(uint8_t i = 0; i < 70; i++) {
        const uint16_t on = (bits[i] == '1') ? ON1 : ON0;

        if(i == 30) {
            // The gap that should have ended this symbol never arrived, so its period and the
            // next one are measured as one.
            const uint16_t next = (bits[i + 1] == '1') ? ON1 : ON0;
            push(WGAP, previous + on + WGAP + next);
            previous = WGAP;
            i++;
            continue;
        }

        push(WGAP, previous + on);
        previous = WGAP;
    }
    push(SGAP, PAUSE_PGM);

    decode("a clean frame and one with a missed gap");
    CHECK("frames", result.frames, 2);
    CHECK("the clean frame decodes", result.frame[0].kind, T5577SniffFramePasswordedWrite);
    CHECK("its password", result.frame[0].password, 0x50415353);
    CHECK("its data", result.frame[0].data, 0x00148040);
    // The damaged one is a symbol short, so no layout matches and it says so rather than
    // decoding to something plausible.
    CHECK("the damaged frame", result.frame[1].kind, T5577SniffFrameUnknown);
    CHECK("its length is shown", result.frame[1].symbols, 69);
}

// A blue cloner, 2026-10-27: nine passworded writes 8.1 ms apart, buried in junk bursts whose
// periods run up near the pause threshold. Counting every burst put the estimate at 987 us
// against real classes of 381 and 762, so every symbol read zero and all nine decoded as
// nothing. Its timings are its own - neither this firmware's nor the Varta reader's.
#define BC_ON0  310
#define BC_ON1  690
#define BC_GAP  72
#define BC_SGAP 220

static void test_blue_cloner(void) {
    char bits[128];

    begin();
    on_0 = BC_ON0;
    on_1 = BC_ON1;

    // Junk first: bursts of a length no command has, at periods near the pause threshold.
    const uint8_t junk[] = {26, 16, 13, 28, 22};

    for(uint8_t j = 0; j < sizeof(junk); j++) {
        push(90, 9000);
        for(uint8_t i = 0; i < junk[j]; i++) push(90, 1500 + (i % 5) * 90);
    }

    // And a two-symbol burst, which IS a command length, carrying periods up near the pause
    // threshold. This is the shape that broke the clustering estimate: a handful of samples at
    // the top of the range pulled the boundary above both real classes.
    push(90, 9000);
    push(90, 1950);
    push(90, 1890);

    // Then the real sequence, nine passworded writes of the same password.
    for(uint8_t block = 0; block < 9; block++) {
        frame_bits(bits, false, true, 0xA5B6C7D8, true, 0x10000000 + block, block & 7, false);
        emit(bits, 8100, BC_SGAP);
    }
    push(BC_SGAP, 8100);

    decode("blue cloner: nine passworded writes among junk");
    CHECK("the split sits between the classes", result.split_us > BC_ON0 + BC_GAP, true);
    CHECK("and below the long class", result.split_us < BC_ON1 + BC_GAP, true);

    uint8_t writes = 0;

    for(uint8_t i = 0; i < result.frames; i++) {
        if(result.frame[i].kind != T5577SniffFramePasswordedWrite) continue;
        writes++;
        CHECK("password", result.frame[i].password, 0xA5B6C7D8);
    }

    CHECK("every write decoded", writes, 9);
}

int main(void) {
    sniff = t5577_sniff_alloc();

    test_plain_write();
    test_passworded_read();
    test_read_short_pause();
    test_passworded_write();
    test_leading_zero();
    test_short_commands();
    test_plain_read();
    test_wide_start_gap();
    test_restore_sequence();
    test_ring_wrap();
    test_overflow_frames();
    test_open_air();
    test_spurious_gaps();
    test_marker_needs_the_pause();
    test_capture_ends();
    test_lock_bit();
    test_long_leading_reference();
    test_account();
    test_merged_symbol();
    test_blue_cloner();

    t5577_sniff_free(sniff);

    printf("\n%s (%d failures)\n", failures ? "FAILED" : "all passed", failures);
    return failures ? 1 : 0;
}
