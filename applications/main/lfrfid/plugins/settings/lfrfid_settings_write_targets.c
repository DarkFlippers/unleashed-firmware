#include "lfrfid_settings_plugin.h"

#include <flipper_application/flipper_application.h>
#include <lfrfid/lfrfid_settings.h>

// Item order matches LFRFIDWriteTarget, so the list index is the target.
static struct {
    LFRFIDSettings settings;
    uint32_t loaded_mask;
    VariableItem* items[LFRFIDWriteTargetMax];
} page;

static void lfrfid_settings_write_targets_apply(VariableItem* item, LFRFIDWriteTarget target) {
    bool enabled = variable_item_get_current_value_index(item) != 0;

    variable_item_set_current_value_text(item, enabled ? "ON" : "OFF");

    if(enabled) {
        page.settings.write_target_mask |= LFRFID_WRITE_TARGET_BIT(target);
    } else {
        page.settings.write_target_mask &= ~LFRFID_WRITE_TARGET_BIT(target);
    }
}

static void lfrfid_settings_write_targets_changed(VariableItem* item) {
    lfrfid_settings_write_targets_apply(
        item, (LFRFIDWriteTarget)(uintptr_t)variable_item_get_context(item));
}

// OK toggles the highlighted chip, so the list is usable without discovering left/right.
// Runs inside the list's own model lock, so it must not call back into the list itself -
// hence the item pointers cached at build time.
static void lfrfid_settings_write_targets_entered(void* context, uint32_t index) {
    UNUSED(context);
    if(index >= LFRFIDWriteTargetMax) return;

    VariableItem* item = page.items[index];
    variable_item_set_current_value_index(
        item, variable_item_get_current_value_index(item) ? 0 : 1);
    lfrfid_settings_write_targets_apply(item, (LFRFIDWriteTarget)index);
}

static void lfrfid_settings_write_targets_on_enter(const LfRfidSettingsPluginCtx* ctx) {
    lfrfid_settings_load(&page.settings);
    page.loaded_mask = page.settings.write_target_mask;

    for(LFRFIDWriteTarget target = 0; target < LFRFIDWriteTargetMax; target++) {
        bool enabled = (page.settings.write_target_mask & LFRFID_WRITE_TARGET_BIT(target)) != 0;

        VariableItem* item = variable_item_list_add(
            ctx->list,
            lfrfid_write_target_label(target),
            2,
            lfrfid_settings_write_targets_changed,
            (void*)(uintptr_t)target);

        variable_item_set_current_value_index(item, enabled ? 1 : 0);
        variable_item_set_current_value_text(item, enabled ? "ON" : "OFF");
        page.items[target] = item;
    }

    variable_item_list_set_enter_callback(ctx->list, lfrfid_settings_write_targets_entered, NULL);

    view_dispatcher_switch_to_view(ctx->view_dispatcher, ctx->view_id);
}

static void lfrfid_settings_write_targets_on_exit(const LfRfidSettingsPluginCtx* ctx) {
    UNUSED(ctx);

    // Written once on the way out rather than on every keypress, and only when something
    // actually changed: the whole struct is rewritten each time, and nothing reads it until
    // the next write attempt.
    if(page.settings.write_target_mask != page.loaded_mask) {
        lfrfid_settings_save(&page.settings);
    }
}

static const LfRfidSettingsPluginPage lfrfid_settings_write_targets_page = {
    .on_enter = lfrfid_settings_write_targets_on_enter,
    .on_exit = lfrfid_settings_write_targets_on_exit,
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
