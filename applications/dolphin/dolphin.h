#pragma once

#include "helpers/dolphin_deed.h"

typedef struct Dolphin Dolphin;

/* Load Dolphin state
 * Thread safe
 */

bool dolphin_load(Dolphin* dolphin);

/* Deed complete notification. Call it on deed completion.
 * See dolphin_deed.h for available deeds. In futures it will become part of assets.
 * Thread safe
 */

void dolphin_deed(Dolphin* dolphin, DolphinDeed deed);

/* Save Dolphin state (write to permanent memory)
 * Thread safe
 */

void dolphin_save(Dolphin* dolphin);

/* Retrieve dolphin's icounter and butthurt values
 * Thread safe
 */

DolphinDeedWeight dolphin_stats(Dolphin* dolphin);