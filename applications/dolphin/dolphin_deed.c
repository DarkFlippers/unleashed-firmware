#include "dolphin_deed.h"

static const DolphinDeedWeight dolphin_deed_weights[DolphinDeedMax] = {
    {1, 2, 60},
    {1, 2, 60},
    {1, 2, 60},
    {-1, 2, 60},
};

const DolphinDeedWeight* dolphin_deed_weight(DolphinDeed deed) {
    return &dolphin_deed_weights[deed];
}
