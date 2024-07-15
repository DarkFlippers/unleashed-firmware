#pragma once
#include "js_thread_i.h"
#include <flipper_application/flipper_application.h>
#include <flipper_application/plugins/plugin_manager.h>
#include <flipper_application/plugins/composite_resolver.h>

#define PLUGIN_APP_ID      "js"
#define PLUGIN_API_VERSION 1

typedef void* (*JsModeConstructor)(struct mjs* mjs, mjs_val_t* object);
typedef void (*JsModeDestructor)(void* inst);

typedef struct {
    char* name;
    JsModeConstructor create;
    JsModeDestructor destroy;
} JsModuleDescriptor;

typedef struct JsModules JsModules;

JsModules* js_modules_create(struct mjs* mjs, CompositeApiResolver* resolver);

void js_modules_destroy(JsModules* modules);

mjs_val_t js_module_require(JsModules* modules, const char* name, size_t name_len);
