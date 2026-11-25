#include "../sim800_app_i.h"

#define SIM800_INBOX_REFRESH_ITEM 0xFE
#define SIM800_INBOX_PREVIEW_LEN  32

static void sim800_inbox_view_callback(Sim800SmsViewAction action, size_t index, void* context) {
    Sim800App* app = context;
    if(action == Sim800SmsViewActionSelect) {
        view_dispatcher_send_custom_event(app->view_dispatcher, index);
    } else if(action == Sim800SmsViewActionRight) {
        view_dispatcher_send_custom_event(app->view_dispatcher, Sim800CustomEventButtonRight);
    }
}

static void sim800_inbox_decode_entry(Sim800App* app, Sim800SmsEntry* entry, FuriString* scratch) {
    if(app->sms_plugin) {
        app->sms_plugin->decode_text(furi_string_get_cstr(entry->from), scratch);
        furi_string_set(entry->from, scratch);

        app->sms_plugin->decode_text(furi_string_get_cstr(entry->body), scratch);
        furi_string_set(entry->body, scratch);
    }
    entry->last_part_full = sim800_sms_part_looks_full(furi_string_get_cstr(entry->body));
    // Keep only a short single-line preview in RAM; opening the SMS reads it in full via AT+CMGR
    sim800_sms_trim_preview(entry->body, SIM800_INBOX_PREVIEW_LEN);
}

/** Merge a continuation segment into a matching recent multipart SMS when possible. */
static bool sim800_inbox_try_merge(Sim800Inbox* inbox, const Sim800SmsEntry* candidate) {
    size_t back = (inbox->count > 4) ? (inbox->count - 4) : 0;
    const char* cand_from = furi_string_get_cstr(candidate->from);
    const char* cand_date = furi_string_get_cstr(candidate->date);

    for(size_t i = inbox->count; i > back; i--) {
        Sim800SmsEntry* prev = &inbox->items[i - 1];
        if(!prev->last_part_full) continue;
        if(prev->part_count >= SIM800_SMS_MAX_PARTS) continue;
        if(strcmp(furi_string_get_cstr(prev->from), cand_from) != 0) continue;
        if(!sim800_sms_timestamps_close(furi_string_get_cstr(prev->date), cand_date)) continue;

        prev->part_indices[prev->part_count++] = candidate->index;
        prev->last_part_full = candidate->last_part_full;
        return true;
    }
    return false;
}

static bool sim800_inbox_query_cpms(Sim800App* app, uint32_t* used_out, uint32_t* total_out) {
    FuriString* resp = furi_string_alloc();
    FuriString* value = furi_string_alloc();
    FuriString* field = furi_string_alloc();
    bool ok = false;

    Sim800AtResult res = sim800_uart_command(app->uart, "AT+CPMS?", 2000, resp);
    if(res != Sim800AtResultOk) {
        res = sim800_uart_command(app->uart, "AT+CPMS=\"SM\",\"SM\",\"SM\"", 2000, resp);
        if(res == Sim800AtResultOk) {
            res = sim800_uart_command(app->uart, "AT+CPMS?", 2000, resp);
        }
    }

    // +CPMS: "SM",<used1>,<total1>,"SM",<used2>,<total2>,...
    if(res == Sim800AtResultOk && sim800_response_field(resp, "+CPMS", value)) {
        const char* csv = furi_string_get_cstr(value);
        if(sim800_csv_field(csv, 1, field)) {
            *used_out = (uint32_t)strtoul(furi_string_get_cstr(field), NULL, 10);
            if(sim800_csv_field(csv, 2, field)) {
                *total_out = (uint32_t)strtoul(furi_string_get_cstr(field), NULL, 10);
                ok = true;
            }
        }
    }

    furi_string_free(resp);
    furi_string_free(value);
    furi_string_free(field);
    return ok;
}

static void sim800_inbox_task(Sim800App* app) {
    sim800_inbox_reset(&app->inbox);
    furi_string_reset(app->task_response);

    if(!sim800_uart_is_open(app->uart) || !sim800_uart_probe(app->uart)) {
        app->task_result = Sim800AtResultNotOpen;
        return;
    }

    app->task_result = sim800_uart_command(app->uart, "AT+CMGF=1", 2000, app->task_response);
    if(app->task_result != Sim800AtResultOk) return;

    if(sim800_uart_command(app->uart, "AT+CSCS=\"GSM\"", 2000, NULL) == Sim800AtResultError) {
        sim800_uart_command(app->uart, "AT+CSCS=\"IRA\"", 2000, NULL);
    }

    uint32_t used = 0;
    uint32_t total = 0;
    bool have_cpms = sim800_inbox_query_cpms(app, &used, &total);

    if(have_cpms && used == 0) {
        app->task_result = Sim800AtResultOk;
        app->inbox_loaded = true;
        return;
    }

    if(have_cpms && total > 0) {
        FuriString* resp = furi_string_alloc();
        FuriString* scratch = furi_string_alloc();
        Sim800SmsEntry tmp = {
            .status = furi_string_alloc(),
            .from = furi_string_alloc(),
            .date = furi_string_alloc(),
            .body = furi_string_alloc(),
        };
        char cmd[24];
        bool use_mode1 = true;
        uint32_t found_slots = 0;

        if(total > 200) total = 200;

        for(uint32_t idx = 1; idx <= total && found_slots < used; idx++) {
            if(app->inbox_cancel) break;

            if(use_mode1) {
                snprintf(cmd, sizeof(cmd), "AT+CMGR=%lu,1", (unsigned long)idx);
            } else {
                snprintf(cmd, sizeof(cmd), "AT+CMGR=%lu", (unsigned long)idx);
            }

            Sim800AtResult res = sim800_uart_command(app->uart, cmd, 3000, resp);
            if(res == Sim800AtResultError && use_mode1 && furi_string_size(resp) == 0) {
                // Plain ERROR without +CMS ERROR means ,1 mode is unsupported: retry without it
                use_mode1 = false;
                snprintf(cmd, sizeof(cmd), "AT+CMGR=%lu", (unsigned long)idx);
                res = sim800_uart_command(app->uart, cmd, 3000, resp);
            }

            if(res == Sim800AtResultTimeout) {
                app->task_result = Sim800AtResultTimeout;
                break;
            }

            if(res == Sim800AtResultOk && sim800_sms_parse_cmgr(&tmp, idx, resp)) {
                found_slots++;
                sim800_inbox_decode_entry(app, &tmp, scratch);

                if(!sim800_inbox_try_merge(&app->inbox, &tmp) &&
                   app->inbox.count < SIM800_INBOX_MAX) {
                    Sim800SmsEntry* dst = &app->inbox.items[app->inbox.count++];
                    dst->index = tmp.index;
                    dst->part_indices[0] = tmp.index;
                    dst->part_count = 1;
                    dst->last_part_full = tmp.last_part_full;
                    furi_string_set(dst->status, tmp.status);
                    furi_string_set(dst->from, tmp.from);
                    furi_string_set(dst->date, tmp.date);
                    furi_string_set(dst->body, tmp.body);
                }

                snprintf(
                    app->popup_text,
                    sizeof(app->popup_text),
                    "Reading %lu/%lu...",
                    (unsigned long)found_slots,
                    (unsigned long)used);
                popup_set_text(app->popup, app->popup_text, 64, 38, AlignCenter, AlignTop);
            }
        }

        furi_string_free(tmp.status);
        furi_string_free(tmp.from);
        furi_string_free(tmp.date);
        furi_string_free(tmp.body);
        furi_string_free(resp);
        furi_string_free(scratch);

        if(!app->inbox_cancel && app->task_result == Sim800AtResultOk) {
            app->inbox_loaded = true;
        }
        return;
    }

    // Fallback if AT+CPMS? didn't report slot counts: use AT+CMGL="ALL" and trim previews
    app->task_result =
        sim800_uart_command_long(app->uart, "AT+CMGL=\"ALL\"", 5000, 60000, app->task_response);
    if(app->task_result == Sim800AtResultOk) {
        sim800_inbox_parse(&app->inbox, app->task_response);
        FuriString* scratch = furi_string_alloc();
        for(size_t i = 0; i < app->inbox.count; i++) {
            sim800_inbox_decode_entry(app, &app->inbox.items[i], scratch);
        }
        furi_string_free(scratch);
        app->inbox_loaded = true;
    }
}

static void sim800_inbox_error_text(Sim800App* app, FuriString* out) {
    if(app->task_result == Sim800AtResultOk || app->inbox_loaded) {
        furi_string_set_str(out, "Inbox is empty");
        return;
    }

    furi_string_set(out, app->task_response);
    furi_string_trim(out);
    size_t nl = furi_string_search_char(out, '\n');
    if(nl != FURI_STRING_FAILURE) furi_string_left(out, nl);
    furi_string_trim(out);
    if(furi_string_size(out) == 0) {
        furi_string_set_str(out, sim800_at_result_text(app->task_result));
    }
}

static void sim800_inbox_show(Sim800App* app) {
    FuriString* status = furi_string_alloc();
    sim800_inbox_error_text(app, status);

    if(app->selected_sms >= app->inbox.count) {
        app->selected_sms = (app->inbox.count > 0) ? (app->inbox.count - 1) : 0;
    }

    if(app->sms_plugin) {
        sim800_sms_view_set_callback(app->sms_view, sim800_inbox_view_callback, app);
        sim800_sms_view_set_inbox(
            app->sms_view, &app->inbox, app->selected_sms, furi_string_get_cstr(status));
        view_dispatcher_switch_to_view(app->view_dispatcher, Sim800ViewSms);
        furi_string_free(status);
        return;
    }

    // Fallback when the optional .fal plugin isn't present
    submenu_reset(app->submenu);
    submenu_set_header(app->submenu, "SMS inbox");
    submenu_add_item(
        app->submenu, "Refresh", SIM800_INBOX_REFRESH_ITEM, sim800_submenu_callback, app);

    if(app->inbox.count == 0) {
        submenu_add_item(
            app->submenu, furi_string_get_cstr(status), 0xFF, sim800_submenu_callback, app);
    } else {
        FuriString* label = furi_string_alloc();
        for(size_t i = 0; i < app->inbox.count; i++) {
            furi_string_printf(
                label,
                "%s: %s",
                furi_string_get_cstr(app->inbox.items[i].from),
                furi_string_get_cstr(app->inbox.items[i].body));
            submenu_add_item(
                app->submenu, furi_string_get_cstr(label), i, sim800_submenu_callback, app);
        }
        furi_string_free(label);
    }

    furi_string_free(status);
    view_dispatcher_switch_to_view(app->view_dispatcher, Sim800ViewSubmenu);
}

static void sim800_inbox_reload(Sim800App* app) {
    app->inbox_loaded = false;
    app->inbox_cancel = false;
    snprintf(app->popup_text, sizeof(app->popup_text), "Reading...");
    sim800_show_busy(app, "SMS inbox", app->popup_text);
    sim800_task_start(app, sim800_inbox_task);
}

void sim800_scene_inbox_on_enter(void* context) {
    Sim800App* app = context;
    app->sms_plugin_keep = false;
    sim800_sms_plugin_load(app);

    if(app->inbox_loaded) {
        sim800_inbox_show(app);
    } else {
        sim800_inbox_reload(app);
    }
}

bool sim800_scene_inbox_on_event(void* context, SceneManagerEvent event) {
    Sim800App* app = context;

    if(event.type != SceneManagerEventTypeCustom) return false;

    if(event.event == Sim800CustomEventTaskDone) {
        if(!app->inbox_cancel && app->task_result != Sim800AtResultOk) {
            sim800_notify_result(app, false);
        }
        sim800_inbox_show(app);
        return true;
    }

    if(event.event == Sim800CustomEventButtonRight || event.event == SIM800_INBOX_REFRESH_ITEM ||
       event.event == Sim800CustomEventNewSms) {
        if(!sim800_task_is_busy(app)) {
            sim800_inbox_reload(app);
        }
        return true;
    }

    if(event.event < app->inbox.count) {
        app->selected_sms = event.event;
        app->sms_plugin_keep = true;
        scene_manager_next_scene(app->scene_manager, Sim800SceneSmsRead);
        return true;
    }

    return false;
}

void sim800_scene_inbox_on_exit(void* context) {
    Sim800App* app = context;
    app->inbox_cancel = true;
    sim800_task_join(app);
    submenu_reset(app->submenu);
    popup_reset(app->popup);
    if(!app->sms_plugin_keep) {
        sim800_sms_plugin_unload(app);
    }
}
