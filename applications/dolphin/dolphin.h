#pragma once

#include <core/pubsub.h>
#include "gui/view.h"
#include "helpers/dolphin_deed.h"
#include <stdbool.h>

#ifdef __cplusplus
extern "C" {
#endif

#define RECORD_DOLPHIN "dolphin"

typedef struct Dolphin Dolphin;

typedef struct {
    uint32_t icounter;
    uint32_t butthurt;
    uint64_t timestamp;
    uint8_t level;
    bool level_up_is_pending;
} DolphinStats;

typedef enum {
    DolphinPubsubEventUpdate,
} DolphinPubsubEvent;

#define DOLPHIN_DEED(deed)                                        \
    do {                                                          \
        Dolphin* dolphin = (Dolphin*)furi_record_open("dolphin"); \
        dolphin_deed(dolphin, deed);                              \
        furi_record_close("dolphin");                             \
    } while(0)

/** Deed complete notification. Call it on deed completion.
 * See dolphin_deed.h for available deeds. In futures it will become part of assets.
 * Thread safe, async
 */
void dolphin_deed(Dolphin* dolphin, DolphinDeed deed);

/** Retrieve dolphin stats
 * Thread safe, blocking
 */
DolphinStats dolphin_stats(Dolphin* dolphin);

/** Flush dolphin queue and save state
 * Thread safe, blocking
 */
void dolphin_flush(Dolphin* dolphin);

void dolphin_upgrade_level(Dolphin* dolphin);

FuriPubSub* dolphin_get_pubsub(Dolphin* dolphin);

#ifdef __cplusplus
}
#endif
