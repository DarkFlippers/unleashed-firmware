#include "../sim800_app_i.h"
#include "../helpers/sim800_history.h"

static void sim800_sms_read_view_callback(Sim800SmsViewAction action, size_t index, void* context) {
    UNUSED(index);
    Sim800App* app = context;
    if(action == Sim800SmsViewActionLeft) {
        view_dispatcher_send_custom_event(app->view_dispatcher, Sim800CustomEventButtonLeft);
    } else if(action == Sim800SmsViewActionRight) {
        view_dispatcher_send_custom_event(app->view_dispatcher, Sim800CustomEventButtonRight);
    }
}

static bool sim800_sms_read_confirm_delete(Sim800App* app) {
    char detail[48];
    if(app->sms_full.part_count > 1) {
        snprintf(
            detail, sizeof(detail), "Remove all %u parts\nfrom SIM?", (unsigned)app->sms_full.part_count);
    } else if(furi_string_size(app->sms_full.date) > 0) {
        snprintf(detail, sizeof(detail), "%s", furi_string_get_cstr(app->sms_full.date));
    } else {
        snprintf(detail, sizeof(detail), "Remove from SIM?");
    }

    DialogMessage* message = dialog_message_alloc();
    dialog_message_set_header(message, "Delete SMS?", 64, 4, AlignCenter, AlignTop);
    dialog_message_set_text(message, detail, 64, 28, AlignCenter, AlignCenter);
    dialog_message_set_buttons(message, "Cancel", NULL, "Delete");

    DialogMessageButton answer = dialog_message_show(app->dialogs, message);
    dialog_message_free(message);

    return answer == DialogMessageButtonRight;
}

static void sim800_sms_read_task(Sim800App* app) {
    Sim800SmsEntry* cached = &app->inbox.items[app->selected_sms];

    // Seed with cached metadata/preview so offline/error fallback still shows something useful
    app->sms_full.index = cached->index;
    app->sms_full.part_count = (cached->part_count > 0) ? cached->part_count : 1;
    memcpy(app->sms_full.part_indices, cached->part_indices, sizeof(app->sms_full.part_indices));
    if(cached->part_count == 0) app->sms_full.part_indices[0] = cached->index;
    furi_string_set(app->sms_full.status, cached->status);
    furi_string_set(app->sms_full.from, cached->from);
    furi_string_set(app->sms_full.date, cached->date);
    furi_string_set(app->sms_full.body, cached->body);

    if(!sim800_uart_is_open(app->uart) || !sim800_uart_probe(app->uart)) {
        app->task_result = Sim800AtResultNotOpen;
        return;
    }

    sim800_uart_command(app->uart, "AT+CMGF=1", 2000, NULL);

    FuriString* resp = furi_string_alloc();
    FuriString* decoded = furi_string_alloc();
    Sim800SmsEntry part = {
        .status = furi_string_alloc(),
        .from = furi_string_alloc(),
        .date = furi_string_alloc(),
        .body = furi_string_alloc(),
    };
    char cmd[24];
    bool loaded_any = false;

    for(uint8_t p = 0; p < app->sms_full.part_count; p++) {
        uint32_t slot = app->sms_full.part_indices[p];
        snprintf(cmd, sizeof(cmd), "AT+CMGR=%lu", (unsigned long)slot);
        Sim800AtResult res = sim800_uart_command(app->uart, cmd, 5000, resp);
        if(res != Sim800AtResultOk || !sim800_sms_parse_cmgr(&part, slot, resp)) continue;

        if(app->sms_plugin) {
            app->sms_plugin->decode_text(furi_string_get_cstr(part.from), decoded);
            furi_string_set(part.from, decoded);

            app->sms_plugin->decode_text(furi_string_get_cstr(part.body), decoded);
            furi_string_set(part.body, decoded);
        }

        if(!loaded_any) {
            furi_string_set(app->sms_full.status, part.status);
            furi_string_set(app->sms_full.from, part.from);
            furi_string_set(app->sms_full.date, part.date);
            furi_string_set(app->sms_full.body, part.body);
            loaded_any = true;
        } else {
            furi_string_cat(app->sms_full.body, part.body);
        }
    }

    app->task_result = loaded_any ? Sim800AtResultOk : Sim800AtResultError;

    furi_string_free(part.status);
    furi_string_free(part.from);
    furi_string_free(part.date);
    furi_string_free(part.body);
    furi_string_free(decoded);
    furi_string_free(resp);
}

static void sim800_sms_read_show(Sim800App* app) {
    Sim800SmsEntry* sms = &app->sms_full;

    // Reading a message is the manual act that logs it; the key keeps a re-read from
    // writing the same one twice
    FuriString* key = furi_string_alloc_printf(
        "%s  %s", furi_string_get_cstr(sms->from), furi_string_get_cstr(sms->date));
    if(!sim800_history_contains(furi_string_get_cstr(key))) {
        sim800_history_add_sms(
            furi_string_get_cstr(sms->from),
            furi_string_get_cstr(sms->date),
            furi_string_get_cstr(sms->body));
    }
    furi_string_free(key);

    if(app->sms_plugin) {
        if(sms->part_count > 1) {
            FuriString* date_parts = furi_string_alloc_printf(
                "%s (%up)", furi_string_get_cstr(sms->date), (unsigned)sms->part_count);
            furi_string_set(sms->date, date_parts);
            furi_string_free(date_parts);
        }
        sim800_sms_view_set_callback(app->sms_view, sim800_sms_read_view_callback, app);
        sim800_sms_view_set_reader(app->sms_view, sms);
        view_dispatcher_switch_to_view(app->view_dispatcher, Sim800ViewSms);
        return;
    }

    furi_string_printf(
        app->text_box_store,
        "\e#%s\e#\n%s\n%s",
        furi_string_get_cstr(sms->from),
        furi_string_get_cstr(sms->date),
        furi_string_get_cstr(sms->body));

    widget_reset(app->widget);
    widget_add_text_scroll_element(
        app->widget, 0, 0, 128, 50, furi_string_get_cstr(app->text_box_store));
    widget_add_button_element(
        app->widget, GuiButtonTypeRight, "Delete", sim800_widget_button_callback, app);
    widget_add_button_element(
        app->widget, GuiButtonTypeLeft, "Reply", sim800_widget_button_callback, app);

    view_dispatcher_switch_to_view(app->view_dispatcher, Sim800ViewWidget);
}

void sim800_scene_sms_read_on_enter(void* context) {
    Sim800App* app = context;
    sim800_sms_plugin_load(app);
    sim800_show_busy(app, "SMS", "Loading...");
    sim800_task_start(app, sim800_sms_read_task);
}

bool sim800_scene_sms_read_on_event(void* context, SceneManagerEvent event) {
    Sim800App* app = context;

    if(event.type != SceneManagerEventTypeCustom) return false;

    if(event.event == Sim800CustomEventTaskDone) {
        sim800_sms_read_show(app);
        return true;
    }

    if(event.event == Sim800CustomEventButtonRight) {
        if(!sim800_sms_read_confirm_delete(app)) {
            return true;
        }

        sim800_uart_probe(app->uart);
        bool all_ok = true;
        char cmd[24];
        for(uint8_t p = 0; p < app->sms_full.part_count; p++) {
            snprintf(cmd, sizeof(cmd), "AT+CMGD=%lu", (unsigned long)app->sms_full.part_indices[p]);
            if(sim800_uart_command(app->uart, cmd, 5000, NULL) != Sim800AtResultOk) {
                all_ok = false;
            }
        }
        sim800_notify_result(app, all_ok);
        if(all_ok && app->selected_sms < app->inbox.count) {
            // Update cached inbox in place so going back doesn't need a full SIM re-read
            for(size_t i = app->selected_sms; i + 1 < app->inbox.count; i++) {
                app->inbox.items[i].index = app->inbox.items[i + 1].index;
                app->inbox.items[i].part_count = app->inbox.items[i + 1].part_count;
                app->inbox.items[i].last_part_full = app->inbox.items[i + 1].last_part_full;
                memcpy(
                    app->inbox.items[i].part_indices,
                    app->inbox.items[i + 1].part_indices,
                    sizeof(app->inbox.items[i].part_indices));
                furi_string_set(app->inbox.items[i].status, app->inbox.items[i + 1].status);
                furi_string_set(app->inbox.items[i].from, app->inbox.items[i + 1].from);
                furi_string_set(app->inbox.items[i].date, app->inbox.items[i + 1].date);
                furi_string_set(app->inbox.items[i].body, app->inbox.items[i + 1].body);
            }
            app->inbox.count--;
        } else if(!all_ok) {
            app->inbox_loaded = false;
        }
        scene_manager_previous_scene(app->scene_manager);
        return true;
    }

    if(event.event == Sim800CustomEventButtonLeft) {
        furi_string_set(app->number, app->sms_full.from);
        app->mode = Sim800ModeSms;
        app->number_saved = false;
        furi_string_reset(app->sms_text);
        scene_manager_next_scene(app->scene_manager, Sim800SceneSmsText);
        return true;
    }

    return false;
}

void sim800_scene_sms_read_on_exit(void* context) {
    Sim800App* app = context;
    sim800_task_join(app);
    widget_reset(app->widget);
    popup_reset(app->popup);
    furi_string_reset(app->sms_full.body);
}
