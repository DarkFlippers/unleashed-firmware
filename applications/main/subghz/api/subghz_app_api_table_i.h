#include "../subghz_i.h"

/*
 * A list of app's private functions and objects to expose for plugins.
 * It is used to generate a table of symbols for import resolver to use.
 *
 * Keep this list small: anything exported here is pinned into the firmware
 * image for good, since the resolver counts as a reference.
 */
static constexpr auto subghz_app_api_table = sort(create_array_t<sym_entry>(
    // Frequency Analyzer: its worker walks the frequency list, its view refuses to hand back a
    // frequency the current radio cannot tune, and its scene persists the trigger and feedback
    // levels on the way out.
    API_METHOD(subghz_txrx_get_setting, SubGhzSetting*, (SubGhzTxRx*)),
    API_METHOD(subghz_txrx_radio_device_is_frequency_valid, bool, (SubGhzTxRx*, uint32_t)),
    API_METHOD(subghz_last_settings_save, bool, (SubGhzLastSettings*)),
    // Add Manually: every generator sets the radio preset before serializing, and each scene that
    // finishes a remote clears the loaded file name so the save lands on a fresh one.
    API_METHOD(subghz_txrx_set_preset, void, (SubGhzTxRx*, const char*, uint32_t, uint8_t*, size_t)),
    API_METHOD(subghz_file_name_clear, void, (SubGhz*))));
