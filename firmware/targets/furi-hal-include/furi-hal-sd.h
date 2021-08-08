#include <stdint.h>
#include <stdbool.h>

#ifdef __cplusplus
extern "C" {
#endif

/** Init SD card detect */
void hal_sd_detect_init(void);

/** Set SD card detect pin to low */
void hal_sd_detect_set_low(void);

/**
 * Get SD card status
 * @return true if SD card present
 * @return false if SD card not present
 */
bool hal_sd_detect(void);

#ifdef __cplusplus
}
#endif