#include "js_event_loop.h"

static constexpr auto js_event_loop_api_table = sort(
    create_array_t<sym_entry>(API_METHOD(js_event_loop_get_loop, FuriEventLoop*, (JsEventLoop*))));
