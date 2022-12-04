## .kl file format

A .kl file is a binary file format which stores, for each ascii code point in order from *ASCII(0)* to *ASCII(127)*, a 16 bit little endian value composed of a *USB HID code* in the low order byte and a *bit set of modifier keys* in the high order byte. 

*USB HID codes* for each key of a standard 102/105 key keyboard are (as displayed passing option `-k` to the software):


         ---------------------------------------------------------------------- 
        | 35 | 1e | 1f | 20 | 21 | 22 | 23 | 24 | 25 | 26 | 27 | 2d | 2e |  <- |
         ---------------------------------------------------------------------- 
        |  -> | 14 | 1a | 08 | 15 | 17 | 1c | 18 | 0c | 12 | 13 | 2f | 30 |    |
         ------------------------------------------------------------------|   |
        |   o  | 04 | 16 | 07 | 09 | 0a | 0b | 0d | 0e | 0f | 33 | 34 | 31 |   |
         ---------------------------------------------------------------------- 
        |  ^  | 64 | 1d | 1b | 06 | 19 | 05 | 11 | 10 | 36 | 37 | 38 |    ^    |
         ---------------------------------------------------------------------- 
        | ctrl | gui | alt |              2c                | alt | gui | ctrl |
         ---------------------------------------------------------------------- 

*Modifier key bits* are, in order starting from bit 1: *lctrl*, *lshift*, *lalt*, *lgui*, *rctrl*, *rshift*, *ralt*, *rgui*. 
