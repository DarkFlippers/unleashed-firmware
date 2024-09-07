#pragma once

#include <stdbool.h>
#include <core/pubsub.h>

#include "helpers/dolphin_deed.h"

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

typedef struct {
    bool happy_mode;
} DolphinSettings;

typedef enum {
    DolphinPubsubEventUpdate,
} DolphinPubsubEvent;

/** Deed complete notification. Call it on deed completion.
 * See dolphin_deed.h for available deeds. In futures it will become part of assets.
 * Thread safe, async
 */
void dolphin_deed(DolphinDeed deed);

void dolphin_get_settings(Dolphin* dolphin, DolphinSettings* settings);

void dolphin_set_settings(Dolphin* dolphin, DolphinSettings* settings);

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
