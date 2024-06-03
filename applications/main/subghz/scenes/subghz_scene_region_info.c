#include "../subghz_i.h" // IWYU pragma: keep

#include <furi_hal_region.h>

void subghz_scene_region_info_on_enter(void* context) {
    SubGhz* subghz = context;
    const FuriHalRegion* const region = furi_hal_region_get();
    FuriString* buffer = furi_string_alloc();

    if(region) {
        furi_string_cat_printf(buffer, "Region: %s\nBands:\n", region->country_code);
        for(uint16_t i = 0; i < region->bands_count; ++i) {
            furi_string_cat_printf(
                buffer,
                "%lu-%lu kHz\n",
                region->bands[i].start / 1000,
                region->bands[i].end / 1000);
        }
    } else {
        furi_string_cat_printf(buffer, "Region: N/A\n");
    }

    widget_add_string_multiline_element(
        subghz->widget, 0, 0, AlignLeft, AlignTop, FontPrimary, "Region Information");

    widget_add_string_multiline_element(
        subghz->widget, 0, 13, AlignLeft, AlignTop, FontSecondary, furi_string_get_cstr(buffer));

    furi_string_free(buffer);
    view_dispatcher_switch_to_view(subghz->view_dispatcher, SubGhzViewIdWidget);
}

bool subghz_scene_region_info_on_event(void* context, SceneManagerEvent event) {
    UNUSED(context);
    UNUSED(event);
    return false;
}

void subghz_scene_region_info_on_exit(void* context) {
    SubGhz* subghz = context;
    widget_reset(subghz->widget);
}
