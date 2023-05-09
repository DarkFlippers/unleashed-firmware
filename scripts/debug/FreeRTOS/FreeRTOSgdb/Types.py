# File: Types.py
# Author: Carl Allendorph
# Date: 05NOV2014
#
# Description:
#    This file contains the implementation of some
# standard types

import gdb


class StdTypes:
    uint32_t = gdb.lookup_type("uint32_t")
    uint16_t = gdb.lookup_type("uint16_t")
