/**
 * @file view_stack.h
 * GUI: ViewStack API
 *
 * ViewStack accumulates several Views in one stack.
 * Draw callbacks are called sequenctially starting from
 * first added. Input callbacks are called in reverse order.
 * Consumed input is not passed on underlying layers.
 */

#pragma once

#include <stdbool.h>
#include "view.h"

#ifdef __cplusplus
extern "C" {
#endif

/** ViewStack, anonymous type. */
typedef struct ViewStack ViewStack;

/** Allocate and init ViewStack
 *
 * @return      ViewStack instance
 */
ViewStack* view_stack_alloc(void);

/** Free ViewStack instance
 *
 * @param       view_stack  instance
 */
void view_stack_free(ViewStack* view_stack);

/** Get View of ViewStack.
 * Should this View to any view manager such as
 * ViewDispatcher or ViewHolder.
 *
 * @param       view_stack  instance
 */
View* view_stack_get_view(ViewStack* view_stack);

/** Add View to ViewStack.
 * Adds View on top of ViewStack.
 *
 * @param       view_stack  instance
 * @view        view        view to add
 */
void view_stack_add_view(ViewStack* view_stack, View* view);

/** Remove any View in ViewStack.
 * If no View to remove found - ignore.
 *
 * @param       view_stack  instance
 * @view        view        view to remove
 */
void view_stack_remove_view(ViewStack* view_stack, View* view);

#ifdef __cplusplus
}
#endif
