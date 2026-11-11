#include "../t5577_app.h"

// The modulations block 0 can ask for, with the field value beside the name: the index is not
// the value, since BIPHASE is 0x10000 and DIPHASE 0x18000 while the eight before them step by
// 0x1000. The bit rate needs no such table - its field is contiguous, so it is a shift.
static const struct {
    const char* name;
    uint32_t bits;
} t5577_app_modulations[] = {
    {"Direct", LFRFID_T5577_MODULATION_DIRECT},
    {"PSK1", LFRFID_T5577_MODULATION_PSK1},
    {"PSK2", LFRFID_T5577_MODULATION_PSK2},
    {"PSK3", LFRFID_T5577_MODULATION_PSK3},
    {"FSK1", LFRFID_T5577_MODULATION_FSK1},
    {"FSK2", LFRFID_T5577_MODULATION_FSK2},
    {"FSK1a", LFRFID_T5577_MODULATION_FSK1a},
    {"FSK2a", LFRFID_T5577_MODULATION_FSK2a},
    {"ASK/MC", LFRFID_T5577_MODULATION_MANCHESTER},
    {"Biphase", LFRFID_T5577_MODULATION_BIPHASE},
    {"Diphase", LFRFID_T5577_MODULATION_DIPHASE},
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
    ConfigItemCount,
} ConfigItem;

static_assert(
    ConfigItemCount == T5577_APP_CONFIG_ITEMS,
    "the app holds one pointer per row of this screen");

// Where a modulation sits in the table, and whether it is in there at all: index 0 is not
// "unknown", it is Direct, and writing that back over an unnamed word is how a configuration
// gets lost by being looked at.
static bool t5577_app_modulation_index(uint32_t field, uint8_t* index) {
    for(size_t i = 0; i < COUNT_OF(t5577_app_modulations); i++) {
        if(t5577_app_modulations[i].bits != field) continue;

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
    char word[9];

    snprintf(word, sizeof(word), "%08lX", app->dump.block[0][0]);
    variable_item_set_current_value_text(app->config_item[ConfigItemWord], word);
}

// One picker's value text. Entering the screen needs this as much as changing it does, and
// entering must not compose: a word the pickers cannot name would be rewritten by being looked
// at, and the raw row exists for exactly those words.
static void t5577_app_scene_edit_config_show(T5577App* app, ConfigItem which) {
    VariableItem* item = app->config_item[which];
    const uint8_t index = variable_item_get_current_value_index(item);
    char blocks[4];

    switch(which) {
    case ConfigItemModulation:
        variable_item_set_current_value_text(item, t5577_app_modulations[index].name);
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
    case ConfigItemCount:
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

    block0 |= t5577_app_modulations[variable_item_get_current_value_index(
                                        app->config_item[ConfigItemModulation])]
                  .bits;
    block0 |= (uint32_t)variable_item_get_current_value_index(app->config_item[ConfigItemBitrate])
              << LFRFID_T5577_BITRATE_SHIFT;
    block0 |= (uint32_t)variable_item_get_current_value_index(app->config_item[ConfigItemBlocks])
              << LFRFID_T5577_MAXBLOCK_SHIFT;
    if(variable_item_get_current_value_index(app->config_item[ConfigItemPassword])) {
        block0 |= LFRFID_T5577_PWD;
    }

    t5577_dump_set(&app->dump, 0, 0, block0);
    t5577_app_scene_edit_config_word(app);
}

static void t5577_app_scene_edit_config_change(VariableItem* item) {
    T5577App* app = variable_item_get_context(item);

    t5577_app_scene_edit_config_show(app, t5577_app_config_which(app, item));
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
    uint8_t modulation = 0;

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

    const bool named = t5577_app_modulation_index(t5577_config_modulation(block0), &modulation);
    variable_item_set_current_value_index(app->config_item[ConfigItemModulation], modulation);
    variable_item_set_current_value_index(
        app->config_item[ConfigItemBitrate],
        t5577_config_bitrate(block0) >> LFRFID_T5577_BITRATE_SHIFT);
    variable_item_set_current_value_index(
        app->config_item[ConfigItemBlocks], t5577_config_max_block(block0));
    // The write-protect bit alone. t5577_config_needs_password() answers a reader's question and
    // counts answer-on-request too, which this picker would then write back as a password bit a
    // tag never had.
    variable_item_set_current_value_index(
        app->config_item[ConfigItemPassword], (block0 & LFRFID_T5577_PWD) ? 1 : 0);

    for(uint8_t which = ConfigItemModulation; which <= ConfigItemPassword; which++) {
        t5577_app_scene_edit_config_show(app, (ConfigItem)which);
    }

    if(!named) {
        variable_item_set_current_value_text(
            app->config_item[ConfigItemModulation], T5577_APP_CONFIG_UNNAMED);
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
