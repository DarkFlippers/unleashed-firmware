#include "../t5577_app.h"

// The fields of block 0 that have names, and the names the datasheet gives them. The order is the
// field's own, but the value is not the index: BIPHASE is 0x10000 and DIPHASE 0x18000, so the
// last two rows sit at indices 9 and 10 with field values 16 and 24. Hence the second table.
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

// A modulation this screen has no name for, shown rather than silently called Direct: the field
// is five bits and only eleven of its thirty-two values are named.
#define T5577_APP_CONFIG_UNNAMED "?"

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

// Where a field value sits in its table. False when the table has no name for it, which the
// caller has to carry: index 0 is not "unknown", it is Direct and RF/8.
static bool
    t5577_app_config_index(const uint32_t* values, size_t count, uint32_t field, uint8_t* index) {
    for(size_t i = 0; i < count; i++) {
        if(values[i] != field) continue;

        *index = (uint8_t)i;
        return true;
    }

    *index = 0;
    return false;
}

static ConfigItem t5577_app_config_which(const T5577App* app, const VariableItem* item) {
    for(uint8_t which = 0; which < T5577_APP_CONFIG_ITEMS; which++) {
        if(app->config_item[which] == item) return (ConfigItem)which;
    }

    furi_crash("config item is not one of this screen's");
}

// The word as it stands, on the row that opens it for typing. Shown rather than implied, because
// the pickers above cannot say everything block 0 holds.
static void t5577_app_scene_edit_config_word(T5577App* app) {
    snprintf(app->name, sizeof(app->name), "%08lX", app->dump.block[0][0]);
    variable_item_set_current_value_text(app->config_item[ConfigItemWord], app->name);
}

// One picker's value text. Entering the screen needs this as much as changing it does, and
// entering must not compose: a word the pickers cannot name would be rewritten by being looked
// at, and the raw row exists for exactly those words.
static void t5577_app_scene_edit_config_show(T5577App* app, ConfigItem which, bool named) {
    VariableItem* item = app->config_item[which];
    const uint8_t index = variable_item_get_current_value_index(item);
    char blocks[4];

    switch(which) {
    case ConfigItemModulation:
        variable_item_set_current_value_text(
            item, named ? t5577_app_modulations[index] : T5577_APP_CONFIG_UNNAMED);
        break;
    case ConfigItemBitrate:
        variable_item_set_current_value_text(item, t5577_app_bitrates[index]);
        break;
    case ConfigItemPassword:
        variable_item_set_current_value_text(item, t5577_app_switch[index]);
        break;
    case ConfigItemBlocks:
        snprintf(blocks, sizeof(blocks), "%u", index);
        variable_item_set_current_value_text(item, blocks);
        break;
    case ConfigItemWord:
        break;
    }
}

// The four named fields replaced, everything else left where it was: AOR, the PSK carrier, the
// sequence terminator, the init delay, X-mode and the master key are all things this screen
// cannot say and has no business clearing.
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

    if(block0 != app->dump.block[0][0] || !t5577_dump_was_read(&app->dump, 0, 0)) {
        app->edited = true;
    }

    t5577_dump_set(&app->dump, 0, 0, block0);
    t5577_app_scene_edit_config_word(app);
}

static void t5577_app_scene_edit_config_change(VariableItem* item) {
    T5577App* app = variable_item_get_context(item);

    // A picker the user has moved is on a value this screen named, whatever the word held before.
    t5577_app_scene_edit_config_show(app, t5577_app_config_which(app, item), true);
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
    uint8_t index = 0;

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
    // One value rather than none: a row with no values takes no text, and this one is the word.
    app->config_item[ConfigItemWord] =
        variable_item_list_add(app->config_list, "Word", 1, NULL, app);

    const bool named = t5577_app_config_index(
        t5577_app_modulation_bits,
        COUNT_OF(t5577_app_modulation_bits),
        t5577_config_modulation(block0),
        &index);
    variable_item_set_current_value_index(app->config_item[ConfigItemModulation], index);

    t5577_app_config_index(
        t5577_app_bitrate_bits,
        COUNT_OF(t5577_app_bitrate_bits),
        t5577_config_bitrate(block0),
        &index);
    variable_item_set_current_value_index(app->config_item[ConfigItemBitrate], index);

    variable_item_set_current_value_index(
        app->config_item[ConfigItemBlocks], t5577_config_max_block(block0));
    // The write-protect bit alone. t5577_config_needs_password() answers a reader's question and
    // counts answer-on-request too, which this picker would then write back as a password bit a
    // tag never had.
    variable_item_set_current_value_index(
        app->config_item[ConfigItemPassword], (block0 & LFRFID_T5577_PWD) ? 1 : 0);

    for(uint8_t which = ConfigItemModulation; which <= ConfigItemPassword; which++) {
        t5577_app_scene_edit_config_show(
            app, (ConfigItem)which, which != ConfigItemModulation || named);
    }

    t5577_app_scene_edit_config_word(app);

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
