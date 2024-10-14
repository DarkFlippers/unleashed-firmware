#pragma once
#include "../js_thread_i.h"

void* js_flipper_create(struct mjs* mjs, mjs_val_t* object, JsModules* modules);
