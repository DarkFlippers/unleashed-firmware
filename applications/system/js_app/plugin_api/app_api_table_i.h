#include "js_plugin_api.h"
/* 
 * A list of app's private functions and objects to expose for plugins.
 * It is used to generate a table of symbols for import resolver to use.
 * TBD: automatically generate this table from app's header files
 */
static constexpr auto app_api_table = sort(create_array_t<sym_entry>(
    API_METHOD(js_delay_with_flags, bool, (struct mjs*, uint32_t)),
    API_METHOD(js_flags_set, void, (struct mjs*, uint32_t)),
    API_METHOD(js_flags_wait, uint32_t, (struct mjs*, uint32_t, uint32_t)),
    API_METHOD(js_module_get, void*, (JsModules*, const char*))));
