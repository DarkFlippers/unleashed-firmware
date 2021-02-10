#pragma once

#include <api-hal-resources.h>

/* Input Types
 * Some of them are physical events and some logical
 */
typedef enum {
    InputTypePress, /* Press event, emitted after debounce */
    InputTypeRelease, /* Release event, emitted after debounce */
    InputTypeShort, /* Short event, emitted after InputTypeRelease done withing INPUT_LONG_PRESS interval */
    InputTypeLong, /* Long event, emmited after INPUT_LONG_PRESS interval, asynchronouse to InputTypeRelease  */
} InputType;

/* Input Event, dispatches with PubSub */
typedef struct {
    InputKey key;
    InputType type;
} InputEvent;
