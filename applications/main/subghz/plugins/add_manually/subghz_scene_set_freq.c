#include <subghz/subghz_i.h>
#include "subghz_add_manually_i.h"

typedef enum {
    Band433_92AM650,
    Band433_92FM12K,
    Band433_42AM650,
    Band434_42FM476,
    Band434_42FM12K,
    Band868_35AM650,
    Band868_00AM650,
    Band868_46AM650,
    Band868_80FM476,
    Band868_80FM12K,
    Band300_00AM650,
    Band310_00AM650,
    Band315_00AM650,
    Band390_00AM650,
    Band430_50AM650,
    BandCount,
} Band;

/** A frequency with the modulation it is used on, as the picker lists it. */
typedef struct {
    uint32_t freq;
    const char* mod;
    const char* name;
} SetTypeBand;

/* Every band Add Manually generates on, 433 first, then 868, then the rest. A system's own bands
   head the picker; the others stay reachable for an installation that uses an odd one. */
static const SetTypeBand set_type_bands[BandCount] = {
    [Band433_92AM650] = {433920000, "AM650", "433.92 AM650"},
    [Band433_92FM12K] = {433920000, "FM12K", "433.92 FM12K"},
    [Band433_42AM650] = {433420000, "AM650", "433.42 AM650"},
    [Band434_42FM476] = {434420000, "FM476", "434.42 FM476"},
    [Band434_42FM12K] = {434420000, "FM12K", "434.42 FM12K"},
    [Band868_35AM650] = {868350000, "AM650", "868.35 AM650"},
    [Band868_00AM650] = {868000000, "AM650", "868.00 AM650"},
    [Band868_46AM650] = {868460000, "AM650", "868.46 AM650"},
    [Band868_80FM476] = {868800000, "FM476", "868.80 FM476"},
    [Band868_80FM12K] = {868800000, "FM12K", "868.80 FM12K"},
    [Band300_00AM650] = {300000000, "AM650", "300.00 AM650"},
    [Band310_00AM650] = {310000000, "AM650", "310.00 AM650"},
    [Band315_00AM650] = {315000000, "AM650", "315.00 AM650"},
    [Band390_00AM650] = {390000000, "AM650", "390.00 AM650"},
    [Band430_50AM650] = {430500000, "AM650", "430.50 AM650"},
};

/** A band a system uses besides the one its own entry generates on. */
typedef struct {
    SetType row;
    uint8_t band;
    /* The variant whose generation data that band needs - the same system unless the band changes
       more than the radio, which is how Sommer offers both of its modulations from one row. */
    SetType data;
} SetTypeAltBand;

static const SetTypeAltBand set_type_alt_bands[] = {
    {SetTypeAllmatic, Band868_35AM650, SetTypeAllmatic},
    {SetTypeBeninca, Band868_35AM650, SetTypeBeninca},
    {SetTypeCAME12bit, Band868_35AM650, SetTypeCAME12bit},
    {SetTypeCAME24bit, Band868_35AM650, SetTypeCAME24bit},
    {SetTypeCameAtomo, Band868_35AM650, SetTypeCameAtomo},
    {SetTypeComunello, Band868_35AM650, SetTypeComunello},
    {SetTypeDoorHan, Band315_00AM650, SetTypeDoorHan},
    {SetTypeErreka, Band868_35AM650, SetTypeErreka},
    {SetTypeFAACRCXT, Band868_35AM650, SetTypeFAACRCXT},
    {SetTypeFaacSLH, Band868_35AM650, SetTypeFaacSLH},
    {SetTypeFaacTm, Band868_35AM650, SetTypeFaacTm},
    {SetTypeGenius, Band868_35AM650, SetTypeGenius},
    {SetTypeMarantec, Band868_35AM650, SetTypeMarantec},
    {SetTypePricenton, Band315_00AM650, SetTypePricenton},
    {SetTypeSecPlus_v1, Band315_00AM650, SetTypeSecPlus_v1},
    {SetTypeSecPlus_v1, Band390_00AM650, SetTypeSecPlus_v1},
    {SetTypeSecPlus_v2, Band310_00AM650, SetTypeSecPlus_v2},
    {SetTypeSecPlus_v2, Band315_00AM650, SetTypeSecPlus_v2},
    {SetTypeSecPlus_v2, Band390_00AM650, SetTypeSecPlus_v2},
    {SetTypeSommer, Band868_80FM12K, SetTypeSommer},
    {SetTypeSommer, Band434_42FM476, SetTypeSommer_FM476},
    {SetTypeSommer, Band868_80FM476, SetTypeSommer_FM476},
};

/* Rebuilt on every entry, so the row's own bands lead and the item index can address them. */
static uint8_t band_order[BandCount];

static void subghz_scene_set_freq_submenu_callback(void* context, uint32_t index) {
    SubGhz* subghz = context;

    view_dispatcher_send_custom_event(subghz->view_dispatcher, index);
}

static SetType subghz_scene_set_freq_row(SubGhz* subghz) {
    return scene_manager_get_scene_state(subghz->scene_manager, SubGhzSceneSetType);
}

void subghz_scene_set_freq_on_enter(void* context) {
    SubGhz* subghz = context;
    SetType row = subghz_scene_set_freq_row(subghz);

    /* Which band a system generates on is part of its data, so fill it to read that. The band
       picked here decides which variant gets filled for good. */
    subghz_scene_set_type_fill(subghz, row);

    bool listed[BandCount] = {0};
    uint8_t count = 0;
    for(uint8_t i = 0; i < BandCount; i++) {
        if((set_type_bands[i].freq == subghz->gen_info->freq) &&
           (strcmp(set_type_bands[i].mod, subghz->gen_info->mod) == 0)) {
            band_order[count++] = i;
            listed[i] = true;
            break;
        }
    }
    for(size_t i = 0; i < COUNT_OF(set_type_alt_bands); i++) {
        const SetTypeAltBand* alt = &set_type_alt_bands[i];
        if((alt->row == row) && !listed[alt->band]) {
            band_order[count++] = alt->band;
            listed[alt->band] = true;
        }
    }
    for(uint8_t i = 0; i < BandCount; i++) {
        if(!listed[i]) {
            band_order[count++] = i;
        }
    }
    furi_check(count == BandCount);

    submenu_set_header(subghz->submenu, subghz_set_type_name(row));
    for(uint8_t i = 0; i < BandCount; i++) {
        submenu_add_item(
            subghz->submenu,
            set_type_bands[band_order[i]].name,
            i,
            subghz_scene_set_freq_submenu_callback,
            subghz);
    }

    view_dispatcher_switch_to_view(subghz->view_dispatcher, SubGhzViewIdMenu);
}

bool subghz_scene_set_freq_on_event(void* context, SceneManagerEvent event) {
    SubGhz* subghz = context;

    if((event.type != SceneManagerEventTypeCustom) || (event.event >= BandCount)) {
        return false;
    }

    SetType row = subghz_scene_set_freq_row(subghz);
    uint8_t band = band_order[event.event];

    SetType data = row;
    for(size_t i = 0; i < COUNT_OF(set_type_alt_bands); i++) {
        if((set_type_alt_bands[i].row == row) && (set_type_alt_bands[i].band == band)) {
            data = set_type_alt_bands[i].data;
            break;
        }
    }

    subghz_scene_set_type_fill(subghz, data);
    subghz->gen_info->freq = set_type_bands[band].freq;
    subghz->gen_info->mod = set_type_bands[band].mod;

    return subghz_scene_set_type_proceed(subghz);
}

void subghz_scene_set_freq_on_exit(void* context) {
    SubGhz* subghz = context;

    submenu_reset(subghz->submenu); // Takes the system's header with it
}
