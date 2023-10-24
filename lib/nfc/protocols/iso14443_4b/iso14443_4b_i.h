#pragma once

#include "iso14443_4b.h"

struct Iso14443_4bData {
    Iso14443_3bData* iso14443_3b_data;
};

Iso14443_4bError iso14443_4b_process_error(Iso14443_3bError error);
