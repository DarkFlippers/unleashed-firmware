/**
 * @file infrared_app_i.h
 * @brief Main Infrared application types and functions.
 */
#pragma once

#include <gui/gui.h>
#include <gui/view.h>
#include <assets_icons.h>
#include <gui/view_stack.h>
#include <gui/view_dispatcher.h>
#include <gui/scene_manager.h>

#include <gui/modules/popup.h>
#include <gui/modules/loading.h>
#include <gui/modules/submenu.h>
#include <gui/modules/dialog_ex.h>
#include <gui/modules/text_input.h>
#include <gui/modules/button_menu.h>
#include <gui/modules/button_panel.h>

#include <storage/storage.h>
#include <dialogs/dialogs.h>

#include <notification/notification_messages.h>

#include <infrared_worker.h>

#include "infrared_app.h"
#include "infrared_remote.h"
#include "infrared_brute_force.h"
#include "infrared_custom_event.h"

#include "scenes/infrared_scene.h"
#include "views/infrared_progress_view.h"
#include "views/infrared_debug_view.h"
#include "views/infrared_move_view.h"

#include "rpc/rpc_app.h"

#define INFRARED_FILE_NAME_SIZE 100
#define INFRARED_TEXT_STORE_NUM 2
#define INFRARED_TEXT_STORE_SIZE 128

#define INFRARED_MAX_BUTTON_NAME_LENGTH 22
#define INFRARED_MAX_REMOTE_NAME_LENGTH 22

#define INFRARED_APP_FOLDER ANY_PATH("infrared")
#define INFRARED_APP_EXTENSION ".ir"

#define INFRARED_DEFAULT_REMOTE_NAME "Remote"
#define INFRARED_LOG_TAG "InfraredApp"

/**
 * @brief Enumeration of invalid remote button indices.
 */
typedef enum {
    InfraredButtonIndexNone = -1, /**< No button is currently selected. */
} InfraredButtonIndex;

/**
 * @brief Enumeration of editing targets.
 */
typedef enum {
    InfraredEditTargetNone, /**< No editing target is selected. */
    InfraredEditTargetRemote, /**< Whole remote is selected as editing target. */
    InfraredEditTargetButton, /**< Single button is selected as editing target. */
} InfraredEditTarget;

/**
 * @brief Enumeration of editing modes.
 */
typedef enum {
    InfraredEditModeNone, /**< No editing mode is selected. */
    InfraredEditModeRename, /**< Rename mode is selected. */
    InfraredEditModeDelete, /**< Delete mode is selected. */
} InfraredEditMode;

/**
 * @brief Infrared application state type.
 */
typedef struct {
    bool is_learning_new_remote; /**< Learning new remote or adding to an existing one. */
    bool is_debug_enabled; /**< Whether to enable or disable debugging features. */
    bool is_transmitting; /**< Whether a signal is currently being transmitted. */
    InfraredEditTarget edit_target : 8; /**< Selected editing target (a remote or a button). */
    InfraredEditMode edit_mode : 8; /**< Selected editing operation (rename or delete). */
    int32_t current_button_index; /**< Selected button index (move destination). */
    int32_t prev_button_index; /**< Previous button index (move source). */
    uint32_t last_transmit_time; /**< Lat time a signal was transmitted. */
} InfraredAppState;

/**
 * @brief Infrared application type.
 */
struct InfraredApp {
    SceneManager* scene_manager; /**< Pointer to a SceneManager instance. */
    ViewDispatcher* view_dispatcher; /**< Pointer to a ViewDispatcher instance. */

    Gui* gui; /**< Pointer to a Gui instance. */
    Storage* storage; /**< Pointer to a Storage instance. */
    DialogsApp* dialogs; /**< Pointer to a DialogsApp instance. */
    NotificationApp* notifications; /**< Pointer to a NotificationApp instance. */
    InfraredWorker* worker; /**< Used to send or receive signals. */
    InfraredRemote* remote; /**< Holds the currently loaded remote. */
    InfraredSignal* current_signal; /**< Holds the currently loaded signal. */
    InfraredBruteForce* brute_force; /**< Used for the Universal Remote feature. */

    Submenu* submenu; /**< Standard view for displaying application menus. */
    TextInput* text_input; /**< Standard view for receiving user text input. */
    DialogEx* dialog_ex; /**< Standard view for displaying dialogs. */
    ButtonMenu* button_menu; /**< Custom view for interacting with IR remotes. */
    Popup* popup; /**< Standard view for displaying messages. */

    ViewStack* view_stack; /**< Standard view for displaying stacked interfaces. */
    InfraredDebugView* debug_view; /**< Custom view for displaying debug information. */
    InfraredMoveView* move_view; /**< Custom view for rearranging buttons in a remote. */

    ButtonPanel* button_panel; /**< Standard view for displaying control panels. */
    Loading* loading; /**< Standard view for informing about long operations. */
    InfraredProgressView* progress; /**< Custom view for showing brute force progress. */

    FuriString* file_path; /**< Full path to the currently loaded file. */
    FuriString* button_name; /** Name of the button requested in RPC mode. */
    /** Arbitrary text storage for various inputs. */
    char text_store[INFRARED_TEXT_STORE_NUM][INFRARED_TEXT_STORE_SIZE + 1];
    InfraredAppState app_state; /**< Application state. */

    void* rpc_ctx; /**< Pointer to the RPC context object. */
};

/**
 * @brief Enumeration of all used view types.
 */
typedef enum {
    InfraredViewSubmenu,
    InfraredViewTextInput,
    InfraredViewDialogEx,
    InfraredViewButtonMenu,
    InfraredViewPopup,
    InfraredViewStack,
    InfraredViewDebugView,
    InfraredViewMove,
} InfraredView;

/**
 * @brief Enumeration of all notification message types.
 */
typedef enum {
    InfraredNotificationMessageSuccess, /**< Play a short happy tune. */
    InfraredNotificationMessageGreenOn, /**< Turn green LED on. */
    InfraredNotificationMessageGreenOff, /**< Turn green LED off. */
    InfraredNotificationMessageYellowOn, /**< Turn yellow LED on. */
    InfraredNotificationMessageYellowOff, /**< Turn yellow LED off. */
    InfraredNotificationMessageBlinkStartRead, /**< Blink the LED to indicate receiver mode. */
    InfraredNotificationMessageBlinkStartSend, /**< Blink the LED to indicate transmitter mode. */
    InfraredNotificationMessageBlinkStop, /**< Stop blinking the LED. */
    InfraredNotificationMessageCount, /**< Special value equal to the message type count. */
} InfraredNotificationMessage;

/**
 * @brief Add a new remote with a single signal.
 *
 * The filename will be automatically generated depending on
 * the names and number of other files in the infrared data directory.
 *
 * @param[in] infrared pointer to the application instance.
 * @param[in] name pointer to a zero-terminated string containing the signal name.
 * @param[in] signal pointer to the signal to be added.
 * @return true if the remote was successfully created, false otherwise.
 */
bool infrared_add_remote_with_button(
    const InfraredApp* infrared,
    const char* name,
    const InfraredSignal* signal);

/**
 * @brief Rename the currently loaded remote.
 *
 * @param[in] infrared pointer to the application instance.
 * @param[in] new_name pointer to a zero-terminated string containing the new remote name.
 * @return true if the remote was successfully renamed, false otherwise.
 */
bool infrared_rename_current_remote(const InfraredApp* infrared, const char* new_name);

/**
 * @brief Begin transmission of the currently loaded signal.
 *
 * The signal will be repeated indefinitely until stopped.
 *
 * @param[in,out] infrared pointer to the application instance.
 */
void infrared_tx_start(InfraredApp* infrared);

/**
 * @brief Load a signal under the given index and begin transmission.
 *
 * The signal will be repeated indefinitely until stopped.
 *
 * @param[in,out] infrared pointer to the application instance.
 * @param[in] button_index index of the signal to be loaded.
 * @returns true if the signal could be loaded, false otherwise.
 */
void infrared_tx_start_button_index(InfraredApp* infrared, size_t button_index);

/**
 * @brief Stop transmission of the currently loaded signal.
 *
 * @param[in,out] infrared pointer to the application instance.
 */
void infrared_tx_stop(InfraredApp* infrared);

/**
 * @brief Set the internal text store with formatted text.
 *
 * @param[in,out] infrared pointer to the application instance.
 * @param[in] bank index of text store bank (0 or 1).
 * @param[in] fmt pointer to a zero-terminated string containing the format text.
 * @param[in] ... additional arguments.
 */
void infrared_text_store_set(InfraredApp* infrared, uint32_t bank, const char* fmt, ...)
    _ATTRIBUTE((__format__(__printf__, 3, 4)));

/**
 * @brief Clear the internal text store.
 *
 * @param[in,out] infrared pointer to the application instance.
 * @param[in] bank index of text store bank (0 or 1).
 */
void infrared_text_store_clear(InfraredApp* infrared, uint32_t bank);

/**
 * @brief Play a sound and/or blink the LED.
 *
 * @param[in] infrared pointer to the application instance.
 * @param[in] message type of the message to play.
 */
void infrared_play_notification_message(
    const InfraredApp* infrared,
    InfraredNotificationMessage message);

/**
 * @brief Show a loading pop-up screen.
 *
 * In order for this to work, a Stack view must be currently active and
 * the main view must be added to it.
 *
 * @param[in] infrared pointer to the application instance.
 * @param[in] show whether to show or hide the pop-up.
 */
void infrared_show_loading_popup(const InfraredApp* infrared, bool show);

/**
 * @brief Show a formatted error messsage.
 *
 * @param[in] infrared pointer to the application instance.
 * @param[in] fmt pointer to a zero-terminated string containing the format text.
 * @param[in] ... additional arguments.
 */
void infrared_show_error_message(const InfraredApp* infrared, const char* fmt, ...)
    _ATTRIBUTE((__format__(__printf__, 2, 3)));

/**
 * @brief Common received signal callback.
 *
 * Called when the worker has received a complete infrared signal.
 *
 * @param[in,out] context pointer to the user-specified context object.
 * @param[in] received_signal pointer to the received signal.
 */
void infrared_signal_received_callback(void* context, InfraredWorkerSignal* received_signal);

/**
 * @brief Common text input callback.
 *
 * Called when the input has been accepted by the user.
 *
 * @param[in,out] context pointer to the user-specified context object.
 */
void infrared_text_input_callback(void* context);

/**
 * @brief Common popup close callback.
 *
 * Called when the popup has been closed either by the user or after a timeout.
 *
 * @param[in,out] context pointer to the user-specified context object.
 */
void infrared_popup_closed_callback(void* context);
