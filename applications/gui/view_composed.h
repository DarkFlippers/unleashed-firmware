#pragma once

#include <stdbool.h>
#include "view.h"

typedef struct ViewComposed ViewComposed;

ViewComposed* view_composed_alloc(void);
void view_composed_free(ViewComposed* view_composed);
void view_composed_top_enable(ViewComposed* view_composed, bool enable);
void view_composed_tie_views(ViewComposed* view_composed, View* view_bottom, View* view_top);
View* view_composed_get_view(ViewComposed* view_composed);
