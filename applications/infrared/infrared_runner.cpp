#include "infrared_app.h"

extern "C" int32_t infrared_app(void* p) {
    InfraredApp* app = new InfraredApp();
    int32_t result = app->run(p);
    delete app;

    return result;
}
