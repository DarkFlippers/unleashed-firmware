#pragma once

#ifdef __cplusplus
extern "C" {
#endif

#include <furi.h>
#include "u2f.h"

typedef struct U2fHid U2fHid;

U2fHid* u2f_hid_start(U2fData* u2f_inst);

void u2f_hid_stop(U2fHid* u2f_hid);

#ifdef __cplusplus
}
#endif
