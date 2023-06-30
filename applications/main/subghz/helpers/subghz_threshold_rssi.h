#pragma once

#include <furi.h>

typedef struct {
    float rssi; /**< Current RSSI */
    bool is_above; /**< Exceeded threshold level */
} SubGhzThresholdRssiData;

typedef struct SubGhzThresholdRssi SubGhzThresholdRssi;

/** Allocate SubGhzThresholdRssi
 * 
 * @return SubGhzThresholdRssi* 
 */
SubGhzThresholdRssi* subghz_threshold_rssi_alloc(void);

/** Free SubGhzThresholdRssi
 * 
 * @param instance Pointer to a SubGhzThresholdRssi
 */
void subghz_threshold_rssi_free(SubGhzThresholdRssi* instance);

/** Set threshold
 * 
 * @param instance Pointer to a SubGhzThresholdRssi
 * @param rssi RSSI threshold
 */
void subghz_threshold_rssi_set(SubGhzThresholdRssi* instance, float rssi);

/** Get threshold
 * 
 * @param instance Pointer to a SubGhzThresholdRssi
 * @return float RSSI threshold
 */
float subghz_threshold_rssi_get(SubGhzThresholdRssi* instance);

/** Check threshold
 * 
 * @param instance Pointer to a SubGhzThresholdRssi
 * @param rssi Current RSSI
 * @return SubGhzThresholdRssiData 
 */
SubGhzThresholdRssiData subghz_threshold_get_rssi_data(SubGhzThresholdRssi* instance, float rssi);
