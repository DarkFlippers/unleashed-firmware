#pragma once

#include "dolphin_deed.h"

typedef struct Dolphin Dolphin;

/* Deed complete notification. Call it on deed completion.
 * See dolphin_deed.h for available deeds. In futures it will become part of assets.
 * Thread safe
 */
void dolphin_deed(Dolphin* dolphin, DolphinDeed deed);
