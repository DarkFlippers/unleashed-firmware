/* https://github.com/openocd-org/openocd/blob/master/src/helper/ */
// SPDX-License-Identifier: GPL-2.0-or-later

/***************************************************************************
 *   Copyright (C) 2015 Andreas Fritiofson                                 *
 *   andreas.fritiofson@gmail.com                                          *
 ***************************************************************************/

#include "jep106.h"

static const char* const jep106[][126] = {
#include "jep106.inc"
};

const char* jep106_table_manufacturer(unsigned int bank, unsigned int id) {
    if(id < 1 || id > 126) {
        return "<invalid>";
    }

    /* index is zero based */
    id--;

    if(bank >= 14 || jep106[bank][id] == 0) return "<unknown>";

    return jep106[bank][id];
}
