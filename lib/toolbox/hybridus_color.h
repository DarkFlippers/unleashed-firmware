#pragma once
#include <stdint.h>
#include <stdbool.h>

// Hybridus UI colour setting. Private built-in API; public FAP SDK stays unchanged.
#ifdef __cplusplus
extern "C" {
#endif
// 0 = actual OTP colour, 1 = black, 2 = white, 3 = transparent.
void hybridus_color_load(void);
uint8_t hybridus_color_get(void);
bool hybridus_color_set(uint8_t color);

#ifdef __cplusplus
}
#endif
