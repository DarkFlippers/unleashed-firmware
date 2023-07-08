#pragma once
#include <furi.h>
#include <gui/canvas.h>
#include <gui/modules/file_browser.h>

#ifdef __cplusplus
extern "C" {
#endif

/****************** COMMON ******************/

#define RECORD_DIALOGS "dialogs"

typedef struct DialogsApp DialogsApp;

/****************** FILE BROWSER ******************/

/**
 * File browser dialog extra options.
 * This can be default-initialized using {@link dialog_file_browser_set_basic_options}.
 * @param extension file extension to be offered for selection
 * @param base_path root folder path for navigation with back key
 * @param skip_assets true - do not show assets folders
 * @param hide_dot_files true - hide dot files
 * @param icon file icon pointer, NULL for default icon
 * @param hide_ext true - hide extensions for files
 * @param item_loader_callback callback function for providing custom icon & entry name
 * @param hide_ext callback context
 */
typedef struct {
    const char* extension;
    const char* base_path;
    bool skip_assets;
    bool hide_dot_files;
    const Icon* icon;
    bool hide_ext;
    FileBrowserLoadItemCallback item_loader_callback;
    void* item_loader_context;
} DialogsFileBrowserOptions;

/**
 * Initialize file browser dialog options and set default values.
 * This is guaranteed to initialize all fields
 * so it is safe to pass pointer to uninitialized {@code options}
 * and assume that the data behind it becomes fully initialized after the call.
 * @param options pointer to options structure
 * @param extension file extension to filter
 * @param icon file icon pointer, NULL for default icon
 */
void dialog_file_browser_set_basic_options(
    DialogsFileBrowserOptions* options,
    const char* extension,
    const Icon* icon);

/**
 * Shows and processes the file browser dialog
 * @param context api pointer
 * @param result_path selected file path string pointer
 * @param path preselected file path string pointer
 * @param options file browser dialog extra options, may be null
 * @return bool whether a file was selected
 */
bool dialog_file_browser_show(
    DialogsApp* context,
    FuriString* result_path,
    FuriString* path,
    const DialogsFileBrowserOptions* options);

/****************** MESSAGE ******************/

/**
 * Message result type
 */
typedef enum {
    DialogMessageButtonBack,
    DialogMessageButtonLeft,
    DialogMessageButtonCenter,
    DialogMessageButtonRight,
} DialogMessageButton;

/**
 * Message struct
 */
typedef struct DialogMessage DialogMessage;

/**
 * Allocate and fill message
 * @return DialogMessage* 
 */
DialogMessage* dialog_message_alloc();

/**
 * Free message struct
 * @param message message pointer
 */
void dialog_message_free(DialogMessage* message);

/**
 * Set message text
 * @param message message pointer
 * @param text text, can be NULL if you don't want to display the text
 * @param x x position
 * @param y y position
 * @param horizontal horizontal alignment
 * @param vertical vertical alignment
 */
void dialog_message_set_text(
    DialogMessage* message,
    const char* text,
    uint8_t x,
    uint8_t y,
    Align horizontal,
    Align vertical);

/**
 * Set message header
 * @param message message pointer
 * @param text text, can be NULL if you don't want to display the header
 * @param x x position
 * @param y y position
 * @param horizontal horizontal alignment
 * @param vertical vertical alignment
 */
void dialog_message_set_header(
    DialogMessage* message,
    const char* text,
    uint8_t x,
    uint8_t y,
    Align horizontal,
    Align vertical);

/**
 * Set message icon
 * @param message message pointer
 * @param icon icon pointer, can be NULL if you don't want to display the icon
 * @param x x position
 * @param y y position
 */
void dialog_message_set_icon(DialogMessage* message, const Icon* icon, uint8_t x, uint8_t y);

/**
 * Set message buttons text, button text can be NULL if you don't want to display and process some buttons
 * @param message message pointer
 * @param left left button text, can be NULL if you don't want to display the left button
 * @param center center button text, can be NULL if you don't want to display the center button
 * @param right right button text, can be NULL if you don't want to display the right button
 */
void dialog_message_set_buttons(
    DialogMessage* message,
    const char* left,
    const char* center,
    const char* right);

/**
 * Show message from filled struct
 * @param context api pointer
 * @param message message struct pointer to be shown
 * @return DialogMessageButton type
 */
DialogMessageButton dialog_message_show(DialogsApp* context, const DialogMessage* message);

/**
 * Show SD error message (with question sign)
 * @param context 
 * @param error_text 
 */
void dialog_message_show_storage_error(DialogsApp* context, const char* error_text);

#ifdef __cplusplus
}
#endif
