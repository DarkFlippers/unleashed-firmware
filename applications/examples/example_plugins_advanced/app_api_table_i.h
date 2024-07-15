#include "app_api.h"

/* 
 * A list of app's private functions and objects to expose for plugins.
 * It is used to generate a table of symbols for import resolver to use.
 * TBD: automatically generate this table from app's header files
 */
static constexpr auto app_api_table = sort(create_array_t<sym_entry>(
    API_METHOD(app_api_accumulator_set, void, (uint32_t)),
    API_METHOD(app_api_accumulator_get, uint32_t, ()),
    API_METHOD(app_api_accumulator_add, void, (uint32_t)),
    API_METHOD(app_api_accumulator_sub, void, (uint32_t)),
    API_METHOD(app_api_accumulator_mul, void, (uint32_t))));
