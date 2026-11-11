#include "../remote_analyzer_i.h"

enum RaSettingIndex {
    RaSettingIndexFrequency,
    RaSettingIndexModulation,
    RaSettingIndexHopping,
    RaSettingIndexSound,
    RaSettingIndexSweep,
    RaSettingIndexAlert,
    RaSettingIndexResetToDefault,
};

#define COMBO_BOX_COUNT 2

static const char* const combobox_text[COMBO_BOX_COUNT] = {
    "OFF",
    "ON",
};

/**
 * Off, plus the same ladder the Sub-GHz receiver config offers for its own hopping threshold.
 * One item, because the level is what hopping stops on - there is no hopping without it.
 */
#define RA_HOPPING_COUNT 12
/** Levels only: index 0 of the item is OFF, so a chosen level is at index + 1. */
#define RA_TRIGGER_COUNT (RA_HOPPING_COUNT - 1)

static const char* const hopping_text[RA_HOPPING_COUNT] = {
    "OFF",
    "-90dB",
    "-85dB",
    "-80dB",
    "-75dB",
    "-70dB",
    "-65dB",
    "-60dB",
    "-55dB",
    "-50dB",
    "-45dB",
    "-40dB",
};

static const float trigger_value[RA_TRIGGER_COUNT] = {
    -90.0f,
    -85.0f,
    -80.0f,
    -75.0f,
    -70.0f,
    -65.0f,
    -60.0f,
    -55.0f,
    -50.0f,
    -45.0f,
    -40.0f,
};

/** Hopping takes the frequency out of the user's hands, so the item says so instead. */
static void ra_scene_read_config_frequency_text(RemoteAnalyzer* app, VariableItem* item) {
    if(app->settings.hopping) {
        variable_item_set_current_value_text(item, "-----");
        return;
    }

    char text_buf[12] = {0};
    snprintf(
        text_buf,
        sizeof(text_buf),
        "%lu.%02lu",
        app->settings.frequency / 1000000,
        (app->settings.frequency % 1000000) / 10000);
    variable_item_set_current_value_text(item, text_buf);
}

static void ra_scene_read_config_set_frequency(VariableItem* item) {
    RemoteAnalyzer* app = variable_item_get_context(item);
    SubGhzSetting* setting = ra_radio_get_setting(app->radio);
    const uint8_t index = variable_item_get_current_value_index(item);

    if(!app->settings.hopping)
        app->settings.frequency = subghz_setting_get_frequency(setting, index);
    ra_scene_read_config_frequency_text(app, item);
}

static void ra_scene_read_config_set_modulation(VariableItem* item) {
    RemoteAnalyzer* app = variable_item_get_context(item);
    SubGhzSetting* setting = ra_radio_get_setting(app->radio);
    const uint8_t index = variable_item_get_current_value_index(item);

    variable_item_set_current_value_text(item, subghz_setting_get_preset_name(setting, index));
    app->settings.preset_index = index;
}

static void ra_scene_read_config_set_hopping(VariableItem* item) {
    RemoteAnalyzer* app = variable_item_get_context(item);
    const uint8_t index = variable_item_get_current_value_index(item);

    variable_item_set_current_value_text(item, hopping_text[index]);
    app->settings.hopping = index > 0;
    /* Turning it off keeps the level: the sweep and the LED go on treating it as what counts
     * as a signal, and scrolling back to ON lands where it was left. */
    if(index > 0) app->settings.rssi_trigger = trigger_value[index - 1];

    ra_scene_read_config_frequency_text(
        app, variable_item_list_get(app->var_item_list, RaSettingIndexFrequency));
}

static void ra_scene_read_config_set_sound(VariableItem* item) {
    RemoteAnalyzer* app = variable_item_get_context(item);
    const uint8_t index = variable_item_get_current_value_index(item);

    variable_item_set_current_value_text(item, combobox_text[index]);
    app->settings.sound = index == 1;
}

static void ra_scene_read_config_set_sweep(VariableItem* item) {
    RemoteAnalyzer* app = variable_item_get_context(item);
    const uint8_t index = variable_item_get_current_value_index(item);

    variable_item_set_current_value_text(item, combobox_text[index]);
    app->settings.sweep = index == 1;
}

static void ra_scene_read_config_set_alert(VariableItem* item) {
    RemoteAnalyzer* app = variable_item_get_context(item);
    const uint8_t index = variable_item_get_current_value_index(item);

    variable_item_set_current_value_text(item, combobox_text[index]);
    app->settings.alert = index == 1;
}

/** Index of @p frequency in the configured list, or the list's own default. */
static uint8_t ra_scene_read_config_frequency_index(RemoteAnalyzer* app, uint32_t frequency) {
    SubGhzSetting* setting = ra_radio_get_setting(app->radio);
    for(size_t i = 0; i < subghz_setting_get_frequency_count(setting); i++) {
        if(subghz_setting_get_frequency(setting, i) == frequency) return i;
    }
    return subghz_setting_get_frequency_default_index(setting);
}

/** Nearest entry in the ladder, so a hand-edited settings file still lands on a value. */
static uint8_t ra_scene_read_config_trigger_index(float trigger) {
    uint8_t best = 0;
    float best_diff = 1000.0f;
    for(uint8_t i = 0; i < RA_TRIGGER_COUNT; i++) {
        const float diff = (trigger > trigger_value[i]) ? (trigger - trigger_value[i]) :
                                                          (trigger_value[i] - trigger);
        if(diff < best_diff) {
            best_diff = diff;
            best = i;
        }
    }
    return best;
}

static void ra_scene_read_config_enter_callback(void* context, uint32_t index) {
    furi_assert(context);
    RemoteAnalyzer* app = context;
    if(index == RaSettingIndexResetToDefault) {
        view_dispatcher_send_custom_event(app->view_dispatcher, RaCustomEventConfigReset);
    }
}

void ra_scene_read_config_on_enter(void* context) {
    RemoteAnalyzer* app = context;
    SubGhzSetting* setting = ra_radio_get_setting(app->radio);
    VariableItem* item;
    uint8_t value_index;

    item = variable_item_list_add(
        app->var_item_list,
        "Frequency",
        subghz_setting_get_frequency_count(setting),
        ra_scene_read_config_set_frequency,
        app);
    value_index = ra_scene_read_config_frequency_index(app, app->settings.frequency);
    variable_item_set_current_value_index(item, value_index);
    ra_scene_read_config_frequency_text(app, item);

    item = variable_item_list_add(
        app->var_item_list,
        "Modulation",
        subghz_setting_get_preset_count(setting),
        ra_scene_read_config_set_modulation,
        app);
    value_index = app->settings.preset_index;
    variable_item_set_current_value_index(item, value_index);
    variable_item_set_current_value_text(
        item, subghz_setting_get_preset_name(setting, value_index));

    /* Walks the hopper frequency list while nothing is on the air, so a remote on another
     * band is found without going back into this screen to change the frequency. The level
     * is what it stops on, and what the sweep and the LED treat as a signal. */
    item = variable_item_list_add(
        app->var_item_list, "Hopping", RA_HOPPING_COUNT, ra_scene_read_config_set_hopping, app);
    value_index = app->settings.hopping ?
                      ra_scene_read_config_trigger_index(app->settings.rssi_trigger) + 1 :
                      0;
    variable_item_set_current_value_index(item, value_index);
    variable_item_set_current_value_text(item, hopping_text[value_index]);

    // Mirrors the demodulator onto the speaker, so you can hear what the remote sounds like
    item = variable_item_list_add(
        app->var_item_list, "Sound", COMBO_BOX_COUNT, ra_scene_read_config_set_sound, app);
    value_index = app->settings.sound ? 1 : 0;
    variable_item_set_current_value_index(item, value_index);
    variable_item_set_current_value_text(item, combobox_text[value_index]);

    /* Walks the other modulations when a signal is on the air that nothing decodes. Off by
     * default: it takes the receiver off the chosen modulation for a second at a time. */
    item = variable_item_list_add(
        app->var_item_list, "Mod. sweep", COMBO_BOX_COUNT, ra_scene_read_config_set_sweep, app);
    value_index = app->settings.sweep ? 1 : 0;
    variable_item_set_current_value_index(item, value_index);
    variable_item_set_current_value_text(item, combobox_text[value_index]);

    /* Only a code that differs from the last one buzzes. The LED still blinks on every
     * frame, so a remote held down stays visible without shaking the thing apart. */
    item = variable_item_list_add(
        app->var_item_list, "Notify", COMBO_BOX_COUNT, ra_scene_read_config_set_alert, app);
    value_index = app->settings.alert ? 1 : 0;
    variable_item_set_current_value_index(item, value_index);
    variable_item_set_current_value_text(item, combobox_text[value_index]);

    variable_item_list_add(app->var_item_list, "Reset to default", 1, NULL, NULL);
    variable_item_list_set_enter_callback(
        app->var_item_list, ra_scene_read_config_enter_callback, app);

    view_dispatcher_switch_to_view(app->view_dispatcher, RaViewIdVarItemList);
}

bool ra_scene_read_config_on_event(void* context, SceneManagerEvent event) {
    RemoteAnalyzer* app = context;

    if(event.type == SceneManagerEventTypeCustom && event.event == RaCustomEventConfigReset) {
        // Only the items on this screen; the analyzer keeps its own trigger and feedback
        app->settings.frequency = RA_SETTINGS_DEFAULT_FREQUENCY;
        app->settings.preset_index = RA_SETTINGS_DEFAULT_PRESET;
        app->settings.hopping = false;
        app->settings.rssi_trigger = RA_SETTINGS_DEFAULT_RSSI;
        app->settings.sound = false;
        app->settings.sweep = false;
        app->settings.alert = true;
        scene_manager_previous_scene(app->scene_manager);
        return true;
    }
    return false;
}

void ra_scene_read_config_on_exit(void* context) {
    RemoteAnalyzer* app = context;
    variable_item_list_set_selected_item(app->var_item_list, 0);
    variable_item_list_reset(app->var_item_list);
    ra_settings_save(&app->settings);
}
