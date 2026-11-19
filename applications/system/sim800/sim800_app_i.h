#pragma once

#include <furi.h>
#include <furi_hal.h>

#include <gui/gui.h>
#include <gui/view_dispatcher.h>
#include <gui/scene_manager.h>
#include <gui/modules/submenu.h>
#include <gui/modules/text_box.h>
#include <gui/modules/widget.h>
#include <gui/modules/variable_item_list.h>
#include <gui/modules/popup.h>
#include <dialogs/dialogs.h>
#include <notification/notification_messages.h>
#include <power/power_service/power.h>
#include <storage/storage.h>

#include "sim800_icons.h"

#include "helpers/sim800_types.h"
#include "helpers/sim800_custom_event.h"
#include "helpers/sim800_settings.h"
#include "helpers/sim800_uart.h"
#include "helpers/sim800_at.h"
#include "helpers/sim800_phonebook.h"
#include "helpers/sim800_history.h"
#include "helpers/sim800_sms.h"
#include "views/sim800_text_input.h"
#include "views/sim800_connect_view.h"
#include "views/sim800_dtmf_view.h"
#include "views/sim800_call_view.h"
#include "views/sim800_status_view.h"
#include "views/sim800_entry_view.h"
#include "views/sim800_howto_view.h"
#include "scenes/sim800_scene.h"

#define SIM800_TAG "Sim800"

typedef enum {
    Sim800ViewSubmenu,
    Sim800ViewTextInput,
    Sim800ViewTextBox,
    Sim800ViewWidget,
    Sim800ViewVarItemList,
    Sim800ViewPopup,
    Sim800ViewCall,
    Sim800ViewStatus,
    Sim800ViewEntry,
    Sim800ViewHowto,
    Sim800ViewConnect,
    Sim800ViewDtmf,
} Sim800ViewId;

typedef enum {
    Sim800ModeCall,
    Sim800ModeSms,
    Sim800ModeEditNumber, //!< rewriting the number of a saved entry
    Sim800ModeNewEntry, //!< adding a number to the phonebook, with no call or SMS
    Sim800ModeSmsReady, //!< an SMS whose body is already picked, so only the number is left
} Sim800Mode;

typedef enum {
    Sim800SaveTargetNumber,
    Sim800SaveTargetNewEntry,
    Sim800SaveTargetPreset,
    Sim800SaveTargetNewPreset,
    Sim800SaveTargetRenameEntry,
    Sim800SaveTargetRenamePreset,
} Sim800SaveTarget;

typedef struct Sim800App Sim800App;

typedef void (*Sim800TaskFn)(Sim800App* app);

struct Sim800App {
    Gui* gui;
    ViewDispatcher* view_dispatcher;
    SceneManager* scene_manager;
    NotificationApp* notifications;
    DialogsApp* dialogs;
    Power* power;

    Submenu* submenu;
    Sim800TextInput* text_input;
    TextBox* text_box;
    Widget* widget;
    VariableItemList* var_item_list;
    Popup* popup;
    Sim800CallView* call_view;
    Sim800StatusView* status_view;
    Sim800EntryView* entry_view;
    Sim800HowtoView* howto_view;
    Sim800ConnectView* connect_view;
    Sim800DtmfView* dtmf_view;

    Sim800Settings settings;
    Sim800Uart* uart;
    Sim800Inbox inbox;

    FuriString* number;
    FuriString* sms_text;
    FuriString* file_path;
    FuriString* entry_name;
    FuriString* text_box_store; // backs the text box, which keeps the pointer, not a copy
    FuriString* log;
    FuriString* urc_line;
    FuriString* task_response;

    FuriMutex* log_mutex; // log is written on the UART worker, read on the GUI thread
    FuriSemaphore* urc_sem;

    char text_store[SIM800_TEXT_STORE_SIZE];
    char incoming_number[SIM800_NUMBER_MAX];
    char incoming_sms[16]; //!< storage index the last +CMTI named
    // RING repeats for as long as the caller waits, so one call must not log many times
    char history_last[SIM800_NUMBER_MAX];
    uint32_t history_tick;
    // The module shouts UNDER-VOLTAGE before it drops the network, which is the one piece of
    // evidence that separates a weak supply from a weak signal
    char power_alarm[24];
    uint32_t power_alarm_tick;

    FuriThread* task_thread;
    Sim800TaskFn task_fn;
    bool task_busy;
    Sim800AtResult task_result;

    Sim800Mode mode;
    Sim800SaveTarget save_target;
    bool call_incoming;
    bool number_saved;
    bool preset_is_new;
    uint32_t pending_baud; // rate the Set module baud screen is moving to
    // popup_set_text keeps the pointer it is handed, so popup strings cannot live on a stack
    char popup_text[40];
    bool last_ok; // outcome of the last background task, for the notification
    bool otg_was_on; // 5V was already requested when we started, so it is not ours to drop
    bool redial; // a call to the current number ended, so offer Re-dial
    uint32_t call_poll; // ticks since entering the call screen
    uint32_t call_run; // polls actually run, which is far fewer
    uint8_t call_idle_polls; // consecutive polls that saw no call
    Sim800CallState call_last_state; // what we last gave feedback for
    char dtmf_digit; // tone the pad is sending right now
    char dtmf_sent[32]; // what has gone out this visit, shown back on the pad
    bool dtmf_watch; // the running task is the call-still-up check, not a tone
    // Last carrier and signal seen anywhere, so a screen can open with them already filled
    FuriString* net_operator;
    uint8_t net_rssi;
    uint32_t call_start_tick;
    bool call_keep; // leaving the call scene to push another, not to hang up
    size_t selected_sms;
    uint8_t audio_volume;
    uint8_t audio_ringer;
    uint8_t audio_mic;
    uint8_t audio_channel;

    FuriString* preset_names[SIM800_PRESET_MAX];
    size_t preset_count;
};

/** Open the configured port and put the module into a known state. */
/** Reopen the port and re-apply the module's settings.
 *
 * @param attempts how many "AT"s to spend locking on; few when the UI is waiting, many when a
 *                 module has just booted and may still be on autobaud.
 */
bool sim800_app_connect(Sim800App* app, size_t attempts);

/** Ask where the message body comes from, or go straight to the keyboard if nothing is saved. */
void sim800_sms_compose(Sim800App* app);

/** Ask where the number comes from, or go straight to the keyboard if the phonebook is empty. */
void sim800_number_ask(Sim800App* app);

/** Route on from a number that is now in ::Sim800App::number, by the current mode. */
void sim800_number_chosen(Sim800App* app);

/** Run @p fn off the GUI thread; posts ::Sim800CustomEventTaskDone when done.
 *  One at a time, and the scene that starts one must join it on exit. */
void sim800_task_start(Sim800App* app, Sim800TaskFn fn);
void sim800_task_join(Sim800App* app);
bool sim800_task_is_busy(Sim800App* app);

void sim800_log_read(Sim800App* app, FuriString* out);
void sim800_log_clear(Sim800App* app);

/** USSD answers OK and sends +CUSD after, so arm before the command and wait after. */
void sim800_arm_urc(Sim800App* app);
bool sim800_wait_urc(Sim800App* app, uint32_t timeout_ms, FuriString* out);

void sim800_show_busy(Sim800App* app, const char* header, const char* text);

/** The app's only feedback: green on a finished action, red on one that failed. */
void sim800_notify_result(Sim800App* app, bool ok);

/** Opening bar of Gran Vals, and a single backlight nudge. No vibro, no blinking. */
extern const NotificationSequence sim800_sequence_ring;

typedef enum {
    Sim800PwrkeyReady,
    Sim800PwrkeyNoPin, //!< set to Off
    Sim800PwrkeyUartPin, //!< the pin chosen is one the active UART is driving
} Sim800PwrkeyState;

Sim800PwrkeyState sim800_pwrkey_state(Sim800App* app);

/** Hold PWRKEY low long enough to toggle the module, false unless the state is Ready. */
bool sim800_pwrkey_pulse(Sim800App* app);

/** How long the module needs after the pulse before it answers AT. */
#define SIM800_PWRKEY_BOOT_MS 3000
const char* sim800_at_result_text(Sim800AtResult result);
void sim800_widget_button_callback(GuiButtonType result, InputType type, void* context);
void sim800_input_done_callback(void* context);
void sim800_submenu_callback(void* context, uint32_t index);
