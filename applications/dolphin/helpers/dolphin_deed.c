#include "dolphin_deed.h"
#include <furi.h>

static const DolphinDeedWeight dolphin_deed_weights[DolphinDeedMax] = {
    {1, -1, 60},
    {1, -1, 60},
    {1, -1, 60},
    {-1, 1, 60},
};

const DolphinDeedWeight* dolphin_deed_weight(DolphinDeed deed) {
    furi_assert(deed < DolphinDeedMax);
    return &dolphin_deed_weights[deed];
}
