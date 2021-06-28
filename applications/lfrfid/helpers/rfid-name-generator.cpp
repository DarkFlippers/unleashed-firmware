#include "rfid-name-generator.h"
#include <stdio.h>
#include <stdlib.h>

void rfid_generate_random_name(char* name, uint8_t max_name_size) {
    const uint8_t prefix_size = 9;
    const char* prefix[prefix_size] = {
        "good",
        "nice",
        "best",
        "some",
        "strange",
        "working",
        "that",
        "forgettable",
        "easy",
    };

    const uint8_t suffix_size = 7;
    const char* suffix[suffix_size] = {
        "pass",
        "card",
        "key",
        "fob",
        "permit",
        "pass",
        "one",
    };

    sniprintf(
        name, max_name_size, "%s_%s", prefix[rand() % prefix_size], suffix[rand() % suffix_size]);

    // to upper
    name[0] = name[0] - ('a' - 'A');
}
