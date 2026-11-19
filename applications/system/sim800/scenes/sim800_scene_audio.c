#include "../sim800_app_i.h"

typedef enum {
    AudioIndexVolume,
    AudioIndexRinger,
    AudioIndexMic,
    AudioIndexChannel,
} AudioIndex;

// AT+CHFA takes 0 or 1 on SIM800; the third value is a SIM900 one and comes back as an error
static const char* const audio_channel_names[] = {"Main", "Aux"};

static uint8_t
    sim800_audio_read(Sim800App* app, const char* cmd, const char* prefix, uint8_t fallback) {
    FuriString* value = furi_string_alloc();
    FuriString* field = furi_string_alloc();
    uint8_t result = fallback;

    if(sim800_at_query(app->uart, cmd, prefix, value) &&
       sim800_csv_field(furi_string_get_cstr(value), 0, field)) {
        result = (uint8_t)strtoul(furi_string_get_cstr(field), NULL, 10);
    }

    furi_string_free(value);
    furi_string_free(field);
    return result;
}

static void sim800_audio_task(Sim800App* app) {
    if(!sim800_uart_is_open(app->uart)) return;
    app->audio_volume = sim800_audio_read(app, "AT+CLVL?", "+CLVL", app->audio_volume);
    app->audio_ringer = sim800_audio_read(app, "AT+CRSL?", "+CRSL", app->audio_ringer);
    app->audio_channel = sim800_audio_read(app, "AT+CHFA?", "+CHFA", app->audio_channel);
}

static void sim800_audio_send(Sim800App* app, const char* format, uint32_t value) {
    FuriString* cmd = furi_string_alloc();
    furi_string_printf(cmd, format, (unsigned long)value);
    sim800_uart_command(app->uart, furi_string_get_cstr(cmd), 2000, NULL);
    furi_string_free(cmd);
}

static void sim800_audio_volume_changed(VariableItem* item) {
    Sim800App* app = variable_item_get_context(item);
    char text[8];

    app->audio_volume = variable_item_get_current_value_index(item) * 10;
    snprintf(text, sizeof(text), "%u", app->audio_volume);
    variable_item_set_current_value_text(item, text);
    sim800_audio_send(app, "AT+CLVL=%lu", app->audio_volume);
}

static void sim800_audio_ringer_changed(VariableItem* item) {
    Sim800App* app = variable_item_get_context(item);
    char text[8];

    app->audio_ringer = variable_item_get_current_value_index(item) * 10;
    snprintf(text, sizeof(text), "%u", app->audio_ringer);
    variable_item_set_current_value_text(item, text);
    sim800_audio_send(app, "AT+CRSL=%lu", app->audio_ringer);
}

static void sim800_audio_mic_changed(VariableItem* item) {
    Sim800App* app = variable_item_get_context(item);
    char text[8];
    FuriString* cmd = furi_string_alloc();

    app->audio_mic = variable_item_get_current_value_index(item);
    snprintf(text, sizeof(text), "%u", app->audio_mic);
    variable_item_set_current_value_text(item, text);

    furi_string_printf(cmd, "AT+CMIC=%u,%u", app->audio_channel, app->audio_mic);
    sim800_uart_command(app->uart, furi_string_get_cstr(cmd), 2000, NULL);
    furi_string_free(cmd);
}

static void sim800_audio_channel_changed(VariableItem* item) {
    Sim800App* app = variable_item_get_context(item);

    app->audio_channel = variable_item_get_current_value_index(item);
    variable_item_set_current_value_text(item, audio_channel_names[app->audio_channel]);
    sim800_audio_send(app, "AT+CHFA=%lu", app->audio_channel);
}

static void sim800_audio_build(Sim800App* app) {
    VariableItem* item;
    char text[8];

    variable_item_list_reset(app->var_item_list);

    item = variable_item_list_add(
        app->var_item_list, "Speaker", 11, sim800_audio_volume_changed, app);
    variable_item_set_current_value_index(item, app->audio_volume / 10);
    snprintf(text, sizeof(text), "%u", app->audio_volume);
    variable_item_set_current_value_text(item, text);

    item =
        variable_item_list_add(app->var_item_list, "Ringer", 11, sim800_audio_ringer_changed, app);
    variable_item_set_current_value_index(item, app->audio_ringer / 10);
    snprintf(text, sizeof(text), "%u", app->audio_ringer);
    variable_item_set_current_value_text(item, text);

    item =
        variable_item_list_add(app->var_item_list, "Mic gain", 16, sim800_audio_mic_changed, app);
    variable_item_set_current_value_index(item, app->audio_mic);
    snprintf(text, sizeof(text), "%u", app->audio_mic);
    variable_item_set_current_value_text(item, text);

    item = variable_item_list_add(
        app->var_item_list,
        "Channel",
        COUNT_OF(audio_channel_names),
        sim800_audio_channel_changed,
        app);
    variable_item_set_current_value_index(item, app->audio_channel);
    variable_item_set_current_value_text(item, audio_channel_names[app->audio_channel]);

    view_dispatcher_switch_to_view(app->view_dispatcher, Sim800ViewVarItemList);
}

void sim800_scene_audio_on_enter(void* context) {
    Sim800App* app = context;
    sim800_show_busy(app, "Audio", "Reading...");
    sim800_task_start(app, sim800_audio_task);
}

bool sim800_scene_audio_on_event(void* context, SceneManagerEvent event) {
    Sim800App* app = context;

    if(event.type == SceneManagerEventTypeCustom && event.event == Sim800CustomEventTaskDone) {
        if(app->audio_volume > 100) app->audio_volume = 100;
        if(app->audio_ringer > 100) app->audio_ringer = 100;
        if(app->audio_channel >= COUNT_OF(audio_channel_names)) app->audio_channel = 0;
        sim800_audio_build(app);
        return true;
    }
    return false;
}

void sim800_scene_audio_on_exit(void* context) {
    Sim800App* app = context;
    sim800_task_join(app);
    variable_item_list_reset(app->var_item_list);
    popup_reset(app->popup);
}
