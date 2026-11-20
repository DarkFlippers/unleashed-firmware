#include <furi.h>
#include <furi_hal.h>

#include <targets/f7/furi_hal/furi_hal_subghz_i.h>

#include <flipper_format/flipper_format_i.h>

void subghz_dangerous_freq() {
    bool is_extended_i = false;

    Storage* storage = furi_record_open(RECORD_STORAGE);
    FlipperFormat* fff_data_file = flipper_format_file_alloc(storage);

    if(flipper_format_file_open_existing(fff_data_file, "/ext/subghz/assets/dangerous_settings")) {
        flipper_format_read_bool(
            fff_data_file, "yes_i_want_to_destroy_my_flipper", &is_extended_i, 1);
    }

    furi_hal_subghz_set_dangerous_frequency(is_extended_i);

    flipper_format_free(fff_data_file);

    // Load the external module LED/amp setting straight from the Sub-GHz settings file,
    // so this hook does not depend on the Sub-GHz app, which ships as a FAP.
    bool leds_and_amp = true;
    FlipperFormat* settings = flipper_format_file_alloc(storage);
    FuriString* filetype = furi_string_alloc();
    uint32_t version = 0;
    if(flipper_format_file_open_existing(
           settings, EXT_PATH("subghz/assets/last_subghz.settings")) &&
       flipper_format_read_header(settings, filetype, &version) &&
       furi_string_equal_str(filetype, "Flipper SubGhz Last Setting File") && version == 3) {
        if(!flipper_format_read_bool(settings, "LedAndPowerAmp", &leds_and_amp, 1)) {
            leds_and_amp = true;
        }
    }
    furi_string_free(filetype);
    flipper_format_free(settings);

    furi_hal_subghz_set_ext_leds_and_amp(leds_and_amp);

    furi_record_close(RECORD_STORAGE);
}
