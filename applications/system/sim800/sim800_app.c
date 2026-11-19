#include "sim800_app_i.h"

#include <expansion/expansion.h>
#include <furi_hal_power.h>
#include <furi_hal_resources.h>

static bool sim800_app_custom_event_callback(void* context, uint32_t event) {
    furi_assert(context);
    Sim800App* app = context;

    // Logging happens here rather than in the UART callback: this is the app thread, so a
    // slow card write cannot stall the reader and drop bytes
    if(event == Sim800CustomEventIncomingCall) {
        uint32_t now = furi_get_tick();
        bool same = (strcmp(app->history_last, app->incoming_number) == 0) &&
                    ((now - app->history_tick) < furi_ms_to_ticks(60000));
        if(!same) {
            strlcpy(app->history_last, app->incoming_number, sizeof(app->history_last));
            app->history_tick = now;
            sim800_history_add(
                Sim800HistoryCall,
                (app->incoming_number[0] != '\0') ? app->incoming_number : "unknown");
        }
    } else if(event == Sim800CustomEventNewSms) {
        char detail[32];
        snprintf(
            detail,
            sizeof(detail),
            "index %s",
            (app->incoming_sms[0] != '\0') ? app->incoming_sms : "?");
        sim800_history_add(Sim800HistorySms, detail);
    }

    // A ring can land in any scene, so the call screen is pushed here rather than in each one
    if(event == Sim800CustomEventIncomingCall &&
       scene_manager_get_current_scene(app->scene_manager) != Sim800SceneCall) {
        app->call_incoming = true;
        // Nothing says this caller is in the phonebook, so let the screen offer to save it
        app->number_saved = false;
        furi_string_set_str(app->number, app->incoming_number);
        scene_manager_set_scene_state(app->scene_manager, Sim800SceneCall, 0);
        scene_manager_next_scene(app->scene_manager, Sim800SceneCall);
        return true;
    }

    // Worth one error beep, but the module repeats the warning for as long as the rail sags
    if(event == Sim800CustomEventPowerAlarm) {
        uint32_t now = furi_get_tick();
        if((app->power_alarm_tick == 0) ||
           ((now - app->power_alarm_tick) > furi_ms_to_ticks(10000))) {
            app->power_alarm_tick = now;
            sim800_notify_result(app, false);
        }
        return true;
    }

    return scene_manager_handle_custom_event(app->scene_manager, event);
}

static bool sim800_app_back_event_callback(void* context) {
    furi_assert(context);
    Sim800App* app = context;
    return scene_manager_handle_back_event(app->scene_manager);
}

static void sim800_app_tick_event_callback(void* context) {
    furi_assert(context);
    Sim800App* app = context;
    scene_manager_handle_tick_event(app->scene_manager);
}

static void sim800_app_log_line(Sim800App* app, const char* line) {
    furi_mutex_acquire(app->log_mutex, FuriWaitForever);

    furi_string_cat_str(app->log, line);
    furi_string_cat_str(app->log, "\n");
    if(furi_string_size(app->log) > SIM800_LOG_MAX) {
        size_t drop = furi_string_size(app->log) - SIM800_LOG_MAX;
        size_t newline = furi_string_search_char(app->log, '\n', drop);
        furi_string_right(app->log, (newline == FURI_STRING_FAILURE) ? drop : newline + 1);
    }

    furi_mutex_release(app->log_mutex);
}

static void sim800_app_line_callback(const char* line, void* context) {
    Sim800App* app = context;

    sim800_app_log_line(app, line);

    if(strncmp(line, "+CLIP:", 6) == 0) {
        FuriString* field = furi_string_alloc();
        const char* payload = line + 6;
        while(*payload == ' ')
            payload++;
        if(sim800_csv_field(payload, 0, field)) {
            strlcpy(
                app->incoming_number, furi_string_get_cstr(field), sizeof(app->incoming_number));
        }
        furi_string_free(field);
    } else if(strcmp(line, "RING") == 0) {
        view_dispatcher_send_custom_event(app->view_dispatcher, Sim800CustomEventIncomingCall);
    } else if(
        strcmp(line, "NO CARRIER") == 0 || strcmp(line, "BUSY") == 0 ||
        strcmp(line, "NO ANSWER") == 0 || strcmp(line, "NO DIALTONE") == 0) {
        view_dispatcher_send_custom_event(app->view_dispatcher, Sim800CustomEventCallEnded);
    } else if(strstr(line, "VOLTAGE") != NULL) {
        strlcpy(app->power_alarm, line, sizeof(app->power_alarm));
        view_dispatcher_send_custom_event(app->view_dispatcher, Sim800CustomEventPowerAlarm);
    } else if(strncmp(line, "+CMTI:", 6) == 0) {
        FuriString* field = furi_string_alloc();
        const char* payload = line + 6;
        while(*payload == ' ')
            payload++;
        if(sim800_csv_field(payload, 1, field)) {
            strlcpy(app->incoming_sms, furi_string_get_cstr(field), sizeof(app->incoming_sms));
        }
        furi_string_free(field);
        view_dispatcher_send_custom_event(app->view_dispatcher, Sim800CustomEventNewSms);
    } else if(strncmp(line, "+CUSD:", 6) == 0) {
        furi_string_set_str(app->urc_line, line);
        furi_semaphore_release(app->urc_sem);
    }
}

void sim800_log_read(Sim800App* app, FuriString* out) {
    furi_mutex_acquire(app->log_mutex, FuriWaitForever);
    furi_string_set(out, app->log);
    furi_mutex_release(app->log_mutex);
}

void sim800_log_clear(Sim800App* app) {
    furi_mutex_acquire(app->log_mutex, FuriWaitForever);
    furi_string_reset(app->log);
    furi_mutex_release(app->log_mutex);
}

void sim800_arm_urc(Sim800App* app) {
    while(furi_semaphore_acquire(app->urc_sem, 0) == FuriStatusOk) {
    }
    furi_string_reset(app->urc_line);
}

bool sim800_wait_urc(Sim800App* app, uint32_t timeout_ms, FuriString* out) {
    if(furi_semaphore_acquire(app->urc_sem, timeout_ms) != FuriStatusOk) return false;
    furi_string_set(out, app->urc_line);
    return true;
}

bool sim800_app_connect(Sim800App* app, size_t attempts) {
    sim800_uart_close(app->uart);

    FuriHalSerialId serial_id = (app->settings.uart_port == Sim800UartLpuart) ?
                                    FuriHalSerialIdLpuart :
                                    FuriHalSerialIdUsart;
    if(!sim800_uart_open(app->uart, serial_id, app->settings.baudrate)) return false;

    if(!sim800_uart_sync(app->uart, attempts)) return false;

    sim800_uart_command(app->uart, app->settings.echo ? "ATE1" : "ATE0", 300, NULL);
    sim800_uart_command(app->uart, "AT+CMEE=2", 300, NULL);
    sim800_uart_command(app->uart, "AT+CLIP=1", 300, NULL);
    sim800_uart_command(app->uart, "AT+CMGF=1", 300, NULL);
    sim800_uart_command(app->uart, "AT+CSCS=\"GSM\"", 300, NULL);
    sim800_uart_command(app->uart, "AT+CNMI=2,1,0,0,0", 300, NULL);

    return true;
}

static int32_t sim800_task_thread(void* context) {
    Sim800App* app = context;

    if(app->task_fn) app->task_fn(app);

    app->task_busy = false;
    view_dispatcher_send_custom_event(app->view_dispatcher, Sim800CustomEventTaskDone);
    return 0;
}

void sim800_task_start(Sim800App* app, Sim800TaskFn fn) {
    sim800_task_join(app);

    app->task_fn = fn;
    app->task_busy = true;
    app->task_thread = furi_thread_alloc_ex("Sim800Task", 2048, sim800_task_thread, app);
    furi_thread_start(app->task_thread);
}

void sim800_task_join(Sim800App* app) {
    if(app->task_thread == NULL) return;
    furi_thread_join(app->task_thread);
    furi_thread_free(app->task_thread);
    app->task_thread = NULL;
    app->task_busy = false;
}

bool sim800_task_is_busy(Sim800App* app) {
    return app->task_busy;
}

void sim800_show_busy(Sim800App* app, const char* header, const char* text) {
    popup_reset(app->popup);
    popup_set_header(app->popup, header, 64, 20, AlignCenter, AlignTop);
    popup_set_text(app->popup, text, 64, 38, AlignCenter, AlignTop);
    view_dispatcher_switch_to_view(app->view_dispatcher, Sim800ViewPopup);
}

static const GpioPin* const sim800_pwrkey_pins[SIM800_PWRKEY_COUNT] = {
    NULL,
    &gpio_ext_pa7,
    &gpio_ext_pa6,
    &gpio_ext_pa4,
    &gpio_ext_pb3,
    &gpio_ext_pb2,
    &gpio_usart_tx,
    &gpio_usart_rx,
    &gpio_ext_pc1,
    &gpio_ext_pc0,
};

// Same order, as the numbers printed on the case
static const uint8_t sim800_pwrkey_numbers[SIM800_PWRKEY_COUNT] =
    {0, 2, 3, 4, 5, 6, 13, 14, 15, 16};

Sim800PwrkeyState sim800_pwrkey_state(Sim800App* app) {
    uint32_t index = app->settings.pwrkey_pin;
    if(index == 0 || index >= SIM800_PWRKEY_COUNT) return Sim800PwrkeyNoPin;

    // Driving a pin the UART owns would cut the link we talk to the module over
    uint8_t number = sim800_pwrkey_numbers[index];
    bool lpuart = (app->settings.uart_port == Sim800UartLpuart);
    bool taken = lpuart ? (number == 15 || number == 16) : (number == 13 || number == 14);

    return taken ? Sim800PwrkeyUartPin : Sim800PwrkeyReady;
}

bool sim800_pwrkey_pulse(Sim800App* app) {
    if(sim800_pwrkey_state(app) != Sim800PwrkeyReady) return false;
    const GpioPin* pin = sim800_pwrkey_pins[app->settings.pwrkey_pin];

    // Open drain, never push-pull: PWRKEY idles at VBAT through the module's own pull-up, so a
    // pin driven high would be fighting a ~4 V rail from 3.3 V. Low asserts, high is hi-Z.
    furi_hal_gpio_init(pin, GpioModeOutputOpenDrain, GpioPullNo, GpioSpeedLow);
    furi_hal_gpio_write(pin, true);
    furi_hal_gpio_write(pin, false);
    furi_delay_ms(1200);
    furi_hal_gpio_write(pin, true);

    return true;
}

void sim800_number_ask(Sim800App* app) {
    // An empty phonebook has nothing to pick from, so do not ask
    scene_manager_next_scene(
        app->scene_manager,
        (sim800_phonebook_count() > 0) ? Sim800SceneNumberSource : Sim800SceneNumberInput);
}

void sim800_number_chosen(Sim800App* app) {
    if(app->mode == Sim800ModeCall) {
        scene_manager_set_scene_state(app->scene_manager, Sim800SceneCall, 0);
        scene_manager_next_scene(app->scene_manager, Sim800SceneCall);
    } else if(app->mode == Sim800ModeSmsReady) {
        scene_manager_set_scene_state(app->scene_manager, Sim800SceneSmsResult, 0);
        scene_manager_next_scene(app->scene_manager, Sim800SceneSmsResult);
    } else {
        sim800_sms_compose(app);
    }
}

void sim800_sms_compose(Sim800App* app) {
    furi_string_reset(app->sms_text);
    // With nothing saved there is nothing to choose between, so skip the question
    app->preset_count = sim800_preset_list(app->preset_names, SIM800_PRESET_MAX);
    scene_manager_next_scene(
        app->scene_manager, (app->preset_count > 0) ? Sim800SceneSmsSource : Sim800SceneSmsText);
}

// sequence_audiovisual_alert buzzes, blinks the screen and runs long. For something that
// repeats every couple of seconds while a call rings, that is far too much.
const NotificationSequence sim800_sequence_ring = {
    &message_display_backlight_on,
    &message_vibro_on,
    &message_note_e6,
    &message_delay_100,
    &message_sound_off,
    &message_delay_25,
    &message_note_d6,
    &message_delay_100,
    // Off well before the melody ends: a buzz through the whole ring is the heavy part
    &message_vibro_off,
    &message_sound_off,
    &message_delay_25,
    &message_note_fs5,
    &message_delay_250,
    &message_sound_off,
    &message_delay_25,
    &message_note_gs5,
    &message_delay_250,
    &message_sound_off,
    NULL,
};

void sim800_notify_result(Sim800App* app, bool ok) {
    if(!app->settings.notify) return;
    notification_message(app->notifications, ok ? &sequence_success : &sequence_error);
}

const char* sim800_at_result_text(Sim800AtResult result) {
    switch(result) {
    case Sim800AtResultOk:
        return "OK";
    case Sim800AtResultError:
        return "Module returned an error";
    case Sim800AtResultTimeout:
        return "No answer from the module";
    case Sim800AtResultNotOpen:
    default:
        return "UART is not open";
    }
}

void sim800_widget_button_callback(GuiButtonType result, InputType type, void* context) {
    Sim800App* app = context;
    if(type != InputTypeShort) return;

    if(result == GuiButtonTypeLeft) {
        view_dispatcher_send_custom_event(app->view_dispatcher, Sim800CustomEventButtonLeft);
    } else if(result == GuiButtonTypeCenter) {
        view_dispatcher_send_custom_event(app->view_dispatcher, Sim800CustomEventButtonCenter);
    } else if(result == GuiButtonTypeRight) {
        view_dispatcher_send_custom_event(app->view_dispatcher, Sim800CustomEventButtonRight);
    }
}

void sim800_input_done_callback(void* context) {
    Sim800App* app = context;
    view_dispatcher_send_custom_event(app->view_dispatcher, Sim800CustomEventTextInputDone);
}

void sim800_submenu_callback(void* context, uint32_t index) {
    Sim800App* app = context;
    view_dispatcher_send_custom_event(app->view_dispatcher, index);
}

static Sim800App* sim800_app_alloc(void) {
    Sim800App* app = malloc(sizeof(Sim800App));

    app->gui = furi_record_open(RECORD_GUI);
    app->notifications = furi_record_open(RECORD_NOTIFICATION);
    app->dialogs = furi_record_open(RECORD_DIALOGS);
    app->power = furi_record_open(RECORD_POWER);

    app->view_dispatcher = view_dispatcher_alloc();
    app->scene_manager = scene_manager_alloc(&sim800_scene_handlers, app);

    view_dispatcher_set_event_callback_context(app->view_dispatcher, app);
    view_dispatcher_set_custom_event_callback(
        app->view_dispatcher, sim800_app_custom_event_callback);
    view_dispatcher_set_navigation_event_callback(
        app->view_dispatcher, sim800_app_back_event_callback);
    view_dispatcher_set_tick_event_callback(
        app->view_dispatcher, sim800_app_tick_event_callback, 1000);

    app->submenu = submenu_alloc();
    view_dispatcher_add_view(
        app->view_dispatcher, Sim800ViewSubmenu, submenu_get_view(app->submenu));

    app->text_input = sim800_text_input_alloc();
    view_dispatcher_add_view(
        app->view_dispatcher, Sim800ViewTextInput, sim800_text_input_get_view(app->text_input));

    app->text_box = text_box_alloc();
    view_dispatcher_add_view(
        app->view_dispatcher, Sim800ViewTextBox, text_box_get_view(app->text_box));

    app->widget = widget_alloc();
    view_dispatcher_add_view(app->view_dispatcher, Sim800ViewWidget, widget_get_view(app->widget));

    app->var_item_list = variable_item_list_alloc();
    view_dispatcher_add_view(
        app->view_dispatcher,
        Sim800ViewVarItemList,
        variable_item_list_get_view(app->var_item_list));

    app->popup = popup_alloc();
    view_dispatcher_add_view(app->view_dispatcher, Sim800ViewPopup, popup_get_view(app->popup));

    app->call_view = sim800_call_view_alloc();
    view_dispatcher_add_view(
        app->view_dispatcher, Sim800ViewCall, sim800_call_view_get_view(app->call_view));

    app->status_view = sim800_status_view_alloc();
    view_dispatcher_add_view(
        app->view_dispatcher, Sim800ViewStatus, sim800_status_view_get_view(app->status_view));

    app->entry_view = sim800_entry_view_alloc();
    view_dispatcher_add_view(
        app->view_dispatcher, Sim800ViewEntry, sim800_entry_view_get_view(app->entry_view));

    app->howto_view = sim800_howto_view_alloc();
    app->connect_view = sim800_connect_view_alloc();
    app->dtmf_view = sim800_dtmf_view_alloc();
    app->net_operator = furi_string_alloc();
    app->net_rssi = 99;
    view_dispatcher_add_view(
        app->view_dispatcher, Sim800ViewHowto, sim800_howto_view_get_view(app->howto_view));
    view_dispatcher_add_view(
        app->view_dispatcher, Sim800ViewConnect, sim800_connect_view_get_view(app->connect_view));
    view_dispatcher_add_view(
        app->view_dispatcher, Sim800ViewDtmf, sim800_dtmf_view_get_view(app->dtmf_view));

    app->number = furi_string_alloc();
    app->sms_text = furi_string_alloc();
    app->file_path = furi_string_alloc_set_str(SIM800_PHONEBOOK_FOLDER);
    app->entry_name = furi_string_alloc();
    app->text_box_store = furi_string_alloc();
    app->log = furi_string_alloc();
    app->urc_line = furi_string_alloc();
    app->task_response = furi_string_alloc();
    app->log_mutex = furi_mutex_alloc(FuriMutexTypeNormal);
    app->urc_sem = furi_semaphore_alloc(1, 0);

    for(size_t i = 0; i < SIM800_PRESET_MAX; i++) {
        app->preset_names[i] = furi_string_alloc();
    }
    app->preset_count = 0;

    sim800_inbox_init(&app->inbox);

    app->text_store[0] = '\0';
    app->incoming_number[0] = '\0';
    app->task_thread = NULL;
    app->task_fn = NULL;
    app->task_busy = false;
    app->task_result = Sim800AtResultNotOpen;
    app->mode = Sim800ModeCall;
    app->save_target = Sim800SaveTargetNumber;
    app->call_incoming = false;
    app->number_saved = false;
    app->preset_is_new = false;
    app->last_ok = false;
    app->redial = false;
    app->call_keep = false;
    app->call_start_tick = 0;
    app->selected_sms = 0;
    app->audio_volume = 50;
    app->audio_ringer = 50;
    app->audio_mic = 8;
    app->audio_channel = 0;

    sim800_settings_load(&app->settings);

    app->uart = sim800_uart_alloc();
    sim800_uart_set_line_callback(app->uart, sim800_app_line_callback, app);

    return app;
}

static void sim800_app_free(Sim800App* app) {
    sim800_task_join(app);

    // The dispatcher has stopped draining its queue, so stop feeding it before the worker dies
    sim800_uart_set_line_callback(app->uart, NULL, NULL);
    sim800_uart_free(app->uart);

    view_dispatcher_remove_view(app->view_dispatcher, Sim800ViewSubmenu);
    view_dispatcher_remove_view(app->view_dispatcher, Sim800ViewTextInput);
    view_dispatcher_remove_view(app->view_dispatcher, Sim800ViewTextBox);
    view_dispatcher_remove_view(app->view_dispatcher, Sim800ViewWidget);
    view_dispatcher_remove_view(app->view_dispatcher, Sim800ViewVarItemList);
    view_dispatcher_remove_view(app->view_dispatcher, Sim800ViewPopup);
    view_dispatcher_remove_view(app->view_dispatcher, Sim800ViewCall);
    view_dispatcher_remove_view(app->view_dispatcher, Sim800ViewStatus);
    view_dispatcher_remove_view(app->view_dispatcher, Sim800ViewEntry);
    view_dispatcher_remove_view(app->view_dispatcher, Sim800ViewHowto);
    view_dispatcher_remove_view(app->view_dispatcher, Sim800ViewConnect);
    view_dispatcher_remove_view(app->view_dispatcher, Sim800ViewDtmf);

    submenu_free(app->submenu);
    sim800_text_input_free(app->text_input);
    text_box_free(app->text_box);
    widget_free(app->widget);
    variable_item_list_free(app->var_item_list);
    popup_free(app->popup);
    sim800_call_view_free(app->call_view);
    sim800_status_view_free(app->status_view);
    sim800_entry_view_free(app->entry_view);
    sim800_howto_view_free(app->howto_view);
    sim800_connect_view_free(app->connect_view);
    sim800_dtmf_view_free(app->dtmf_view);
    furi_string_free(app->net_operator);

    view_dispatcher_free(app->view_dispatcher);
    scene_manager_free(app->scene_manager);

    sim800_inbox_free(&app->inbox);
    for(size_t i = 0; i < SIM800_PRESET_MAX; i++) {
        furi_string_free(app->preset_names[i]);
    }

    furi_string_free(app->number);
    furi_string_free(app->sms_text);
    furi_string_free(app->file_path);
    furi_string_free(app->entry_name);
    furi_string_free(app->text_box_store);
    furi_string_free(app->log);
    furi_string_free(app->urc_line);
    furi_string_free(app->task_response);
    furi_mutex_free(app->log_mutex);
    furi_semaphore_free(app->urc_sem);

    furi_record_close(RECORD_POWER);
    furi_record_close(RECORD_DIALOGS);
    furi_record_close(RECORD_NOTIFICATION);
    furi_record_close(RECORD_GUI);

    free(app);
}

/** The external CC1101's power amp keys off pin 7, and a SubGHz session that ended badly can
 * leave it high. Park it low - RX, amp off - so it is not keyed while we hold the 5V rail.
 */
#define SIM800_EXT_AMP_GPIO (&gpio_ext_pc3)

static void sim800_ext_amp_park(void) {
    furi_hal_gpio_init_simple(SIM800_EXT_AMP_GPIO, GpioModeOutputPushPull);
    furi_hal_gpio_write(SIM800_EXT_AMP_GPIO, false);
}

static void sim800_ext_amp_release(void) {
    furi_hal_gpio_init_simple(SIM800_EXT_AMP_GPIO, GpioModeAnalog);
}

/** Bring up pin 1, retrying because the module's bulk cap browns the boost out on inrush.
 *
 * The request goes through the Power service rather than the HAL so every other 5V consumer
 * keeps seeing the true rail state; the service retries five times of its own, but back to
 * back, which a few thousand microfarads will lose every time. The delay here is the point.
 */
static void sim800_power_5v_on(Sim800App* app) {
    app->otg_was_on = power_is_otg_enabled(app->power);
    if(app->otg_was_on) return;

    for(size_t attempt = 0; attempt < 5; attempt++) {
        power_enable_otg(app->power, true);
        furi_delay_ms(20);
        if(furi_hal_power_is_otg_enabled()) break;
    }
}

static void sim800_power_5v_off(Sim800App* app) {
    if(app->otg_was_on) return;
    power_enable_otg(app->power, false);
}

int32_t sim800_app(void* arg) {
    Expansion* expansion = furi_record_open(RECORD_EXPANSION);
    expansion_disable(expansion);

    Sim800App* app = sim800_app_alloc();

    // Park the amp before the rail comes up, not after: order matters here
    sim800_ext_amp_park();
    sim800_power_5v_on(app);

    sim800_storage_make_folders();

    view_dispatcher_attach_to_gui(app->view_dispatcher, app->gui, ViewDispatcherTypeFullscreen);
    scene_manager_next_scene(app->scene_manager, Sim800SceneStart);

    // Archive hands us the .phone file to open straight into
    if((arg != NULL) && (strlen(arg) != 0)) {
        furi_string_set_str(app->file_path, (const char*)arg);
        if(sim800_phonebook_load(
               furi_string_get_cstr(app->file_path), app->entry_name, app->number)) {
            // It came out of the phonebook, so the call screen must not offer to save it
            app->number_saved = true;
            app->redial = false;
            scene_manager_next_scene(app->scene_manager, Sim800SceneSavedMenu);
        }
    }

    // Connecting goes on top of whatever we landed on, and pops itself when it is done
    scene_manager_next_scene(app->scene_manager, Sim800SceneConnect);

    view_dispatcher_run(app->view_dispatcher);

    sim800_power_5v_off(app);
    sim800_ext_amp_release();

    // Hand PWRKEY back before the pin is anyone else's
    if(sim800_pwrkey_state(app) == Sim800PwrkeyReady) {
        furi_hal_gpio_init_simple(sim800_pwrkey_pins[app->settings.pwrkey_pin], GpioModeAnalog);
    }

    sim800_app_free(app);

    expansion_enable(expansion);
    furi_record_close(RECORD_EXPANSION);

    return 0;
}
