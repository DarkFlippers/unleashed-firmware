#include "../../js_modules.h" // IWYU pragma: keep
#include "js_gui.h"
#include <gui/modules/loading.h>

static const JsViewDescriptor view_descriptor = {
    .alloc = (JsViewAlloc)loading_alloc,
    .free = (JsViewFree)loading_free,
    .get_view = (JsViewGetView)loading_get_view,
    .prop_cnt = 0,
    .props = {},
};
JS_GUI_VIEW_DEF(loading, &view_descriptor);
