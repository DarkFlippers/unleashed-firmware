/* https://github.com/openocd-org/openocd/blob/master/src/helper/ */
/* SPDX-License-Identifier: GPL-2.0-or-later */

/***************************************************************************
 *   Copyright (C) 2015 Andreas Fritiofson                                 *
 *   andreas.fritiofson@gmail.com                                          *
 ***************************************************************************/

#ifndef OPENOCD_HELPER_JEP106_H
#define OPENOCD_HELPER_JEP106_H

/**
 * Get the manufacturer name associated with a JEP106 ID.
 * @param bank The bank (number of continuation codes) of the manufacturer ID.
 * @param id The 7-bit manufacturer ID (i.e. with parity stripped).
 * @return A pointer to static const storage containing the name of the
 *         manufacturer associated with bank and id, or one of the strings
 *         "<invalid>" and "<unknown>".
 */
const char* jep106_table_manufacturer(unsigned int bank, unsigned int id);

static inline const char* jep106_manufacturer(unsigned int manufacturer) {
    return jep106_table_manufacturer(manufacturer >> 7, manufacturer & 0x7f);
}

#endif /* OPENOCD_HELPER_JEP106_H */
