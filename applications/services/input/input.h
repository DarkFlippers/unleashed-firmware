/**
 * @file input.h
 * Input: main API
 */

#pragma once

#include <furi_hal_resources.h>

#ifdef __cplusplus
extern "C" {
#endif

#define RECORD_INPUT_EVENTS "input_events"

/** Input Types
 * Some of them are physical events and some logical
 */
typedef enum {
    InputTypePress, /**< Press event, emitted after debounce */
    InputTypeRelease, /**< Release event, emitted after debounce */
    InputTypeShort, /**< Short event, emitted after InputTypeRelease done withing INPUT_LONG_PRESS interval */
    InputTypeLong, /**< Long event, emmited after INPUT_LONG_PRESS interval, asynchronouse to InputTypeRelease  */
    InputTypeRepeat, /**< Repeat event, emmited with INPUT_REPEATE_PRESS period after InputTypeLong event */
} InputType;

/** Input Event, dispatches with FuriPubSub */
typedef struct {
    uint32_t sequence;
    InputKey key;
    InputType type;
} InputEvent;

/** Get human readable input key name
 * @param key - InputKey
 * @return string
 */
const char* input_get_key_name(InputKey key);

/** Get human readable input type name
 * @param type - InputType
 * @return string
 */
const char* input_get_type_name(InputType type);

#ifdef __cplusplus
}
#endif
