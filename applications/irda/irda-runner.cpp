#include "irda-app.hpp"

extern "C" int32_t irda_app(void* p) {
    IrdaApp* app = new IrdaApp();
    int32_t result = app->run(p);
    delete app;

    return result;
}
