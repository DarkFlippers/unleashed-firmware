#include "lfrfid_settings_plugin.h"

#include <flipper_application/flipper_application.h>
#include <lfrfid/lfrfid_settings.h>

static struct {
    LFRFIDWriteTargetMask mask;
    LFRFIDWriteTargetMask saved_mask;
} page;

static void lfrfid_settings_write_targets_changed(VariableItem* item) {
    LFRFIDWriteTarget target = (LFRFIDWriteTarget)(uintptr_t)variable_item_get_context(item);
    bool enabled = variable_item_get_current_value_index(item) != 0;

    variable_item_set_current_value_text(item, enabled ? "ON" : "OFF");

    if(enabled) {
        page.mask |= LFRFID_WRITE_TARGET_BIT(target);
    } else {
        page.mask &= ~LFRFID_WRITE_TARGET_BIT(target);
    }
}

// OK toggles the highlighted chip, so the list is usable without discovering left/right.
static void lfrfid_settings_write_targets_entered(void* context, uint32_t index) {
    VariableItem* item = variable_item_list_get(context, index);
    furi_check(item); // the list holds one row per target and nothing else

    variable_item_set_current_value_index(
        item, variable_item_get_current_value_index(item) ? 0 : 1);
    lfrfid_settings_write_targets_changed(item);
}

static void lfrfid_settings_write_targets_on_enter(VariableItemList* list) {
    page.mask = lfrfid_settings_get_write_targets();
    page.saved_mask = page.mask;

    for(LFRFIDWriteTarget target = 0; target < LFRFIDWriteTargetMax; target++) {
        bool enabled = (page.mask & LFRFID_WRITE_TARGET_BIT(target)) != 0;

        VariableItem* item = variable_item_list_add(
            list,
            lfrfid_write_target_name(target),
            2,
            lfrfid_settings_write_targets_changed,
            (void*)(uintptr_t)target);

        variable_item_set_current_value_index(item, enabled ? 1 : 0);
        variable_item_set_current_value_text(item, enabled ? "ON" : "OFF");
    }

    variable_item_list_set_enter_callback(list, lfrfid_settings_write_targets_entered, list);
}

static bool lfrfid_settings_write_targets_on_save(void) {
    // Once on the way out rather than on every keypress, and only when something changed: the
    // whole file is rewritten each time, and nothing reads it until the next write attempt.
    if(page.mask == page.saved_mask) return true;

    return lfrfid_settings_set_write_targets(page.mask);
}

static const LfRfidSettingsPluginPage lfrfid_settings_write_targets_page = {
    .on_enter = lfrfid_settings_write_targets_on_enter,
    .on_save = lfrfid_settings_write_targets_on_save,
};

static const LfRfidSettingsPlugin lfrfid_settings_plugin = {
    .write_targets = &lfrfid_settings_write_targets_page,
};

static const FlipperAppPluginDescriptor lfrfid_settings_plugin_descriptor = {
    .appid = LFRFID_SETTINGS_PLUGIN_APP_ID,
    .ep_api_version = LFRFID_SETTINGS_PLUGIN_API_VERSION,
    .entry_point = &lfrfid_settings_plugin,
};

const FlipperAppPluginDescriptor* lfrfid_settings_plugin_ep(void) {
    return &lfrfid_settings_plugin_descriptor;
}
