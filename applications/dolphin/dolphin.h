#pragma once

#include "furi/pubsub.h"
#include "helpers/dolphin_deed.h"
#include <stdbool.h>

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
