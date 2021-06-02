#include "irda-app.hpp"

extern "C" int32_t irda(void* p) {
    IrdaApp* app = new IrdaApp();
    app->run();
    delete app;

    return 0;
}
