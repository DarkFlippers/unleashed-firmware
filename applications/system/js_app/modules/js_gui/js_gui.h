#include "../../js_modules.h"
#include <gui/view.h>

#ifdef __cplusplus
extern "C" {
#endif

typedef enum {
    JsViewPropTypeString,
    JsViewPropTypeNumber,
    JsViewPropTypeArr,
    JsViewPropTypeTypedArr,
    JsViewPropTypeBool,
} JsViewPropType;

typedef union {
    const char* string;
    int32_t number;
    bool boolean;
    mjs_val_t term;
} JsViewPropValue;

/**
 * @brief Assigns a value to a view property
 * 
 * The name and the type are implicit and defined in the property descriptor
 */
typedef bool (
    *JsViewPropAssign)(struct mjs* mjs, void* specific_view, JsViewPropValue value, void* context);

/** @brief Property descriptor */
typedef struct {
    const char* name; //<! Property name, as visible from JS
    JsViewPropType type; // <! Property type, ensured by the GUI module
    JsViewPropAssign assign; // <! Property assignment callback
} JsViewPropDescriptor;

// View method signatures

/** @brief View's `_alloc` method */
typedef void* (*JsViewAlloc)(void);
/** @brief View's `_get_view` method */
typedef View* (*JsViewGetView)(void* specific_view);
/** @brief View's `_free` method */
typedef void (*JsViewFree)(void* specific_view);

// Glue code method signatures

/** @brief Context instantiation for glue code */
typedef void* (*JsViewCustomMake)(struct mjs* mjs, void* specific_view, mjs_val_t view_obj);
/** @brief Context destruction for glue code */
typedef void (*JsViewCustomDestroy)(void* specific_view, void* custom_state, FuriEventLoop* loop);
/** @brief `addChild` callback for glue code */
typedef bool (
    *JsViewAddChild)(struct mjs* mjs, void* specific_view, void* custom_state, mjs_val_t child_obj);
/** @brief `resetChildren` callback for glue code */
typedef void (*JsViewResetChildren)(void* specific_view, void* custom_state);

/**
 * @brief Descriptor for a JS view
 * 
 * Contains:
 *   - Pointers to generic view methods (`alloc`, `get_view` and `free`)
 *   - Pointers to glue code context ctor/dtor methods (`custom_make`,
 *     `custom_destroy`)
 *   - Descriptors of properties visible from JS (`prop_cnt`, `props`)
 * 
 * `js_gui` uses this descriptor to produce view factories and views.
 */
typedef struct {
    JsViewAlloc alloc;
    JsViewGetView get_view;
    JsViewFree free;

    JsViewCustomMake custom_make; // <! May be NULL
    JsViewCustomDestroy custom_destroy; // <! May be NULL

    JsViewAddChild add_child; // <! May be NULL
    JsViewResetChildren reset_children; // <! May be NULL

    size_t prop_cnt; //<! Number of properties visible from JS
    JsViewPropDescriptor props[]; // <! Descriptors of properties visible from JS
} JsViewDescriptor;

// Callback ordering:
//                                   +-> add_child       -+
//                                   +-> reset_children  -+
// alloc -> get_view -> custom_make -+-> props[i].assign -+> custom_destroy -> free
// \__________ creation __________/      \____ use ____/     \___ destruction ____/

/**
 * @brief Creates a JS `ViewFactory` object
 * 
 * This function is intended to be used by individual view adapter modules that
 * wish to create a unified JS API interface in a declarative way. Usually this
 * is done via the `JS_GUI_VIEW_DEF` macro which hides all the boilerplate.
 * 
 * The `ViewFactory` object exposes two methods, `make` and `makeWith`, each
 * returning a `View` object. These objects fully comply with the expectations
 * of the `ViewDispatcher`, TS type definitions and the proposed Flipper JS
 * coding style.
 */
mjs_val_t js_gui_make_view_factory(struct mjs* mjs, const JsViewDescriptor* view_descriptor);

/**
 * @brief Defines a module implementing `View` glue code
 */
#define JS_GUI_VIEW_DEF(name, descriptor)                                                \
    static void* view_mod_ctor(struct mjs* mjs, mjs_val_t* object, JsModules* modules) { \
        UNUSED(modules);                                                                 \
        *object = js_gui_make_view_factory(mjs, descriptor);                             \
        return NULL;                                                                     \
    }                                                                                    \
    static const JsModuleDescriptor js_mod_desc = {                                      \
        "gui__" #name,                                                                   \
        view_mod_ctor,                                                                   \
        NULL,                                                                            \
        NULL,                                                                            \
    };                                                                                   \
    static const FlipperAppPluginDescriptor plugin_descriptor = {                        \
        .appid = PLUGIN_APP_ID,                                                          \
        .ep_api_version = PLUGIN_API_VERSION,                                            \
        .entry_point = &js_mod_desc,                                                     \
    };                                                                                   \
    const FlipperAppPluginDescriptor* js_view_##name##_ep(void) {                        \
        return &plugin_descriptor;                                                       \
    }

#ifdef __cplusplus
}
#endif
