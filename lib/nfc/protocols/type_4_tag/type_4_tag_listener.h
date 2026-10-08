#pragma once

#include "type_4_tag.h"

#ifdef __cplusplus
extern "C" {
#endif

typedef struct Type4TagListener Type4TagListener;

typedef enum {
    Type4TagListenerEventTypeCustomCommand,
} Type4TagListenerEventType;

typedef struct {
    BitBuffer* buffer;
} Type4TagListenerEventData;

typedef struct {
    Type4TagListenerEventType type;
    Type4TagListenerEventData* data;
} Type4TagListenerEvent;

#ifdef __cplusplus
}
#endif
