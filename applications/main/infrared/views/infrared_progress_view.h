/**
  * @file infrared_progress_view.h
  * Infrared: Custom Infrared view module.
  * It shows popup progress bar during brute force.
  */
#pragma once
#include <gui/view.h>

#ifdef __cplusplus
extern "C" {
#endif

/** Anonymous instance */
typedef struct InfraredProgressView InfraredProgressView;

typedef enum {
    InfraredProgressViewInputStop,
    InfraredProgressViewInputPause,
    InfraredProgressViewInputResume,
    InfraredProgressViewInputPreviousSignal,
    InfraredProgressViewInputNextSignal,
    InfraredProgressViewInputSendSingle,
} InfraredProgressViewInput;

/** Callback for input handling */
typedef void (*InfraredProgressViewInputCallback)(void* context, InfraredProgressViewInput event);

/** Allocate and initialize Infrared view
 *
 * @retval new allocated instance
 */
InfraredProgressView* infrared_progress_view_alloc(void);

/** Free previously allocated Progress view module instance
 *
 * @param instance to free
 */
void infrared_progress_view_free(InfraredProgressView* instance);

/** Get progress view module view
 *
 * @param instance view module
 * @retval view
 */
View* infrared_progress_view_get_view(InfraredProgressView* instance);

/** Set progress of progress view module
 *
 * @param instance view module
 * @param progress progress value
 */
bool infrared_progress_view_set_progress(InfraredProgressView* instance, uint16_t progress);

/** Set maximum progress value
 *
 * @param instance - view module
 * @param progress_max - maximum value of progress
 */
void infrared_progress_view_set_progress_total(
    InfraredProgressView* instance,
    uint16_t progress_max);

/** Selects the variant of the View
 * 
 * @param instance view instance
 * @param is_paused the "paused" variant is displayed if true; the "sending" one if false
 */
void infrared_progress_view_set_paused(InfraredProgressView* instance, bool is_paused);

/** Set input callback
 *
 * @param instance - view module
 * @param callback - callback to call for input
 * @param context - context to pass to callback
 */
void infrared_progress_view_set_input_callback(
    InfraredProgressView* instance,
    InfraredProgressViewInputCallback callback,
    void* context);

#ifdef __cplusplus
}
#endif
