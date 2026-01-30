#pragma once

#include <gui/view.h>
#include "../helpers/duck3_script.h"

typedef struct BadDuck3View BadDuck3View;
typedef void (*BadDuck3ButtonCallback)(InputKey key, void* context);

BadDuck3View* bad_duck3_view_alloc(void);

void bad_duck3_view_free(BadDuck3View* view);

View* bad_duck3_view_get_view(BadDuck3View* view);

void bad_duck3_view_set_button_callback(
    BadDuck3View* view,
    BadDuck3ButtonCallback callback,
    void* context);

void bad_duck3_view_set_file_name(BadDuck3View* view, const char* name);

void bad_duck3_view_set_layout(BadDuck3View* view, const char* layout);

void bad_duck3_view_set_state(BadDuck3View* view, Duck3State* st);

void bad_duck3_view_set_interface(BadDuck3View* view, Duck3HidInterface interface);

bool bad_duck3_view_is_idle_state(BadDuck3View* view);
