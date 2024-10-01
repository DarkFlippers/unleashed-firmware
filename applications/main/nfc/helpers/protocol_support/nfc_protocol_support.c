/**
 * @file nfc_protocol_support.c
 * @brief Common implementation of application-level protocol support.
 *
 * @see nfc_protocol_support_base.h
 * @see nfc_protocol_support_common.h
 */
#include "nfc_protocol_support.h"

#include "nfc/nfc_app_i.h"

#include "nfc_protocol_support_defs.h"
#include "nfc_protocol_support_gui_common.h"

/**
 * @brief Common scene entry handler.
 *
 * @param[in,out] instance pointer to the NFC application instance.
 */
typedef void (*NfcProtocolSupportCommonOnEnter)(NfcApp* instance);

/**
 * @brief Common scene custom event handler.
 *
 * @param[in,out] instance pointer to the NFC application instance.
 * @param[in] event custom event to be handled.
 * @returns true if the event was handled, false otherwise.
 */
typedef bool (*NfcProtocolSupportCommonOnEvent)(NfcApp* instance, SceneManagerEvent event);

/**
 * @brief Common scene exit handler.
 *
 * @param[in,out] instance pointer to the NFC application instance.
 */
typedef void (*NfcProtocolSupportCommonOnExit)(NfcApp* instance);

/**
 * @brief Structure containing common scene handler pointers.
 */
typedef struct {
    NfcProtocolSupportCommonOnEnter on_enter; /**< Pointer to the on_enter() function. */
    NfcProtocolSupportCommonOnEvent on_event; /**< Pointer to the on_event() function. */
    NfcProtocolSupportCommonOnExit on_exit; /**< Pointer to the on_exit() function. */
} NfcProtocolSupportCommonSceneBase;

static const NfcProtocolSupportCommonSceneBase nfc_protocol_support_scenes[];

// Interface functions
void nfc_protocol_support_on_enter(NfcProtocolSupportScene scene, void* context) {
    furi_assert(scene < NfcProtocolSupportSceneCount);
    furi_assert(context);

    NfcApp* instance = context;
    nfc_protocol_support_scenes[scene].on_enter(instance);
}

bool nfc_protocol_support_on_event(
    NfcProtocolSupportScene scene,
    void* context,
    SceneManagerEvent event) {
    furi_assert(scene < NfcProtocolSupportSceneCount);
    furi_assert(context);

    NfcApp* instance = context;
    return nfc_protocol_support_scenes[scene].on_event(instance, event);
}

void nfc_protocol_support_on_exit(NfcProtocolSupportScene scene, void* context) {
    furi_assert(scene < NfcProtocolSupportSceneCount);
    furi_assert(context);

    NfcApp* instance = context;
    nfc_protocol_support_scenes[scene].on_exit(instance);
}

bool nfc_protocol_support_has_feature(NfcProtocol protocol, NfcProtocolFeature feature) {
    return nfc_protocol_support[protocol]->features & feature;
}

// Common scene handlers
// SceneInfo
static void nfc_protocol_support_scene_info_on_enter(NfcApp* instance) {
    const NfcProtocol protocol = nfc_device_get_protocol(instance->nfc_device);
    nfc_protocol_support[protocol]->scene_info.on_enter(instance);

    if(nfc_protocol_support_has_feature(protocol, NfcProtocolFeatureMoreInfo)) {
        widget_add_button_element(
            instance->widget,
            GuiButtonTypeRight,
            "More",
            nfc_protocol_support_common_widget_callback,
            instance);
    }

    view_dispatcher_switch_to_view(instance->view_dispatcher, NfcViewWidget);
}

static bool nfc_protocol_support_scene_info_on_event(NfcApp* instance, SceneManagerEvent event) {
    bool consumed = false;

    if(event.type == SceneManagerEventTypeCustom) {
        if(event.event == GuiButtonTypeRight) {
            scene_manager_next_scene(instance->scene_manager, NfcSceneMoreInfo);
            consumed = true;
        }
    } else if(event.type == SceneManagerEventTypeBack) {
        // If the card could not be parsed, return to the respective menu
        if(!scene_manager_get_scene_state(instance->scene_manager, NfcSceneSupportedCard)) {
            const uint32_t scenes[] = {NfcSceneSavedMenu, NfcSceneReadMenu};
            scene_manager_search_and_switch_to_previous_scene_one_of(
                instance->scene_manager, scenes, COUNT_OF(scenes));
            consumed = true;
        }
    }

    return consumed;
}

static void nfc_protocol_support_scene_info_on_exit(NfcApp* instance) {
    widget_reset(instance->widget);
}

// SceneMoreInfo
static void nfc_protocol_support_scene_more_info_on_enter(NfcApp* instance) {
    const NfcProtocol protocol = nfc_device_get_protocol(instance->nfc_device);
    nfc_protocol_support[protocol]->scene_more_info.on_enter(instance);
}

static bool
    nfc_protocol_support_scene_more_info_on_event(NfcApp* instance, SceneManagerEvent event) {
    bool consumed = false;

    const NfcProtocol protocol = nfc_device_get_protocol(instance->nfc_device);
    consumed = nfc_protocol_support[protocol]->scene_more_info.on_event(instance, event);

    return consumed;
}

static void nfc_protocol_support_scene_more_info_on_exit(NfcApp* instance) {
    text_box_reset(instance->text_box);
    widget_reset(instance->widget);
    furi_string_reset(instance->text_box_store);
}

// SceneRead
static void nfc_protocol_support_scene_read_on_enter(NfcApp* instance) {
    popup_set_header(instance->popup, "Don't move", 85, 27, AlignCenter, AlignTop);
    popup_set_icon(instance->popup, 12, 23, &A_Loading_24);

    view_dispatcher_switch_to_view(instance->view_dispatcher, NfcViewPopup);

    const NfcProtocol protocol = nfc_detected_protocols_get_selected(instance->detected_protocols);
    instance->poller = nfc_poller_alloc(instance->nfc, protocol);

    view_dispatcher_switch_to_view(instance->view_dispatcher, NfcViewPopup);
    nfc_supported_cards_load_cache(instance->nfc_supported_cards);

    // Start poller with the appropriate callback
    nfc_protocol_support[protocol]->scene_read.on_enter(instance);

    nfc_blink_read_start(instance);
}

static bool nfc_protocol_support_scene_read_on_event(NfcApp* instance, SceneManagerEvent event) {
    bool consumed = false;

    if(event.type == SceneManagerEventTypeCustom) {
        if(event.event == NfcCustomEventPollerSuccess) {
            nfc_poller_stop(instance->poller);
            nfc_poller_free(instance->poller);
            notification_message(instance->notifications, &sequence_success);
            scene_manager_next_scene(instance->scene_manager, NfcSceneReadSuccess);
            dolphin_deed(DolphinDeedNfcReadSuccess);
            consumed = true;
        } else if(event.event == NfcCustomEventPollerIncomplete) {
            nfc_poller_stop(instance->poller);
            nfc_poller_free(instance->poller);
            bool card_read = nfc_supported_cards_read(
                instance->nfc_supported_cards, instance->nfc_device, instance->nfc);
            if(card_read) {
                notification_message(instance->notifications, &sequence_success);
                scene_manager_next_scene(instance->scene_manager, NfcSceneReadSuccess);
                dolphin_deed(DolphinDeedNfcReadSuccess);
                consumed = true;
            } else {
                const NfcProtocol protocol =
                    nfc_detected_protocols_get_selected(instance->detected_protocols);
                consumed = nfc_protocol_support[protocol]->scene_read.on_event(instance, event);
            }
        } else if(event.event == NfcCustomEventPollerFailure) {
            nfc_poller_stop(instance->poller);
            nfc_poller_free(instance->poller);
            if(scene_manager_has_previous_scene(instance->scene_manager, NfcSceneDetect)) {
                scene_manager_search_and_switch_to_previous_scene(
                    instance->scene_manager, NfcSceneDetect);
            }
            consumed = true;
        } else if(event.event == NfcCustomEventCardDetected) {
            const NfcProtocol protocol =
                nfc_detected_protocols_get_selected(instance->detected_protocols);
            consumed = nfc_protocol_support[protocol]->scene_read.on_event(instance, event);
        }
    } else if(event.type == SceneManagerEventTypeBack) {
        nfc_poller_stop(instance->poller);
        nfc_poller_free(instance->poller);
        static const uint32_t possible_scenes[] = {NfcSceneSelectProtocol, NfcSceneStart};
        scene_manager_search_and_switch_to_previous_scene_one_of(
            instance->scene_manager, possible_scenes, COUNT_OF(possible_scenes));
        consumed = true;
    }

    return consumed;
}

static void nfc_protocol_support_scene_read_on_exit(NfcApp* instance) {
    popup_reset(instance->popup);

    nfc_blink_stop(instance);
}

// SceneReadMenu
static void nfc_protocol_support_scene_read_menu_on_enter(NfcApp* instance) {
    const NfcProtocol protocol = nfc_device_get_protocol(instance->nfc_device);

    Submenu* submenu = instance->submenu;

    submenu_add_item(
        submenu,
        "Save",
        SubmenuIndexCommonSave,
        nfc_protocol_support_common_submenu_callback,
        instance);

    if(scene_manager_has_previous_scene(instance->scene_manager, NfcSceneGenerateInfo)) {
        submenu_add_item(
            submenu,
            "Change UID",
            SubmenuIndexCommonEdit,
            nfc_protocol_support_common_submenu_callback,
            instance);
    }

    if(nfc_protocol_support_has_feature(protocol, NfcProtocolFeatureEmulateUid)) {
        submenu_add_item(
            submenu,
            "Emulate UID",
            SubmenuIndexCommonEmulate,
            nfc_protocol_support_common_submenu_callback,
            instance);

    } else if(nfc_protocol_support_has_feature(protocol, NfcProtocolFeatureEmulateFull)) {
        submenu_add_item(
            submenu,
            "Emulate",
            SubmenuIndexCommonEmulate,
            nfc_protocol_support_common_submenu_callback,
            instance);
    }

    nfc_protocol_support[protocol]->scene_read_menu.on_enter(instance);

    submenu_add_item(
        submenu,
        "Info",
        SubmenuIndexCommonInfo,
        nfc_protocol_support_common_submenu_callback,
        instance);

    submenu_set_selected_item(
        instance->submenu,
        scene_manager_get_scene_state(instance->scene_manager, NfcSceneReadMenu));

    view_dispatcher_switch_to_view(instance->view_dispatcher, NfcViewMenu);
}

static bool
    nfc_protocol_support_scene_read_menu_on_event(NfcApp* instance, SceneManagerEvent event) {
    bool consumed = false;

    if(event.type == SceneManagerEventTypeCustom) {
        scene_manager_set_scene_state(instance->scene_manager, NfcSceneReadMenu, event.event);

        if(event.event == SubmenuIndexCommonSave) {
            scene_manager_next_scene(instance->scene_manager, NfcSceneSaveName);
            consumed = true;
        } else if(event.event == SubmenuIndexCommonInfo) {
            scene_manager_next_scene(instance->scene_manager, NfcSceneInfo);
            consumed = true;
        } else if(event.event == SubmenuIndexCommonEmulate) {
            dolphin_deed(DolphinDeedNfcEmulate);
            scene_manager_next_scene(instance->scene_manager, NfcSceneEmulate);
            consumed = true;
        } else {
            const NfcProtocol protocol = nfc_device_get_protocol(instance->nfc_device);
            consumed = nfc_protocol_support[protocol]->scene_read_menu.on_event(instance, event);
        }

    } else if(event.type == SceneManagerEventTypeBack) {
        scene_manager_set_scene_state(instance->scene_manager, NfcSceneSavedMenu, 0);
    }

    return consumed;
}

// Same for read_menu and saved_menu
static void nfc_protocol_support_scene_read_saved_menu_on_exit(NfcApp* instance) {
    submenu_reset(instance->submenu);
}

// SceneReadSuccess
static void nfc_protocol_support_scene_read_success_on_enter(NfcApp* instance) {
    Widget* widget = instance->widget;

    FuriString* temp_str = furi_string_alloc();
    if(nfc_supported_cards_parse(instance->nfc_supported_cards, instance->nfc_device, temp_str)) {
        widget_add_text_scroll_element(
            instance->widget, 0, 0, 128, 52, furi_string_get_cstr(temp_str));
    } else {
        const NfcProtocol protocol = nfc_device_get_protocol(instance->nfc_device);
        nfc_protocol_support[protocol]->scene_read_success.on_enter(instance);
    }

    furi_string_free(temp_str);

    widget_add_button_element(
        widget, GuiButtonTypeLeft, "Retry", nfc_protocol_support_common_widget_callback, instance);
    widget_add_button_element(
        widget, GuiButtonTypeRight, "More", nfc_protocol_support_common_widget_callback, instance);

    notification_message_block(instance->notifications, &sequence_set_green_255);
    view_dispatcher_switch_to_view(instance->view_dispatcher, NfcViewWidget);
}

static bool
    nfc_protocol_support_scene_read_success_on_event(NfcApp* instance, SceneManagerEvent event) {
    bool consumed = false;

    if(event.type == SceneManagerEventTypeCustom) {
        if(event.event == GuiButtonTypeLeft) {
            scene_manager_next_scene(instance->scene_manager, NfcSceneRetryConfirm);
            consumed = true;

        } else if(event.event == GuiButtonTypeRight) {
            scene_manager_next_scene(instance->scene_manager, NfcSceneReadMenu);
            consumed = true;
        }

    } else if(event.type == SceneManagerEventTypeBack) {
        scene_manager_next_scene(instance->scene_manager, NfcSceneExitConfirm);
        consumed = true;
    }

    return consumed;
}

static void nfc_protocol_support_scene_read_success_on_exit(NfcApp* instance) {
    notification_message_block(instance->notifications, &sequence_reset_green);
    widget_reset(instance->widget);
}

// SceneSavedMenu
static void nfc_protocol_support_scene_saved_menu_on_enter(NfcApp* instance) {
    const NfcProtocol protocol = nfc_device_get_protocol(instance->nfc_device);

    Submenu* submenu = instance->submenu;

    // Header submenu items
    if(nfc_protocol_support_has_feature(protocol, NfcProtocolFeatureEmulateUid)) {
        submenu_add_item(
            submenu,
            "Emulate UID",
            SubmenuIndexCommonEmulate,
            nfc_protocol_support_common_submenu_callback,
            instance);

    } else if(nfc_protocol_support_has_feature(protocol, NfcProtocolFeatureEmulateFull)) {
        submenu_add_item(
            submenu,
            "Emulate",
            SubmenuIndexCommonEmulate,
            nfc_protocol_support_common_submenu_callback,
            instance);
    }

    if(nfc_protocol_support_has_feature(protocol, NfcProtocolFeatureEditUid)) {
        submenu_add_item(
            submenu,
            "Edit UID",
            SubmenuIndexCommonEdit,
            nfc_protocol_support_common_submenu_callback,
            instance);
    }

    // Protocol-dependent menu items
    nfc_protocol_support[protocol]->scene_saved_menu.on_enter(instance);

    // Trailer submenu items
    if(nfc_has_shadow_file(instance)) {
        submenu_add_item(
            submenu,
            "Restore to Original State",
            SubmenuIndexCommonRestore,
            nfc_protocol_support_common_submenu_callback,
            instance);
    }

    submenu_add_item(
        submenu,
        "Rename",
        SubmenuIndexCommonRename,
        nfc_protocol_support_common_submenu_callback,
        instance);
    submenu_add_item(
        submenu,
        "Delete",
        SubmenuIndexCommonDelete,
        nfc_protocol_support_common_submenu_callback,
        instance);
    submenu_add_item(
        submenu,
        "Info",
        SubmenuIndexCommonInfo,
        nfc_protocol_support_common_submenu_callback,
        instance);

    submenu_set_selected_item(
        instance->submenu,
        scene_manager_get_scene_state(instance->scene_manager, NfcSceneSavedMenu));

    view_dispatcher_switch_to_view(instance->view_dispatcher, NfcViewMenu);
}

static bool
    nfc_protocol_support_scene_saved_menu_on_event(NfcApp* instance, SceneManagerEvent event) {
    bool consumed = false;

    if(event.type == SceneManagerEventTypeCustom) {
        scene_manager_set_scene_state(instance->scene_manager, NfcSceneSavedMenu, event.event);

        if(event.event == SubmenuIndexCommonRestore) {
            scene_manager_next_scene(instance->scene_manager, NfcSceneRestoreOriginalConfirm);
            consumed = true;
        } else if(event.event == SubmenuIndexCommonInfo) {
            scene_manager_next_scene(instance->scene_manager, NfcSceneSupportedCard);
            consumed = true;
        } else if(event.event == SubmenuIndexCommonRename) {
            scene_manager_next_scene(instance->scene_manager, NfcSceneSaveName);
            consumed = true;
        } else if(event.event == SubmenuIndexCommonDelete) {
            scene_manager_next_scene(instance->scene_manager, NfcSceneDelete);
            consumed = true;
        } else if(event.event == SubmenuIndexCommonEmulate) {
            const bool is_added =
                scene_manager_has_previous_scene(instance->scene_manager, NfcSceneSetType);
            dolphin_deed(is_added ? DolphinDeedNfcAddEmulate : DolphinDeedNfcEmulate);
            scene_manager_next_scene(instance->scene_manager, NfcSceneEmulate);
            consumed = true;
        } else if(event.event == SubmenuIndexCommonEdit) {
            scene_manager_next_scene(instance->scene_manager, NfcSceneSetUid);
            consumed = true;
        } else {
            const NfcProtocol protocol = nfc_device_get_protocol(instance->nfc_device);
            consumed = nfc_protocol_support[protocol]->scene_saved_menu.on_event(instance, event);
        }

    } else if(event.type == SceneManagerEventTypeBack) {
        scene_manager_set_scene_state(instance->scene_manager, NfcSceneSavedMenu, 0);
    }

    return consumed;
}

// SceneSaveName

static void nfc_protocol_support_scene_save_name_on_enter(NfcApp* instance) {
    FuriString* folder_path = furi_string_alloc();
    TextInput* text_input = instance->text_input;

    bool name_is_empty = furi_string_empty(instance->file_name);
    if(name_is_empty) {
        furi_string_set(instance->file_path, NFC_APP_FOLDER);
        name_generator_make_auto(
            instance->text_store, NFC_TEXT_STORE_SIZE, NFC_APP_FILENAME_PREFIX);
        furi_string_set(folder_path, NFC_APP_FOLDER);
    } else {
        nfc_text_store_set(instance, "%s", furi_string_get_cstr(instance->file_name));
        path_extract_dirname(furi_string_get_cstr(instance->file_path), folder_path);
    }

    text_input_set_header_text(text_input, "Name the card");
    text_input_set_result_callback(
        text_input,
        nfc_protocol_support_common_text_input_done_callback,
        instance,
        instance->text_store,
        NFC_NAME_SIZE,
        name_is_empty);

    ValidatorIsFile* validator_is_file = validator_is_file_alloc_init(
        furi_string_get_cstr(folder_path),
        NFC_APP_EXTENSION,
        furi_string_get_cstr(instance->file_name));
    text_input_set_validator(text_input, validator_is_file_callback, validator_is_file);

    furi_string_free(folder_path);

    view_dispatcher_switch_to_view(instance->view_dispatcher, NfcViewTextInput);
}

static bool
    nfc_protocol_support_scene_save_name_on_event(NfcApp* instance, SceneManagerEvent event) {
    bool consumed = false;

    if(event.type == SceneManagerEventTypeCustom) {
        if(event.event == NfcCustomEventTextInputDone) {
            if(!furi_string_empty(instance->file_name)) {
                nfc_delete(instance);
            }
            furi_string_set(instance->file_name, instance->text_store);

            if(nfc_save(instance)) {
                scene_manager_next_scene(instance->scene_manager, NfcSceneSaveSuccess);
                dolphin_deed(
                    scene_manager_has_previous_scene(instance->scene_manager, NfcSceneSetType) ?
                        DolphinDeedNfcAddSave :
                        DolphinDeedNfcSave);

                const NfcProtocol protocol = nfc_device_get_protocol(instance->nfc_device);
                consumed =
                    nfc_protocol_support[protocol]->scene_save_name.on_event(instance, event);
            } else {
                consumed = scene_manager_search_and_switch_to_previous_scene(
                    instance->scene_manager, NfcSceneStart);
            }
        }
    }

    return consumed;
}

static void nfc_protocol_support_scene_save_name_on_exit(NfcApp* instance) {
    void* validator_context = text_input_get_validator_callback_context(instance->text_input);
    text_input_set_validator(instance->text_input, NULL, NULL);
    validator_is_file_free(validator_context);

    text_input_reset(instance->text_input);
}

// SceneEmulate
/**
 * @brief Current view displayed on the emulation scene.
 *
 * The emulation scehe has two states: the default one showing information about
 * the card being emulated, and the logs which show the raw data received from the reader.
 *
 * The user has the ability to switch betweeen these two scenes, however the prompt to switch is
 * only shown after some information had appered in the log view.
 */
enum {
    NfcSceneEmulateStateWidget, /**< Widget view is displayed. */
    NfcSceneEmulateStateWidgetLog, /**< Widget view with Log button is displayed */
    NfcSceneEmulateStateTextBox, /**< TextBox view is displayed. */
};

static void nfc_protocol_support_scene_emulate_on_enter(NfcApp* instance) {
    Widget* widget = instance->widget;
    TextBox* text_box = instance->text_box;

    FuriString* temp_str = furi_string_alloc();
    const NfcProtocol protocol = nfc_device_get_protocol(instance->nfc_device);

    widget_add_icon_element(widget, 0, 3, &I_NFC_dolphin_emulation_51x64);

    if(nfc_protocol_support_has_feature(protocol, NfcProtocolFeatureEmulateUid)) {
        widget_add_string_element(
            widget, 90, 26, AlignCenter, AlignCenter, FontPrimary, "Emulating UID");

        size_t uid_len;
        const uint8_t* uid = nfc_device_get_uid(instance->nfc_device, &uid_len);

        for(size_t i = 0; i < uid_len; ++i) {
            furi_string_cat_printf(temp_str, "%02X ", uid[i]);
        }

        furi_string_trim(temp_str);

    } else {
        widget_add_string_element(
            widget, 90, 26, AlignCenter, AlignCenter, FontPrimary, "Emulating");
        if(!furi_string_empty(instance->file_name)) {
            furi_string_set(temp_str, instance->file_name);
        } else {
            furi_string_printf(
                temp_str,
                "Unsaved\n%s",
                nfc_device_get_name(instance->nfc_device, NfcDeviceNameTypeFull));
            furi_string_replace_str(temp_str, "Mifare", "MIFARE");
        }
    }

    widget_add_text_box_element(
        widget, 50, 33, 78, 31, AlignCenter, AlignTop, furi_string_get_cstr(temp_str), false);

    furi_string_free(temp_str);

    text_box_set_font(text_box, TextBoxFontHex);
    text_box_set_focus(text_box, TextBoxFocusEnd);
    furi_string_reset(instance->text_box_store);

    // instance->listener is allocated in the respective on_enter() handler
    nfc_protocol_support[protocol]->scene_emulate.on_enter(instance);

    scene_manager_set_scene_state(
        instance->scene_manager, NfcSceneEmulate, NfcSceneEmulateStateWidget);

    view_dispatcher_switch_to_view(instance->view_dispatcher, NfcViewWidget);
    nfc_blink_emulate_start(instance);
}

static bool
    nfc_protocol_support_scene_emulate_on_event(NfcApp* instance, SceneManagerEvent event) {
    bool consumed = false;

    const uint32_t state = scene_manager_get_scene_state(instance->scene_manager, NfcSceneEmulate);

    if(event.type == SceneManagerEventTypeCustom) {
        if(event.event == NfcCustomEventListenerUpdate) {
            // Add data button to widget if data is received for the first time
            if(furi_string_size(instance->text_box_store)) {
                widget_add_button_element(
                    instance->widget,
                    GuiButtonTypeCenter,
                    "Log",
                    nfc_protocol_support_common_widget_callback,
                    instance);
                scene_manager_set_scene_state(
                    instance->scene_manager, NfcSceneEmulate, NfcSceneEmulateStateWidgetLog);
            }
            // Update TextBox data
            text_box_set_text(instance->text_box, furi_string_get_cstr(instance->text_box_store));
            consumed = true;
        } else if(event.event == GuiButtonTypeCenter) {
            if(state == NfcSceneEmulateStateWidgetLog) {
                view_dispatcher_switch_to_view(instance->view_dispatcher, NfcViewTextBox);
                scene_manager_set_scene_state(
                    instance->scene_manager, NfcSceneEmulate, NfcSceneEmulateStateTextBox);
                consumed = true;
            }
        }
    } else if(event.type == SceneManagerEventTypeBack) {
        if(state == NfcSceneEmulateStateTextBox) {
            view_dispatcher_switch_to_view(instance->view_dispatcher, NfcViewWidget);
            scene_manager_set_scene_state(
                instance->scene_manager, NfcSceneEmulate, NfcSceneEmulateStateWidgetLog);
            consumed = true;
        }
    }

    return consumed;
}

static void nfc_protocol_support_scene_emulate_stop_listener(NfcApp* instance) {
    nfc_listener_stop(instance->listener);

    const NfcProtocol protocol = nfc_device_get_protocol(instance->nfc_device);

    if(protocol == nfc_listener_get_protocol(instance->listener)) {
        const NfcDeviceData* data = nfc_listener_get_data(instance->listener, protocol);

        if(!nfc_device_is_equal_data(instance->nfc_device, protocol, data)) {
            nfc_device_set_data(instance->nfc_device, protocol, data);
            nfc_save_shadow_file(instance);
        }
    }

    nfc_listener_free(instance->listener);
}

static void nfc_protocol_support_scene_emulate_on_exit(NfcApp* instance) {
    nfc_protocol_support_scene_emulate_stop_listener(instance);

    // Clear view
    widget_reset(instance->widget);
    text_box_reset(instance->text_box);
    furi_string_reset(instance->text_box_store);

    nfc_blink_stop(instance);
}

static void nfc_protocol_support_scene_rpc_on_enter(NfcApp* instance) {
    UNUSED(instance);
}

static void nfc_protocol_support_scene_rpc_setup_ui_and_emulate(NfcApp* instance) {
    nfc_text_store_set(instance, "emulating\n%s", furi_string_get_cstr(instance->file_name));

    popup_set_header(instance->popup, "NFC", 89, 42, AlignCenter, AlignBottom);
    popup_set_text(instance->popup, instance->text_store, 89, 44, AlignCenter, AlignTop);
    popup_set_icon(instance->popup, 0, 12, &I_RFIDDolphinSend_97x61);

    view_dispatcher_switch_to_view(instance->view_dispatcher, NfcViewPopup);

    notification_message(instance->notifications, &sequence_display_backlight_on);
    nfc_blink_emulate_start(instance);

    const NfcProtocol protocol = nfc_device_get_protocol(instance->nfc_device);
    nfc_protocol_support[protocol]->scene_emulate.on_enter(instance);

    instance->rpc_state = NfcRpcStateEmulating;
}

static bool nfc_protocol_support_scene_rpc_on_event(NfcApp* instance, SceneManagerEvent event) {
    bool consumed = false;

    if(event.type == SceneManagerEventTypeCustom) {
        if(event.event == NfcCustomEventRpcLoadFile) {
            bool success = false;
            if(instance->rpc_state == NfcRpcStateIdle) {
                if(nfc_load_file(instance, instance->file_path, false)) {
                    nfc_protocol_support_scene_rpc_setup_ui_and_emulate(instance);
                    success = true;
                } else {
                    rpc_system_app_set_error_code(
                        instance->rpc_ctx, RpcAppSystemErrorCodeParseFile);
                    rpc_system_app_set_error_text(instance->rpc_ctx, "Cannot load key file");
                }
            }
            rpc_system_app_confirm(instance->rpc_ctx, success);
        } else if(event.event == NfcCustomEventRpcExit) {
            rpc_system_app_confirm(instance->rpc_ctx, true);
            scene_manager_stop(instance->scene_manager);
            view_dispatcher_stop(instance->view_dispatcher);
        } else if(event.event == NfcCustomEventRpcSessionClose) {
            scene_manager_stop(instance->scene_manager);
            view_dispatcher_stop(instance->view_dispatcher);
        }
        consumed = true;
    }

    return consumed;
}

static void nfc_protocol_support_scene_rpc_on_exit(NfcApp* instance) {
    if(instance->rpc_state == NfcRpcStateEmulating) {
        nfc_protocol_support_scene_emulate_stop_listener(instance);
    }

    popup_reset(instance->popup);
    text_box_reset(instance->text_box);
    furi_string_reset(instance->text_box_store);

    nfc_blink_stop(instance);
}

static const NfcProtocolSupportCommonSceneBase
    nfc_protocol_support_scenes[NfcProtocolSupportSceneCount] = {
        [NfcProtocolSupportSceneInfo] =
            {
                .on_enter = nfc_protocol_support_scene_info_on_enter,
                .on_event = nfc_protocol_support_scene_info_on_event,
                .on_exit = nfc_protocol_support_scene_info_on_exit,
            },
        [NfcProtocolSupportSceneMoreInfo] =
            {
                .on_enter = nfc_protocol_support_scene_more_info_on_enter,
                .on_event = nfc_protocol_support_scene_more_info_on_event,
                .on_exit = nfc_protocol_support_scene_more_info_on_exit,
            },
        [NfcProtocolSupportSceneRead] =
            {
                .on_enter = nfc_protocol_support_scene_read_on_enter,
                .on_event = nfc_protocol_support_scene_read_on_event,
                .on_exit = nfc_protocol_support_scene_read_on_exit,
            },
        [NfcProtocolSupportSceneReadMenu] =
            {
                .on_enter = nfc_protocol_support_scene_read_menu_on_enter,
                .on_event = nfc_protocol_support_scene_read_menu_on_event,
                .on_exit = nfc_protocol_support_scene_read_saved_menu_on_exit,
            },
        [NfcProtocolSupportSceneReadSuccess] =
            {
                .on_enter = nfc_protocol_support_scene_read_success_on_enter,
                .on_event = nfc_protocol_support_scene_read_success_on_event,
                .on_exit = nfc_protocol_support_scene_read_success_on_exit,
            },
        [NfcProtocolSupportSceneSavedMenu] =
            {
                .on_enter = nfc_protocol_support_scene_saved_menu_on_enter,
                .on_event = nfc_protocol_support_scene_saved_menu_on_event,
                .on_exit = nfc_protocol_support_scene_read_saved_menu_on_exit,
            },
        [NfcProtocolSupportSceneSaveName] =
            {
                .on_enter = nfc_protocol_support_scene_save_name_on_enter,
                .on_event = nfc_protocol_support_scene_save_name_on_event,
                .on_exit = nfc_protocol_support_scene_save_name_on_exit,
            },
        [NfcProtocolSupportSceneEmulate] =
            {
                .on_enter = nfc_protocol_support_scene_emulate_on_enter,
                .on_event = nfc_protocol_support_scene_emulate_on_event,
                .on_exit = nfc_protocol_support_scene_emulate_on_exit,
            },
        [NfcProtocolSupportSceneRpc] =
            {
                .on_enter = nfc_protocol_support_scene_rpc_on_enter,
                .on_event = nfc_protocol_support_scene_rpc_on_event,
                .on_exit = nfc_protocol_support_scene_rpc_on_exit,
            },
};
