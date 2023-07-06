#include "subghz_test_frequency.h"

const uint32_t subghz_frequencies_testing[] = {
    /* 300 - 348 */
    300000000,
    304500000,
    310000000,
    312025000,
    313250000,
    313625000,
    315000000,
    315225000,
    321950000,
    348000000,
    /* 387 - 464 */
    387000000,
    433075000, /* LPD433 first */
    433825000,
    433920000, /* LPD433 mid */
    434420000,
    434775000, /* LPD433 last channels */
    438900000,
    464000000,
    /* 779 - 928 */
    779000000,
    868150000,
    868350000,
    868550000,
    915000000,
    925000000,
    926500000,
    927950000,
    928000000,
};

const uint32_t subghz_frequencies_count_testing =
    sizeof(subghz_frequencies_testing) / sizeof(uint32_t);
const uint32_t subghz_frequencies_433_92_testing = 13;
