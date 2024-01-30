/**
 * @file nfc_protocol_support_gui_common.h
 * @brief Common GUI functions and definitions.
 */
#pragma once

#include <gui/modules/widget.h>

#include "nfc/nfc_app.h"
#include "nfc/nfc_app_i.h"

/**
 * @brief Common submenu indices.
 */
enum {
    SubmenuIndexCommonSave, /**< Save menu option. */
    SubmenuIndexCommonEmulate, /**< Emulate menu option. */
    SubmenuIndexCommonEdit, /**< Edit menu option. */
    SubmenuIndexCommonInfo, /**< Info menu option. */
    SubmenuIndexCommonRename, /**< Rename menu option. */
    SubmenuIndexCommonDelete, /**< Delete menu option. */
    SubmenuIndexCommonRestore, /**< Restore menu option. */
    SubmenuIndexCommonMax, /**< Special value, internal use. */
};

/**
 * @brief Common submenu callback.
 *
 * Called each time the user presses on a selected submenu item.
 *
 * @param[in,out] context pointer to a user-defined context object.
 * @param[in] index index of the item that was activated.
 */
void nfc_protocol_support_common_submenu_callback(void* context, uint32_t index);

/**
 * @brief Common widget callback.
 *
 * Called each time the user presses on a selected widget element.
 *
 * @param[in] result identifier of the activated button.
 * @param[in] type type of press action.
 * @param[in,out] context pointer to a user-defined context object.
 */
void nfc_protocol_support_common_widget_callback(
    GuiButtonType result,
    InputType type,
    void* context);

/**
 * @brief Common byte input callback.
 *
 * Called each time the user accepts the byte input.
 *
 * @param[in,out] context pointer to a user-defined context object.
 */
void nfc_protocol_support_common_byte_input_done_callback(void* context);

/**
 * @brief Common text input callback.
 *
 * Called each time the user accepts the text input.
 *
 * @param[in,out] context pointer to a user-defined context object.
 */
void nfc_protocol_support_common_text_input_done_callback(void* context);

/**
 * @brief Empty on_enter() handler.
 *
 * Does nothing.
 *
 * @param[in] instance pointer to the NFC application instance.
 */
void nfc_protocol_support_common_on_enter_empty(NfcApp* instance);

/**
 * @brief Empty on_event() handler.
 *
 * Does nothing and returns true.
 *
 * @param[in] instance pointer to the NFC application instance.
 * @param[in] event custom event type that has occurred.
 * @returns always true.
 */
bool nfc_protocol_support_common_on_event_empty(NfcApp* instance, SceneManagerEvent event);
