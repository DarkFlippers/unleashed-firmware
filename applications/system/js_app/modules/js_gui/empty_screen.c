#include "../../js_modules.h" // IWYU pragma: keep
#include "js_gui.h"
#include <gui/modules/empty_screen.h>

static const JsViewDescriptor view_descriptor = {
    .alloc = (JsViewAlloc)empty_screen_alloc,
    .free = (JsViewFree)empty_screen_free,
    .get_view = (JsViewGetView)empty_screen_get_view,
    .prop_cnt = 0,
    .props = {},
};
JS_GUI_VIEW_DEF(empty_screen, &view_descriptor);
