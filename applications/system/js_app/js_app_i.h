#include <furi.h>
#include <gui/gui.h>
#include <gui/view_dispatcher.h>
#include <gui/modules/loading.h>
#include "views/console_view.h"

typedef enum {
    JsAppViewConsole,
    JsAppViewLoading,
} JsAppView;
