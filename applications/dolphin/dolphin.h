#pragma once

#include "helpers/dolphin_deed.h"

typedef struct Dolphin Dolphin;

typedef struct {
    uint32_t icounter;
    uint32_t butthurt;
} DolphinStats;

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