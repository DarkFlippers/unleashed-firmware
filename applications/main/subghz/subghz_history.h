
#pragma once

#include <math.h>
#include <furi.h>
#include <furi_hal.h>
#include <lib/flipper_format/flipper_format.h>
#include <lib/subghz/types.h>

#include "helpers/subghz_txrx.h"

/** Hard cap on entries, and the reason it is this exact number. m-lib grows its array by
 * MAX(8, n) * 2, so the steps are 16, 32, 64, 128, 256, and realloc here copies, leaving
 * the old and the new array live at once. The step to 256 would ask for 10k on top of a
 * heap the history has just filled, which is where it ran out. Stopping at 128 lands on a
 * step boundary, so that last doubling is never asked for. The number sits above what the
 * heap can hold anyway, so it is a backstop rather than a limit anyone should meet */
#define SUBGHZ_HISTORY_CAPACITY 128

/** No slot limit below the capacity, the heap is the limit. The send block is held out of
 * the heap, so this covers the record about to be stored plus the menu entry, the status
 * bar strings and the scene work that follow it. With old signals being deleted to make
 * room the list sits at this margin for as long as it keeps receiving, so every one of
 * those allocations happens at the edge and the margin has to hold the largest of them */
#define SUBGHZ_HISTORY_RESERVE_ITEM 6144

/** A BinRAW capture prints up to 2048 bytes as hex text and grows by realloc, so the old
 * and the new copy are live at once */
#define SUBGHZ_HISTORY_RESERVE_RAW 14336

/** Largest single allocation one record makes, which has to come out of one block. For a
 * decoded signal that is the string the protocol serializes itself into, a few hundred
 * bytes. Asking for more than it needs makes a delete free nothing until several freed
 * records happen to merge, which is what made one signal drop a whole run of them. The
 * string is grown by doubling, so the last step asks for as much again as it already holds */
#define SUBGHZ_HISTORY_BLOCK_ITEM 1024
#define SUBGHZ_HISTORY_BLOCK_RAW  6144

/** What the allocator puts in front of a block, rounded up to its alignment. A free block
 * of exactly N bytes cannot serve a request for N, so every size checked here carries this
 * on top - and no more than this, or a block just freed reads as too small to take back */
#define SUBGHZ_HISTORY_BLOCK_OVERHEAD 8

/** Extra room a full history has to get back before it counts as not full again. At the
 * point it fills up the margin is a few hundred bytes, and anything else that runs - a
 * signal opened and closed, a send - moves the heap by more than that, so a bare recheck
 * flips the answer back and forth. Once it reads full it stays full until there is real
 * room again, or until a record is deleted */
#define SUBGHZ_HISTORY_FULL_CLEAR_SLACK 2048

typedef struct SubGhzHistory SubGhzHistory;

/** Can the history take no more signals of this type? Either the heap says so, or the
 * reserved capacity is used up - the latter is set past what the heap can hold, so it is
 * a backstop against the array doubling rather than a limit anyone should meet
 *
 * @param   instance  SubGhzHistory instance
 * @param   type      SubGhzProtocolType of the signal about to be stored
 * @return  bool
 */
bool subghz_history_is_full(SubGhzHistory* instance, uint8_t type);

/** Is what the history is short of plain heap? Dropping a record always returns some, so
 * while this is true dropping another one gets closer to room. A block wide enough for a
 * big signal is the other case: that only appears if freed records happen to sit next to
 * each other, and a run of small ones never will, so whoever is making room has to stop
 * there instead of emptying the list for a signal that gets turned away anyway
 *
 * @param   instance  SubGhzHistory instance
 * @param   type      SubGhzProtocolType of the signal about to be stored
 * @return  bool
 */
bool subghz_history_needs_heap(SubGhzHistory* instance, uint8_t type);

/** Allocate SubGhzHistory
 * 
 * @return SubGhzHistory* 
 */
SubGhzHistory* subghz_history_alloc(void);

/** Free SubGhzHistory
 * 
 * @param instance - SubGhzHistory instance
 */
void subghz_history_free(SubGhzHistory* instance);

/** Clear history
 * 
 * @param instance - SubGhzHistory instance
 */
void subghz_history_reset(SubGhzHistory* instance);

void subghz_history_delete_item(SubGhzHistory* instance, uint16_t idx);

/** Get frequency to history[idx]
 * 
 * @param instance  - SubGhzHistory instance
 * @param idx       - record index  
 * @return frequency - frequency Hz
 */
uint32_t subghz_history_get_frequency(SubGhzHistory* instance, uint16_t idx);

SubGhzRadioPreset* subghz_history_get_radio_preset(SubGhzHistory* instance, uint16_t idx);

/** Get preset to history[idx]
 * 
 * @param instance  - SubGhzHistory instance
 * @param idx       - record index  
 * @return preset   - preset name
 */
const char* subghz_history_get_preset(SubGhzHistory* instance, uint16_t idx);

/** Get history index write 
 * 
 * @param instance  - SubGhzHistory instance
 * @return idx      - current record index  
 */
uint16_t subghz_history_get_item(SubGhzHistory* instance);

/** Get type protocol to history[idx]
 * 
 * @param instance  - SubGhzHistory instance
 * @param idx       - record index  
 * @return type      - type protocol  
 */
uint8_t subghz_history_get_type_protocol(SubGhzHistory* instance, uint16_t idx);

/** Get name protocol to history[idx]
 * 
 * @param instance  - SubGhzHistory instance
 * @param idx       - record index  
 * @return name      - const char* name protocol  
 */
const char* subghz_history_get_protocol_name(SubGhzHistory* instance, uint16_t idx);

/** Get datetime from history[idx]
 * 
 * @param instance  - SubGhzHistory instance
 * @param idx       - record index  
 * @return datetime - DateTime received timestamp  
 */
DateTime subghz_history_get_datetime(SubGhzHistory* instance, uint16_t idx);

/** Get string item menu to history[idx]
 * 
 * @param instance  - SubGhzHistory instance
 * @param output    - FuriString* output
 * @param idx       - record index
 */
void subghz_history_get_text_item_menu(SubGhzHistory* instance, FuriString* output, uint16_t idx);

/** Get time item menu to history[idx]
 * 
 * @param instance  - SubGhzHistory instance
 * @param output    - FuriString* output
 * @param idx       - record index
 */
void subghz_history_get_time_item_menu(SubGhzHistory* instance, FuriString* output, uint16_t idx);

/** Get string the remaining number of records to history
 * 
 * @param instance  - SubGhzHistory instance
 * @param output    - FuriString* output
 * @param type      - SubGhzProtocolType of the signal about to be stored
 * @param unlimited - true while old signals are dropped to make room, so the history
 *                    never reads as full
 * @return bool - is FULL
 */
bool subghz_history_get_text_space_left(
    SubGhzHistory* instance,
    FuriString* output,
    uint8_t type,
    bool unlimited);

/** Return last index
 *
 * @param instance - SubGhzHistory instance
 * @return
 */
uint16_t subghz_history_get_last_index(SubGhzHistory* instance);

/** Add protocol to history
 * 
 * Repeats of the same signal are dropped while they keep arriving within a short
 * window of each other. The window is measured with air_time so that it covers
 * the air between the two frames, not the wall time between the moments the app
 * was told about them - decoding can be suspended (a scene is pushed on top of
 * it, RX is restarted, ...) and a frame can be reported long after it was sent.
 *
 * @param instance  - SubGhzHistory instance
 * @param context    - SubGhzProtocolCommon context
 * @param preset    - SubGhzRadioPreset preset
 * @param air_time  - air decoded so far, ms, see subghz_txrx_get_air_time_ms()
 * @return bool;
 */
bool subghz_history_add_to_history(
    SubGhzHistory* instance,
    void* context,
    SubGhzRadioPreset* preset,
    uint32_t air_time);

/** Get SubGhzProtocolCommonLoad to load into the protocol decoder bin data
 * 
 * @param instance  - SubGhzHistory instance
 * @param idx       - record index
 * @return SubGhzProtocolCommonLoad*
 */
FlipperFormat* subghz_history_get_raw_data(SubGhzHistory* instance, uint16_t idx);
