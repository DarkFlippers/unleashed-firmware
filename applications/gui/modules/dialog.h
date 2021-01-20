#pragma once

#include <gui/view.h>

/* Dialog anonymous structure */
typedef struct Dialog Dialog;

/* Dialog result */
typedef enum {
    DialogResultLeft,
    DialogResultRight,
} DialogResult;

/* Dialog result callback type
 * @warning comes from GUI thread
 */
typedef void (*DialogResultCallback)(DialogResult result, void* context);

/* Allocate and initialize dialog
 * This dialog used to ask simple questions like Yes/
 */
Dialog* dialog_alloc();

/* Deinitialize and free dialog
 * @param dialog - Dialog instance
 */
void dialog_free(Dialog* dialog);

/* Get dialog view
 * @param dialog - Dialog instance
 * @return View instance that can be used for embedding
 */
View* dialog_get_view(Dialog* dialog);

/* Set dialog header text
 * @param dialog - Dialog instance
 * @param text - text to be shown
 */
void dialog_set_result_callback(Dialog* dialog, DialogResultCallback callback);

/* Set dialog header text
 * @param dialog - Dialog instance
 * @param context - context pointer, will be passed to result callback
 */
void dialog_set_context(Dialog* dialog, void* context);

/* Set dialog header text
 * @param dialog - Dialog instance
 * @param text - text to be shown
 */
void dialog_set_header_text(Dialog* dialog, const char* text);

/* Set dialog text
 * @param dialog - Dialog instance
 * @param text - text to be shown
 */
void dialog_set_text(Dialog* dialog, const char* text);

/* Set left button text
 * @param dialog - Dialog instance
 * @param text - text to be shown
 */
void dialog_set_left_button_text(Dialog* dialog, const char* text);

/* Set right button text
 * @param dialog - Dialog instance
 * @param text - text to be shown
 */
void dialog_set_right_button_text(Dialog* dialog, const char* text);
