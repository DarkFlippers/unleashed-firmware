#pragma once

#include <stdbool.h>
#include <stdint.h>

#ifdef __cplusplus
extern "C" {
#endif

/*
 * Additinal non-connetable beacon API.
 * Not to be used directly, but through furi_hal_bt_extra_beacon_* APIs.
 */

#define EXTRA_BEACON_MAX_DATA_SIZE (31)
#define EXTRA_BEACON_MAC_ADDR_SIZE (6)

typedef enum {
    GapAdvChannelMap37 = 0b001,
    GapAdvChannelMap38 = 0b010,
    GapAdvChannelMap39 = 0b100,
    GapAdvChannelMapAll = 0b111,
} GapAdvChannelMap;

typedef enum {
    GapAdvPowerLevel_Neg40dBm = 0x00,
    GapAdvPowerLevel_Neg20_85dBm = 0x01,
    GapAdvPowerLevel_Neg19_75dBm = 0x02,
    GapAdvPowerLevel_Neg18_85dBm = 0x03,
    GapAdvPowerLevel_Neg17_6dBm = 0x04,
    GapAdvPowerLevel_Neg16_5dBm = 0x05,
    GapAdvPowerLevel_Neg15_25dBm = 0x06,
    GapAdvPowerLevel_Neg14_1dBm = 0x07,
    GapAdvPowerLevel_Neg13_15dBm = 0x08,
    GapAdvPowerLevel_Neg12_05dBm = 0x09,
    GapAdvPowerLevel_Neg10_9dBm = 0x0A,
    GapAdvPowerLevel_Neg9_9dBm = 0x0B,
    GapAdvPowerLevel_Neg8_85dBm = 0x0C,
    GapAdvPowerLevel_Neg7_8dBm = 0x0D,
    GapAdvPowerLevel_Neg6_9dBm = 0x0E,
    GapAdvPowerLevel_Neg5_9dBm = 0x0F,
    GapAdvPowerLevel_Neg4_95dBm = 0x10,
    GapAdvPowerLevel_Neg4dBm = 0x11,
    GapAdvPowerLevel_Neg3_15dBm = 0x12,
    GapAdvPowerLevel_Neg2_45dBm = 0x13,
    GapAdvPowerLevel_Neg1_8dBm = 0x14,
    GapAdvPowerLevel_Neg1_3dBm = 0x15,
    GapAdvPowerLevel_Neg0_85dBm = 0x16,
    GapAdvPowerLevel_Neg0_5dBm = 0x17,
    GapAdvPowerLevel_Neg0_15dBm = 0x18,
    GapAdvPowerLevel_0dBm = 0x19,
    GapAdvPowerLevel_1dBm = 0x1A,
    GapAdvPowerLevel_2dBm = 0x1B,
    GapAdvPowerLevel_3dBm = 0x1C,
    GapAdvPowerLevel_4dBm = 0x1D,
    GapAdvPowerLevel_5dBm = 0x1E,
    GapAdvPowerLevel_6dBm = 0x1F,
} GapAdvPowerLevelInd;

typedef enum {
    GapAddressTypePublic = 0,
    GapAddressTypeRandom = 1,
} GapAddressType;

typedef struct {
    uint16_t min_adv_interval_ms, max_adv_interval_ms;
    GapAdvChannelMap adv_channel_map;
    GapAdvPowerLevelInd adv_power_level;
    GapAddressType address_type;
    uint8_t address[EXTRA_BEACON_MAC_ADDR_SIZE];
} GapExtraBeaconConfig;

typedef enum {
    GapExtraBeaconStateUndefined = 0,
    GapExtraBeaconStateStopped,
    GapExtraBeaconStateStarted,
} GapExtraBeaconState;

void gap_extra_beacon_init(void);

GapExtraBeaconState gap_extra_beacon_get_state(void);

bool gap_extra_beacon_start(void);

bool gap_extra_beacon_stop(void);

bool gap_extra_beacon_set_config(const GapExtraBeaconConfig* config);

const GapExtraBeaconConfig* gap_extra_beacon_get_config(void);

bool gap_extra_beacon_set_data(const uint8_t* data, uint8_t length);

// Fill "data" with last configured extra beacon data and return its length
uint8_t gap_extra_beacon_get_data(uint8_t* data);

#ifdef __cplusplus
}
#endif
