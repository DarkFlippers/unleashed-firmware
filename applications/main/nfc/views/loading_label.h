#pragma once
#include <gui/view.h>

#ifdef __cplusplus
extern "C" {
#endif

/** LoadingLabel anonymous structure */
typedef struct LoadingLabel LoadingLabel;

/** Allocate and initialize
 *
 * Like the Loading view, but draws the animated spinner on the left with a text
 * label to its right (e.g. to name the slow operation in progress).
 *
 * @return     LoadingLabel View instance
 */
LoadingLabel* loading_label_alloc(void);

/** Deinitialize and free LoadingLabel View
 *
 * @param      instance  LoadingLabel instance
 */
void loading_label_free(LoadingLabel* instance);

/** Get LoadingLabel view
 *
 * @param      instance  LoadingLabel instance
 *
 * @return     View instance that can be used for embedding
 */
View* loading_label_get_view(LoadingLabel* instance);

/** Set the label shown next to the spinner
 *
 * @param      instance  LoadingLabel instance
 * @param      text      Text to show (caller-owned, must outlive the view); may be multiline
 */
void loading_label_set_text(LoadingLabel* instance, const char* text);

#ifdef __cplusplus
}
#endif
