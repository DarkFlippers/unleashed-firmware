#include "random_name.h"
#include <stdio.h>
#include <stdlib.h>
#include <stdbool.h>
#include <furi.h>

void set_random_name(char* name, uint8_t max_name_size) {
    static bool rand_generator_inited = false;

    if(!rand_generator_inited) {
        srand(DWT->CYCCNT);
        rand_generator_inited = true;
    }
    const char* prefix[] = {
        "super",
        "big",
        "little",
        "liquid",
        "unknown",
        "thin",
        "thick",
        "great",
        "my",
        "mini",
        "ultra",
        "haupt",
        "small",
        "random",
        "strange",
    };

    const char* suffix[] = {
        "maslina",
        "sus",
        "anomalija",
        "artefact",
        "monolit",
        "burer",
        "sidorovich",
        "habar",
        "radar",
        "borov",
        "pda",
        "konserva",
        "aptechka",
        "door",
        "thing",
        "stuff",
    };
    // sus is not (sus)pect - this is about super sus
    uint8_t prefix_i = rand() % COUNT_OF(prefix);
    uint8_t suffix_i = rand() % COUNT_OF(suffix);

    snprintf(name, max_name_size, "%s_%s", prefix[prefix_i], suffix[suffix_i]);
    // Set first symbol to upper case
    name[0] = name[0] - 0x20;
}