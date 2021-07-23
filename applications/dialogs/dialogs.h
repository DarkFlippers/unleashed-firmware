#pragma once
#include <furi.h>
#include <gui/canvas.h>

#ifdef __cplusplus
extern "C" {
#endif

/****************** COMMON ******************/

typedef struct DialogsApp DialogsApp;

/****************** FILE SELECT ******************/

/**
 * Shows and processes the file selection dialog
 * @param context api pointer
 * @param path path to directory
 * @param extension file extension to be offered for selection
 * @param selected_filename buffer where the selected filename will be saved
 * @param selected_filename_size and the size of this buffer
 * @param preselected_filename filename to be preselected
 * @return bool whether a file was selected
 */
bool dialog_file_select_show(
    DialogsApp* context,
    const char* path,
    const char* extension,
    char* result,
    uint8_t result_size,
    const char* preselected_filename);

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

#ifdef __cplusplus
}
#endif