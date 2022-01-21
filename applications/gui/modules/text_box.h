/**
 * @file text_box.h
 * GUI: TextBox view module API
 */

#pragma once

#include <gui/view.h>

#ifdef __cplusplus
extern "C" {
#endif

/** TextBox anonymous structure */
typedef struct TextBox TextBox;
typedef void (*TextBoxExitCallback)(void* context);

typedef enum {
    TextBoxFontText,
    TextBoxFontHex,
} TextBoxFont;

/** Allocate and initialize text_box
 *
 * @return     TextBox instance
 */
TextBox* text_box_alloc();

/** Deinitialize and free text_box
 *
 * @param      text_box  text_box instance
 */
void text_box_free(TextBox* text_box);

/** Get text_box view
 *
 * @param      text_box  TextBox instance
 *
 * @return     View instance that can be used for embedding
 */
View* text_box_get_view(TextBox* text_box);

/** Clean text_box
 *
 * @param      text_box  TextBox instance
 */
void text_box_reset(TextBox* text_box);

/** Set text for text_box
 *
 * @param      text_box  TextBox instance
 * @param      text      text to set
 */
void text_box_set_text(TextBox* text_box, const char* text);

/** Set TextBox font
 *
 * @param      text_box  TextBox instance
 * @param      font      TextBoxFont instance
 */
void text_box_set_font(TextBox* text_box, TextBoxFont font);

/** Set text_box context
 *
 * @param      text_box  TextBox instance
 * @param      context   context pointer
 */
void text_box_set_context(TextBox* text_box, void* context);

/** Set exit callback
 *
 * @param      text_box  TextBox instance
 * @param      callback  TextBoxExitCallback callback pointer
 */
void text_box_set_exit_callback(TextBox* text_box, TextBoxExitCallback callback);

#ifdef __cplusplus
}
#endif
