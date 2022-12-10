
#pragma once

#include <math.h>
#include <furi.h>
#include <furi_hal.h>
#include <lib/flipper_format/flipper_format.h>
#include <lib/subghz/types.h>

typedef struct PCSGHistory PCSGHistory;

/** History state add key */
typedef enum {
    PCSGHistoryStateAddKeyUnknown,
    PCSGHistoryStateAddKeyTimeOut,
    PCSGHistoryStateAddKeyNewDada,
    PCSGHistoryStateAddKeyUpdateData,
    PCSGHistoryStateAddKeyOverflow,
} PCSGHistoryStateAddKey;

/** Allocate PCSGHistory
 * 
 * @return PCSGHistory* 
 */
PCSGHistory* pcsg_history_alloc(void);

/** Free PCSGHistory
 * 
 * @param instance - PCSGHistory instance
 */
void pcsg_history_free(PCSGHistory* instance);

/** Clear history
 * 
 * @param instance - PCSGHistory instance
 */
void pcsg_history_reset(PCSGHistory* instance);

/** Get frequency to history[idx]
 * 
 * @param instance  - PCSGHistory instance
 * @param idx       - record index  
 * @return frequency - frequency Hz
 */
uint32_t pcsg_history_get_frequency(PCSGHistory* instance, uint16_t idx);

SubGhzRadioPreset* pcsg_history_get_radio_preset(PCSGHistory* instance, uint16_t idx);

/** Get preset to history[idx]
 * 
 * @param instance  - PCSGHistory instance
 * @param idx       - record index  
 * @return preset   - preset name
 */
const char* pcsg_history_get_preset(PCSGHistory* instance, uint16_t idx);

/** Get history index write 
 * 
 * @param instance  - PCSGHistory instance
 * @return idx      - current record index  
 */
uint16_t pcsg_history_get_item(PCSGHistory* instance);

/** Get type protocol to history[idx]
 * 
 * @param instance  - PCSGHistory instance
 * @param idx       - record index  
 * @return type      - type protocol  
 */
uint8_t pcsg_history_get_type_protocol(PCSGHistory* instance, uint16_t idx);

/** Get name protocol to history[idx]
 * 
 * @param instance  - PCSGHistory instance
 * @param idx       - record index  
 * @return name      - const char* name protocol  
 */
const char* pcsg_history_get_protocol_name(PCSGHistory* instance, uint16_t idx);

/** Get string item menu to history[idx]
 * 
 * @param instance  - PCSGHistory instance
 * @param output    - FuriString* output
 * @param idx       - record index
 */
void pcsg_history_get_text_item_menu(PCSGHistory* instance, FuriString* output, uint16_t idx);

/** Get string the remaining number of records to history
 * 
 * @param instance  - PCSGHistory instance
 * @param output    - FuriString* output
 * @return bool - is FUUL
 */
bool pcsg_history_get_text_space_left(PCSGHistory* instance, FuriString* output);

/** Add protocol to history
 * 
 * @param instance  - PCSGHistory instance
 * @param context    - SubGhzProtocolCommon context
 * @param preset    - SubGhzRadioPreset preset
 * @return PCSGHistoryStateAddKey;
 */
PCSGHistoryStateAddKey
    pcsg_history_add_to_history(PCSGHistory* instance, void* context, SubGhzRadioPreset* preset);

/** Get SubGhzProtocolCommonLoad to load into the protocol decoder bin data
 * 
 * @param instance  - PCSGHistory instance
 * @param idx       - record index
 * @return SubGhzProtocolCommonLoad*
 */
FlipperFormat* pcsg_history_get_raw_data(PCSGHistory* instance, uint16_t idx);
