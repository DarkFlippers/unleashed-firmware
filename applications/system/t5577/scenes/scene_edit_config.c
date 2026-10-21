#include "../t5577_app.h"

// The fields of block 0 that have names, and the names the datasheet gives them. Order is the
// field's own encoding, so the index into either table is the value that goes into the word.
static const char* const t5577_app_modulations[] = {
    "Direct",
    "PSK1",
    "PSK2",
    "PSK3",
    "FSK1",
    "FSK2",
    "FSK1a",
    "FSK2a",
    "ASK/MC",
    "Biphase",
    "Diphase",
};

static const uint32_t t5577_app_modulation_bits[] = {
    LFRFID_T5577_MODULATION_DIRECT,
    LFRFID_T5577_MODULATION_PSK1,
    LFRFID_T5577_MODULATION_PSK2,
    LFRFID_T5577_MODULATION_PSK3,
    LFRFID_T5577_MODULATION_FSK1,
    LFRFID_T5577_MODULATION_FSK2,
    LFRFID_T5577_MODULATION_FSK1a,
    LFRFID_T5577_MODULATION_FSK2a,
    LFRFID_T5577_MODULATION_MANCHESTER,
    LFRFID_T5577_MODULATION_BIPHASE,
    LFRFID_T5577_MODULATION_DIPHASE,
};

static const char* const t5577_app_bitrates[] = {
    "RF/8",
    "RF/16",
    "RF/32",
    "RF/40",
    "RF/50",
    "RF/64",
    "RF/100",
    "RF/128",
};

static const uint32_t t5577_app_bitrate_bits[] = {
    LFRFID_T5577_BITRATE_RF_8,
    LFRFID_T5577_BITRATE_RF_16,
    LFRFID_T5577_BITRATE_RF_32,
    LFRFID_T5577_BITRATE_RF_40,
    LFRFID_T5577_BITRATE_RF_50,
    LFRFID_T5577_BITRATE_RF_64,
    LFRFID_T5577_BITRATE_RF_100,
    LFRFID_T5577_BITRATE_RF_128,
};

static const char* const t5577_app_switch[] = {"Off", "On"};

typedef enum {
    ConfigItemModulation,
    ConfigItemBitrate,
    ConfigItemBlocks,
    ConfigItemPassword,
    ConfigItemWord,
} ConfigItem;

static_assert(
    COUNT_OF(t5577_app_modulations) == COUNT_OF(t5577_app_modulation_bits),
    "every modulation needs its field value");
static_assert(
    COUNT_OF(t5577_app_bitrates) == COUNT_OF(t5577_app_bitrate_bits),
    "every bit rate needs its field value");

// Where a field value sits in its table, or 0 for a word that holds something the table cannot
// name - which only Direct and RF/8 can be, since both are zero.
static uint8_t t5577_app_config_index(const uint32_t* values, size_t count, uint32_t field) {
    for(size_t i = 0; i < count; i++) {
        if(values[i] == field) return (uint8_t)i;
    }

    return 0;
}

// The four named fields replaced, everything else left where it was: AOR, the PSK carrier, the
// sequence terminator, the init delay and the master key are all things this screen cannot say
// and has no business clearing.
static void t5577_app_scene_edit_config_compose(T5577App* app) {
    uint32_t block0 = app->dump.block[0][0];

    block0 &= ~(
        LFRFID_T5577_MODULATION_MASK | LFRFID_T5577_BITRATE_MASK |
        ((uint32_t)LFRFID_T5577_MAXBLOCK_MASK << LFRFID_T5577_MAXBLOCK_SHIFT) | LFRFID_T5577_PWD);

    block0 |= t5577_app_modulation_bits[variable_item_get_current_value_index(
        app->config_item[ConfigItemModulation])];
    block0 |= t5577_app_bitrate_bits[variable_item_get_current_value_index(
        app->config_item[ConfigItemBitrate])];
    block0 |= (uint32_t)variable_item_get_current_value_index(app->config_item[ConfigItemBlocks])
              << LFRFID_T5577_MAXBLOCK_SHIFT;
    if(variable_item_get_current_value_index(app->config_item[ConfigItemPassword])) {
        block0 |= LFRFID_T5577_PWD;
    }

    // Only a word that differs is a change. This runs once per item when the screen opens, so
    // without the test, looking at the configuration would be enough to be asked to save it.
    if(block0 != app->dump.block[0][0] || !t5577_dump_was_read(&app->dump, 0, 0)) {
        app->edited = true;
    }

    t5577_dump_set(&app->dump, 0, 0, block0);

    // The word the pickers add up to, on the item that opens it for typing: this screen cannot
    // say everything block 0 can hold, so what it does say is shown rather than implied.
    snprintf(app->name, sizeof(app->name), "%08lX", block0);
    variable_item_set_current_value_text(app->config_item[ConfigItemWord], app->name);
}

static void t5577_app_scene_edit_config_change(VariableItem* item) {
    T5577App* app = variable_item_get_context(item);

    const uint8_t index = variable_item_get_current_value_index(item);

    if(item == app->config_item[ConfigItemModulation]) {
        variable_item_set_current_value_text(item, t5577_app_modulations[index]);
    } else if(item == app->config_item[ConfigItemBitrate]) {
        variable_item_set_current_value_text(item, t5577_app_bitrates[index]);
    } else if(item == app->config_item[ConfigItemPassword]) {
        variable_item_set_current_value_text(item, t5577_app_switch[index]);
    } else {
        char blocks[4];
        snprintf(blocks, sizeof(blocks), "%u", index);
        variable_item_set_current_value_text(item, blocks);
    }

    t5577_app_scene_edit_config_compose(app);
}

static void t5577_app_scene_edit_config_enter(void* context, uint32_t index) {
    T5577App* app = context;

    if(index != ConfigItemWord) return;

    scene_manager_next_scene(app->scene_manager, T5577AppSceneBlock);
}

void t5577_app_scene_edit_config_on_enter(void* context) {
    T5577App* app = context;

    const uint32_t block0 = app->dump.block[0][0];

    variable_item_list_reset(app->config_list);

    app->config_item[ConfigItemModulation] = variable_item_list_add(
        app->config_list,
        "Modulation",
        COUNT_OF(t5577_app_modulations),
        t5577_app_scene_edit_config_change,
        app);
    app->config_item[ConfigItemBitrate] = variable_item_list_add(
        app->config_list,
        "Bit rate",
        COUNT_OF(t5577_app_bitrates),
        t5577_app_scene_edit_config_change,
        app);
    app->config_item[ConfigItemBlocks] = variable_item_list_add(
        app->config_list,
        "Data blocks",
        LFRFID_T5577_BLOCKS_IN_PAGE_0,
        t5577_app_scene_edit_config_change,
        app);
    app->config_item[ConfigItemPassword] = variable_item_list_add(
        app->config_list,
        "Password bit",
        COUNT_OF(t5577_app_switch),
        t5577_app_scene_edit_config_change,
        app);
    // One value rather than none: the list draws a right arrow on any item whose index is below
    // values_count - 1, and that subtraction underflows at zero.
    app->config_item[ConfigItemWord] =
        variable_item_list_add(app->config_list, "Word", 1, NULL, app);

    variable_item_set_current_value_index(
        app->config_item[ConfigItemModulation],
        t5577_app_config_index(
            t5577_app_modulation_bits,
            COUNT_OF(t5577_app_modulation_bits),
            t5577_config_modulation(block0)));
    variable_item_set_current_value_index(
        app->config_item[ConfigItemBitrate],
        t5577_app_config_index(
            t5577_app_bitrate_bits,
            COUNT_OF(t5577_app_bitrate_bits),
            t5577_config_bitrate(block0)));
    variable_item_set_current_value_index(
        app->config_item[ConfigItemBlocks], t5577_config_max_block(block0));
    variable_item_set_current_value_index(
        app->config_item[ConfigItemPassword], t5577_config_needs_password(block0) ? 1 : 0);

    // Through the change callback, so the four value texts and the word are drawn by the one
    // place that knows how to write them - and so that entering this screen cannot leave the
    // word disagreeing with the pickers above it.
    for(uint8_t item = ConfigItemModulation; item <= ConfigItemPassword; item++) {
        t5577_app_scene_edit_config_change(app->config_item[item]);
    }

    variable_item_list_set_enter_callback(
        app->config_list, t5577_app_scene_edit_config_enter, app);
    variable_item_list_set_selected_item(
        app->config_list,
        scene_manager_get_scene_state(app->scene_manager, T5577AppSceneEditConfig));

    view_dispatcher_switch_to_view(app->view_dispatcher, T5577AppViewConfig);
}

bool t5577_app_scene_edit_config_on_event(void* context, SceneManagerEvent event) {
    UNUSED(context);
    UNUSED(event);

    return false;
}

void t5577_app_scene_edit_config_on_exit(void* context) {
    T5577App* app = context;

    scene_manager_set_scene_state(
        app->scene_manager,
        T5577AppSceneEditConfig,
        variable_item_list_get_selected_item_index(app->config_list));
    variable_item_list_reset(app->config_list);
}
