#pragma once
#include "../js_thread_i.h"
#include "../js_modules.h"

void* js_tests_create(struct mjs* mjs, mjs_val_t* object, JsModules* modules);
