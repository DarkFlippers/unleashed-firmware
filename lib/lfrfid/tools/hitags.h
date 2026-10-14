#pragma once
#include <stdint.h>
#include <stdbool.h>

#ifdef __cplusplus
extern "C" {
#endif

#define LFRFID_HITAGS_PAGE_SIZE 4
#define LFRFID_HITAGS_UID_SIZE  4

// Data to clone an EM4100 ID onto an ID8268 / Hitag S magic chip, at RF/64 - the only rate the
// chip's factory TTF config emits. The tag streams pages 4 and 5 as the EM4100 frame, so the two
// halves go there; which page is which is hitags_write()'s business, not a caller's. All fields
// are MSB-first, exactly as they are transmitted.
typedef struct {
    uint8_t page4[LFRFID_HITAGS_PAGE_SIZE]; // EM4100 frame bits 63..32
    uint8_t page5[LFRFID_HITAGS_PAGE_SIZE]; // EM4100 frame bits 31..0
} LFRFIDHitagS;

/** Polled while a read runs; return true to give up. */
typedef bool (*HitagSAbortCallback)(void* context);

/** Read the UID of a Hitag S family tag, which SELECT needs before anything can be written.
 * Firmware internal, not exported to apps.
 *
 * Unlike its sibling writers this one direction is closed-loop, because it has to be: the UID is
 * per tag. The Flipper's envelope detector cannot resolve an anticollision '1', so the reply is
 * decoded from its '0' cells and the remaining unknowns - the leading bits and the frame alignment
 * - are put back to the tag as AC SEQUENCE prefixes, which only the tag whose UID starts that way
 * answers. A UID is therefore returned only once the tag itself has confirmed it.
 *
 * Budget: a 450 ms detector settle plus up to four interrogation passes of ~66 ms, so ~0.7 s to
 * conclude there is no tag and ~0.9 s to confirm one. A degraded capture that leaves many
 * alignments to put to the tag can take ten seconds.
 *
 * The 125 kHz field must be OFF on entry and is off again on return.
 *
 * @param      uid      Receives LFRFID_HITAGS_UID_SIZE bytes in air order (what Proxmark3's
 *                      `lf hitag hts reader` prints), untouched unless the call returns true
 * @param      abort    Polled while the detector settles and before every probe put to the tag, so
 *                      Back lands within ~50 ms. May be NULL.
 * @param      context  Passed to abort
 * @return     true if a tag confirmed its UID
 */
bool hitags_read_uid(uint8_t* uid, HitagSAbortCallback abort, void* context);

/** Write an EM4100 clone to an ID8268 / Hitag S magic chip.
 * Firmware internal, not exported to apps.
 *
 * Open-loop like em4305_write() and t5577_write(): the reader->tag frames are transmitted and no
 * response is read. Each page is written in its own power-cycled session - UID REQUEST, SELECT, the
 * 82xx password unlock, then the page - because the tag drops the session after a write. Success is
 * confirmed afterwards by the worker re-reading the EM4100 emulation.
 *
 * The 125 kHz field must be OFF on entry, as it is after hitags_read_uid() or a verify read, and is
 * off again on return.
 *
 * @param      data  The payload to clone; the caller fully populates page4/page5
 * @param      uid   LFRFID_HITAGS_UID_SIZE bytes from hitags_read_uid(); SELECT needs the real UID,
 *                   and a wrong one simply leaves the tag unselected and unwritten
 */
void hitags_write(const LFRFIDHitagS* data, const uint8_t* uid);

/** Check the CRC-8, the frame builders and the anticollision decoder against datasheet and
 * Proxmark3 vectors. Run by the lfrfid unit test suite; firmware internal, not exported to apps.
 *
 * A wrong CRC breaks every write silently and a drifting decoder returns a confident wrong UID,
 * and neither is visible without a tag and a Proxmark3 - so both are pinned here instead.
 *
 * @return     NULL when every vector passes, else the name of the first that did not
 */
const char* hitags_selftest(void);

#ifdef __cplusplus
}
#endif
